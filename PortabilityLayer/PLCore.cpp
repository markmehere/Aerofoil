#include "PLCore.h"

#include "PLArrayView.h"
#include "PLApplication.h"
#include "PLPasStr.h"
#include "PLKeyEncoding.h"
#include "PLQDraw.h"

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
#include "IGpCursor.h"
#include "IGpDisplayDriver.h"
#include "IGpThreadRelay.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "MacFileInfo.h"
#include "MacRomanConversion.h"
#include "MemoryManager.h"
#include "MenuManager.h"
#include "MemReaderStream.h"
#include "MMHandleBlock.h"
#include "RenderedFont.h"
#include "ResTypeID.h"
#include "RandomNumberGenerator.h"
#include "PLArrayViewIterator.h"
#include "PLBigEndian.h"
#include "PLEventQueue.h"
#include "PLKeyEncoding.h"
#include "PLSysCalls.h"
#include "PLTimeTaggedVOSEvent.h"
#include "PLWidgets.h"
#include "QDManager.h"
#include "Vec2i.h"
#include "WindowDef.h"
#include "WindowManager.h"

#include <assert.h>
#include <algorithm>

class PLMainThreadRelay final : public IGpThreadRelay
{
public:
	void Invoke(Callback_t callback, void *context) const override;

	static PLMainThreadRelay *GetInstance();

private:
	static PLMainThreadRelay ms_instance;
};

void PLMainThreadRelay::Invoke(Callback_t callback, void *context) const
{
	PLSysCalls::RunOnVOSThread(callback, context);
}

PLMainThreadRelay *PLMainThreadRelay::GetInstance()
{
	return &ms_instance;
}

PLMainThreadRelay PLMainThreadRelay::ms_instance;

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

Point BEPoint::ToPoint() const
{
	Point point;
	point.h = this->h;
	point.v = this->v;

	return point;
}

void HideCursor()
{
	PortabilityLayer::HostDisplayDriver::GetInstance()->SetStandardCursor(EGpStandardCursors::kHidden);
}

void Delay(int ticks, UInt32 *endTickCount)
{
	PLSysCalls::Sleep(ticks);

	if (endTickCount)
		*endTickCount = PortabilityLayer::DisplayDeviceManager::GetInstance()->GetTickCount();
}

void ForceSyncFrame()
{
	PLSysCalls::ForceSyncFrame();
}

short FindWindow(Point point, WindowPtr *window)
{
	short part = 0;
	PortabilityLayer::WindowManager::GetInstance()->FindWindow(point, window, &part);

	return part;
}

bool TrackGoAway(WindowPtr window, Point point)
{
	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();

	return wm->HandleCloseBoxClick(window, point);
}

PortabilityLayer::Vec2i TrackResize(WindowPtr window, Point start, uint16_t minWidth, uint16_t minHeight, Rect *size)
{
	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();

	const Rect baseRect = window->GetSurfaceRect();

	const PortabilityLayer::Vec2i baseSize = PortabilityLayer::Vec2i(baseRect.Width(), baseRect.Height());
	const PortabilityLayer::Vec2i basePoint = PortabilityLayer::Vec2i(start.h, start.v);
	PortabilityLayer::Vec2i currentTarget = baseSize;

	wm->SetResizeInProgress(window, baseSize);

	for (;;)
	{
		TimeTaggedVOSEvent evt;
		if (WaitForEvent(&evt, 1))
		{
			if (evt.m_vosEvent.m_eventType == GpVOSEventTypes::kMouseInput)
			{
				const GpMouseInputEvent &mouseEvt = evt.m_vosEvent.m_event.m_mouseInputEvent;

				const PortabilityLayer::Vec2i nextPoint = PortabilityLayer::Vec2i(mouseEvt.m_x, mouseEvt.m_y);
				PortabilityLayer::Vec2i requestedSize = baseSize + nextPoint - basePoint;

				if (requestedSize.m_x < static_cast<int32_t>(minWidth))
					requestedSize.m_x = minWidth;

				if (requestedSize.m_y < static_cast<int32_t>(minHeight))
					requestedSize.m_y = minHeight;

				if (requestedSize != currentTarget)
				{
					currentTarget = requestedSize;
					wm->SetResizeInProgress(window, currentTarget);
				}

				if (mouseEvt.m_button == GpMouseButtons::kLeft && mouseEvt.m_eventType == GpMouseEventTypes::kUp)
				{
					wm->ClearResizeInProgress();
					return requestedSize;
				}
			}
		}
	}

	return PortabilityLayer::Vec2i(0, 0);
}

WindowPtr GetNewCWindow(int resID, void *storage, WindowPtr behind)
{
	Handle windowResource = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('WIND', resID);

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

long MenuSelect(Point point)
{
	int16_t menuID = 0;
	uint16_t menuItem = 0;

	PortabilityLayer::MenuManager::GetInstance()->MenuSelect(PortabilityLayer::Vec2i(point.h, point.v), &menuID, &menuItem);

	return (static_cast<int32_t>(menuID) << 16) | (static_cast<int32_t>(menuItem));
}

long MenuKey(intptr_t charCode)
{
	if (PL_KEY_GET_EVENT_TYPE(charCode) != KeyEventType::KeyEventType_ASCII)
		return 0;

	const uint8_t asciiChar = PL_KEY_GET_VALUE(charCode);

	uint16_t menuID;
	uint16_t itemID;
	if (PortabilityLayer::MenuManager::GetInstance()->FindMenuShortcut(menuID, itemID, asciiChar))
		return (menuID << 16) | (itemID + 1);

	return 0;
}

long TickCount()
{
	return PortabilityLayer::DisplayDeviceManager::GetInstance()->GetTickCount();
}

short LoWord(Int32 v)
{
	return ((v ^ 0x8000) & 0xffff) - 0x8000;
}

short HiWord(Int32 v)
{
	return (((v >> 16) ^ 0x8000) & 0xffff) - 0x8000;
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
		outChar++;
	}

	const ptrdiff_t strLength = outChar - firstChar;
	const size_t numDigits = static_cast<size_t>(outChar - firstDigitLoc);
	const size_t halfNumDigits = numDigits / 2;

	// Swap to MSD order
	for (size_t i = 0; i < halfNumDigits; i++)
		std::swap(firstDigitLoc[i], *(outChar - 1 - i));

	str[0] = static_cast<uint8_t>(strLength);
}

PLError_t AEProcessAppleEvent(EventRecord *evt)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

void GetIndString(unsigned char *str, int stringsID, int fnameIndex)
{
	if (fnameIndex < 1)
	{
		str[0] = 0;
		return;
	}

	THandle<uint8_t> istrRes = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('STR#', stringsID).StaticCast<uint8_t>();
	if (istrRes && *istrRes)
	{
		const uint8_t *contentsBytes = *istrRes;
		const uint8_t *endPos = contentsBytes + istrRes.MMBlock()->m_size;
		const uint8_t *lineStart = contentsBytes;
		const uint8_t *lineEnd = contentsBytes;

		for (;;)
		{
			if (contentsBytes == endPos)
			{
				if (fnameIndex == 1)
				{
					lineEnd = contentsBytes;
					break;
				}
				else
				{
					str[0] = 0;
					return;
				}
			}

			const uint8_t lchar = contentsBytes[0];

			if (lchar == '\n')
			{
				if (fnameIndex == 1)
				{
					lineEnd = contentsBytes;
					break;
				}
				else
				{
					fnameIndex--;
					contentsBytes++;
					lineStart = lineEnd = contentsBytes;
				}
			}
			else
				contentsBytes++;
		}

		ptrdiff_t strLength = lineEnd - lineStart;
		if (strLength < 0 || strLength > 255)
			strLength = 255;

		str[0] = static_cast<uint8_t>(strLength);

		memcpy(str + 1, lineStart, strLength);
	}
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
			GpIOStream *stream = fs->OpenFile(dirID, filename, false, GpFileCreationDispositions::kOpenExisting);
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

void GetMouse(Window *window, Point *point)
{
	const PortabilityLayer::Vec2i mousePos = PortabilityLayer::InputManager::GetInstance()->GetMousePosition();
	const PortabilityLayer::Vec2i relativePos = mousePos - window->GetPosition();
	point->h = relativePos.m_x;
	point->v = relativePos.m_y;
}

Boolean StillDown()
{
	return PortabilityLayer::InputManager::GetInstance()->GetKeys()->m_mouse.Get(GpMouseButtons::kLeft);
}

Boolean WaitMouseUp()
{
	const Boolean isDown = StillDown();
	if (isDown)
		PLSysCalls::Sleep(1);

	return isDown;
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

void FlushEvents()
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

Handle NewHandle(size_t size)
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

PLError_t SetHandleSize(Handle hdl, size_t newSize)
{
	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();
	if (!mm->ResizeHandle(hdl.MMBlock(), newSize))
		return PLErrors::kOutOfMemory;

	return PLErrors::kNone;
}

void *NewPtr(size_t size)
{
	return PortabilityLayer::MemoryManager::GetInstance()->Alloc(size);
}

void *NewPtrClear(size_t size)
{
	void *data = NewPtr(size);
	if (data != nullptr && size != 0)
		memset(data, 0, size);

	return data;
}

void DisposePtr(void *ptr)
{
	PortabilityLayer::MemoryManager::GetInstance()->Release(ptr);
}

bool WaitForEvent(TimeTaggedVOSEvent *eventOut, uint32_t ticks)
{
	for (;;)
	{
		if (PortabilityLayer::EventQueue::GetInstance()->Dequeue(eventOut))
			return true;

		if (ticks == 0)
			break;

		Delay(1, nullptr);

		ticks--;
	}

	return false;
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
	PortabilityLayer::QDManager::GetInstance()->Init();
	PortabilityLayer::MenuManager::GetInstance()->Init();

	PortabilityLayer::HostFileSystem::GetInstance()->SetMainThreadRelay(PLMainThreadRelay::GetInstance());
}

WindowPtr PL_GetPutInFrontWindowPtr()
{
	return PortabilityLayer::WindowManager::GetInstance()->GetPutInFrontSentinel();
}

Window::Window()
	: m_surface(PortabilityLayer::QDPortType_Window)
	, m_wmX(0)
	, m_wmY(0)
	, m_widgets(nullptr)
	, m_numWidgets(0)
	, m_widgetWithFocus(nullptr)
	, m_numTickReceivingWidgets(0)
{
}

Window::~Window()
{
	if (m_widgets)
	{
		for (size_t i = 0; i < m_numWidgets; i++)
			m_widgets[i]->Destroy();

		PortabilityLayer::MemoryManager::GetInstance()->Release(m_widgets);
	}
}

DrawSurface *Window::GetDrawSurface() const
{
	return const_cast<DrawSurface*>(&m_surface);
}

Point Window::TouchToLocal(const GpTouchInputEvent &evt) const
{
	return Point::Create(evt.m_x - m_wmX, evt.m_y - m_wmY);
}

Point Window::MouseToLocal(const GpMouseInputEvent &evt) const
{
	return Point::Create(evt.m_x - m_wmX, evt.m_y - m_wmY);
}

Point Window::GetTopLeftCoord() const
{
	const PortabilityLayer::Vec2i position = GetPosition();
	return Point::Create(position.m_x, position.m_y);
}

Rect Window::GetSurfaceRect() const
{
	return m_surface.m_port.GetRect();
}

void Window::SetPosition(const PortabilityLayer::Vec2i &pos)
{
	m_wmX = pos.m_x;
	m_wmY = pos.m_y;
}

PortabilityLayer::Vec2i Window::GetPosition() const
{
	return PortabilityLayer::Vec2i(m_wmX, m_wmY);
}

bool Window::AddWidget(PortabilityLayer::Widget *widget)
{
	if (m_widgets == nullptr)
	{
		m_widgets = static_cast<PortabilityLayer::Widget**>(PortabilityLayer::MemoryManager::GetInstance()->Alloc(sizeof(PortabilityLayer::Widget *)));
		if (m_widgets == nullptr)
			return false;
	}
	else
	{
		void *newBuffer = PortabilityLayer::MemoryManager::GetInstance()->Realloc(m_widgets, (m_numWidgets + 1) * sizeof(PortabilityLayer::Widget*));
		if (newBuffer == nullptr)
			return false;
		m_widgets = static_cast<PortabilityLayer::Widget**>(newBuffer);
	}

	m_widgets[m_numWidgets++] = widget;

	if (widget->HandlesTickEvents())
		m_numTickReceivingWidgets++;

	return true;
}

ArrayView<PortabilityLayer::Widget*> Window::GetWidgets() const
{
	return ArrayView<PortabilityLayer::Widget*>(m_widgets, m_numWidgets);
}

PortabilityLayer::Widget* Window::GetWidgetById() const
{
	//for (size_t i = 0; i < m_numWidgets; i++)
	//	m_widgets[i]->get
	//return ArrayView<PortabilityLayer::Widget*>(m_widgets, m_numWidgets);
	return nullptr;
}

bool Window::ReplaceWidget(PortabilityLayer::Widget *oldWidget, PortabilityLayer::Widget *newWidget)
{
	for (size_t i = 0; i < m_numWidgets; i++)
	{
		if (m_widgets[i] == oldWidget)
		{
			assert(newWidget->GetWindow() == nullptr);

			oldWidget->Destroy();
			m_widgets[i] = newWidget;
			newWidget->SetWindow(this);

			newWidget->DrawControl(&m_surface);
			m_surface.m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);

			return true;
		}
	}

	return false;
}


void Window::FocusWidget(PortabilityLayer::Widget *widget)
{
	if (m_widgetWithFocus != widget)
	{
		assert(widget->GetWindow() == this);

		if (m_widgetWithFocus)
			m_widgetWithFocus->LoseFocus();

		m_widgetWithFocus = widget;

		widget->GainFocus();
	}
}

PortabilityLayer::Widget *Window::GetWidgetWithFocus() const
{
	return m_widgetWithFocus;
}

void Window::DrawControls()
{
	DrawSurface *surface = GetDrawSurface();

	for (size_t i = 0; i < m_numWidgets; i++)
	{
		PortabilityLayer::Widget *widget = m_widgets[i];
		if (widget->IsVisible())
			widget->DrawControl(surface);
	}
}

bool Window::IsHandlingTickEvents()
{
	return m_numTickReceivingWidgets > 0;
}

void Window::OnTick()
{
	if (m_numTickReceivingWidgets == 0)
		return;

	for (size_t i = 0; i < m_numWidgets; i++)
		m_widgets[i]->OnTick();
}

DrawSurface *Window::GetChromeSurface(WindowChromeSide_t aChromeSide) const
{
	return const_cast<DrawSurface*>(m_chromeSurfaces + aChromeSide);
}
