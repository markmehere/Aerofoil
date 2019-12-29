#pragma once

#include <stdint.h>

#include "SharedTypes.h"
#include "ScanlineMaskDataStorage.h"

namespace PortabilityLayer
{
	class ScanlineMaskBuilder;
	class ScanlineMaskIterator;

	class ScanlineMask
	{
	public:
		void Destroy();
		const Rect &GetRect() const;
		ScanlineMaskIterator GetIterator() const;

		static ScanlineMask *Create(const Rect &rect, const ScanlineMaskBuilder &builder);

	private:
		explicit ScanlineMask(const Rect &rect, ScanlineMaskDataStorage dataStorage, const void *data, size_t numSpans);
		~ScanlineMask();

		const ScanlineMaskDataStorage m_dataStorage;
		const void *m_data;
		const size_t m_numSpans;
		const Rect m_rect;
	};
}
