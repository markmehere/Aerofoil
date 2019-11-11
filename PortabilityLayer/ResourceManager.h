#pragma once

#include "VirtualDirectory.h"

class PLPasStr;

namespace PortabilityLayer
{
	struct MMHandleBlock;
	class ResTypeID;

	class ResourceManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void SetResLoad(bool load) = 0;

		virtual short OpenResFork(EVirtualDirectory virtualDir, const PLPasStr &filename) = 0;
		virtual MMHandleBlock *GetResource(const ResTypeID &resType, int id) = 0;

		virtual short GetCurrentResFile() const = 0;
		virtual void SetCurrentResFile(short ref) = 0;

		static ResourceManager *GetInstance();
	};
}
