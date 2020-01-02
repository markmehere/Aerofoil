#include "PLButtonWidget.h"
#include "PLCore.h"
#include "PLTimeTaggedVOSEvent.h"

namespace PortabilityLayer
{
	ButtonWidget::ButtonWidget(const WidgetBasicState &state)
		: WidgetSpec<ButtonWidget>(state)
		, m_haveMouseDown(false)
	{
	}

	WidgetHandleState_t ButtonWidget::ProcessEvent(Window *window, const TimeTaggedVOSEvent &evt)
	{
		if (m_haveMouseDown)
		{
			if (evt.IsLMouseUpEvent())
			{
				m_haveMouseDown = false;

				const Point pt = window->MouseToLocal(evt.m_vosEvent.m_event.m_mouseInputEvent);
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
				const Point pt = window->MouseToLocal(evt.m_vosEvent.m_event.m_mouseInputEvent);

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

	bool ButtonWidget::Init(const WidgetBasicState &state)
	{
		(void)state;
		return true;
	}
}
