#pragma once

#include "HostDisplayDriver.h"

struct IGpDisplayDriver;

class GpPLGlueDisplayDriver final : public PortabilityLayer::HostDisplayDriver
{
public:
	GpPLGlueDisplayDriver();

	void GetDisplayResolution(unsigned int *width, unsigned int *height, PortabilityLayer::PixelFormat *bpp) override;
	IGpColorCursor *LoadColorCursor(int id) override;
	void SetColorCursor(IGpColorCursor *colorCursor) override;
	void SetStandardCursor(EGpStandardCursor_t standardCursor) override;

	void SetGpDisplayDriver(IGpDisplayDriver *displayDriver);

	static GpPLGlueDisplayDriver *GetInstance();

private:
	IGpDisplayDriver *m_displayDriver;

	static GpPLGlueDisplayDriver ms_instance;
};
