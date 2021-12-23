#include "RhythmProvider.h"
#include "SpectrumEngine.h"

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

void RhythmProvider::submitAudioData(std::vector<float> data)
{
}