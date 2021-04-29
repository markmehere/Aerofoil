#include "SDL.h"

#include "GpMain.h"
#include "GpAllocator_C.h"
#include "GpAudioDriverFactory.h"
#include "GpDisplayDriverFactory.h"
#include "GpGlobalConfig.h"
#include "GpFileSystem_Win32.h"
#include "GpLogDriver_Win32.h"
#include "GpFontHandlerFactory.h"
#include "GpInputDriverFactory.h"
#include "GpAppInterface.h"
#include "GpSystemServices_Win32.h"
#include "GpVOSEvent.h"

#include "IGpFileSystem.h"
#include "IGpThreadEvent.h"
#include "IGpVOSEventQueue.h"

#include "GpWindows.h"

#include "resource.h"

#include <shellapi.h>
#include <stdio.h>
#include <windowsx.h>


GpWindowsGlobals g_gpWindowsGlobals;

extern "C" __declspec(dllimport) IGpFontHandler *GpDriver_CreateFontHandler_FreeType2(const GpFontHandlerProperties &properties);

IGpDisplayDriver *GpDriver_CreateDisplayDriver_SDL_GL2(const GpDisplayDriverProperties &properties);
IGpAudioDriver *GpDriver_CreateAudioDriver_SDL(const GpAudioDriverProperties &properties);
IGpInputDriver *GpDriver_CreateInputDriver_SDL2_Gamepad(const GpInputDriverProperties &properties);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	IGpAllocator *alloc = GpAllocator_C::GetInstance();

	GpFileSystem_Win32 *fs = GpFileSystem_Win32::CreateInstance(alloc);
	if (!fs)
		return -1;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
		return -1;

	LPWSTR cmdLine = GetCommandLineW();

	int nArgs;
	LPWSTR *cmdLineArgs = CommandLineToArgvW(cmdLine, &nArgs);

	for (int i = 1; i < nArgs; i++)
	{
		if (!wcscmp(cmdLineArgs[i], L"-diagnostics"))
			GpLogDriver_Win32::Init();

		if (!wcscmp(cmdLineArgs[i], L"-touchscreensimulation"))
			GpSystemServices_Win32::GetInstance()->SetTouchscreenSimulation(true);
	}

	IGpLogDriver *logger = GpLogDriver_Win32::GetInstance();
	GpDriverCollection *drivers = GpAppInterface_Get()->PL_GetDriverCollection();

	drivers->SetDriver<GpDriverIDs::kFileSystem>(fs);
	drivers->SetDriver<GpDriverIDs::kSystemServices>(GpSystemServices_Win32::GetInstance());
	drivers->SetDriver<GpDriverIDs::kLog>(GpLogDriver_Win32::GetInstance());
	drivers->SetDriver<GpDriverIDs::kAlloc>(GpAllocator_C::GetInstance());

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
		EGpInputDriverType_SDL2_Gamepad
	};

	g_gpGlobalConfig.m_inputDriverTypes = inputDrivers;
	g_gpGlobalConfig.m_numInputDrivers = sizeof(inputDrivers) / sizeof(inputDrivers[0]);

	g_gpGlobalConfig.m_osGlobals = &g_gpWindowsGlobals;
	g_gpGlobalConfig.m_logger = logger;
	g_gpGlobalConfig.m_systemServices = GpSystemServices_Win32::GetInstance();
	g_gpGlobalConfig.m_allocator = alloc;

	GpDisplayDriverFactory::RegisterDisplayDriverFactory(EGpDisplayDriverType_SDL_GL2, GpDriver_CreateDisplayDriver_SDL_GL2);
	GpAudioDriverFactory::RegisterAudioDriverFactory(EGpAudioDriverType_SDL2, GpDriver_CreateAudioDriver_SDL);
	GpInputDriverFactory::RegisterInputDriverFactory(EGpInputDriverType_SDL2_Gamepad, GpDriver_CreateInputDriver_SDL2_Gamepad);
	GpFontHandlerFactory::RegisterFontHandlerFactory(EGpFontHandlerType_FreeType2, GpDriver_CreateFontHandler_FreeType2);

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "SDL environment configured, starting up");

	int returnCode = GpMain::Run();

	if (logger)
		logger->Printf(IGpLogDriver::Category_Information, "SDL environment exited with code %i, cleaning up", returnCode);

	LocalFree(cmdLineArgs);

	fs->Destroy();

	return returnCode;
}
