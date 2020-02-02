#include "GpMain.h"
#include "GpAudioDriverFactory.h"
#include "GpColorCursor_Win32.h"
#include "GpDisplayDriverFactory.h"
#include "GpGlobalConfig.h"
#include "GpFiber_Win32.h"
#include "GpFileSystem_Win32.h"
#include "GpInputDriverFactory.h"
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
extern "C" __declspec(dllimport) IGpInputDriver *GpDriver_CreateInputDriver_XInput(const GpInputDriverProperties &properties);

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

static bool IdentifyVKey(const WPARAM &wparam, const LPARAM &lparam, GpKeyIDSubset_t &outSubset, GpKeyboardInputEvent::KeyUnion &outKey)
{
	switch (wparam)
	{
	case VK_ESCAPE:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kEscape;
		break;
	case VK_PRINT:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kPrintScreen;
		break;
	case VK_SCROLL:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kScrollLock;
		break;
	case VK_PAUSE:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kPause;
		break;
	case VK_INSERT:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kInsert;
		break;
	case VK_HOME:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kHome;
		break;
	case VK_PRIOR:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kPageUp;
		break;
	case VK_NEXT:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kPageDown;
		break;
	case VK_DELETE:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kDelete;
		break;
	case VK_TAB:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kTab;
		break;
	case VK_END:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kEnd;
		break;
	case VK_BACK:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kBackspace;
		break;
	case VK_CAPITAL:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kCapsLock;
		break;
	case VK_RETURN:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kEnter;
		break;
	case VK_SHIFT:
		{
			UINT vkey = MapVirtualKeyW((lparam >> 16) & 0xff, MAPVK_VSC_TO_VK_EX);

			if (vkey == VK_LSHIFT || vkey == VK_SHIFT)
			{
				outSubset = GpKeyIDSubsets::kSpecial;
				outKey.m_specialKey = GpKeySpecials::kLeftShift;
			}
			else if (vkey == VK_RSHIFT)
			{
				outSubset = GpKeyIDSubsets::kSpecial;
				outKey.m_specialKey = GpKeySpecials::kRightShift;
			}
			else
				return false;
		}
		break;
	case VK_RSHIFT:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kRightShift;
		break;
	case VK_CONTROL:
		outSubset = GpKeyIDSubsets::kSpecial;
		if (lparam & 0x01000000)
			outKey.m_specialKey = GpKeySpecials::kRightCtrl;
		else
			outKey.m_specialKey = GpKeySpecials::kLeftCtrl;
		break;
	case VK_MENU:
		outSubset = GpKeyIDSubsets::kSpecial;
		if (lparam & 0x01000000)
			outKey.m_specialKey = GpKeySpecials::kRightAlt;
		else
			outKey.m_specialKey = GpKeySpecials::kLeftAlt;
		break;
	case VK_NUMLOCK:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kNumLock;
		break;
	case VK_NUMPAD0:
	case VK_NUMPAD1:
	case VK_NUMPAD2:
	case VK_NUMPAD3:
	case VK_NUMPAD4:
	case VK_NUMPAD5:
	case VK_NUMPAD6:
	case VK_NUMPAD7:
	case VK_NUMPAD8:
	case VK_NUMPAD9:
		outSubset = GpKeyIDSubsets::kNumPadNumber;
		outKey.m_numPadNumber = static_cast<uint8_t>(wparam - VK_NUMPAD0);
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
		outSubset = GpKeyIDSubsets::kASCII;
		outKey.m_asciiChar = static_cast<char>(wparam);
		break;
	case VK_F1:
	case VK_F2:
	case VK_F3:
	case VK_F4:
	case VK_F5:
	case VK_F6:
	case VK_F7:
	case VK_F8:
	case VK_F9:
	case VK_F10:
	case VK_F11:
	case VK_F12:
	case VK_F13:
	case VK_F14:
	case VK_F15:
	case VK_F16:
	case VK_F17:
	case VK_F18:
	case VK_F19:
	case VK_F20:
	case VK_F21:
	case VK_F22:
	case VK_F23:
	case VK_F24:
		outSubset = GpKeyIDSubsets::kFKey;
		outKey.m_fKey = static_cast<uint8_t>(wparam - VK_F1 + 1);
		break;
	case VK_OEM_COMMA:
		outSubset = GpKeyIDSubsets::kASCII;
		outKey.m_asciiChar = ',';
		break;
	case VK_OEM_MINUS:
		outSubset = GpKeyIDSubsets::kASCII;
		outKey.m_asciiChar = '-';
		break;
	case VK_OEM_PERIOD:
		outSubset = GpKeyIDSubsets::kASCII;
		outKey.m_asciiChar = '.';
		break;
	case VK_OEM_PLUS:
		outSubset = GpKeyIDSubsets::kASCII;
		outKey.m_asciiChar = '+';
		break;
	case VK_UP:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kUpArrow;
		break;
	case VK_DOWN:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kDownArrow;
		break;
	case VK_LEFT:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kLeftArrow;
		break;
	case VK_RIGHT:
		outSubset = GpKeyIDSubsets::kSpecial;
		outKey.m_specialKey = GpKeySpecials::kRightArrow;
		break;
	default:
		{
			if (wparam >= VK_OEM_1 && wparam <= VK_OEM_102)
			{
				UINT charCode = MapVirtualKeyW(static_cast<UINT>(wparam), MAPVK_VK_TO_CHAR);
				if (charCode == 0)
					return false;

				if (charCode < 128)
				{
					outSubset = GpKeyIDSubsets::kASCII;
					outKey.m_asciiChar = static_cast<char>(charCode);
					break;
				}
				else
				{
					outSubset = GpKeyIDSubsets::kUnicode;
					outKey.m_unicodeChar = charCode;;
					break;
				}
			}
		}
		return false;
	}

	return true;
}

static void PostKeyboardEvent(IGpVOSEventQueue *eventQueue, GpKeyboardInputEventType_t eventType, GpKeyIDSubset_t subset, const GpKeyboardInputEvent::KeyUnion &key, uint32_t repeatCount)
{
	if (GpVOSEvent *evt = eventQueue->QueueEvent())
	{
		evt->m_eventType = GpVOSEventTypes::kKeyboardInput;

		GpKeyboardInputEvent &mEvent = evt->m_event.m_keyboardInputEvent;
		mEvent.m_key = key;
		mEvent.m_eventType = eventType;
		mEvent.m_keyIDSubset = subset;
		mEvent.m_repeatCount = repeatCount;
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
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			{
				GpKeyIDSubset_t subset;
				GpKeyboardInputEvent::KeyUnion key;
				bool isRepeat = ((lParam & 0x40000000) != 0);
				const GpKeyboardInputEventType_t keyEventType = isRepeat ? GpKeyboardInputEventTypes::kAuto : GpKeyboardInputEventTypes::kDown;
				if (!isRepeat && IdentifyVKey(wParam, lParam, subset, key))
					PostKeyboardEvent(eventQueue, keyEventType, subset, key, static_cast<uint32_t>(lParam & 0xffff));

				(void)TranslateMessage(msg);
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			{
				GpKeyIDSubset_t subset;
				GpKeyboardInputEvent::KeyUnion key;
				if (IdentifyVKey(wParam, lParam, subset, key))
					PostKeyboardEvent(eventQueue, GpKeyboardInputEventTypes::kUp, subset, key, (lParam & 0xffff));
			}
			break;
		case WM_CHAR:
			{
				bool isRepeat = ((lParam & 0x4000000) != 0);
				const GpKeyboardInputEventType_t keyEventType = isRepeat ? GpKeyboardInputEventTypes::kAutoChar : GpKeyboardInputEventTypes::kDownChar;
				GpKeyboardInputEvent::KeyUnion key;
				key.m_asciiChar = static_cast<char>(wParam);
				PostKeyboardEvent(eventQueue, keyEventType, GpKeyIDSubsets::kASCII, key, (lParam & 0xffff));
			}
			break;
		case WM_UNICHAR:
			{
				bool isRepeat = ((lParam & 0x4000000) != 0);
				const GpKeyboardInputEventType_t keyEventType = isRepeat ? GpKeyboardInputEventTypes::kAutoChar : GpKeyboardInputEventTypes::kDownChar;
				GpKeyboardInputEvent::KeyUnion key;
				key.m_unicodeChar = static_cast<uint32_t>(wParam);
				PostKeyboardEvent(eventQueue, keyEventType, GpKeyIDSubsets::kUnicode, key, (lParam & 0xffff));
			}
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
	g_gpGlobalConfig.m_audioDriverType = EGpAudioDriverType_XAudio2;

	EGpInputDriverType inputDrivers[] =
	{
		EGpInputDriverType_XInput
	};

	g_gpGlobalConfig.m_inputDriverTypes = inputDrivers;
	g_gpGlobalConfig.m_numInputDrivers = sizeof(inputDrivers) / sizeof(inputDrivers[0]);

	g_gpGlobalConfig.m_osGlobals = &g_gpWindowsGlobals;

	GpDisplayDriverFactory::RegisterDisplayDriverFactory(EGpDisplayDriverType_D3D11, GpDriver_CreateDisplayDriver_D3D11);
	GpAudioDriverFactory::RegisterAudioDriverFactory(EGpAudioDriverType_XAudio2, GpDriver_CreateAudioDriver_XAudio2);
	GpInputDriverFactory::RegisterInputDriverFactory(EGpInputDriverType_XInput, GpDriver_CreateInputDriver_XInput);

	return GpMain::Run();
}
