#pragma once

#include "CoreDefs.h"

#include <stdint.h>

template<class T>
class ArrayViewIterator
{
public:
#if GP_DEBUG_CONFIG
	ArrayViewIterator(T *items, size_t count, size_t index);
#else
	ArrayViewIterator(T *item);
#endif
	ArrayViewIterator(const ArrayViewIterator<T> &other);

	ArrayViewIterator<T> operator++(int);
	ArrayViewIterator<T> &operator++();

	ArrayViewIterator<T> operator--(int);
	ArrayViewIterator<T> &operator--();

	ArrayViewIterator<T> &operator+=(ptrdiff_t delta);
	ArrayViewIterator<T> &operator-=(ptrdiff_t delta);

	bool operator==(const ArrayViewIterator<T> &other) const;
	bool operator!=(const ArrayViewIterator<T> &other) const;

	operator T*() const;

private:
#if GP_DEBUG_CONFIG
	T *m_items;
	size_t m_count;
	size_t m_index;
#else
	T *m_iter;
#endif
};

#if GP_DEBUG_CONFIG

#include <assert.h>

template<class T>
inline ArrayViewIterator<T>::ArrayViewIterator(T *items, size_t count, size_t index)
	: m_items(items)
	, m_count(count)
	, m_index(index)
{
}

template<class T>
inline ArrayViewIterator<T>::ArrayViewIterator(const ArrayViewIterator<T> &other)
	: m_items(other.m_items)
	, m_count(other.m_count)
	, m_index(other.m_index)
{
}

template<class T>
inline ArrayViewIterator<T> &ArrayViewIterator<T>::operator+=(ptrdiff_t delta)
{
	if (delta < 0)
	{
		assert(-static_cast<ptrdiff_t>(m_index) >= delta);
	}
	else
	{
		assert(static_cast<ptrdiff_t>(m_count) >= delta && static_cast<ptrdiff_t>(m_count - m_index) >= delta);
	}

	m_index = static_cast<size_t>(static_cast<ptrdiff_t>(m_index) + delta);
	return *this;
}

template<class T>
inline ArrayViewIterator<T> &ArrayViewIterator<T>::operator-=(ptrdiff_t delta)
{
	if (delta >= 0)
	{
		assert(static_cast<ptrdiff_t>(m_index) >= delta);
	}
	else
	{
		assert(-static_cast<ptrdiff_t>(m_count) <= delta && -static_cast<ptrdiff_t>(m_count - m_index) <= delta);
	}

	m_index = static_cast<size_t>(static_cast<ptrdiff_t>(m_index) - delta);
	return *this;
}

template<class T>
inline bool ArrayViewIterator<T>::operator==(const ArrayViewIterator<T> &other) const
{
	return m_index == other.m_index && m_items == other.m_items;
}

template<class T>
inline bool ArrayViewIterator<T>::operator!=(const ArrayViewIterator<T> &other) const
{
	return m_index != other.m_index || m_items != other.m_items;
}

template<class T>
inline ArrayViewIterator<T>::operator T*() const
{
	assert(m_index < m_count);
	return m_items + m_index;
}

#else

template<class T>
inline ArrayViewIterator<T>::ArrayViewIterator(T *item)
	: m_iter(item)
{
}

template<class T>
inline ArrayViewIterator<T>::ArrayViewIterator(const ArrayViewIterator<T> &other)
	: m_iter(other.m_iter)
{
}

template<class T>
inline ArrayViewIterator<T> &ArrayViewIterator<T>::operator+=(ptrdiff_t delta)
{
	m_iter += delta;
	return *this;
}

template<class T>
inline ArrayViewIterator<T> &ArrayViewIterator<T>::operator-=(ptrdiff_t delta)
{
	m_iter += delta;
	return *this;
}

template<class T>
inline bool ArrayViewIterator<T>::operator==(const ArrayViewIterator<T> &other) const
{
	return m_iter == other.m_iter;
}

template<class T>
inline bool ArrayViewIterator<T>::operator!=(const ArrayViewIterator<T> &other) const
{
	return m_iter != other.m_iter;
}

template<class T>
inline ArrayViewIterator<T>::operator T*() const
{
	return m_iter;
}
#endif


template<class T>
inline ArrayViewIterator<T> ArrayViewIterator<T>::operator++(int)
{
	ArrayViewIterator<T> copy = *this;
	++(*this);
	return copy;
}


template<class T>
inline ArrayViewIterator<T> ArrayViewIterator<T>::operator--(int)
{
	ArrayViewIterator<T> copy = *this;
	--(*this);
	return copy;
}

template<class T>
inline ArrayViewIterator<T> &ArrayViewIterator<T>::operator++()
{
	(*this) += 1;
	return *this;
}

template<class T>
inline ArrayViewIterator<T> &ArrayViewIterator<T>::operator--()
{
	(*this) -= 1;
	return *this;
}
