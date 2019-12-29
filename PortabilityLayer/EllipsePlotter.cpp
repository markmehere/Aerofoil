#include "EllipsePlotter.h"

#include "Rect2i.h"

#include <assert.h>

namespace
{
	static int32_t SquareInt32(int32_t v)
	{
		return v * v;
	}
}

namespace PortabilityLayer
{
	EllipsePlotter::EllipsePlotter()
		: m_2center(0, 0)
		, m_point(0, 0)
#if PL_DEBUG_ELLIPSE_PLOTTER 1
		, m_2offsetFromCenter(0, 0)
#endif
		, m_quadrant(Quadrant_PxPy)
		, m_sqDistFromEdge(0)
	{
	}

	PlotDirection EllipsePlotter::PlotNext()
	{
		// distance = (m_2offsetFromCenter.x*m_diameters.y)^2 + (m_2offsetFromCenter.y*m_diameters.x)^2 <= (m_diameters.x*m_diameters.y)^2
		// We want to minimize distance while keeping it less than (m_diameters.x*m_diameters.y)^2

		// Stepping X:
		// ((m_2offsetFromCenter.x + n)*m_diameters.y)^2 = (m_2offsetFromCenter.x*m_diameters.y)^2 + t
		// t = ((m_2offsetFromCenter.x + n)*m_diameters.y)^2 - (m_2offsetFromCenter.x*m_diameters.y)^2
		// t = (m_2offsetFromCenter.x*m_diameters.y + n*m_diameters.y)^2 - (m_2offsetFromCenter.x*m_diameters.y)^2
		// t = 2*(m_2offsetFromCenter.x*m_diameters.y*n*m_diameters.y) + (n*m_diameters.y)^2

		// For n=2:
		// t = 2 * (m_2offsetFromCenter.x*m_diameters.y*2*m_diameters.y) + (2*m_diameters.y) ^ 2
		// t = m_2offsetFromCenter.x * 4 * (m_diameters.y*)^2 + 4*(m_diameters.y) ^ 2

		// For n=-2
		// t = 2 * (m_2offsetFromCenter.x*m_diameters.y*-2*m_diameters.y) + (-2*m_diameters.y) ^ 2
		// t = m_2offsetFromCenter.x * -4 * (m_diameters.y*m_diameters.y) + 4 * (m_diameters.y) ^ 2

#if PL_DEBUG_ELLIPSE_PLOTTER
		{
			const int32_t diameterSq = SquareInt32(m_diameters.m_x*m_diameters.m_y);
			const int32_t sqDistX = SquareInt32(m_2offsetFromCenter.m_x*m_diameters.m_y);
			const int32_t sqDistY = SquareInt32(m_2offsetFromCenter.m_y*m_diameters.m_x);

			assert(m_sqDistFromEdge >= 0);
			assert(sqDistX + sqDistY >= diameterSq);
			assert(sqDistX + sqDistY - diameterSq == m_sqDistFromEdge);
			assert(m_xChangeCostDynamicFactor == m_2offsetFromCenter.m_x * 4 * SquareInt32(m_diameters.m_y));
			assert(m_yChangeCostDynamicFactor == m_2offsetFromCenter.m_y * 4 * SquareInt32(m_diameters.m_x));

			Vec2i actualCoordinate = (m_2center + m_2offsetFromCenter);
			actualCoordinate.m_x /= 2;
			actualCoordinate.m_y /= 2;

			int n = 0;
		}
#endif

		PlotDirection plotDir = PlotDirection_Exhausted;

		switch (m_quadrant)
		{
		case Quadrant_PxPy:
			{
				const int32_t xStepCost = -m_xChangeCostDynamicFactor + m_xChangeCostStaticFactor;
				const int32_t yStepCost = m_yChangeCostDynamicFactor + m_yChangeCostStaticFactor;

				const int32_t diagonalCostDelta = xStepCost + yStepCost;

				if (diagonalCostDelta < 0)
				{
					// Diagonal movement will move closer to the edge (first octant)
					IncrementY();

					const int32_t distWithDiagonalMovement = m_sqDistFromEdge + xStepCost;
					if (distWithDiagonalMovement >= 0)
					{
						DecrementX();
						plotDir = PlotDirection_NegX_PosY;
					}
					else
						plotDir = PlotDirection_0X_PosY;
				}
				else
				{
					// Diagonal movement will move farther from the center (second octant)
					DecrementX();

					if (m_sqDistFromEdge < 0)
					{
						IncrementY();
						plotDir = PlotDirection_NegX_PosY;
					}
					else
						plotDir = PlotDirection_NegX_0Y;
				}

				if (m_xChangeCostDynamicFactor <= 0)
					m_quadrant = Quadrant_NxPy;

				return plotDir;
			}
			break;

		case Quadrant_NxPy:
			{
				const int32_t xStepCost = -m_xChangeCostDynamicFactor + m_xChangeCostStaticFactor;
				const int32_t yStepCost = -m_yChangeCostDynamicFactor + m_yChangeCostStaticFactor;

				const int32_t diagonalCostDelta = xStepCost + yStepCost;

				if (diagonalCostDelta < 0)
				{
					// Diagonal movement will move closer to the edge (first octant)
					DecrementX();

					const int32_t distWithDiagonalMovement = m_sqDistFromEdge + yStepCost;
					if (distWithDiagonalMovement >= 0)
					{
						DecrementY();
						plotDir = PlotDirection_NegX_NegY;
					}
					else
						plotDir = PlotDirection_NegX_0Y;
				}
				else
				{
					// Diagonal movement will move farther from the center (second octant)
					DecrementY();

					if (m_sqDistFromEdge < 0)
					{
						DecrementX();
						plotDir = PlotDirection_NegX_NegY;
					}
					else
						plotDir = PlotDirection_0X_NegY;
				}

				if (m_yChangeCostDynamicFactor <= 0)
					m_quadrant = Quadrant_NxNy;

				return plotDir;
			}
			break;

		case Quadrant_NxNy:
			{
				const int32_t xStepCost = m_xChangeCostDynamicFactor + m_xChangeCostStaticFactor;
				const int32_t yStepCost = -m_yChangeCostDynamicFactor + m_yChangeCostStaticFactor;

				const int32_t diagonalCostDelta = xStepCost + yStepCost;

				if (diagonalCostDelta < 0)
				{
					// Diagonal movement will move closer to the edge (first octant)
					DecrementY();

					const int32_t distWithDiagonalMovement = m_sqDistFromEdge + xStepCost;
					if (distWithDiagonalMovement >= 0)
					{
						IncrementX();
						plotDir = PlotDirection_PosX_NegY;
					}
					else
						plotDir = PlotDirection_0X_NegY;
				}
				else
				{
					// Diagonal movement will move farther from the center (second octant)
					IncrementX();

					if (m_sqDistFromEdge < 0)
					{
						DecrementY();
						plotDir = PlotDirection_PosX_NegY;
					}
					else
						plotDir = PlotDirection_PosX_0Y;
				}

				if (m_xChangeCostDynamicFactor >= 0)
					m_quadrant = Quadrant_PxNy;

				return plotDir;
			}
			break;

		case Quadrant_PxNy:
			{
				const int32_t xStepCost = m_xChangeCostDynamicFactor + m_xChangeCostStaticFactor;
				const int32_t yStepCost = m_yChangeCostDynamicFactor + m_yChangeCostStaticFactor;

				const int32_t diagonalCostDelta = xStepCost + yStepCost;

				if (diagonalCostDelta < 0)
				{
					// Diagonal movement will move closer to the edge (first octant)
					IncrementX();

					const int32_t distWithDiagonalMovement = m_sqDistFromEdge + yStepCost;
					if (distWithDiagonalMovement >= 0)
					{
						IncrementY();
						plotDir = PlotDirection_PosX_PosY;
					}
					else
						plotDir = PlotDirection_PosX_0Y;
				}
				else
				{
					// Diagonal movement will move farther from the center
					IncrementY();

					if (m_sqDistFromEdge < 0)
					{
						IncrementX();
						plotDir = PlotDirection_PosX_PosY;
					}
					else
						plotDir = PlotDirection_0X_PosY;
				}

				if (m_yChangeCostDynamicFactor >= 0)
					m_quadrant = Quadrant_Finished;

				return plotDir;
			}
			break;

		case Quadrant_Finished:
			return PlotDirection_Exhausted;

		default:
			assert(false);
			return PlotDirection_Exhausted;
		}
	}

	void EllipsePlotter::IncrementX()
	{
		const int32_t xStepCost = m_xChangeCostDynamicFactor + m_xChangeCostStaticFactor;

		m_sqDistFromEdge += xStepCost;
		m_xChangeCostDynamicFactor += m_xChangeCostDynamicFactorStep;
		m_point.m_x++;

#if PL_DEBUG_ELLIPSE_PLOTTER
		m_2offsetFromCenter.m_x += 2;
#endif
	}

	bool EllipsePlotter::ConditionalIncrementX()
	{
		const int32_t xStepCost = m_xChangeCostDynamicFactor + m_xChangeCostStaticFactor;

		if (m_sqDistFromEdge + xStepCost >= 0)
		{
			m_sqDistFromEdge += xStepCost;
			m_xChangeCostDynamicFactor += m_xChangeCostDynamicFactorStep;
			m_point.m_x++;

#if PL_DEBUG_ELLIPSE_PLOTTER
			m_2offsetFromCenter.m_x += 2;
#endif
			return true;
		}

		return false;
	}

	void EllipsePlotter::IncrementY()
	{
		const int32_t yStepCost = m_yChangeCostDynamicFactor + m_yChangeCostStaticFactor;

		m_sqDistFromEdge += yStepCost;
		m_yChangeCostDynamicFactor += m_yChangeCostDynamicFactorStep;
		m_point.m_y++;

#if PL_DEBUG_ELLIPSE_PLOTTER
		m_2offsetFromCenter.m_y += 2;
#endif
	}

	bool EllipsePlotter::ConditionalIncrementY()
	{
		const int32_t yStepCost = m_yChangeCostDynamicFactor + m_yChangeCostStaticFactor;

		if (m_sqDistFromEdge + yStepCost >= 0)
		{
			m_sqDistFromEdge += yStepCost;
			m_yChangeCostDynamicFactor += m_yChangeCostDynamicFactorStep;
			m_point.m_y++;

#if PL_DEBUG_ELLIPSE_PLOTTER
			m_2offsetFromCenter.m_y += 2;
#endif
			return true;
		}

		return false;
	}

	void EllipsePlotter::DecrementX()
	{
		const int32_t xStepCost = -m_xChangeCostDynamicFactor + m_xChangeCostStaticFactor;

		m_sqDistFromEdge += xStepCost;
		m_xChangeCostDynamicFactor -= m_xChangeCostDynamicFactorStep;
		m_point.m_x--;

#if PL_DEBUG_ELLIPSE_PLOTTER
		m_2offsetFromCenter.m_x -= 2;
#endif
	}

	bool EllipsePlotter::ConditionalDecrementX()
	{
		const int32_t xStepCost = -m_xChangeCostDynamicFactor + m_xChangeCostStaticFactor;

		if (m_sqDistFromEdge + xStepCost >= 0)
		{
			m_sqDistFromEdge += xStepCost;
			m_xChangeCostDynamicFactor -= m_xChangeCostDynamicFactorStep;
			m_point.m_x--;

#if PL_DEBUG_ELLIPSE_PLOTTER
			m_2offsetFromCenter.m_x -= 2;
#endif
			return true;
		}

		return false;
	}

	void EllipsePlotter::DecrementY()
	{
		const int32_t yStepCost = -m_yChangeCostDynamicFactor + m_yChangeCostStaticFactor;

		m_sqDistFromEdge += yStepCost;
		m_yChangeCostDynamicFactor -= m_yChangeCostDynamicFactorStep;
		m_point.m_y--;

#if PL_DEBUG_ELLIPSE_PLOTTER
		m_2offsetFromCenter.m_y -= 2;
#endif
	}

	bool EllipsePlotter::ConditionalDecrementY()
	{
		const int32_t yStepCost = -m_yChangeCostDynamicFactor + m_yChangeCostStaticFactor;

		if (m_sqDistFromEdge + yStepCost >= 0)
		{
			m_sqDistFromEdge += yStepCost;
			m_yChangeCostDynamicFactor -= m_yChangeCostDynamicFactorStep;
			m_point.m_y--;

#if PL_DEBUG_ELLIPSE_PLOTTER
			m_2offsetFromCenter.m_y -= 2;
#endif
			return true;
		}

		return false;
	}

	const Vec2i &EllipsePlotter::GetPoint() const
	{
		return m_point;
	}

	void EllipsePlotter::Reset(const Rect2i &bounds)
	{
		assert(bounds.IsValid());
		m_quadrant = Quadrant_PxPy;

		m_2center = bounds.m_topLeft + bounds.m_bottomRight - Vec2i(1, 1);
		m_diameters = bounds.m_bottomRight - bounds.m_topLeft - Vec2i(1, 1);

		m_point.m_x = bounds.m_bottomRight.m_x - 1;
		m_point.m_y = (m_2center.m_y + 1) / 2;

		const Vec2i offsetFromCenterTimes2 = (m_point + m_point) - m_2center;

#if PL_DEBUG_ELLIPSE_PLOTTER
		m_2offsetFromCenter = offsetFromCenterTimes2;
#endif

		m_sqDistFromEdge = SquareInt32(offsetFromCenterTimes2.m_x * m_diameters.m_y) + SquareInt32(offsetFromCenterTimes2.m_y * m_diameters.m_x) - SquareInt32(m_diameters.m_x * m_diameters.m_y);

		const int32_t xCostMultiplier = 4 * SquareInt32(m_diameters.m_y);
		const int32_t yCostMultiplier = 4 * SquareInt32(m_diameters.m_x);

		m_xChangeCostDynamicFactorStep = 2 * xCostMultiplier;
		m_yChangeCostDynamicFactorStep = 2 * yCostMultiplier;
		m_xChangeCostStaticFactor = 4 * SquareInt32(m_diameters.m_y);
		m_yChangeCostStaticFactor = 4 * SquareInt32(m_diameters.m_x);

		m_xChangeCostDynamicFactor = offsetFromCenterTimes2.m_x * xCostMultiplier;
		m_yChangeCostDynamicFactor = offsetFromCenterTimes2.m_y * yCostMultiplier;
	}
}
