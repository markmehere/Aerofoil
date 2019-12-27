#include "DisplayDeviceManager.h"

#include "HostDisplayDriver.h"
#include "IGpDisplayDriver.h"
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

		GpPixelFormat_t GetPixelFormat() const override;
		void SyncPalette(IGpDisplayDriver *displayDriver) override;

		void IncrementTickCount(uint32_t count) override;
		uint32_t GetTickCount() override;

		static DisplayDeviceManagerImpl *GetInstance();

	private:
		uint32_t m_tickCount;
		GpPixelFormat_t m_pixelFormat;
		bool m_paletteIsDirty;

		PortabilityLayer::RGBAColor *m_palette;
		uint8_t m_paletteStorage[256 * sizeof(PortabilityLayer::RGBAColor) + PL_SYSTEM_MEMORY_ALIGNMENT];

		static DisplayDeviceManagerImpl ms_instance;
	};

	DisplayDeviceManagerImpl::DisplayDeviceManagerImpl()
		: m_tickCount(1)
		, m_paletteIsDirty(true)
		, m_pixelFormat(GpPixelFormats::k8BitStandard)
	{
		uint8_t *paletteStorage = m_paletteStorage;
		while (reinterpret_cast<intptr_t>(paletteStorage) % PL_SYSTEM_MEMORY_ALIGNMENT != 0)
			paletteStorage++;

		m_palette = reinterpret_cast<PortabilityLayer::RGBAColor*>(paletteStorage);

		for (size_t i = 0; i < 256; i++)
		{
			PortabilityLayer::RGBAColor &color = m_palette[i];
			color.r = color.g = color.b = i;
			color.a = 255;
		}
	}

	void DisplayDeviceManagerImpl::Init()
	{
		HostDisplayDriver::GetInstance()->GetDisplayResolution(nullptr, nullptr, &m_pixelFormat);

		const PortabilityLayer::RGBAColor *spColors = StandardPalette::GetInstance()->GetColors();
		for (size_t i = 0; i < 256; i++)
			m_palette[i] = spColors[i];

		m_paletteIsDirty = true;
	}

	void DisplayDeviceManagerImpl::Shutdown()
	{
	}

	GpPixelFormat_t DisplayDeviceManagerImpl::GetPixelFormat() const
	{
		return m_pixelFormat;
	}

	void DisplayDeviceManagerImpl::SyncPalette(IGpDisplayDriver *displayDriver)
	{
		if (m_paletteIsDirty)
		{
			displayDriver->UpdatePalette(m_palette);
			m_paletteIsDirty = false;
		}
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
