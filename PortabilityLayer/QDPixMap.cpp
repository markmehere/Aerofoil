#include "QDPixMap.h"
#include "CoreDefs.h"

#include <assert.h>

namespace PortabilityLayer
{
	PixMapImpl::PixMapImpl(int16_t left, int16_t top, uint16_t width, uint16_t height, PixelFormat pixelFormat)
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

	size_t PixMapImpl::SizeForDimensions(uint16_t width, uint16_t height, PixelFormat pixelFormat)
	{
		return AlignedSize() + PitchForWidth(width, pixelFormat) * height;
	}

	size_t PixMapImpl::AlignedSize()
	{
		const size_t szBase = sizeof(PixMapImpl) + PL_SYSTEM_MEMORY_ALIGNMENT - 1;
		const size_t szAdjusted = szBase - szBase % PL_SYSTEM_MEMORY_ALIGNMENT;

		return szAdjusted;
	}

	size_t PixMapImpl::PitchForWidth(uint16_t width, PixelFormat pixelFormat)
	{
		size_t rowByteCount = 0;

		switch (pixelFormat)
		{
		case PixelFormat_8BitCustom:
		case PixelFormat_8BitStandard:
			rowByteCount = width;
			break;
		case PixelFormat_RGB555:
			rowByteCount = width * 2;
			break;
		case PixelFormat_RGB24:
			rowByteCount = width * 3;
			break;
		case PixelFormat_RGB32:
			rowByteCount = width * 4;
			break;
		default:
			assert(false);
			return 0;
		}
		const size_t szBase = rowByteCount + PL_SYSTEM_MEMORY_ALIGNMENT - 1;
		const size_t szAdjusted = szBase - szBase % PL_SYSTEM_MEMORY_ALIGNMENT;

		return szAdjusted;
	}
}

void PixMap::Init(const Rect &rect, PortabilityLayer::PixelFormat pixelFormat, size_t pitch, void *dataPtr)
{
	BitMap::Init(rect, pixelFormat, pitch, dataPtr);
}
