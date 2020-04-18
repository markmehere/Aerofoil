#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE

#include <Windows.h>

#undef CreateMutex
#undef DeleteFile
#undef LoadCursor

struct IGpFiber;
struct IGpBWCursor_Win32;
struct IGpCursor_Win32;
struct IGpVOSEventQueue;

struct GpWindowsGlobals
{
	HINSTANCE m_hInstance;
	HINSTANCE m_hPrevInstance;
	LPCSTR m_cmdLine;
	LPCWSTR m_baseDir;
	HWND m_hwnd;
	int m_nCmdShow;

	IGpFiber *(*m_createFiberFunc)(LPVOID fiber);
	IGpCursor_Win32 *(*m_loadCursorFunc)(const wchar_t *path);
	void (*m_translateWindowsMessageFunc)(const MSG *msg, IGpVOSEventQueue *eventQueue, float pixelScaleX, float pixelScaleY);
};
