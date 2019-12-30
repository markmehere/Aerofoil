#include "MemoryManager.h"
#include "MMBlock.h"
#include "MMHandleBlock.h"
#include "ResourceCompiledRef.h"
#include "ResourceManager.h"

#include <stdlib.h>
#include <new>
#include <assert.h>
#include <string.h>

namespace PortabilityLayer
{
	class MemoryManagerImpl final : public MemoryManager
	{
	public:
		void Init() override;
		void Shutdown() override;

		void *Alloc(size_t size) override;
		void *Realloc(void *buf, size_t newSize) override;
		void Release(void *buf) override;

		MMHandleBlock *AllocHandle(size_t size) override;
		bool ResizeHandle(MMHandleBlock *hdl, size_t newSize) override;
		void ReleaseHandle(MMHandleBlock *hdl) override;

		static MemoryManagerImpl *GetInstance();

	private:

		static MemoryManagerImpl ms_instance;
	};

	void MemoryManagerImpl::Init()
	{
	}

	void MemoryManagerImpl::Shutdown()
	{
	}

	void *MemoryManagerImpl::Realloc(void *buf, size_t newSize)
	{
		assert(buf != nullptr);

		const size_t mmBlockSize = MMBlock::AlignedSize();
		uint8_t *oldBufBytes = static_cast<uint8_t*>(buf);
		const MMBlock *oldBufMMBlock = reinterpret_cast<const MMBlock*>(oldBufBytes - MMBlock::AlignedSize());

		const size_t oldBufOffsetFromAlignLoc = oldBufMMBlock->m_offsetFromAllocLocation;
		uint8_t *oldBufBase = oldBufBytes - MMBlock::AlignedSize() - oldBufOffsetFromAlignLoc;

		const size_t mmBlockSizeWithMaxPadding = MMBlock::AlignedSize() + PL_SYSTEM_MEMORY_ALIGNMENT - 1;
		if (SIZE_MAX - newSize < mmBlockSizeWithMaxPadding)
			return nullptr;

		const size_t newBufferSize = newSize + mmBlockSizeWithMaxPadding;
		uint8_t *newBuffer = static_cast<uint8_t*>(realloc(oldBufBase, newSize + mmBlockSizeWithMaxPadding));
		if (!newBuffer)
			return nullptr;

		const intptr_t offsetFromAlignPoint = reinterpret_cast<intptr_t>(newBuffer) & static_cast<intptr_t>(PL_SYSTEM_MEMORY_ALIGNMENT - 1);
		intptr_t alignPadding = 0;
		if (offsetFromAlignPoint != 0)
			alignPadding = static_cast<intptr_t>(PL_SYSTEM_MEMORY_ALIGNMENT) - offsetFromAlignPoint;

		// Check if the alignment changed, if so relocate
		if (static_cast<size_t>(alignPadding) != oldBufOffsetFromAlignLoc)
			memmove(newBuffer + alignPadding, newBuffer + oldBufOffsetFromAlignLoc, MMBlock::AlignedSize() + newSize);

		MMBlock *newMMBlock = reinterpret_cast<MMBlock*>(newBuffer + alignPadding);
		newMMBlock->m_offsetFromAllocLocation = static_cast<SmallestUInt<PL_SYSTEM_MEMORY_ALIGNMENT>::ValueType_t>(alignPadding);

		return newBuffer + alignPadding + MMBlock::AlignedSize();
	}

	void *MemoryManagerImpl::Alloc(size_t size)
	{
		if (size == 0)
			return nullptr;

		const size_t mmBlockSizeWithMaxPadding = MMBlock::AlignedSize() + PL_SYSTEM_MEMORY_ALIGNMENT - 1;
		if (SIZE_MAX - size < mmBlockSizeWithMaxPadding)
			return nullptr;

		uint8_t *buffer = static_cast<uint8_t*>(malloc(size + mmBlockSizeWithMaxPadding));
		if (!buffer)
			return nullptr;

		const intptr_t offsetFromAlignPoint = reinterpret_cast<intptr_t>(buffer) & static_cast<intptr_t>(PL_SYSTEM_MEMORY_ALIGNMENT - 1);
		intptr_t alignPadding = 0;
		if (offsetFromAlignPoint != 0)
			alignPadding = static_cast<intptr_t>(PL_SYSTEM_MEMORY_ALIGNMENT) - offsetFromAlignPoint;

		MMBlock *mmBlock = reinterpret_cast<MMBlock*>(buffer + alignPadding);
		mmBlock->m_offsetFromAllocLocation = static_cast<SmallestUInt<PL_SYSTEM_MEMORY_ALIGNMENT>::ValueType_t>(alignPadding);

		return buffer + alignPadding + MMBlock::AlignedSize();
	}

	void MemoryManagerImpl::Release(void *buf)
	{
		if (!buf)
			return;

		const size_t mmBlockSize = MMBlock::AlignedSize();

		uint8_t *bytes = static_cast<uint8_t*>(buf);
		const MMBlock *mmBlock = reinterpret_cast<const MMBlock*>(bytes - MMBlock::AlignedSize());

		free(bytes - MMBlock::AlignedSize() - mmBlock->m_offsetFromAllocLocation);
	}

	MMHandleBlock *MemoryManagerImpl::AllocHandle(size_t size)
	{
		void *contents = Alloc(size);
		MMHandleBlock *handleBlock = static_cast<MMHandleBlock*>(Alloc(sizeof(MMHandleBlock)));

		return new (handleBlock) MMHandleBlock(contents, size);
	}

	bool MemoryManagerImpl::ResizeHandle(MMHandleBlock *hdl, size_t newSize)
	{
		if (hdl->m_contents == nullptr)
			return false;

		if (newSize != hdl->m_size)
		{
			void *newBuf = Realloc(hdl->m_contents, newSize);
			if (!newBuf)
				return false;

			hdl->m_contents = newBuf;
			hdl->m_size = newSize;
		}

		return true;
	}

	void MemoryManagerImpl::ReleaseHandle(MMHandleBlock *hdl)
	{
		if (!hdl)
			return;

		if (hdl->m_rmSelfRef)
			PortabilityLayer::ResourceManager::GetInstance()->DissociateHandle(hdl);

		if (hdl->m_contents)
			Release(hdl->m_contents);

		hdl->~MMHandleBlock();

		Release(hdl);
	}

	MemoryManagerImpl *MemoryManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	MemoryManagerImpl MemoryManagerImpl::ms_instance;

	MemoryManager *MemoryManager::GetInstance()
	{
		return MemoryManagerImpl::GetInstance();
	}
}
