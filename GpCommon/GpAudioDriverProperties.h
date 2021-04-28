#pragma once

#include "EGpAudioDriverType.h"

struct IGpSystemServices;
struct IGpAudioDriver;
struct IGpLogDriver;
struct IGpAllocator;

struct GpAudioDriverProperties
{
	EGpAudioDriverType m_type;

	unsigned int m_sampleRate;
	bool m_debug;

	IGpLogDriver *m_logger;
	IGpSystemServices *m_systemServices;
	IGpAllocator *m_alloc;
};
