#include "ResolveCachingColor.h"

#include "PLCore.h"
#include "QDStandardPalette.h"

namespace PortabilityLayer
{
	ResolveCachingColor::ResolveCachingColor()
		: m_isResolved16(false)
		, m_isResolved8(false)
		, m_resolved16(0)
		, m_resolved8(0)
		, m_rgbaColor(RGBAColor::Create(0, 0, 0, 255))
	{
	}

	ResolveCachingColor::ResolveCachingColor(const RGBAColor &color)
		: m_isResolved16(false)
		, m_isResolved8(false)
		, m_resolved16(0)
		, m_resolved8(0)
		, m_rgbaColor(color)
	{
	}

	ResolveCachingColor::ResolveCachingColor(const ResolveCachingColor &color)
		: m_isResolved16(color.m_isResolved16)
		, m_isResolved8(color.m_isResolved8)
		, m_resolved16(color.m_resolved16)
		, m_resolved8(color.m_resolved8)
		, m_rgbaColor(color.m_rgbaColor)
	{
	}

	uint8_t ResolveCachingColor::Resolve8(const RGBAColor *palette, unsigned int numColors)
	{
		if (m_isResolved8)
			return m_resolved8;

		if (palette)
		{
			PL_NotYetImplemented();
			return 0;
		}
		else
		{
			const uint8_t resolvedColor = StandardPalette::GetInstance()->MapColorLUT(m_rgbaColor);

			m_isResolved8 = true;
			m_resolved8 = resolvedColor;

			return resolvedColor;
		}
	}

	ResolveCachingColor &ResolveCachingColor::operator=(const ResolveCachingColor &other)
	{
		m_isResolved16 = other.m_isResolved16;
		m_isResolved8 = other.m_isResolved8;
		m_resolved16 = other.m_resolved16;
		m_resolved8 = other.m_resolved8;
		m_rgbaColor = other.m_rgbaColor;

		return *this;
	}

	ResolveCachingColor ResolveCachingColor::FromStandardColor(uint8_t standardColor)
	{
		ResolveCachingColor result(StandardPalette::GetInstance()->GetColors()[standardColor]);
		result.m_isResolved8 = true;
		result.m_resolved8 = standardColor;

		return result;
	}
}
