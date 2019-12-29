#pragma once

#include "GpPixelFormat.h"
#include "PLErrorCodes.h"

struct ColorTable;
struct CGraf;
struct Rect;

namespace PortabilityLayer
{
	class QDPort;
	struct QDState;

	class QDManager
	{
	public:
		virtual void Init() = 0;
		virtual QDPort *GetPort() const = 0;
		virtual void SetPort(QDPort *gw) = 0;
		virtual PLError_t NewGWorld(CGraf **gw, GpPixelFormat_t pixelFormat, const Rect &bounds, ColorTable **colorTable) = 0;
		virtual void DisposeGWorld(CGraf *gw) = 0;

		virtual QDState *GetState() = 0;

		static QDManager *GetInstance();
	};
}
