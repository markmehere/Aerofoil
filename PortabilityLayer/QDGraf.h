#pragma once

#include <stdint.h>

#include "GpPixelFormat.h"
#include "QDState.h"
#include "QDPort.h"

struct PixMap;
struct Rect;
struct IGpDisplayDriver;
struct IGpDisplayDriverSurface;

struct CGraf final
{
	CGraf()
		: m_port(PortabilityLayer::QDPortType_CGraf)
		, m_ddSurface(nullptr)
	{
	}

	explicit CGraf(PortabilityLayer::QDPortType overridePortType)
		: m_port(overridePortType)
		, m_ddSurface(nullptr)
	{
	}

	PLError_t Init(const Rect &rect, GpPixelFormat_t pixelFormat)
	{
		if (PLError_t errorCode = m_port.Init(rect, pixelFormat))
			return errorCode;

		return PLErrors::kNone;
	}

	bool Resize(const Rect &rect)
	{
		return m_port.Resize(rect);
	}

	void PushToDDSurface(IGpDisplayDriver *displayDriver);

	// Must be the first item
	PortabilityLayer::QDPort m_port;

	IGpDisplayDriverSurface *m_ddSurface;
};
