#pragma once

template<class T>
class GpComPtr final
{
public:
	GpComPtr();
	const GpComPtr(const GpComPtr<T> &other);
	explicit GpComPtr(T *ptr);
	~GpComPtr();

	GpComPtr<T> &operator=(const GpComPtr<T> &other);
	GpComPtr<T> &operator=(T *other);

	bool operator==(const GpComPtr<T> &other) const;
	bool operator!=(const GpComPtr<T> &other) const;

	bool operator==(const T *other) const;
	bool operator!=(const T *other) const;

	operator T*() const;
	T *operator->() const;

	T **GetMutablePtr();

private:
	T *m_ptr;
};

template<class T>
inline GpComPtr<T>::GpComPtr()
	: m_ptr(nullptr)
{
}

template<class T>
inline GpComPtr<T>::GpComPtr(const GpComPtr<T> &other)
	: m_ptr(other.m_ptr)
{
	if (m_ptr)
		m_ptr->AddRef();
}

template<class T>
inline GpComPtr<T>::GpComPtr(T *ptr)
	: m_ptr(ptr)
{
	if (ptr)
		ptr->AddRef();
}

template<class T>
inline GpComPtr<T>::~GpComPtr()
{
	if (m_ptr)
		m_ptr->Release();
}

template<class T>
inline GpComPtr<T> &GpComPtr<T>::operator=(const GpComPtr<T> &other)
{
	(*this) = other.m_ptr;
	return *this;
}

template<class T>
inline GpComPtr<T> &GpComPtr<T>::operator=(T *other)
{
	if (other)
		other->AddRef();

	if (m_ptr)
		m_ptr->Release();

	m_ptr = other;

	return *this;
}

template<class T>
inline bool GpComPtr<T>::operator==(const GpComPtr<T> &other) const
{
	return m_ptr == other.m_ptr;
}

template<class T>
inline bool GpComPtr<T>::operator!=(const GpComPtr<T> &other) const
{
	return !((*this) == other);
}

template<class T>
inline bool GpComPtr<T>::operator==(const T *other) const
{
	return m_ptr == other;
}

template<class T>
inline bool GpComPtr<T>::operator!=(const T *other) const
{
	return !((*this) == other);
}

template<class T>
T **GpComPtr<T>::GetMutablePtr()
{
	return &m_ptr;
}

template<class T>
inline GpComPtr<T>::operator T*() const
{
	return m_ptr;
}

template<class T>
inline T *GpComPtr<T>::operator->() const
{
	return m_ptr;
}
