#pragma once

#include <xaudio2.h>

class GpAudioChannelXAudio2;

class GpAudioChannelXAudio2Callbacks final : public IXAudio2VoiceCallback
{
public:
	explicit GpAudioChannelXAudio2Callbacks(GpAudioChannelXAudio2 *owner);

	void OnVoiceProcessingPassStart(UINT32 BytesRequired) override;
	void OnVoiceProcessingPassEnd() override;
	void OnStreamEnd() override;
	void OnBufferStart(void* pBufferContext) override;
	void OnBufferEnd(void* pBufferContext) override;
	void OnLoopEnd(void* pBufferContext) override;
	void OnVoiceError(void* pBufferContext, HRESULT Error) override;

private:
	GpAudioChannelXAudio2 *m_owner;
};
