#pragma once

#include "HostSystemServices.h"
#include "GpCoreDefs.h"
#include "GpWindows.h"

#pragma push_macro("CreateMutex")
#ifdef CreateMutex
#undef CreateMutex
#endif


class GpSystemServices_Win32 final : public PortabilityLayer::HostSystemServices
{
public:
	GpSystemServices_Win32();

	int64_t GetTime() const override;
	void GetLocalDateTime(unsigned int &year, unsigned int &month, unsigned int &day, unsigned int &hour, unsigned int &minute, unsigned int &second) const override;
	PortabilityLayer::HostMutex *CreateMutex() override;
	PortabilityLayer::HostMutex *CreateRecursiveMutex() override;
	PortabilityLayer::HostThreadEvent *CreateThreadEvent(bool autoReset, bool startSignaled) override;
	uint64_t GetFreeMemoryCosmetic() const override;
	void Beep() const override;

	static GpSystemServices_Win32 *GetInstance();

private:
	static GpSystemServices_Win32 ms_instance;
};

#pragma pop_macro("CreateMutex")
