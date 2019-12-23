#pragma once

#include <stdint.h>

#include "GpPixelFormat.h"
#include "QDState.h"
#include "QDPort.h"

struct PixMap;
struct Rect;
struct IGpDisplayDriverSurface;

struct CGraf 
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

	int Init(const Rect &rect, GpPixelFormat_t pixelFormat)
	{
		if (int errorCode = m_port.Init(rect, pixelFormat))
			return errorCode;

		return 0;
	}

	bool Resize(const Rect &rect)
	{
		return m_port.Resize(rect);
	}

	// Must be the first item
	PortabilityLayer::QDPort m_port;

	IGpDisplayDriverSurface *m_ddSurface;
};
