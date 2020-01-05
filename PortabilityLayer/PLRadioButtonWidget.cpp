#include "PLRadioButtonWidget.h"
#include "PLStandardColors.h"
#include "FontFamily.h"

#include <algorithm>

namespace PortabilityLayer
{
	RadioButtonWidget::RadioButtonWidget(const WidgetBasicState &state)
		: WidgetSpec<RadioButtonWidget>(state)
		, m_text(state.m_text)
	{
	}

	RadioButtonWidget::~RadioButtonWidget()
	{
	}

	bool RadioButtonWidget::Init(const WidgetBasicState &state)
	{
		(void)state;

		return true;
	}

	void RadioButtonWidget::DrawControl(DrawSurface *surface)
	{
		if (!m_rect.IsValid())
			return;

		uint16_t radioFrameSize = std::min<uint16_t>(12, std::min(m_rect.Width(), m_rect.Height()));
		int16_t top = (m_rect.top + m_rect.bottom - static_cast<int16_t>(radioFrameSize)) / 2;

		surface->SetForeColor(StdColors::Black());
		const Rect radioRect = Rect::Create(top, m_rect.left, top + static_cast<int16_t>(radioFrameSize), m_rect.left + static_cast<int16_t>(radioFrameSize));
		surface->FillEllipse(radioRect);
		if (!m_state)
		{
			surface->SetForeColor(StdColors::White());
			surface->FillEllipse(radioRect.Inset(1, 1));
		}

		surface->SetForeColor(StdColors::Black());
		surface->SetSystemFont(12, FontFamilyFlag_Bold);
		int32_t textV = (m_rect.top + m_rect.bottom + surface->MeasureFontAscender()) / 2;
		surface->DrawString(Point::Create(m_rect.left + radioFrameSize + 2, textV), m_text.ToShortStr());
	}

	void RadioButtonWidget::SetString(const PLPasStr &str)
	{
		m_text = PascalStr<255>(str);
	}

}
