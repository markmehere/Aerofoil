#include "PLPopupMenuWidget.h"

#include "MenuManager.h"
#include "PLMenus.h"
#include "PLPasStr.h"
#include "PLStandardColors.h"
#include "FontFamily.h"

namespace PortabilityLayer
{
	PopupMenuWidget::PopupMenuWidget(const WidgetBasicState &state)
		: WidgetSpec<PopupMenuWidget>(state)
	{
	}

	bool PopupMenuWidget::Init(const WidgetBasicState &state)
	{
		m_menu = GetMenu(state.m_resID);
		if (!m_menu)
			return false;

		return true;
	}

	//WidgetHandleState_t PopupMenuWidget::ProcessEvent(const TimeTaggedVOSEvent &evt);
	//int16_t PopupMenuWidget::Capture(const Point &pos, WidgetUpdateCallback_t callback);
	void PopupMenuWidget::DrawControl(DrawSurface *surface)
	{
		const Rect rect = m_rect;
		const Rect innerRect = rect.Inset(2, 2);

		surface->SetForeColor(StdColors::White());
		surface->FillRect(m_rect.Inset(1, 1));

		surface->SetForeColor(StdColors::Black());
		surface->FrameRect(m_rect);

		Rect textRect = innerRect;
		textRect.right -= 9;

		surface->SetSystemFont(12, PortabilityLayer::FontFamilyFlag_Bold);
		Point basePoint = Point::Create(textRect.left + 2, (textRect.top + textRect.bottom + surface->MeasureFontAscender() + 1) / 2);

		surface->DrawStringConstrained(basePoint, GetString(), true, textRect);

		Point arrowMidPoint = Point::Create(textRect.right + 5, (textRect.top + textRect.bottom + 1) / 2);

		const Rect arrowRects[3] =
		{
			Rect::Create(arrowMidPoint.v - 1, arrowMidPoint.h - 2, arrowMidPoint.v, arrowMidPoint.h + 3),
			Rect::Create(arrowMidPoint.v, arrowMidPoint.h - 1, arrowMidPoint.v + 1, arrowMidPoint.h + 2),
			Rect::Create(arrowMidPoint.v + 1, arrowMidPoint.h, arrowMidPoint.v + 2, arrowMidPoint.h + 1)
		};

		for (int i = 0; i < 3; i++)
		{
			const Rect constrainedRect = innerRect.Intersect(arrowRects[i]);
			surface->FillRect(constrainedRect);
		}
	}

	PLPasStr PopupMenuWidget::GetString() const
	{
		const Menu *menu = (*m_menu);
		return PortabilityLayer::MenuManager::GetInstance()->GetItemText(m_menu, m_state - 1);
	}
}
