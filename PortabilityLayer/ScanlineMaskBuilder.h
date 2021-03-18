#pragma once

#include <stdint.h>
#include <stddef.h>

namespace PortabilityLayer
{
	class ScanlineMaskBuilder
	{
	public:
		ScanlineMaskBuilder();
		~ScanlineMaskBuilder();

		bool AppendSpan(size_t span);

		size_t GetLongestSpan() const;
		const size_t *GetSpans() const;
		size_t GetNumSpans() const;

	private:
		size_t *m_spans;
		size_t m_numSpans;
		size_t m_capacity;
		size_t m_longestSpan;
	};
}
