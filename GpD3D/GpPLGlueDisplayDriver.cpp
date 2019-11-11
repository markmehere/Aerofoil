#include "GpPLGlueDisplayDriver.h"
#include "IGpDisplayDriver.h"

GpPLGlueDisplayDriver::GpPLGlueDisplayDriver()
	: m_displayDriver(nullptr)
{
}

void GpPLGlueDisplayDriver::GetDisplayResolution(unsigned int &width, unsigned int &height)
{
	m_displayDriver->GetDisplayResolution(width, height);
}

void GpPLGlueDisplayDriver::HideCursor()
{
}

GpPLGlueDisplayDriver *GpPLGlueDisplayDriver::GetInstance()
{
	return &ms_instance;
}

void GpPLGlueDisplayDriver::SetGpDisplayDriver(IGpDisplayDriver *displayDriver)
{
	m_displayDriver = displayDriver;
}

GpPLGlueDisplayDriver GpPLGlueDisplayDriver::ms_instance;
