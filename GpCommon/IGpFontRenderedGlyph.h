#pragma once

struct GpRenderedGlyphMetrics;

struct IGpFontRenderedGlyph
{
	virtual const GpRenderedGlyphMetrics &GetMetrics() const = 0;
	virtual const void *GetData() const = 0;
	virtual void Destroy() = 0;
};
