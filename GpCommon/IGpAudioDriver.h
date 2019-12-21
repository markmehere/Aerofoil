#pragma once

struct IGpAudioChannel;

struct IGpAudioDriver
{
public:
	virtual IGpAudioChannel *CreateChannel() = 0;

	virtual void Shutdown() = 0;
};
