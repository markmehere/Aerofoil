#pragma once

struct IGpAudioChannel;

class IGpAudioDriver
{
public:
	virtual ~IGpAudioDriver() {}

	virtual IGpAudioChannel *CreateChannel() = 0;

	virtual void Shutdown() = 0;
};
