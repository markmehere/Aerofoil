#pragma once

#include "SharedTypes.h"
#include "PascalStr.h"

namespace PortabilityLayer
{
	class IOStream;

	struct WindowDef
	{
		Rect m_initialRect;
		int16_t m_wdefResID;
		uint16_t m_visibilityStatus;
		uint16_t m_hasCloseBox;
		uint32_t m_referenceConstant;
		uint16_t m_positionSpec;
		uint8_t m_title[256];

		bool Deserialize(IOStream *stream);
	};
}
