#include "DisplayDeviceManager.h"

#include "HostDisplayDriver.h"
#include "PLQDraw.h"
#include "MemoryManager.h"
#include "QDStandardPalette.h"

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
		if (m_mainDevice)
		{
			GDevice *device = *m_mainDevice;

			HostDisplayDriver::GetInstance()->GetDisplayResolution(nullptr, nullptr, &device->pixelFormat);

			uint8_t *paletteStorage = device->paletteStorage;
			while (reinterpret_cast<intptr_t>(paletteStorage) % PL_SYSTEM_MEMORY_ALIGNMENT != 0)
				paletteStorage++;

			PortabilityLayer::RGBAColor *paletteData = reinterpret_cast<PortabilityLayer::RGBAColor*>(paletteStorage);
			device->paletteDataOffset = static_cast<uint8_t>(paletteStorage - device->paletteStorage);

			const PortabilityLayer::RGBAColor *spColors = StandardPalette::GetInstance()->GetColors();
			for (size_t i = 0; i < 256; i++)
				paletteData[i] = spColors[i];

			device->paletteIsDirty = true;
		}
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
