#ifndef SpectrumEngine_h__
#define SpectrumEngine_h__

#include <AudioCapture.h>
#include "SpectrumAnalyser.h"

const int MAX_BUFFER_SIZE = 1000000;

class SpectrumProvider;
class RhythmProvider;

class SpectrumEngine {
	friend class SpectrumProvider;
	friend class RhythmProvider;
private:
	SpectrumEngine() {};
public:
	static SpectrumEngine* getInstance();
	std::vector<std::shared_ptr<AudioDevice>> enumDevices(EDataFlow type = eRender);
	void start(std::shared_ptr<AudioDevice> device, int intervalMs = 30);
	bool isRunning();
	void stop();
	void submitAudioData(unsigned char* data, size_t size);
private:
	int barIndex(const double& frequency);
	void onWaveFormatChanged(WAVEFORMATEX format);
	void onCalculateSpectrum();
private:
	AudioCapture capture;
	SpectrumAnalyser analyser;
	std::shared_ptr<AudioDevice> currentDevice;
	unsigned char buffer[MAX_BUFFER_SIZE];
	int bufferSize;
	int spectrumSize;
	std::atomic<bool> analyzing = false;
	std::shared_ptr<std::thread> analyserThread;
	std::list<SpectrumProvider*> specProviders;
	std::list<RhythmProvider*> rhythmProviders;
};

#endif // SpectrumEngine_h__