#pragma once

#include "RGBAColor.h"

namespace PortabilityLayer
{
	class ResolveCachingColor
	{
	public:
		ResolveCachingColor();
		ResolveCachingColor(const RGBAColor &color);
		ResolveCachingColor(const ResolveCachingColor &color);

		uint8_t Resolve8(const RGBAColor *palette, unsigned int numColors);

		ResolveCachingColor &operator=(const ResolveCachingColor &other);

		static ResolveCachingColor FromStandardColor(uint8_t standardColor);

		const RGBAColor &GetRGBAColor() const;

	private:
		RGBAColor m_rgbaColor;

		uint16_t m_resolved16;
		uint8_t m_resolved8;

		bool m_isResolved16;
		bool m_isResolved8;
	};
}

inline const PortabilityLayer::RGBAColor &PortabilityLayer::ResolveCachingColor::GetRGBAColor() const
{
	return m_rgbaColor;
}
