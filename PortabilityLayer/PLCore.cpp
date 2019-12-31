#include "PLCore.h"
#include "PLApplication.h"
#include "PLPasStr.h"
#include "PLKeyEncoding.h"
#include "PLQDraw.h"

#include "AEManager.h"
#include "DisplayDeviceManager.h"
#include "FileManager.h"
#include "FilePermission.h"
#include "FontFamily.h"
#include "FontManager.h"
#include "GpVOSEvent.h"
#include "HostDirectoryCursor.h"
#include "HostFileSystem.h"
#include "HostSuspendCallArgument.h"
#include "HostSuspendHook.h"
#include "HostDisplayDriver.h"
#include "HostSystemServices.h"
#include "HostVOSEventQueue.h"
#include "IGpColorCursor.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "MacFileInfo.h"
#include "MacRoman.h"
#include "MemoryManager.h"
#include "MenuManager.h"
#include "MemReaderStream.h"
#include "MMHandleBlock.h"
#include "RenderedFont.h"
#include "ResTypeID.h"
#include "RandomNumberGenerator.h"
#include "PLBigEndian.h"
#include "PLEventQueue.h"
#include "PLKeyEncoding.h"
#include "QDManager.h"
#include "Vec2i.h"
#include "WindowDef.h"
#include "WindowManager.h"

#include <assert.h>
#include <algorithm>

static bool ConvertFilenameToSafePStr(const char *str, uint8_t *pstr)
{
	const char *strBase = str;
	while (*str)
	{
		const char c = *str++;

		if (c == '.' || c == ' ' || c == '_' || c == '\'' || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
			continue;
		else
			return false;
	}

	ptrdiff_t len = str - strBase;
	if (len > 31)
		return false;

	memcpy(pstr + 1, strBase, static_cast<size_t>(len));
	pstr[0] = static_cast<uint8_t>(len);

	return true;
}

static void TranslateMouseInputEvent(const GpMouseInputEvent &vosEvent, PortabilityLayer::EventQueue *queue)
{
	if (vosEvent.m_button == GpMouseButtons::kLeft)
	{
		if (vosEvent.m_eventType == GpMouseEventTypes::kDown)
		{
			if (EventRecord *evt = queue->Enqueue())
			{
				evt->what = mouseDown;
				evt->where.h = std::min<int32_t>(INT16_MAX, std::max<int32_t>(INT16_MIN, vosEvent.m_x));
				evt->where.v = std::min<int32_t>(INT16_MAX, std::max<int32_t>(INT16_MIN, vosEvent.m_y));
			}
		}
		else if (vosEvent.m_eventType == GpMouseEventTypes::kUp)
		{
			if (EventRecord *evt = queue->Enqueue())
			{
				evt->what = mouseUp;
				evt->where.h = std::min<int32_t>(INT16_MAX, std::max<int32_t>(INT16_MIN, vosEvent.m_x));
				evt->where.v = std::min<int32_t>(INT16_MAX, std::max<int32_t>(INT16_MIN, vosEvent.m_y));
			}
		}
	}
	else if (vosEvent.m_eventType == GpMouseEventTypes::kMove)
	{
		if (EventRecord *evt = queue->Enqueue())
		{
			evt->what = mouseMove;
			evt->where.h = std::min<int32_t>(INT16_MAX, std::max<int32_t>(INT16_MIN, vosEvent.m_x));
			evt->where.v = std::min<int32_t>(INT16_MAX, std::max<int32_t>(INT16_MIN, vosEvent.m_y));
		}
	}
}

static void TranslateGamepadInputEvent(const GpGamepadInputEvent &vosEvent, PortabilityLayer::EventQueue *queue)
{
	PortabilityLayer::InputManager *inputManager = PortabilityLayer::InputManager::GetInstance();

	inputManager->ApplyGamepadEvent(vosEvent);

	PL_DEAD(queue);
}

static void TranslateKeyboardInputEvent(const GpKeyboardInputEvent &vosEvent, PortabilityLayer::EventQueue *queue)
{
	PL_STATIC_ASSERT((1 << PL_INPUT_PLAYER_INDEX_BITS) >= PL_INPUT_MAX_PLAYERS);
	PL_STATIC_ASSERT((1 << PL_INPUT_TYPE_CODE_BITS) >= KeyEventType_Count);

	PortabilityLayer::InputManager *inputManager = PortabilityLayer::InputManager::GetInstance();

	if (vosEvent.m_eventType == GpKeyboardInputEventTypes::kUp || vosEvent.m_eventType == GpKeyboardInputEventTypes::kDown)
		inputManager->ApplyKeyboardEvent(vosEvent);

	intptr_t msg = 0;

	switch (vosEvent.m_keyIDSubset)
	{
	case GpKeyIDSubsets::kASCII:
		msg = PL_KEY_ASCII(vosEvent.m_key.m_asciiChar);
		break;
	case GpKeyIDSubsets::kFKey:
		msg = PL_KEY_FKEY(vosEvent.m_key.m_fKey);
		break;
	case GpKeyIDSubsets::kNumPadNumber:
		msg = PL_KEY_NUMPAD_NUMBER(vosEvent.m_key.m_numPadNumber);
		break;
	case GpKeyIDSubsets::kSpecial:
		msg = PL_KEY_SPECIAL_ENCODE(vosEvent.m_key.m_specialKey);
		break;
	case GpKeyIDSubsets::kNumPadSpecial:
		msg = PL_KEY_NUMPAD_SPECIAL_ENCODE(vosEvent.m_key.m_numPadSpecialKey);
		break;
	case GpKeyIDSubsets::kUnicode:
		for (int i = 128; i < 256; i++)
		{
			if (PortabilityLayer::MacRoman::g_toUnicode[i] == vosEvent.m_key.m_unicodeChar)
			{
				msg = PL_KEY_MACROMAN(i);
				break;
			}
		}
		break;
	case GpKeyIDSubsets::kGamepadButton:
		msg = PL_KEY_GAMEPAD_BUTTON_ENCODE(vosEvent.m_key.m_gamepadKey.m_button, vosEvent.m_key.m_gamepadKey.m_player);
		break;
	default:
		PL_NotYetImplemented();
	}

	if (msg == 0)
		return;

	EventRecord *evt = queue->Enqueue();

	switch (vosEvent.m_eventType)
	{
	case GpKeyboardInputEventTypes::kUp:
		evt->what = keyUp;
		break;
	case GpKeyboardInputEventTypes::kDown:
		evt->what = keyUp;
		break;
	case GpKeyboardInputEventTypes::kAuto:
		evt->what = autoKey;
		break;
	};

	evt->message = msg;

	PL_NotYetImplemented_TODO("Modifiers");
}

static void TranslateVOSEvent(const GpVOSEvent *vosEvent, PortabilityLayer::EventQueue *queue)
{
	switch (vosEvent->m_eventType)
	{
	case GpVOSEventTypes::kMouseInput:
		TranslateMouseInputEvent(vosEvent->m_event.m_mouseInputEvent, queue);
		break;
	case GpVOSEventTypes::kKeyboardInput:
		TranslateKeyboardInputEvent(vosEvent->m_event.m_keyboardInputEvent, queue);
		break;
	case GpVOSEventTypes::kGamepadInput:
		TranslateGamepadInputEvent(vosEvent->m_event.m_gamepadInputEvent, queue);
		break;
	}
}

static void ImportVOSEvents()
{
	PortabilityLayer::EventQueue *plQueue = PortabilityLayer::EventQueue::GetInstance();

	PortabilityLayer::HostVOSEventQueue *evtQueue = PortabilityLayer::HostVOSEventQueue::GetInstance();
	while (const GpVOSEvent *evt = evtQueue->GetNext())
	{
		TranslateVOSEvent(evt, plQueue);
		evtQueue->DischargeOne();
	}
}

void InitCursor()
{
	PortabilityLayer::HostDisplayDriver::GetInstance()->SetStandardCursor(EGpStandardCursors::kArrow);
}

Rect BERect::ToRect() const
{
	Rect rect;
	rect.top = this->top;
	rect.bottom = this->bottom;
	rect.left = this->left;
	rect.right = this->right;

	return rect;
}

CursHandle GetCursor(int cursorID)
{
	return GetResource('CURS', cursorID).ReinterpretCast<Cursor>();
}

CCrsrHandle GetCCursor(int cursorID)
{
	PortabilityLayer::HostDisplayDriver *driver = PortabilityLayer::HostDisplayDriver::GetInstance();
	IGpColorCursor *hwCursor = driver->LoadColorCursor(cursorID);

	if (!hwCursor)
		return nullptr;

	CCrsrHandle hdl = PortabilityLayer::MemoryManager::GetInstance()->NewHandle<CCursor>();
	if (!hdl)
	{
		hwCursor->Destroy();
		return nullptr;
	}

	CCursor *ccursor = *hdl;
	ccursor->hwCursor = hwCursor;

	return hdl;
}

void SetCCursor(CCrsrHandle handle)
{
	assert(handle);
	PortabilityLayer::HostDisplayDriver::GetInstance()->SetColorCursor((*handle)->hwCursor);
}

void HideCursor()
{
	PortabilityLayer::HostDisplayDriver::GetInstance()->SetStandardCursor(EGpStandardCursors::kHidden);
}

void SetCursor(CursPtr cursor)
{
	PL_NotYetImplemented();
}

void SetBuiltinCursor(int builtinCursor)
{
	PL_NotYetImplemented();
}

void DisposeCCursor(CCrsrHandle handle)
{
	(*handle)->hwCursor->Destroy();

	PortabilityLayer::MemoryManager::GetInstance()->ReleaseHandle(handle.MMBlock());
}

void Delay(int ticks, UInt32 *endTickCount)
{
	if (ticks > 0)
	{
		PortabilityLayer::HostSuspendCallArgument args[1];
		args[0].m_uint = static_cast<uint32_t>(ticks);

		PortabilityLayer::SuspendApplication(PortabilityLayer::HostSuspendCallID_Delay, args, nullptr);

		ImportVOSEvents();
	}

	if (endTickCount)
		*endTickCount = PortabilityLayer::DisplayDeviceManager::GetInstance()->GetTickCount();
}

short Alert(int dialogID, void *unknown)
{
	PL_NotYetImplemented();
	return 0;
}

Handle GetResource(int32_t resType, int id)
{
	return PortabilityLayer::ResourceManager::GetInstance()->GetResource(PortabilityLayer::ResTypeID(resType), id);
}

Handle GetResource(const char(&resTypeLiteral)[5], int id)
{
	PL_NotYetImplemented();
	return nullptr;
}

short FindWindow(Point point, WindowPtr *window)
{
	short part = 0;
	PortabilityLayer::WindowManager::GetInstance()->FindWindow(point, window, &part);

	return part;
}

void DragWindow(WindowPtr window, Point start, Rect *bounds)
{
	PL_NotYetImplemented();
}

void SendBehind(WindowPtr window, WindowPtr behind)
{
	PL_NotYetImplemented();
}

void BringToFront(WindowPtr window)
{
	PL_NotYetImplemented();
}

void ShowHide(WindowPtr window, Boolean hide)
{
	PL_NotYetImplemented();
}

bool TrackGoAway(WindowPtr window, Point point)
{
	PL_NotYetImplemented();
	return false;
}

Int32 GrowWindow(WindowPtr window, Point start, Rect *size)
{
	PL_NotYetImplemented();
	return 0;
}

bool TrackBox(WindowPtr window, Point point, int part)
{
	PL_NotYetImplemented();
	return false;
}

void ZoomWindow(WindowPtr window, int part, bool bringToFront)
{
	PL_NotYetImplemented();
}

void HiliteWindow(WindowPtr window, bool highlighted)
{
	PL_NotYetImplemented();
}

void DisposeWindow(WindowPtr window)
{
	PL_NotYetImplemented();
}

void GetWindowBounds(WindowPtr window, WindowRegionType windowRegion, Rect *rect)
{
	if (windowRegion == kWindowContentRgn)
		*rect = window->m_graf.m_port.GetRect();
	else
	{
		PL_NotYetImplemented();
	}
}

WindowPtr GetNewCWindow(int resID, void *storage, WindowPtr behind)
{
	Handle windowResource = GetResource('WIND', resID);

	if (!windowResource)
		return nullptr;

	long resSize = GetHandleSize(windowResource);

	PortabilityLayer::MemReaderStream stream(*windowResource, resSize);

	PortabilityLayer::WindowDef def;
	if (!def.Deserialize(&stream))
		return nullptr;

	WindowPtr window = PortabilityLayer::WindowManager::GetInstance()->CreateWindow(def);
	if (window)
		PortabilityLayer::WindowManager::GetInstance()->PutWindowBehind(window, behind);

	return window;
}

WindowPtr NewCWindow(void *storage, const Rect *bounds, const PLPasStr &title, Boolean visible, int wdef, WindowPtr behind, Boolean hasCloseBox, long userdata)
{
	PL_NotYetImplemented();
	return nullptr;
}

WindowPtr NewWindow(void *storage, const Rect *bounds, const PLPasStr &title, Boolean visible, int wdef, WindowPtr behind, Boolean hasCloseBox, long userdata)
{
	PL_NotYetImplemented();
	return nullptr;
}

void SizeWindow(WindowPtr window, int width, int height, Boolean addToUpdateRegion)
{
	PortabilityLayer::WindowManager::GetInstance()->ResizeWindow(window, width, height);
}

void MoveWindow(WindowPtr window, int x, int y, Boolean moveToFront)
{
	PortabilityLayer::WindowManager *windowManager = PortabilityLayer::WindowManager::GetInstance();

	windowManager->MoveWindow(window, x, y);
	if (moveToFront != 0)
		windowManager->PutWindowBehind(window, windowManager->GetPutInFrontSentinel());
}

void ShowWindow(WindowPtr window)
{
	PortabilityLayer::WindowManager::GetInstance()->ShowWindow(window);
}

void SetWTitle(WindowPtr window, const PLPasStr &title)
{
	PL_NotYetImplemented();
}

bool PeekNextEvent(int32_t eventMask, EventRecord *event)
{
	assert(eventMask == everyEvent);	// We don't support other use cases

	PortabilityLayer::EventQueue *queue = PortabilityLayer::EventQueue::GetInstance();
	const EventRecord *record = queue->Peek();

	if (record)
	{
		*event = *record;
		return PL_TRUE;
	}
	else
		return PL_FALSE;
}

bool GetNextEvent(int32_t eventMask, EventRecord *event)
{
	assert(eventMask == everyEvent);	// We don't support other use cases

	PortabilityLayer::EventQueue *queue = PortabilityLayer::EventQueue::GetInstance();
	return queue->Dequeue(event) ? PL_TRUE : PL_FALSE;
}

long MenuSelect(Point point)
{
	int16_t menuID = 0;
	uint16_t menuItem = 0;

	PortabilityLayer::MenuManager::GetInstance()->MenuSelect(PortabilityLayer::Vec2i(point.h, point.v), &menuID, &menuItem);

	return (static_cast<int32_t>(menuID) << 16) | (static_cast<int32_t>(menuItem));
}

long MenuKey(int charCode)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

long TickCount()
{
	return PortabilityLayer::DisplayDeviceManager::GetInstance()->GetTickCount();
}

void GetKeys(KeyMap &keyMap)
{
	PortabilityLayer::InputManager::GetInstance()->GetKeys(keyMap);
}

short LoWord(Int32 v)
{
	return ((v ^ 0x8000) & 0xffff) - 0x8000;
}

short HiWord(Int32 v)
{
	return (((v >> 16) ^ 0x8000) & 0xffff) - 0x8000;
}

static bool BitTestEitherSpecial(const KeyMap &keyMap, int eitherSpecial)
{
	switch (eitherSpecial)
	{
	case KeyEventEitherSpecialCategories::kAlt:
		return keyMap.m_special.Get(GpKeySpecials::kLeftAlt) || keyMap.m_special.Get(GpKeySpecials::kRightAlt);
	case KeyEventEitherSpecialCategories::kShift:
		return keyMap.m_special.Get(GpKeySpecials::kLeftShift) || keyMap.m_special.Get(GpKeySpecials::kRightShift);
	case KeyEventEitherSpecialCategories::kControl:
		return keyMap.m_special.Get(GpKeySpecials::kLeftCtrl) || keyMap.m_special.Get(GpKeySpecials::kRightCtrl);
	default:
		assert(false);
		return false;
	}
}

bool BitTst(const KeyMap &keyMap, int encodedKey)
{
	const KeyEventType evtType = PL_KEY_GET_EVENT_TYPE(encodedKey);
	const int evtValue = PL_KEY_GET_VALUE(encodedKey);

	switch (evtType)
	{
	case KeyEventType_Special:
		return keyMap.m_special.Get(evtValue);
	case KeyEventType_ASCII:
		return keyMap.m_ascii.Get(evtValue);
	case KeyEventType_MacRoman:
		assert(evtValue >= 128 && evtValue < 256);
		return keyMap.m_macRoman.Get(evtValue - 128);
	case KeyEventType_NumPadNumber:
		return keyMap.m_numPadNumber.Get(evtValue);
	case KeyEventType_NumPadSpecial:
		return keyMap.m_numPadSpecial.Get(evtValue);
	case KeyEventType_FKey:
		assert(evtValue >= 1 && evtValue <= GpFKeyMaximumInclusive);
		return keyMap.m_fKey.Get(evtValue - 1);
	case KeyEventType_EitherSpecial:
		return BitTestEitherSpecial(keyMap, evtValue);
	case KeyEventType_GamepadButton:
		{
			unsigned int playerNum = evtValue & ((1 << PL_INPUT_PLAYER_INDEX_BITS) - 1);
			assert(playerNum < PL_INPUT_MAX_PLAYERS);
			unsigned int button = evtValue >> PL_INPUT_PLAYER_INDEX_BITS;

			return keyMap.m_gamepadButtons[playerNum].Get(button);
		}
		break;
	default:
		assert(false);
		return false;
	}
}

void NumToString(long number, unsigned char *str)
{
	unsigned char *firstChar = str + 1;
	unsigned char *outChar = firstChar;
	unsigned char *firstDigitLoc = firstChar;

	int firstDigit = 0;
	if (number == LONG_MIN)
	{
		*outChar = '-';
		outChar++;
		firstDigitLoc++;

		// Non-negatable number
		const long halfAbsNumberDiv2 = (number >> 1) & ~(static_cast<long>(1) << (sizeof(long) * 8 - 1));
		const long halfAbsNumberDiv5 = halfAbsNumberDiv2 / 5;
		const long halfAbsNumberMod5 = halfAbsNumberDiv2 % 5;

		firstDigit = static_cast<int>(halfAbsNumberMod5) * 2;
		number = halfAbsNumberDiv5;
	}
	else if (number < 0)
	{
		*outChar = '-';
		outChar++;
		firstDigitLoc++;

		number = -number;
		firstDigit = static_cast<int>(number % 10);
		number /= 10;
	}
	else
	{
		firstDigit = static_cast<int>(number % 10);
		number /= 10;
	}

	*outChar = '0' + firstDigit;
	outChar++;

	while (number > 0)
	{
		const int digit = number % 10;
		number /= 10;

		*outChar = '0' + digit;
		*outChar++;
	}

	const ptrdiff_t strLength = outChar - firstChar;
	const size_t numDigits = static_cast<size_t>(outChar - firstDigitLoc);
	const size_t halfNumDigits = numDigits / 2;

	// Swap to MSD order
	for (size_t i = 0; i < halfNumDigits; i++)
		std::swap(firstDigitLoc[i], *(outChar - 1 - i));

	str[0] = static_cast<uint8_t>(strLength);
}

void ParamText(const PLPasStr &title, const PLPasStr &a, const PLPasStr &b, const PLPasStr &c)
{
	PL_NotYetImplemented();
}


UInt32 FreeMem()
{
	PL_NotYetImplemented_Minor();
	return 256 * 1024 * 1024;
}

PLError_t AEProcessAppleEvent(EventRecord *evt)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

void GetIndString(unsigned char *str, int stringsID, int fnameIndex)
{
	if (fnameIndex < 1)
		return;

	THandle<uint8_t> istrRes = PortabilityLayer::ResourceManager::GetInstance()->GetResource('STR#', stringsID).StaticCast<uint8_t>();
	if (istrRes && *istrRes)
	{
		const uint8_t *contentsBytes = *istrRes;
		const BEUInt16_t *pArraySize = reinterpret_cast<const BEUInt16_t*>(contentsBytes);

		const uint16_t arraySize = *pArraySize;

		if (fnameIndex > static_cast<int>(arraySize))
			return;

		const uint8_t *strStart = contentsBytes + 2;
		for (int i = 1; i < fnameIndex; i++)
			strStart += (*strStart) + 1;

		str[0] = strStart[0];
		memcpy(str + 1, strStart + 1, *strStart);
	}
}

PLError_t PBDirCreate(HFileParam *fileParam, bool asynchronous)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}


VFileSpec MakeVFileSpec(PortabilityLayer::VirtualDirectory_t dir, const PLPasStr &fileName)
{
	VFileSpec spec;

	assert(fileName.Length() < sizeof(spec.m_name));

	spec.m_dir = dir;
	spec.m_name[0] = static_cast<uint8_t>(fileName.Length());
	memcpy(spec.m_name + 1, fileName.UChars(), fileName.Length());

	return spec;
}

PLError_t FSpGetFInfo(const VFileSpec &spec, VFileInfo &finfo)
{
	PortabilityLayer::MacFileProperties mfp;
	if (!PortabilityLayer::FileManager::GetInstance()->ReadFileProperties(spec.m_dir, spec.m_name, mfp))
		return PLErrors::kFileNotFound;

	finfo.m_type = PortabilityLayer::ResTypeID(mfp.m_fileType);
	finfo.m_creator = PortabilityLayer::ResTypeID(mfp.m_fileCreator);

	return PLErrors::kNone;
}

DirectoryFileListEntry *GetDirectoryFiles(PortabilityLayer::VirtualDirectory_t dirID)
{
	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();
	PortabilityLayer::HostFileSystem *fs = PortabilityLayer::HostFileSystem::GetInstance();
	PortabilityLayer::HostDirectoryCursor *dirCursor = fs->ScanDirectory(dirID);

	DirectoryFileListEntry *firstDFL = nullptr;
	DirectoryFileListEntry *lastDFL = nullptr;

	if (!dirCursor)
		return nullptr;

	const char *filename;
	char fnCopy[256];
	while (dirCursor->GetNext(filename))
	{
		const size_t fnLen = strlen(filename);
		if (fnLen < 5 || fnLen > 255)
			continue;

		memcpy(fnCopy, filename, fnLen + 1);

		if (!strcmp(&filename[fnLen - 4], ".gpf"))
		{
			const size_t dotPos = fnLen - 4;
			PortabilityLayer::IOStream *stream = fs->OpenFile(dirID, filename, false, false);
			if (!stream)
				continue;

			PortabilityLayer::MacFileProperties mfp;
			PortabilityLayer::MacFilePropertiesSerialized mfs;

			const size_t gpfSize = stream->Read(mfs.m_data, PortabilityLayer::MacFilePropertiesSerialized::kSize);
			stream->Close();

			if (gpfSize != PortabilityLayer::MacFilePropertiesSerialized::kSize)
				continue;

			mfs.Deserialize(mfp);

			fnCopy[dotPos] = '\0';

			DirectoryFileListEntry tempDFL;
			tempDFL.finderInfo.fdType = PortabilityLayer::ResTypeIDCodec::Decode(mfp.m_fileType);
			tempDFL.finderInfo.fdCreator = PortabilityLayer::ResTypeIDCodec::Decode(mfp.m_fileCreator);
			tempDFL.nextEntry = nullptr;
			if (!ConvertFilenameToSafePStr(fnCopy, tempDFL.name))
				continue;

			DirectoryFileListEntry *dfl = static_cast<DirectoryFileListEntry*>(mm->Alloc(sizeof(DirectoryFileListEntry)));
			if (!dfl)
			{
				if (firstDFL)
					DisposeDirectoryFiles(firstDFL);

				return nullptr;
			}

			new (dfl) DirectoryFileListEntry(tempDFL);

			dfl->nextEntry = nullptr;

			if (lastDFL)
				lastDFL->nextEntry = dfl;
			else
				firstDFL = dfl;

			lastDFL = dfl;
		}
	}

	dirCursor->Destroy();

	return firstDFL;
}

void DisposeDirectoryFiles(DirectoryFileListEntry *firstDFL)
{
	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

	DirectoryFileListEntry *dfl = firstDFL;
	while (dfl)
	{
		DirectoryFileListEntry *nextDFL = dfl->nextEntry;
		mm->Release(dfl);
		dfl = nextDFL;
	}
}

short StringWidth(const PLPasStr &str)
{
	const PortabilityLayer::QDState *qdState = PortabilityLayer::QDManager::GetInstance()->GetState();
	PortabilityLayer::FontManager *fontManager = PortabilityLayer::FontManager::GetInstance();

	PortabilityLayer::FontFamily *fontFamily = qdState->m_fontFamily;

	if (!fontFamily)
		return 0;

	const int variationFlags = qdState->m_fontVariationFlags;
	const int fontSize = qdState->m_fontSize;

	PortabilityLayer::RenderedFont *rfont = fontManager->GetRenderedFontFromFamily(fontFamily, fontSize, variationFlags);
	if (!rfont)
		return 0;

	const size_t width = rfont->MeasureString(str.UChars(), str.Length());
	if (width > SHRT_MAX)
		return SHRT_MAX;

	return static_cast<short>(width);
}

void GetMouse(Point *point)
{
	PL_NotYetImplemented();
}

Boolean Button()
{
	PL_NotYetImplemented();
	return false;
}

Boolean StillDown()
{
	PL_NotYetImplemented();
	return false;
}

Boolean WaitMouseUp()
{
	PL_NotYetImplemented();
	return false;
}

void LocalToGlobal(Point *point)
{
	PL_NotYetImplemented();
}

void GlobalToLocal(Point *point)
{
	PL_NotYetImplemented();
}

short Random()
{
	// Should return with range -32767..32767
	uint32_t bits = PortabilityLayer::RandomNumberGenerator::GetInstance()->GetNextAndAdvance();
	uint16_t rWord = (bits & 0xffff);
	if (rWord == 0)
	{
		rWord = (bits >> 16) & 0xffff;
		if (rWord == 0)
			return 0;	// This should be extremely rare
	}

	return static_cast<short>(static_cast<int32_t>(rWord) - 0x8000);
}

void GetDateTime(UInt32 *dateTime)
{
	PL_NotYetImplemented();
}

void GetTime(DateTimeRec *dateTime)
{
	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;
	PortabilityLayer::HostSystemServices::GetInstance()->GetLocalDateTime(year, month, day, hour, minute, second);

	dateTime->month = month;
	dateTime->hour = hour;
	dateTime->minute = minute;
}

UInt32 GetDblTime()
{
	PL_NotYetImplemented_Minor();
	return 30;
}

void FlushEvents(int mask, int unknown)
{
	PortabilityLayer::EventQueue *queue = PortabilityLayer::EventQueue::GetInstance();

	while (queue->Dequeue(nullptr))
	{
	}
}

void ExitToShell()
{
	PL_NotYetImplemented();
}

void InvalWindowRect(WindowPtr window, const Rect *rect)
{
	PL_NotYetImplemented();
}

Handle NewHandle(Size size)
{
	PortabilityLayer::MMHandleBlock *hBlock = PortabilityLayer::MemoryManager::GetInstance()->AllocHandle(size);
	if (!hBlock)
		return nullptr;

	return &hBlock->m_contents;
}

long GetHandleSize(Handle handle)
{
	if (!handle)
		return 0;

	return handle.MMBlock()->m_size;
}

PLError_t PtrAndHand(const void *data, Handle handle, Size size)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

PLError_t SetHandleSize(Handle hdl, Size newSize)
{
	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();
	if (!mm->ResizeHandle(hdl.MMBlock(), newSize))
		return PLErrors::kOutOfMemory;

	return PLErrors::kNone;
}

void *NewPtr(Size size)
{
	return PortabilityLayer::MemoryManager::GetInstance()->Alloc(size);
}

void *NewPtrClear(Size size)
{
	void *data = NewPtr(size);
	if (data != nullptr && size != 0)
		memset(data, 0, size);

	return data;
}

void DisposePtr(void *ptr)
{
	PL_NotYetImplemented();
}

void BlockMove(const void *src, void *dest, Size size)
{
	memcpy(dest, src, size);
}

Boolean WaitNextEvent(int eventMask, EventRecord *eventOut, long sleep, void *unknown)
{
	for (;;)
	{
		Boolean hasEvent = GetNextEvent(eventMask, eventOut);
		if (hasEvent)
			return hasEvent;

		Delay(1, nullptr);

		if (sleep == 0)
			break;

		sleep--;
	}

	return PL_FALSE;
}

void DrawControls(WindowPtr window)
{
	PL_NotYetImplemented();
}

void DrawGrowIcon(WindowPtr window)
{
	PL_NotYetImplemented();
}

void DebugStr(const PLPasStr &str)
{
	PL_NotYetImplemented();
}

void PL_NotYetImplemented()
{
	assert(false);
}

void PL_NotYetImplemented_Minor()
{
}

void PL_NotYetImplemented_TODO(const char *category)
{
	(void)category;
}

void PL_Init()
{
	PortabilityLayer::FontManager::GetInstance()->Init();
	PortabilityLayer::MemoryManager::GetInstance()->Init();
	PortabilityLayer::ResourceManager::GetInstance()->Init();
	PortabilityLayer::DisplayDeviceManager::GetInstance()->Init();
	PortabilityLayer::AEManager::GetInstance()->Init();
	PortabilityLayer::QDManager::GetInstance()->Init();
	PortabilityLayer::MenuManager::GetInstance()->Init();
}

WindowPtr PL_GetPutInFrontWindowPtr()
{
	return PortabilityLayer::WindowManager::GetInstance()->GetPutInFrontSentinel();
}

Window::Window()
	: m_graf(PortabilityLayer::QDPortType_Window)
	, m_wmX(0)
	, m_wmY(0)
{
}

DrawSurface *Window::GetDrawSurface() const
{
	return const_cast<DrawSurface*>(&m_graf);
}
