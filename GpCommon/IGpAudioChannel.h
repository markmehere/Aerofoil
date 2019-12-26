#pragma once

struct IGpAudioChannelCallbacks;

struct IGpAudioChannel
{
	virtual void SetAudioChannelContext(IGpAudioChannelCallbacks *callbacks) = 0;
	virtual void PostBuffer(const void *buffer, size_t bufferSize) = 0;
	virtual void Stop() = 0;
	virtual void Destroy() = 0;
};
