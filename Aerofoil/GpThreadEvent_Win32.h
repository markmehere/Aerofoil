#pragma once

#include "HostThreadEvent.h"

#include "GpWindows.h"

class GpThreadEvent_Win32 final : public PortabilityLayer::HostThreadEvent
{
public:
	void Wait() override;
	bool WaitTimed(uint32_t msec) override;
	void Signal() override;
	void Destroy() override;

	static GpThreadEvent_Win32 *Create(bool autoReset, bool startSignaled);

private:
	explicit GpThreadEvent_Win32(const HANDLE &handle);
	~GpThreadEvent_Win32();

	HANDLE m_event;
};
