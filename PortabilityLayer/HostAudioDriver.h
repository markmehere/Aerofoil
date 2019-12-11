#pragma once

namespace PortabilityLayer
{
	class HostAudioChannel;

	class HostAudioDriver
	{
	public:
		virtual HostAudioChannel *CreateChannel() = 0;

		static HostAudioDriver *GetInstance();
		static void SetInstance(HostAudioDriver *instance);

	private:
		static HostAudioDriver *ms_instance;
	};
}
