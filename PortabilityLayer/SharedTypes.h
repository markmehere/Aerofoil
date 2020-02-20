#pragma once

#include "CoreDefs.h"
#include "GpPixelFormat.h"
#include "PLBigEndian.h"
#include "RGBAColor.h"

struct BitmapImage;

struct Point
{
	int16_t v;
	int16_t h;

	Point operator-(const Point &other) const;
	Point operator+(const Point &other) const;

	Point &operator-=(const Point &other);
	Point &operator+=(const Point &other);

	static Point Create(int16_t h, int16_t v);
};

struct Rect
{
	int16_t top;
	int16_t left;
	int16_t bottom;
	int16_t right;

	bool IsValid() const;
	Rect MakeValid() const;

	Rect Intersect(const Rect &rect) const;
	Rect Inset(int16_t h, int16_t v) const;

	Rect operator-(const Point &point) const;
	Rect operator+(const Point &point) const;

	Rect &operator-=(const Point &point);
	Rect &operator+=(const Point &point);

	bool operator!=(const Rect &other) const;
	bool operator==(const Rect &other) const;

	uint16_t Height() const;
	uint16_t Width() const;

	bool Contains(const Point &point) const;

	static Rect Create(int16_t top, int16_t left, int16_t bottom, int16_t right);
	static Rect CreateFromPoints(const Point &topLeft, const Point &bottomRight);
	static Rect CreateLargest();
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
	GpPixelFormat_t pixelFormat;

	uint8_t paletteStorage[256 * 4 + GP_SYSTEM_MEMORY_ALIGNMENT];
	uint8_t paletteDataOffset;
	bool paletteIsDirty;
};

inline Point Point::operator-(const Point &other) const
{
	return Point::Create(this->h - other.h, this->v - other.v);
}

inline Point Point::operator+(const Point &other) const
{
	return Point::Create(this->h + other.h, this->v + other.v);
}

inline Point &Point::operator-=(const Point &other)
{
	this->h -= other.h;
	this->v -= other.v;
	return *this;
}

inline Point &Point::operator+=(const Point &other)
{
	this->h += other.h;
	this->v += other.v;
	return *this;
}

inline Point Point::Create(int16_t h, int16_t v)
{
	Point p;
	p.h = h;
	p.v = v;

	return p;
}

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

inline Rect Rect::Inset(int16_t h, int16_t v) const
{
	return Rect::Create(top + v, left + h, bottom - v, right - h);
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

inline Rect Rect::operator-(const Point &point) const
{
	return Rect::Create(this->top - point.v, this->left - point.h, this->bottom - point.v, this->right - point.h);
}

inline Rect Rect::operator+(const Point &point) const
{
	return Rect::Create(this->top + point.v, this->left + point.h, this->bottom + point.v, this->right + point.h);
}

inline Rect &Rect::operator-=(const Point &point)
{
	this->top -= point.v;
	this->bottom -= point.v;
	this->left -= point.h;
	this->right -= point.h;

	return *this;
}

inline Rect &Rect::operator+=(const Point &point)
{
	this->top += point.v;
	this->bottom += point.v;
	this->left += point.h;
	this->right += point.h;

	return *this;
}

inline bool Rect::operator!=(const Rect &other) const
{
	if (this->top != other.top || this->bottom != other.bottom || this->left != other.left || this->right != other.right)
		return true;

	return false;
}

inline bool Rect::operator==(const Rect &other) const
{
	return !((*this) != other);
}


inline uint16_t Rect::Height() const
{
	return static_cast<uint16_t>(static_cast<int32_t>(this->bottom) - static_cast<int32_t>(this->top));
}

inline uint16_t Rect::Width() const
{
	return static_cast<uint16_t>(static_cast<int32_t>(this->right) - static_cast<int32_t>(this->left));
}

inline bool Rect::Contains(const Point &point) const
{
	return point.h >= this->left && point.h < this->right && point.v >= this->top && point.v < this->bottom;
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

inline Rect Rect::CreateFromPoints(const Point &topLeft, const Point &bottomRight)
{
	return Rect::Create(topLeft.v, topLeft.h, bottomRight.v, bottomRight.h);
}

inline Rect Rect::CreateLargest()
{
	return Rect::Create(-0x8000, -0x8000, 0x7fff, 0x7fff);
}
