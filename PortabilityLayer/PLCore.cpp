#include "PLCore.h"
#include "PLApplication.h"
#include "PLPasStr.h"

#include "AEManager.h"
#include "DisplayDeviceManager.h"
#include "FileManager.h"
#include "FilePermission.h"
#include "HostSuspendCallArgument.h"
#include "HostSuspendHook.h"
#include "HostDisplayDriver.h"
#include "HostSystemServices.h"
#include "ResourceManager.h"
#include "MemoryManager.h"
#include "MMHandleBlock.h"
#include "ResTypeID.h"
#include "RandomNumberGenerator.h"

#include <assert.h>

void InitCursor()
{
}

OSErr FSClose(short fsRef)
{
	PL_NotYetImplemented();
	return noErr;
}

CursHandle GetCursor(int cursorID)
{
	return reinterpret_cast<CursHandle>(GetResource('CURS', cursorID));
}

CCrsrHandle GetCCursor(int cursorID)
{
	PortabilityLayer::ResourceManager *resManager = PortabilityLayer::ResourceManager::GetInstance();
	PortabilityLayer::MMHandleBlock *ccRes = resManager->GetResource('crsr', cursorID);

	if (!ccRes)
		return nullptr;

	PortabilityLayer::MMHandleBlock *copy = PortabilityLayer::MemoryManager::GetInstance()->AllocHandle(ccRes->m_size);
	memcpy(copy->m_contents, ccRes->m_contents, ccRes->m_size);

	return reinterpret_cast<CCrsrHandle>(copy);
}

void SetCCursor(CCrsrHandle handle)
{
	PL_NotYetImplemented_Minor();
}

void HideCursor()
{
	PortabilityLayer::HostDisplayDriver::GetInstance()->HideCursor();
}

void SetCursor(CursPtr cursor)
{
	PL_NotYetImplemented();
}

void DisposeCCursor(CCrsrHandle handle)
{
	PL_NotYetImplemented();
}

void Delay(int ticks, UInt32 *endTickCount)
{
	if (ticks > 0)
	{
		PortabilityLayer::HostSuspendCallArgument args[1];
		args[0].m_uint = static_cast<uint32_t>(ticks);

		PortabilityLayer::SuspendApplication(PortabilityLayer::HostSuspendCallID_Delay, args, nullptr);
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
	PortabilityLayer::MMHandleBlock *block = PortabilityLayer::ResourceManager::GetInstance()->GetResource(PortabilityLayer::ResTypeID(resType), id);
	if (!block)
		return nullptr;

	return &block->m_contents;
}

Handle GetResource(const char(&resTypeLiteral)[5], int id)
{
	PL_NotYetImplemented();
	return nullptr;
}

short FindWindow(Point point, WindowPtr *window)
{
	PL_NotYetImplemented();
	return 0;
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
	PL_NotYetImplemented();
}

WindowPtr GetNewCWindow(int resID, void *storage, WindowPtr behind)
{
	PL_NotYetImplemented();
	return nullptr;
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
	PL_NotYetImplemented();
}

void MoveWindow(WindowPtr window, int x, int y, Boolean moveToFront)
{
	PL_NotYetImplemented();
}

void ShowWindow(WindowPtr window)
{
	PL_NotYetImplemented();
}

void SetWTitle(WindowPtr window, const PLPasStr &title)
{
	PL_NotYetImplemented();
}

bool GetNextEvent(int32_t eventMask, EventRecord *event)
{
	PL_NotYetImplemented();
	return noErr;
}

long MenuSelect(Point point)
{
	PL_NotYetImplemented();
	return noErr;
}

long MenuKey(int charCode)
{
	PL_NotYetImplemented();
	return noErr;
}

long TickCount()
{
	return PortabilityLayer::DisplayDeviceManager::GetInstance()->GetTickCount();
}

void GetKeys(KeyMap keyMap)
{
	PL_NotYetImplemented();
}

short LoWord(Int32 v)
{
	return ((v ^ 0x8000) & 0xffff) - 0x8000;
}

short HiWord(Int32 v)
{
	return (((v >> 16) ^ 0x8000) & 0xffff) - 0x8000;
}

bool BitTst(const KeyMap *keyMap, int bit)
{
	PL_NotYetImplemented();
	return noErr;
}

void NumToString(long number, unsigned char *str)
{
	PL_NotYetImplemented();
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

OSErr AEProcessAppleEvent(EventRecord *evt)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr FindFolder(int refNum, int posType, bool createFolder, short *volumeRef, long *dirID)
{
	switch (posType)
	{
	case kPreferencesFolderType:
		*volumeRef = 0;
		*dirID = PortabilityLayer::EVirtualDirectory_Prefs;
		return noErr;
	default:
		return genericErr;
	}
}

void GetIndString(unsigned char *str, int stringsID, int fnameIndex)
{
	PL_NotYetImplemented();
}

OSErr PBDirCreate(HFileParam *fileParam, bool asynchronous)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr FSMakeFSSpec(int refNum, long dirID, const PLPasStr &fileName, FSSpec *spec)
{

	if (fileName.Length() >= sizeof(spec->name))
		return genericErr;

	PortabilityLayer::Utils::MakePStr(spec->name, fileName.Length(), fileName.Chars());
	spec->vRefNum = refNum;
	spec->parID = dirID;

	if (!PortabilityLayer::FileManager::GetInstance()->FileExists(dirID, fileName))
		return fnfErr;

	return noErr;
}

OSErr FSpCreate(const FSSpec *spec, UInt32 creator, UInt32 fileType, ScriptCode scriptTag)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr FSpDirCreate(const FSSpec *spec, ScriptCode script, long *outDirID)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr FSpOpenDF(const FSSpec *spec, int permission, short *refNum)
{
	PortabilityLayer::EFilePermission perm = PortabilityLayer::EFilePermission_Any;
	switch (permission)
	{
	case fsRdPerm:
		perm = PortabilityLayer::EFilePermission_Read;
		break;
	case fsWrPerm:
	case fsRdWrPerm:
		perm = PortabilityLayer::EFilePermission_ReadWrite;
		break;
	case fsCurPerm:
		perm = PortabilityLayer::EFilePermission_Any;
		break;
	default:
		return permErr;
	}

	return PortabilityLayer::FileManager::GetInstance()->OpenFileDF(spec->parID, spec->name, perm, refNum);
}

OSErr FSWrite(short refNum, long *byteCount, const void *data)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr FSRead(short refNum, long *byteCount, void *data)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr FSpDelete(const FSSpec *spec)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr FSpGetFInfo(const FSSpec *spec, FInfo *finfo)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr SetFPos(short refNum, SetFPosWhere where, long offset)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr GetEOF(short refNum, long *byteCount)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr SetEOF(short refNum, long byteCount)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr PBGetCatInfo(CInfoPBPtr paramBlock, Boolean async)
{
	PL_NotYetImplemented();
	return noErr;
}

short StringWidth(const PLPasStr &str)
{
	PL_NotYetImplemented();
	return 0;
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
	PL_NotYetImplemented();
}

UInt32 GetDblTime()
{
	PL_NotYetImplemented_Minor();
	return 30;
}

void FlushEvents(int mask, int unknown)
{
	PL_NotYetImplemented();
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

void DisposeHandle(Handle handle)
{
	PL_NotYetImplemented();
}

long GetHandleSize(Handle handle)
{
	PL_NotYetImplemented();
	return 0;
}

void HNoPurge(Handle hdl)
{
}

void MoveHHi(Handle hdl)
{
}

void HLock(Handle hdl)
{
}

void HUnlock(Handle hdl)
{
}

OSErr PtrAndHand(const void *data, Handle handle, Size size)
{
	PL_NotYetImplemented();
	return noErr;
}

void SetHandleSize(Handle hdl, Size newSize)
{
	PL_NotYetImplemented();
}

void *NewPtr(Size size)
{
	PL_NotYetImplemented();
	return nullptr;
}

void *NewPtrClear(Size size)
{
	PL_NotYetImplemented();
	return nullptr;
}

void DisposePtr(void *ptr)
{
	PL_NotYetImplemented();
}

Size MaxMem(Size *growBytes)
{
	PL_NotYetImplemented();
	return 0;
}

void PurgeSpace(long *totalFree, long *contiguousFree)
{
	PL_NotYetImplemented();
}

void HSetState(Handle handle, char state)
{
	PL_NotYetImplemented();
}

char HGetState(Handle handle)
{
	PL_NotYetImplemented();
	return 0;
}

OSErr MemError()
{
	PL_NotYetImplemented();
	return 0;
}

void BlockMove(const void *src, void *dest, Size size)
{
	PL_NotYetImplemented();
}

Boolean WaitNextEvent(int eventMask, EventRecord *eventOut, long sleep, void *unknown)
{
	PL_NotYetImplemented();
	return 0;
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

void PL_Init()
{
	PortabilityLayer::MemoryManager::GetInstance()->Init();
	PortabilityLayer::ResourceManager::GetInstance()->Init();
	PortabilityLayer::DisplayDeviceManager::GetInstance()->Init();
	PortabilityLayer::AEManager::GetInstance()->Init();
}
