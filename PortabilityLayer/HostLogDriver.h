#pragma once

struct IGpLogDriver;

namespace PortabilityLayer
{
	class HostLogDriver
	{
	public:
		static void SetInstance(IGpLogDriver *instance);
		static IGpLogDriver *GetInstance();

	private:
		static IGpLogDriver *ms_instance;
	};
}
