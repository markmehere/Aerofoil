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
	LPCWSTR m_cmdLine;
	int m_cmdLineArgc;
	LPWSTR *m_cmdLineArgv;
	LPCWSTR m_baseDir;
	HWND m_hwnd;
	HICON m_hIcon;
	HICON m_hIconSm;
	int m_nCmdShow;

	IGpFiber *(*m_createFiberFunc)(LPVOID fiber);
	IGpCursor_Win32 *(*m_loadCursorFunc)(const wchar_t *path);
	void (*m_translateWindowsMessageFunc)(const MSG *msg, IGpVOSEventQueue *eventQueue, float pixelScaleX, float pixelScaleY);
};
