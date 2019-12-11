#pragma once
#ifndef __PL_HOST_SYSTEM_SERVICES_H__
#define __PL_HOST_SYSTEM_SERVICES_H__

#include <stdint.h>

#ifdef CreateMutex
#error "CreateMutex was macrod"
#endif

namespace PortabilityLayer
{
	class HostMutex;
	class HostThreadEvent;

	class HostSystemServices
	{
	public:
		virtual uint32_t GetTime() const = 0;
		virtual HostMutex *CreateMutex() = 0;
		virtual HostThreadEvent *CreateThreadEvent(bool autoReset, bool startSignaled) = 0;

		static void SetInstance(HostSystemServices *instance);
		static HostSystemServices *GetInstance();

	private:
		static HostSystemServices *ms_instance;
	};
}

#endif
