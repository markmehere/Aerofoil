
//============================================================================
//----------------------------------------------------------------------------
//								  ColorUtils.c
//----------------------------------------------------------------------------
//============================================================================


#include "Externs.h"
#include "PLPalettes.h"
#include "PLPasStr.h"
#include "QDStandardPalette.h"


//==============================================================  Functions
//--------------------------------------------------------------  ColorText

// Given a string and a color index (index into the current palette),É
// this function draws text in that color.  It assumes the current port,É
// the current font, the current pen location, etc.

void ColorText (DrawSurface *surface, const Point &point, StringPtr theStr, long color)
{
	const PortabilityLayer::RGBAColor &rgbaColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[color];

	const PortabilityLayer::RGBAColor wasColor = surface->GetForeColor();
	surface->SetForeColor(rgbaColor);
	surface->DrawString(point, theStr, true);
	surface->SetForeColor(wasColor);
}

//--------------------------------------------------------------  ColorRect

// Given a rectangle and color index, this function draws a solidÉ
// rectangle in that color.  Current port, pen mode, etc. assumed.

void ColorRect (DrawSurface *surface, const Rect &theRect, long color)
{
	const PortabilityLayer::RGBAColor &rgbaColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[color];

	const PortabilityLayer::RGBAColor wasColor = surface->GetForeColor();
	surface->SetForeColor(rgbaColor);
	surface->FillRect(theRect);
	surface->SetForeColor(wasColor);
}

//--------------------------------------------------------------  ColorOval

// Given a rectangle and color index, this function draws a solidÉ
// oval in that color.  Current port, pen mode, etc. assumed.

void ColorOval (DrawSurface *surface, const Rect &theRect, long color)
{
	const PortabilityLayer::RGBAColor &rgbaColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[color];

	const PortabilityLayer::RGBAColor wasColor = surface->GetForeColor();
	surface->SetForeColor(rgbaColor);
	surface->FillEllipse(theRect);
	surface->SetForeColor(wasColor);
}

void ColorOvalMaskPattern(DrawSurface *surface, const Rect &theRect, long color, bool isMask, const uint8_t *pattern)
{
	const PortabilityLayer::RGBAColor &rgbaColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[color];

	const PortabilityLayer::RGBAColor wasColor = surface->GetForeColor();
	surface->SetForeColor(rgbaColor);
	surface->FillEllipseWithPattern(theRect, isMask, pattern);
	surface->SetForeColor(wasColor);
}

//--------------------------------------------------------------  ColorRegionMaskPattern

// Given a region and color index, this function draws a solidÉ
// region in that color.  Current port, pen mode, etc. assumed.

void ColorRegionMaskPattern (DrawSurface *surface, PortabilityLayer::ScanlineMask *scanlineMask, long colorIndex, bool isMask, const uint8_t *pattern)
{
	const PortabilityLayer::RGBAColor &rgbaColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[colorIndex];

	const PortabilityLayer::RGBAColor wasColor = surface->GetForeColor();
	surface->SetForeColor(rgbaColor);
	surface->FillScanlineMaskWithPattern(scanlineMask, isMask, pattern);
	surface->SetForeColor(wasColor);
}

//--------------------------------------------------------------  ColorLine

// Given a the end points for a line and color index, this functionÉ
// draws a line in that color.  Current port, pen mode, etc. assumed.

void ColorLine (DrawSurface *surface, short h0, short v0, short h1, short v1, long color)
{
	const PortabilityLayer::RGBAColor &rgbaColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[color];

	const PortabilityLayer::RGBAColor wasColor = surface->GetForeColor();
	surface->SetForeColor(rgbaColor);
	surface->DrawLine(Point::Create(h0, v0), Point::Create(h1, v1));
	surface->SetForeColor(wasColor);
}

//--------------------------------------------------------------  HiliteRect

// Given a rect and two hilite colors, this function frames the top andÉ
// left edges of the rect with color 1 and frames the bottom and rightÉ
// sides with color 2.  A rect can be made to appear "hi-lit" or "3D"É
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

// Given a rectangle and color index, this function frames aÉ
// rectangle in that color.  Current port, pen mode, etc. assumed.

void ColorFrameRect (DrawSurface *surface, const Rect &theRect, long color)
{
	const PortabilityLayer::RGBAColor &rgbaColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[color];

	const PortabilityLayer::RGBAColor wasColor = surface->GetForeColor();
	surface->SetForeColor(rgbaColor);
	surface->FrameRect(theRect);
	surface->SetForeColor(wasColor);
}

//--------------------------------------------------------------  ColorFrameWHRect

// Given a the top-left corner of a rectangle, its width and height,É
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

// Given a rectangle and color index, this function frames anÉ
// oval in that color.  Current port, pen mode, etc. assumed.

void ColorFrameOval (DrawSurface *surface, const Rect &theRect, long color)
{
	const PortabilityLayer::RGBAColor &rgbaColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[color];

	const PortabilityLayer::RGBAColor wasColor = surface->GetForeColor();
	surface->SetForeColor(rgbaColor);
	surface->FrameEllipse(theRect);
	surface->SetForeColor(wasColor);
}

//--------------------------------------------------------------  LtGrayForeColor

// This function finds the closest match to a "light gray" in theÉ
// current palette and sets the pen color to that.

void LtGrayForeColor (DrawSurface *surface)
{
	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(191, 191, 191, 255));
}

//--------------------------------------------------------------  GrayForeColor

// This function finds the closest match to a "medium gray" in theÉ
// current palette and sets the pen color to that.

void GrayForeColor (DrawSurface *surface)
{
	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(127, 127, 127, 255));
}

//--------------------------------------------------------------  DkGrayForeColor

// This function finds the closest match to a "dark gray" in theÉ
// current palette and sets the pen color to that.

void DkGrayForeColor (DrawSurface *surface)
{
	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(63, 63, 63, 255));
}
