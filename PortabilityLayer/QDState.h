#pragma once

#include "RGBAColor.h"
#include "SharedTypes.h"

namespace PortabilityLayer
{
	class FontFamily;

	struct QDState
	{
		QDState();

		FontFamily *m_fontFamily;
		int m_fontVariationFlags;
		int m_fontSize;
		Rect m_clipRect;
	};
}
