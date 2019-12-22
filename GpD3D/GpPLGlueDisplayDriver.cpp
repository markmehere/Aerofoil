#include "GpPLGlueDisplayDriver.h"
#include "VirtualDirectory.h"
#include "IGpDisplayDriver.h"

GpPLGlueDisplayDriver::GpPLGlueDisplayDriver()
	: m_displayDriver(nullptr)
{
}

void GpPLGlueDisplayDriver::GetDisplayResolution(unsigned int *width, unsigned int *height, PortabilityLayer::PixelFormat *bpp)
{
	m_displayDriver->GetDisplayResolution(width, height, bpp);
}

IGpColorCursor *GpPLGlueDisplayDriver::LoadColorCursor(int cursorID)
{
	return m_displayDriver->LoadColorCursor(cursorID);
}

void GpPLGlueDisplayDriver::SetColorCursor(IGpColorCursor *colorCursor)
{
	m_displayDriver->SetColorCursor(colorCursor);
}

void GpPLGlueDisplayDriver::SetStandardCursor(EGpStandardCursor_t standardCursor)
{
	m_displayDriver->SetStandardCursor(standardCursor);
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
