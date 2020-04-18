#pragma once
#ifndef __PL_DEVICE_MANAGER_H__
#define __PL_DEVICE_MANAGER_H__

#include <stdint.h>
#include "GpPixelFormat.h"

struct IGpDisplayDriver;

namespace PortabilityLayer
{
	class DisplayDeviceManager
	{
	public:
		struct IResolutionChangeHandler
		{
			virtual void OnResolutionChanged(uint32_t prevWidth, uint32_t prevHeight, uint32_t newWidth, uint32_t newHeight) = 0;
			virtual void AdjustRequestedResolution(uint32_t &physicalWidth, uint32_t &physicalHeight, uint32_t &virtualWidth, uint32_t &virtualheight, float &pixelScaleX, float &pixelScaleY) = 0;
		};

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual GpPixelFormat_t GetPixelFormat() const = 0;
		virtual void SyncPalette(IGpDisplayDriver *displayDriver) = 0;

		virtual void IncrementTickCount(uint32_t count) = 0;
		virtual uint32_t GetTickCount() = 0;

		virtual void SetResolutionChangeHandler(IResolutionChangeHandler *handler) = 0;
		virtual IResolutionChangeHandler *GetResolutionChangeHandler() const = 0;

		static DisplayDeviceManager *GetInstance();

	public:
	};
}

#endif
