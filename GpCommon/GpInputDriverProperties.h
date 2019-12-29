#pragma once

#include "EGpInputDriverType.h"

struct IGpAudioDriver;
struct IGpVOSEventQueue;

struct GpInputDriverProperties
{
	EGpInputDriverType m_type;

	IGpVOSEventQueue *m_eventQueue;
};
