#pragma once

#include "EGpDisplayDriverType.h"

struct GpGlobalConfig
{
	EGpDisplayDriverType m_displayDriverType;
	void *m_osGlobals;
};

extern GpGlobalConfig g_gpGlobalConfig;
