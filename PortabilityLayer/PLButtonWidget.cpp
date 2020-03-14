#include "PLButtonWidget.h"
#include "PLCore.h"
#include "PLTimeTaggedVOSEvent.h"
#include "PLStandardColors.h"
#include "FontFamily.h"

namespace PortabilityLayer
{
	ButtonWidget::ButtonWidget(const WidgetBasicState &state)
		: WidgetSpec<ButtonWidget>(state)
		, m_text(state.m_text)
	{
	}

	WidgetHandleState_t ButtonWidget::ProcessEvent(const TimeTaggedVOSEvent &evt)
	{
		if (!m_visible || !m_enabled)
			return WidgetHandleStates::kIgnored;

		if (evt.IsLMouseDownEvent())
		{
			const Point pt = m_window->MouseToLocal(evt.m_vosEvent.m_event.m_mouseInputEvent);

			if (m_rect.Contains(pt))
			{
				if (Capture(pt, nullptr) == RegionIDs::kNone)
					return WidgetHandleStates::kDigested;
				else
					return WidgetHandleStates::kActivated;
			}
			else
				return WidgetHandleStates::kIgnored;
		}

		return WidgetHandleStates::kIgnored;
	}

	void ButtonWidget::OnEnabledChanged()
	{
		DrawControl(m_window->GetDrawSurface());
	}

	int16_t ButtonWidget::Capture(const Point &pos, WidgetUpdateCallback_t callback)
	{
		bool drawingDownState = false;
		bool isInBounds = m_rect.Contains(pos);
		for (;;)
		{
			if (drawingDownState != isInBounds)
			{
				DrawControlInternal(m_window->GetDrawSurface(), isInBounds);
				drawingDownState = isInBounds;
			}

			TimeTaggedVOSEvent evt;
			if (WaitForEvent(&evt, 1))
			{
				if (evt.m_vosEvent.m_eventType == GpVOSEventTypes::kMouseInput)
				{
					const Point pt = m_window->MouseToLocal(evt.m_vosEvent.m_event.m_mouseInputEvent);
					isInBounds = m_rect.Contains(pt);

					if (evt.IsLMouseUpEvent())
					{
						DrawControlInternal(m_window->GetDrawSurface(), false);

						if (isInBounds)
							return RegionIDs::kContent;
						else
							return RegionIDs::kNone;
					}
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
		DrawControlInternal(surface, false);
	}

	void ButtonWidget::DrawControlInternal(DrawSurface *surface, bool inverted)
	{
		surface->SetForeColor(inverted ? StdColors::Black() : StdColors::White());
		surface->FillRect(this->m_rect.Inset(1, 1));

		if (m_enabled)
			surface->SetForeColor(StdColors::Black());
		else
			surface->SetForeColor(RGBAColor::Create(136, 136, 136, 255));

		surface->FrameRect(this->m_rect);

		if (m_enabled)
			surface->SetForeColor(inverted ? StdColors::White() : StdColors::Black());
		else
			surface->SetForeColor(RGBAColor::Create(136, 136, 136, 255));

		surface->SetSystemFont(12, PortabilityLayer::FontFamilyFlag_Bold);
		int32_t x = (m_rect.left + m_rect.right - static_cast<int32_t>(surface->MeasureString(m_text.ToShortStr()))) / 2;
		int32_t y = (m_rect.top + m_rect.bottom + static_cast<int32_t>(surface->MeasureFontAscender())) / 2;
		surface->DrawString(Point::Create(x, y), m_text.ToShortStr(), true);
	}
}
