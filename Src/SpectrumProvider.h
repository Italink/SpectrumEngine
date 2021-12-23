#ifndef SpectrumProvider_h__
#define SpectrumProvider_h__

#include <vector>

class SpectrumProvider {
	friend class SpectrumEngine;
public:
	explicit SpectrumProvider(int low, int high, int barsCount);
	~SpectrumProvider();

	std::vector<double> getBars() const;

	int getLowFreq() const;
	void setLowFreq(int value);

	int getHighFreq() const;
	void setHighFreq(int value);

	void setBarCount(int count);
	int getBarCount() const;

	int getSmoothRange() const;
	void setSmoothRange(int value);

	double getSmoothFactor() const;
	void setSmoothFactor(double value);

	double getFilterFactor() const;
	void setFilterFactor(double value);

	unsigned int getChannelIndex() const;
	void setChannelIndex(unsigned int val);

private:
	void smoothing();
	friend class SpecEngine;
private:
	int lowFreq;
	int highFreq;
	unsigned int channelIndex = 0;

	std::vector<double> bars;
	std::vector<double> smoothBuffer;
	int smoothRange;
	double smoothFactor;
	double filterFactor;
};

#endif // SpectrumProvider_h__
