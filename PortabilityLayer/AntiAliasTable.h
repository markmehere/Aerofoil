#pragma once

#include <stdint.h>
#include <stddef.h>

namespace PortabilityLayer
{
	struct RGBAColor;

	struct AntiAliasTable
	{
		// Striped 256x16 because constant background color is more likely than constant sample
		uint8_t m_aaTranslate[256][16];

#if 0
		void GenerateForPalette(const RGBAColor &baseColor, const RGBAColor *colors, size_t numColors, bool cacheable);
#endif
		void GenerateForPaletteFast(const RGBAColor &baseColor);
		void GenerateForSimpleScale(uint8_t colorChannel, bool cacheable);

	private:
		bool LoadFromCache(const char *path);
		void SaveToCache(const char *path);

		static const unsigned int kCacheVersion = 1;
	};
}
