#include "GpAudioChannelXAudio2Callbacks.h"
#include "GpAudioChannelXAudio2.h"

GpAudioChannelXAudio2Callbacks::GpAudioChannelXAudio2Callbacks(GpAudioChannelXAudio2 *owner)
	: m_owner(owner)
{
}

void GpAudioChannelXAudio2Callbacks::OnVoiceProcessingPassStart(UINT32 BytesRequired)
{
}

void GpAudioChannelXAudio2Callbacks::OnVoiceProcessingPassEnd()
{
}

void GpAudioChannelXAudio2Callbacks::OnStreamEnd()
{
}

void GpAudioChannelXAudio2Callbacks::OnBufferStart(void* pBufferContext)
{
}

void GpAudioChannelXAudio2Callbacks::OnBufferEnd(void* pBufferContext)
{
	m_owner->OnBufferEnd();
}

void GpAudioChannelXAudio2Callbacks::OnLoopEnd(void* pBufferContext)
{
}

void GpAudioChannelXAudio2Callbacks::OnVoiceError(void* pBufferContext, HRESULT Error)
{
}
