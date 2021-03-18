#pragma once

#include <stdint.h>
#include <stddef.h>

#include "ScanlineMaskDataStorage.h"

namespace PortabilityLayer
{
	class ScanlineMaskIterator
	{
	public:
		ScanlineMaskIterator(const void *data, ScanlineMaskDataStorage dataStorage);
		size_t Next();

	private:
		const void *m_loc;
		const ScanlineMaskDataStorage m_storage;
	};
}
