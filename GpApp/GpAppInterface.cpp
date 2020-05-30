#include "GpAppInterface.h"

#include "DisplayDeviceManager.h"
#include "HostAudioDriver.h"
#include "HostFileSystem.h"
#include "HostFontHandler.h"
#include "HostDisplayDriver.h"
#include "HostLogDriver.h"
#include "HostSystemServices.h"
#include "HostVOSEventQueue.h"
#include "MenuManager.h"
#include "WindowManager.h"

int gpAppMain();

class GpAppInterfaceImpl final : public GpAppInterface
{
public:
	int ApplicationMain() override;
	void PL_IncrementTickCounter(uint32_t count) override;
	void PL_Render(IGpDisplayDriver *displayDriver) override;
	void PL_HostFileSystem_SetInstance(PortabilityLayer::HostFileSystem *instance) override;
	void PL_HostDisplayDriver_SetInstance(IGpDisplayDriver *instance) override;
	void PL_HostSystemServices_SetInstance(PortabilityLayer::HostSystemServices *instance) override;
	void PL_HostAudioDriver_SetInstance(IGpAudioDriver *instance) override;
	void PL_HostLogDriver_SetInstance(IGpLogDriver *instance) override;
	void PL_HostFontHandler_SetInstance(PortabilityLayer::HostFontHandler *instance) override;
	void PL_HostVOSEventQueue_SetInstance(PortabilityLayer::HostVOSEventQueue *instance) override;
	void PL_InstallHostSuspendHook(PortabilityLayer::HostSuspendHook_t hook, void *context) override;
	bool PL_AdjustRequestedResolution(uint32_t &physicalWidth, uint32_t &physicalHeight, uint32_t &virtualWidth, uint32_t &virtualheight, float &pixelScaleX, float &pixelScaleY) override;
};


int GpAppInterfaceImpl::ApplicationMain()
{
	return gpAppMain();
}

void GpAppInterfaceImpl::PL_IncrementTickCounter(uint32_t count)
{
	PortabilityLayer::DisplayDeviceManager::GetInstance()->IncrementTickCount(count);
}

void GpAppInterfaceImpl::PL_Render(IGpDisplayDriver *displayDriver)
{
	PortabilityLayer::WindowManager::GetInstance()->RenderFrame(displayDriver);
	PortabilityLayer::MenuManager::GetInstance()->RenderFrame(displayDriver);
}

void GpAppInterfaceImpl::PL_HostFileSystem_SetInstance(PortabilityLayer::HostFileSystem *instance)
{
	PortabilityLayer::HostFileSystem::SetInstance(instance);
}

void GpAppInterfaceImpl::PL_HostDisplayDriver_SetInstance(IGpDisplayDriver *instance)
{
	PortabilityLayer::HostDisplayDriver::SetInstance(instance);
}

void GpAppInterfaceImpl::PL_HostLogDriver_SetInstance(IGpLogDriver *instance)
{
	PortabilityLayer::HostLogDriver::SetInstance(instance);
}

void GpAppInterfaceImpl::PL_HostSystemServices_SetInstance(PortabilityLayer::HostSystemServices *instance)
{
	PortabilityLayer::HostSystemServices::SetInstance(instance);
}

void GpAppInterfaceImpl::PL_HostAudioDriver_SetInstance(IGpAudioDriver *instance)
{
	PortabilityLayer::HostAudioDriver::SetInstance(instance);
}

void GpAppInterfaceImpl::PL_HostFontHandler_SetInstance(PortabilityLayer::HostFontHandler *instance)
{
	PortabilityLayer::HostFontHandler::SetInstance(instance);
}

void GpAppInterfaceImpl::PL_HostVOSEventQueue_SetInstance(PortabilityLayer::HostVOSEventQueue *instance)
{
	PortabilityLayer::HostVOSEventQueue::SetInstance(instance);
}

void GpAppInterfaceImpl::PL_InstallHostSuspendHook(PortabilityLayer::HostSuspendHook_t hook, void *context)
{
	PortabilityLayer::InstallHostSuspendHook(hook, context);
}

bool GpAppInterfaceImpl::PL_AdjustRequestedResolution(uint32_t &physicalWidth, uint32_t &physicalHeight, uint32_t &virtualWidth, uint32_t &virtualheight, float &pixelScaleX, float &pixelScaleY)
{
	PortabilityLayer::DisplayDeviceManager::IResolutionChangeHandler *handler = PortabilityLayer::DisplayDeviceManager::GetInstance()->GetResolutionChangeHandler();

	if (!handler)
		return false;

	handler->AdjustRequestedResolution(physicalWidth, physicalHeight, virtualWidth, virtualheight, pixelScaleX, pixelScaleY);

	return true;
}


static GpAppInterfaceImpl gs_application;


GP_APP_DLL_EXPORT_API GpAppInterface *GpAppInterface_Get()
{
	return &gs_application;
}
