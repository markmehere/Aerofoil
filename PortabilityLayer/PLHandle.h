#pragma once

namespace PortabilityLayer
{
	struct MMHandleBlock;
}

class THandleBase
{
public:
	explicit THandleBase(PortabilityLayer::MMHandleBlock *hdl);
	THandleBase(const THandleBase &other);

	PortabilityLayer::MMHandleBlock *MMBlock() const;

	void Dispose();

protected:
	PortabilityLayer::MMHandleBlock *m_hdl;
};

template<class T>
class THandle final : public THandleBase
{
public:
	THandle();
	THandle(T **hdl);
	explicit THandle(PortabilityLayer::MMHandleBlock *hdl);
	THandle(const THandle<T> &other);

	operator T **() const;

	template<class TOther>
	THandle<TOther> StaticCast() const;

	template<class TOther>
	THandle<TOther> ReinterpretCast() const;

	bool operator==(const THandle<T> &other) const;
	bool operator!=(const THandle<T> &other) const;

	bool operator==(T** other) const;
	bool operator!=(T** other) const;
};

typedef THandle<void> Handle;

#include "MMHandleBlock.h"

inline THandleBase::THandleBase(PortabilityLayer::MMHandleBlock *hdl)
	: m_hdl(hdl)
{
}

inline THandleBase::THandleBase(const THandleBase &other)
	: m_hdl(other.m_hdl)
{
}

inline PortabilityLayer::MMHandleBlock *THandleBase::MMBlock() const
{
	return m_hdl;
}

template<class T>
inline THandle<T>::THandle()
	: THandleBase(nullptr)
{
}

template<class T>
inline THandle<T>::THandle(T **hdl)
	: THandleBase(reinterpret_cast<PortabilityLayer::MMHandleBlock*>(hdl))
{
}

template<class T>
inline THandle<T>::THandle(PortabilityLayer::MMHandleBlock *hdl)
	: THandleBase(hdl)
{
}

template<class T>
inline THandle<T>::THandle(const THandle<T> &other)
	: THandleBase(other.m_hdl)
{
}

template<class T>
bool THandle<T>::operator==(const THandle<T> &other) const
{
	return m_hdl == other.m_hdl;
}

template<class T>
bool THandle<T>::operator!=(const THandle<T> &other) const
{
	return m_hdl != other.m_hdl;
}

template<class T>
bool THandle<T>::operator==(T** other) const
{
	return static_cast<void*>(&m_hdl->m_contents) == static_cast<void*>(other);
}

template<class T>
bool THandle<T>::operator!=(T** other) const
{
	return static_cast<void*>(&m_hdl->m_contents) != static_cast<void*>(other);
}

template<class T>
inline THandle<T>::operator T**() const
{
	return reinterpret_cast<T**>(&m_hdl->m_contents);
}

template<class T>
template<class TOther>
THandle<TOther> THandle<T>::StaticCast() const
{
	(void)(static_cast<TOther*>(static_cast<T*>(nullptr)));
	return THandle<TOther>(m_hdl);
}

template<class T>
template<class TOther>
THandle<TOther> THandle<T>::ReinterpretCast() const
{
	(void)(reinterpret_cast<TOther*>(static_cast<T*>(nullptr)));
	return THandle<TOther>(m_hdl);
}
