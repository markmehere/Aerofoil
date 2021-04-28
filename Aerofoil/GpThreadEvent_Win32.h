#pragma once

#include "IGpThreadEvent.h"

#include "GpWindows.h"

class GpThreadEvent_Win32 final : public IGpThreadEvent
{
public:
	void Wait() override;
	bool WaitTimed(uint32_t msec) override;
	void Signal() override;
	void Destroy() override;

	static GpThreadEvent_Win32 *Create(IGpAllocator *alloc, bool autoReset, bool startSignaled);

private:
	explicit GpThreadEvent_Win32(IGpAllocator *alloc, const HANDLE &handle);
	~GpThreadEvent_Win32();

	HANDLE m_event;
	IGpAllocator *m_alloc;
};
