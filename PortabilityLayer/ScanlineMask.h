#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	class ScanlineMaskBuilder;

	class ScanlineMask
	{
	public:
		void Destroy();

		static ScanlineMask *Create(const ScanlineMaskBuilder &builder);

	private:
		enum DataStorage
		{
			DataStorage_UInt8,
			DataStorage_UInt16,
			DataStorage_UInt32,
		};

		explicit ScanlineMask(DataStorage dataStorage, const void *data, size_t numSpans);
		~ScanlineMask();

		const DataStorage m_dataStorage;
		const void *m_data;
		const size_t m_numSpans;
	};
}
