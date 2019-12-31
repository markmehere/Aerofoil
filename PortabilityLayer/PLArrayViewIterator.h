#pragma once

#include <stdint.h>

template<class T>
class ArrayViewIterator
{
public:
	ArrayViewIterator(T *item);
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
	T *m_iter;
};

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
inline ArrayViewIterator<T> ArrayViewIterator<T>::operator++(int)
{
	ArrayViewIterator<T> copy = *this;
	m_iter++;
	return copy;
}

template<class T>
inline ArrayViewIterator<T> &ArrayViewIterator<T>::operator++()
{
	m_iter++;
	return *this;
}

template<class T>
inline ArrayViewIterator<T> ArrayViewIterator<T>::operator--(int)
{
	ArrayViewIterator<T> copy = *this;
	m_iter--;
	return copy;
}

template<class T>
inline ArrayViewIterator<T> &ArrayViewIterator<T>::operator--()
{
	m_iter--;
	return *this;
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
	return m_iter == other.m_iter;
}

template<class T>
inline ArrayViewIterator<T>::operator T*() const
{
	return m_iter;
}
