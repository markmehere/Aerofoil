#pragma once

#include "DataTypes.h"

namespace PortabilityLayer
{
	namespace BytePack
	{
		void BigUInt64(uint8_t *bytes, uint64_t value);
		void BigUInt32(uint8_t *bytes, uint32_t value);
		void BigUInt16(uint8_t *bytes, uint16_t value);

		void LittleUInt64(uint8_t *bytes, uint64_t value);
		void LittleUInt32(uint8_t *bytes, uint32_t value);
		void LittleUInt16(uint8_t *bytes, uint16_t value);
	}
}

namespace PortabilityLayer
{
	namespace BytePack
	{
		inline void BigUInt64(uint8_t *bytes, uint64_t value)
		{
			bytes[0] = static_cast<uint8_t>((value >> 56) & 0xff);
			bytes[1] = static_cast<uint8_t>((value >> 48) & 0xff);
			bytes[2] = static_cast<uint8_t>((value >> 40) & 0xff);
			bytes[3] = static_cast<uint8_t>((value >> 32) & 0xff);
			bytes[4] = static_cast<uint8_t>((value >> 24) & 0xff);
			bytes[5] = static_cast<uint8_t>((value >> 16) & 0xff);
			bytes[6] = static_cast<uint8_t>((value >> 8) & 0xff);
			bytes[7] = static_cast<uint8_t>(value & 0xff);
		}

		inline void BigUInt32(uint8_t *bytes, uint32_t value)
		{
			bytes[0] = static_cast<uint8_t>((value >> 24) & 0xff);
			bytes[1] = static_cast<uint8_t>((value >> 16) & 0xff);
			bytes[2] = static_cast<uint8_t>((value >> 8) & 0xff);
			bytes[3] = static_cast<uint8_t>(value & 0xff);
		}

		inline void BigUInt16(uint8_t *bytes, uint16_t value)
		{
			bytes[0] = static_cast<uint8_t>((value >> 8) & 0xff);
			bytes[1] = static_cast<uint8_t>(value & 0xff);
		}

		inline void BigInt64(uint8_t *bytes, int64_t value)
		{
			bytes[0] = static_cast<uint8_t>((value >> 56) & 0xff);
			bytes[1] = static_cast<uint8_t>((value >> 48) & 0xff);
			bytes[2] = static_cast<uint8_t>((value >> 40) & 0xff);
			bytes[3] = static_cast<uint8_t>((value >> 32) & 0xff);
			bytes[4] = static_cast<uint8_t>((value >> 24) & 0xff);
			bytes[5] = static_cast<uint8_t>((value >> 16) & 0xff);
			bytes[6] = static_cast<uint8_t>((value >> 8) & 0xff);
			bytes[7] = static_cast<uint8_t>(value & 0xff);
		}

		inline void BigInt32(uint8_t *bytes, uint32_t value)
		{
			bytes[0] = static_cast<uint8_t>((value >> 24) & 0xff);
			bytes[1] = static_cast<uint8_t>((value >> 16) & 0xff);
			bytes[2] = static_cast<uint8_t>((value >> 8) & 0xff);
			bytes[3] = static_cast<uint8_t>(value & 0xff);
		}

		inline void BigInt16(uint8_t *bytes, uint16_t value)
		{
			bytes[0] = static_cast<uint8_t>((value >> 8) & 0xff);
			bytes[1] = static_cast<uint8_t>(value & 0xff);
		}

		inline void LittleUInt64(uint8_t *bytes, uint64_t value)
		{
			bytes[0] = static_cast<uint8_t>(value & 0xff);
			bytes[1] = static_cast<uint8_t>((value >> 8) & 0xff);
			bytes[2] = static_cast<uint8_t>((value >> 16) & 0xff);
			bytes[3] = static_cast<uint8_t>((value >> 24) & 0xff);
			bytes[4] = static_cast<uint8_t>((value >> 32) & 0xff);
			bytes[5] = static_cast<uint8_t>((value >> 40) & 0xff);
			bytes[6] = static_cast<uint8_t>((value >> 48) & 0xff);
			bytes[7] = static_cast<uint8_t>((value >> 56) & 0xff);
		}

		inline void LittleUInt32(uint8_t *bytes, uint32_t value)
		{
			bytes[0] = static_cast<uint8_t>(value & 0xff);
			bytes[1] = static_cast<uint8_t>((value >> 8) & 0xff);
			bytes[2] = static_cast<uint8_t>((value >> 16) & 0xff);
			bytes[3] = static_cast<uint8_t>((value >> 24) & 0xff);
		}

		inline void LittleUInt16(uint8_t *bytes, uint16_t value)
		{
			bytes[0] = static_cast<uint8_t>(value & 0xff);
			bytes[1] = static_cast<uint8_t>((value >> 8) & 0xff);
		}

		inline void LittleInt64(uint8_t *bytes, int64_t value)
		{
			bytes[0] = static_cast<uint8_t>(value & 0xff);
			bytes[1] = static_cast<uint8_t>((value >> 8) & 0xff);
			bytes[2] = static_cast<uint8_t>((value >> 16) & 0xff);
			bytes[3] = static_cast<uint8_t>((value >> 24) & 0xff);
			bytes[4] = static_cast<uint8_t>((value >> 32) & 0xff);
			bytes[5] = static_cast<uint8_t>((value >> 40) & 0xff);
			bytes[6] = static_cast<uint8_t>((value >> 48) & 0xff);
			bytes[7] = static_cast<uint8_t>((value >> 56) & 0xff);
		}

		inline void LittleInt32(uint8_t *bytes, uint32_t value)
		{
			bytes[0] = static_cast<uint8_t>(value & 0xff);
			bytes[1] = static_cast<uint8_t>((value >> 8) & 0xff);
			bytes[2] = static_cast<uint8_t>((value >> 16) & 0xff);
			bytes[3] = static_cast<uint8_t>((value >> 24) & 0xff);
		}

		inline void LittleInt16(uint8_t *bytes, uint16_t value)
		{
			bytes[0] = static_cast<uint8_t>(value & 0xff);
			bytes[1] = static_cast<uint8_t>((value >> 8) & 0xff);
		}
	}
}
