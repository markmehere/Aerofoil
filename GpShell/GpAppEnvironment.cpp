#include "GpAppEnvironment.h"
#include "GpFiberStarter.h"
#include "GpAppInterface.h"
#include "GpDisplayDriverTickStatus.h"
#include "GpFontHandlerFactory.h"
#include "HostSuspendCallArgument.h"
#include "IGpDisplayDriver.h"
#include "IGpFiber.h"
#include "IGpInputDriver.h"

#include <assert.h>

GpAppEnvironment::GpAppEnvironment()
	: m_displayDriver(nullptr)
	, m_audioDriver(nullptr)
	, m_inputDrivers(nullptr)
	, m_numInputDrivers(0)
	, m_fontHandler(nullptr)
	, m_vosEventQueue(nullptr)
	, m_systemServices(nullptr)
	, m_suspendCallID(PortabilityLayer::HostSuspendCallID_Unknown)
	, m_suspendArgs(nullptr)
	, m_suspendReturnValue(nullptr)
{
}

GpAppEnvironment::~GpAppEnvironment()
{
}

void GpAppEnvironment::Init()
{
	GpAppInterface_Get()->ApplicationInit();
}

void GpAppEnvironment::Run()
{
	InitializeApplicationState();
	GpAppInterface_Get()->ApplicationMain();
}


void GpAppEnvironment::Render()
{
	GpAppInterface_Get()->PL_Render(m_displayDriver);
}

bool GpAppEnvironment::AdjustRequestedResolution(uint32_t &physicalWidth, uint32_t &physicalHeight, uint32_t &virtualWidth, uint32_t &virtualheight, float &pixelScaleX, float &pixelScaleY)
{
	return GpAppInterface_Get()->PL_AdjustRequestedResolution(physicalWidth, physicalHeight, virtualWidth, virtualheight, pixelScaleX, pixelScaleY);
}

void GpAppEnvironment::SetDisplayDriver(IGpDisplayDriver *displayDriver)
{
	m_displayDriver = displayDriver;
}

void GpAppEnvironment::SetAudioDriver(IGpAudioDriver *audioDriver)
{
	m_audioDriver = audioDriver;
}

void GpAppEnvironment::SetInputDrivers(IGpInputDriver *const* inputDrivers, size_t numDrivers)
{
	m_inputDrivers = inputDrivers;
	m_numInputDrivers = numDrivers;
}

void GpAppEnvironment::SetFontHandler(IGpFontHandler *fontHandler)
{
	m_fontHandler = fontHandler;
}

void GpAppEnvironment::SetVOSEventQueue(GpVOSEventQueue *eventQueue)
{
	m_vosEventQueue = eventQueue;
}

void GpAppEnvironment::SetSystemServices(IGpSystemServices *systemServices)
{
	m_systemServices = systemServices;
}

void GpAppEnvironment::InitializeApplicationState()
{
	GpDriverCollection *drivers = GpAppInterface_Get()->PL_GetDriverCollection();
	drivers->SetDriver<GpDriverIDs::kDisplay>(m_displayDriver);
	drivers->SetDriver<GpDriverIDs::kAudio>(m_audioDriver);
	drivers->SetDrivers<GpDriverIDs::kInput>(m_inputDrivers, m_numInputDrivers);
	drivers->SetDriver<GpDriverIDs::kFont>(m_fontHandler);
	drivers->SetDriver<GpDriverIDs::kEventQueue>(m_vosEventQueue);
}

void GpAppEnvironment::SynchronizeState()
{
	const size_t numInputDrivers = m_numInputDrivers;
	for (size_t i = 0; i < numInputDrivers; i++)
		m_inputDrivers[i]->ProcessInput();
}
