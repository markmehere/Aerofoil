#include "PLCTabReducer.h"
#include "RGBAColor.h"

#include "SharedTypes.h"

namespace PortabilityLayer
{
	uint8_t CTabReducer::DecodeClutItemChannel(const uint8_t *color16)
	{
		const int colorHigh = color16[0];
		const int colorLow = color16[1];

		const int lowDelta = colorLow - colorHigh;
		if (lowDelta < -128)
			return static_cast<uint8_t>(colorHigh - 1);
		else if (lowDelta > 128)
			return static_cast<uint8_t>(colorHigh + 1);
		return static_cast<uint8_t>(colorHigh);
	}

	RGBAColor CTabReducer::DecodeClutItem(const BEColorTableItem &clutItem)
	{
		const uint8_t r = DecodeClutItemChannel(clutItem.m_red);
		const uint8_t g = DecodeClutItemChannel(clutItem.m_green);
		const uint8_t b = DecodeClutItemChannel(clutItem.m_blue);
		return RGBAColor::Create(r, g, b, 255);
	}
}
