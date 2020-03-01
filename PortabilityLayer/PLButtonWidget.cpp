#include "PLButtonWidget.h"
#include "PLCore.h"
#include "PLTimeTaggedVOSEvent.h"
#include "PLStandardColors.h"
#include "FontFamily.h"

namespace PortabilityLayer
{
	ButtonWidget::ButtonWidget(const WidgetBasicState &state)
		: WidgetSpec<ButtonWidget>(state)
		, m_haveMouseDown(false)
		, m_text(state.m_text)
	{
	}

	WidgetHandleState_t ButtonWidget::ProcessEvent(const TimeTaggedVOSEvent &evt)
	{
		if (!m_visible || !m_enabled)
			return WidgetHandleStates::kIgnored;

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

	void ButtonWidget::OnEnabledChanged()
	{
		DrawControl(m_window->GetDrawSurface());
	}

	int16_t ButtonWidget::Capture(const Point &pos, WidgetUpdateCallback_t callback)
	{
		for (;;)
		{
			TimeTaggedVOSEvent evt;
			if (WaitForEvent(&evt, 1))
			{
				if (evt.IsLMouseUpEvent())
				{
					const Point pt = m_window->MouseToLocal(evt.m_vosEvent.m_event.m_mouseInputEvent);

					if (m_rect.Contains(pt))
						return RegionIDs::kContent;
					else
						return RegionIDs::kNone;
				}
			}
		}
	}

	bool ButtonWidget::Init(const WidgetBasicState &state)
	{
		(void)state;
		return true;
	}

	void ButtonWidget::DrawControl(DrawSurface *surface)
	{
		surface->SetForeColor(StdColors::White());
		surface->FillRect(this->m_rect.Inset(1, 1));

		if (m_enabled)
			surface->SetForeColor(StdColors::Black());
		else
			surface->SetForeColor(RGBAColor::Create(136, 136, 136, 255));

		surface->FrameRect(this->m_rect);
		surface->SetSystemFont(12, PortabilityLayer::FontFamilyFlag_Bold);
		int32_t x = (m_rect.left + m_rect.right - static_cast<int32_t>(surface->MeasureString(m_text.ToShortStr()))) / 2;
		int32_t y = (m_rect.top + m_rect.bottom + static_cast<int32_t>(surface->MeasureFontAscender())) / 2;
		surface->DrawString(Point::Create(x, y), m_text.ToShortStr(), true);
	}
}
