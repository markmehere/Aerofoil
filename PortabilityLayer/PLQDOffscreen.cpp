#include "PLQDOffscreen.h"


OSErr NewGWorld(GWorldPtr *gworld, int depth, Rect *bounds, CTabHandle colorTable, GDHandle device, int flags)
{
	PL_NotYetImplemented();
	return noErr;
}

void DisposeGWorld(GWorldPtr gworld)
{
	PL_NotYetImplemented();
}

PixMapHandle GetGWorldPixMap(GWorldPtr gworld)
{
	PL_NotYetImplemented();
	return nullptr;
}

void LockPixels(PixMapHandle pixmap)
{
	PL_NotYetImplemented();
}

PicHandle GetPicture(short resID)
{
	PL_NotYetImplemented();
	return nullptr;
}

void OffsetRect(Rect *rect, int right, int down)
{
	PL_NotYetImplemented();
}

void DrawPicture(PicHandle pict, Rect *bounds)
{
	PL_NotYetImplemented();
}

void GetGWorld(CGrafPtr *gw, GDHandle *gdHandle)
{
	PL_NotYetImplemented();
}

void SetGWorld(CGrafPtr gw, GDHandle gdHandle)
{
	PL_NotYetImplemented();
}
