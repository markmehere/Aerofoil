#pragma once

#include "RenderedFont.h"
#include "GpRenderedFontMetrics.h"

namespace PortabilityLayer
{
	class CompositeRenderedFont : public RenderedFont
	{
	public:
		CompositeRenderedFont(RenderedFont *rfont, RenderedFont *fallbackFont);

		bool GetGlyph(unsigned int character, const GpRenderedGlyphMetrics *&outMetricsPtr, const void *&outData) const override;
		const GpRenderedFontMetrics &GetMetrics() const override;
		size_t MeasureString(const uint8_t *chars, size_t len) const override;
		bool IsAntiAliased() const override;

		void Destroy() override;

		size_t MeasureCharStr(const char *str, size_t len) const;
		size_t MeasurePStr(const PLPasStr &pstr) const;

	private:
		GpRenderedFontMetrics m_metrics;
		RenderedFont *m_font;
		RenderedFont *m_fallbackFont;
	};
}
