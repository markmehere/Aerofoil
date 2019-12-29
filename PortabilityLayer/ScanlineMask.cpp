#include "CoreDefs.h"
#include "ScanlineMask.h"
#include "ScanlineMaskBuilder.h"
#include "ScanlineMaskIterator.h"

#include <stdlib.h>
#include <new>

namespace PortabilityLayer
{
	void ScanlineMask::Destroy()
	{
		this->~ScanlineMask();
		free(this);
	}

	const Rect &ScanlineMask::GetRect() const
	{
		return m_rect;
	}

	ScanlineMaskIterator ScanlineMask::GetIterator() const
	{
		return ScanlineMaskIterator(m_data, m_dataStorage);
	}

	ScanlineMask *ScanlineMask::Create(const Rect &rect, const ScanlineMaskBuilder &builder)
	{
		size_t alignedPrefixSize = sizeof(ScanlineMask) + PL_SYSTEM_MEMORY_ALIGNMENT - 1;
		alignedPrefixSize -= alignedPrefixSize % PL_SYSTEM_MEMORY_ALIGNMENT;

		const size_t longestSpan = builder.GetLongestSpan();
		const size_t numSpans = builder.GetNumSpans();
		const size_t *spans = builder.GetSpans();

		size_t storageSize = numSpans;
		ScanlineMaskDataStorage dataStorage = ScanlineMaskDataStorage_UInt8;
		if (longestSpan <= 0xff)
			dataStorage = ScanlineMaskDataStorage_UInt8;
		else if (longestSpan <= 0xffff)
		{
			storageSize *= 2;
			dataStorage = ScanlineMaskDataStorage_UInt16;
		}
		else if (longestSpan <= 0xffffffff)
		{
			storageSize *= 4;
			dataStorage = ScanlineMaskDataStorage_UInt32;
		}
		else
			return nullptr;

		void *storage = malloc(alignedPrefixSize + storageSize);
		if (!storage)
			return nullptr;

		void *spanStorage = static_cast<uint8_t*>(storage) + alignedPrefixSize;

		ScanlineMask *mask = new (storage) ScanlineMask(rect, dataStorage, spanStorage, numSpans);

		for (size_t i = 0; i < numSpans; i++)
		{
			switch (dataStorage)
			{
			case ScanlineMaskDataStorage_UInt8:
				static_cast<uint8_t*>(spanStorage)[i] = static_cast<uint8_t>(spans[i]);
				break;
			case ScanlineMaskDataStorage_UInt16:
				static_cast<uint16_t*>(spanStorage)[i] = static_cast<uint16_t>(spans[i]);
				break;
			case ScanlineMaskDataStorage_UInt32:
				static_cast<uint32_t*>(spanStorage)[i] = static_cast<uint32_t>(spans[i]);
				break;
			}
		}

		return mask;
	}

	ScanlineMask::ScanlineMask(const Rect &rect, ScanlineMaskDataStorage dataStorage, const void *data, size_t numSpans)
		: m_dataStorage(dataStorage)
		, m_data(data)
		, m_numSpans(numSpans)
		, m_rect(rect)
	{
	}

	ScanlineMask::~ScanlineMask()
	{
	}
}

