#pragma once

#include <stdint.h>

struct BEColorTableItem;

namespace PortabilityLayer
{
	struct RGBAColor;

	class CTabReducer
	{
	public:
		static uint8_t DecodeClutItemChannel(const uint8_t *color16);
		static RGBAColor DecodeClutItem(const BEColorTableItem &clutItem);
	};
}
