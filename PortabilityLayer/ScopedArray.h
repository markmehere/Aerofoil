#pragma once

#ifndef __PL_SCOPEDARRAY_H__
#define __PL_SCOPEDARRAY_H__

#include "CoreDefs.h"

namespace PortabilityLayer
{
	template<class T>
	class ScopedArray
	{
	public:
		ScopedArray();
		ScopedArray(T *ref);
		~ScopedArray();

		void Swap(ScopedArray<T> &other);

		operator T*();
		operator const T*() const;

		void Set(T *ref);

	private:
		ScopedArray(const ScopedArray<T> &other) GP_DELETED;
		void operator=(const ScopedArray<T> &other) GP_DELETED;
		T *m_ref;
	};
}

namespace PortabilityLayer
{
	template<class T>
	inline ScopedArray<T>::ScopedArray()
		: m_ref(nullptr)
	{
	}

	template<class T>
	inline ScopedArray<T>::ScopedArray(T *ref)
		: m_ref(ref)
	{
	}

	template<class T>
	inline ScopedArray<T>::~ScopedArray()
	{
		if (m_ref)
			delete[] m_ref;
	}

	template<class T>
	inline void ScopedArray<T>::Swap(ScopedArray<T> &other)
	{
		T *temp = m_ref;
		m_ref = other.m_ref;
		other.m_ref = temp;
	}

	template<class T>
	inline ScopedArray<T>::operator T*()
	{
		return m_ref;
	}

	template<class T>
	inline ScopedArray<T>::operator const T*() const
	{
		return m_ref;
	}

	template<class T>
	inline void ScopedArray<T>::Set(T *ref)
	{
		if (m_ref && m_ref != ref)
			delete m_ref;

		m_ref = ref;
	}
}

#endif
