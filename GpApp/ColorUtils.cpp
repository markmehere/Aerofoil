
//============================================================================
//----------------------------------------------------------------------------
//								  ColorUtils.c
//----------------------------------------------------------------------------
//============================================================================


#include "Externs.h"
#include "PLPasStr.h"
#include "QDStandardPalette.h"
#include "ResolveCachingColor.h"


//==============================================================  Functions
//--------------------------------------------------------------  ColorText

// Given a string and a color index (index into the current palette),…
// this function draws text in that color.  It assumes the current port,…
// the current font, the current pen location, etc.

void ColorText (DrawSurface *surface, const Point &point, StringPtr theStr, long color, PortabilityLayer::RenderedFont *font)
{
	PortabilityLayer::ResolveCachingColor rColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(color);

	surface->DrawString(point, theStr, rColor, font);
}

//--------------------------------------------------------------  ColorRect

// Given a rectangle and color index, this function draws a solid…
// rectangle in that color.  Current port, pen mode, etc. assumed.

void ColorRect (DrawSurface *surface, const Rect &theRect, long color)
{
	PortabilityLayer::ResolveCachingColor rColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(color);

	surface->FillRect(theRect, rColor);
}

//--------------------------------------------------------------  ColorOval

// Given a rectangle and color index, this function draws a solid…
// oval in that color.  Current port, pen mode, etc. assumed.

void ColorOval (DrawSurface *surface, const Rect &theRect, long color)
{
	PortabilityLayer::ResolveCachingColor rColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(color);

	surface->FillEllipse(theRect, rColor);
}

void ColorOvalMaskPattern(DrawSurface *surface, const Rect &theRect, long color, const uint8_t *pattern)
{
	PortabilityLayer::ResolveCachingColor rColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(color);

	surface->FillEllipseWithMaskPattern(theRect, pattern, rColor);
}

//--------------------------------------------------------------  ColorRegionMaskPattern

// Given a region and color index, this function draws a solid…
// region in that color.  Current port, pen mode, etc. assumed.

void ColorRegionMaskPattern (DrawSurface *surface, PortabilityLayer::ScanlineMask *scanlineMask, long colorIndex, const uint8_t *pattern)
{
	PortabilityLayer::ResolveCachingColor rColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(colorIndex);
	surface->FillScanlineMaskWithMaskPattern(scanlineMask, pattern, rColor);
}

//--------------------------------------------------------------  ColorLine

// Given a the end points for a line and color index, this function…
// draws a line in that color.  Current port, pen mode, etc. assumed.

void ColorLine (DrawSurface *surface, short h0, short v0, short h1, short v1, long color)
{
	PortabilityLayer::ResolveCachingColor cacheColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(color);

	surface->DrawLine(Point::Create(h0, v0), Point::Create(h1, v1), cacheColor);
}

//--------------------------------------------------------------  HiliteRect

// Given a rect and two hilite colors, this function frames the top and…
// left edges of the rect with color 1 and frames the bottom and right…
// sides with color 2.  A rect can be made to appear "hi-lit" or "3D"…
// in this way.

void HiliteRect (DrawSurface *surface, const Rect &theRect, short color1, short color2)
{
	ColorLine(surface, theRect.left, theRect.top, theRect.right - 2,
			theRect.top, color1);
	ColorLine(surface, theRect.left, theRect.top, theRect.left,
			theRect.bottom - 2, color1);
	ColorLine(surface, theRect.right - 1, theRect.top, theRect.right - 1,
			theRect.bottom - 2, color2);
	ColorLine(surface, theRect.left + 1, theRect.bottom - 1, theRect.right - 1,
			theRect.bottom - 1, color2);
}

//--------------------------------------------------------------  ColorFrameRect

// Given a rectangle and color index, this function frames a…
// rectangle in that color.  Current port, pen mode, etc. assumed.

void ColorFrameRect (DrawSurface *surface, const Rect &theRect, long color)
{
	PortabilityLayer::ResolveCachingColor rColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(color);

	surface->FrameRect(theRect, rColor);
}

//--------------------------------------------------------------  ColorFrameWHRect

// Given a the top-left corner of a rectangle, its width and height,…
// and a color index, this function frames a rectangle in that color.
// Current port, pen mode, etc. assumed.

void ColorFrameWHRect (DrawSurface *surface, short left, short top, short wide, short high, long color)
{
	Rect		theRect;
	
	theRect.left = left;
	theRect.top = top;
	theRect.right = left + wide;
	theRect.bottom = top + high;
	ColorFrameRect(surface, theRect, color);
}

//--------------------------------------------------------------  ColorFrameOval

// Given a rectangle and color index, this function frames an…
// oval in that color.  Current port, pen mode, etc. assumed.

void ColorFrameOval (DrawSurface *surface, const Rect &theRect, long color)
{
	PortabilityLayer::ResolveCachingColor rColor = PortabilityLayer::ResolveCachingColor::FromStandardColor(color);

	surface->FrameEllipse(theRect, rColor);
}

//--------------------------------------------------------------  LtGrayForeColor

// This function finds the closest match to a "light gray" in the…
// current palette and sets the pen color to that.

PortabilityLayer::ResolveCachingColor LtGrayForeColor ()
{
	return PortabilityLayer::RGBAColor::Create(191, 191, 191, 255);
}

//--------------------------------------------------------------  GrayForeColor

// This function finds the closest match to a "medium gray" in the…
// current palette and sets the pen color to that.

PortabilityLayer::ResolveCachingColor GrayForeColor ()
{
	return PortabilityLayer::RGBAColor::Create(127, 127, 127, 255);
}

//--------------------------------------------------------------  DkGrayForeColor

// This function finds the closest match to a "dark gray" in the…
// current palette and sets the pen color to that.

PortabilityLayer::ResolveCachingColor DkGrayForeColor ()
{
	return PortabilityLayer::RGBAColor::Create(63, 63, 63, 255);
}
