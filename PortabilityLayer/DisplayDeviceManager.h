#pragma once
#ifndef __PL_DEVICE_MANAGER_H__
#define __PL_DEVICE_MANAGER_H__

#include <stdint.h>

struct GDevice;

namespace PortabilityLayer
{
	class DisplayDeviceManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual GDevice **GetMainDevice() = 0;
		virtual void IncrementTickCount(uint32_t count) = 0;
		virtual uint32_t GetTickCount() = 0;

		static DisplayDeviceManager *GetInstance();
	};
}

#endif
