#pragma once

#include "CoreDefs.h"

#include <stdint.h>

struct IGpAllocator;

template<class T, size_t TStaticSize>
class GpVectorStaticData
{
protected:
	GP_ALIGNED(GP_SYSTEM_MEMORY_ALIGNMENT) uint8_t m_staticElementData[sizeof(T) * TStaticSize];

	T *GetStaticElements();
	const T *GetStaticElements() const;
};

template<class T>
class GpVectorStaticData<T, 0>
{
protected:
	T *GetStaticElements();
	const T *GetStaticElements() const;
};


template<class T, size_t TStaticSize = 0>
class GpVector : public GpVectorStaticData<T, TStaticSize>
{
public:
	explicit GpVector(IGpAllocator *alloc);
	GpVector(GpVector<T, TStaticSize> &&other);
	~GpVector();

	T &operator[](size_t index);
	const T &operator[](size_t index) const;

	bool Resize(size_t newSize);
	bool ResizeNoConstruct(size_t newSize);

	T *Buffer();
	const T *Buffer() const;

	const size_t Count() const;

private:
	GpVector(const GpVector<T, TStaticSize> &other) GP_DELETED;

	static const size_t kStaticSize = TStaticSize;

	T *m_elements;
	size_t m_capacity;
	size_t m_count;
	IGpAllocator *m_alloc;
};

#include <new>
#include <cassert>
#include "IGpAllocator.h"

template<class T, size_t TStaticSize>
T *GpVectorStaticData<T, TStaticSize>::GetStaticElements()
{
	return reinterpret_cast<T*>(this->m_staticElementData);
}

template<class T, size_t TStaticSize>
const T *GpVectorStaticData<T, TStaticSize>::GetStaticElements() const
{
	return reinterpret_cast<const T*>(this->m_staticElementData);
}

template<class T>
T *GpVectorStaticData<T, 0>::GetStaticElements()
{
	return nullptr;
}

template<class T>
const T *GpVectorStaticData<T, 0>::GetStaticElements() const
{
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// GpVector
template<class T, size_t TStaticSize>
GpVector<T, TStaticSize>::GpVector(IGpAllocator *alloc)
	: m_elements(this->GetStaticElements())
	, m_capacity(TStaticSize)
	, m_count(0)
	, m_alloc(alloc)
{
}

template<class T, size_t TStaticSize>
GpVector<T, TStaticSize>::GpVector(GpVector<T, TStaticSize> &&other)
	: m_elements(other.m_elements)
	, m_capacity(other.m_capacity)
	, m_count(other.m_count)
	, m_alloc(other.m_alloc)
{
	if (m_capacity <= TStaticSize)
	{
		const size_t count = m_count;
		T *elements = this->GetStaticElements();
		const T *srcElements = other.m_elements;
		m_elements = elements;
		for (size_t i = 0; i < count; i++)
			new (m_elements + i) T(static_cast<T&&>(other.m_elements));
	}

	other.m_count = 0;
	other.m_capacity = 0;
	other.m_elements = nullptr;
}

template<class T, size_t TStaticSize>
GpVector<T, TStaticSize>::~GpVector()
{
	T *elements = m_elements;
	size_t remaining = m_count;

	while (remaining > 0)
	{
		remaining--;
		elements[remaining].~T();
	}

	if (m_capacity > TStaticSize)
	{
		m_alloc->Release(m_elements);
	}
}

template<class T, size_t TStaticSize>
T &GpVector<T, TStaticSize>::operator[](size_t index)
{
	assert(index <= m_count);
	return m_elements[index];
}

template<class T, size_t TStaticSize>
const T &GpVector<T, TStaticSize>::operator[](size_t index) const
{
	assert(index <= m_count);
	return m_elements[index];
}


template<class T, size_t TStaticSize>
bool GpVector<T, TStaticSize>::Resize(size_t newSize)
{
	const size_t oldCount = m_count;

	if (!ResizeNoConstruct(newSize))
		return false;

	for (size_t i = oldCount; i < newSize; i++)
		new (m_elements + i) T();

	return true;
}

template<class T, size_t TStaticSize>
bool GpVector<T, TStaticSize>::ResizeNoConstruct(size_t newSize)
{
	T *elements = m_elements;

	if (newSize <= m_count)
	{
		size_t count = m_count;
		while (count > newSize)
		{
			count--;
			m_elements[count].~T();
		}

		m_count = count;
		return true;
	}

	if (newSize <= m_capacity)
	{
		m_count = newSize;
		return true;
	}

	size_t newCapacity = newSize;
	assert(newCapacity > kStaticSize);

	T *newElements = static_cast<T*>(m_alloc->Alloc(newCapacity * sizeof(T)));
	if (!newElements)
		return false;

	const size_t oldCount = m_count;
	for (size_t i = 0; i < oldCount; i++)
		new (newElements + i) T(static_cast<T&&>(elements[i]));

	for (size_t i = 0; i < oldCount; i++)
		elements[oldCount - 1 - i].~T();

	if (m_capacity > kStaticSize)
		m_alloc->Release(m_elements);

	m_elements = newElements;
	m_capacity = newCapacity;
	m_count = newSize;

	return true;
}

template<class T, size_t TStaticSize>
const size_t GpVector<T, TStaticSize>::Count() const
{
	return m_count;
}

template<class T, size_t TStaticSize>
T *GpVector<T, TStaticSize>::Buffer()
{
	return m_elements;
}

template<class T, size_t TStaticSize>
const T *GpVector<T, TStaticSize>::Buffer() const
{
	return m_elements;
}
