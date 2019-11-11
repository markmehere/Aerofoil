#pragma once

#include "HostDisplayDriver.h"

class IGpDisplayDriver;

class GpPLGlueDisplayDriver final : public PortabilityLayer::HostDisplayDriver
{
public:
	GpPLGlueDisplayDriver();

	void GetDisplayResolution(unsigned int &width, unsigned int &height) override;
	void HideCursor() override;

	void SetGpDisplayDriver(IGpDisplayDriver *displayDriver);

	static GpPLGlueDisplayDriver *GetInstance();

private:
	IGpDisplayDriver *m_displayDriver;

	static GpPLGlueDisplayDriver ms_instance;
};
