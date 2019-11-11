#pragma once
#ifndef __PL_HOST_SYSTEM_SERVICES_H__
#define __PL_HOST_SYSTEM_SERVICES_H__

#include <stdint.h>

namespace PortabilityLayer
{
	class HostSystemServices
	{
	public:
		virtual uint32_t GetTime() const = 0;

		static void SetInstance(HostSystemServices *instance);
		static HostSystemServices *GetInstance();

	private:
		static HostSystemServices *ms_instance;
	};
}

#endif
