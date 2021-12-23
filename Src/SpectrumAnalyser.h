#ifndef SpectrumAnalyser_h__
#define SpectrumAnalyser_h__

#include <vector>
#include <fftw3.h>
#include <mmdeviceapi.h>
#include <functional>

struct SpectrumData {
	struct Channel {
		struct Element {
			Element()
				: frequency(0.0), amplitude(0.0)
			{ }
			double frequency;

			double amplitude;
		};
		std::vector<Element> specs;
	};
	std::vector<Channel> channels;
};

template<int N> struct PowerOfTwo {
	static const int Result = PowerOfTwo<N - 1>::Result * 2;
};

template<> struct PowerOfTwo<0> {
	static const int Result = 1;
};

const int SpectrumLengthSamples = PowerOfTwo<11>::Result;

enum class WindowFunction {
	NoWindow,
	GuassWindow,
	HannWindow,
	HammingWindow,
	BartlettWindow,
	TriangleWindow,
	BlackmanWindow,
	NuttallWindow,
	SinWindow
};

class SpectrumAnalyser {
public:
	SpectrumAnalyser();
	~SpectrumAnalyser();
	void updateFormat(WAVEFORMATEX format);
	void calculate(unsigned char* data, size_t size);
public:
	SpectrumData specData;
private:
	std::vector<double> window;
	double spectrumAnalyserMultiplier;
	double* input;
	double* output;
	fftw_plan plan;
	WAVEFORMATEX currentFormat;
	std::function<double(unsigned char*)> funcPcmToReal[4];
	int funcIndex = 0;
}; 

#endif // SpectrumAnalyser_h__
