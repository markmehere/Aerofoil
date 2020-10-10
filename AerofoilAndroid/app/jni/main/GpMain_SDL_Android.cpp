#include "SDL.h"

#include "GpMain.h"
#include "GpAudioDriverFactory.h"
#include "GpDisplayDriverFactory.h"
#include "GpGlobalConfig.h"
#include "GpFiber_Win32.h"
#include "GpFiber_SDL.h"
#include "GpFileSystem_Android.h"
#include "GpFontHandlerFactory.h"
#include "GpInputDriverFactory.h"
#include "GpAppInterface.h"
#include "GpSystemServices_Android.h"
#include "GpVOSEvent.h"
#include "IGpVOSEventQueue.h"

#include "HostFileSystem.h"
#include "HostThreadEvent.h"

#include "GpAndroid.h"

#include "resource.h"

GpAndroidGlobals g_gpAndroidGlobals;

extern "C" IGpFontHandler *GpDriver_CreateFontHandler_FreeType2(const GpFontHandlerProperties &properties);

IGpDisplayDriver *GpDriver_CreateDisplayDriver_SDL_GL2(const GpDisplayDriverProperties &properties);
IGpAudioDriver *GpDriver_CreateAudioDriver_SDL(const GpAudioDriverProperties &properties);


int main(int argc, const char **argv)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return -1;

	GpAppInterface_Get()->PL_HostFileSystem_SetInstance(GpFileSystem_Android::GetInstance());
	GpAppInterface_Get()->PL_HostSystemServices_SetInstance(GpSystemServices_Android::GetInstance());

	g_gpGlobalConfig.m_displayDriverType = EGpDisplayDriverType_SDL_GL2;

	g_gpGlobalConfig.m_audioDriverType = EGpAudioDriverType_SDL2;

	g_gpGlobalConfig.m_fontHandlerType = EGpFontHandlerType_FreeType2;

	g_gpGlobalConfig.m_inputDriverTypes = nullptr;
	g_gpGlobalConfig.m_numInputDrivers = 0;

	g_gpGlobalConfig.m_osGlobals = &g_gpAndroidGlobals;
	g_gpGlobalConfig.m_logger = nullptr;
	g_gpGlobalConfig.m_systemServices = GpSystemServices_Android::GetInstance();

	GpDisplayDriverFactory::RegisterDisplayDriverFactory(EGpDisplayDriverType_SDL_GL2, GpDriver_CreateDisplayDriver_SDL_GL2);
	GpAudioDriverFactory::RegisterAudioDriverFactory(EGpAudioDriverType_SDL2, GpDriver_CreateAudioDriver_SDL);
	GpFontHandlerFactory::RegisterFontHandlerFactory(EGpFontHandlerType_FreeType2, GpDriver_CreateFontHandler_FreeType2);

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "SDL environment configured, starting up");

	int returnCode = GpMain::Run();

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "SDL environment exited with code %i, cleaning up", returnCode);

	return returnCode;
}
