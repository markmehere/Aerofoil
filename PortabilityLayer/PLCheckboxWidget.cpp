#include "PLCheckboxWidget.h"
#include "PLStandardColors.h"
#include "FontFamily.h"
#include "PLTimeTaggedVOSEvent.h"

#include <algorithm>

namespace PortabilityLayer
{
	CheckboxWidget::CheckboxWidget(const WidgetBasicState &state)
		: WidgetSpec<CheckboxWidget>(state)
		, m_text(state.m_text)
		, m_haveMouseDown(false)
	{
	}

	CheckboxWidget::~CheckboxWidget()
	{
	}

	bool CheckboxWidget::Init(const WidgetBasicState &state)
	{
		(void)state;

		return true;
	}

	void CheckboxWidget::DrawControl(DrawSurface *surface)
	{
		if (!m_rect.IsValid())
			return;

		surface->SetForeColor(StdColors::White());
		surface->FillRect(m_rect);

		uint16_t checkFrameSize = std::min<uint16_t>(12, std::min(m_rect.Width(), m_rect.Height()));
		int16_t top = (m_rect.top + m_rect.bottom - static_cast<int16_t>(checkFrameSize)) / 2;

		surface->SetForeColor(StdColors::Black());
		const Rect checkRect = Rect::Create(top, m_rect.left, top + static_cast<int16_t>(checkFrameSize), m_rect.left + static_cast<int16_t>(checkFrameSize));
		surface->FillRect(checkRect);

		surface->SetForeColor(StdColors::White());
		surface->FillRect(checkRect.Inset(1, 1));

		if (m_state)
		{
			surface->SetForeColor(StdColors::Black());
			surface->DrawLine(Point::Create(checkRect.left + 1, checkRect.top + 1), Point::Create(checkRect.right - 2, checkRect.bottom - 2));
			surface->DrawLine(Point::Create(checkRect.right - 2, checkRect.top + 1), Point::Create(checkRect.left + 1, checkRect.bottom - 2));
		}

		surface->SetForeColor(StdColors::Black());
		surface->SetSystemFont(12, FontFamilyFlag_Bold);
		int32_t textV = (m_rect.top + m_rect.bottom + surface->MeasureFontAscender()) / 2;
		surface->DrawString(Point::Create(m_rect.left + checkFrameSize + 2, textV), m_text.ToShortStr(), true);
	}

	void CheckboxWidget::SetString(const PLPasStr &str)
	{
		m_text = PascalStr<255>(str);
	}

	PLPasStr CheckboxWidget::GetString() const
	{
		return m_text.ToShortStr();
	}

	void CheckboxWidget::OnStateChanged()
	{
		if (m_window)
			DrawControl(&m_window->m_surface);
	}

	WidgetHandleState_t CheckboxWidget::ProcessEvent(const TimeTaggedVOSEvent &evt)
	{
		if (m_haveMouseDown)
		{
			if (evt.IsLMouseUpEvent())
			{
				m_haveMouseDown = false;

				const Point pt = m_window->MouseToLocal(evt.m_vosEvent.m_event.m_mouseInputEvent);
				if (m_rect.Contains(pt))
					return WidgetHandleStates::kActivated;
				else
					return WidgetHandleStates::kIgnored;
			}

			return WidgetHandleStates::kCaptured;
		}
		else
		{
			if (evt.IsLMouseDownEvent())
			{
				const Point pt = m_window->MouseToLocal(evt.m_vosEvent.m_event.m_mouseInputEvent);

				if (m_rect.Contains(pt))
				{
					m_haveMouseDown = true;
					return WidgetHandleStates::kCaptured;
				}
				else
					return WidgetHandleStates::kIgnored;
			}
		}

		return WidgetHandleStates::kIgnored;
	}
}
