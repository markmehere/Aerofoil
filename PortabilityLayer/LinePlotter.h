#pragma once

#include "IPlotter.h"
#include "PlotDirection.h"
#include "Vec2i.h"

namespace PortabilityLayer
{
	class LinePlotter final : public IPlotter
	{
	public:
		LinePlotter();
		PlotDirection PlotNext() override;
		const Vec2i &GetPoint() const override;

		void Reset(const Vec2i &pointA, const Vec2i &pointB);

	private:
		Vec2i m_point;
		Vec2i m_endPoint;
		int32_t m_dx;
		int32_t m_dy;
		int32_t m_err;
		bool m_xIncrPos;
		bool m_yIncrPos;

		PlotDirection m_xyMove;
		PlotDirection m_xMove;
		PlotDirection m_yMove;
	};
}
