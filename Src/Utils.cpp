#include "Utils.h"

#define M_PI 3.1415926535

const unsigned int PCMS8MaxAmplitude = 128;
const unsigned short PCMS16MaxAmplitude = 32768; // because minimum is -32768
const unsigned int PCMS24MaxAmplitude = 8388608;
const unsigned int PCMS32MaxAmplitude = 2147483648;

struct int24 {
	unsigned int data : 24;
};

std::function<double(unsigned char*)> getFuncPcmToReal(int bits)
{
	static std::function<double(unsigned char*)> funcPcmToReal[4] = {
	 [](unsigned char* data) {
		return (*reinterpret_cast<const char*>(data)) / double(PCMS8MaxAmplitude);
	},
	[](unsigned char* data) {
		return (*reinterpret_cast<const int16_t*>(data)) / double(PCMS16MaxAmplitude);
	},
	[](unsigned char* data) {
		return (*reinterpret_cast<const int24*>(data)).data / double(PCMS24MaxAmplitude);
	},
	[](unsigned char* data) {
		return (*reinterpret_cast<const int32_t*>(data)) / double(PCMS32MaxAmplitude);
	}
	};
	return funcPcmToReal[bits / 8];
}

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