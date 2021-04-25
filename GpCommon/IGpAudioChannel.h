#pragma once

#include <stddef.h>

struct IGpAudioChannelCallbacks;
struct IGpAudioBuffer;

struct IGpAudioChannel
{
	virtual void SetAudioChannelContext(IGpAudioChannelCallbacks *callbacks) = 0;
	virtual bool PostBuffer(IGpAudioBuffer *buffer) = 0;
	virtual void Stop() = 0;
	virtual void Destroy() = 0;
};
