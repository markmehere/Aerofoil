#include "MemoryManager.h"
#include "MMHandleBlock.h"
#include "ResourceCompiledRef.h"
#include "ResourceManager.h"
#include "IGpAllocator.h"
#include "PLDrivers.h"

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
		return PLDrivers::GetAlloc()->Realloc(buf, newSize);
	}

	void *MemoryManagerImpl::Alloc(size_t size)
	{
		return PLDrivers::GetAlloc()->Realloc(nullptr, size);
	}

	void MemoryManagerImpl::Release(void *buf)
	{
		PLDrivers::GetAlloc()->Realloc(buf, 0);
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
		{
			if (newSize != 0)
			{
				void *newBuf = Alloc(newSize);
				if (!newBuf)
					return false;

				hdl->m_contents = newBuf;
				hdl->m_size = newSize;
			}
		}

		if (newSize != hdl->m_size)
		{
			void *newBuf = Realloc(hdl->m_contents, newSize);
			if (!newBuf && newSize != 0)
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
