#include "GpAppInterface.h"

#include "DisplayDeviceManager.h"
#include "HostAudioDriver.h"
#include "HostFileSystem.h"
#include "HostDisplayDriver.h"
#include "HostSystemServices.h"

int gpAppMain();

class GpAppInterfaceImpl final : public GpAppInterface
{
public:
	int ApplicationMain() override;
	void PL_IncrementTickCounter(uint32_t count) override;
	void PL_HostFileSystem_SetInstance(PortabilityLayer::HostFileSystem *instance) override;
	void PL_HostDisplayDriver_SetInstance(PortabilityLayer::HostDisplayDriver *instance) override;
	void PL_HostSystemServices_SetInstance(PortabilityLayer::HostSystemServices *instance) override;
	void PL_HostAudioDriver_SetInstance(PortabilityLayer::HostAudioDriver *instance) override;
	void PL_InstallHostSuspendHook(PortabilityLayer::HostSuspendHook_t hook, void *context) override;
};


int GpAppInterfaceImpl::ApplicationMain()
{
	return gpAppMain();
}

void GpAppInterfaceImpl::PL_IncrementTickCounter(uint32_t count)
{
	PortabilityLayer::DisplayDeviceManager::GetInstance()->IncrementTickCount(count);
}

void GpAppInterfaceImpl::PL_HostFileSystem_SetInstance(PortabilityLayer::HostFileSystem *instance)
{
	PortabilityLayer::HostFileSystem::SetInstance(instance);
}

void GpAppInterfaceImpl::PL_HostDisplayDriver_SetInstance(PortabilityLayer::HostDisplayDriver *instance)
{
	PortabilityLayer::HostDisplayDriver::SetInstance(instance);
}

void GpAppInterfaceImpl::PL_HostSystemServices_SetInstance(PortabilityLayer::HostSystemServices *instance)
{
	PortabilityLayer::HostSystemServices::SetInstance(instance);
}

void GpAppInterfaceImpl::PL_HostAudioDriver_SetInstance(PortabilityLayer::HostAudioDriver *instance)
{
	PortabilityLayer::HostAudioDriver::SetInstance(instance);
}

void GpAppInterfaceImpl::PL_InstallHostSuspendHook(PortabilityLayer::HostSuspendHook_t hook, void *context)
{
	PortabilityLayer::InstallHostSuspendHook(hook, context);
}

static GpAppInterfaceImpl gs_application;


GP_APP_DLL_EXPORT_API GpAppInterface *GpAppInterface_Get()
{
	return &gs_application;
}
