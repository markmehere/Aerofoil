#pragma once

#include "EGpDisplayDriverType.h"
#include "EGpAudioDriverType.h"
#include "EGpFontHandlerType.h"
#include "EGpInputDriverType.h"

struct IGpLogDriver;

struct GpGlobalConfig
{
	EGpDisplayDriverType m_displayDriverType;
	EGpAudioDriverType m_audioDriverType;
	EGpFontHandlerType m_fontHandlerType;

	const EGpInputDriverType *m_inputDriverTypes;
	size_t m_numInputDrivers;

	IGpLogDriver *m_logger;
	void *m_osGlobals;
};

extern GpGlobalConfig g_gpGlobalConfig;
