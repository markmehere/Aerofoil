#pragma once

#include "IPlotter.h"
#include "PlotDirection.h"
#include "Vec2i.h"

#define PL_DEBUG_ELLIPSE_PLOTTER 1

namespace PortabilityLayer
{
	struct Rect2i;

	class EllipsePlotter final : public IPlotter
	{
	public:
		EllipsePlotter();
		PlotDirection PlotNext() override;
		const Vec2i &GetPoint() const override;

		void Reset(const Rect2i &bounds);

	private:
		enum Quadrant
		{
			Quadrant_NxNy,
			Quadrant_NxPy,
			Quadrant_PxNy,
			Quadrant_PxPy,

			Quadrant_Finished
		};

		void IncrementX();
		bool ConditionalIncrementX();

		void IncrementY();
		bool ConditionalIncrementY();

		void DecrementX();
		bool ConditionalDecrementX();

		void DecrementY();
		bool ConditionalDecrementY();

		Vec2i m_2center;
		Vec2i m_point;
		Vec2i m_diameters;
		int32_t m_sqDistFromEdge;
		Quadrant m_quadrant;

		int32_t m_xChangeCostDynamicFactor;
		int32_t m_yChangeCostDynamicFactor;
		int32_t m_xChangeCostDynamicFactorStep;
		int32_t m_yChangeCostDynamicFactorStep;
		int32_t m_xChangeCostStaticFactor;
		int32_t m_yChangeCostStaticFactor;

#if PL_DEBUG_ELLIPSE_PLOTTER 1
		Vec2i m_2offsetFromCenter;
#endif
	};
}
