#pragma once

#include "DataTypes.h"
#include "PLErrorCodes.h"
#include "SharedTypes.h"
#include "QDPort.h"
#include "QDGraf.h"
#include "ResTypeID.h"
#include "VirtualDirectory.h"
#include "PLHandle.h"

#ifdef _MSC_VER
#pragma warning(error:4311)	// Pointer truncation to int
#endif

template<class T>
class ArrayView;
struct IGpCursor;
struct GpVOSEvent;
struct GpMouseInputEvent;
struct TimeTaggedVOSEvent;


namespace PortabilityLayer
{
	struct MMHandleBlock;
	class IOStream;
	class Widget;
}

typedef uint8_t Boolean;
typedef uint8_t Byte;
typedef uint8_t UInt8;
typedef int16_t SInt16;
typedef int32_t Int32;
typedef uint32_t UInt32;

typedef size_t Size;

typedef unsigned char Str15[16];
typedef unsigned char Str31[32];
typedef unsigned char Str63[64];
typedef unsigned char Str27[28];
typedef unsigned char Str32[33];
typedef unsigned char Str255[256];
typedef unsigned char *StringPtr;

class PLPasStr;
struct DrawSurface;

struct Menu;

typedef void *Ptr;

#define PL_DEAD(n) ((void)0)

struct FinderInfoBlock
{
	Int32 fdType;
	Int32 fdCreator;
};

struct DirectoryFileListEntry
{
	FinderInfoBlock finderInfo;
	Str32 name;
	DirectoryFileListEntry *nextEntry;
};

struct Cursor
{
};

namespace WindowChromeSides
{
	enum WindowChromeSide
	{
		kTop,
		kLeft,
		kBottom,
		kRight,

		kCount
	};
}

typedef WindowChromeSides::WindowChromeSide WindowChromeSide_t;

struct Window
{
	Window();

	DrawSurface *GetDrawSurface() const;

	// Convenience method to convert a mouse event to local point
	Point MouseToLocal(const GpMouseInputEvent &evt) const;

	// Convenience method that returns a 16-bit precision X/Y
	Point TopLeftCoord() const;

	bool AddWidget(PortabilityLayer::Widget *widget);
	ArrayView<PortabilityLayer::Widget*> GetWidgets() const;
	PortabilityLayer::Widget* GetWidgetById() const;

	void FocusWidget(PortabilityLayer::Widget *widget);
	PortabilityLayer::Widget *GetWidgetWithFocus() const;

	void DrawControls();
	bool IsHandlingTickEvents();

	void OnTick();

	DrawSurface *GetChromeSurface(WindowChromeSide_t aChromeSide) const;

	DrawSurface m_surface;	// Must be the first item until the immediate mode draw API is completely removed

	// The port is always at 0,0
	// These are the WM coordinates
	int32_t m_wmX;
	int32_t m_wmY;

protected:
	~Window();

	DrawSurface m_chromeSurfaces[WindowChromeSides::kCount];

	PortabilityLayer::Widget **m_widgets;
	size_t m_numWidgets;
	size_t m_numTickReceivingWidgets;

	PortabilityLayer::Widget *m_widgetWithFocus;
};

struct DateTimeRec
{
	int month;
	int hour;
	int minute;
};

struct VersionRecord
{
	Byte majorVer;
	Byte minorVer;
	Byte stage;
	Byte rcVer;

	short countryCode;
	Str255 shortVersion;
	Str255 unused;
};

struct ColorSpec
{
	uint8_t r, g, b;
};

struct VFileSpec
{
	PortabilityLayer::VirtualDirectory_t m_dir;
	Str63 m_name;
};

struct VFileInfo
{
	PortabilityLayer::ResTypeID m_type;
	PortabilityLayer::ResTypeID m_creator;
};

struct HFileParam
{
	int ioVRefNum;
	long ioDirID;
	const unsigned char *ioNamePtr;
	void *ioCompletion;
};

struct EventRecord
{
	Point where;
	uint32_t when;
	intptr_t message;
	int what;
	int modifiers;
};

typedef Window *WindowPtr;
typedef Cursor *CursPtr;
typedef Menu *MenuPtr;
typedef VersionRecord *VersRecPtr;

typedef THandle<Cursor> CursHandle;
typedef THandle<Menu> MenuHandle;
typedef THandle<VersionRecord> VersRecHndl;

typedef WindowPtr WindowRef;	// wtf?

struct KeyDownStates;

enum RegionID
{
	inMenuBar = 1,
	inContent,
	inDrag,
	inGrow,
	inGoAway,
	inZoomIn,
	inZoomOut,
};

enum WindowRegionType
{
	kWindowContentRgn
};

enum EventCode
{
	mouseDown,
	mouseUp,
	mouseMove,
	keyDown,
	keyUp,
	autoKey,
	updateEvt,
	osEvt,
	kHighLevelEvent,
};

static const int everyEvent = -1;

static const int iBeamCursor = 1;
static const int watchCursor = 4;

static const Boolean TRUE = 1;
static const Boolean FALSE = 0;

#define nil nullptr

static const int nullEvent = 0;

//void FlushEvents(int eventMask, int stopMask);

void InitCursor();
CursHandle GetCursor(int cursorID);
void HideCursor();

void Delay(int ticks, UInt32 *endTickCount);

short FindWindow(Point point, WindowPtr *window);	// Translates global coordinates to window coordinates, returns a region ID
void BringToFront(WindowPtr window);
bool TrackGoAway(WindowPtr window, Point point);	// Returns true if the close box was actually clicked (?)
Int32 GrowWindow(WindowPtr window, Point start, Rect *size);
bool TrackBox(WindowPtr window, Point point, int part);	// Returns true if grow/shrink box was clicked (part corresponds to type)
void ZoomWindow(WindowPtr window, int part, bool bringToFront);
void HiliteWindow(WindowPtr window, bool highlighted);
void DisposeWindow(WindowPtr window);
void GetWindowBounds(WindowPtr window, WindowRegionType windowRegion, Rect *rect);

WindowPtr GetNewCWindow(int resID, void *storage, WindowPtr behind);
WindowPtr NewCWindow(void *storage, const Rect *bounds, const PLPasStr &title, Boolean visible, int wdef, WindowPtr behind, long userdata);
WindowPtr NewWindow(void *storage, const Rect *bounds, const PLPasStr &title, Boolean visible, int wdef, WindowPtr behind, long userdata);
void SizeWindow(WindowPtr window, int width, int height, Boolean addToUpdateRegion);
void MoveWindow(WindowPtr window, int x, int y, Boolean moveToFront);
void ShowWindow(WindowPtr window);
void SetWTitle(WindowPtr window, const PLPasStr &title);

long MenuSelect(Point point);	// Breaks into menu select routine (in practice we'll just forward one from the queue?)

long MenuKey(intptr_t charCode);
long TickCount();

short LoWord(Int32 v);
short HiWord(Int32 v);

void NumToString(long number, unsigned char *str);

PLError_t AEProcessAppleEvent(EventRecord *evt);

void GetIndString(unsigned char *str, int stringsID, int fnameIndex);	// Fetches a string resource of some sort

VFileSpec MakeVFileSpec(PortabilityLayer::VirtualDirectory_t dir, const PLPasStr &fileName);

PLError_t FSpGetFInfo(const VFileSpec &spec, VFileInfo &finfoOut);

DirectoryFileListEntry *GetDirectoryFiles(PortabilityLayer::VirtualDirectory_t dirID);
void DisposeDirectoryFiles(DirectoryFileListEntry *firstDFL);

void GetMouse(Window *window, Point *point);
Boolean Button();	// Returns true if there's a mouse down event in the queue
Boolean StillDown();
Boolean WaitMouseUp();

short Random();
void GetTime(DateTimeRec *dateTime);
UInt32 GetDblTime();

void FlushEvents(int mask, int unknown);
void ExitToShell();

void InvalWindowRect(WindowPtr window, const Rect *rect);

Handle NewHandle(Size size);
long GetHandleSize(Handle handle);

PLError_t SetHandleSize(Handle hdl, Size newSize);

void *NewPtr(Size size);
void *NewPtrClear(Size size);
void DisposePtr(void *ptr);

void BlockMove(const void *src, void *dest, Size size);

bool WaitForEvent(TimeTaggedVOSEvent *evt, uint32_t ticks);

void DrawControls(WindowPtr window);
void DrawGrowIcon(WindowPtr window);

void DebugStr(const PLPasStr &str);

static const Boolean PL_TRUE = 1;
static const Boolean PL_FALSE = 0;

WindowPtr PL_GetPutInFrontWindowPtr();

void PL_NotYetImplemented();
void PL_NotYetImplemented_Minor();
void PL_NotYetImplemented_TODO(const char *category);
void PL_Init();
