#pragma once

#include "IGpThreadEvent.h"

#include <mutex>
#include <condition_variable>

class GpThreadEvent_Cpp11 final : public IGpThreadEvent
{
public:
	~GpThreadEvent_Cpp11();

	void Wait() override;
	bool WaitTimed(uint32_t msec) override;
	void Signal() override;
	void Destroy() override;

	static GpThreadEvent_Cpp11 *Create(bool autoReset, bool startSignaled);

private:
	GpThreadEvent_Cpp11(bool autoReset, bool startSignaled);
	GpThreadEvent_Cpp11() = delete;

	std::mutex m_mutex;
	std::condition_variable m_cvar;
	bool m_flag;
	bool m_autoReset;
};
