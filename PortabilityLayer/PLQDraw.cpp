#include "PLQDraw.h"
#include "QDManager.h"
#include "QDState.h"
#include "BitmapImage.h"
#include "DisplayDeviceManager.h"
#include "FontFamily.h"
#include "FontManager.h"
#include "LinePlotter.h"
#include "MMHandleBlock.h"
#include "MemoryManager.h"
#include "MemReaderStream.h"
#include "HostFontHandler.h"
#include "PLPasStr.h"
#include "RenderedFont.h"
#include "RenderedFontMetrics.h"
#include "RenderedGlyphMetrics.h"
#include "Rect2i.h"
#include "ResourceManager.h"
#include "ResTypeID.h"
#include "RGBAColor.h"
#include "ScanlineMask.h"
#include "ScanlineMaskConverter.h"
#include "ScanlineMaskIterator.h"
#include "QDGraf.h"
#include "QDStandardPalette.h"
#include "WindowManager.h"
#include "QDGraf.h"
#include "QDPixMap.h"
#include "Vec2i.h"

#include <algorithm>
#include <assert.h>


void GetPort(GrafPtr *graf)
{
	PL_NotYetImplemented();
}

void SetPort(GrafPtr graf)
{
	PortabilityLayer::QDManager::GetInstance()->SetPort(graf);
}

void EndUpdate(WindowPtr graf)
{
	graf->m_surface.m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
}

PLError_t GetIconSuite(Handle *suite, short resID, IconSuiteFlags flags)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
}

PLError_t PlotIconSuite(Rect *rect, Handle iconSuite)
{
	PL_NotYetImplemented();
	return PLErrors::kNone;
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
	PortabilityLayer::QDManager::GetInstance()->SetPort(&window->m_surface.m_port);
}

void SetPortDialogPort(Dialog *dialog)
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
	Point &penPos = PortabilityLayer::QDManager::GetInstance()->GetState()->m_penPos;

	penPos.h = x;
	penPos.v = y;
}

static void PlotLine(PortabilityLayer::QDState *qdState, DrawSurface *surface, const PortabilityLayer::Vec2i &pointA, const PortabilityLayer::Vec2i &pointB)
{
	const Rect lineRect = Rect::Create(
		std::min(pointA.m_y, pointB.m_y),
		std::min(pointA.m_x, pointB.m_x),
		std::max(pointA.m_y, pointB.m_y) + 1,
		std::max(pointA.m_x, pointB.m_x) + 1);

	// If the points are a straight line, paint as a rect
	if (pointA.m_y == pointB.m_y || pointA.m_x == pointB.m_x)
	{
		surface->FillRect(lineRect);
		return;
	}

	PortabilityLayer::QDPort *port = &surface->m_port;

	GpPixelFormat_t pixelFormat = surface->m_port.GetPixelFormat();

	Rect constrainedRect = port->GetRect();

	constrainedRect = constrainedRect.Intersect(qdState->m_clipRect);
	constrainedRect = constrainedRect.Intersect(lineRect);

	if (!constrainedRect.IsValid())
		return;

	PortabilityLayer::Vec2i upperPoint = pointA;
	PortabilityLayer::Vec2i lowerPoint = pointB;

	if (pointA.m_y > pointB.m_y)
		std::swap(upperPoint, lowerPoint);

	PortabilityLayer::PixMapImpl *pixMap = static_cast<PortabilityLayer::PixMapImpl*>(*port->GetPixMap());
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

	surface->m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
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
	*color = RGBColor(foreColor.r, foreColor.g, foreColor.b);
}

static void DrawGlyph(PortabilityLayer::QDState *qdState, PixMap *pixMap, const Rect &rect, Point &penPos, const PortabilityLayer::RenderedFont *rfont, unsigned int character,
	PortabilityLayer::AntiAliasTable *&cachedAATable, PortabilityLayer::RGBAColor &cachedAATableColor)
{
	assert(rect.IsValid());

	const PortabilityLayer::RenderedGlyphMetrics *metrics;
	const void *data;
	if (!rfont->GetGlyph(character, metrics, data))
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

	const bool isAA = rfont->IsAntiAliased();

	switch (pixMap->m_pixelFormat)
	{
	case GpPixelFormats::k8BitStandard:
		{
			uint8_t *firstOutputRowData = static_cast<uint8_t*>(pixMap->m_data) + firstOutputRow * outputPitch;

			const PortabilityLayer::AntiAliasTable *aaTable = nullptr;

			if (isAA)
			{
				const PortabilityLayer::RGBAColor foreColor = qdState->GetForeColor();
				if (foreColor == PortabilityLayer::RGBAColor::Create(0, 0, 0, 255))
					aaTable = &PortabilityLayer::StandardPalette::GetInstance()->GetBlackAATable();
				else if (foreColor == PortabilityLayer::RGBAColor::Create(255, 255, 255, 255))
					aaTable = &PortabilityLayer::StandardPalette::GetInstance()->GetWhiteAATable();
				else if (cachedAATable != nullptr && foreColor == cachedAATableColor)
					aaTable = cachedAATable;
				else
				{
					if (!cachedAATable)
					{
						cachedAATable = static_cast<PortabilityLayer::AntiAliasTable*>(PortabilityLayer::MemoryManager::GetInstance()->Alloc(sizeof(PortabilityLayer::AntiAliasTable)));
						if (!cachedAATable)
							return;
					}

					cachedAATableColor = foreColor;
					cachedAATable->GenerateForPalette(foreColor, PortabilityLayer::StandardPalette::GetInstance()->GetColors(), 256);

					aaTable = cachedAATable;
				}
			}

			const uint8_t color = qdState->ResolveForeColor8(nullptr, 0);
			for (uint32_t row = 0; row < numRows; row++)
			{
				const uint8_t *inputRowData = firstInputRowData + row * inputPitch;
				uint8_t *outputRowData = firstOutputRowData + row * outputPitch;

				// It should be possible to speed this up, if needed.  The input is guaranteed to be well-aligned and not mutable within this loop.
				if (isAA)
				{
					for (uint32_t col = 0; col < numCols; col++)
					{
						const size_t inputOffset = firstInputCol + col;

						const unsigned int grayLevel = (inputRowData[inputOffset / 2] >> ((inputOffset & 1) * 4)) & 0xf;
						uint8_t &targetPixel = outputRowData[firstOutputCol + col];

						targetPixel = aaTable->m_aaTranslate[targetPixel][grayLevel];
					}
				}
				else
				{
					for (uint32_t col = 0; col < numCols; col++)
					{
						const size_t inputOffset = firstInputCol + col;
						if (inputRowData[inputOffset / 8] & (1 << (inputOffset & 0x7)))
							outputRowData[firstOutputCol + col] = color;
					}
				}
			}
		}
		break;
	default:
		PL_NotYetImplemented();
	}
}

void DrawSurface::DrawString(const Point &point, const PLPasStr &str, bool aa)
{
	PortabilityLayer::QDPort *port = &m_port;

	PortabilityLayer::QDState *qdState = m_port.GetState();

	PortabilityLayer::FontManager *fontManager = PortabilityLayer::FontManager::GetInstance();

	const int fontSize = qdState->m_fontSize;
	const int fontVariationFlags = qdState->m_fontVariationFlags;
	PortabilityLayer::FontFamily *fontFamily = qdState->m_fontFamily;

	PortabilityLayer::RenderedFont *rfont = fontManager->GetRenderedFontFromFamily(fontFamily, fontSize, aa, fontVariationFlags);

	Point penPos = point;
	const size_t len = str.Length();
	const uint8_t *chars = str.UChars();

	PixMap *pixMap = *port->GetPixMap();

	const Rect rect = pixMap->m_rect;

	if (!rect.IsValid())
		return;	// ???

	Point paraStartPos = penPos;

	for (size_t i = 0; i < len; i++)
	{
		if (chars[i] == static_cast<uint8_t>('\r'))
		{
			paraStartPos.v += rfont->GetMetrics().m_linegap;
			penPos = paraStartPos;
		}
		else
			DrawGlyph(qdState, pixMap, rect, penPos, rfont, chars[i], m_cachedAATable, m_cachedAAColor);
	}

	m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
}

void DrawSurface::DrawStringWrap(const Point &point, const Rect &constrainRect, const PLPasStr &str, bool aa)
{
	PortabilityLayer::QDPort *port = &m_port;

	PortabilityLayer::QDState *qdState = m_port.GetState();

	PortabilityLayer::FontManager *fontManager = PortabilityLayer::FontManager::GetInstance();

	const int fontSize = qdState->m_fontSize;
	const int fontVariationFlags = qdState->m_fontVariationFlags;
	PortabilityLayer::FontFamily *fontFamily = qdState->m_fontFamily;

	PortabilityLayer::RenderedFont *rfont = fontManager->GetRenderedFontFromFamily(fontFamily, fontSize, aa, fontVariationFlags);

	Point penPos = point;
	const size_t len = str.Length();
	const uint8_t *chars = str.UChars();

	PixMap *pixMap = *port->GetPixMap();

	const Rect limitRect = pixMap->m_rect.Intersect(constrainRect);;
	const Rect areaRect = constrainRect;

	if (!limitRect.IsValid() || !areaRect.IsValid())
		return;	// ???

	Point paraStartPos = penPos;

	size_t currentStartChar = 0;
	size_t currentSpanLength = 0;

	for (;;)
	{
		if (currentStartChar == len)
			break;

		size_t lastWhitespace = currentStartChar;
		bool shouldSkipSpaces = false;

		size_t committedLength = 0;

		size_t i = currentStartChar;

		// Find a span to print
		int32_t spanWidth = 0;
		for (;;)
		{
			if (i == len)
			{
				committedLength = i - currentStartChar;
				break;
			}

			uint8_t character = chars[i];

			if (character <= ' ')
			{
				committedLength = i - currentStartChar + 1;
				if (character == '\r')
					break;

				i++;
			}
			else
			{
				const PortabilityLayer::RenderedGlyphMetrics *metrics = nullptr;
				const void *glyphData = nullptr;
				if (!rfont->GetGlyph(chars[i], metrics, glyphData))
				{
					i++;
					continue;
				}

				spanWidth += metrics->m_advanceX;

				int32_t glyphEnd = penPos.h + spanWidth;

				if (glyphEnd >= constrainRect.right)
				{
					if (committedLength == 0)
					{
						// Word didn't fit
						committedLength = i - currentStartChar;
						if (committedLength == 0)
							committedLength = 1;	// Nothing fit, consume one char
					}

					break;
				}

				i++;
			}
		}

		for (size_t ci = 0; ci < committedLength; ci++)
		{
			const uint8_t character = chars[currentStartChar + ci];

			DrawGlyph(qdState, pixMap, pixMap->m_rect, penPos, rfont, character, m_cachedAATable, m_cachedAAColor);
		}

		currentStartChar += committedLength;

		paraStartPos.v += rfont->GetMetrics().m_linegap;
		penPos = paraStartPos;
	}

	m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
}

size_t DrawSurface::MeasureString(const PLPasStr &str)
{
	const PortabilityLayer::QDState *qdState = m_port.GetState();
	PortabilityLayer::FontManager *fontManager = PortabilityLayer::FontManager::GetInstance();

	PortabilityLayer::FontFamily *fontFamily = qdState->m_fontFamily;

	if (!fontFamily)
		return 0;

	const int variationFlags = qdState->m_fontVariationFlags;
	const int fontSize = qdState->m_fontSize;

	PortabilityLayer::RenderedFont *rfont = fontManager->GetRenderedFontFromFamily(fontFamily, fontSize, false, variationFlags);
	if (!rfont)
		return 0;

	return rfont->MeasureString(str.UChars(), str.Length());
}

int32_t DrawSurface::MeasureFontAscender()
{
	const PortabilityLayer::QDState *qdState = m_port.GetState();
	PortabilityLayer::FontManager *fontManager = PortabilityLayer::FontManager::GetInstance();

	PortabilityLayer::FontFamily *fontFamily = qdState->m_fontFamily;

	if (!fontFamily)
		return 0;

	const int variationFlags = qdState->m_fontVariationFlags;
	const int fontSize = qdState->m_fontSize;

	PortabilityLayer::RenderedFont *rfont = fontManager->GetRenderedFontFromFamily(fontFamily, fontSize, false, variationFlags);
	if (!rfont)
		return 0;

	return rfont->GetMetrics().m_ascent;
}

int32_t DrawSurface::MeasureFontLineGap()
{
	const PortabilityLayer::QDState *qdState = m_port.GetState();
	PortabilityLayer::FontManager *fontManager = PortabilityLayer::FontManager::GetInstance();

	PortabilityLayer::FontFamily *fontFamily = qdState->m_fontFamily;

	if (!fontFamily)
		return 0;

	const int variationFlags = qdState->m_fontVariationFlags;
	const int fontSize = qdState->m_fontSize;

	PortabilityLayer::RenderedFont *rfont = fontManager->GetRenderedFontFromFamily(fontFamily, fontSize, false, variationFlags);
	if (!rfont)
		return 0;

	return rfont->GetMetrics().m_linegap;
}

void DrawSurface::DrawPicture(THandle<BitmapImage> pictHdl, const Rect &bounds)
{
	if (!pictHdl)
		return;

	if (!bounds.IsValid() || bounds.Width() == 0 || bounds.Height() == 0)
		return;

	BitmapImage *bmpPtr = *pictHdl;
	if (!bmpPtr)
		return;

	const size_t bmpSize = bmpPtr->m_fileHeader.m_fileSize;
	const Rect picRect = bmpPtr->GetRect();

	if (picRect.Width() == 0 || picRect.Height() == 0)
		return;

	if (bounds.right - bounds.left != picRect.right - picRect.left || bounds.bottom - bounds.top != picRect.bottom - picRect.top)
	{
		PL_NotYetImplemented_TODO("Palette");

		DrawSurface *scaleSurface = nullptr;
		if (PortabilityLayer::QDManager::GetInstance()->NewGWorld(&scaleSurface, this->m_port.GetPixelFormat(), picRect, nullptr) != PLErrors::kNone)
			return;

		scaleSurface->DrawPicture(pictHdl, picRect);

		const uint16_t newWidth = bounds.Width();
		const uint16_t newHeight = bounds.Height();

		THandle<PortabilityLayer::PixMapImpl> scaled = static_cast<PortabilityLayer::PixMapImpl*>(*scaleSurface->m_port.GetPixMap())->ScaleTo(newWidth, newHeight);
		PortabilityLayer::QDManager::GetInstance()->DisposeGWorld(scaleSurface);

		if (scaled)
			CopyBits(*scaled, *this->m_port.GetPixMap(), &(*scaled)->m_rect, &bounds, srcCopy);

		PortabilityLayer::PixMapImpl::Destroy(scaled);

		m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);

		return;
	}

	PortabilityLayer::QDPort *port = &m_port;

	if (!port)
		return;

	PortabilityLayer::PixMapImpl *pixMap = static_cast<PortabilityLayer::PixMapImpl*>(*port->GetPixMap());

	long handleSize = pictHdl.MMBlock()->m_size;
	PortabilityLayer::MemReaderStream stream(bmpPtr, handleSize);

	// Adjust draw origin
	const PortabilityLayer::Vec2i drawOrigin = PortabilityLayer::Vec2i(bounds.left, bounds.top);
	const Rect targetPixMapRect = pixMap->m_rect;

	const int32_t truncatedTop = std::max<int32_t>(0, targetPixMapRect.top - bounds.top);
	const int32_t truncatedBottom = std::max<int32_t>(0, bounds.bottom - targetPixMapRect.bottom);
	const int32_t truncatedLeft = std::max<int32_t>(0, targetPixMapRect.left - bounds.left);
	const int32_t truncatedRight = std::max<int32_t>(0, bounds.right - targetPixMapRect.right);

	uint8_t paletteMapping[256];
	for (int i = 0; i < 256; i++)
		paletteMapping[i] = 0;

	// Parse bitmap header
	const uint8_t *bmpBytes = reinterpret_cast<const uint8_t*>(bmpPtr);

	PortabilityLayer::BitmapFileHeader fileHeader;
	PortabilityLayer::BitmapInfoHeader infoHeader;

	memcpy(&fileHeader, bmpBytes, sizeof(fileHeader));
	memcpy(&infoHeader, bmpBytes + sizeof(fileHeader), sizeof(infoHeader));

	const uint16_t bpp = infoHeader.m_bitsPerPixel;

	if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 24)
		return;

	const uint32_t numColors = infoHeader.m_numColors;
	if (numColors > 256)
		return;

	const uint8_t *ctabLoc = bmpBytes + sizeof(fileHeader) + infoHeader.m_thisStructureSize;

	const size_t ctabSize = infoHeader.m_numColors * sizeof(PortabilityLayer::BitmapColorTableEntry);
	const size_t availCTabBytes = bmpSize - sizeof(fileHeader) - infoHeader.m_thisStructureSize;

	if (ctabSize > availCTabBytes)
		return;

	if (bpp <= 8)
	{
		// Perform palette mapping
		if (pixMap->GetPixelFormat() == GpPixelFormats::kBW1)
		{
			const PortabilityLayer::BitmapColorTableEntry *ctab = reinterpret_cast<const PortabilityLayer::BitmapColorTableEntry*>(ctabLoc);
			for (size_t i = 0; i < numColors; i++)
			{
				const PortabilityLayer::BitmapColorTableEntry &ctabEntry = ctab[i];
				if (ctabEntry.m_r + ctabEntry.m_g + ctabEntry.m_b < 383)
					paletteMapping[i] = 255;
				else
					paletteMapping[i] = 0;
			}
		}
		else if (pixMap->GetPixelFormat() == GpPixelFormats::k8BitStandard)
		{
			const PortabilityLayer::BitmapColorTableEntry *ctab = reinterpret_cast<const PortabilityLayer::BitmapColorTableEntry*>(ctabLoc);
			for (size_t i = 0; i < numColors; i++)
			{
				const PortabilityLayer::BitmapColorTableEntry &ctabEntry = ctab[i];
				paletteMapping[i] = PortabilityLayer::StandardPalette::GetInstance()->MapColorLUT(PortabilityLayer::RGBAColor::Create(ctabEntry.m_r, ctabEntry.m_g, ctabEntry.m_b, 255));
				const PortabilityLayer::RGBAColor &resultColor = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[paletteMapping[i]];
			}
		}
		else
		{
			PL_NotYetImplemented();
		}
	}

	const uint32_t imageDataOffset = fileHeader.m_imageDataStart;

	if (imageDataOffset > bmpSize)
		return;

	const size_t availImageDataSize = bmpSize - imageDataOffset;

	const size_t sourcePitch = (bpp * infoHeader.m_width + 31) / 32 * 4;
	const size_t inDataSize = sourcePitch * infoHeader.m_height;

	if (inDataSize > availImageDataSize)
		return;

	const uint8_t *imageDataStart = reinterpret_cast<const uint8_t*>(bmpPtr) + imageDataOffset;
	const uint8_t *sourceFirstImageRowStart = imageDataStart + (infoHeader.m_height - 1) * sourcePitch;

	// Determine rect
	const PortabilityLayer::Rect2i sourceRect = PortabilityLayer::Rect2i(truncatedTop, truncatedLeft, static_cast<int32_t>(infoHeader.m_height) - truncatedBottom, static_cast<int32_t>(infoHeader.m_width) - truncatedRight);

	if (sourceRect.m_topLeft.m_x >= sourceRect.m_bottomRight.m_x || sourceRect.m_topLeft.m_y >= sourceRect.m_bottomRight.m_y)
		return;	// Entire rect was culled away

	const uint32_t numCopyRows = static_cast<uint32_t>(sourceRect.m_bottomRight.m_y - sourceRect.m_topLeft.m_y);
	const uint32_t numCopyCols = static_cast<uint32_t>(sourceRect.m_bottomRight.m_x - sourceRect.m_topLeft.m_x);

	const uint8_t *firstSourceRow = sourceFirstImageRowStart - static_cast<uint32_t>(sourceRect.m_topLeft.m_y) * sourcePitch;
	int32_t firstSourceCol = sourceRect.m_topLeft.m_x;

	const size_t destPitch = pixMap->GetPitch();
	uint8_t *firstDestRow = static_cast<uint8_t*>(pixMap->GetPixelData()) + destPitch * static_cast<uint32_t>(drawOrigin.m_y + truncatedTop);
	size_t firstDestCol = static_cast<uint32_t>(drawOrigin.m_x + truncatedLeft);

	const PortabilityLayer::StandardPalette *stdPalette = PortabilityLayer::StandardPalette::GetInstance();

	GpPixelFormat_t destFormat = pixMap->GetPixelFormat();
	switch (destFormat)
	{
	case GpPixelFormats::kBW1:
	case GpPixelFormats::k8BitStandard:
	{
		const uint8_t *currentSourceRow = firstSourceRow;
		uint8_t *currentDestRow = firstDestRow;
		for (uint32_t row = 0; row < numCopyRows; row++)
		{
			assert(currentSourceRow >= imageDataStart && currentSourceRow <= imageDataStart + inDataSize);

			if (bpp == 1)
			{
				for (size_t col = 0; col < numCopyCols; col++)
				{
					const size_t srcColIndex = col + firstSourceCol;
					const size_t destColIndex = col + firstDestCol;

					const unsigned int srcIndex = (currentSourceRow[srcColIndex / 8] >> (8 - ((srcColIndex & 7) + 1))) & 0x01;
					currentDestRow[destColIndex] = paletteMapping[srcIndex];
				}
			}
			else if (bpp == 4)
			{
				for (size_t col = 0; col < numCopyCols; col++)
				{
					const size_t srcColIndex = col + firstSourceCol;
					const size_t destColIndex = col + firstDestCol;

					const unsigned int srcIndex = (currentSourceRow[srcColIndex / 2] >> (8 - ((srcColIndex & 1) + 1) * 4)) & 0x0f;
					currentDestRow[destColIndex] = paletteMapping[srcIndex];
				}
			}
			else if (bpp == 8)
			{
				for (size_t col = 0; col < numCopyCols; col++)
				{
					const size_t srcColIndex = col + firstSourceCol;
					const size_t destColIndex = col + firstDestCol;

					const unsigned int srcIndex = currentSourceRow[srcColIndex];
					currentDestRow[destColIndex] = paletteMapping[srcIndex];
				}
			}
			else if (bpp == 16)
			{
				if (destFormat == GpPixelFormats::kBW1)
				{
					for (size_t col = 0; col < numCopyCols; col++)
					{
						const size_t srcColIndex = col + firstSourceCol;
						const size_t destColIndex = col + firstDestCol;

						const uint8_t srcLow = currentSourceRow[srcColIndex * 2 + 0];
						const uint8_t srcHigh = currentSourceRow[srcColIndex * 2 + 1];

						const unsigned int combinedValue = srcLow | (srcHigh << 8);
						const unsigned int b = (combinedValue & 0x1f);
						const unsigned int g = ((combinedValue >> 5) & 0x1f);
						const unsigned int r = ((combinedValue >> 10) & 0x1f);

						if (r + g + b > 46)
							currentDestRow[destColIndex] = 0;
						else
							currentDestRow[destColIndex] = 1;
					}
				}
				else
				{
					for (size_t col = 0; col < numCopyCols; col++)
					{
						const size_t srcColIndex = col + firstSourceCol;
						const size_t destColIndex = col + firstDestCol;

						const uint8_t srcLow = currentSourceRow[srcColIndex * 2 + 0];
						const uint8_t srcHigh = currentSourceRow[srcColIndex * 2 + 1];

						const unsigned int combinedValue = srcLow | (srcHigh << 8);
						const unsigned int b = (srcLow & 0x1f);
						const unsigned int g = ((srcLow >> 5) & 0x1f);
						const unsigned int r = ((srcLow >> 10) & 0x1f);

						const unsigned int xr = (r << 5) | (r >> 2);
						const unsigned int xg = (g << 5) | (g >> 2);
						const unsigned int xb = (b << 5) | (b >> 2);

						currentDestRow[destColIndex] = stdPalette->MapColorLUT(PortabilityLayer::RGBAColor::Create(xr, xg, xb, 255));
					}
				}
			}
			else if (bpp == 24)
			{
				if (destFormat == GpPixelFormats::kBW1)
				{
					for (size_t col = 0; col < numCopyCols; col++)
					{
						const size_t srcColIndex = col + firstSourceCol;
						const size_t destColIndex = col + firstDestCol;

						const uint8_t srcLow = currentSourceRow[srcColIndex * 2 + 0];
						const uint8_t srcHigh = currentSourceRow[srcColIndex * 2 + 1];

						const unsigned int combinedValue = srcLow | (srcHigh << 8);
						const unsigned int b = (combinedValue & 0x1f);
						const unsigned int g = ((combinedValue >> 5) & 0x1f);
						const unsigned int r = ((combinedValue >> 10) & 0x1f);

						if (r + g + b > 46)
							currentDestRow[destColIndex] = 0;
						else
							currentDestRow[destColIndex] = 1;
					}
				}
				else
				{
					for (size_t col = 0; col < numCopyCols; col++)
					{
						const size_t srcColIndex = col + firstSourceCol;
						const size_t destColIndex = col + firstDestCol;

						const unsigned int b = currentSourceRow[srcColIndex * 3 + 0];
						const unsigned int g = currentSourceRow[srcColIndex * 3 + 1];
						const unsigned int r = currentSourceRow[srcColIndex * 3 + 2];

						currentDestRow[destColIndex] = stdPalette->MapColorLUT(PortabilityLayer::RGBAColor::Create(r, g, b, 255));
					}
				}
			}

			currentSourceRow -= sourcePitch;
			currentDestRow += destPitch;
		}
	}
	break;
	default:
		// TODO: Implement higher-resolution pixel blitters
		assert(false);
		return;
	};

	m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
}


void DrawSurface::SetPattern8x8(const uint8_t *pattern)
{
	m_port.GetState()->SetPenPattern8x8(pattern);
}

void DrawSurface::ClearPattern()
{
	m_port.GetState()->SetPenPattern8x8(nullptr);
}

void DrawSurface::SetMaskMode(bool maskMode)
{
	m_port.GetState()->m_penMask = maskMode;
}

Rect DrawSurface::GetClipRect() const
{
	return m_port.GetState()->m_clipRect;
}

void DrawSurface::SetClipRect(const Rect &rect)
{
	m_port.GetState()->m_clipRect = rect;;
}

void DrawSurface::FillRect(const Rect &rect)
{
	if (!rect.IsValid())
		return;

	PortabilityLayer::QDPort *qdPort = &m_port;

	GpPixelFormat_t pixelFormat = qdPort->GetPixelFormat();

	Rect constrainedRect = rect;

	PortabilityLayer::QDState *qdState = qdPort->GetState();
	constrainedRect = constrainedRect.Intersect(qdState->m_clipRect);
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

	m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
}

void DrawSurface::FillRectWithPattern8x8(const Rect &rect, const uint8_t *pattern)
{
	PL_NotYetImplemented_TODO("FillWithPattern");
	m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
}

void DrawSurface::SetApplicationFont(int size, int variationFlags)
{
	PortabilityLayer::FontFamily *fontFamily = PortabilityLayer::FontManager::GetInstance()->GetApplicationFont(size, variationFlags);
	if (!fontFamily)
		return;

	PortabilityLayer::QDState *qdState = m_port.GetState();

	qdState->m_fontFamily = fontFamily;
	qdState->m_fontSize = size;
	qdState->m_fontVariationFlags = variationFlags;
}

void DrawSurface::SetSystemFont(int size, int variationFlags)
{
	PortabilityLayer::FontFamily *fontFamily = PortabilityLayer::FontManager::GetInstance()->GetSystemFont(size, variationFlags);
	if (!fontFamily)
		return;

	PortabilityLayer::QDState *qdState = m_port.GetState();

	qdState->m_fontFamily = fontFamily;
	qdState->m_fontSize = size;
	qdState->m_fontVariationFlags = variationFlags;
}

void DrawSurface::FillEllipse(const Rect &rect)
{
	if (!rect.IsValid() || rect.Width() < 1 || rect.Height() < 1)
		return;

	if (rect.Width() <= 2 || rect.Height() <= 2)
	{
		FillRect(rect);
		return;
	}

	PortabilityLayer::ScanlineMask *mask = PortabilityLayer::ScanlineMaskConverter::CompileEllipse(PortabilityLayer::Rect2i(rect.top, rect.left, rect.bottom, rect.right));
	if (mask)
	{
		FillScanlineMask(mask);
		mask->Destroy();
	}
}

void DrawSurface::FrameEllipse(const Rect &rect)
{
	PL_NotYetImplemented();
}

static void FillScanlineSpan(uint8_t *rowStart, size_t startCol, size_t endCol, uint8_t patternByte, uint8_t foreColor, uint8_t bgColor, bool mask)
{
	if (patternByte == 0xff)
	{
		for (size_t col = startCol; col < endCol; col++)
			rowStart[col] = foreColor;
	}
	else
	{
		if (mask)
		{
			for (size_t col = startCol; col < endCol; col++)
			{
				if (patternByte & (0x80 >> (col & 7)))
					rowStart[col] = foreColor;
			}
		}
		else
		{
			for (size_t col = startCol; col < endCol; col++)
			{
				if (patternByte & (0x80 >> (col & 7)))
					rowStart[col] = foreColor;
				else
					rowStart[col] = bgColor;
			}
		}
	}
}

void DrawSurface::FillScanlineMask(const PortabilityLayer::ScanlineMask *scanlineMask)
{
	if (!scanlineMask)
		return;

	PortabilityLayer::QDPort *port = &m_port;
	PortabilityLayer::QDState *qdState = port->GetState();

	PixMap *pixMap = *port->GetPixMap();
	const Rect portRect = port->GetRect();
	const Rect maskRect = scanlineMask->GetRect();

	const Rect constrainedRect = portRect.Intersect(maskRect);
	if (!constrainedRect.IsValid())
		return;

	const size_t firstMaskRow = static_cast<size_t>(constrainedRect.top - maskRect.top);
	const size_t firstMaskCol = static_cast<size_t>(constrainedRect.left - maskRect.left);
	const size_t firstPortRow = static_cast<size_t>(constrainedRect.top - portRect.top);
	const size_t firstPortCol = static_cast<size_t>(constrainedRect.left - portRect.left);
	const size_t pitch = pixMap->m_pitch;
	const size_t maskSpanWidth = scanlineMask->GetRect().right - scanlineMask->GetRect().left;

	// Skip mask rows
	PortabilityLayer::ScanlineMaskIterator iter = scanlineMask->GetIterator();
	for (size_t i = 0; i < firstMaskRow; i++)
	{
		size_t spanRemaining = maskSpanWidth;
		while (spanRemaining > 0)
			spanRemaining -= iter.Next();
	}

	uint8_t foreColor8 = 0;
	uint8_t backColor8 = 0;
	const bool isMask = qdState->m_penMask;

	const GpPixelFormat_t pixelFormat = pixMap->m_pixelFormat;

	size_t pixelSize = 0;
	switch (pixMap->m_pixelFormat)
	{
	case GpPixelFormats::k8BitStandard:
		foreColor8 = qdState->ResolveForeColor8(nullptr, 256);
		backColor8 = qdState->ResolveBackColor8(nullptr, 256);
		break;
	default:
		PL_NotYetImplemented();
		return;
	}

	uint8_t pattern8x8[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	bool havePattern = false;
	if (const uint8_t *statePattern = qdState->GetPattern8x8())
	{
		memcpy(pattern8x8, statePattern, 8);
		for (int i = 0; i < 8; i++)
		{
			if (pattern8x8[i] != 0xff)
			{
				havePattern = true;
				break;
			}
		}
	}

	const size_t constrainedRectWidth = static_cast<size_t>(constrainedRect.right - constrainedRect.left);

	uint8_t *pixMapData = static_cast<uint8_t*>(pixMap->m_data);
	uint8_t *firstRowStart = pixMapData + (constrainedRect.top * pitch);
	const size_t numRows = static_cast<size_t>(constrainedRect.bottom - constrainedRect.top);
	for (size_t row = 0; row < numRows; row++)
	{
		uint8_t *thisRowStart = firstRowStart + row * pitch;
		uint8_t thisRowPatternRow = pattern8x8[row & 7];

		bool spanState = false;

		size_t currentSpan = iter.Next();
		{
			// Skip prefix cols.  If the span ends at the first col, this must not advance the iterator to it (currentSpan should be 0 instead)
			size_t prefixColsRemaining = firstMaskCol;

			while (prefixColsRemaining > 0)
			{
				if (prefixColsRemaining <= currentSpan)
				{
					currentSpan -= prefixColsRemaining;
					break;
				}
				else
				{
					prefixColsRemaining -= currentSpan;
					currentSpan = iter.Next();
					spanState = !spanState;
				}
			}
		}

		// Paint in-bound cols.  If the span ends at the end of the mask, this must not advance the iterator beyond it (currentSpan should be 0 instead)
		size_t paintColsRemaining = constrainedRectWidth;
		size_t spanStartCol = firstPortCol;

		while (paintColsRemaining > 0)
		{
			if (paintColsRemaining <= currentSpan)
			{
				currentSpan -= paintColsRemaining;
				break;
			}
			else
			{
				const size_t spanEndCol = spanStartCol + currentSpan;
				if (spanState)
					FillScanlineSpan(thisRowStart, spanStartCol, spanEndCol, thisRowPatternRow, foreColor8, backColor8, isMask);

				spanStartCol = spanEndCol;
				paintColsRemaining -= currentSpan;
				currentSpan = iter.Next();
				spanState = !spanState;
			}
		}

		// Flush any lingering span
		if (spanState)
		{
			const size_t spanEndCol = firstPortCol + constrainedRectWidth;
			FillScanlineSpan(thisRowStart, spanStartCol, spanEndCol, thisRowPatternRow, foreColor8, backColor8, isMask);
		}

		if (row != numRows - 1)
		{
			size_t terminalColsRemaining = maskSpanWidth - constrainedRectWidth - firstMaskCol;

			assert(currentSpan <= terminalColsRemaining);

			terminalColsRemaining -= currentSpan;

			while (terminalColsRemaining > 0)
			{
				currentSpan = iter.Next();

				assert(currentSpan <= terminalColsRemaining);
				terminalColsRemaining -= currentSpan;
			}
		}
	}

	m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
}


void DrawSurface::DrawLine(const Point &a, const Point &b)
{
	PlotLine(m_port.GetState(), this, PortabilityLayer::Vec2i(a.h, a.v), PortabilityLayer::Vec2i(b.h, b.v));
}

void DrawSurface::InvertDrawLine(const Point &a, const Point &b, const uint8_t *pattern)
{
	PL_NotYetImplemented();
}

void GetClip(Rect *rect)
{
	PortabilityLayer::QDState *qdState = PortabilityLayer::QDManager::GetInstance()->GetState();
	*rect = qdState->m_clipRect;
}

void ClipRect(const Rect *rect)
{
	if (!rect->IsValid())
		return;

	PortabilityLayer::QDState *qdState = PortabilityLayer::QDManager::GetInstance()->GetState();
	qdState->m_clipRect = *rect;
}

void DrawSurface::FrameRect(const Rect &rect)
{
	if (!rect.IsValid())
		return;

	uint16_t width = rect.right - rect.left;
	uint16_t height = rect.bottom - rect.top;

	if (width <= 2 || height <= 2)
		FillRect(rect);
	else
	{
		// This is stupid, especially in the vertical case, but oh well
		Rect edgeRect;

		edgeRect = rect;
		edgeRect.right = edgeRect.left + 1;
		FillRect(edgeRect);

		edgeRect = rect;
		edgeRect.left = edgeRect.right - 1;
		FillRect(edgeRect);

		edgeRect = rect;
		edgeRect.bottom = edgeRect.top + 1;
		FillRect(edgeRect);

		edgeRect = rect;
		edgeRect.top = edgeRect.bottom - 1;
		FillRect(edgeRect);
	}

	m_port.SetDirty(PortabilityLayer::QDPortDirtyFlag_Contents);
}

void DrawSurface::FrameRoundRect(const Rect &rect, int quadrantWidth, int quadrantHeight)
{
	PL_NotYetImplemented_TODO("RoundRect");
	this->FrameRect(rect);
}

void DrawSurface::InvertFrameRect(const Rect &rect, const uint8_t *pattern)
{
	PL_NotYetImplemented();
}

void DrawSurface::InvertFillRect(const Rect &rect, const uint8_t *pattern)
{
	PL_NotYetImplemented_TODO("InvertFillRect");
}

void DrawSurface::SetForeColor(const PortabilityLayer::RGBAColor &color)
{
	m_port.GetState()->SetForeColor(color);
}

const PortabilityLayer::RGBAColor &DrawSurface::GetForeColor() const
{
	return m_port.GetState()->GetForeColor();
}

void DrawSurface::SetBackColor(const PortabilityLayer::RGBAColor &color)
{
	m_port.GetState()->SetBackColor(color);
}

const PortabilityLayer::RGBAColor &DrawSurface::GetBackColor() const
{
	return m_port.GetState()->GetBackColor();
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
	PortabilityLayer::QDState *qdState = PortabilityLayer::QDManager::GetInstance()->GetState();
	qdState->SetPenPattern8x8(*pattern);
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

	THandle<uint8_t> patternList = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('PAT#', patListID).StaticCast<uint8_t>();
	const uint8_t *patternRes = *patternList;

	int numPatterns = (patternRes[0] << 8) | patternRes[1];
	if (index > numPatterns)
		return;

	memcpy(pattern, patternRes + 2 + (index - 1) * 8, 8);
}

static void CopyBitsComplete(const BitMap *srcBitmap, const BitMap *maskBitmap, BitMap *destBitmap, const Rect *srcRectBase, const Rect *maskRectBase, const Rect *destRectBase, const Rect *maskConstraintRect)
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
		Rect constrainedDestRect = destRectBase->Intersect(destBounds);

		if (maskConstraintRect)
			constrainedDestRect = constrainedDestRect.Intersect(*maskConstraintRect);

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

	Rect constrainedSrcRect = srcRect;
	constrainedSrcRect.left += destRect.left - destRect.left;
	constrainedSrcRect.right += destRect.right - destRect.right;
	constrainedSrcRect.top += destRect.top - destRect.top;
	constrainedSrcRect.bottom += destRect.bottom - destRect.bottom;

	Rect constrainedMaskRect = maskRect;
	if (maskRectBase != nullptr)
	{
		constrainedMaskRect.left += destRect.left - destRect.left;
		constrainedMaskRect.right += destRect.right - destRect.right;
		constrainedMaskRect.top += destRect.top - destRect.top;
		constrainedMaskRect.bottom += destRect.bottom - destRect.bottom;
	}

	const size_t srcFirstCol = constrainedSrcRect.left - srcBitmap->m_rect.left;
	const size_t srcFirstRow = constrainedSrcRect.top - srcBitmap->m_rect.top;

	const size_t destFirstCol = destRect.left - destBitmap->m_rect.left;
	const size_t destFirstRow = destRect.top - destBitmap->m_rect.top;

	const size_t maskFirstCol = maskBitmap ? constrainedMaskRect.left - maskBitmap->m_rect.left : 0;
	const size_t maskFirstRow = maskBitmap ? constrainedMaskRect.top - maskBitmap->m_rect.top : 0;

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

void CopyBits(const BitMap *srcBitmap, BitMap *destBitmap, const Rect *srcRectBase, const Rect *destRectBase, CopyBitsMode copyMode)
{
	CopyBitsConstrained(srcBitmap, destBitmap, srcRectBase, destRectBase, copyMode, nullptr);
}

void CopyBitsConstrained(const BitMap *srcBitmap, BitMap *destBitmap, const Rect *srcRectBase, const Rect *destRectBase, CopyBitsMode copyMode, const Rect *constrainRect)
{
	const BitMap *maskBitmap = nullptr;
	const Rect *maskRect = nullptr;
	if (copyMode == transparent && srcBitmap->m_pixelFormat == GpPixelFormats::k8BitStandard)
	{
		maskBitmap = srcBitmap;
		maskRect = srcRectBase;
	}

	CopyBitsComplete(srcBitmap, maskBitmap, destBitmap, srcRectBase, maskRect, destRectBase, constrainRect);
}

void CopyMaskConstrained(const BitMap *srcBitmap, const BitMap *maskBitmap, BitMap *destBitmap, const Rect *srcRectBase, const Rect *maskRectBase, const Rect *destRectBase, const Rect *constrainRect)
{
	CopyBitsComplete(srcBitmap, maskBitmap, destBitmap, srcRectBase, maskRectBase, destRectBase, constrainRect);
}


bool PointInScanlineMask(Point point, PortabilityLayer::ScanlineMask *scanlineMask)
{
	PL_NotYetImplemented();
	return false;
}

void CopyMask(const BitMap *srcBitmap, const BitMap *maskBitmap, BitMap *destBitmap, const Rect *srcRectBase, const Rect *maskRectBase, const Rect *destRectBase)
{
	CopyBitsComplete(srcBitmap, maskBitmap, destBitmap, srcRectBase, maskRectBase, destRectBase, nullptr);
}

BitMap *GetPortBitMapForCopyBits(DrawSurface *grafPtr)
{
	return *grafPtr->m_port.GetPixMap();
}

DrawSurface *GetWindowPort(WindowPtr window)
{
	return &window->m_surface;
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
