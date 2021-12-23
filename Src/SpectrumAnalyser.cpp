#include "SpectrumAnalyser.h"
#include <math.h>
#include <algorithm>

#define M_PI 3.1415926535

const unsigned int PCMS8MaxAmplitude = 128;
const unsigned short PCMS16MaxAmplitude = 32768; // because minimum is -32768
const unsigned int PCMS24MaxAmplitude = 8388608;
const unsigned int PCMS32MaxAmplitude = 2147483648;

struct int24 {
	unsigned int data : 24;
};

std::vector<double> createWindowFunction(int size, WindowFunction funcType)
{
	std::vector<double> window(size);
	for (int n = 0; n < size; ++n) {
		float x = 0.0;
		switch (funcType) {
		case WindowFunction::NoWindow:
		x = 1.0;
		break;
		case WindowFunction::GuassWindow: {
			float tmp = (n - (size - 1) / 2.0) / (0.4 * (size - 1) / 2);
			x = exp(-0.5 * (tmp * tmp));
			break;
		}
		case WindowFunction::HannWindow:
		x = 0.5 * (1 - cos((2 * M_PI * n) / (size - 1)));
		break;
		case WindowFunction::HammingWindow:
		x = 0.53836 - 0.46164 * cos(2 * M_PI * n / (size - 1));
		break;
		case WindowFunction::BartlettWindow:
		x = 2.0 / (size - 1) * ((size - 1) / 2.0 - abs(n - (size - 1) / 2.0));
		break;
		case WindowFunction::TriangleWindow:
		x = 2.0 / size * (size / 2.0 - abs(n - (size - 1) / 2.0));
		break;
		case WindowFunction::BlackmanWindow:
		x = 0.42 - 0.5 * cos(2 * M_PI * n / (size - 1)) + 0.08 * cos(4 * M_PI * n / (size - 1));
		break;
		case WindowFunction::NuttallWindow:
		x = 0.355768 - 0.487396 * cos(2 * M_PI * n / (size - 1)) + 0.1444232 * cos(4 * M_PI * n / (size - 1)) + 0.012604 * cos(6 * M_PI * n / (size - 1));
		break;
		case WindowFunction::SinWindow:
		x = sin(M_PI * n / (size - 1));
		break;
		}
		window[n] = x;
	}
	return window;
}

SpectrumAnalyser::SpectrumAnalyser()
	: spectrumAnalyserMultiplier(0.15)
	, window(createWindowFunction(SpectrumLengthSamples, WindowFunction::HannWindow))
{
	input = (double*)fftw_malloc(sizeof(double) * SpectrumLengthSamples);
	output = (double*)fftw_malloc(sizeof(double) * SpectrumLengthSamples);
	plan = fftw_plan_r2r_1d(SpectrumLengthSamples, input, output, fftw_r2r_kind::FFTW_R2HC, FFTW_ESTIMATE);

	funcPcmToReal[0] = [](unsigned char* data) {
		return (*reinterpret_cast<const char*>(data))/double(PCMS8MaxAmplitude);
	};
	funcPcmToReal[1] = [](unsigned char* data) {
		return (*reinterpret_cast<const int16_t*>(data)) / double(PCMS16MaxAmplitude);
	};
	funcPcmToReal[2] = [](unsigned char* data) {
		return (*reinterpret_cast<const int24*>(data)).data / double(PCMS24MaxAmplitude);
	};
	funcPcmToReal[3] = [](unsigned char* data) {
		return (*reinterpret_cast<const int32_t*>(data)) / double(PCMS32MaxAmplitude);
	};

}

SpectrumAnalyser::~SpectrumAnalyser()
{
	fftw_free(input);
	fftw_free(output);
}

void SpectrumAnalyser::updateFormat(WAVEFORMATEX format)
{
	currentFormat = format;
	funcIndex = std::clamp((format.wBitsPerSample / 8) - 1, 0, 3);
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
	for (int i = 0; i < currentFormat.nChannels; i++) {
		unsigned char* offset = data + i * currentFormat.wBitsPerSample / 8;
		for (int j = 0; j < SpectrumLengthSamples; ++j) {
			const double realSample = funcPcmToReal[funcIndex](offset);
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

