#pragma once
#ifndef __PL_HOST_DISPLAY_DRIVER_H__
#define __PL_HOST_DISPLAY_DRIVER_H__

#include "GpPixelFormat.h"
#include "EGpStandardCursor.h"

struct IGpColorCursor;
struct IGpDisplayDriver;

namespace PortabilityLayer
{
	class HostDisplayDriver
	{
	public:
		static void SetInstance(IGpDisplayDriver *instance);
		static IGpDisplayDriver *GetInstance();

	private:
		static IGpDisplayDriver *ms_instance;
	};
}

#endif
