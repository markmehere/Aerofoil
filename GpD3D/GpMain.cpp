#include "GpMain.h"
#include "GpAudioDriverFactory.h"
#include "GpAudioDriverProperties.h"
#include "GpDisplayDriverFactory.h"
#include "GpDisplayDriverProperties.h"
#include "GpGlobalConfig.h"
#include "GpAppEnvironment.h"
#include "IGpAudioDriver.h"
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

	ddProps.m_type = EGpDisplayDriverType_D3D11;

	ddProps.m_frameTimeLockNumerator = 1;
	ddProps.m_frameTimeLockDenominator = 60;

	// +/- 1% tolerance for frame time variance
	ddProps.m_frameTimeLockMinNumerator = 99;
	ddProps.m_frameTimeLockMinDenominator = 6000;
	ddProps.m_frameTimeLockMaxNumerator = 101;
	ddProps.m_frameTimeLockMaxDenominator = 6000;

	ddProps.m_tickFunc = TickAppEnvironment;
	ddProps.m_tickFuncContext = appEnvironment;
	ddProps.m_type = g_gpGlobalConfig.m_displayDriverType;

	GpAudioDriverProperties adProps;
	memset(&adProps, 0, sizeof(adProps));

	// The sample rate used in all of Glider PRO's sound is 0x56ee8ba3
	// This appears to be the "standard" Mac sample rate, probably rounded from 244800/11.
	adProps.m_type = EGpAudioDriverType_XAudio2;
	adProps.m_sampleRate = (244800 * 2 + 11) / (11 * 2);
	adProps.m_debug = true;

	IGpDisplayDriver *displayDriver = GpDisplayDriverFactory::CreateDisplayDriver(ddProps);
	IGpAudioDriver *audioDriver = GpAudioDriverFactory::CreateAudioDriver(adProps);

	appEnvironment->Init();

	appEnvironment->SetDisplayDriver(displayDriver);
	appEnvironment->SetAudioDriver(audioDriver);

	// Start the display loop
	displayDriver->Run();

	return 0;
}
