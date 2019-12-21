#pragma once

#include "CoreDefs.h"
#include "PLBigEndian.h"
#include "PixelFormat.h"
#include "RGBAColor.h"

struct Point
{
	int16_t v;
	int16_t h;
};

struct Rect
{
	int16_t top;
	int16_t left;
	int16_t bottom;
	int16_t right;

	bool IsValid() const;
	Rect Intersect(const Rect &rect) const;
	Rect MakeValid() const;
	static Rect Create(int16_t top, int16_t left, int16_t bottom, int16_t right);
};

struct Region
{
	uint16_t size;
	Rect rect;
};


struct BERect
{
	BEInt16_t top;
	BEInt16_t left;
	BEInt16_t bottom;
	BEInt16_t right;

	Rect ToRect() const;
};

struct BERegion
{
	BEUInt16_t recordSize;
	BERect rect;
};

struct Picture
{
	uint8_t sizeLowBytes[2];	// Low-order bytes of size, deprecated

	BERect picFrame;
};

struct BEBitMap
{
	BERect m_bounds;
};

struct BEPixMap
{
	BERect m_bounds;
	BEUInt16_t m_version;
	BEUInt16_t m_packType;
	BEUInt32_t m_packSize;
	BEUInt32_t m_hRes;
	BEUInt32_t m_vRes;
	BEUInt16_t m_pixelType;
	BEUInt16_t m_pixelSize;
	BEUInt16_t m_componentCount;
	BEUInt16_t m_componentSize;
	BEUInt32_t m_planeSizeBytes;
	BEUInt32_t m_clutHandle;
	BEUInt32_t m_unused;
};

struct BEColorTableHeader
{
	BEUInt32_t m_resourceID;
	BEUInt16_t m_flags;	// 0x8000 = indexed, 0x0000 = pixel map
	BEUInt16_t m_numItemsMinusOne;
};

struct BEColorTableItem
{
	BEUInt16_t m_index;
	uint8_t m_red[2];
	uint8_t m_green[2];
	uint8_t m_blue[2];
};

struct GDevice
{
	PortabilityLayer::PixelFormat pixelFormat;

	uint8_t paletteStorage[256 * 4 + PL_SYSTEM_MEMORY_ALIGNMENT];
	uint8_t paletteDataOffset;
	bool paletteIsDirty;
};

inline bool Rect::IsValid() const
{
	return this->top <= this->bottom && this->left <= this->right;
}

inline Rect Rect::Intersect(const Rect &other) const
{
	Rect result = *this;

	if (result.left < other.left)
		result.left = other.left;
	if (result.right > other.right)
		result.right = other.right;
	if (result.top < other.top)
		result.top = other.top;
	if (result.bottom > other.bottom)
		result.bottom = other.bottom;

	return result;
}

inline Rect Rect::MakeValid() const
{
	Rect result = *this;

	if (result.right < result.left)
		result.right = result.left;
	if (result.bottom < result.top)
		result.bottom = result.top;

	return result;
}

inline Rect Rect::Create(int16_t top, int16_t left, int16_t bottom, int16_t right)
{
	Rect result;
	result.top = top;
	result.left = left;
	result.bottom = bottom;
	result.right = right;

	return result;
}
