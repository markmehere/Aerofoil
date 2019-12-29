#pragma once

#include "EGpDisplayDriverType.h"
#include "EGpAudioDriverType.h"
#include "EGpInputDriverType.h"

struct GpGlobalConfig
{
	EGpDisplayDriverType m_displayDriverType;
	EGpAudioDriverType m_audioDriverType;
	const EGpInputDriverType *m_inputDriverTypes;
	size_t m_numInputDrivers;

	void *m_osGlobals;
};

extern GpGlobalConfig g_gpGlobalConfig;
