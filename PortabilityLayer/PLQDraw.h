#pragma once

#include "PLCore.h"
#include "QDGraf.h"
#include "SharedTypes.h"

namespace PortabilityLayer
{
	class ScanlineMask;
}

struct Dialog;


enum IconSuiteFlags
{
	svAllLargeData = 1,
};

enum TextFlags
{
	bold = 1,
};

enum SystemFontID
{
	systemFont = 0,	// System font
	applFont = 1,	// Application font
};

enum SystemColorID
{
	whiteColor = 1,
	blackColor,
	yellowColor,
	magentaColor,
	redColor,
	cyanColor,
	greenColor,
	blueColor,
};

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

typedef PortabilityLayer::QDPort GrafPort;
typedef GrafPort *GrafPtr;

typedef Byte Pattern[8];

void GetPort(GrafPtr *graf);
void SetPort(GrafPtr graf);
void SetPortWindowPort(WindowPtr window);
void SetPortDialogPort(Dialog *dialog);

void EndUpdate(WindowPtr graf);

PLError_t GetIconSuite(Handle *suite, short resID, IconSuiteFlags flags);
PLError_t PlotIconSuite(Rect *rect, Handle iconSuite);

void SetRect(Rect *rect, short left, short top, short right, short bottom);

int TextWidth(const PLPasStr &str, int firstChar1Based, int length);
void ForeColor(SystemColorID color);
void BackColor(SystemColorID color);
void GetForeColor(RGBColor *color);

void ClipRect(const Rect *rect);
void GetClip(Rect *rect);

void PenSize(int w, int h);
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
DrawSurface *GetWindowPort(WindowPtr window);

// Subtracts srcPoint from destPoint (reverse of DeltaPoint)
void SubPt(Point srcPoint, Point *destPoint);

Boolean SectRect(const Rect *rectA, const Rect *rectB, Rect *outIntersection);

void RestoreDeviceClut(void *unknown);


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
