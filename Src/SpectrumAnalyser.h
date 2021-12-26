#ifndef SpectrumAnalyser_h__
#define SpectrumAnalyser_h__

#include <vector>
#include <fftw3.h>
#include <mmdeviceapi.h>
#include <functional>
#include <mutex>
#include <algorithm>

struct SpectrumData {
	struct Channel {
		struct Element {
			Element()
				: frequency(0.0), amplitude(0.0), speed(0.001)
			{ }
			double frequency;
			double amplitude;
			double speed;
		};
		std::vector<Element> specs;
		std::vector<double> controlPoint;
		int nSamplesPerSec;
		double getAmplitude(double frequency) {
			double offset = (specs.size() - 1) * frequency / nSamplesPerSec;
			int index = std::clamp((int)std::round(offset), 0, int(specs.size() - 1));
			const double& p0 = controlPoint[index];
			const double& p1 = specs[index].amplitude;
			const double& p2 = specs[index + 1].amplitude;
			double t = offset - index + 0.5;
			return (1 - t) * (1 - t) * p0 + 2 * t * (1 - t) * p1 + t * t * p2;
		}
	};
	std::vector<Channel> channels;
};

class SpectrumAnalyser {
public:
	SpectrumAnalyser();
	~SpectrumAnalyser();
	void updateFormat(WAVEFORMATEX format, int SpectrumSample);
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
	std::mutex mmutex;
};

#endif // SpectrumAnalyser_h__
