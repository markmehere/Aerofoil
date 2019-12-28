#include "PLQDraw.h"
#include "QDManager.h"
#include "QDState.h"
#include "DisplayDeviceManager.h"
#include "FontFamily.h"
#include "FontManager.h"
#include "LinePlotter.h"
#include "MMHandleBlock.h"
#include "MemoryManager.h"
#include "HostFontHandler.h"
#include "PLPasStr.h"
#include "RenderedFont.h"
#include "RenderedGlyphMetrics.h"
#include "Rect2i.h"
#include "ResourceManager.h"
#include "ResTypeID.h"
#include "RGBAColor.h"
#include "QDStandardPalette.h"
#include "WindowManager.h"
#include "QDGraf.h"
#include "QDPixMap.h"
#include "QDUtils.h"
#include "Vec2i.h"

#include <algorithm>
#include <assert.h>


enum PaintColorResolution
{
	PaintColorResolution_Fore,
	PaintColorResolution_Back,
	PaintColorResolution_Pen,
};

static void PaintRectWithPCR(const Rect &rect, PaintColorResolution pcr);

void GetPort(GrafPtr *graf)
{
	PL_NotYetImplemented();
}

void SetPort(GrafPtr graf)
{
	PortabilityLayer::QDManager::GetInstance()->SetPort(graf);
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

void SetPortWindowPort(WindowPtr window)
{
	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();
	PortabilityLayer::QDManager::GetInstance()->SetPort(&window->m_graf.m_port);
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

static void PlotLine(PortabilityLayer::QDState *qdState, PortabilityLayer::QDPort *qdPort, const PortabilityLayer::Vec2i &pointA, const PortabilityLayer::Vec2i &pointB)
{
	const Rect lineRect = Rect::Create(
		std::min(pointA.m_y, pointB.m_y),
		std::min(pointA.m_x, pointB.m_x),
		std::max(pointA.m_y, pointB.m_y) + 1,
		std::max(pointA.m_x, pointB.m_x) + 1);

	// If the points are a straight line, paint as a rect
	if (pointA.m_y == pointB.m_y || pointA.m_x == pointB.m_x)
	{
		PaintRectWithPCR(lineRect, PaintColorResolution_Fore);
		return;
	}

	GpPixelFormat_t pixelFormat = qdPort->GetPixelFormat();

	Rect constrainedRect = qdPort->GetRect();

	if (qdState->m_clipRegion)
	{
		const Region &region = **qdState->m_clipRegion;

		if (region.size > sizeof(Region))
			PL_NotYetImplemented();

		constrainedRect = constrainedRect.Intersect(region.rect);
	}

	constrainedRect = constrainedRect.Intersect(lineRect);

	if (!constrainedRect.IsValid())
		return;

	PortabilityLayer::Vec2i upperPoint = pointA;
	PortabilityLayer::Vec2i lowerPoint = pointB;

	if (pointA.m_y > pointB.m_y)
		std::swap(upperPoint, lowerPoint);

	PortabilityLayer::PixMapImpl *pixMap = static_cast<PortabilityLayer::PixMapImpl*>(*qdPort->GetPixMap());
	const size_t pitch = pixMap->GetPitch();
	uint8_t *pixData = static_cast<uint8_t*>(pixMap->GetPixelData());

	PortabilityLayer::LinePlotter plotter;
	plotter.Reset(upperPoint, lowerPoint);

	PortabilityLayer::Vec2i currentPoint = upperPoint;
	while (currentPoint.m_x < constrainedRect.left || currentPoint.m_y < constrainedRect.top || currentPoint.m_x >= constrainedRect.right)
	{
		PortabilityLayer::PlotDirection plotDir = plotter.PlotNext();
		if (plotDir == PortabilityLayer::PlotDirection_Exhausted)
			return;

		currentPoint = plotter.GetPoint();
	}

	assert(currentPoint.m_y < constrainedRect.bottom);

	size_t plotIndex = static_cast<size_t>(currentPoint.m_y) * pitch + static_cast<size_t>(currentPoint.m_x);
	const size_t plotLimit = pixMap->GetPitch() * (pixMap->m_rect.bottom - pixMap->m_rect.top);

	switch (pixelFormat)
	{
	case GpPixelFormats::k8BitStandard:
		{
			const size_t pixelSize = 1;
			const uint8_t color = qdState->ResolveForeColor8(nullptr, 0);

			while (currentPoint.m_x >= constrainedRect.left && currentPoint.m_x < constrainedRect.right && currentPoint.m_y < constrainedRect.bottom)
			{
				assert(plotIndex < plotLimit);
				pixData[plotIndex] = color;

				PortabilityLayer::PlotDirection plotDir = plotter.PlotNext();
				if (plotDir == PortabilityLayer::PlotDirection_Exhausted)
					return;

				switch (plotDir)
				{
				default:
				case PortabilityLayer::PlotDirection_Exhausted:
					return;

				case PortabilityLayer::PlotDirection_NegX_NegY:
				case PortabilityLayer::PlotDirection_0X_NegY:
				case PortabilityLayer::PlotDirection_PosX_NegY:
					// These should never happen, the point order is swapped so that Y is always 0 or positive
					assert(false);
					return;

				case PortabilityLayer::PlotDirection_NegX_PosY:
					currentPoint.m_x--;
					currentPoint.m_y++;
					plotIndex = plotIndex + pitch - pixelSize;
					break;
				case PortabilityLayer::PlotDirection_0X_PosY:
					currentPoint.m_y++;
					plotIndex = plotIndex + pitch;
					break;
				case PortabilityLayer::PlotDirection_PosX_PosY:
					currentPoint.m_x++;
					currentPoint.m_y++;
					plotIndex = plotIndex + pitch + pixelSize;
					break;

				case PortabilityLayer::PlotDirection_NegX_0Y:
					currentPoint.m_x--;
					plotIndex = plotIndex - pixelSize;
					break;
				case PortabilityLayer::PlotDirection_PosX_0Y:
					currentPoint.m_x++;
					plotIndex = plotIndex + pixelSize;
					break;
				}
			}
		}
		break;
	default:
		PL_NotYetImplemented();
		return;
	}
}

void LineTo(int x, int y)
{
	PortabilityLayer::QDManager *qdManager = PortabilityLayer::QDManager::GetInstance();
	PortabilityLayer::QDState *qdState = qdManager->GetState();

	PlotLine(qdState, qdManager->GetPort(), PortabilityLayer::Vec2i(qdState->m_penPos.h, qdState->m_penPos.v), PortabilityLayer::Vec2i(x, y));
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
	PortabilityLayer::QDPort *port = PortabilityLayer::QDManager::GetInstance()->GetPort();

	GpPixelFormat_t pf = port->GetPixelFormat();
	if (pf == GpPixelFormats::k8BitCustom)
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

static void DrawGlyph(PortabilityLayer::QDState *qdState, PixMap *pixMap, const Rect &rect, Point &penPos, const PortabilityLayer::RenderedFont *rfont, unsigned int character)
{
	assert(rect.IsValid());

	const PortabilityLayer::RenderedGlyphMetrics *metrics;
	const void *data;
	if (!rfont->GetGlyph(character, &metrics, &data))
		return;

	const Point originalPoint = penPos;

	penPos.h += metrics->m_advanceX;

	const int32_t leftCoord = originalPoint.h + metrics->m_bearingX;
	const int32_t topCoord = originalPoint.v - metrics->m_bearingY;
	const int32_t rightCoord = leftCoord + metrics->m_glyphWidth;
	const int32_t bottomCoord = topCoord + metrics->m_glyphHeight;

	const int32_t clampedLeftCoord = std::max<int32_t>(leftCoord, rect.left);
	const int32_t clampedTopCoord = std::max<int32_t>(topCoord, rect.top);
	const int32_t clampedRightCoord = std::min<int32_t>(rightCoord, rect.right);
	const int32_t clampedBottomCoord = std::min<int32_t>(bottomCoord, rect.bottom);

	if (clampedLeftCoord >= clampedRightCoord || clampedTopCoord >= clampedBottomCoord)
		return;

	const uint32_t firstOutputRow = clampedTopCoord - rect.top;
	const uint32_t firstOutputCol = clampedLeftCoord - rect.left;

	const uint32_t firstInputRow = clampedTopCoord - topCoord;
	const uint32_t firstInputCol = clampedLeftCoord - leftCoord;

	const uint32_t numCols = clampedRightCoord - clampedLeftCoord;
	const uint32_t numRows = clampedBottomCoord - clampedTopCoord;

	const size_t inputPitch = metrics->m_glyphDataPitch;
	const size_t outputPitch = pixMap->m_pitch;
	const uint8_t *firstInputRowData = static_cast<const uint8_t*>(data) + firstInputRow * inputPitch;

	switch (pixMap->m_pixelFormat)
	{
	case GpPixelFormats::k8BitStandard:
		{
			uint8_t *firstOutputRowData = static_cast<uint8_t*>(pixMap->m_data) + firstOutputRow * outputPitch;

			const uint8_t color = qdState->ResolveForeColor8(nullptr, 0);
			for (uint32_t row = 0; row < numRows; row++)
			{
				const uint8_t *inputRowData = firstInputRowData + row * inputPitch;
				uint8_t *outputRowData = firstOutputRowData + row * outputPitch;

				// It should be possible to speed this up, if needed.  The input is guaranteed to be well-aligned and not mutable within this loop.
				for (uint32_t col = 0; col < numCols; col++)
				{
					const size_t inputOffset = firstInputCol + col;
					if (inputRowData[inputOffset / 8] & (1 << (inputOffset & 0x7)))
						outputRowData[firstOutputCol + col] = color;
				}
			}
		}
		break;
	default:
		PL_NotYetImplemented();
	}
}

void DrawString(const PLPasStr &str)
{
	PortabilityLayer::QDManager *qdManager = PortabilityLayer::QDManager::GetInstance();

	PortabilityLayer::QDPort *port = qdManager->GetPort();

	PortabilityLayer::QDState *qdState = qdManager->GetState();

	PortabilityLayer::FontManager *fontManager = PortabilityLayer::FontManager::GetInstance();

	const int textSize = qdState->m_textSize;
	const int textFace = qdState->m_textFace;
	const int fontID = qdState->m_fontID;

	int variationFlags = 0;
	if (textFace & bold)
		variationFlags |= PortabilityLayer::FontFamilyFlag_Bold;

	const PortabilityLayer::FontFamily *fontFamily = nullptr;

	switch (fontID)
	{
	case applFont:
		fontFamily = fontManager->GetApplicationFont(textSize, variationFlags);
		break;
	case systemFont:
		fontFamily = fontManager->GetSystemFont(textSize, variationFlags);
		break;
	default:
		PL_NotYetImplemented();
		return;
	}

	const int realVariation = fontFamily->GetVariationForFlags(variationFlags);
	PortabilityLayer::HostFont *font = fontFamily->GetFontForVariation(realVariation);

	if (!font)
		return;

	PortabilityLayer::RenderedFont *rfont = fontManager->GetRenderedFont(font, textSize, fontFamily->GetHacksForVariation(realVariation));

	Point penPos = qdState->m_penPos;
	const size_t len = str.Length();
	const uint8_t *chars = str.UChars();

	PixMap *pixMap = *port->GetPixMap();

	const Rect rect = pixMap->m_rect;

	if (!rect.IsValid())
		return;	// ???

	for (size_t i = 0; i < len; i++)
		DrawGlyph(qdState, pixMap, rect, penPos, rfont, chars[i]);
}

void PaintRectWithPCR(const Rect &rect, PaintColorResolution pcr)
{
	if (!rect.IsValid())
		return;

	PortabilityLayer::QDPort *qdPort = PortabilityLayer::QDManager::GetInstance()->GetPort();

	GpPixelFormat_t pixelFormat = qdPort->GetPixelFormat();

	Rect constrainedRect = rect;

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
	case GpPixelFormats::k8BitStandard:
		{
			uint8_t color = 0;
			switch (pcr)
			{
			case PaintColorResolution_Fore:
				color = qdState->ResolveForeColor8(nullptr, 0);
				break;
			case PaintColorResolution_Back:
				color = qdState->ResolveBackColor8(nullptr, 0);
				break;
			default:
				assert(false);
				break;
			}

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

void PaintRect(const Rect *rect)
{
	PaintRectWithPCR(*rect, PaintColorResolution_Fore);
}


void PaintOval(const Rect *rect)
{
	PL_NotYetImplemented_TODO("Ovals");
}

void PaintRgn(RgnHandle region)
{
	PL_NotYetImplemented_TODO("Polys");
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
	PL_NotYetImplemented_TODO("Rects");
}

void FrameOval(const Rect *rect)
{
	PL_NotYetImplemented_TODO("Ovals");
}

void FrameRoundRect(const Rect *rect, int w, int h)
{
	PL_NotYetImplemented_TODO("Ovals");
}

void PenInvertMode(bool invertMode)
{
	PortabilityLayer::QDState *qdState = PortabilityLayer::QDManager::GetInstance()->GetState();
	qdState->m_penInvert = invertMode;
}

void PenMask(bool maskMode)
{
	PortabilityLayer::QDState *qdState = PortabilityLayer::QDManager::GetInstance()->GetState();
	qdState->m_penMask = maskMode;
}

void PenPat(const Pattern *pattern)
{
	PL_NotYetImplemented_TODO("Polys");
}

void PenSize(int w, int h)
{
	PL_NotYetImplemented();
}

void PenNormal()
{
	PortabilityLayer::QDState *qdState = PortabilityLayer::QDManager::GetInstance()->GetState();
	qdState->m_penInvert = false;
	qdState->m_penMask = false;
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
	rect->left += x;
	rect->right -= x;
	rect->top += y;
	rect->bottom -= y;
}

void Line(int x, int y)
{
	PortabilityLayer::QDManager *qdManager = PortabilityLayer::QDManager::GetInstance();
	PortabilityLayer::QDState *qdState = qdManager->GetState();

	const PortabilityLayer::Vec2i oldPos = PortabilityLayer::Vec2i(qdState->m_penPos.h, qdState->m_penPos.v);

	qdState->m_penPos.h += x;
	qdState->m_penPos.v += y;

	const PortabilityLayer::Vec2i newPos = PortabilityLayer::Vec2i(qdState->m_penPos.h, qdState->m_penPos.v);

	PlotLine(qdState, qdManager->GetPort(), oldPos, newPos);

}

Pattern *GetQDGlobalsGray(Pattern *pattern)
{
	uint8_t *patternBytes = *pattern;
	for (int i = 0; i < 8; i += 2)
	{
		patternBytes[i] = 0xaa;
		patternBytes[i + 1] = 0x55;
	}

	return pattern;
}

Pattern *GetQDGlobalsBlack(Pattern *pattern)
{
	uint8_t *patternBytes = *pattern;
	for (int i = 0; i < 8; i++)
		patternBytes[i] = 255;

	return pattern;
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

static void CopyBitsComplete(const BitMap *srcBitmap, const BitMap *maskBitmap, BitMap *destBitmap, const Rect *srcRectBase, const Rect *maskRectBase, const Rect *destRectBase, RgnHandle maskRegion)
{
	assert(srcBitmap->m_pixelFormat == destBitmap->m_pixelFormat);

	const Rect &srcBounds = srcBitmap->m_rect;
	const Rect &destBounds = destBitmap->m_rect;
	const GpPixelFormat_t pixelFormat = srcBitmap->m_pixelFormat;
	const size_t srcPitch = srcBitmap->m_pitch;
	const size_t destPitch = destBitmap->m_pitch;
	const size_t maskPitch = (maskBitmap != nullptr) ? maskBitmap->m_pitch : 0;

	if (srcRectBase->right - srcRectBase->left != destRectBase->right - destRectBase->left ||
		srcRectBase->bottom - srcRectBase->top != destRectBase->bottom - destRectBase->top)
	{
		PL_NotYetImplemented_TODO("ScaledBlit");
		return;
	}

	if (maskBitmap)
	{
		assert(maskRectBase);
		assert(maskRectBase->right - maskRectBase->left == srcRectBase->right - srcRectBase->left);
		assert(maskBitmap->m_pixelFormat == GpPixelFormats::kBW1 || maskBitmap->m_pixelFormat == GpPixelFormats::k8BitStandard);
	}

	assert((maskBitmap == nullptr) == (maskRectBase == nullptr));

	Rect srcRect;
	Rect destRect;
	Rect maskRect;

	{
		const Rect constrainedSrcRect = srcRectBase->Intersect(srcBounds);
		const Rect constrainedDestRect = destRectBase->Intersect(destBounds);

		const int32_t leftNudge = std::max(constrainedSrcRect.left - srcRectBase->left, constrainedDestRect.left - destRectBase->left);
		const int32_t topNudge = std::max(constrainedSrcRect.top - srcRectBase->top, constrainedDestRect.top - destRectBase->top);
		const int32_t bottomNudge = std::min(constrainedSrcRect.bottom - srcRectBase->bottom, constrainedDestRect.bottom - destRectBase->bottom);
		const int32_t rightNudge = std::min(constrainedSrcRect.right - srcRectBase->right, constrainedDestRect.right - destRectBase->right);

		const int32_t srcLeft = srcRectBase->left + leftNudge;
		const int32_t srcRight = srcRectBase->right + rightNudge;
		const int32_t srcTop = srcRectBase->top + topNudge;
		const int32_t srcBottom = srcRectBase->bottom + bottomNudge;

		maskRect = Rect::Create(0, 0, 0, 0);
		if (maskRectBase)
		{
			maskRect.left = maskRectBase->left + leftNudge;
			maskRect.right = maskRectBase->right + rightNudge;
			maskRect.top = maskRectBase->top + topNudge;
			maskRect.bottom = maskRectBase->bottom + bottomNudge;
		}

		if (srcTop >= srcBottom)
			return;

		if (srcLeft >= srcRight)
			return;

		srcRect.left = srcLeft;
		srcRect.right = srcRight;
		srcRect.top = srcTop;
		srcRect.bottom = srcBottom;

		destRect.left = destRectBase->left + leftNudge;
		destRect.right = destRectBase->right + rightNudge;
		destRect.top = destRectBase->top + topNudge;
		destRect.bottom = destRectBase->bottom + bottomNudge;

		if (maskRectBase)
		{
			maskRect.left = maskRectBase->left + leftNudge;
			maskRect.right = maskRectBase->right + rightNudge;
			maskRect.top = maskRectBase->top + topNudge;
			maskRect.bottom = maskRectBase->bottom + bottomNudge;
		}
	}

	assert(srcRect.top >= srcBounds.top);
	assert(srcRect.bottom <= srcBounds.bottom);
	assert(srcRect.left >= srcBounds.left);
	assert(srcRect.right <= srcBounds.right);

	assert(destRect.top >= destBounds.top);
	assert(destRect.bottom <= destBounds.bottom);
	assert(destRect.left >= destBounds.left);
	assert(destRect.right <= destBounds.right);

	const Region *mask = nullptr;

	if (maskRegion)
		mask = *maskRegion;

	const Rect constrainedDestRect = mask ? destRect.Intersect(mask->rect) : destRect;
	if (!constrainedDestRect.IsValid())
		return;

	Rect constrainedSrcRect = srcRect;
	constrainedSrcRect.left += constrainedDestRect.left - destRect.left;
	constrainedSrcRect.right += constrainedDestRect.right - destRect.right;
	constrainedSrcRect.top += constrainedDestRect.top - destRect.top;
	constrainedSrcRect.bottom += constrainedDestRect.bottom - destRect.bottom;

	Rect constrainedMaskRect = maskRect;
	if (maskRectBase != nullptr)
	{
		constrainedMaskRect.left += constrainedDestRect.left - destRect.left;
		constrainedMaskRect.right += constrainedDestRect.right - destRect.right;
		constrainedMaskRect.top += constrainedDestRect.top - destRect.top;
		constrainedMaskRect.bottom += constrainedDestRect.bottom - destRect.bottom;
	}

	const size_t srcFirstCol = constrainedSrcRect.left - srcBitmap->m_rect.left;
	const size_t srcFirstRow = constrainedSrcRect.top - srcBitmap->m_rect.top;

	const size_t destFirstCol = constrainedDestRect.left - destBitmap->m_rect.left;
	const size_t destFirstRow = constrainedDestRect.top - destBitmap->m_rect.top;

	const size_t maskFirstCol = maskBitmap ? constrainedMaskRect.left - maskBitmap->m_rect.left : 0;
	const size_t maskFirstRow = maskBitmap ? constrainedMaskRect.top - maskBitmap->m_rect.top : 0;

	if (mask && mask->size != sizeof(Region))
	{
		PL_NotYetImplemented();
	}
	else
	{
		size_t pixelSizeBytes = 0;

		switch (pixelFormat)
		{
		case GpPixelFormats::k8BitCustom:
		case GpPixelFormats::k8BitStandard:
			pixelSizeBytes = 1;
			break;
		case GpPixelFormats::kRGB555:
			pixelSizeBytes = 2;
			break;
		case GpPixelFormats::kRGB24:
			pixelSizeBytes = 3;
			break;
		case GpPixelFormats::kRGB32:
			pixelSizeBytes = 4;
			break;
		};

		const uint8_t *srcBytes = static_cast<const uint8_t*>(srcBitmap->m_data);
		uint8_t *destBytes = static_cast<uint8_t*>(destBitmap->m_data);
		const uint8_t *maskBytes = maskBitmap ? static_cast<uint8_t*>(maskBitmap->m_data) : nullptr;

		const size_t firstSrcByte = srcFirstRow * srcPitch + srcFirstCol * pixelSizeBytes;
		const size_t firstDestByte = destFirstRow * destPitch + destFirstCol * pixelSizeBytes;
		const size_t firstMaskRowByte = maskBitmap ? maskFirstRow * maskPitch : 0;

		const size_t numCopiedRows = srcRect.bottom - srcRect.top;
		const size_t numCopiedCols = srcRect.right - srcRect.left;
		const size_t numCopiedBytesPerScanline = numCopiedCols * pixelSizeBytes;

		if (maskBitmap)
		{
			for (size_t i = 0; i < numCopiedRows; i++)
			{
				uint8_t *destRow = destBytes + firstDestByte + i * destPitch;
				const uint8_t *srcRow = srcBytes + firstSrcByte + i * srcPitch;
				const uint8_t *rowMaskBytes = maskBytes + firstMaskRowByte + i * maskPitch;

				size_t span = 0;

				for (size_t col = 0; col < numCopiedCols; col++)
				{
					const size_t maskBitOffset = maskFirstCol + col;
					//const bool maskBit = ((maskBytes[maskBitOffset / 8] & (0x80 >> (maskBitOffset & 7))) != 0);
					const bool maskBit = (rowMaskBytes[maskBitOffset] != 0);
					if (maskBit)
						span += pixelSizeBytes;
					else
					{
						if (span != 0)
							memcpy(destRow + col * pixelSizeBytes - span, srcRow + col * pixelSizeBytes - span, span);

						span = 0;
					}
				}

				if (span != 0)
					memcpy(destRow + numCopiedCols * pixelSizeBytes - span, srcRow + numCopiedCols * pixelSizeBytes - span, span);
			}
		}
		else
		{
			for (size_t i = 0; i < numCopiedRows; i++)
				memcpy(destBytes + firstDestByte + i * destPitch, srcBytes + firstSrcByte + i * srcPitch, numCopiedBytesPerScanline);
		}
	}
}

void CopyBits(const BitMap *srcBitmap, BitMap *destBitmap, const Rect *srcRectBase, const Rect *destRectBase, CopyBitsMode copyMode, RgnHandle maskRegion)
{
	const BitMap *maskBitmap = nullptr;
	const Rect *maskRect = nullptr;
	if (copyMode == transparent && srcBitmap->m_pixelFormat == GpPixelFormats::k8BitStandard)
	{
		maskBitmap = srcBitmap;
		maskRect = srcRectBase;
	}

	CopyBitsComplete(srcBitmap, maskBitmap, destBitmap, srcRectBase, maskRect, destRectBase, maskRegion);
}

void CopyMask(const BitMap *srcBitmap, const BitMap *maskBitmap, BitMap *destBitmap, const Rect *srcRectBase, const Rect *maskRectBase, const Rect *destRectBase)
{
	CopyBitsComplete(srcBitmap, maskBitmap, destBitmap, srcRectBase, maskRectBase, destRectBase, nullptr);
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
	PL_NotYetImplemented_TODO("Polys");
}

void DisposeRgn(RgnHandle rgn)
{
	DisposeHandle(reinterpret_cast<Handle>(rgn));
}

void OpenRgn()
{
	PL_NotYetImplemented_TODO("Polys");
}

void CloseRgn(RgnHandle rgn)
{
	PL_NotYetImplemented_TODO("Polys");
}

Boolean PtInRgn(Point point, RgnHandle rgn)
{
	PL_NotYetImplemented();
	return false;
}

void GetClip(RgnHandle rgn)
{
	PL_NotYetImplemented_TODO("Polys");
}

void SetClip(RgnHandle rgn)
{
	PL_NotYetImplemented_TODO("Polys");
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
	*outIntersection = rectA->Intersect(*rectB);

	return outIntersection->IsValid() ? PL_TRUE : PL_FALSE;
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

void BitMap::Init(const Rect &rect, GpPixelFormat_t pixelFormat, size_t pitch, void *dataPtr)
{
	m_rect = rect;
	m_pixelFormat = pixelFormat;
	m_pitch = pitch;
	m_data = dataPtr;
}

#include "stb_image_write.h"

void DebugPixMap(PixMap **pixMapH, const char *outName)
{
	PixMap *pixMap = *pixMapH;
	char outPath[1024];
	strcpy_s(outPath, outName);
	strcat_s(outPath, ".png");

	stbi_write_png(outPath, pixMap->m_rect.right - pixMap->m_rect.left, pixMap->m_rect.bottom - pixMap->m_rect.top, 1, pixMap->m_data, pixMap->m_pitch);
}
