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

	void Dispose() const;

protected:
	PortabilityLayer::MMHandleBlock *m_hdl;
};

template<class T>
class THandle final : public THandleBase
{
public:
	THandle();
	THandle(std::nullptr_t);
	THandle(T **hdl);
	explicit THandle(PortabilityLayer::MMHandleBlock *hdl);
	THandle(const THandle<T> &other);

	operator T *const*() const;
	operator T **();

	template<class TOther>
	THandle<TOther> StaticCast() const;

	template<class TOther>
	THandle<TOther> ReinterpretCast() const;

	template<class TOther>
	THandle<TOther> ImplicitCast() const;

	bool operator==(const THandle<T> &other) const;
	bool operator!=(const THandle<T> &other) const;

	bool operator==(T** other) const;
	bool operator!=(T** other) const;

	static THandle<T> NullPtr();
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
inline THandle<T>::THandle(std::nullptr_t)
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
inline bool THandle<T>::operator==(const THandle<T> &other) const
{
	return m_hdl == other.m_hdl;
}

template<class T>
inline bool THandle<T>::operator!=(const THandle<T> &other) const
{
	return m_hdl != other.m_hdl;
}

template<class T>
inline bool THandle<T>::operator==(T** other) const
{
	return static_cast<void*>(&m_hdl->m_contents) == static_cast<void*>(other);
}

template<class T>
inline bool THandle<T>::operator!=(T** other) const
{
	return static_cast<void*>(&m_hdl->m_contents) != static_cast<void*>(other);
}

template<class T>
THandle<T> THandle<T>::NullPtr()
{
	return THandle<T>(static_cast<PortabilityLayer::MMHandleBlock *>(nullptr));
}

template<class T>
inline THandle<T>::operator T*const*() const
{
	// Should use const_cast here but then I'd have to strip qualifiers, blah, do the lazy thing
	return (T*const*)(&m_hdl->m_contents);
}

template<class T>
inline THandle<T>::operator T**()
{
	// Should use const_cast here but then I'd have to strip qualifiers, blah, do the lazy thing
	return (T**)(&m_hdl->m_contents);
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

template<class T>
template<class TOther>
THandle<TOther> THandle<T>::ImplicitCast() const
{
	const TOther *target = static_cast<const T*>(nullptr);
	(void)target;

	return THandle<TOther>(m_hdl);
}
