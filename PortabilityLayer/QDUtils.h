#pragma once

struct Region;
struct Rect;

namespace PortabilityLayer
{
	class QDUtils
	{
	public:
		static Region **CreateRegion(const Rect &rect);
		static void ResetRegionToRect(Region **region, const Rect &rect);
	};
}
