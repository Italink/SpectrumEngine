#include "SpectrumProvider.h"
#include "SpectrumEngine.h"

SpectrumProvider::SpectrumProvider(int low, int high, int barsCount)
	:lowFreq(low)
	, highFreq(high)
	, bars(barsCount, 0)
	, smoothBuffer(barsCount, 0)
	, smoothRange(2)
	, smoothFactor(0.7)
	, filterFactor(0)
{
	SpectrumEngine::getInstance()->spectrumProviders.push_back(this);
}

SpectrumProvider::~SpectrumProvider()
{
	SpectrumEngine::getInstance()->spectrumProviders.remove(this);
}

int SpectrumProvider::getLowFreq() const
{
	return lowFreq;
}

void SpectrumProvider::setLowFreq(int value)
{
	lowFreq = value;
}

int SpectrumProvider::getHighFreq() const
{
	return highFreq;
}

void SpectrumProvider::setHighFreq(int value)
{
	highFreq = value;
}

void SpectrumProvider::setBarCount(int count)
{
	bars.assign(count, 0);
	smoothBuffer.assign(count, 0);
}

int SpectrumProvider::getBarCount() const
{
	return bars.size();
}

void SpectrumProvider::smoothing()
{
	if (smoothRange) {
		std::list<double> window;
		double sum = 0;
		for (int i = 0; i < smoothRange * 2 + 1; i++) {
			window.push_back(bars[i]);
			sum += bars[i];
		}
		int rWidth = smoothRange;
		int i = rWidth;
		while (i < getBarCount() - rWidth) {
			bars[i] = sum / window.size();
			window.push_back(bars[i + rWidth]);
			sum = sum - window.front() + window.back();
			window.pop_front();
			i++;
		}
	}
	std::vector<double> sorted = bars;
	std::sort(sorted.begin(), sorted.end());
	double level = sorted.empty() ? 0 : sorted[(sorted.size() - 1) * filterFactor];
	for (int i = 0; i < bars.size(); i++) {
		bars[i] = max(0.0, (bars[i] - level) / (1.0 - level));
		if (bars[i] < smoothBuffer[i]) {
			bars[i] = smoothBuffer[i] - (smoothBuffer[i] - bars[i]) * smoothFactor;
		}
		smoothBuffer[i] = bars[i];
	}
}

int SpectrumProvider::getSmoothRange() const
{
	return smoothRange;
}

void SpectrumProvider::setSmoothRange(int value)
{
	smoothRange = value;
}

double SpectrumProvider::getSmoothFactor() const
{
	return smoothFactor;
}

void SpectrumProvider::setSmoothFactor(double value)
{
	smoothFactor = value;
}

double SpectrumProvider::getFilterFactor() const
{
	return filterFactor;
}

void SpectrumProvider::setFilterFactor(double value)
{
	filterFactor = value;
}

std::vector<double> SpectrumProvider::getBars() const
{
	return bars;
}

unsigned int SpectrumProvider::getChannelIndex() const
{
	return channelIndex;
}

void SpectrumProvider::setChannelIndex(unsigned int val)
{
	channelIndex = val;
}