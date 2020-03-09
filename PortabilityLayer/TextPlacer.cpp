#include "TextPlacer.h"

#include "PLPasStr.h"
#include "RenderedFontMetrics.h"
#include "RenderedGlyphMetrics.h"
#include "RenderedFont.h"

#include <assert.h>

namespace PortabilityLayer
{
	TextPlacer::TextPlacer(const Vec2i &basePoint, int32_t spanWidth, PortabilityLayer::RenderedFont *rfont, const PLPasStr &str)
		: m_basePoint(basePoint)
		, m_penPos(0, 0)
		, m_paraStartPos(0, 0)
		, m_rfont(rfont)
		, m_chars(str.UChars())
		, m_length(str.Length())
		, m_currentStartChar(0)
		, m_currentSpanLength(0)
		, m_committedLength(0)
		, m_emitOffset(0)
		, m_haveCommitted(false)
		, m_maxSpanWidth(spanWidth)
	{
	}

	TextPlacer::~TextPlacer()
	{
	}

	bool TextPlacer::PlaceGlyph(GlyphPlacementCharacteristics &outCharacteristics)
	{
		for (;;)
		{
			if (!m_haveCommitted)
			{
				if (m_currentStartChar == m_length)
					return false;

				size_t lastWhitespace = m_currentStartChar;
				bool shouldSkipSpaces = false;

				size_t committedLength = 0;

				size_t i = m_currentStartChar;

				// Find a span to print
				int32_t spanWidth = 0;
				for (;;)
				{
					if (i == m_length)
					{
						committedLength = i - m_currentStartChar;
						break;
					}

					uint8_t character = m_chars[i];

					if (character <= ' ')
					{
						committedLength = i - m_currentStartChar + 1;
						if (character == '\r')
							break;

						const PortabilityLayer::RenderedGlyphMetrics *metrics = nullptr;
						const void *glyphData = nullptr;
						if (m_rfont->GetGlyph(m_chars[i], metrics, glyphData))
							spanWidth += metrics->m_advanceX;

						i++;
					}
					else
					{
						const PortabilityLayer::RenderedGlyphMetrics *metrics = nullptr;
						const void *glyphData = nullptr;
						if (!m_rfont->GetGlyph(m_chars[i], metrics, glyphData))
						{
							i++;
							continue;
						}

						spanWidth += metrics->m_advanceX;

						if (m_maxSpanWidth >= 0 && spanWidth > m_maxSpanWidth)
						{
							if (committedLength == 0)
							{
								// Word didn't fit
								committedLength = i - m_currentStartChar;
								if (committedLength == 0)
									committedLength = 1;	// Nothing fit, consume one char
							}

							break;
						}

						i++;
					}
				}

				m_haveCommitted = true;
				m_committedLength = committedLength;
				m_emitOffset = 0;
			}

			assert(m_emitOffset < m_committedLength);

			const size_t charIndex = m_currentStartChar + m_emitOffset;
			const uint8_t character = m_chars[charIndex];

			outCharacteristics.m_glyphStartPos = m_penPos + m_basePoint;
			outCharacteristics.m_glyphEndPos = outCharacteristics.m_glyphStartPos;
			outCharacteristics.m_isParaStart = (m_emitOffset == 0);
			outCharacteristics.m_isParaEnd = (m_emitOffset == m_committedLength - 1);
			outCharacteristics.m_characterIndex = m_currentStartChar + m_emitOffset;
			outCharacteristics.m_character = character;

			const PortabilityLayer::RenderedGlyphMetrics *metrics;
			const void *data;
			if (!m_rfont->GetGlyph(character, metrics, data))
			{
				outCharacteristics.m_haveGlyph = false;
				outCharacteristics.m_glyphMetrics = nullptr;
				outCharacteristics.m_glyphData = nullptr;
			}
			else
			{
				m_penPos.m_x += metrics->m_advanceX;

				outCharacteristics.m_haveGlyph = true;
				outCharacteristics.m_glyphMetrics = metrics;
				outCharacteristics.m_glyphData = data;
				outCharacteristics.m_glyphEndPos.m_x += metrics->m_advanceX;
			}

			m_emitOffset++;

			if (m_emitOffset == m_committedLength)
			{
				m_currentStartChar += m_committedLength;
				m_haveCommitted = false;

				m_paraStartPos.m_y += m_rfont->GetMetrics().m_linegap;
				m_penPos = m_paraStartPos;
			}

			return true;
		}
	}
}
