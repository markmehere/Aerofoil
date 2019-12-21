#pragma once

#include "HostDisplayDriver.h"

struct IGpDisplayDriver;

class GpPLGlueDisplayDriver final : public PortabilityLayer::HostDisplayDriver
{
public:
	GpPLGlueDisplayDriver();

	void GetDisplayResolution(unsigned int *width, unsigned int *height, PortabilityLayer::PixelFormat *bpp) override;
	void HideCursor() override;

	void SetGpDisplayDriver(IGpDisplayDriver *displayDriver);

	static GpPLGlueDisplayDriver *GetInstance();

private:
	IGpDisplayDriver *m_displayDriver;

	static GpPLGlueDisplayDriver ms_instance;
};
