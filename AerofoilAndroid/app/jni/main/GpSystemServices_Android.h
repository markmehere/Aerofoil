#pragma once

#include "HostSystemServices.h"
#include "GpCoreDefs.h"

class GpSystemServices_Android final : public PortabilityLayer::HostSystemServices
{
public:
	GpSystemServices_Android();

	int64_t GetTime() const override;
	void GetLocalDateTime(unsigned int &year, unsigned int &month, unsigned int &day, unsigned int &hour, unsigned int &minute, unsigned int &second) const override;
	PortabilityLayer::HostMutex *CreateMutex() override;
	PortabilityLayer::HostMutex *CreateRecursiveMutex() override;
	PortabilityLayer::HostThreadEvent *CreateThreadEvent(bool autoReset, bool startSignaled) override;
	uint64_t GetFreeMemoryCosmetic() const override;
	void Beep() const override;

	static GpSystemServices_Android *GetInstance();

private:
	static GpSystemServices_Android ms_instance;
};
