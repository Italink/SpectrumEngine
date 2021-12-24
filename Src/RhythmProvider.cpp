#include "RhythmProvider.h"
#include "SpectrumEngine.h"

int64_t audioLength(WAVEFORMATEX wav, int64_t microSeconds)
{
	int64_t result = (wav.nSamplesPerSec * wav.nChannels * (wav.wBitsPerSample / 8))
		* microSeconds / 1000000;
	result -= result % (wav.nChannels * wav.wBitsPerSample);
	return result;
}

RhythmProvider::RhythmProvider(double timeMs, RhythmProvider::Type type)
	:timeMs(timeMs)
	, type(type)
{
	SpectrumEngine::getInstance()->rhythmProviders.push_back(this);
}

RhythmProvider::~RhythmProvider()
{
	SpectrumEngine::getInstance()->rhythmProviders.remove(this);
}

void RhythmProvider::update()
{
	int64_t size = SpectrumEngine::getInstance()->bufferSize;
	unsigned char* buffer = SpectrumEngine::getInstance()->buffer;
	unsigned char* ptr = buffer + size - timeMs;
	const unsigned char* end = ptr + timeMs;
	auto func = SpectrumEngine::getInstance()->funcPcmToReal();
	int byteOffset = SpectrumEngine::getInstance()->getCurrentWaveFormat().wBitsPerSample / 8;

	if (type == Type::Rms) {
		rhythm = 0;
		while (ptr < end) {
			double var = func(ptr);
			rhythm += var * var;
			ptr += byteOffset;
		}
		rhythm = rhythm / (timeMs / byteOffset);
	}

	else if (type == Type::Peak) {
		rhythm = 0;
		while (ptr < end) {
			rhythm = max(rhythm, func(ptr));
			ptr += byteOffset;
		}
	}
}