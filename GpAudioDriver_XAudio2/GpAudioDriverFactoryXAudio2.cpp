#include "GpAudioDriverFactoryXAudio2.h"
#include "GpAudioDriverXAudio2.h"

IGpAudioDriver *GpAudioDriverFactoryXAudio2::Create(const GpAudioDriverProperties &properties)
{
	return GpAudioDriverXAudio2::Create(properties);
}


extern "C" __declspec(dllexport) IGpAudioDriver *GpDriver_CreateAudioDriver_XAudio2(const GpAudioDriverProperties &properties)
{
	return GpAudioDriverFactoryXAudio2::Create(properties);
}
