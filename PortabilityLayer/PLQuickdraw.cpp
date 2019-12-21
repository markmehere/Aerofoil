#include "PLQuickdraw.h"
#include "QDManager.h"
#include "QDState.h"
#include "DisplayDeviceManager.h"
#include "MMHandleBlock.h"
#include "MemoryManager.h"
#include "ResourceManager.h"
#include "ResTypeID.h"
#include "RGBAColor.h"
#include "QDStandardPalette.h"
#include "WindowManager.h"
#include "QDGraf.h"
#include "QDPixMap.h"
#include "QDUtils.h"

#include <assert.h>

void GetPort(GrafPtr *graf)
{
	PL_NotYetImplemented();
}

void SetPort(GrafPtr graf)
{
	PortabilityLayer::QDManager *qd = PortabilityLayer::QDManager::GetInstance();

	GDHandle device;
	qd->GetPort(nullptr, &device);
	qd->SetPort(graf, device);
}

void BeginUpdate(WindowPtr graf)
{
	(void)graf;
}

void EndUpdate(WindowPtr graf)
{
	graf->m_graf.m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
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
	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();

	GDevice **device = wm->GetWindowDevice(window);

	PortabilityLayer::QDManager::GetInstance()->SetPort(&window->m_graf.m_port, device);
}

void SetPortDialogPort(Dialog *dialog)
{
	PL_NotYetImplemented();
}

void TextSize(int sz)
{
	PortabilityLayer::QDManager::GetInstance()->GetState()->m_textSize = sz;
}

void TextFace(int face)
{
	PortabilityLayer::QDManager::GetInstance()->GetState()->m_textFace = face;
}

void TextFont(int fontID)
{
	PortabilityLayer::QDManager::GetInstance()->GetState()->m_fontID = fontID;
}

int TextWidth(const PLPasStr &str, int firstChar1Based, int length)
{
	PL_NotYetImplemented();
	return 0;
}

void MoveTo(int x, int y)
{
	Point &penPos = PortabilityLayer::QDManager::GetInstance()->GetState()->m_penPos;

	penPos.h = x;
	penPos.v = y;
}

void LineTo(int x, int y)
{
	PL_NotYetImplemented();
}

void SetOrigin(int x, int y)
{
	PL_NotYetImplemented();
}

namespace
{
	static bool SystemColorToRGBAColor(SystemColorID color, PortabilityLayer::RGBAColor &rgbaColor)
	{
		switch (color)
		{
		default:
			return false;
		case whiteColor:
			rgbaColor.r = rgbaColor.g = rgbaColor.b = 255;
			break;
		case blackColor:
			rgbaColor.r = rgbaColor.g = rgbaColor.b = 0;
			break;
		case yellowColor:
			rgbaColor.r = rgbaColor.g = 255;
			rgbaColor.b = 0;
			break;
		case magentaColor:
			rgbaColor.r = rgbaColor.b = 255;
			rgbaColor.g = 0;
			break;
		case redColor:
			rgbaColor.r = 255;
			rgbaColor.g = rgbaColor.b = 0;
			break;
		case cyanColor:
			rgbaColor.g = rgbaColor.b = 255;
			rgbaColor.r = 0;
			break;
		case greenColor:
			rgbaColor.g = 255;
			rgbaColor.r = rgbaColor.b = 0;
			break;
		case blueColor:
			rgbaColor.b = 255;
			rgbaColor.r = rgbaColor.g = 0;
			break;
		}
		rgbaColor.a = 255;

		return true;
	}
}

void ForeColor(SystemColorID color)
{
	PortabilityLayer::RGBAColor rgbaColor;
	if (SystemColorToRGBAColor(color, rgbaColor))
	{
		PortabilityLayer::QDState *qdState = PortabilityLayer::QDManager::GetInstance()->GetState();
		qdState->SetForeColor(rgbaColor);
	}
}

void BackColor(SystemColorID color)
{
	PortabilityLayer::RGBAColor rgbaColor;
	if (SystemColorToRGBAColor(color, rgbaColor))
	{
		PortabilityLayer::QDState *qdState = PortabilityLayer::QDManager::GetInstance()->GetState();
		qdState->SetBackColor(rgbaColor);
	}
}

void GetForeColor(RGBColor *color)
{
	const PortabilityLayer::RGBAColor foreColor = PortabilityLayer::QDManager::GetInstance()->GetState()->GetForeColor();
	color->red = foreColor.r * 0x0101;
	color->green = foreColor.g * 0x0101;
	color->blue = foreColor.b * 0x0101;
}

void Index2Color(int index, RGBColor *color)
{
	PortabilityLayer::QDPort *port;

	PortabilityLayer::QDManager::GetInstance()->GetPort(&port, nullptr);

	PortabilityLayer::PixelFormat pf = port->GetPixelFormat();
	if (pf == PortabilityLayer::PixelFormat_8BitCustom)
	{
		PL_NotYetImplemented();
	}
	else
	{
		const PortabilityLayer::RGBAColor color8 = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[index];
		color->red = color8.r * 0x0101;
		color->green = color8.g * 0x0101;
		color->blue = color8.b * 0x0101;
	}
}

void RGBForeColor(const RGBColor *color)
{
	PortabilityLayer::RGBAColor truncatedColor;
	truncatedColor.r = (color->red >> 8);
	truncatedColor.g = (color->green >> 8);
	truncatedColor.b = (color->blue >> 8);
	truncatedColor.a = 255;

	PortabilityLayer::QDManager::GetInstance()->GetState()->SetForeColor(truncatedColor);
}

void DrawString(const PLPasStr &str)
{
	PL_NotYetImplemented_TODO("Text");
}

void PaintRect(const Rect *rect)
{
	if (!rect->IsValid())
		return;

	PortabilityLayer::QDPort *qdPort;
	PortabilityLayer::QDManager::GetInstance()->GetPort(&qdPort, nullptr);

	PortabilityLayer::PixelFormat pixelFormat = qdPort->GetPixelFormat();

	Rect constrainedRect = *rect;

	PortabilityLayer::QDState *qdState = qdPort->GetState();

	if (qdState->m_clipRegion)
	{
		const Region &region = **qdState->m_clipRegion;

		if (region.size > sizeof(Region))
			PL_NotYetImplemented();

		constrainedRect = constrainedRect.Intersect(region.rect);
	}

	constrainedRect = constrainedRect.Intersect(qdPort->GetRect());

	if (!constrainedRect.IsValid())
		return;

	PortabilityLayer::PixMapImpl *pixMap = static_cast<PortabilityLayer::PixMapImpl*>(*qdPort->GetPixMap());
	const size_t pitch = pixMap->GetPitch();
	const size_t firstIndex = static_cast<size_t>(constrainedRect.top) * pitch + static_cast<size_t>(constrainedRect.left);
	const size_t numLines = static_cast<size_t>(constrainedRect.bottom - constrainedRect.top);
	const size_t numCols = static_cast<size_t>(constrainedRect.right - constrainedRect.left);
	uint8_t *pixData = static_cast<uint8_t*>(pixMap->GetPixelData());

	switch (pixelFormat)
	{
	case PortabilityLayer::PixelFormat_8BitStandard:
		{
			const uint8_t color = qdState->ResolveForeColor8(nullptr, 0);

			size_t scanlineIndex = 0;
			for (size_t ln = 0; ln < numLines; ln++)
			{
				const size_t firstLineIndex = firstIndex + ln * pitch;
				for (size_t col = 0; col < numCols; col++)
					pixData[firstLineIndex + col] = color;
			}
		}
		break;
	default:
		PL_NotYetImplemented();
		return;
	}
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
	if (!rect->IsValid())
		return;

	PortabilityLayer::QDState *qdState = PortabilityLayer::QDManager::GetInstance()->GetState();
	if (!qdState->m_clipRegion)
		qdState->m_clipRegion = PortabilityLayer::QDUtils::CreateRegion(*rect);
	else
		PortabilityLayer::QDUtils::ResetRegionToRect(qdState->m_clipRegion, *rect);
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

void PenMode(CopyBitsMode copyBitsMode)
{
	PL_NotYetImplemented();
}

void PenMode(PenModeID penMode)
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
	if (index < 1)
		return;

	PortabilityLayer::MMHandleBlock *patternList = PortabilityLayer::ResourceManager::GetInstance()->GetResource('PAT#', patListID);
	const uint8_t *patternRes = static_cast<const uint8_t*>(patternList->m_contents);

	int numPatterns = (patternRes[0] << 8) | patternRes[1];
	if (index > numPatterns)
		return;

	memcpy(pattern, patternRes + 2 + (index - 1) * 8, 8);
}



void CopyBits(const BitMap *srcBitmap, BitMap *destBitmap, const Rect *srcRect, const Rect *destRect, CopyBitsMode copyMode, RgnHandle maskRegion)
{
	assert(srcBitmap->m_pixelFormat == destBitmap->m_pixelFormat);

	const Rect &srcBounds = srcBitmap->m_rect;
	const Rect &destBounds = destBitmap->m_rect;
	const PortabilityLayer::PixelFormat pixelFormat = srcBitmap->m_pixelFormat;
	const size_t srcPitch = srcBitmap->m_pitch;
	const size_t destPitch = destBitmap->m_pitch;

	assert(srcRect->top >= srcBounds.top);
	assert(srcRect->bottom <= srcBounds.bottom);
	assert(srcRect->left >= srcBounds.left);
	assert(srcRect->right <= srcBounds.right);

	assert(destRect->top >= destBounds.top);
	assert(destRect->bottom <= destBounds.bottom);
	assert(destRect->left >= destBounds.left);
	assert(destRect->right <= destBounds.right);

	assert(srcRect->right - srcRect->left == destRect->right - destRect->left);
	assert(srcRect->bottom - srcRect->top == destRect->bottom - destRect->top);

	const Region *mask = *maskRegion;

	const Rect constrainedDestRect = destRect->Intersect(mask->rect);
	if (!constrainedDestRect.IsValid())
		return;

	Rect constrainedSrcRect = *srcRect;
	constrainedSrcRect.left += constrainedDestRect.left - destRect->left;
	constrainedSrcRect.right += constrainedDestRect.right - destRect->right;
	constrainedSrcRect.top += constrainedDestRect.top - destRect->top;
	constrainedSrcRect.bottom += constrainedDestRect.bottom - destRect->bottom;

	const size_t srcFirstCol = constrainedSrcRect.left - srcBitmap->m_rect.left;
	const size_t srcFirstRow = constrainedSrcRect.top - srcBitmap->m_rect.top;

	const size_t destFirstCol = constrainedDestRect.left - destBitmap->m_rect.left;
	const size_t destFirstRow = constrainedDestRect.top - destBitmap->m_rect.top;

	if (mask->size != sizeof(Region))
	{
		PL_NotYetImplemented();
	}
	else
	{
		size_t pixelSizeBytes = 0;

		switch (pixelFormat)
		{
		case PortabilityLayer::PixelFormat_8BitCustom:
		case PortabilityLayer::PixelFormat_8BitStandard:
			pixelSizeBytes = 1;
			break;
		case PortabilityLayer::PixelFormat_RGB555:
			pixelSizeBytes = 2;
			break;
		case PortabilityLayer::PixelFormat_RGB24:
			pixelSizeBytes = 3;
			break;
		case PortabilityLayer::PixelFormat_RGB32:
			pixelSizeBytes = 4;
			break;
		};

		const uint8_t *srcBytes = static_cast<const uint8_t*>(srcBitmap->m_data);
		uint8_t *destBytes = static_cast<uint8_t*>(destBitmap->m_data);

		const size_t firstSrcByte = srcFirstRow * srcPitch + srcFirstCol * pixelSizeBytes;
		const size_t firstDestByte = destFirstRow * destPitch + destFirstCol * pixelSizeBytes;

		const size_t numCopiedRows = srcRect->bottom - srcRect->top;
		const size_t numCopiedCols = srcRect->right - srcRect->left;
		const size_t numCopiedBytesPerScanline = numCopiedCols * pixelSizeBytes;

		for (size_t i = 0; i < numCopiedRows; i++)
			memcpy(destBytes + firstDestByte + i * destPitch, srcBytes + firstSrcByte + i * srcPitch, numCopiedBytesPerScanline);
	}
}

void CopyMask(const BitMap *srcBitmap, const BitMap *maskBitmap, BitMap *destBitmap, const Rect *srcRect, const Rect *maskRect, const Rect *destRect)
{
	PL_NotYetImplemented();
}


RgnHandle NewRgn()
{
	PortabilityLayer::MMHandleBlock *handle = PortabilityLayer::MemoryManager::GetInstance()->AllocHandle(sizeof(Region));

	Region *rgn = static_cast<Region*>(handle->m_contents);

	rgn->size = sizeof(Region);
	rgn->rect = Rect::Create(0, 0, 0, 0);

	return reinterpret_cast<Region**>(&handle->m_contents);
}

void RectRgn(RgnHandle region, const Rect *rect)
{
	Region *rgn = *region;

	if (rgn->size != sizeof(Region))
	{
		PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();
		PortabilityLayer::MMHandleBlock *hdlBlock = reinterpret_cast<PortabilityLayer::MMHandleBlock*>(region);

		// OK if this fails, I guess
		if (mm->ResizeHandle(hdlBlock, sizeof(Region)))
			rgn = static_cast<Region*>(hdlBlock->m_contents);

		rgn->size = sizeof(Region);
	}

	rgn->rect = *rect;
}

void UnionRgn(RgnHandle regionA, RgnHandle regionB, RgnHandle regionC)
{
	PL_NotYetImplemented();
}

void DisposeRgn(RgnHandle rgn)
{
	DisposeHandle(reinterpret_cast<Handle>(rgn));
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
	return *grafPtr->m_port.GetPixMap();
}

CGrafPtr GetWindowPort(WindowPtr window)
{
	return &window->m_graf;
}

RgnHandle GetPortVisibleRegion(CGrafPtr port, RgnHandle region)
{
	const Rect rect = port->m_port.GetRect();

	RectRgn(region, &rect);
	return region;
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

void BitMap::Init(const Rect &rect, PortabilityLayer::PixelFormat pixelFormat, size_t pitch, void *dataPtr)
{
	m_rect = rect;
	m_pixelFormat = pixelFormat;
	m_pitch = pitch;
	m_data = dataPtr;
}
