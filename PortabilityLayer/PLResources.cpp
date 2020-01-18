#include "PLResources.h"

#include "HostFileSystem.h"
#include "MemoryManager.h"
#include "MMHandleBlock.h"
#include "PLPasStr.h"
#include "ResourceManager.h"
#include "ResourceCompiledRef.h"

#include <assert.h>

struct PLOpenedResFile
{
	bool m_isOpen;
};

static const unsigned int kPLMaxOpenedResFiles = 64;
static PLOpenedResFile gs_resFiles[kPLMaxOpenedResFiles];

short CurResFile()
{
	return PortabilityLayer::ResourceManager::GetInstance()->GetCurrentResFile();
}

void UseResFile(short fid)
{
	PortabilityLayer::ResourceManager::GetInstance()->SetCurrentResFile(fid);
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

void SetResLoad(Boolean load)
{
	PortabilityLayer::ResourceManager::GetInstance()->SetResLoad(load != 0);
}

long GetMaxResourceSize(Handle res)
{
	const PortabilityLayer::MMHandleBlock *hBlock = res.MMBlock();
	const PortabilityLayer::ResourceArchiveRef *resRef = hBlock->m_rmSelfRef;
	return resRef->m_size;
}

short HOpenResFile(PortabilityLayer::VirtualDirectory_t dirID, const PLPasStr &name, int permissions)
{
	PL_NotYetImplemented();
	return 0;
}
