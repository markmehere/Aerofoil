#include "LinePlotter.h"
#include "Vec2i.h"

#include <assert.h>

namespace PortabilityLayer
{
	LinePlotter::LinePlotter()
		: m_point(0, 0)
		, m_endPoint(0, 0)
		, m_dx(0)
		, m_dy(0)
		, m_err(0)
		, m_xIncrPos(true)
		, m_yIncrPos(true)
		, m_xMove(PlotDirection_Exhausted)
		, m_yMove(PlotDirection_Exhausted)
		, m_xyMove(PlotDirection_Exhausted)
	{
	}

	PlotDirection LinePlotter::PlotNext()
	{
		if (m_point == m_endPoint)
			return PlotDirection_Exhausted;

		bool movedX = false;
		const int32_t err2 = 2 * m_err;
		if (err2 >= m_dy)
		{
			m_err += m_dy;
			m_point.m_x += m_xIncrPos ? 1 : -1;
			movedX = true;
		}

		if (err2 <= m_dx)
		{
			m_err += m_dx;
			m_point.m_y += m_yIncrPos ? 1 : -1;

			if (movedX)
				return m_xyMove;
			else
				return m_yMove;
		}
		else
		{
			assert(movedX);
			return m_xMove;
		}
	}

	const Vec2i &LinePlotter::GetPoint() const
	{
		return m_point;
	}

	void LinePlotter::Reset(const Vec2i &pointA, const Vec2i &pointB)
	{
		m_dx = pointB.m_x - pointA.m_x;
		if (m_dx < 0)
			m_dx = -m_dx;

		m_dy = pointB.m_y - pointA.m_y;
		if (m_dy > 0)
			m_dy = -m_dy;

		m_xIncrPos = (pointA.m_x < pointB.m_x);
		m_yIncrPos = (pointA.m_y < pointB.m_y);

		m_err = m_dx + m_dy;

		m_point = pointA;
		m_endPoint = pointB;

		if (m_xIncrPos)
		{
			if (m_yIncrPos)
				m_xyMove = PlotDirection_PosX_PosY;
			else
				m_xyMove = PlotDirection_PosX_NegY;

			m_xMove = PlotDirection_PosX_0Y;
		}
		else
		{
			if (m_yIncrPos)
				m_xyMove = PlotDirection_NegX_PosY;
			else
				m_xyMove = PlotDirection_NegX_NegY;

			m_xMove = PlotDirection_NegX_0Y;
		}


		if (m_yIncrPos)
			m_yMove = PlotDirection_0X_PosY;
		else
			m_yMove = PlotDirection_0X_NegY;
	}
}
