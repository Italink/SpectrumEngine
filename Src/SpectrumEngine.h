#ifndef SpectrumEngine_h__
#define SpectrumEngine_h__

#include <AudioCapture.h>
#include "SpectrumAnalyser.h"
#include <mutex>

const int MAX_BUFFER_SIZE = 1000000;

class SpectrumProvider;
class RhythmProvider;

class SpectrumEngine {
	friend class SpectrumProvider;
	friend class RhythmProvider;
	friend class SpectrumAnalyser;
	friend class AudioCapture;
private:
	SpectrumEngine() {};
public:
	static SpectrumEngine* getInstance();
	std::vector<std::shared_ptr<AudioDevice>> enumDevices(EDataFlow type = eRender);
	void start(std::shared_ptr<AudioDevice> device, int intervalMs = 30);
	bool isRunning();
	void stop();
	void submitAudioData(unsigned char* data, size_t size);
	void setSpectrumSampleLevel(const int level);
private:
	std::function<double(unsigned char*)> funcPcmToReal();
	WAVEFORMATEX getCurrentWaveFormat();
	void restart();
private:
	void onWaveFormatChanged(WAVEFORMATEX format);
	void onCalculateSpectrum();
private:
	AudioCapture capture;
	SpectrumAnalyser analyser;
	std::shared_ptr<AudioDevice> currentDevice;
	unsigned char buffer[MAX_BUFFER_SIZE];
	int64_t bufferSize;
	int64_t submitSize;
	int spectrumSampleLevel = 12;
	std::atomic<bool> analyzing = false;
	std::promise<bool> stoped;
	std::thread analyserThread;
	std::list<SpectrumProvider*> spectrumProviders;
	std::list<RhythmProvider*> rhythmProviders;
	std::condition_variable cv;
};

#endif // SpectrumEngine_h__