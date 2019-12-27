#pragma once

#include "GpPixelFormat.h"

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
		virtual int NewGWorld(CGraf **gw, int depth, const Rect &bounds, ColorTable **colorTable, int flags) = 0;
		virtual void DisposeGWorld(CGraf *gw) = 0;

		virtual QDState *GetState() = 0;

		virtual int DepthForPixelFormat(GpPixelFormat_t pixelFormat) const = 0;

		static QDManager *GetInstance();
	};
}
