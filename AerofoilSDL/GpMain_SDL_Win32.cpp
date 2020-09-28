#include "SDL.h"

#include "GpMain.h"
#include "GpAudioDriverFactory.h"
#include "GpDisplayDriverFactory.h"
#include "GpGlobalConfig.h"
#include "GpFiber_Win32.h"
#include "GpFiber_SDL.h"
#include "GpFileSystem_Win32.h"
#include "GpLogDriver_Win32.h"
#include "GpFontHandlerFactory.h"
#include "GpInputDriverFactory.h"
#include "GpAppInterface.h"
#include "GpSystemServices_Win32.h"
#include "GpVOSEvent.h"
#include "IGpVOSEventQueue.h"

#include "HostFileSystem.h"
#include "HostThreadEvent.h"

#include "GpWindows.h"

#include "resource.h"

#include <shellapi.h>
#include <stdio.h>
#include <windowsx.h>


GpWindowsGlobals g_gpWindowsGlobals;

extern "C" __declspec(dllimport) IGpInputDriver *GpDriver_CreateInputDriver_XInput(const GpInputDriverProperties &properties);
extern "C" __declspec(dllimport) IGpFontHandler *GpDriver_CreateFontHandler_FreeType2(const GpFontHandlerProperties &properties);

IGpDisplayDriver *GpDriver_CreateDisplayDriver_SDL_GL2(const GpDisplayDriverProperties &properties);
IGpAudioDriver *GpDriver_CreateAudioDriver_SDL(const GpAudioDriverProperties &properties);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return -1;

	LPWSTR cmdLine = GetCommandLineW();

	int nArgs;
	LPWSTR *cmdLineArgs = CommandLineToArgvW(cmdLine, &nArgs);

	for (int i = 1; i < nArgs; i++)
	{
		if (!wcscmp(cmdLineArgs[i], L"-diagnostics"))
			GpLogDriver_Win32::Init();
	}

	IGpLogDriver *logger = GpLogDriver_Win32::GetInstance();

	GpAppInterface_Get()->PL_HostFileSystem_SetInstance(GpFileSystem_Win32::GetInstance());
	GpAppInterface_Get()->PL_HostSystemServices_SetInstance(GpSystemServices_Win32::GetInstance());
	GpAppInterface_Get()->PL_HostLogDriver_SetInstance(GpLogDriver_Win32::GetInstance());

	g_gpWindowsGlobals.m_hInstance = hInstance;
	g_gpWindowsGlobals.m_hPrevInstance = hPrevInstance;
	g_gpWindowsGlobals.m_cmdLine = cmdLine;
	g_gpWindowsGlobals.m_cmdLineArgc = nArgs;
	g_gpWindowsGlobals.m_cmdLineArgv = cmdLineArgs;
	g_gpWindowsGlobals.m_nCmdShow = nCmdShow;
	g_gpWindowsGlobals.m_baseDir = GpFileSystem_Win32::GetInstance()->GetBasePath();
	g_gpWindowsGlobals.m_hwnd = nullptr;

	g_gpGlobalConfig.m_displayDriverType = EGpDisplayDriverType_SDL_GL2;

	g_gpGlobalConfig.m_audioDriverType = EGpAudioDriverType_SDL2;

	g_gpGlobalConfig.m_fontHandlerType = EGpFontHandlerType_FreeType2;

	EGpInputDriverType inputDrivers[] =
	{
		EGpInputDriverType_XInput
	};

	g_gpGlobalConfig.m_inputDriverTypes = inputDrivers;
	g_gpGlobalConfig.m_numInputDrivers = sizeof(inputDrivers) / sizeof(inputDrivers[0]);

	g_gpGlobalConfig.m_osGlobals = &g_gpWindowsGlobals;
	g_gpGlobalConfig.m_logger = logger;
	g_gpGlobalConfig.m_systemServices = GpSystemServices_Win32::GetInstance();

	GpDisplayDriverFactory::RegisterDisplayDriverFactory(EGpDisplayDriverType_SDL_GL2, GpDriver_CreateDisplayDriver_SDL_GL2);
	GpAudioDriverFactory::RegisterAudioDriverFactory(EGpAudioDriverType_SDL2, GpDriver_CreateAudioDriver_SDL);
	GpInputDriverFactory::RegisterInputDriverFactory(EGpInputDriverType_XInput, GpDriver_CreateInputDriver_XInput);
	GpFontHandlerFactory::RegisterFontHandlerFactory(EGpFontHandlerType_FreeType2, GpDriver_CreateFontHandler_FreeType2);

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "SDL environment configured, starting up");

	int returnCode = GpMain::Run();

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "SDL environment exited with code %i, cleaning up", returnCode);

	LocalFree(cmdLineArgs);

	return returnCode;
}
