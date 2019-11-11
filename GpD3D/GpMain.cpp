#include "GpMain.h"
#include "GpDisplayDriverFactory.h"
#include "GpDisplayDriverProperties.h"
#include "GpGlobalConfig.h"
#include "GpAppEnvironment.h"
#include "IGpDisplayDriver.h"

#include <string.h>

namespace
{
	void TickAppEnvironment(void *context, GpFiber *vosFiber)
	{
		static_cast<GpAppEnvironment*>(context)->Tick(vosFiber);
	}
}

int GpMain::Run()
{
	GpAppEnvironment *appEnvironment = new GpAppEnvironment();

	GpDisplayDriverProperties ddProps;
	memset(&ddProps, 0, sizeof(ddProps));

	ddProps.m_FrameTimeLockNumerator = 1;
	ddProps.m_FrameTimeLockDenominator = 60;

	// +/- 1% tolerance for frame time variance
	ddProps.m_FrameTimeLockMinNumerator = 99;
	ddProps.m_FrameTimeLockMinDenominator = 6000;
	ddProps.m_FrameTimeLockMaxNumerator = 101;
	ddProps.m_FrameTimeLockMaxDenominator = 6000;

	ddProps.m_TickFunc = TickAppEnvironment;
	ddProps.m_TickFuncContext = appEnvironment;
	ddProps.m_Type = g_gpGlobalConfig.m_displayDriverType;

	IGpDisplayDriver *displayDriver = GpDisplayDriverFactory::CreateDisplayDriver(ddProps);

	appEnvironment->Init();

	appEnvironment->SetDisplayDriver(displayDriver);

	// Start the display loop
	displayDriver->Run();

	return 0;
}
