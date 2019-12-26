#pragma once

#include "HostAudioChannel.h"
#include "IGpAudioChannelCallbacks.h"

struct IGpAudioChannel;

class GpPLGlueAudioChannel final : public PortabilityLayer::HostAudioChannel, public IGpAudioChannelCallbacks
{
public:
	void SetClientAudioChannelContext(PortabilityLayer::ClientAudioChannelContext *context) override;
	void PostBuffer(const void *buffer, size_t bufferSize) override;
	void Stop() override;
	void Destroy() override;

	void NotifyBufferFinished() override;

	static GpPLGlueAudioChannel *Create(IGpAudioChannel *audioChannel);

private:
	explicit GpPLGlueAudioChannel(IGpAudioChannel *audioChannel);
	~GpPLGlueAudioChannel();

	PortabilityLayer::ClientAudioChannelContext *m_clientContext;
	IGpAudioChannel *m_audioChannel;
};
