#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	class ClientAudioChannelContext
	{
	public:
		virtual void NotifyBufferFinished() = 0;
	};
}
