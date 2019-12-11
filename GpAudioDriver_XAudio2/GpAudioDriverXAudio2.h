#pragma once

#include "IGpAudioDriver.h"
#include "GpCoreDefs.h"
#include "GpAudioDriverProperties.h"

struct IXAudio2;
struct IXAudio2MasteringVoice;

class GpAudioDriverXAudio2 : public IGpAudioDriver
{
public:
	IGpAudioChannel *CreateChannel() override;
	void Shutdown() override;

	const GpAudioDriverProperties &GetProperties() const;
	IXAudio2 *GetXA2() const;
	IXAudio2MasteringVoice *GetMasteringVoice() const;
	unsigned int GetRealSampleRate() const;

	static GpAudioDriverXAudio2 *Create(const GpAudioDriverProperties &properties);

private:
	GpAudioDriverXAudio2(const GpAudioDriverProperties &properties, unsigned int realSampleRate, IXAudio2* xa2, IXAudio2MasteringVoice *mv);
	~GpAudioDriverXAudio2();

	GpAudioDriverProperties m_properties;

	IXAudio2* m_xa2;
	IXAudio2MasteringVoice *m_mv;
	unsigned int m_realSampleRate;
};
