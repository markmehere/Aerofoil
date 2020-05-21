#include "QDState.h"

#include "PLQDraw.h"
#include "QDStandardPalette.h"

namespace PortabilityLayer
{
	QDState::QDState()
		: m_fontFamily(nullptr)
		, m_fontSize(12)
		, m_fontVariationFlags(0)
		, m_clipRect(Rect::Create(INT16_MIN, INT16_MIN, INT16_MAX, INT16_MAX))
	{
	}
}
