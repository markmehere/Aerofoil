#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE

#include <Windows.h>

#undef CreateMutex

struct IGpFiber;
struct IGpColorCursor_Win32;
struct IGpVOSEventQueue;

struct GpWindowsGlobals
{
	HINSTANCE m_hInstance;
	HINSTANCE m_hPrevInstance;
	LPCSTR m_cmdLine;
	LPCWSTR m_baseDir;
	int m_nCmdShow;

	IGpFiber *(*m_createFiberFunc)(LPVOID fiber);
	IGpColorCursor_Win32 *(*m_loadColorCursorFunc)(const wchar_t *path);
	void (*m_translateWindowsMessageFunc)(const MSG *msg, IGpVOSEventQueue *eventQueue);
};

