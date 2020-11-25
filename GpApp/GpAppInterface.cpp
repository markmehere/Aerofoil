#include "GpAppInterface.h"

#include "DisplayDeviceManager.h"
#include "MenuManager.h"
#include "WindowManager.h"

#include "PLDrivers.h"

int gpAppMain();
void gpAppInit();


class GpAppInterfaceImpl final : public GpAppInterface
{
public:
	void ApplicationInit() override;
	int ApplicationMain() override;

	void PL_IncrementTickCounter(uint32_t count) override;
	void PL_Render(IGpDisplayDriver *displayDriver) override;
	GpDriverCollection *PL_GetDriverCollection() override;
	void PL_InstallHostSuspendHook(PortabilityLayer::HostSuspendHook_t hook, void *context) override;
	bool PL_AdjustRequestedResolution(uint32_t &physicalWidth, uint32_t &physicalHeight, uint32_t &virtualWidth, uint32_t &virtualheight, float &pixelScaleX, float &pixelScaleY) override;
};

void GpAppInterfaceImpl::ApplicationInit()
{
	gpAppInit();
}

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

GpDriverCollection *GpAppInterfaceImpl::PL_GetDriverCollection()
{
	return PLDrivers::GetDriverCollection();
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
