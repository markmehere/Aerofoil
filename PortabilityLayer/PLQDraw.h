#pragma once

#include "PLCore.h"
#include "QDGraf.h"
#include "SharedTypes.h"

namespace PortabilityLayer
{
	class ScanlineMask;
	class RenderedFont;
}

struct Dialog;

enum CopyBitsMode
{
	srcCopy,
	srcXor,
	transparent,
};

enum PenModeID
{
	PenMode_Solid,
	PenMode_Pattern,
	PenMode_PatternInvert,
};

struct CIcon
{
};

struct BitMap
{
	Rect m_rect;
	GpPixelFormat_t m_pixelFormat;
	size_t m_pitch;
	void *m_data;

	void Init(const Rect &rect, GpPixelFormat_t pixelFormat, size_t pitch, void *dataPtr);
};

class RGBColor
{
public:
	RGBColor(uint8_t r, uint8_t g, uint8_t b);
	RGBColor(const RGBColor &other);

	uint8_t GetRed() const;
	uint8_t GetGreen() const;
	uint8_t GetBlue() const;

private:
	uint8_t m_r;
	uint8_t m_g;
	uint8_t m_b;
};

typedef CIcon *CIconPtr;
typedef CIconPtr *CIconHandle;

typedef Byte Pattern[8];

void EndUpdate(WindowPtr graf);

void SetRect(Rect *rect, short left, short top, short right, short bottom);

void ClipRect(const Rect *rect);
void GetClip(Rect *rect);

void InsetRect(Rect *rect, int x, int y);
Pattern *GetQDGlobalsGray(Pattern *pattern);
Pattern *GetQDGlobalsBlack(Pattern *pattern);

// Finds a pattern from a 'PAT#' resource
// Index is 1-based
void GetIndPattern(Pattern *pattern, int patListID, int index);

void DebugPixMap(PixMap **pixMap, const char *outName);

void CopyBits(const BitMap *srcBitmap, BitMap *destBitmap, const Rect *srcRect, const Rect *destRect, CopyBitsMode copyMode);
void CopyBitsConstrained(const BitMap *srcBitmap, BitMap *destBitmap, const Rect *srcRect, const Rect *destRect, CopyBitsMode copyMode, const Rect *constraintRect);
void CopyMask(const BitMap *srcBitmap, const BitMap *maskBitmap, BitMap *destBitmap, const Rect *srcRect, const Rect *maskRect, const Rect *destRect);
void CopyMaskConstrained(const BitMap *srcBitmap, const BitMap *maskBitmap, BitMap *destBitmap, const Rect *srcRectBase, const Rect *maskRectBase, const Rect *destRectBase, const Rect *constraintRect);

void ImageInvert(const PixMap *invertMask, PixMap *targetBitmap, const Rect &srcRect, const Rect &destRect);

bool PointInScanlineMask(Point point, PortabilityLayer::ScanlineMask *scanlineMask);

PixMap *GetPortBitMapForCopyBits(DrawSurface *grafPtr);

Boolean SectRect(const Rect *rectA, const Rect *rectB, Rect *outIntersection);

PortabilityLayer::RenderedFont *GetApplicationFont(int size, int variationFlags, bool aa);
PortabilityLayer::RenderedFont *GetSystemFont(int size, int variationFlags, bool aa);
PortabilityLayer::RenderedFont *GetHandwritingFont(int size, int variationFlags, bool aa);
PortabilityLayer::RenderedFont *GetMonospaceFont(int size, int variationFlags, bool aa);


inline RGBColor::RGBColor(uint8_t r, uint8_t g, uint8_t b)
	: m_r(r)
	, m_g(g)
	, m_b(b)
{
}

inline RGBColor::RGBColor(const RGBColor &other)
	: m_r(other.m_r)
	, m_g(other.m_g)
	, m_b(other.m_b)
{
}
