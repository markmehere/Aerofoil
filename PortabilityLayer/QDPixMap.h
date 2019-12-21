#pragma once

#include "PLBigEndian.h"
#include "PLQuickdraw.h"
#include "PixelFormat.h"

#include <stdint.h>

struct PixMap : public BitMap
{
	void Init(const Rect &rect, PortabilityLayer::PixelFormat pixelFormat, size_t pitch, void *dataPtr);
};

namespace PortabilityLayer
{
	class PixMapImpl final : public PixMap
	{
	public:
		PixMapImpl(int16_t left, int16_t top, uint16_t width, uint16_t height, PixelFormat pixelFormat);
		
		PixelFormat GetPixelFormat() const;
		size_t GetPitch() const;
		void *GetPixelData();
		const void *GetPixelData() const;
		size_t GetDataCapacity() const;

		static size_t SizeForDimensions(uint16_t width, uint16_t height, PixelFormat pixelFormat);

	private:
		static size_t AlignedSize();
		static size_t PitchForWidth(uint16_t width, PixelFormat pixelFormat);

		int16_t m_left;
		int16_t m_top;
		uint16_t m_width;
		uint16_t m_height;

		size_t m_dataCapacity;
	};
}

inline PortabilityLayer::PixelFormat PortabilityLayer::PixMapImpl::GetPixelFormat() const
{
	return m_pixelFormat;
}

inline size_t PortabilityLayer::PixMapImpl::GetPitch() const
{
	return m_pitch;
}

inline void *PortabilityLayer::PixMapImpl::GetPixelData()
{
	return m_data;
}

inline const void *PortabilityLayer::PixMapImpl::GetPixelData() const
{
	return m_data;
}

inline size_t PortabilityLayer::PixMapImpl::GetDataCapacity() const
{
	return m_dataCapacity;
}
