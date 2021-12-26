#include "SpectrumAnalyser.h"
#include <math.h>
#include <algorithm>
#include "Utils.h"
#include "SpectrumEngine.h"

SpectrumAnalyser::SpectrumAnalyser()
	: spectrumAnalyserMultiplier(0.15)
{
	
}

SpectrumAnalyser::~SpectrumAnalyser()
{
}

void SpectrumAnalyser::updateFormat(WAVEFORMATEX format,int SpectrumSample){
	std::lock_guard<std::mutex> locker(mmutex);
	currentFormat = format;
	specData.channels.resize(format.nChannels);
	window = (createWindowFunction(SpectrumSample, WindowFunction::HannWindow));
	if (input != nullptr || output != nullptr) {
		fftw_free(input);
		fftw_free(output);
	}
	input = (double*)fftw_malloc(sizeof(double) * SpectrumSample);
	output = (double*)fftw_malloc(sizeof(double) * SpectrumSample);
	plan = fftw_plan_r2r_1d(SpectrumSample, input, output, fftw_r2r_kind::FFTW_R2HC, FFTW_ESTIMATE);

	for (auto& channel : specData.channels) {
		channel.specs.resize(SpectrumSample / 2);
		channel.controlPoint.resize(channel.specs.size() + 1);
		channel.nSamplesPerSec = format.nSamplesPerSec;
		for (int i = 0; i < channel.specs.size(); i++) {
			channel.specs[i].frequency = (i * format.nSamplesPerSec) / SpectrumSample * 2;
			channel.specs[i].amplitude = 0;
			channel.specs[i].speed = 0.001;
		}
	}
}

void SpectrumAnalyser::calculate(unsigned char* data, size_t size)
{
	const int bytesPerSample = currentFormat.wBitsPerSample / 8 * currentFormat.nChannels;
	auto func = SpectrumEngine::getInstance()->funcPcmToReal();
	for (int i = 0; i < currentFormat.nChannels; i++) {
		unsigned char* offset = data + i * currentFormat.wBitsPerSample / 8;
		for (int j = 0; j < window.size(); ++j) {
			const double realSample = func(offset);
			const double windowedSample = realSample * window[j];
			input[j] = windowedSample;
			offset += bytesPerSample;
		}
		fftw_execute(plan);
		for (int j = 0; j < specData.channels[i].specs.size() ; j++) {
			SpectrumData::Channel::Element& elem = specData.channels[i].specs[j];
			const double real = output[j];
			const double imag = output[window.size() - 1 - (window.size() / 2 + j)];
			const double magnitude = sqrt(real * real + imag * imag);
			double amplitude = spectrumAnalyserMultiplier * log(magnitude);
			if (amplitude > elem.amplitude) {
				elem.speed = 0.01;
			}
			else {
				amplitude = elem.amplitude - elem.speed;
				elem.speed += 0.009;
			}
			elem.amplitude = std::clamp(amplitude, 0.0, 1.0);
		}
		specData.channels[i].controlPoint[0] = specData.channels[i].specs[0].amplitude;
		specData.channels[i].controlPoint[specData.channels[i].specs.size()] = specData.channels[i].specs[specData.channels[i].specs.size() - 1].amplitude;
		for (int j = 1; j < specData.channels[i].specs.size(); j++) {
			specData.channels[i].controlPoint[j] = (specData.channels[i].specs[j - 1].amplitude + specData.channels[i].specs[j].amplitude) / 2;
		}
	}
}