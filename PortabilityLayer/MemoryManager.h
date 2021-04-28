#pragma once

#include <stdint.h>
#include <stddef.h>

struct IGpAllocator;

namespace PortabilityLayer
{
	struct MMHandleBlock;

	class MemoryManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void *Alloc(size_t size) = 0;
		virtual void Release(void *buf) = 0;
		virtual void *Realloc(void *buf, size_t newSize) = 0;

		virtual MMHandleBlock *AllocHandle(size_t size) = 0;
		virtual bool ResizeHandle(MMHandleBlock *hdl, size_t newSize) = 0;
		virtual void ReleaseHandle(MMHandleBlock *hdl) = 0;

		template<class T>
		T **NewHandle();

		static MemoryManager *GetInstance();
	};
}

#include <new>
#include "CoreDefs.h"
#include "MMHandleBlock.h"

namespace PortabilityLayer
{
	template<class T>
	T **MemoryManager::NewHandle()
	{
		MMHandleBlock *hdl = this->AllocHandle(sizeof(T));
		if (!hdl)
			return nullptr;

		T **objectHdl = reinterpret_cast<T**>(hdl);
		T *objectPtr = *objectHdl;
		new (objectPtr) T();

		return objectHdl;
	}
}
