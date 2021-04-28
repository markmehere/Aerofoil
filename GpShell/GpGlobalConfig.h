#pragma once

#include "EGpDisplayDriverType.h"
#include "EGpAudioDriverType.h"
#include "EGpFontHandlerType.h"
#include "EGpInputDriverType.h"

#include <stdint.h>
#include <stddef.h>

struct IGpLogDriver;
struct IGpSystemServices;
struct IGpAllocator;

struct GpGlobalConfig
{
	EGpDisplayDriverType m_displayDriverType;
	EGpAudioDriverType m_audioDriverType;
	EGpFontHandlerType m_fontHandlerType;

	const EGpInputDriverType *m_inputDriverTypes;
	size_t m_numInputDrivers;

	IGpLogDriver *m_logger;
	IGpSystemServices *m_systemServices;
	IGpAllocator *m_allocator;
	void *m_osGlobals;
};

extern GpGlobalConfig g_gpGlobalConfig;
