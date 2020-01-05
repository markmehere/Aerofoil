#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	class HostAudioChannel;

	class HostAudioDriver
	{
	public:
		virtual HostAudioChannel *CreateChannel() = 0;
		virtual void SetMasterVolume(uint32_t vol, uint32_t maxVolume) = 0;

		static HostAudioDriver *GetInstance();
		static void SetInstance(HostAudioDriver *instance);

	private:
		static HostAudioDriver *ms_instance;
	};
}
