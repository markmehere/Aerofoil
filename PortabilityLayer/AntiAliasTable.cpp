#include "AntiAliasTable.h"
#include "RGBAColor.h"

// TODO: This is not gamma correct... do we care?
namespace PortabilityLayer
{
	void AntiAliasTable::GenerateForPalette(const RGBAColor &baseColorRef, const RGBAColor *colors, size_t numColors)
	{
		const RGBAColor baseColor = baseColorRef;

		if (numColors > 256)
			numColors = 256;

		unsigned int baseCh[3] = { baseColor.r, baseColor.g, baseColor.b };

		for (size_t i = 0; i < numColors; i++)
		{
			const RGBAColor existingColor = colors[i];

			unsigned int existingCh[3] = { existingColor.r, existingColor.g, existingColor.b };

			// 0 alpha is always the same color
			m_aaTranslate[i][0] = static_cast<uint8_t>(i);

			for (unsigned int b = 1; b < 16; b++)
			{
				unsigned int newCh[3];

				for (unsigned int ch = 0; ch < 3; ch++)
					newCh[ch] = (15 - b) * existingCh[ch] + b * baseCh[ch];

				uint32_t bestError = 0xffffffffU;
				size_t bestColor = 0;
				for (size_t cmp = 0; cmp < numColors; cmp++)
				{
					int16_t existingChScaled[3] = { colors[cmp].r * 15, colors[cmp].g * 15, colors[cmp].b * 15 };

					uint32_t error = 0;
					for (unsigned int ch = 0; ch < 3; ch++)
					{
						int16_t delta = static_cast<int16_t>(newCh[ch]) - existingChScaled[ch];
						error += static_cast<uint32_t>(delta * delta);
					}

					if (error < bestError)
					{
						bestError = error;
						bestColor = cmp;
					}
				}

				m_aaTranslate[i][b] = static_cast<uint8_t>(bestColor);
			}
		}
	}
}
