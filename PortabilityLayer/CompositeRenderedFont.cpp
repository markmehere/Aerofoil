#include "CompositeRenderedFont.h"
#include "GpRenderedFontMetrics.h"
#include "GpRenderedGlyphMetrics.h"

#include <assert.h>

PortabilityLayer::CompositeRenderedFont::CompositeRenderedFont(RenderedFont *rfont, RenderedFont *fallbackFont)
	: m_font(rfont)
	, m_fallbackFont(fallbackFont)
	, m_metrics(rfont->GetMetrics())
{
	assert(rfont->IsAntiAliased() == fallbackFont->IsAntiAliased());

	const GpRenderedFontMetrics fallbackMetrics = fallbackFont->GetMetrics();

	if (fallbackMetrics.m_ascent > m_metrics.m_ascent)
		m_metrics.m_ascent = fallbackMetrics.m_ascent;

	if (fallbackMetrics.m_descent < m_metrics.m_descent)
		m_metrics.m_descent = fallbackMetrics.m_descent;

	if (fallbackMetrics.m_linegap > m_metrics.m_linegap)
		m_metrics.m_linegap = fallbackMetrics.m_linegap;
}

bool PortabilityLayer::CompositeRenderedFont::GetGlyph(unsigned int character, const GpRenderedGlyphMetrics *&outMetricsPtr, const void *&outData) const
{
	if (m_font->GetGlyph(character, outMetricsPtr, outData))
		return true;

	return m_fallbackFont->GetGlyph(character, outMetricsPtr, outData);
}

const GpRenderedFontMetrics &PortabilityLayer::CompositeRenderedFont::GetMetrics() const
{
	return m_metrics;
}

size_t PortabilityLayer::CompositeRenderedFont::MeasureString(const uint8_t *chars, size_t len) const
{
	int32_t measure = 0;
	for (size_t i = 0; i < len; i++)
	{
		const uint8_t character = chars[i];
		const GpRenderedGlyphMetrics *metrics = nullptr;
		const void *data = nullptr;
		if (m_font->GetGlyph(chars[i], metrics, data))
			measure += metrics->m_advanceX;
		else if (m_fallbackFont->GetGlyph(chars[i], metrics, data))
			measure += metrics->m_advanceX;
	}

	return static_cast<size_t>(measure);
}

bool PortabilityLayer::CompositeRenderedFont::IsAntiAliased() const
{
	return m_font->IsAntiAliased();
}

void PortabilityLayer::CompositeRenderedFont::Destroy()
{
	assert(false);
}
