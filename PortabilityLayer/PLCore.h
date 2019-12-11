#pragma once
#ifndef __PL_CORE_H__
#define __PL_CORE_H__

#include "DataTypes.h"
#include "PLErrorCodes.h"
#include "SharedTypes.h"
#include "QDPort.h"

#ifdef _MSC_VER
#pragma warning(error:4311)	// Pointer truncation to int
#endif

namespace PortabilityLayer
{
	struct MMHandleBlock;
}

typedef uint8_t Boolean;
typedef uint8_t Byte;
typedef uint8_t UInt8;
typedef int16_t SInt16;
typedef int32_t Int32;
typedef uint32_t UInt32;
typedef int OSErr;

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
struct Region;

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
	int ioFDirIndex;	// Index (1-based!)
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

struct Cursor
{
};

struct CCursor
{
};


struct Window
{
	Window();

	PortabilityLayer::QDPort m_port;	// Must be the first item
};

struct Menu
{
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

struct FSSpec
{
	Str63 name;
	UInt32 parID;	// Directory ID
	SInt16 vRefNum;	// Volume ID
};

typedef struct FInfo
{
	UInt32 fdType;
} FInfo;

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
typedef FSSpec *FSSpecPtr;
typedef Menu *MenuPtr;
typedef Region *RgnPtr;
typedef CInfoPBRec *CInfoPBPtr;
typedef VersionRecord *VersRecPtr;

typedef CursPtr *CursHandle;
typedef CCrsrPtr *CCrsrHandle;
typedef MenuPtr *MenuHandle;
typedef RgnPtr *RgnHandle;
typedef VersRecPtr *VersRecHndl;

typedef void *Ptr;
typedef Ptr *Handle;

typedef WindowPtr WindowRef;	// wtf?


typedef unsigned char KeyMap[16];


enum RegionID
{
	inDesk,
	inMenuBar,
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
	keyDown,
	autoKey,
	updateEvt,
	osEvt,
	kHighLevelEvent,
};

enum ScriptCode
{
	smSystemScript,
};

enum FindFolderRefNum
{
	kOnSystemDisk,
};

enum FindFolderType
{
	kPreferencesFolderType,
};

enum SetFPosWhere
{
	fsFromStart,
};

enum BuiltinWDEFs
{
	noGrowDocProc = 4,		// Movable, not resizable
};

static const int everyEvent = -1;

static const int iBeamCursor = 1;
static const int watchCursor = 4;

static const int charCodeMask = 0xff;
static const int keyCodeMask = 0xff00;

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

OSErr FSClose(short fsRef);

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

bool GetNextEvent(int32_t eventMask, EventRecord *event);

long MenuSelect(Point point);	// Breaks into menu select routine (in practice we'll just forward one from the queue?)

long MenuKey(int charCode);
long TickCount();
void GetKeys(KeyMap keyMap);

short LoWord(Int32 v);
short HiWord(Int32 v);
bool BitTst(const KeyMap *keyMap, int bit);

void NumToString(long number, unsigned char *str);
void ParamText(const PLPasStr &title, const PLPasStr &a, const PLPasStr &b, const PLPasStr &c);

UInt32 FreeMem();

OSErr AEProcessAppleEvent(EventRecord *evt);

OSErr FindFolder(int refNum, int posType, bool createFolder, short *volumeRef, long *dirID);
void GetIndString(unsigned char *str, int stringsID, int fnameIndex);	// Fetches a string resource of some sort
OSErr PBDirCreate(HFileParam *fileParam, bool asynchronous);

OSErr FSMakeFSSpec(int refNum, long dirID, const PLPasStr &fileName, FSSpec *spec);
OSErr FSpCreate(const FSSpec *spec, UInt32 creator, UInt32 fileType, ScriptCode scriptTag);
OSErr FSpDirCreate(const FSSpec *spec, ScriptCode script, long *outDirID);
OSErr FSpOpenDF(const FSSpec *spec, int permission, short *refNum);
OSErr FSWrite(short refNum, long *byteCount, const void *data);
OSErr FSRead(short refNum, long *byteCount, void *data);
OSErr FSpDelete(const FSSpec *spec);
OSErr FSpGetFInfo(const FSSpec *spec, FInfo *finfo);
OSErr SetFPos(short refNum, SetFPosWhere where, long offset); 
OSErr GetEOF(short refNum, long *byteCount);
OSErr SetEOF(short refNum, long byteCount);

OSErr PBGetCatInfo(CInfoPBPtr paramBlock, Boolean async);

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

void HNoPurge(Handle hdl);	// Marks a handle as not purgeable
void MoveHHi(Handle hdl);	// Relocates a block to the top of the heap
void HLock(Handle hdl);		// Disable relocation
void HUnlock(Handle hdl);	// Re-enable relocation

OSErr PtrAndHand(const void *data, Handle handle, Size size);	// Appends data to the end of a handle
void SetHandleSize(Handle hdl, Size newSize);

void *NewPtr(Size size);
void *NewPtrClear(Size size);
void DisposePtr(void *ptr);

Size MaxMem(Size *growBytes);
void PurgeSpace(long *totalFree, long *contiguousFree);

void HSetState(Handle handle, char state);
char HGetState(Handle handle);

OSErr MemError();

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
void PL_Init();


#endif
