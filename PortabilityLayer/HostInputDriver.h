#pragma once

#include "HostKeyID.h"

namespace PortabilityLayer
{
	class HostInputDriver
	{
	public:

		static HostInputDriver *GetInstance();
		static void SetInstance(HostInputDriver *instance);

	private:
		static HostInputDriver *ms_instance;
	};
}
