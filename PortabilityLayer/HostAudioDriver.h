#pragma once

struct IGpAudioDriver;

namespace PortabilityLayer
{
	class HostAudioDriver
	{
	public:
		static IGpAudioDriver *GetInstance();
		static void SetInstance(IGpAudioDriver *instance);

	private:
		static IGpAudioDriver *ms_instance;
	};
}
