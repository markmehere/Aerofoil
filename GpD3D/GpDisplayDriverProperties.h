#pragma once

#include "EGpDisplayDriverType.h"

class IGpDisplayDriver;
class GpFiber;

struct GpDisplayDriverProperties
{
	typedef void(*TickFunc_t)(void *context, GpFiber *vosFiber);

	EGpDisplayDriverType m_Type;

	unsigned int m_FrameTimeLockNumerator;
	unsigned int m_FrameTimeLockDenominator;

	unsigned int m_FrameTimeLockMinNumerator;
	unsigned int m_FrameTimeLockMinDenominator;

	unsigned int m_FrameTimeLockMaxNumerator;
	unsigned int m_FrameTimeLockMaxDenominator;

	// Tick function and context to call when a frame needs to be served.
	TickFunc_t m_TickFunc;
	void *m_TickFuncContext;
};
