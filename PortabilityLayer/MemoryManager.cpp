#include "MemoryManager.h"
#include "MMBlock.h"
#include "MMHandleBlock.h"
#include "ResourceCompiledRef.h"

#include <stdlib.h>
#include <new>

namespace PortabilityLayer
{
	class MemoryManagerImpl final : public MemoryManager
	{
	public:
		void Init() override;
		void Shutdown() override;

		void *Alloc(size_t size) override;
		void Release(void *buf) override;

		MMHandleBlock *AllocHandle(size_t size) override;
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

	void MemoryManagerImpl::ReleaseHandle(MMHandleBlock *hdl)
	{
		if (!hdl)
			return;

		if (hdl->m_rmSelfRef != nullptr)
			hdl->m_rmSelfRef->m_handle = nullptr;

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
