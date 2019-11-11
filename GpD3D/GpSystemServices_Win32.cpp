#include "GpSystemServices_Win32.h"

#include <assert.h>

GpSystemServices_Win32::GpSystemServices_Win32()
{
}

uint32_t GpSystemServices_Win32::GetTime() const
{
	return 0;
}

GpSystemServices_Win32 *GpSystemServices_Win32::GetInstance()
{
	return &ms_instance;
}

GpSystemServices_Win32 GpSystemServices_Win32::ms_instance;
