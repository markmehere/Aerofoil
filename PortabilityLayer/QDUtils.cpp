#include "QDUtils.h"
#include "MemoryManager.h"
#include "SharedTypes.h"

namespace PortabilityLayer
{
	Region **QDUtils::CreateRegion(const Rect &rect)
	{
		PL_STATIC_ASSERT(sizeof(Region) == 10);

		Region **rgnHandle = MemoryManager::GetInstance()->NewHandle<Region>();
		if (!rgnHandle)
			return nullptr;

		Region &region = (**rgnHandle);
		region.rect = rect;
		region.size = sizeof(Region);

		return rgnHandle;
	}

	void QDUtils::ResetRegionToRect(Region **regionHdl, const Rect &rect)
	{
		if (MemoryManager::GetInstance()->ResizeHandle(reinterpret_cast<MMHandleBlock*>(regionHdl), sizeof(Region)))
		{
			Region &region = **regionHdl;
			region.size = sizeof(Region);
			region.rect = rect;
		}
	}
}
