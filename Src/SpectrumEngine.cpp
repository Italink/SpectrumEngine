#include "SpectrumEngine.h"
#include <corecrt_memcpy_s.h>
#include "SpectrumProvider.h"

SpectrumEngine* SpectrumEngine::getInstance()
{
	static SpectrumEngine instance;
	return &instance;
}

std::vector<std::shared_ptr<AudioDevice>> SpectrumEngine::enumDevices(EDataFlow type /*= eRender*/)
{
	return capture.enumDevices(type);
}

void SpectrumEngine::start(std::shared_ptr<AudioDevice> device, int interval)
{
	currentDevice = device;
	onWaveFormatChanged(device->format);
	capture.start(device);
	analyzing = true;
	analyserThread = std::make_shared<std::thread>([interval, this]() {
		while (analyzing) {
			onCalculateSpectrum();
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
		}
	});
}

bool SpectrumEngine::isRunning()
{
	return capture.isRunning();
}

void SpectrumEngine::stop()
{
	capture.stop();
	analyzing = false;
	analyserThread.reset();
}

void SpectrumEngine::submitAudioData(unsigned char* data, size_t size)
{
	if (bufferSize + size >= MAX_BUFFER_SIZE) {
		memcpy_s(buffer, spectrumSize, buffer + bufferSize - spectrumSize, spectrumSize);
		bufferSize = spectrumSize;
	}
	memcpy_s(buffer + bufferSize, size, data, size);
	bufferSize += size;
}

int SpectrumEngine::barIndex(const double& frequency)
{
	return 	std::clamp((int)std::round(SpectrumLengthSamples * frequency / currentDevice->format.nSamplesPerSec)/2, 0, SpectrumLengthSamples);
}

void SpectrumEngine::onWaveFormatChanged(WAVEFORMATEX format)
{
	spectrumSize = SpectrumLengthSamples * (format.wBitsPerSample / 8) * format.nChannels;
	analyser.updateFormat(format);
}

void SpectrumEngine::onCalculateSpectrum()
{
	if (bufferSize > spectrumSize) {
		analyser.calculate(buffer + bufferSize - spectrumSize, spectrumSize);
		for (auto& spec : specProviders) {
			float begin = spec->lowFreq;
			float offset = (spec->highFreq - begin) / (spec->getBarCount() - 1);
			const auto& channel = analyser.specData.channels[spec->getChannelIndex() % analyser.specData.channels.size()];
			for (auto& it : spec->bars) {
				it = channel.specs[barIndex(begin)].amplitude;
				begin += offset;
			}
			spec->smoothing();
		}
	}
}