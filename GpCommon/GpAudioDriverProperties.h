#pragma once

#include "EGpAudioDriverType.h"

class IGpAudioDriver;

struct GpAudioDriverProperties
{
	EGpAudioDriverType m_type;

	unsigned int m_sampleRate;
	bool m_debug;
};
