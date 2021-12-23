#ifndef AudioCapture_h__
#define AudioCapture_h__

#include <AudioClient.h>
#include <string>
#include <vector>
#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <setupapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <memory>
#include <thread>
#include <atomic>
#include <future>

struct AudioDevice {
private:
	AudioDevice() {}
	friend class AudioCapture;
public:
	IMMDevice* pDevice;
	std::wstring name;
	EDataFlow type;
	WAVEFORMATEX format;
	LPWSTR id;
	bool isDefault = false;
};

class AudioCapture :public IMMNotificationClient {
public:
	AudioCapture();
	std::vector<std::shared_ptr<AudioDevice>> enumDevices(EDataFlow type = eRender);
	void start(std::shared_ptr<AudioDevice> device);
	bool isRunning();
	void stop();
protected:
	ULONG STDMETHODCALLTYPE AddRef() { return 0; };
	ULONG STDMETHODCALLTYPE Release() { return 0; };
	HRESULT STDMETHODCALLTYPE QueryInterface(
			REFIID, VOID**) {
		return 0;
	}
	HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(
		EDataFlow flow, ERole role,
			LPCWSTR pwstrDeviceId);
	HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId);
	HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId);
	HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR, DWORD) { return 0; }

	HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY) { return 0; }
private:
	IMMDeviceEnumerator* pOutputEnumerator;
	std::shared_ptr<std::thread> captureThread;
	bool running;
	std::mutex lock;
	std::promise<bool> stoped;
};

#endif // AudioCapture_h__
