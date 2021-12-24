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
};

#endif // SpectrumAnalyser_h__
