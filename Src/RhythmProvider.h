#ifndef RhythmProvider_h__
#define RhythmProvider_h__

#include <vector>

class RhythmProvider {
	friend class SpectrumEngine;
public:
	enum class Type {
		Rms,
		Peak,
		Pulse
	};
	RhythmProvider(double timeMs, RhythmProvider::Type type);
	~RhythmProvider();
private:
	void update();
private:
	unsigned int channelIndex = 0;
	double rhythm;
	int64_t timeMs;
	Type type;
};

#endif // RhythmProvider_h__
