#pragma once

#include "PLCore.h"

struct IGpAudioBuffer;

namespace PortabilityLayer
{
	struct AudioChannel;

	typedef void (*AudioChannelCallback_t)(PortabilityLayer::AudioChannel *channel);

	struct AudioChannel
	{
		virtual void Destroy(bool wait) = 0;
		virtual bool AddBuffer(IGpAudioBuffer *buffer, bool blocking) = 0;
		virtual bool AddCallback(AudioChannelCallback_t callback, bool blocking) = 0;
		virtual void ClearAllCommands() = 0;
		virtual void Stop() = 0;
	};

	class SoundSystem
	{
	public:
		virtual AudioChannel *CreateChannel() = 0;

		virtual void SetVolume(uint8_t vol) = 0;
		virtual uint8_t GetVolume() const = 0;

		static SoundSystem *GetInstance();
	};
}
