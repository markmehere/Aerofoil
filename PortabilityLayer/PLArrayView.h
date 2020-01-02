#pragma once

#include <stdint.h>

template<class T>
class ArrayViewIterator;

template<class T>
class ArrayView
{
public:
	ArrayView(const T *items, size_t count);
	ArrayView(const ArrayView<T> &other);

	size_t Count() const;
	const T &operator[](size_t index) const;

	ArrayViewIterator<T> begin() const;
	ArrayViewIterator<T> end() const;

private:
	const T *m_items;
	size_t m_count;
};

#include "CoreDefs.h"
#include "PLArrayViewIterator.h"

#if GP_DEBUG_CONFIG
#include <assert.h>
#endif

template<class T>
inline ArrayView<T>::ArrayView(const T *items, size_t count)
	: m_items(items)
	, m_count(count)
{
}

template<class T>
inline ArrayView<T>::ArrayView(const ArrayView<T> &other)
	: m_items(other.m_items)
	, m_count(other.m_count)
{
}

template<class T>
inline size_t ArrayView<T>::Count() const
{
	return m_count;
}

template<class T>
const T &ArrayView<T>::operator[](size_t index) const
{
#if GP_DEBUG_CONFIG
	assert(index < m_count);
#endif

	return m_items[index];
}

template<class T>
inline ArrayViewIterator<T> ArrayView<T>::begin() const
{
#if GP_DEBUG_CONFIG
	return ArrayViewIterator<T>(m_items, m_count, 0);
#else
	return ArrayViewIterator<T>(m_items);
#endif
}

template<class T>
inline ArrayViewIterator<T> ArrayView<T>::end() const
{
#if GP_DEBUG_CONFIG
	return ArrayViewIterator<T>(m_items, m_count, m_count);
#else
	return ArrayViewIterator<T>(m_items + m_count);
#endif
}
