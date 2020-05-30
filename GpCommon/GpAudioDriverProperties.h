#pragma once

#include "EGpAudioDriverType.h"

struct IGpAudioDriver;
struct IGpLogDriver;

struct GpAudioDriverProperties
{
	EGpAudioDriverType m_type;

	unsigned int m_sampleRate;
	bool m_debug;

	IGpLogDriver *m_logger;
};
