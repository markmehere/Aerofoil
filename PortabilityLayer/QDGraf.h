#pragma once

#include <stdint.h>

#include "PixelFormat.h"
#include "QDState.h"
#include "QDPort.h"

struct PixMap;
struct Rect;

struct CGraf 
{
	CGraf()
		: m_port(PortabilityLayer::QDPortType_CGraf)
	{
	}

	int Init(const Rect &rect, PortabilityLayer::PixelFormat pixelFormat)
	{
		if (int errorCode = m_port.Init(rect, pixelFormat))
			return errorCode;

		return 0;
	}

	// Must be the first item
	PortabilityLayer::QDPort m_port;
};

namespace PortabilityLayer
{
	class CGrafImpl final : public CGraf
	{
	};
}
