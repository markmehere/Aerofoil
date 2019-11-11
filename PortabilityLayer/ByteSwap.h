#pragma once
#ifndef __PL_BYTESWAP_H__
#define __PL_BYTESWAP_H__

#include <stdint.h>

namespace PortabilityLayer
{
	namespace ByteSwap
	{
		void BigInt16(int16_t &v);
		void BigInt32(int32_t &v);
		void BigUInt16(uint16_t &v);
		void BigUInt32(uint32_t &v);
	}
}

#endif
