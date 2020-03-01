#include "GpMain.h"
#include "GpAudioDriverFactory.h"
#include "GpAudioDriverProperties.h"
#include "GpFontHandlerFactory.h"
#include "GpDisplayDriverFactory.h"
#include "GpDisplayDriverProperties.h"
#include "GpDisplayDriverTickStatus.h"
#include "GpInputDriverFactory.h"
#include "GpInputDriverProperties.h"
#include "GpGlobalConfig.h"
#include "GpAppEnvironment.h"
#include "IGpAudioDriver.h"
#include "IGpDisplayDriver.h"
#include "IGpInputDriver.h"

#include <string.h>
#include <stdlib.h>

namespace
{
	GpDisplayDriverTickStatus_t TickAppEnvironment(void *context, IGpFiber *vosFiber)
	{
		return static_cast<GpAppEnvironment*>(context)->Tick(vosFiber);
	}

	void RenderAppEnvironment(void *context)
	{
		static_cast<GpAppEnvironment*>(context)->Render();
	}
}

int GpMain::Run()
{
	GpVOSEventQueue *eventQueue = new GpVOSEventQueue();
	GpAppEnvironment *appEnvironment = new GpAppEnvironment();

	GpDisplayDriverProperties ddProps;
	memset(&ddProps, 0, sizeof(ddProps));

	ddProps.m_frameTimeLockNumerator = 1;
	ddProps.m_frameTimeLockDenominator = 60;

	// +/- 1% tolerance for frame time variance
	ddProps.m_frameTimeLockMinNumerator = 99;
	ddProps.m_frameTimeLockMinDenominator = 6000;
	ddProps.m_frameTimeLockMaxNumerator = 101;
	ddProps.m_frameTimeLockMaxDenominator = 6000;

	ddProps.m_tickFunc = TickAppEnvironment;
	ddProps.m_tickFuncContext = appEnvironment;

	ddProps.m_renderFunc = RenderAppEnvironment;
	ddProps.m_renderFuncContext = appEnvironment;

	ddProps.m_type = g_gpGlobalConfig.m_displayDriverType;
	ddProps.m_osGlobals = g_gpGlobalConfig.m_osGlobals;
	ddProps.m_eventQueue = eventQueue;

	GpAudioDriverProperties adProps;
	memset(&adProps, 0, sizeof(adProps));

	// The sample rate used in all of Glider PRO's sound is 0x56ee8ba3
	// This appears to be the "standard" Mac sample rate, probably rounded from 244800/11.
	adProps.m_type = g_gpGlobalConfig.m_audioDriverType;
	adProps.m_sampleRate = (244800 * 2 + 11) / (11 * 2);
#ifdef NDEBUG
	adProps.m_debug = true;
#else
	adProps.m_debug = false;
#endif

	IGpInputDriver **inputDrivers = static_cast<IGpInputDriver**>(malloc(sizeof(IGpInputDriver*) * g_gpGlobalConfig.m_numInputDrivers));

	size_t numCreatedInputDrivers = 0;
	if (inputDrivers)
	{
		for (size_t i = 0; i < g_gpGlobalConfig.m_numInputDrivers; i++)
		{
			GpInputDriverProperties inputProps;
			memset(&inputProps, 0, sizeof(inputProps));

			inputProps.m_type = g_gpGlobalConfig.m_inputDriverTypes[i];
			inputProps.m_eventQueue = eventQueue;

			if (IGpInputDriver *driver = GpInputDriverFactory::CreateInputDriver(inputProps))
				inputDrivers[numCreatedInputDrivers++] = driver;
		}
	}

	IGpDisplayDriver *displayDriver = GpDisplayDriverFactory::CreateDisplayDriver(ddProps);
	IGpAudioDriver *audioDriver = GpAudioDriverFactory::CreateAudioDriver(adProps);
	PortabilityLayer::HostFontHandler *fontHandler = GpFontHandlerFactory::Create();

	appEnvironment->Init();

	appEnvironment->SetDisplayDriver(displayDriver);
	appEnvironment->SetAudioDriver(audioDriver);
	appEnvironment->SetInputDrivers(inputDrivers, numCreatedInputDrivers);
	appEnvironment->SetFontHandler(fontHandler);
	appEnvironment->SetVOSEventQueue(eventQueue);

	// Start the display loop
	displayDriver->Run();

	// Clean up
	if (inputDrivers)
	{
		for (size_t i = 0; i < numCreatedInputDrivers; i++)
			inputDrivers[i]->Shutdown();

		free(inputDrivers);
	}

	// GP TODO: Cleanup

	return 0;
}
