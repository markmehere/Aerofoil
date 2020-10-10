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
		virtual int64_t GetTime() const = 0;
		virtual void GetLocalDateTime(unsigned int &year, unsigned int &month, unsigned int &day, unsigned int &hour, unsigned int &minute, unsigned int &second) const = 0;
		virtual HostMutex *CreateMutex() = 0;
		virtual HostMutex *CreateRecursiveMutex() = 0;
		virtual HostThreadEvent *CreateThreadEvent(bool autoReset, bool startSignaled) = 0;
		virtual uint64_t GetFreeMemoryCosmetic() const = 0;	// Returns free memory in bytes, does not have to be accurate
		virtual void Beep() const = 0;

		static void SetInstance(HostSystemServices *instance);
		static HostSystemServices *GetInstance();

	private:
		static HostSystemServices *ms_instance;
	};
}

#endif
