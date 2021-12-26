#include "SpectrumEngine.h"
#include <corecrt_memcpy_s.h>
#include "SpectrumProvider.h"
#include "Utils.h"

SpectrumEngine* SpectrumEngine::getInstance() {
	static SpectrumEngine instance;
	return &instance;
}

std::vector<std::shared_ptr<AudioDevice>> SpectrumEngine::enumDevices(EDataFlow type /*= eRender*/) {
	return capture.enumDevices(type);
}

void SpectrumEngine::start(std::shared_ptr<AudioDevice> device, int interval) {
	currentDevice = device;
	onWaveFormatChanged(device->format);
	capture.start(device);
	analyzing = true;
	stoped = std::promise<bool>();
	analyserThread = std::thread([interval, this]() {
		do {
			onCalculateSpectrum();
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
		} while (analyzing);
		stoped.set_value_at_thread_exit(true);
	});
	analyserThread.detach();
}

bool SpectrumEngine::isRunning()
{
	return capture.isRunning();
}

void SpectrumEngine::stop() {
	capture.stop();
	analyzing = false;
	std::future<bool> future = stoped.get_future();
	future.wait();
}

void SpectrumEngine::submitAudioData(unsigned char* data, size_t size) {
	if (bufferSize + size >= MAX_BUFFER_SIZE) {
		memcpy_s(buffer, submitSize, buffer + bufferSize - submitSize, submitSize);
		bufferSize = submitSize;
	}
	memcpy_s(buffer + bufferSize, size, data, size);
	bufferSize += size;
}

void SpectrumEngine::setSpectrumSampleLevel(const int level)
{
	spectrumSampleLevel = level;
	onWaveFormatChanged(currentDevice->format);
}

std::function<double(unsigned char*)> SpectrumEngine::funcPcmToReal()
{
	return getFuncPcmToReal(getCurrentWaveFormat().wBitsPerSample);
}

WAVEFORMATEX SpectrumEngine::getCurrentWaveFormat()
{
	return currentDevice->format;
}

void SpectrumEngine::restart()
{
	if (isRunning()) {
		stop();
	}
	std::shared_ptr<AudioDevice> newDevice;
	std::vector<std::shared_ptr<AudioDevice>> deviceList = enumDevices(currentDevice->type);
	for (auto& device : deviceList) {
		if (currentDevice->name.size()==device->name.size() && std::equal(device->name.begin(), device->name.end(), currentDevice->name.begin()))
			newDevice = device;
	}
	if (!newDevice) {
		if (deviceList.empty()) {
			printf(" no devices");
			return;
		}
		newDevice = deviceList.front();
	}
	start(newDevice);
}



void SpectrumEngine::onWaveFormatChanged(WAVEFORMATEX format) {
	int64_t spectrumSample = (1 << spectrumSampleLevel);
	submitSize = spectrumSample * (format.wBitsPerSample / 8) * format.nChannels;
	analyser.updateFormat(format, spectrumSample);
}

void SpectrumEngine::onCalculateSpectrum() {
	if (bufferSize > submitSize) {
		analyser.calculate(buffer + bufferSize - submitSize, submitSize);
		
		for (auto& spec : spectrumProviders) {
			float begin = spec->lowFreq;
			float offset = (spec->highFreq - begin) / (spec->getBarCount() - 1);
			auto& channel = analyser.specData.channels[spec->getChannelIndex() % analyser.specData.channels.size()];
			for (auto& it : spec->bars) {
				it = channel.getAmplitude(begin);
				begin += offset;
			}
			spec->smoothing();
		}
	}
}