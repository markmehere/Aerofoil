#pragma once

#include "EGpDisplayDriverType.h"
#include "EGpAudioDriverType.h"
#include "EGpFontHandlerType.h"
#include "EGpInputDriverType.h"

struct IGpLogDriver;

namespace PortabilityLayer
{
	class HostSystemServices;
}

struct GpGlobalConfig
{
	EGpDisplayDriverType m_displayDriverType;
	EGpAudioDriverType m_audioDriverType;
	EGpFontHandlerType m_fontHandlerType;

	const EGpInputDriverType *m_inputDriverTypes;
	size_t m_numInputDrivers;

	IGpLogDriver *m_logger;
	PortabilityLayer::HostSystemServices *m_systemServices;
	void *m_osGlobals;
};

extern GpGlobalConfig g_gpGlobalConfig;
