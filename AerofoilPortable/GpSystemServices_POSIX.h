#pragma once

#include "IGpSystemServices.h"
#include "GpCoreDefs.h"

class GpSystemServices_POSIX : public IGpSystemServices
{
public:
	GpSystemServices_POSIX();

	int64_t GetTime() const override;
	void GetLocalDateTime(unsigned int &year, unsigned int &month, unsigned int &day, unsigned int &hour, unsigned int &minute, unsigned int &second) const override;
	IGpMutex *CreateMutex() override;
	IGpMutex *CreateRecursiveMutex() override;
	IGpThreadEvent *CreateThreadEvent(bool autoReset, bool startSignaled) override;
	uint64_t GetFreeMemoryCosmetic() const override;
};
