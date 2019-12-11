#pragma once

struct ColorTable;
struct CGraf;
struct GDevice;
struct Rect;

namespace PortabilityLayer
{
	class QDPort;
	struct QDState;

	class QDManager
	{
	public:
		virtual void Init() = 0;
		virtual void GetPort(QDPort **gw, GDevice ***gdHandle) = 0;
		virtual void SetPort(QDPort *gw, GDevice **gdHandle) = 0;
		virtual int NewGWorld(CGraf **gw, int depth, const Rect &bounds, ColorTable **colorTable, GDevice **device, int flags) = 0;

		virtual QDState *GetState() = 0;

		static QDManager *GetInstance();
	};
}
