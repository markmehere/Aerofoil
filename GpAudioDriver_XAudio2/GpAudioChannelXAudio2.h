#pragma once

#include "IGpAudioChannel.h"

#include "GpAudioChannelXAudio2Callbacks.h"

class GpAudioDriverXAudio2;
class GpAudioChannelXAudio2Callbacks;
struct IXAudio2SourceVoice;
struct IGpAllocator;

class GpAudioChannelXAudio2 final : public IGpAudioChannel
{
public:
	friend class GpAudioChannelXAudio2Callbacks;

	static GpAudioChannelXAudio2 *Create(IGpAllocator *alloc, GpAudioDriverXAudio2 *driver);

	void SetAudioChannelContext(IGpAudioChannelCallbacks *callbacks) override;
	bool PostBuffer(IGpAudioBuffer *buffer) override;
	void Stop() override;
	void Destroy() override;

	bool Init();

protected:
	void OnBufferEnd();

private:
	enum VoiceState
	{
		VoiceState_Idle,
		VoiceState_Active,
	};

	explicit GpAudioChannelXAudio2(IGpAllocator *alloc, GpAudioDriverXAudio2 *driver);
	~GpAudioChannelXAudio2();

	GpAudioDriverXAudio2 *m_driver;
	IXAudio2SourceVoice *m_sourceVoice;
	GpAudioChannelXAudio2Callbacks m_xAudioCallbacks;
	IGpAudioChannelCallbacks *m_contextCallbacks;
	IGpAllocator *m_alloc;
	VoiceState m_voiceState;
};
