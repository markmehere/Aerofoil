#include "GpMain.h"
#include "GpAudioDriverFactory.h"
#include "GpDisplayDriverFactory.h"
#include "GpDisplayDriverFactoryD3D11.h"
#include "GpGlobalConfig.h"
#include "GpFileSystem_Win32.h"
#include "GpAppInterface.h"
#include "GpSystemServices_Win32.h"

#include "HostFileSystem.h"

#include "GpWindows.h"

#include <stdio.h>

GPWindowsGlobals g_gpWindowsGlobals;

extern "C" __declspec(dllimport) IGpAudioDriver *GpDriver_CreateAudioDriver_XAudio2(const GpAudioDriverProperties &properties);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	GpAppInterface_Get()->PL_HostFileSystem_SetInstance(GpFileSystem_Win32::GetInstance());
	GpAppInterface_Get()->PL_HostSystemServices_SetInstance(GpSystemServices_Win32::GetInstance());

	g_gpWindowsGlobals.m_hInstance = hInstance;
	g_gpWindowsGlobals.m_hPrevInstance = hPrevInstance;
	g_gpWindowsGlobals.m_cmdLine = lpCmdLine;
	g_gpWindowsGlobals.m_nCmdShow = nCmdShow;

	g_gpGlobalConfig.m_displayDriverType = EGpDisplayDriverType_D3D11;

	GpDisplayDriverFactory::RegisterDisplayDriverFactory(EGpDisplayDriverType_D3D11, GpDisplayDriverFactoryD3D11::Create);
	GpAudioDriverFactory::RegisterAudioDriverFactory(EGpAudioDriverType_XAudio2, GpDriver_CreateAudioDriver_XAudio2);

	return GpMain::Run();
}
