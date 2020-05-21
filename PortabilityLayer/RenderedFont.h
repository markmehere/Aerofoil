#pragma once

#include <stdint.h>

class PLPasStr;

namespace PortabilityLayer
{
	struct RenderedFontMetrics;

	struct RenderedGlyphMetrics;

	class RenderedFont
	{
	public:
		virtual bool GetGlyph(unsigned int character, const RenderedGlyphMetrics *&outMetricsPtr, const void *&outData) const = 0;
		virtual const RenderedFontMetrics &GetMetrics() const = 0;
		virtual size_t MeasureString(const uint8_t *chars, size_t len) const = 0;
		virtual bool IsAntiAliased() const = 0;

		virtual void Destroy() = 0;

		size_t MeasureCharStr(const char *str, size_t len) const;
		size_t MeasurePStr(const PLPasStr &pstr) const;
	};
}

#include "PLPasStr.h"

inline size_t PortabilityLayer::RenderedFont::MeasurePStr(const PLPasStr &pstr) const
{
	return this->MeasureString(pstr.UChars(), pstr.Length());
}

inline size_t PortabilityLayer::RenderedFont::MeasureCharStr(const char *str, size_t len) const
{
	return this->MeasureString(reinterpret_cast<const uint8_t*>(str), len);
}
