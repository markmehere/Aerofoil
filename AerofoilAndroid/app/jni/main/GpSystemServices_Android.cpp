#include "GpSystemServices_Android.h"
#include "HostMutex.h"
#include "HostThreadEvent.h"

#include <time.h>
#include <mutex>
#include <condition_variable>

template<class TMutex>
class GpMutex_Cpp11 final : public PortabilityLayer::HostMutex
{
public:
	GpMutex_Cpp11();
	~GpMutex_Cpp11();

	void Destroy() override;

	void Lock() override;
	void Unlock() override;

private:
	TMutex m_mutex;
};

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
void GpMutex_Cpp11<TMutex>::Lock()
{
	m_mutex.lock();
}

template<class TMutex>
void GpMutex_Cpp11<TMutex>::Unlock()
{
	m_mutex.unlock();
}

typedef GpMutex_Cpp11<std::mutex> GpMutex_Cpp11_Vanilla;
typedef GpMutex_Cpp11<std::recursive_mutex> GpMutex_Cpp11_Recursive;


class GpThreadEvent_Cpp11 final : public PortabilityLayer::HostThreadEvent
{
public:
	GpThreadEvent_Cpp11(bool autoReset, bool startSignaled);
	~GpThreadEvent_Cpp11();

	void Wait() override;
	bool WaitTimed(uint32_t msec) override;
	void Signal() override;
	void Destroy() override;

private:
	std::mutex m_mutex;
	std::condition_variable m_cvar;
	bool m_flag;
	bool m_autoReset;
};

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

GpSystemServices_Android::GpSystemServices_Android()
{
}

int64_t GpSystemServices_Android::GetTime() const
{
	time_t t = time(nullptr);
	return static_cast<int64_t>(t) - 2082844800;
}

void GpSystemServices_Android::GetLocalDateTime(unsigned int &year, unsigned int &month, unsigned int &day, unsigned int &hour, unsigned int &minute, unsigned int &second) const
{
	time_t t = time(nullptr);
	tm *tmObject = localtime(&t);
	year = static_cast<unsigned int>(tmObject->tm_year);
	month = static_cast<unsigned int>(tmObject->tm_mon + 1);
	hour = static_cast<unsigned int>(tmObject->tm_hour);
	minute = static_cast<unsigned int>(tmObject->tm_min);
	second = static_cast<unsigned int>(tmObject->tm_sec);
}

PortabilityLayer::HostMutex *GpSystemServices_Android::CreateMutex()
{
	GpMutex_Cpp11_Vanilla *mutex = static_cast<GpMutex_Cpp11_Vanilla*>(malloc(sizeof(GpMutex_Cpp11_Vanilla)));
	if (!mutex)
		return nullptr;

	return new (mutex) GpMutex_Cpp11_Vanilla();
}

PortabilityLayer::HostMutex *GpSystemServices_Android::CreateRecursiveMutex()
{
	GpMutex_Cpp11_Recursive *mutex = static_cast<GpMutex_Cpp11_Recursive*>(malloc(sizeof(GpMutex_Cpp11_Recursive)));
	if (!mutex)
		return nullptr;

	return new (mutex) GpMutex_Cpp11_Recursive();
}

PortabilityLayer::HostThreadEvent *GpSystemServices_Android::CreateThreadEvent(bool autoReset, bool startSignaled)
{
	GpThreadEvent_Cpp11 *evt = static_cast<GpThreadEvent_Cpp11*>(malloc(sizeof(GpThreadEvent_Cpp11)));
	if (!evt)
		return nullptr;

	return new (evt) GpThreadEvent_Cpp11(autoReset, startSignaled);
}

uint64_t GpSystemServices_Android::GetFreeMemoryCosmetic() const
{
	return 0;
}

void GpSystemServices_Android::Beep() const
{
}

bool GpSystemServices_Android::IsTouchscreen() const
{
	return true;
}

bool GpSystemServices_Android::IsUsingMouseAsTouch() const
{
	return true;
}

bool GpSystemServices_Android::IsTextInputObstructive() const
{
	return true;
}

GpSystemServices_Android *GpSystemServices_Android::GetInstance()
{
	return &ms_instance;
}

GpSystemServices_Android GpSystemServices_Android::ms_instance;
