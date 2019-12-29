#include "ScanlineMaskIterator.h"

namespace PortabilityLayer
{
	ScanlineMaskIterator::ScanlineMaskIterator(const void *data, ScanlineMaskDataStorage dataStorage)
		: m_loc(data)
		, m_storage(dataStorage)
	{
	}

	size_t ScanlineMaskIterator::Next()
	{
		switch (m_storage)
		{
		case ScanlineMaskDataStorage_UInt8:
			{
				const uint8_t *loc = static_cast<const uint8_t*>(m_loc);
				uint8_t value = *loc;
				m_loc = loc + 1;
				return value;
			}
		case ScanlineMaskDataStorage_UInt16:
			{
				const uint16_t *loc = static_cast<const uint16_t*>(m_loc);
				uint16_t value = *loc;
				m_loc = loc + 1;
				return value;
			}
		case ScanlineMaskDataStorage_UInt32:
			{
				const uint32_t *loc = static_cast<const uint32_t*>(m_loc);
				uint32_t value = *loc;
				m_loc = loc + 1;
				return value;
			}
		default:
			return 0;
		}
	}
}
