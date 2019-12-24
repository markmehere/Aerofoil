#include "GpMain.h"
#include "GpAudioDriverFactory.h"
#include "GpColorCursor_Win32.h"
#include "GpDisplayDriverFactory.h"
#include "GpGlobalConfig.h"
#include "GpFiber_Win32.h"
#include "GpFileSystem_Win32.h"
#include "GpAppInterface.h"
#include "GpSystemServices_Win32.h"
#include "GpVOSEvent.h"
#include "IGpVOSEventQueue.h"

#include "HostFileSystem.h"

#include "GpWindows.h"

#include <stdio.h>
#include <windowsx.h>

GpWindowsGlobals g_gpWindowsGlobals;

extern "C" __declspec(dllimport) IGpAudioDriver *GpDriver_CreateAudioDriver_XAudio2(const GpAudioDriverProperties &properties);
extern "C" __declspec(dllimport) IGpDisplayDriver *GpDriver_CreateDisplayDriver_D3D11(const GpDisplayDriverProperties &properties);

static void PostMouseEvent(IGpVOSEventQueue *eventQueue, GpMouseEventType_t eventType, GpMouseButton_t button, int32_t x, int32_t y)
{
	if (GpVOSEvent *evt = eventQueue->QueueEvent())
	{
		evt->m_eventType = GpVOSEventTypes::kMouseInput;

		GpMouseInputEvent &mEvent = evt->m_event.m_mouseInputEvent;
		mEvent.m_button = button;
		mEvent.m_x = x;
		mEvent.m_y = y;
		mEvent.m_eventType = eventType;
	}
}

static void TranslateWindowsMessage(const MSG *msg, IGpVOSEventQueue *eventQueue)
{
	WPARAM wParam = msg->wParam;
	LPARAM lParam = msg->lParam;

	switch (msg->message)
	{
		case WM_LBUTTONDOWN:
			PostMouseEvent(eventQueue, GpMouseEventTypes::kDown, GpMouseButtons::kLeft, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_LBUTTONUP:
			PostMouseEvent(eventQueue, GpMouseEventTypes::kUp, GpMouseButtons::kLeft, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_MBUTTONDOWN:
			PostMouseEvent(eventQueue, GpMouseEventTypes::kDown, GpMouseButtons::kMiddle, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_MBUTTONUP:
			PostMouseEvent(eventQueue, GpMouseEventTypes::kUp, GpMouseButtons::kMiddle, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_RBUTTONDOWN:
			PostMouseEvent(eventQueue, GpMouseEventTypes::kDown, GpMouseButtons::kRight, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_RBUTTONUP:
			PostMouseEvent(eventQueue, GpMouseEventTypes::kUp, GpMouseButtons::kRight, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_XBUTTONDOWN:
			if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
				PostMouseEvent(eventQueue, GpMouseEventTypes::kDown, GpMouseButtons::kX1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2)
				PostMouseEvent(eventQueue, GpMouseEventTypes::kDown, GpMouseButtons::kX2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_XBUTTONUP:
			if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
				PostMouseEvent(eventQueue, GpMouseEventTypes::kUp, GpMouseButtons::kX1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2)
				PostMouseEvent(eventQueue, GpMouseEventTypes::kUp, GpMouseButtons::kX2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_MOUSEMOVE:
			PostMouseEvent(eventQueue, GpMouseEventTypes::kMove, GpMouseButtons::kNone, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_MOUSELEAVE:
			PostMouseEvent(eventQueue, GpMouseEventTypes::kLeave, GpMouseButtons::kNone, 0, 0);
			break;
		default:
			break;
	}
}

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
	g_gpWindowsGlobals.m_translateWindowsMessageFunc = TranslateWindowsMessage;

	g_gpGlobalConfig.m_displayDriverType = EGpDisplayDriverType_D3D11;
	g_gpGlobalConfig.m_osGlobals = &g_gpWindowsGlobals;

	GpDisplayDriverFactory::RegisterDisplayDriverFactory(EGpDisplayDriverType_D3D11, GpDriver_CreateDisplayDriver_D3D11);
	GpAudioDriverFactory::RegisterAudioDriverFactory(EGpAudioDriverType_XAudio2, GpDriver_CreateAudioDriver_XAudio2);

	return GpMain::Run();
}
