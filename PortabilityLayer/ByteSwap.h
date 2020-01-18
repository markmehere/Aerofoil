#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	namespace ByteSwap
	{
		void BigInt16(int16_t &v);
		void BigInt32(int32_t &v);
		void BigInt64(int64_t &v);
		void BigUInt16(uint16_t &v);
		void BigUInt32(uint32_t &v);
		void BigUInt64(uint64_t &v);

		void LittleInt16(int16_t &v);
		void LittleInt32(int32_t &v);
		void LittleInt64(int64_t &v);
		void LittleUInt16(uint16_t &v);
		void LittleUInt32(uint32_t &v);
		void LittleUInt64(uint64_t &v);
	}
}
