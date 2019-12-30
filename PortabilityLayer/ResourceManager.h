#pragma once

#include "VirtualDirectory.h"
#include "PLHandle.h"

class PLPasStr;

namespace PortabilityLayer
{
	struct MMHandleBlock;
	struct ResourceCompiledRef;
	class ResTypeID;

	class ResourceManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void SetResLoad(bool load) = 0;

		virtual short OpenResFork(VirtualDirectory_t virtualDir, const PLPasStr &filename) = 0;
		virtual void CloseResFile(short ref) = 0;

		virtual THandle<void> GetResource(const ResTypeID &resType, int id) = 0;

		virtual short GetCurrentResFile() const = 0;
		virtual void SetCurrentResFile(short ref) = 0;

		virtual void DissociateHandle(MMHandleBlock *hdl) const = 0;
		virtual const ResourceCompiledRef *ResourceForHandle(MMHandleBlock *hdl) const = 0;

		static ResourceManager *GetInstance();
	};
}
