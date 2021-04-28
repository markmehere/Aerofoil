#pragma once

#include <stdint.h>

struct IGpAudioChannel;
struct IGpPrefsHandler;
struct IGpAudioBuffer;

struct IGpAudioDriver
{
public:
	virtual IGpAudioBuffer *CreateBuffer(const void *buffer, size_t bufferSize) = 0;
	virtual IGpAudioChannel *CreateChannel() = 0;

	virtual void SetMasterVolume(uint32_t vol, uint32_t maxVolume) = 0;

	virtual void Shutdown() = 0;

	virtual IGpPrefsHandler *GetPrefsHandler() const = 0;
};
