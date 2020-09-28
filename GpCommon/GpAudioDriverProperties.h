#pragma once

#include "EGpAudioDriverType.h"

namespace PortabilityLayer
{
	class HostSystemServices;
}

struct IGpAudioDriver;
struct IGpLogDriver;

struct GpAudioDriverProperties
{
	EGpAudioDriverType m_type;

	unsigned int m_sampleRate;
	bool m_debug;

	IGpLogDriver *m_logger;
	PortabilityLayer::HostSystemServices *m_systemServices;
};
