#pragma once

#include "EGpInputDriverType.h"

struct IGpAudioDriver;
struct IGpVOSEventQueue;
struct IGpAllocator;

struct GpInputDriverProperties
{
	EGpInputDriverType m_type;

	IGpVOSEventQueue *m_eventQueue;
	IGpAllocator *m_alloc;
};
