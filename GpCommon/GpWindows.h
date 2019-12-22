#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE

#include <Windows.h>

struct GpWindowsGlobals
{
	HINSTANCE m_hInstance;
	HINSTANCE m_hPrevInstance;
	LPCSTR m_cmdLine;
	LPCWSTR m_baseDir;
	int m_nCmdShow;
};

extern GpWindowsGlobals g_gpWindowsGlobals;

#undef CreateMutex
