#pragma once
#ifndef __GPAPP_INTERFACE_H__
#define __GPAPP_INTERFACE_H__

#include "HostSuspendHook.h"
#include <stdint.h>

#ifdef GP_APP_DLL

#ifdef GP_APP_DLL_EXPORT
#define GP_APP_DLL_EXPORT_API extern "C" __declspec(dllexport)
#else
#define GP_APP_DLL_EXPORT_API extern "C" __declspec(dllimport)
#endif

#else

#define GP_APP_DLL_EXPORT_API extern "C"

#endif

struct IGpAudioDriver;
struct IGpLogDriver;
struct IGpInputDriver;

namespace PortabilityLayer
{
	class HostFileSystem;
	class HostDisplayDriver;
	class HostSystemServices;
	class HostFontHandler;
	class HostVOSEventQueue;
}

struct IGpDisplayDriver;

class GpAppInterface
{
public:
	virtual int ApplicationMain() = 0;
	virtual void PL_IncrementTickCounter(uint32_t count) = 0;
	virtual void PL_Render(IGpDisplayDriver *displayDriver) = 0;
	virtual void PL_HostAudioDriver_SetInstance(IGpAudioDriver *instance) = 0;
	virtual void PL_HostFileSystem_SetInstance(PortabilityLayer::HostFileSystem *instance) = 0;
	virtual void PL_HostDisplayDriver_SetInstance(IGpDisplayDriver *instance) = 0;
	virtual void PL_HostLogDriver_SetInstance(IGpLogDriver *instance) = 0;
	virtual void PL_HostInputDriver_SetInstances(IGpInputDriver *const* instances, size_t numInstances) = 0;
	virtual void PL_HostSystemServices_SetInstance(PortabilityLayer::HostSystemServices *instance) = 0;
	virtual void PL_HostFontHandler_SetInstance(PortabilityLayer::HostFontHandler *instance) = 0;
	virtual void PL_HostVOSEventQueue_SetInstance(PortabilityLayer::HostVOSEventQueue *instance) = 0;

	virtual void PL_InstallHostSuspendHook(PortabilityLayer::HostSuspendHook_t hook, void *context) = 0;
	virtual bool PL_AdjustRequestedResolution(uint32_t &physicalWidth, uint32_t &physicalHeight, uint32_t &virtualWidth, uint32_t &virtualheight, float &pixelScaleX, float &pixelScaleY) = 0;
};

GP_APP_DLL_EXPORT_API GpAppInterface *GpAppInterface_Get();

#endif
