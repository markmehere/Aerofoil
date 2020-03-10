#include "PLScrollBarWidget.h"
#include "PLControlDefinitions.h"
#include "PLStandardColors.h"

namespace PortabilityLayer
{
	ScrollBarWidget::ScrollBarWidget(const WidgetBasicState &state)
		: WidgetSpec<ScrollBarWidget>(state)
		, m_min(0)
		, m_max(0)
		, m_gripSize(0)
		, m_gripPos(0)
		, m_laneCapacity(0)
	{
	}

	WidgetHandleState_t ScrollBarWidget::ProcessEvent(const TimeTaggedVOSEvent &evt)
	{
		if (!m_visible || !m_enabled)
			return WidgetHandleStates::kIgnored;

		return WidgetHandleStates::kIgnored;
	}

	void ScrollBarWidget::OnEnabledChanged()
	{
		DrawControl(m_window->GetDrawSurface());
	}

	bool ScrollBarWidget::Init(const WidgetBasicState &state)
	{
		m_min = state.m_min;
		m_max = state.m_max;

		RefreshGrip();

		return true;
	}

	void ScrollBarWidget::DrawControl(DrawSurface *surface)
	{
		surface->SetForeColor(StdColors::White());
		surface->FillRect(this->m_rect.Inset(1, 1));

		if (m_rect.Width() < 16 || m_rect.Height() < 16)
			return;

		if (IsHorizontal())
			DrawControlHorizontal(surface);
		else
			DrawControlVertical(surface);
	}

	void ScrollBarWidget::DrawControlHorizontal(DrawSurface *surface)
	{
		surface->SetForeColor(StdColors::Black());
		surface->FrameRect(m_rect);

		const Rect leftArrowRect = Rect::Create(m_rect.top, m_rect.left, m_rect.bottom, m_rect.left + 16);
		DrawBeveledBox(surface, leftArrowRect);

		surface->SetForeColor(StdColors::Black());
		for (int i = 0; i < 4; i++)
		{
			const Rect arrowSegRect = Rect::Create(7 - i + leftArrowRect.top, 6 + i + leftArrowRect.left, 9 + i + leftArrowRect.top, 7 + i + leftArrowRect.left);
			surface->FillRect(arrowSegRect);
		}

		const Rect rightArrowRect = Rect::Create(m_rect.top, m_rect.right - 16, m_rect.bottom, m_rect.right);
		DrawBeveledBox(surface, rightArrowRect);

		surface->SetForeColor(StdColors::Black());
		for (int i = 0; i < 4; i++)
		{
			const Rect arrowSegRect = Rect::Create(4 + i + rightArrowRect.top, 6 + i + rightArrowRect.left, 12 - i + rightArrowRect.top, 7 + i + rightArrowRect.left);
			surface->FillRect(arrowSegRect);
		}

		const Rect laneRect = Rect::Create(m_rect.top, leftArrowRect.right, m_rect.bottom, rightArrowRect.left);

		surface->SetForeColor(RGBAColor::Create(136, 136, 136, 255));
		surface->FillRect(Rect::Create(laneRect.top + 1, laneRect.left, laneRect.top + 2, laneRect.right));

		surface->SetForeColor(RGBAColor::Create(187, 187, 187, 255));
		surface->FillRect(Rect::Create(laneRect.top + 2, laneRect.left, laneRect.bottom - 2, laneRect.right));

		surface->SetForeColor(StdColors::White());
		surface->FillRect(Rect::Create(laneRect.bottom - 2, laneRect.left, laneRect.bottom - 1, laneRect.right));

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(Rect::Create(laneRect.top, laneRect.left, laneRect.top + 1, laneRect.right));
		surface->FillRect(Rect::Create(laneRect.bottom - 1, laneRect.left, laneRect.bottom, laneRect.right));


		if (m_laneCapacity > 0)
			DrawBeveledBox(surface, Rect::Create(laneRect.top, laneRect.left + m_gripPos, laneRect.bottom, laneRect.left + m_gripPos + m_gripSize));
	}

	void ScrollBarWidget::DrawControlVertical(DrawSurface *surface)
	{
		surface->SetForeColor(StdColors::Black());
		surface->FrameRect(m_rect);

		const Rect topArrowRect = Rect::Create(m_rect.top, m_rect.left, m_rect.top + 16, m_rect.right);
		DrawBeveledBox(surface, topArrowRect);

		surface->SetForeColor(StdColors::Black());
		for (int i = 0; i < 4; i++)
		{
			const Rect arrowSegRect = Rect::Create(6 + i + topArrowRect.top, 7 - i + topArrowRect.left, 7 + i + topArrowRect.top, 9 + i + topArrowRect.left);
			surface->FillRect(arrowSegRect);
		}

		const Rect bottomArrowRect = Rect::Create(m_rect.bottom - 16, m_rect.left, m_rect.bottom, m_rect.right);
		DrawBeveledBox(surface, bottomArrowRect);

		surface->SetForeColor(StdColors::Black());
		for (int i = 0; i < 4; i++)
		{
			const Rect arrowSegRect = Rect::Create(6 + i + bottomArrowRect.top, 4 + i + bottomArrowRect.left, 7 + i + bottomArrowRect.top, 12 - i + bottomArrowRect.left);
			surface->FillRect(arrowSegRect);
		}

		const Rect laneRect = Rect::Create(topArrowRect.bottom, m_rect.left, bottomArrowRect.top, m_rect.right);

		surface->SetForeColor(RGBAColor::Create(136, 136, 136, 255));
		surface->FillRect(Rect::Create(laneRect.top, laneRect.left + 1, laneRect.bottom, laneRect.left + 2));

		surface->SetForeColor(RGBAColor::Create(187, 187, 187, 255));
		surface->FillRect(Rect::Create(laneRect.top, laneRect.left + 2, laneRect.bottom, laneRect.right - 2));

		surface->SetForeColor(StdColors::White());
		surface->FillRect(Rect::Create(laneRect.bottom, laneRect.right - 2, laneRect.bottom, laneRect.right - 1));

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(Rect::Create(laneRect.top, laneRect.left, laneRect.bottom, laneRect.left + 1));
		surface->FillRect(Rect::Create(laneRect.top, laneRect.right - 1, laneRect.bottom, laneRect.right));

		if (m_laneCapacity > 0)
			DrawBeveledBox(surface, Rect::Create(laneRect.top + m_gripPos, laneRect.left, laneRect.top + m_gripPos + m_gripSize, laneRect.right));
	}

	void ScrollBarWidget::DrawBeveledBox(DrawSurface *surface, const Rect &rect)
	{
		surface->SetForeColor(StdColors::Black());
		surface->FrameRect(rect);

		surface->SetForeColor(RGBAColor::Create(187, 187, 187, 187));
		surface->FillRect(rect.Inset(1, 1));

		surface->SetForeColor(StdColors::White());
		surface->FillRect(Rect::Create(rect.top + 1, rect.left + 1, rect.top + 2, rect.right - 2));
		surface->FillRect(Rect::Create(rect.top + 2, rect.left + 1, rect.bottom - 2, rect.left + 2));

		surface->SetForeColor(RGBAColor::Create(136, 136, 136, 136));
		surface->FillRect(Rect::Create(rect.bottom - 2, rect.left + 2, rect.bottom - 1, rect.right - 1));
		surface->FillRect(Rect::Create(rect.top + 2, rect.right - 2, rect.bottom - 2, rect.right - 1));
	}

	bool ScrollBarWidget::IsHorizontal() const
	{
		return m_rect.Width() > m_rect.Height();
	}

	bool ScrollBarWidget::Isvertical() const
	{
		return !this->IsHorizontal();
	}

	void ScrollBarWidget::RefreshGrip()
	{
		m_gripSize = 16;
		m_laneCapacity = 0;
		m_gripPos = 0;

		if (m_gripSize > 0 && m_min < m_max)
		{
			if (IsHorizontal())
				m_laneCapacity = static_cast<int32_t>(m_rect.Width() - 32) - m_gripSize;
			else
				m_laneCapacity = static_cast<int32_t>(m_rect.Height() - 32) - m_gripSize;

			if (m_laneCapacity < 0)
				m_laneCapacity = 0;

			if (m_laneCapacity > 0)
			{
				m_gripPos = 0;
				if (m_state >= m_max)
					m_gripPos = m_laneCapacity;
				else if (m_state > m_min)
					m_gripPos = (m_state * m_laneCapacity) / (m_max - m_min);
			}
			else
				m_gripPos = 0;
		}
	}

	void ScrollBarWidget::OnStateChanged()
	{
		RefreshGrip();
		DrawControl(m_window->GetDrawSurface());
	}

	int ScrollBarWidget::ResolvePart(const Point &point) const
	{
		if (!m_rect.Contains(point))
			return 0;

		int32_t span = 0;
		int32_t coord = 0;
		const bool isHorizontal = IsHorizontal();
		if (isHorizontal)
		{
			span = m_rect.Width();
			coord = point.h - m_rect.left;
		}
		else
		{
			span = m_rect.Height();
			coord = point.v - m_rect.top;
		}

		if (coord < 16)
			return isHorizontal ? kControlDownButtonPart : kControlUpButtonPart;

		if (coord < m_gripPos)
			return isHorizontal ? kControlPageDownPart : kControlPageUpPart;

		if (coord - m_gripPos < m_gripSize)
			return kControlIndicatorPart;

		if (coord < span - 16)
			return isHorizontal ? kControlPageUpPart : kControlPageDownPart;

		return isHorizontal ? kControlUpButtonPart : kControlDownButtonPart;
	}
}
