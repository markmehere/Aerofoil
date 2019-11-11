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

namespace PortabilityLayer
{
	class HostFileSystem;
	class HostDisplayDriver;
	class HostSystemServices;
}

class GpAppInterface
{
public:
	virtual int ApplicationMain() = 0;
	virtual void PL_IncrementTickCounter(uint32_t count) = 0;
	virtual void PL_HostFileSystem_SetInstance(PortabilityLayer::HostFileSystem *instance) = 0;
	virtual void PL_HostDisplayDriver_SetInstance(PortabilityLayer::HostDisplayDriver *instance) = 0;
	virtual void PL_HostSystemServices_SetInstance(PortabilityLayer::HostSystemServices *instance) = 0;
	virtual void PL_InstallHostSuspendHook(PortabilityLayer::HostSuspendHook_t hook, void *context) = 0;
};

GP_APP_DLL_EXPORT_API GpAppInterface *GpAppInterface_Get();

#endif
