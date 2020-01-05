#pragma once

#include <stdint.h>

struct IGpAudioChannel;

struct IGpAudioDriver
{
public:
	virtual IGpAudioChannel *CreateChannel() = 0;

	virtual void SetMasterVolume(uint32_t vol, uint32_t maxVolume) = 0;

	virtual void Shutdown() = 0;
};
