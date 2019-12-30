#include "ByteSwap.h"
#include "CoreDefs.h"

namespace PortabilityLayer
{
	namespace ByteSwap
	{
		template<class TNumberType, class TUnsignedType>
		void SwapArbitrary(TNumberType &v)
		{
			PL_STATIC_ASSERT(sizeof(TNumberType) == sizeof(TUnsignedType));

			uint8_t bytes[sizeof(TNumberType)];
			for (size_t i = 0; i < sizeof(TNumberType); i++)
				bytes[i] = reinterpret_cast<const uint8_t*>(&v)[i];

			TUnsignedType result = 0;
			for (size_t i = 0; i < sizeof(TNumberType); i++)
				result |= static_cast<TUnsignedType>(bytes[i]) << (sizeof(TUnsignedType) * 8 - 8 - (i * 8));

			v = static_cast<TNumberType>(result);
		}

		void BigInt16(int16_t &v)
		{
			SwapArbitrary<int16_t, uint16_t>(v);
		}

		void BigInt32(int32_t &v)
		{
			SwapArbitrary<int32_t, uint32_t>(v);
		}

		void BigInt64(int64_t &v)
		{
			SwapArbitrary<int64_t, uint64_t>(v);
		}

		void BigUInt16(uint16_t &v)
		{
			SwapArbitrary<uint16_t, uint16_t>(v);
		}

		void BigUInt32(uint32_t &v)
		{
			SwapArbitrary<uint32_t, uint32_t>(v);
		}

		void BigUInt64(uint64_t &v)
		{
			SwapArbitrary<uint64_t, uint64_t>(v);
		}
	}
}
