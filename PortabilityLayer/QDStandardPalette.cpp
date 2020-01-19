#include "QDStandardPalette.h"

#include <string.h>

// Standard palette mapping is 6x6x6, with the last element omitted for an RGB table
// which ranges from 0..214
// 10 elements of red scale from 215..224
// 10 elements of green scale from 225..234
// 10 elements of blue scale from 235..244
// 10 elements of gray scale from 245..254
// black at 255

namespace
{
	static void DivMod15By5RoundDown(unsigned int value, unsigned int &valueDiv3Out, unsigned int &valueMod3Out)
	{
		const unsigned int valueDiv3 = (value * 11) >> 5;
		const unsigned int valueMod3 = value - valueDiv3 * 3;
		valueDiv3Out = valueDiv3;
		valueMod3Out = valueMod3;
	}
}

namespace PortabilityLayer
{
	StandardPalette::StandardPalette()
	{
		for (unsigned int rs = 0; rs < 6; rs++)
		{
			for (unsigned int gs = 0; gs < 6; gs++)
			{
				for (unsigned int bs = 0; bs < 6; bs++)
				{
					RGBAColor &entry = m_colors[215 - bs - gs * 6 - rs * 36];
					entry.r = rs * 51;
					entry.g = gs * 51;
					entry.b = bs * 51;
					entry.a = 255;
				}
			}
		}

		for (unsigned int scale = 0; scale < 5; scale++)
		{
			for (unsigned int tickUp = 0; tickUp < 2; tickUp++)
			{
				const unsigned int scaleValue = (scale * 3 + tickUp + 1) * 17;
				const unsigned int scaleIndex = scale * 2 + tickUp;

				RGBAColor &rScaleEntry = m_colors[224 - scaleIndex];
				RGBAColor &gScaleEntry = m_colors[234 - scaleIndex];
				RGBAColor &bScaleEntry = m_colors[244 - scaleIndex];
				RGBAColor &grayScaleEntry = m_colors[254 - scaleIndex];

				rScaleEntry.r = scaleValue;
				rScaleEntry.g = 0;
				rScaleEntry.b = 0;
				rScaleEntry.a = 255;

				gScaleEntry.r = 0;
				gScaleEntry.g = scaleValue;
				gScaleEntry.b = 0;
				gScaleEntry.a = 255;

				bScaleEntry.r = 0;
				bScaleEntry.g = 0;
				bScaleEntry.b = scaleValue;
				bScaleEntry.a = 255;

				grayScaleEntry.r = scaleValue;
				grayScaleEntry.g = scaleValue;
				grayScaleEntry.b = scaleValue;
				grayScaleEntry.a = 255;
			}
		}

		RGBAColor &blackEntry = m_colors[255];
		blackEntry.r = blackEntry.g = blackEntry.b = 0;
		blackEntry.a = 255;

		for (unsigned int rs = 0; rs < 16; rs++)
			for (unsigned int gs = 0; gs < 16; gs++)
				for (unsigned int bs = 0; bs < 16; bs++)
					m_lut[rs + (gs << 4) + (bs << 8)] = MapColorAnalyticTruncated(rs, gs, bs);

#if 0
		for (unsigned int i = 0; i < 256; i++)
		{
			unsigned int shortChannels[3] =
			{
				m_colors[i].r / 17,
				m_colors[i].g / 17,
				m_colors[i].b / 17
			};

			for (unsigned int b = 0; b < 16; b++)
			{
				unsigned int whiteScale[3];
				unsigned int blackScale[3];

				for (unsigned int ch = 0; ch < 3; ch++)
				{
					unsigned int scaledBackground = (15 - b) * shortChannels[ch];
					unsigned int scaledWhiteForeground = 15 * b;

					blackScale[ch] = scaledBackground / 15;
					whiteScale[ch] = (scaledBackground + scaledWhiteForeground) / 15;
				}

				m_blackAATable.m_aaTranslate[i][b] = MapColorAnalyticTruncated(blackScale[0], blackScale[1], blackScale[2]);
				m_whiteAATable.m_aaTranslate[i][b] = MapColorAnalyticTruncated(whiteScale[0], whiteScale[1], whiteScale[2]);
			}
		}
#else
		m_blackAATable.GenerateForPalette(RGBAColor::Create(0, 0, 0, 255), m_colors, 256);
		m_whiteAATable.GenerateForPalette(RGBAColor::Create(255, 255, 255, 255), m_colors, 256);
#endif
	}

	const RGBAColor *StandardPalette::GetColors() const
	{
		return m_colors;
	}

	uint8_t StandardPalette::MapColorAnalyticTruncated(unsigned int r, unsigned int g, unsigned int b)
	{
		if (g <= 1 && b <= 1)
		{
			if (r <= 1)
			{
				// Special case low gray scale
				return 255 - b;
			}

			// Red scale
			unsigned int scale6Step;
			unsigned int scale6StepRemainder;
			DivMod15By5RoundDown(r, scale6Step, scale6StepRemainder);

			if (scale6StepRemainder == 0)
			{
				if (scale6Step == 0)
					return 255;
				else
					return 215 - scale6Step * 36;
			}
			else
				return 225 - scale6Step * 2 - scale6StepRemainder;
		}

		if (r <= 1 && b <= 1)
		{
			// Green scale
			unsigned int scale6Step;
			unsigned int scale6StepRemainder;
			DivMod15By5RoundDown(g, scale6Step, scale6StepRemainder);

			if (scale6StepRemainder == 0)
			{
				if (scale6Step == 0)
					return 255;
				else
					return 215 - scale6Step * 6;
			}
			else
				return 235 - scale6Step * 2 - scale6StepRemainder;
		}

		if (r <= 1 && g <= 1)
		{
			// Blue scale
			unsigned int scale6Step;
			unsigned int scale6StepRemainder;
			DivMod15By5RoundDown(b, scale6Step, scale6StepRemainder);

			if (scale6StepRemainder == 0)
			{
				if (scale6Step == 0)
					return 255;
				else
					return 215 - scale6Step;
			}
			else
				return 245 - scale6Step * 2 - scale6StepRemainder;
		}

		const unsigned int grayscaleTimes3 = r + g + b;
		const int rGrayDelta = static_cast<int>(r * 3) - static_cast<int>(grayscaleTimes3);
		const int gGrayDelta = static_cast<int>(g * 3) - static_cast<int>(grayscaleTimes3);
		const int bGrayDelta = static_cast<int>(b * 3) - static_cast<int>(grayscaleTimes3);

		if (rGrayDelta >= -3 && rGrayDelta <= 3 && gGrayDelta >= -3 && gGrayDelta <= 3 && bGrayDelta >= -3 && bGrayDelta <= 3)
		{
			// Divide down to 0..15 range
			const unsigned int grayscaleValue = (grayscaleTimes3 * 21 + 36) >> 6;

			// Divide down again to 0..5 range, rounding down
			unsigned int grayscale6Step;
			unsigned int grayscale6StepRemainder;
			DivMod15By5RoundDown(grayscaleValue, grayscale6Step, grayscale6StepRemainder);

			if (grayscale6StepRemainder == 0)
			{
				if (grayscale6Step == 0)
					return 255;
				else if (grayscale6Step == 5)
					return 0;
				else
					return 215 - 43 * grayscale6Step;
			}
			else
				return 255 - grayscale6Step * 2 - grayscale6StepRemainder;
		}

		// Round to 0..5 range
		const unsigned int rSmallNearest6 = (r * 11 + 11) >> 5;
		const unsigned int gSmallNearest6 = (g * 11 + 11) >> 5;
		const unsigned int bSmallNearest6 = (b * 11 + 11) >> 5;

		if ((rSmallNearest6 | gSmallNearest6 | bSmallNearest6) == 0)
			return 255;
		else
			return 215 - bSmallNearest6 - gSmallNearest6 * 6 - rSmallNearest6 * 36;
	}

	uint8_t StandardPalette::MapColorAnalytic(uint8_t r, uint8_t g, uint8_t b)
	{
		// Round to 0..15 range
		const unsigned int rSmall = (r * 241 + 2048) >> 12;
		const unsigned int gSmall = (g * 241 + 2048) >> 12;
		const unsigned int bSmall = (b * 241 + 2048) >> 12;

		return MapColorAnalyticTruncated(rSmall, gSmall, bSmall);
	}

	uint8_t StandardPalette::MapColorAnalytic(const RGBAColor &color)
	{
		// Round to 0..15 range
		const unsigned int rSmall = (color.r * 241 + 2048) >> 12;
		const unsigned int gSmall = (color.g * 241 + 2048) >> 12;
		const unsigned int bSmall = (color.b * 241 + 2048) >> 12;

		return MapColorAnalyticTruncated(rSmall, gSmall, bSmall);
	}

	uint8_t StandardPalette::MapColorLUT(uint8_t r, uint8_t g, uint8_t b) const
	{
		// Round to 0..15 range
		const unsigned int rSmall = (r * 241 + 2048) >> 12;
		const unsigned int gSmall = (g * 241 + 2048) >> 12;
		const unsigned int bSmall = (b * 241 + 2048) >> 12;

		return m_lut[rSmall + (gSmall << 4) + (bSmall << 8)];
	}

	uint8_t StandardPalette::MapColorLUT(const RGBAColor &color) const
	{
		return MapColorLUT(color.r, color.g, color.b);
	}

	const StandardPalette *StandardPalette::GetInstance()
	{
		return &ms_instance;
	}

	const AntiAliasTable &StandardPalette::GetWhiteAATable() const
	{
		return m_whiteAATable;
	}

	const AntiAliasTable &StandardPalette::GetBlackAATable() const
	{
		return m_blackAATable;
	}

	StandardPalette StandardPalette::ms_instance;
}
