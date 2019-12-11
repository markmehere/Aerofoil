#include "GpPLGlueAudioChannel.h"
#include "ClientAudioChannelContext.h"
#include "IGpAudioChannel.h"

#include <stdlib.h>
#include <new>

void GpPLGlueAudioChannel::SetClientAudioChannelContext(PortabilityLayer::ClientAudioChannelContext *context)
{
	m_clientContext = context;
	m_audioChannel->SetAudioChannelContext(this);
}

void GpPLGlueAudioChannel::PostBuffer(const void *buffer, size_t bufferSize)
{
	m_audioChannel->PostBuffer(buffer, bufferSize);
}

void GpPLGlueAudioChannel::Destroy()
{
	this->~GpPLGlueAudioChannel();
	free(this);
}

void GpPLGlueAudioChannel::NotifyBufferFinished()
{
	if (m_clientContext)
		m_clientContext->NotifyBufferFinished();
}

GpPLGlueAudioChannel *GpPLGlueAudioChannel::Create(IGpAudioChannel *audioChannel)
{
	void *storage = malloc(sizeof(GpPLGlueAudioChannel));
	if (!storage)
		return nullptr;

	return new (storage) GpPLGlueAudioChannel(audioChannel);
}

GpPLGlueAudioChannel::GpPLGlueAudioChannel(IGpAudioChannel *audioChannel)
	: m_audioChannel(audioChannel)
	, m_clientContext(nullptr)
{
}

GpPLGlueAudioChannel::~GpPLGlueAudioChannel()
{
	m_audioChannel->Destroy();
}
