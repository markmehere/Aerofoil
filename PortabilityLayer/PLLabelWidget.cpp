#include "PLLabelWidget.h"
#include "PLQDraw.h"
#include "FontFamily.h"
#include "PLStandardColors.h"

#include <algorithm>

namespace PortabilityLayer
{
	LabelWidget::LabelWidget(const WidgetBasicState &state)
		: WidgetSpec<LabelWidget>(state)
		, m_text(state.m_text)
	{
	}

	bool LabelWidget::Init(const WidgetBasicState &state)
	{
		(void)state;
		return true;
	}

	void LabelWidget::SetString(const PLPasStr &str)
	{
		m_text.Set(str.Length(), str.Chars());
	}

	PLPasStr LabelWidget::GetString() const
	{
		return m_text.ToShortStr();
	}

	void LabelWidget::DrawControl(DrawSurface *surface)
	{
		// FIXME: This is kind of bad
		surface->SetForeColor(StdColors::White());
		surface->FillRect(m_rect);

		surface->SetSystemFont(12, PortabilityLayer::FontFamilyFlag_Bold);
		surface->SetForeColor(StdColors::Black());

		const Point topLeftCorner = Point::Create(m_rect.left, m_rect.top);
		const Point textStartPoint = topLeftCorner + Point::Create(0, surface->MeasureFontAscender());

		surface->DrawStringWrap(textStartPoint, m_rect, m_text.ToShortStr(), true);
	}
}
