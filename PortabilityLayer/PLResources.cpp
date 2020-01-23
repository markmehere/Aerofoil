#include "PLResources.h"

#include "HostFileSystem.h"
#include "MemoryManager.h"
#include "MMHandleBlock.h"
#include "PLPasStr.h"
#include "ResourceManager.h"
#include "ResourceCompiledRef.h"

#include <assert.h>


long GetMaxResourceSize(Handle res)
{
	const PortabilityLayer::MMHandleBlock *hBlock = res.MMBlock();
	const PortabilityLayer::ResourceArchiveRef *resRef = hBlock->m_rmSelfRef;
	return resRef->m_size;
}
