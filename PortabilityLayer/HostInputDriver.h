#pragma once

struct IGpInputDriver;

namespace PortabilityLayer
{
	class HostInputDriver
	{
	public:
		static size_t NumInstances();
		static IGpInputDriver *GetInstance(size_t index);
		static void SetInstances(IGpInputDriver *const* instances, size_t numInstances);

	private:
		static IGpInputDriver *const* ms_instances;
		static size_t ms_numInstances;
	};
}
