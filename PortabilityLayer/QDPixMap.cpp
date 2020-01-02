#include "QDPixMap.h"
#include "CoreDefs.h"
#include "MemoryManager.h"

#include <assert.h>

namespace PortabilityLayer
{
	void PixMapImpl::Destroy(THandle<PixMapImpl> &hdl)
	{
		if (hdl)
		{
			(*hdl)->~PixMapImpl();
		}

		hdl.Dispose();
	}

	PixMapImpl::PixMapImpl(int16_t left, int16_t top, uint16_t width, uint16_t height, GpPixelFormat_t pixelFormat)
		: m_left(left)
		, m_top(top)
		, m_width(width)
		, m_height(height)
		, m_dataCapacity(0)
	{
		const Rect rect = Rect::Create(top, left, static_cast<uint16_t>(top + height), static_cast<uint16_t>(left + width));

		const size_t pitch = PitchForWidth(width, pixelFormat);

		void *dataPtr = reinterpret_cast<uint8_t*>(this) + AlignedSize();
		m_dataCapacity = PitchForWidth(width, pixelFormat) * height;

		static_cast<PixMap*>(this)->Init(rect, pixelFormat, PitchForWidth(width, pixelFormat), dataPtr);
	}

	PixMapImpl::~PixMapImpl()
	{
	}

	size_t PixMapImpl::SizeForDimensions(uint16_t width, uint16_t height, GpPixelFormat_t pixelFormat)
	{
		return AlignedSize() + PitchForWidth(width, pixelFormat) * height;
	}

	size_t PixMapImpl::AlignedSize()
	{
		const size_t szBase = sizeof(PixMapImpl) + GP_SYSTEM_MEMORY_ALIGNMENT - 1;
		const size_t szAdjusted = szBase - szBase % GP_SYSTEM_MEMORY_ALIGNMENT;

		return szAdjusted;
	}

	size_t PixMapImpl::PitchForWidth(uint16_t width, GpPixelFormat_t pixelFormat)
	{
		size_t rowByteCount = 0;

		switch (pixelFormat)
		{
		case GpPixelFormats::k8BitCustom:
		case GpPixelFormats::k8BitStandard:
		case GpPixelFormats::kBW1:
			rowByteCount = width;
			break;
		case GpPixelFormats::kRGB555:
			rowByteCount = width * 2;
			break;
		case GpPixelFormats::kRGB24:
			rowByteCount = width * 3;
			break;
		case GpPixelFormats::kRGB32:
			rowByteCount = width * 4;
			break;
		default:
			assert(false);
			return 0;
		}
		const size_t szBase = rowByteCount + GP_SYSTEM_MEMORY_ALIGNMENT - 1;
		const size_t szAdjusted = szBase - szBase % GP_SYSTEM_MEMORY_ALIGNMENT;

		return szAdjusted;
	}

	THandle<PixMapImpl> PixMapImpl::Create(const Rect &rect, GpPixelFormat_t pixelFormat)
	{
		if (!rect.IsValid())
			return THandle<PixMapImpl>();

		const uint16_t width = static_cast<uint16_t>(rect.right - rect.left);
		const uint16_t height = static_cast<uint16_t>(rect.bottom - rect.top);

		const size_t pixMapSize = PixMapImpl::SizeForDimensions(width, height, pixelFormat);
		if (pixMapSize == 0)
			return THandle<PixMapImpl>();

		MMHandleBlock *pmBlock = PortabilityLayer::MemoryManager::GetInstance()->AllocHandle(pixMapSize);
		if (!pmBlock)
			return THandle<PixMapImpl>();

		memset(pmBlock->m_contents, 0, pixMapSize);

		new (pmBlock->m_contents) PixMapImpl(rect.left, rect.top, width, height, pixelFormat);

		return THandle<PixMapImpl>(pmBlock);
	}
}

void PixMap::Init(const Rect &rect, GpPixelFormat_t pixelFormat, size_t pitch, void *dataPtr)
{
	BitMap::Init(rect, pixelFormat, pitch, dataPtr);
}
