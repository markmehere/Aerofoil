#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE

#include <Windows.h>

#undef CreateMutex
#undef DeleteFile
#undef LoadCursor
#undef CreateFile

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
	IGpCursor_Win32 *(*m_createColorCursorFunc)(size_t width, size_t height, const void *pixelDataRGBA, size_t hotSpotX, size_t hotSpotY);
	IGpCursor_Win32 *(*m_createBWCursorFunc)(size_t width, size_t height, const void *pixelData, const void *maskData, size_t hotSpotX, size_t hotSpotY);
	void (*m_translateWindowsMessageFunc)(const MSG *msg, IGpVOSEventQueue *eventQueue, float pixelScaleX, float pixelScaleY);
};
