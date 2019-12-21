#pragma once

#include "EGpAudioDriverType.h"

struct IGpAudioDriver;

struct GpAudioDriverProperties
{
	EGpAudioDriverType m_type;

	unsigned int m_sampleRate;
	bool m_debug;
};
