#pragma once

struct IGpAudioChannelCallbacks
{
	virtual void NotifyBufferFinished() = 0;
};
