#include "SDL.h"

#include "GpMain.h"
#include "GpAudioDriverFactory.h"
#include "GpDisplayDriverFactory.h"
#include "GpGlobalConfig.h"
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

#include <exception>

GpAndroidGlobals g_gpAndroidGlobals;

extern "C" IGpFontHandler *GpDriver_CreateFontHandler_FreeType2(const GpFontHandlerProperties &properties);

IGpDisplayDriver *GpDriver_CreateDisplayDriver_SDL_GL2(const GpDisplayDriverProperties &properties);
IGpAudioDriver *GpDriver_CreateAudioDriver_SDL(const GpAudioDriverProperties &properties);


int main(int argc, char* argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return -1;

	//SDL_GL_LoadLibrary("libGLESv2.so");

	GpFileSystem_Android::GetInstance()->InitJNI();

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

	int returnCode = GpMain::Run();

	GpFileSystem_Android::GetInstance()->ShutdownJNI();

	SDL_Quit();

	// This doesn't even actually exit, but it does stop this stupid brain-damaged OS from
	// just calling "main" again with no cleanup...
	// That'll have to do until proper cleanup code is added to everything.
	exit(returnCode);

	return returnCode;
}
