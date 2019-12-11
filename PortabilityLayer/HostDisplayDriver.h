#pragma once
#ifndef __PL_HOST_DISPLAY_DRIVER_H__
#define __PL_HOST_DISPLAY_DRIVER_H__

#include "PixelFormat.h"

namespace PortabilityLayer
{
	class HostDisplayDriver
	{
	public:
		virtual void GetDisplayResolution(unsigned int *width, unsigned int *height, PixelFormat *pixelFormat) = 0;
		virtual void HideCursor() = 0;

		static void SetInstance(HostDisplayDriver *instance);
		static HostDisplayDriver *GetInstance();

	private:
		static HostDisplayDriver *ms_instance;
	};
}

#endif
