#pragma once

#include "RGBAColor.h"

struct PixMap;

namespace PortabilityLayer
{
	struct SimpleGraphic
	{
		unsigned int m_width;
		unsigned int m_height;

		const RGBAColor *m_pixelData;
		const uint8_t *m_standardPaletteData;

		void DrawToPixMap(PixMap **pixMap, int16_t x, int16_t y);
		void DrawToPixMapWithMask(PixMap **pixMap, const uint8_t *maskData, int16_t x, int16_t y);

	protected:
		SimpleGraphic(unsigned int width, unsigned int height, const RGBAColor *pixelData, uint8_t *standardPaletteData);
		SimpleGraphic(unsigned int width, unsigned int height, const uint8_t *pixelData, RGBAColor *standardPaletteData);
	};

	template<unsigned int TWidth, unsigned int THeight>
	struct SimpleGraphicInstanceRGBA final : public SimpleGraphic
	{
		uint8_t m_standardPaletteDataInstance[TWidth * THeight];

		explicit SimpleGraphicInstanceRGBA(const RGBAColor *data);
	};

	template<unsigned int TWidth, unsigned int THeight>
	struct SimpleGraphicInstanceStandardPalette final : public SimpleGraphic
	{
		RGBAColor m_pixelDataInstance[TWidth * THeight];

		explicit SimpleGraphicInstanceStandardPalette(const uint8_t *data);
	};
}


namespace PortabilityLayer
{
	template<unsigned int TWidth, unsigned int THeight>
	inline SimpleGraphicInstanceRGBA<TWidth, THeight>::SimpleGraphicInstanceRGBA(const RGBAColor *data)
		: SimpleGraphic(TWidth, THeight, data, m_standardPaletteDataInstance)
	{
	}

	template<unsigned int TWidth, unsigned int THeight>
	inline SimpleGraphicInstanceStandardPalette<TWidth, THeight>::SimpleGraphicInstanceStandardPalette(const uint8_t *data)
		: SimpleGraphic(TWidth, THeight, data, m_pixelDataInstance)
	{
	}
}
