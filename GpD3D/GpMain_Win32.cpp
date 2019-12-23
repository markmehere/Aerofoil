#include "GpMain.h"
#include "GpAudioDriverFactory.h"
#include "GpColorCursor_Win32.h"
#include "GpDisplayDriverFactory.h"
#include "GpGlobalConfig.h"
#include "GpFiber_Win32.h"
#include "GpFileSystem_Win32.h"
#include "GpAppInterface.h"
#include "GpSystemServices_Win32.h"

#include "HostFileSystem.h"

#include "GpWindows.h"

#include <stdio.h>

GpWindowsGlobals g_gpWindowsGlobals;

extern "C" __declspec(dllimport) IGpAudioDriver *GpDriver_CreateAudioDriver_XAudio2(const GpAudioDriverProperties &properties);
extern "C" __declspec(dllimport) IGpDisplayDriver *GpDriver_CreateDisplayDriver_D3D11(const GpDisplayDriverProperties &properties);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	GpAppInterface_Get()->PL_HostFileSystem_SetInstance(GpFileSystem_Win32::GetInstance());
	GpAppInterface_Get()->PL_HostSystemServices_SetInstance(GpSystemServices_Win32::GetInstance());

	g_gpWindowsGlobals.m_hInstance = hInstance;
	g_gpWindowsGlobals.m_hPrevInstance = hPrevInstance;
	g_gpWindowsGlobals.m_cmdLine = lpCmdLine;
	g_gpWindowsGlobals.m_nCmdShow = nCmdShow;
	g_gpWindowsGlobals.m_baseDir = GpFileSystem_Win32::GetInstance()->GetBasePath();

	g_gpWindowsGlobals.m_createFiberFunc = GpFiber_Win32::Create;
	g_gpWindowsGlobals.m_loadColorCursorFunc = GpColorCursor_Win32::Load;

	g_gpGlobalConfig.m_displayDriverType = EGpDisplayDriverType_D3D11;
	g_gpGlobalConfig.m_osGlobals = &g_gpWindowsGlobals;

	GpDisplayDriverFactory::RegisterDisplayDriverFactory(EGpDisplayDriverType_D3D11, GpDriver_CreateDisplayDriver_D3D11);
	GpAudioDriverFactory::RegisterAudioDriverFactory(EGpAudioDriverType_XAudio2, GpDriver_CreateAudioDriver_XAudio2);

	return GpMain::Run();
}
