#include "PLPopupMenuWidget.h"

#include "MenuManager.h"
#include "PLMenus.h"
#include "PLPasStr.h"
#include "PLStandardColors.h"
#include "PLTimeTaggedVOSEvent.h"
#include "FontFamily.h"
#include "Vec2i.h"

static const int kLightGray = 238;
static const int kMidGray = 221;
static const int kMidDarkGray = 170;
static const int kDarkGray = 102;

namespace PortabilityLayer
{
	PopupMenuWidget::PopupMenuWidget(const WidgetBasicState &state)
		: WidgetSpec<PopupMenuWidget>(state)
	{
	}

	PopupMenuWidget::~PopupMenuWidget()
	{
		if (m_menu)
			m_menu.Dispose();
	}

	bool PopupMenuWidget::Init(const WidgetBasicState &state, const void *additionalData)
	{
		m_menu = ::GetMenu(state.m_resID);
		if (!m_menu)
			return false;

		return true;
	}

	WidgetHandleState_t PopupMenuWidget::ProcessEvent(const TimeTaggedVOSEvent &evt)
	{
		if (evt.IsLMouseDownEvent())
		{
			const GpMouseInputEvent &mouseEvent = evt.m_vosEvent.m_event.m_mouseInputEvent;
			const Vec2i globalPoint = Vec2i(mouseEvent.m_x, mouseEvent.m_y);
			const Vec2i localPoint = globalPoint - Vec2i(m_window->m_wmX, m_window->m_wmY);

			if (this->m_rect.Contains(Point::Create(localPoint.m_x, localPoint.m_y)))
			{
				int16_t part = Capture(Point::Create(localPoint.m_x, localPoint.m_y), nullptr);
				if (part >= 1)
					return WidgetHandleStates::kActivated;
				else
					return WidgetHandleStates::kIgnored;
			}
		}

		return WidgetHandleStates::kIgnored;
	}

	int16_t PopupMenuWidget::Capture(const Point &pos, WidgetUpdateCallback_t callback)
	{
		MenuManager *mm = PortabilityLayer::MenuManager::GetInstance();

		const Vec2i popupMenuPos = Vec2i(m_window->m_wmX, m_window->m_wmY) + Vec2i(m_rect.left, m_rect.top);
		const Vec2i globalPos = Vec2i(pos.h, pos.v) + Vec2i(m_window->m_wmX, m_window->m_wmY);

		uint16_t item = 0;
		mm->PopupMenuSelect(m_menu, popupMenuPos, globalPos, m_state - 1, &item);

		if (item < 1)
			return -1;

		this->SetState(item);
		return item;
	}

	void PopupMenuWidget::DrawControl(DrawSurface *surface)
	{
		const Rect rect = m_rect;
		const Rect innerRect = rect.Inset(2, 2);

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(rect);

		surface->SetForeColor(StdColors::White());
		surface->FillRect(rect.Inset(1, 1));

		surface->SetForeColor(RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255));
		surface->FillRect(rect.Inset(2, 2));

		const Rect inset2Rect = rect.Inset(2, 2);

		surface->SetForeColor(RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255));
		surface->FillRect(Rect::Create(inset2Rect.bottom, inset2Rect.left, inset2Rect.bottom + 1, inset2Rect.right + 1));
		surface->FillRect(Rect::Create(inset2Rect.top, inset2Rect.right, inset2Rect.bottom + 1, inset2Rect.right + 1));


		Rect textRect = innerRect;
		textRect.right -= 11;

		surface->SetSystemFont(12, PortabilityLayer::FontFamilyFlag_Bold);
		Point basePoint = Point::Create(textRect.left + 2, (textRect.top + textRect.bottom + surface->MeasureFontAscender() + 1) / 2 - 1);

		surface->SetForeColor(StdColors::Black());
		surface->DrawStringConstrained(basePoint, GetString(), true, textRect);

		Point arrowMidPoint = Point::Create(textRect.right + 5, (textRect.top + textRect.bottom + 1) / 2);

		const Rect arrowRects[4] =
		{
			Rect::Create(arrowMidPoint.v - 2, arrowMidPoint.h - 3, arrowMidPoint.v - 1, arrowMidPoint.h + 4),
			Rect::Create(arrowMidPoint.v - 1, arrowMidPoint.h - 2, arrowMidPoint.v, arrowMidPoint.h + 3),
			Rect::Create(arrowMidPoint.v, arrowMidPoint.h - 1, arrowMidPoint.v + 1, arrowMidPoint.h + 2),
			Rect::Create(arrowMidPoint.v + 1, arrowMidPoint.h, arrowMidPoint.v + 2, arrowMidPoint.h + 1)
		};

		for (int i = 0; i < 4; i++)
		{
			const Rect constrainedRect = innerRect.Intersect(arrowRects[i]);
			surface->FillRect(constrainedRect);
		}
	}

	void PopupMenuWidget::OnStateChanged()
	{
		DrawControl(m_window->GetDrawSurface());
	}

	PLPasStr PopupMenuWidget::GetString() const
	{
		if (m_state < 1)
			return PSTR("");

		const Menu *menu = (*m_menu);
		return PortabilityLayer::MenuManager::GetInstance()->GetItemText(m_menu, m_state - 1);
	}

	const THandle<Menu> &PopupMenuWidget::GetMenu() const
	{
		return m_menu;
	}
}
