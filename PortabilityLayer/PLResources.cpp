#include "PLResources.h"

#include "MMHandleBlock.h"
#include "ResourceManager.h"
#include "ResourceCompiledRef.h"

void DetachResource(Handle hdl)
{
	PL_NotYetImplemented();
}

void ReleaseResource(Handle hdl)
{
	PL_NotYetImplemented();
}

short CurResFile()
{
	return PortabilityLayer::ResourceManager::GetInstance()->GetCurrentResFile();
}

void UseResFile(short fid)
{
	PL_NotYetImplemented();
}

Handle Get1Resource(UInt32 resID, int index)
{
	PL_NotYetImplemented();
	return nullptr;
}

Handle Get1IndResource(UInt32 resID, int index)
{
	PL_NotYetImplemented();
	return nullptr;
}

int Count1Resources(UInt32 resType)
{
	PL_NotYetImplemented();
	return 0;
}

void HCreateResFile(int refNum, long dirID, const PLPasStr &name)
{
	PL_NotYetImplemented();
}

OSErr ResError()
{
	PL_NotYetImplemented();
	return noErr;
}

short FSpOpenResFile(const FSSpec *spec, int permission)
{
	PL_NotYetImplemented();
	return 0;
}

void CloseResFile(short refNum)
{
	PL_NotYetImplemented();
}

void SetResLoad(Boolean load)
{
	PortabilityLayer::ResourceManager::GetInstance()->SetResLoad(load != 0);
}

long GetMaxResourceSize(Handle res)
{
	const PortabilityLayer::MMHandleBlock *hBlock = reinterpret_cast<PortabilityLayer::MMHandleBlock*>(res);
	const PortabilityLayer::ResourceCompiledRef *resRef = hBlock->m_rmSelfRef;
	return resRef->GetSize();
}

void GetResInfo(Handle res, short *resID, ResType *resType, Str255 resName)
{
	PL_NotYetImplemented();
}

short HOpenResFile(short refNum, long parID, const PLPasStr &name, int permissions)
{
	PL_NotYetImplemented();
	return 0;
}
