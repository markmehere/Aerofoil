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
		, m_pitch(PitchForWidth(width, pixelFormat))
		, m_pixelFormat(pixelFormat)
	{
		m_rect.left = left;
		m_rect.top = top;
		m_rect.right = static_cast<uint16_t>(left + width);
		m_rect.bottom = static_cast<uint16_t>(top + height);

		m_pitch = (width + PL_SYSTEM_MEMORY_ALIGNMENT - 1);
		m_pitch -= m_pitch % PL_SYSTEM_MEMORY_ALIGNMENT;

		m_data = reinterpret_cast<uint8_t*>(this) + AlignedSize();
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
