#include "PLQuickdraw.h"
#include "DisplayDeviceManager.h"

void GetPort(GrafPtr *graf)
{
	PL_NotYetImplemented();
}

void SetPort(GrafPtr graf)
{
	PL_NotYetImplemented();
}

void BeginUpdate(GrafPtr graf)
{
	PL_NotYetImplemented();
}

void EndUpdate(GrafPtr graf)
{
	PL_NotYetImplemented();
}

OSErr GetIconSuite(Handle *suite, short resID, IconSuiteFlags flags)
{
	PL_NotYetImplemented();
	return noErr;
}

OSErr PlotIconSuite(Rect *rect, IconAlignmentType alignType, IconTransformType transformType, Handle iconSuite)
{
	PL_NotYetImplemented();
	return noErr;
}

CIconHandle GetCIcon(short resID)
{
	PL_NotYetImplemented();
	return nullptr;
}

OSErr PlotCIcon(Rect *rect, CIconHandle icon)
{
	PL_NotYetImplemented();
	return noErr;
}

void DisposeCIcon(CIconHandle icon)
{
	PL_NotYetImplemented();
}

void SetRect(Rect *rect, short left, short top, short right, short bottom)
{
	rect->left = left;
	rect->top = top;
	rect->bottom = bottom;
	rect->right = right;
}

GDHandle GetMainDevice()
{
	return PortabilityLayer::DisplayDeviceManager::GetInstance()->GetMainDevice();
}

void SetPortWindowPort(WindowPtr window)
{
	PL_NotYetImplemented();
}

void SetPortDialogPort(Dialog *dialog)
{
	PL_NotYetImplemented();
}


void TextSize(int sz)
{
	PL_NotYetImplemented();
}

void TextFace(int face)
{
	PL_NotYetImplemented();
}

void TextFont(int fontID)
{
	PL_NotYetImplemented();
}

int TextWidth(const PLPasStr &str, int firstChar1Based, int length)
{
	PL_NotYetImplemented();
	return 0;
}

void MoveTo(int x, int y)
{
	PL_NotYetImplemented();
}

void LineTo(int x, int y)
{
	PL_NotYetImplemented();
}

void SetOrigin(int x, int y)
{
	PL_NotYetImplemented();
}

void ForeColor(SystemColorID color)
{
	PL_NotYetImplemented();
}

void BackColor(SystemColorID color)
{
	PL_NotYetImplemented();
}

void GetForeColor(RGBColor *color)
{
	PL_NotYetImplemented();
}

void Index2Color(int index, RGBColor *color)
{
	PL_NotYetImplemented();
}

void RGBForeColor(const RGBColor *color)
{
	PL_NotYetImplemented();
}

void DrawString(const PLPasStr &str)
{
	PL_NotYetImplemented();
}

void PaintRect(const Rect *rect)
{
	PL_NotYetImplemented();
}

void PaintOval(const Rect *rect)
{
	PL_NotYetImplemented();
}

void PaintRgn(RgnHandle region)
{
	PL_NotYetImplemented();
}

void ClipRect(const Rect *rect)
{
	PL_NotYetImplemented();
}

void FrameRect(const Rect *rect)
{
	PL_NotYetImplemented();
}

void FrameOval(const Rect *rect)
{
	PL_NotYetImplemented();
}

void FrameRoundRect(const Rect *rect, int w, int h)
{
	PL_NotYetImplemented();
}

void PenMode(int mode)
{
	PL_NotYetImplemented();
}

void PenMode(PenModeID penMode)
{
	PL_NotYetImplemented();
}

void PenMode(CopyBitsMode copyBitsMode)
{
	PL_NotYetImplemented();
}

void PenPat(const Pattern *pattern)
{
	PL_NotYetImplemented();
}

void PenSize(int w, int h)
{
	PL_NotYetImplemented();
}

void PenNormal()
{
	PL_NotYetImplemented();
}

void EraseRect(const Rect *rect)
{
	PL_NotYetImplemented();
}

void InvertRect(const Rect *rect)
{
	PL_NotYetImplemented();
}

void InsetRect(Rect *rect, int x, int y)
{
	PL_NotYetImplemented();
}

void Line(int x, int y)
{
	PL_NotYetImplemented();
}

Pattern *GetQDGlobalsGray(Pattern *pattern)
{
	PL_NotYetImplemented();
	return nullptr;
}

Pattern *GetQDGlobalsBlack(Pattern *pattern)
{
	PL_NotYetImplemented();
	return nullptr;
}

void GetIndPattern(Pattern *pattern, int patListID, int index)
{
	PL_NotYetImplemented();
}



void CopyBits(const BitMap *srcBitmap, BitMap *destBitmap, const Rect *srcRect, const Rect *destRect, CopyBitsMode copyMode, RgnHandle maskRegion)
{
	PL_NotYetImplemented();
}

void CopyMask(const BitMap *srcBitmap, const BitMap *maskBitmap, BitMap *destBitmap, const Rect *srcRect, const Rect *maskRect, const Rect *destRect)
{
	PL_NotYetImplemented();
}


RgnHandle NewRgn()
{
	PL_NotYetImplemented();
	return nullptr;
}

void RectRgn(RgnHandle region, const Rect *rect)
{
	PL_NotYetImplemented();
}

void UnionRgn(RgnHandle regionA, RgnHandle regionB, RgnHandle regionC)
{
	PL_NotYetImplemented();
}

void DisposeRgn(RgnHandle rgn)
{
	PL_NotYetImplemented();
}

void OpenRgn()
{
	PL_NotYetImplemented();
}

void CloseRgn(RgnHandle rgn)
{
	PL_NotYetImplemented();
}

Boolean PtInRgn(Point point, RgnHandle rgn)
{
	PL_NotYetImplemented();
	return false;
}

void GetClip(RgnHandle rgn)
{
	PL_NotYetImplemented();
}

void SetClip(RgnHandle rgn)
{
	PL_NotYetImplemented();
}


BitMap *GetPortBitMapForCopyBits(CGrafPtr grafPtr)
{
	PL_NotYetImplemented();
	return nullptr;
}

CGrafPtr GetWindowPort(WindowPtr window)
{
	PL_NotYetImplemented();
	return nullptr;
}

RgnHandle GetPortVisibleRegion(CGrafPtr port, RgnHandle region)
{
	PL_NotYetImplemented();
	return nullptr;
}


Int32 DeltaPoint(Point pointA, Point pointB)
{
	PL_NotYetImplemented();
	return 0;
}


void SubPt(Point srcPoint, Point *destPoint)
{
	PL_NotYetImplemented();
}


Boolean SectRect(const Rect *rectA, const Rect *rectB, Rect *outIntersection)
{
	PL_NotYetImplemented();
	return false;
}


Boolean PtInRect(Point point, const Rect *rect)
{
	PL_NotYetImplemented();
	return false;
}


void RestoreDeviceClut(void *unknown)
{
	PL_NotYetImplemented();
}

void PaintBehind(void *unknown, RgnHandle region)
{
	PL_NotYetImplemented();
}

RgnHandle GetGrayRgn()
{
	PL_NotYetImplemented();
	return nullptr;
}
