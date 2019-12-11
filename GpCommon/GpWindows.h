#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

struct GPWindowsGlobals
{
	HINSTANCE m_hInstance;
	HINSTANCE m_hPrevInstance;
	LPSTR m_cmdLine;
	int m_nCmdShow;
};

extern GPWindowsGlobals g_gpWindowsGlobals;
