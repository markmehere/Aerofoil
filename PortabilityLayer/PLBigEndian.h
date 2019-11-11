#pragma once
#ifndef __PL_BIG_ENDIAN_H__
#define __PL_BIG_ENDIAN_H__

#include <stdint.h>

template<class T>
struct BEInteger
{
public:
	BEInteger();
	BEInteger(const BEInteger<T> &other);
	explicit BEInteger(T i);

	operator T() const;
	BEInteger<T> &operator=(T value);

	template<class TOther>
	BEInteger<T> &operator+=(TOther value);

	template<class TOther>
	BEInteger<T> &operator-=(TOther value);

	template<class TOther>
	BEInteger<T> &operator*=(TOther value);

	template<class TOther>
	BEInteger<T> &operator/=(TOther value);

	template<class TOther>
	BEInteger<T> &operator%=(TOther value);

	BEInteger<T>& operator--();
	BEInteger<T> operator--(int);

	BEInteger<T>& operator++();
	BEInteger<T> operator++(int);

private:
	T m_beValue;
};

template<class T>
struct BEInteger_SwapHelper
{
};

#include "ByteSwap.h"

template<>
struct BEInteger_SwapHelper<int16_t>
{
	inline static void Swap(int16_t &v)
	{
		PortabilityLayer::ByteSwap::BigInt16(v);
	}
};

template<>
struct BEInteger_SwapHelper<int32_t>
{
	inline static void Swap(int32_t &v)
	{
		PortabilityLayer::ByteSwap::BigInt32(v);
	}
};

template<>
struct BEInteger_SwapHelper<uint16_t>
{
	inline static void Swap(uint16_t &v)
	{
		PortabilityLayer::ByteSwap::BigUInt16(v);
	}
};

template<>
struct BEInteger_SwapHelper<uint32_t>
{
	inline static void Swap(uint32_t &v)
	{
		PortabilityLayer::ByteSwap::BigUInt32(v);
	}
};

// Int16
template<class T>
inline BEInteger<T>::BEInteger()
	: m_beValue(0)
{
}

template<class T>
inline BEInteger<T>::BEInteger(const BEInteger<T> &other)
	: m_beValue(other.m_beValue)
{
}

template<class T>
inline BEInteger<T>::BEInteger(T i)
	: m_beValue(i)
{
	BEInteger_SwapHelper<T>::Swap(m_beValue);
}

template<class T>
inline BEInteger<T>::operator T() const
{
	int16_t result = m_beValue;
	BEInteger_SwapHelper<T>::Swap(result);
	return result;
}

template<class T>
inline BEInteger<T> &BEInteger<T>::operator=(T value)
{
	BEInteger_SwapHelper<T>::Swap(value);
	m_beValue = value;
	return *this;
}

template<class T>
template<class TOther>
BEInteger<T> &BEInteger<T>::operator+=(TOther value)
{
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	m_beValue += value;
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	return *this;
}

template<class T>
template<class TOther>
BEInteger<T> &BEInteger<T>::operator-=(TOther value)
{
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	m_beValue -= value;
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	return *this;
}

template<class T>
template<class TOther>
BEInteger<T> &BEInteger<T>::operator*=(TOther value)
{
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	m_beValue *= value;
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	return *this;
}

template<class T>
template<class TOther>
BEInteger<T> &BEInteger<T>::operator/=(TOther value)
{
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	m_beValue /= value;
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	return *this;
}

template<class T>
template<class TOther>
BEInteger<T> &BEInteger<T>::operator%=(TOther value)
{
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	m_beValue %= value;
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	return *this;
}

template<class T>
BEInteger<T>& BEInteger<T>::operator--()
{
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	m_beValue--;
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	return *this;
}

template<class T>
BEInteger<T> BEInteger<T>::operator--(int)
{
	BEInteger<T> orig(*this);
	--(*this);
	return orig;
}

template<class T>
BEInteger<T>& BEInteger<T>::operator++()
{
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	m_beValue++;
	BEInteger_SwapHelper<T>::Swap(m_beValue);
	return *this;
}

template<class T>
BEInteger<T> BEInteger<T>::operator++(int)
{
	BEInteger<T> orig(*this);
	++(*this);
	return orig;
}


typedef BEInteger<int16_t> BEInt16_t;
typedef BEInteger<int32_t> BEInt32_t;
typedef BEInteger<uint16_t> BEUInt16_t;
typedef BEInteger<uint32_t> BEUInt32_t;

#endif
