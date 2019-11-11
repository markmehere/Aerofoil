#include "ByteSwap.h"

namespace PortabilityLayer
{
	namespace ByteSwap
	{
		void BigInt16(int16_t &v)
		{
			const uint8_t *asU8 = reinterpret_cast<const uint8_t*>(&v);
			const int8_t *asS8 = reinterpret_cast<const int8_t*>(&v);

			v = static_cast<int16_t>((asS8[0] << 8) | asU8[1]);
		}

		void BigInt32(int32_t &v)
		{
			const uint8_t *asU8 = reinterpret_cast<const uint8_t*>(&v);
			const int8_t *asS8 = reinterpret_cast<const int8_t*>(&v);

			v = static_cast<int32_t>((asS8[0] << 24) | (asU8[1] << 16) | (asU8[2] << 8) | asU8[3]);
		}

		void BigUInt16(uint16_t &v)
		{
			const uint8_t *asU8 = reinterpret_cast<const uint8_t*>(&v);

			v = static_cast<uint16_t>((asU8[0] << 8) | asU8[1]);
		}

		void BigUInt32(uint32_t &v)
		{
			const uint8_t *asU8 = reinterpret_cast<const uint8_t*>(&v);

			v = static_cast<uint32_t>((asU8[0] << 24) | (asU8[1] << 16) | (asU8[2] << 8) | asU8[3]);
		}
	}
}
