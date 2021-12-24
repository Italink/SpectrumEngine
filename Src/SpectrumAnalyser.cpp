#include "SpectrumAnalyser.h"
#include <math.h>
#include <algorithm>
#include "Utils.h"
#include "SpectrumEngine.h"

SpectrumAnalyser::SpectrumAnalyser()
	: spectrumAnalyserMultiplier(0.15)
	, window(createWindowFunction(SpectrumLengthSamples, WindowFunction::HannWindow))
{
	input = (double*)fftw_malloc(sizeof(double) * SpectrumLengthSamples);
	output = (double*)fftw_malloc(sizeof(double) * SpectrumLengthSamples);
	plan = fftw_plan_r2r_1d(SpectrumLengthSamples, input, output, fftw_r2r_kind::FFTW_R2HC, FFTW_ESTIMATE);
}

SpectrumAnalyser::~SpectrumAnalyser()
{
	fftw_free(input);
	fftw_free(output);
}

void SpectrumAnalyser::updateFormat(WAVEFORMATEX format)
{
	currentFormat = format;

	specData.channels.resize(format.nChannels);
	for (auto& channel : specData.channels) {
		channel.specs.resize(SpectrumLengthSamples / 2);
		for (int i = 0; i < channel.specs.size(); i++) {
			channel.specs[i].frequency = (i * format.nSamplesPerSec) / SpectrumLengthSamples * 2;
		}
	}
}

void SpectrumAnalyser::calculate(unsigned char* data, size_t size)
{
	const int bytesPerSample = currentFormat.wBitsPerSample / 8 * currentFormat.nChannels;
	auto func = SpectrumEngine::getInstance()->funcPcmToReal();
	for (int i = 0; i < currentFormat.nChannels; i++) {
		unsigned char* offset = data + i * currentFormat.wBitsPerSample / 8;
		for (int j = 0; j < SpectrumLengthSamples; ++j) {
			const double realSample = func(offset);
			const double windowedSample = realSample * window[j];
			input[j] = windowedSample;
			offset += bytesPerSample;
		}
		fftw_execute(plan);
		for (int j = 0; j < SpectrumLengthSamples / 2; j++) {
			const double real = output[j];
			const double imag = output[SpectrumLengthSamples - 1 - (SpectrumLengthSamples / 2 + j)];
			const double magnitude = sqrt(real * real + imag * imag);
			double amplitude = spectrumAnalyserMultiplier * log(magnitude);
			specData.channels[i].specs[j].amplitude = std::clamp(amplitude, 0.0, 1.0);
		}
	}
}