#include "AudioCapture.h"
#include <thread>
#include <minwinbase.h>
#include "SpectrumEngine.h"

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

AudioCapture::AudioCapture()
{
	CoInitialize(NULL);
	CoCreateInstance(__uuidof(MMDeviceEnumerator),
							  NULL,
							  CLSCTX_ALL,
							  __uuidof(IMMDeviceEnumerator),
							  (void**)&pOutputEnumerator);
	pOutputEnumerator->RegisterEndpointNotificationCallback(this);
}

std::vector<std::shared_ptr<AudioDevice>> AudioCapture::enumDevices(EDataFlow type /*= eRender*/)
{
	std::vector<std::shared_ptr<AudioDevice>> devices;
	IMMDeviceCollection* pCollection = NULL;
	IMMDevice* defaultDevice;
	LPWSTR defaultId;
	UINT count;
	IPropertyStore* pProps = NULL;
	PROPVARIANT propVar;
	HRESULT hr;
	hr = pOutputEnumerator->EnumAudioEndpoints(type, eMultimedia, &pCollection);
	if (FAILED(hr)) {
		printf("Unable to activate audio client: %x.\n", hr);
		return devices;
	}

	hr = pOutputEnumerator->GetDefaultAudioEndpoint(type, eMultimedia, &defaultDevice);
	if (FAILED(hr)) {
		printf("Unable to activate audio client: %x.\n", hr);
		return devices;
	}

	hr = defaultDevice->GetId(&defaultId);
	if (FAILED(hr)) {
		printf("Unable to activate audio client: %x.\n", hr);
		return devices;
	}

	hr = pCollection->GetCount(&count);
	if (FAILED(hr)) {
		printf("Unable to activate audio client: %x.\n", hr);
		return devices;
	}

	for (UINT i = 0; i < count; i++) {
		std::shared_ptr<AudioDevice> device(new AudioDevice(), [](AudioDevice* dev) {
			CoTaskMemFree(dev->id);
			SAFE_RELEASE(dev->pDevice);
		});
		hr = pCollection->Item(i, &device->pDevice);
		if (FAILED(hr)) {
			printf("Unable to activate audio client: %x.\n", hr);
			return devices;
		}

		hr = device->pDevice->GetId(&device->id);
		if (FAILED(hr)) {
			printf("Unable to activate audio client: %x.\n", hr);
			return devices;
		}

		device->isDefault = (wcscmp(device->id, defaultId) == 0);
		if (FAILED(hr)) {
			printf("Unable to activate audio client: %x.\n", hr);
			return devices;
		}

		hr = device->pDevice->OpenPropertyStore(STGM_READ, &pProps);
		if (FAILED(hr)) {
			printf("Unable to activate audio client: %x.\n", hr);
			return devices;
		}

		device->type = type;
		PropVariantInit(&propVar);
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &propVar);
		if (FAILED(hr)) {
			printf("Unable to activate audio client: %x.\n", hr);
			return devices;
		}

		device->name = propVar.pwszVal;
		PropVariantClear(&propVar);
		hr = pProps->GetValue(PKEY_AudioEngine_DeviceFormat, &propVar);
		if (FAILED(hr)) {
			printf("Unable to activate audio client: %x.\n", hr);
			return devices;
		}

		device->format = *(PWAVEFORMATEX)propVar.blob.pBlobData;
		device->format.cbSize = 0;
		device->format.wFormatTag = WAVE_FORMAT_PCM;
		PropVariantClear(&propVar);
		devices.push_back(device);
		SAFE_RELEASE(pProps);
	}
	pCollection->Release();
	return devices;
}

void AudioCapture::start(std::shared_ptr<AudioDevice> device) {
	running = true;
	stoped = std::promise<bool>();
	captureThread = std::make_shared<std::thread>([device, this]() {
		IAudioClient* _AudioClient;
		IAudioCaptureClient* _CaptureClient;
		HANDLE _AudioSamplesReadyEvent = NULL;
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		_AudioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
		if (_AudioSamplesReadyEvent == NULL) {
			printf("Unable to create samples ready event: %d.\n", GetLastError());
			return false;
		}

		hr = device->pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, reinterpret_cast<void**>(&_AudioClient));
		if (FAILED(hr)) {
			printf("Unable to activate audio client: %x.\n", hr);
			return false;
		}

		size_t _FrameSize = (device->format.wBitsPerSample / 8) * device->format.nChannels;

		UINT32        _BufferSize;

		//Initialize Audio Engine
		hr = _AudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, device->type == eRender ?
					AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST
				  : (AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST), 20 * 10000, 0, &device->format, NULL);

		if (FAILED(hr)) {
			printf("Unable to initialize audio client: %x.\n", hr);
			return false;
		}

		hr = _AudioClient->GetBufferSize(&_BufferSize);
		if (FAILED(hr)) {
			printf("Unable to get audio client buffer: %x. \n", hr);
			return false;
		}
		hr = _AudioClient->SetEventHandle(_AudioSamplesReadyEvent);
		if (FAILED(hr)) {
			printf("Unable to set ready event: %x.\n", hr);
			return false;
		}

		hr = _AudioClient->GetService(IID_PPV_ARGS(&_CaptureClient));
		if (FAILED(hr)) {
			printf("Unable to get new capture client: %x.\n", hr);
			return false;
		}

		hr = _AudioClient->Start();
		if (FAILED(hr)) {
			printf("Unable to get new capture client: %x.\n", hr);
			return false;
		}
		while (running) {
			DWORD waitResult = WaitForSingleObject(_AudioSamplesReadyEvent, INFINITE);
			BYTE* pData;
			INT nBufferLenght;
			UINT32 framesAvailable;
			DWORD  flags;
			hr = _CaptureClient->GetBuffer(&pData, &framesAvailable, &flags, NULL, NULL);
			if (SUCCEEDED(hr)) {
				if (framesAvailable != 0) {
					if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
						//  Fill 0s from the capture buffer to the output buffer.
					}
					else {
						SpectrumEngine::getInstance()->submitAudioData(pData, framesAvailable * _FrameSize);
					}
				}
				hr = _CaptureClient->ReleaseBuffer(framesAvailable);
				if (FAILED(hr)) {
					printf("Unable to release capture buffer: %x!\n", hr);
				}
			}
		}
		SAFE_RELEASE(_CaptureClient);
		SAFE_RELEASE(_AudioClient);
		this->stoped.set_value_at_thread_exit(true);
		return true;
	});
	captureThread->detach();
}

bool AudioCapture::isRunning()
{
	return running;
}

void AudioCapture::stop() {
	if (!running) {
		return;
	}
	running = false;
	std::future<bool> future = stoped.get_future();
	future.wait();
}

HRESULT STDMETHODCALLTYPE AudioCapture::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
{
	return NULL;
}

HRESULT STDMETHODCALLTYPE AudioCapture::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
	return NULL;
}

HRESULT STDMETHODCALLTYPE AudioCapture::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
	return NULL;
}