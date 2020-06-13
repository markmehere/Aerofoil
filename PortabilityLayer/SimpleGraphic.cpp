#include "SimpleGraphic.h"
#include "QDStandardPalette.h"
#include "QDPixMap.h"
#include "SharedTypes.h"

namespace PortabilityLayer
{
	SimpleGraphic::SimpleGraphic(unsigned int width, unsigned int height, const RGBAColor *pixelData, uint8_t *standardPaletteData)
		: m_width(width)
		, m_height(height)
		, m_pixelData(pixelData)
		, m_standardPaletteData(standardPaletteData)
	{
		const unsigned int numPixels = width * height;

		for (unsigned int i = 0; i < numPixels; i++)
			standardPaletteData[i] = PortabilityLayer::StandardPalette::MapColorAnalytic(pixelData[i]);
	}

	SimpleGraphic::SimpleGraphic(unsigned int width, unsigned int height, const uint8_t *standardPaletteData, RGBAColor *pixelData)
		: m_width(width)
		, m_height(height)
		, m_pixelData(pixelData)
		, m_standardPaletteData(standardPaletteData)
	{
		const unsigned int numPixels = width * height;

		for (unsigned int i = 0; i < numPixels; i++)
			pixelData[i] = PortabilityLayer::StandardPalette::GetInstance()->GetColors()[standardPaletteData[i]];
	}

	void SimpleGraphic::DrawToPixMap(PixMap **pixMapH, int16_t x, int16_t y) const
	{
		DrawToPixMapWithMask(pixMapH, nullptr, x, y);
	}

	void SimpleGraphic::DrawToPixMapWithMask(PixMap **pixMapH, const uint8_t *maskData, int16_t x, int16_t y) const
	{
		if (!pixMapH)
			return;

		PixMap *pixMap = *pixMapH;

		void *pixMapData = pixMap->m_data;
		const size_t destPitch = pixMap->m_pitch;

		const Rect pixMapRect = pixMap->m_rect;
		const int32_t right = x + static_cast<int32_t>(m_width);
		const int32_t bottom = y + static_cast<int32_t>(m_height);

		// Simple graphics must be entirely in bounds
		if (x < 0 || y < 0 || right > pixMapRect.right || bottom > pixMapRect.bottom)
			return;

		const size_t destXOffset = (x - pixMapRect.left);
		const size_t destYOffset = (y - pixMapRect.top);

		const size_t srcHeight = m_height;
		const size_t srcWidth = m_width;

		size_t maskOffset = 0;

		switch (pixMap->m_pixelFormat)
		{
		case GpPixelFormats::k8BitStandard:
			{
				uint8_t *destFirstPixel = static_cast<uint8_t*>(pixMapData) + destXOffset + destYOffset * destPitch;
				const uint8_t *srcPixel = m_standardPaletteData;

				for (size_t row = 0; row < srcHeight; row++)
				{
					uint8_t *destRowFirstPixel = destFirstPixel + row * destPitch;

					if (maskData)
					{
						for (size_t col = 0; col < srcWidth; col++)
						{
							if (maskData[maskOffset / 8] & (0x80 >> (maskOffset & 7)))
								destRowFirstPixel[col] = *srcPixel;
							srcPixel++;
							maskOffset++;
						}
					}
					else
					{
						for (size_t col = 0; col < srcWidth; col++)
						{
							destRowFirstPixel[col] = *srcPixel;
							srcPixel++;
						}
					}
				}
			}
			break;
		case GpPixelFormats::kRGB32:
			{
				uint8_t *destFirstPixel = static_cast<uint8_t*>(pixMapData) + destXOffset * 4 + destYOffset * destPitch;
				const PortabilityLayer::RGBAColor *srcPixel = m_pixelData;

				for (size_t row = 0; row < srcHeight; row++)
				{
					uint8_t *destRowFirstPixel = destFirstPixel + row * destPitch;

					if (maskData)
					{
						for (size_t col = 0; col < srcWidth; col++)
						{
							if (maskData[maskOffset / 8] & (0x80 >> (maskOffset & 7)))
							{
								destRowFirstPixel[col * 4 + 0] = srcPixel->r;
								destRowFirstPixel[col * 4 + 1] = srcPixel->g;
								destRowFirstPixel[col * 4 + 2] = srcPixel->b;
							}
							srcPixel++;
							maskOffset++;
						}
					}
					else
					{
						for (size_t col = 0; col < srcWidth; col++)
						{
							destRowFirstPixel[col * 4 + 0] = srcPixel->r;
							destRowFirstPixel[col * 4 + 1] = srcPixel->g;
							destRowFirstPixel[col * 4 + 2] = srcPixel->b;
							srcPixel++;
						}
					}
				}
			}
			break;
		default:
			PL_NotYetImplemented();
			break;
		}
	}
}
