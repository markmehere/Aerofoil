#include "DisplayDeviceManager.h"

#include "PLQuickdraw.h"

#include "MemoryManager.h"

namespace PortabilityLayer
{
	class DisplayDeviceManagerImpl final : public DisplayDeviceManager
	{
	public:
		DisplayDeviceManagerImpl();

		void Init() override;
		void Shutdown() override;

		GDevice **GetMainDevice() override;


		void IncrementTickCount(uint32_t count) override;
		uint32_t GetTickCount() override;

		static DisplayDeviceManagerImpl *GetInstance();

	private:
		GDHandle m_mainDevice;
		uint32_t m_tickCount;

		static DisplayDeviceManagerImpl ms_instance;
	};

	DisplayDeviceManagerImpl::DisplayDeviceManagerImpl()
		: m_mainDevice(nullptr)
		, m_tickCount(1)
	{
	}

	void DisplayDeviceManagerImpl::Init()
	{
		m_mainDevice = MemoryManager::GetInstance()->NewHandle<GDevice>();
	}

	void DisplayDeviceManagerImpl::Shutdown()
	{
		MemoryManager::GetInstance()->ReleaseHandle(reinterpret_cast<MMHandleBlock*>(m_mainDevice));
	}

	GDevice **DisplayDeviceManagerImpl::GetMainDevice()
	{
		return m_mainDevice;
	}

	void DisplayDeviceManagerImpl::IncrementTickCount(uint32_t count)
	{
		m_tickCount += count;
	}

	uint32_t DisplayDeviceManagerImpl::GetTickCount()
	{
		return m_tickCount;
	}


	DisplayDeviceManagerImpl *DisplayDeviceManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	DisplayDeviceManagerImpl DisplayDeviceManagerImpl::ms_instance;

	DisplayDeviceManager *DisplayDeviceManager::GetInstance()
	{
		return DisplayDeviceManagerImpl::GetInstance();
	}
}
