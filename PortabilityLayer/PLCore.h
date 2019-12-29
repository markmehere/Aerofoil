#pragma once
#ifndef __PL_CORE_H__
#define __PL_CORE_H__

#include "DataTypes.h"
#include "PLErrorCodes.h"
#include "SharedTypes.h"
#include "QDPort.h"
#include "QDGraf.h"
#include "ResTypeID.h"
#include "VirtualDirectory.h"

#ifdef _MSC_VER
#pragma warning(error:4311)	// Pointer truncation to int
#endif

struct IGpColorCursor;

namespace PortabilityLayer
{
	struct MMHandleBlock;
	class IOStream;
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
struct CGraf;
struct Menu;

typedef void *Ptr;
typedef Ptr *Handle;

#define PL_DEAD(n) ((void)0)

struct FinderInfoBlock
{
	Int32 fdType;
	Int32 fdCreator;
};

struct FileInfoBlock
{
	void *ioCompletion;
	short ioVRefNum;	// Volume ref num
	StringPtr ioNamePtr;
	int ioFDirIndex;	// Index: If >0, Nth directory in ioVRefNum.  If 0, lookup by name.  If <0, do behavior that we don't support.
	long ioDirID;		// Input: Directory ID   Output: File ID
	int ioFlAttrib;		// File attributes
	FinderInfoBlock ioFlFndrInfo;
	long ioFlParID;
};

struct DirInfoBlock
{
	void *ioCompletion;
	short ioVRefNum;
	long ioDrDirID;
	unsigned char *ioNamePtr;
	short ioFDirIndex;
	int ioFlAttrib;
};

struct CInfoPBRec
{
	FileInfoBlock hFileInfo;
	DirInfoBlock dirInfo;
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

struct CCursor
{
	IGpColorCursor *hwCursor;
};

struct Window
{
	Window();

	CGraf m_graf;	// Must be the first item

	// The port is always at 0,0
	// These are the WM coordinates
	int32_t m_wmX;
	int32_t m_wmY;
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

typedef CGraf *CGrafPtr;
typedef CGrafPtr GWorldPtr;
typedef Window *WindowPtr;
typedef Cursor *CursPtr;
typedef CCursor *CCrsrPtr;
typedef Menu *MenuPtr;
typedef CInfoPBRec *CInfoPBPtr;
typedef VersionRecord *VersRecPtr;

typedef CursPtr *CursHandle;
typedef CCrsrPtr *CCrsrHandle;
typedef MenuPtr *MenuHandle;
typedef VersRecPtr *VersRecHndl;

typedef WindowPtr WindowRef;	// wtf?

struct KeyMap;

enum RegionID
{
	inMenuBar = 1,
	inSysWindow,
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

enum BuiltinWDEFs
{
	noGrowDocProc = 4,		// Movable, not resizable
};

static const int everyEvent = -1;

static const int iBeamCursor = 1;
static const int watchCursor = 4;

static const int shiftKey = 0x1;
static const int cmdKey = 0x2;		// Ctrl
static const int optionKey = 0x4;	// Alt

static const bool kCreateFolder = true;
static const bool kDontCreateFolder = false;

static const int fsRdPerm = 1;
static const int fsWrPerm = 2;
static const int fsRdWrPerm = (fsRdPerm | fsWrPerm);
static const int fsCurPerm = 4;	// Any allowed permission

static const Boolean TRUE = 1;
static const Boolean FALSE = 0;

#define nil nullptr

static const int nullEvent = 0;

//void FlushEvents(int eventMask, int stopMask);

void InitCursor();
CursHandle GetCursor(int cursorID);
CCrsrHandle GetCCursor(int cursorID);
void SetCCursor(CCrsrHandle handle);
void HideCursor();

void SetCursor(CursPtr cursor);
void SetBuiltinCursor(int builtinCursor);

void DisposeCCursor(CCrsrHandle handle);

void Delay(int ticks, UInt32 *endTickCount);
short Alert(int dialogID, void *unknown);

Handle GetResource(int32_t resType, int id);
Handle GetResource(const char(&resTypeLiteral)[5], int id);

short FindWindow(Point point, WindowPtr *window);	// Translates global coordinates to window coordinates, returns a region ID
void DragWindow(WindowPtr window, Point start, Rect *bounds);	// Drags the window (probably not implemented)
void SendBehind(WindowPtr window, WindowPtr behind);
void BringToFront(WindowPtr window);
void ShowHide(WindowPtr window, Boolean hide);
bool TrackGoAway(WindowPtr window, Point point);	// Returns true if the close box was actually clicked (?)
Int32 GrowWindow(WindowPtr window, Point start, Rect *size);
bool TrackBox(WindowPtr window, Point point, int part);	// Returns true if grow/shrink box was clicked (part corresponds to type)
void ZoomWindow(WindowPtr window, int part, bool bringToFront);
void HiliteWindow(WindowPtr window, bool highlighted);
void DisposeWindow(WindowPtr window);
void GetWindowBounds(WindowPtr window, WindowRegionType windowRegion, Rect *rect);

WindowPtr GetNewCWindow(int resID, void *storage, WindowPtr behind);
WindowPtr NewCWindow(void *storage, const Rect *bounds, const PLPasStr &title, Boolean visible, int wdef, WindowPtr behind, Boolean hasCloseBox, long userdata);
WindowPtr NewWindow(void *storage, const Rect *bounds, const PLPasStr &title, Boolean visible, int wdef, WindowPtr behind, Boolean hasCloseBox, long userdata);
void SizeWindow(WindowPtr window, int width, int height, Boolean addToUpdateRegion);
void MoveWindow(WindowPtr window, int x, int y, Boolean moveToFront);
void ShowWindow(WindowPtr window);
void SetWTitle(WindowPtr window, const PLPasStr &title);

bool PeekNextEvent(int32_t eventMask, EventRecord *event);
bool GetNextEvent(int32_t eventMask, EventRecord *event);

long MenuSelect(Point point);	// Breaks into menu select routine (in practice we'll just forward one from the queue?)

long MenuKey(int charCode);
long TickCount();
void GetKeys(KeyMap &keyMap);

short LoWord(Int32 v);
short HiWord(Int32 v);
bool BitTst(const KeyMap &keyMap, int bit);

void NumToString(long number, unsigned char *str);
void ParamText(const PLPasStr &title, const PLPasStr &a, const PLPasStr &b, const PLPasStr &c);

UInt32 FreeMem();

PLError_t AEProcessAppleEvent(EventRecord *evt);

PLError_t FindFolder(int refNum, int posType, bool createFolder, short *volumeRef, long *dirID);
void GetIndString(unsigned char *str, int stringsID, int fnameIndex);	// Fetches a string resource of some sort
PLError_t PBDirCreate(HFileParam *fileParam, bool asynchronous);

VFileSpec MakeVFileSpec(PortabilityLayer::VirtualDirectory_t dir, const PLPasStr &fileName);

PLError_t FSpCreate(const VFileSpec &spec, UInt32 creator, UInt32 fileType);
PLError_t FSpDirCreate(const VFileSpec &spec, long *outDirID);
PLError_t FSpOpenDF(const VFileSpec &spec, int permission, PortabilityLayer::IOStream *&stream);
PLError_t FSpOpenRF(const VFileSpec &spec, int permission, PortabilityLayer::IOStream *&stream);
PLError_t FSpDelete(const VFileSpec &spec);
PLError_t FSpGetFInfo(const VFileSpec &spec, VFileInfo &finfoOut);

PLError_t PBGetCatInfo(CInfoPBPtr paramBlock, Boolean async);

DirectoryFileListEntry *GetDirectoryFiles(PortabilityLayer::VirtualDirectory_t dirID);
void DisposeDirectoryFiles(DirectoryFileListEntry *firstDFL);

short StringWidth(const PLPasStr &str);

void GetMouse(Point *point);
Boolean Button();	// Returns true if there's a mouse down event in the queue
Boolean StillDown();
Boolean WaitMouseUp();

void LocalToGlobal(Point *point);
void GlobalToLocal(Point *point);
short Random();
void GetDateTime(UInt32 *dateTime);
void GetTime(DateTimeRec *dateTime);
UInt32 GetDblTime();

void FlushEvents(int mask, int unknown);
void ExitToShell();

void InvalWindowRect(WindowPtr window, const Rect *rect);

Handle NewHandle(Size size);
void DisposeHandle(Handle handle);
long GetHandleSize(Handle handle);

PLError_t PtrAndHand(const void *data, Handle handle, Size size);	// Appends data to the end of a handle
PLError_t SetHandleSize(Handle hdl, Size newSize);

void *NewPtr(Size size);
void *NewPtrClear(Size size);
void DisposePtr(void *ptr);

Size MaxMem(Size *growBytes);
void PurgeSpace(long *totalFree, long *contiguousFree);

PLError_t MemError();

void BlockMove(const void *src, void *dest, Size size);

Boolean WaitNextEvent(int eventMask, EventRecord *eventOut, long sleep, void *unknown);

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


#endif
