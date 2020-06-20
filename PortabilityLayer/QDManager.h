#pragma once

#include "GpPixelFormat.h"
#include "PLErrorCodes.h"

struct ColorTable;
struct DrawSurface;
struct Rect;

namespace PortabilityLayer
{
	class QDPort;

	class QDManager
	{
	public:
		virtual void Init() = 0;
		virtual PLError_t NewGWorld(DrawSurface **gw, GpPixelFormat_t pixelFormat, const Rect &bounds, ColorTable **colorTable) = 0;
		virtual void DisposeGWorld(DrawSurface *gw) = 0;

		static QDManager *GetInstance();
	};
}
