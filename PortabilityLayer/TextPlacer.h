#pragma once

#include "Vec2i.h"

class PLPasStr;
struct GpRenderedGlyphMetrics;

namespace PortabilityLayer
{
	class RenderedFont;

	struct GlyphPlacementCharacteristics
	{
		bool m_haveGlyph;
		bool m_isParaStart;							// Character is the first character in the paragraph
		bool m_isParaEnd;							// Character is the last character in the paragraph
		const GpRenderedGlyphMetrics *m_glyphMetrics;	// Glyph metrics
		const void *m_glyphData;					// Glyph data
		Vec2i m_glyphStartPos;						// Glyph start position
		Vec2i m_glyphEndPos;						// Glyph end position
		unsigned int m_character;					// Character code
		size_t m_characterIndex;					// Index in the input string
	};

	class TextPlacer
	{
	public:
		TextPlacer(const Vec2i &basePoint, int32_t spanWidth, PortabilityLayer::RenderedFont *rfont, const PLPasStr &str);
		~TextPlacer();

		bool PlaceGlyph(GlyphPlacementCharacteristics &outCharacteristics);

	private:
		Vec2i m_basePoint;
		Vec2i m_penPos;
		Vec2i m_paraStartPos;
		PortabilityLayer::RenderedFont *m_rfont;
		const uint8_t *m_chars;
		size_t m_length;

		size_t m_currentStartChar;
		size_t m_currentSpanLength;
		size_t m_committedLength;
		size_t m_emitOffset;
		bool m_haveCommitted;

		int32_t m_maxSpanWidth;
	};
}
