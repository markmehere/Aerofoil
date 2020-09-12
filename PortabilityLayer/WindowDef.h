#pragma once

#include "SharedTypes.h"
#include "PascalStr.h"

class PLPasStr;
class GpIOStream;

namespace PortabilityLayer
{
	namespace WindowStyleFlags
	{
		enum WindowStyleFlag
		{
			kTitleBar	= 1,
			kBorderless = 2,
			kResizable	= 4,
			kMiniBar	= 8,
			kAlert		= 16,
			kCloseBox	= 32,
		};
	}

	struct WindowDef
	{
		Rect m_initialRect;
		uint16_t m_styleFlags;
		uint16_t m_visibilityStatus;
		uint32_t m_referenceConstant;
		uint16_t m_positionSpec;
		uint8_t m_title[256];

		bool Deserialize(GpIOStream *stream);

		static WindowDef Create(const Rect &initialRect, uint16_t styleFlags, bool isVisible, uint32_t refConstant, uint16_t positionSpec, const PLPasStr &title);
	};
}
