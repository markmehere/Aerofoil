#pragma once

namespace PortabilityLayer
{
	class ClientAudioChannelContext;

	class HostAudioChannel
	{
	public:
		virtual void SetClientAudioChannelContext(ClientAudioChannelContext *context) = 0;
		virtual void PostBuffer(const void *buffer, size_t bufferSize) = 0;
		virtual void Stop() = 0;
		virtual void Destroy() = 0;
	};
}
