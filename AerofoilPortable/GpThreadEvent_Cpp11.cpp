#include "GpThreadEvent_Cpp11.h"

#include <stdlib.h>

GpThreadEvent_Cpp11::GpThreadEvent_Cpp11(bool autoReset, bool startSignaled)
	: m_flag(startSignaled)
	, m_autoReset(autoReset)
{
}

GpThreadEvent_Cpp11::~GpThreadEvent_Cpp11()
{
}

void GpThreadEvent_Cpp11::Wait()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_autoReset)
	{
		m_cvar.wait(lock,[&]()->bool{
			if (m_flag)
			{
				m_flag = false;
				return true;
			}
			else
				return false;
		});
	}
	else
		m_cvar.wait(lock,[&]()->bool{ return m_flag; });
}

bool GpThreadEvent_Cpp11::WaitTimed(uint32_t msec)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_autoReset)
	{
		if (!m_cvar.wait_for(lock, std::chrono::milliseconds(msec), [&]()->bool{
			if (m_flag)
			{
				m_flag = false;
				return true;
			}
			else
				return false;
		}))
			return false;
	}
	else
	{
		if (!m_cvar.wait_for(lock, std::chrono::milliseconds(msec), [&]()->bool{ return m_flag; }))
			return false;
	}
	return true;
}

void GpThreadEvent_Cpp11::Signal()
{
	m_mutex.lock();
	m_flag = true;
	m_mutex.unlock();
	if (m_autoReset)
		m_cvar.notify_one();
	else
		m_cvar.notify_all();
}

void GpThreadEvent_Cpp11::Destroy()
{
	this->~GpThreadEvent_Cpp11();
	free(this);
}


GpThreadEvent_Cpp11 *GpThreadEvent_Cpp11::Create(bool autoReset, bool startSignaled)
{
	GpThreadEvent_Cpp11 *evt = static_cast<GpThreadEvent_Cpp11*>(malloc(sizeof(GpThreadEvent_Cpp11)));
	if (!evt)
		return nullptr;

	return new (evt) GpThreadEvent_Cpp11(autoReset, startSignaled);
}

