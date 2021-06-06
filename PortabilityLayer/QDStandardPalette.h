#pragma once

#include "AntiAliasTable.h"
#include "RGBAColor.h"

struct IGpMutex;

namespace PortabilityLayer
{
	struct AntiAliasTable;

	class StandardPalette
	{
	public:
		static const unsigned int kSize = 256;

		StandardPalette();

		const RGBAColor *GetColors() const;
		static uint8_t MapColorAnalyticTruncated(unsigned int r, unsigned int g, unsigned int b);
		static uint8_t MapColorAnalytic(uint8_t r, uint8_t g, uint8_t b);
		static uint8_t MapColorAnalytic(const RGBAColor &color);
		uint8_t MapColorLUT(uint8_t r, uint8_t g, uint8_t b) const;
		uint8_t MapColorLUT(const RGBAColor &color) const;

		const AntiAliasTable &GetCachedPaletteAATable(const RGBAColor &color, IGpMutex *mutex = nullptr);
		const AntiAliasTable &GetCachedToneAATable(uint8_t tone, IGpMutex *mutex = nullptr);

		static StandardPalette *GetInstance();

	private:
		static StandardPalette ms_instance;

		struct CachedPaletteTableEntry
		{
			uint8_t m_rgb[3];
			AntiAliasTable m_aaTable;
		};

		struct CachedToneTableEntry
		{
			uint8_t m_tone;
			AntiAliasTable m_aaTable;
		};

		static const size_t kMaxCachedPaletteTables = 16;
		static const size_t kMaxCachedToneTables = 16;

		CachedPaletteTableEntry m_cachedPaletteTables[kMaxCachedPaletteTables];
		size_t m_numCachedPaletteTables;

		CachedToneTableEntry m_cachedToneTables[kMaxCachedToneTables];
		size_t m_numCachedToneTables;

		RGBAColor m_colors[kSize];

		uint8_t m_lut[16 * 16 * 16];
	};

	class Icon4BitPalette
	{
	public:
		static const unsigned int kSize = 16;

		Icon4BitPalette();

		static Icon4BitPalette *GetInstance();
		const RGBAColor *GetColors() const;

	private:
		static Icon4BitPalette ms_instance;
		RGBAColor m_colors[kSize];
	};
}
