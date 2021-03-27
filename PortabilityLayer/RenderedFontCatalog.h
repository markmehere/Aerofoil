#pragma once

#include "PLBigEndian.h"

namespace PortabilityLayer
{
	struct RenderedFontCatalogHeader
	{
		static const int kVersion = 1;

		BEUInt32_t m_version;
		BEUInt32_t m_pathsOffset;
		BEUInt16_t m_numPaths;
		BEUInt16_t m_numRFonts;
	};

	struct RenderedFontCatalogPathEntry
	{
		BEUInt16_t m_pathOffset;
		BEUInt16_t m_pathSize;
	};

	struct RenderedFontCatalogRFontEntry
	{
		uint8_t m_pathIndex;
		uint8_t m_hacks;
		uint8_t m_fontSize;
		uint8_t m_isAA;
	};
}
