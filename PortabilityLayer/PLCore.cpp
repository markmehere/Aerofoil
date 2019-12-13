#include "PLCore.h"
#include "PLApplication.h"
#include "PLPasStr.h"

#include "AEManager.h"
#include "DisplayDeviceManager.h"
#include "FileManager.h"
#include "FilePermission.h"
#include "HostDirectoryCursor.h"
#include "HostFileSystem.h"
#include "HostSuspendCallArgument.h"
#include "HostSuspendHook.h"
#include "HostDisplayDriver.h"
#include "HostSystemServices.h"
#include "ResourceManager.h"
#include "MacFileInfo.h"
#include "MemoryManager.h"
#include "MemReaderStream.h"
#include "MMHandleBlock.h"
#include "ResTypeID.h"
#include "RandomNumberGenerator.h"
#include "PLBigEndian.h"
#include "QDManager.h"
#include "WindowDef.h"
#include "WindowManager.h"

#include <assert.h>

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

void SetBuiltinCursor(int builtinCursor)
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
	if (fnameIndex < 1)
		return;

	PortabilityLayer::MMHandleBlock *istrRes = PortabilityLayer::ResourceManager::GetInstance()->GetResource('STR#', stringsID);
	if (istrRes && istrRes->m_contents)
	{
		const uint8_t *contentsBytes = static_cast<const uint8_t *>(istrRes->m_contents);
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
	PortabilityLayer::IOStream *stream = PortabilityLayer::FileManager::GetInstance()->GetFileStream(refNum);

	const size_t bytesRead = stream->Read(data, static_cast<size_t>(*byteCount));
	*byteCount = static_cast<long>(bytesRead);

	return noErr;
}

OSErr FSpDelete(const FSSpec *spec)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr FSpGetFInfo(const FSSpec *spec, FInfo *finfo)
{
	PortabilityLayer::MacFileProperties mfp;
	if (!PortabilityLayer::FileManager::GetInstance()->ReadFileProperties(static_cast<uint32_t>(spec->parID), spec->name, mfp))
		return fnfErr;

	finfo->fdType = PortabilityLayer::ResTypeIDCodec::Decode(mfp.m_fileType);

	return noErr;
}

OSErr SetFPos(short refNum, SetFPosWhere where, long offset)
{
	switch (where)
	{
	case fsFromStart:
		if (!PortabilityLayer::FileManager::GetInstance()->GetFileStream(refNum)->SeekStart(static_cast<PortabilityLayer::UFilePos_t>(offset)))
			return ioErr;
		break;
	default:
		return genericErr;
	}

	return noErr;
}

OSErr GetEOF(short refNum, long *byteCount)
{
	const PortabilityLayer::UFilePos_t fileSize = PortabilityLayer::FileManager::GetInstance()->GetFileStream(refNum)->Size();

	*byteCount = static_cast<long>(fileSize);
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

DirectoryFileListEntry *GetDirectoryFiles(long dirID)
{
	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();
	PortabilityLayer::HostFileSystem *fs = PortabilityLayer::HostFileSystem::GetInstance();
	PortabilityLayer::HostDirectoryCursor *dirCursor = fs->ScanDirectory(static_cast<PortabilityLayer::EVirtualDirectory>(dirID));

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
			PortabilityLayer::IOStream *stream = fs->OpenFile(static_cast<PortabilityLayer::EVirtualDirectory>(dirID), filename, false, false);
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
	PortabilityLayer::MemoryManager::GetInstance()->ReleaseHandle(reinterpret_cast<PortabilityLayer::MMHandleBlock*>(handle));
}

long GetHandleSize(Handle handle)
{
	if (!handle)
		return 0;

	PortabilityLayer::MMHandleBlock *block = reinterpret_cast<PortabilityLayer::MMHandleBlock*>(handle);
	return static_cast<long>(block->m_size);
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
}

char HGetState(Handle handle)
{
	return 0;
}

OSErr MemError()
{
	PL_NotYetImplemented();
	return 0;
}

void BlockMove(const void *src, void *dest, Size size)
{
	memcpy(dest, src, size);
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

void PL_NotYetImplemented_TODO()
{
}

void PL_Init()
{
	PortabilityLayer::MemoryManager::GetInstance()->Init();
	PortabilityLayer::ResourceManager::GetInstance()->Init();
	PortabilityLayer::DisplayDeviceManager::GetInstance()->Init();
	PortabilityLayer::AEManager::GetInstance()->Init();
	PortabilityLayer::QDManager::GetInstance()->Init();
}

WindowPtr PL_GetPutInFrontWindowPtr()
{
	return PortabilityLayer::WindowManager::GetInstance()->GetPutInFrontSentinel();
}

Window::Window()
	: m_port(PortabilityLayer::QDPortType_Window)
{
}
