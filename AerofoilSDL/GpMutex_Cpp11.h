#pragma once

#include "IGpMutex.h"
#include <mutex>

template<class TMutex>
class GpMutex_Cpp11 final : public IGpMutex
{
public:
	~GpMutex_Cpp11();

	void Destroy() override;
	static GpMutex_Cpp11<TMutex> *Create();

	void Lock() override;
	void Unlock() override;

private:
	GpMutex_Cpp11();

	TMutex m_mutex;
};

#include <stdlib.h>

template<class TMutex>
GpMutex_Cpp11<TMutex>::GpMutex_Cpp11()
{
}

template<class TMutex>
GpMutex_Cpp11<TMutex>::~GpMutex_Cpp11()
{
}

template<class TMutex>
void GpMutex_Cpp11<TMutex>::Destroy()
{
	this->~GpMutex_Cpp11();
	free(this);
}

template<class TMutex>
GpMutex_Cpp11<TMutex> *GpMutex_Cpp11<TMutex>::Create()
{
	GpMutex_Cpp11<TMutex> *mutex = static_cast<GpMutex_Cpp11<TMutex>*>(malloc(sizeof(GpMutex_Cpp11<TMutex>)));
	if (!mutex)
		return nullptr;

	return new (mutex) GpMutex_Cpp11<TMutex>();
}

template<class TMutex>
void GpMutex_Cpp11<TMutex>::Lock()
{
	m_mutex.lock();
}

template<class TMutex>
void GpMutex_Cpp11<TMutex>::Unlock()
{
	m_mutex.unlock();
}

typedef GpMutex_Cpp11<std::mutex> GpMutex_Cpp11_NonRecursive;
typedef GpMutex_Cpp11<std::recursive_mutex> GpMutex_Cpp11_Recursive;
