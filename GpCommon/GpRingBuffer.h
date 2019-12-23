#pragma once

#include <stdint.h>
#include <assert.h>

#include "GpCoreDefs.h"

template<class TItem, size_t TCapacity>
class GpRingBuffer
{
public:
	GpRingBuffer()
		: m_Size(0)
		, m_Start(0)
	{
	}

	TItem &operator[](size_t index)
	{
		assert(index < m_Size);
		return m_Items[(m_Start + index) % TCapacity];
	}

	void RemoveFromStart()
	{
		assert(m_Size >= 1);
		m_Start = (m_Start + 1) % TCapacity;
		m_Size--;
	}

	void RemoveFromEnd()
	{
		assert(m_Size >= 1);
		m_Size--;
	}

	void Clear()
	{
		m_Size = 0;
		m_Start = 0;
	}

	size_t Size() const
	{
		return m_Size;
	}

	TItem *Append()
	{
		if (m_Size == TCapacity)
			return nullptr;

		m_Size++;
		return &m_Items[(m_Start + (m_Size - 1)) % TCapacity];
	}

	static const size_t CAPACITY = TCapacity;

private:
	TItem m_Items[TCapacity];
	size_t m_Size;
	size_t m_Start;
};