#pragma once

#include "EGpDisplayDriverType.h"
#include "GpDisplayDriverTickStatus.h"

struct IGpDisplayDriver;
struct IGpFiber;
struct IGpVOSEventQueue;

struct GpDisplayDriverProperties
{
	typedef GpDisplayDriverTickStatus_t (*TickFunc_t)(void *context, IGpFiber *vosFiber);
	typedef void(*RenderFunc_t)(void *context);

	EGpDisplayDriverType m_type;

	unsigned int m_frameTimeLockNumerator;
	unsigned int m_frameTimeLockDenominator;

	unsigned int m_frameTimeLockMinNumerator;
	unsigned int m_frameTimeLockMinDenominator;

	unsigned int m_frameTimeLockMaxNumerator;
	unsigned int m_frameTimeLockMaxDenominator;

	void *m_osGlobals;

	// Tick function and context to call when a frame needs to be served.
	TickFunc_t m_tickFunc;
	void *m_tickFuncContext;

	RenderFunc_t m_renderFunc;
	void *m_renderFuncContext;

	IGpVOSEventQueue *m_eventQueue;
};
