#pragma once

#include "HostSystemServices.h"
#include "GpCoreDefs.h"
#include "GpWindows.h"


class GpSystemServices_Win32 final : public PortabilityLayer::HostSystemServices
{
public:
	GpSystemServices_Win32();

	uint32_t GetTime() const override;

	static GpSystemServices_Win32 *GetInstance();

private:
	static GpSystemServices_Win32 ms_instance;
};
