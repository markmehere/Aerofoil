#include "PLButtonWidget.h"
#include "PLCore.h"
#include "PLTimeTaggedVOSEvent.h"
#include "PLStandardColors.h"
#include "FontFamily.h"
#include "SimpleGraphic.h"

static const int kLightGray = 238;
static const int kMidGray = 221;
static const int kDarkGray = 102;

static const PortabilityLayer::RGBAColor gs_buttonTopLeftCornerGraphicPixels[] =
{
	{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 },
	{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { kMidGray, kMidGray, kMidGray, 255 },
	{ 0, 0, 0, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { 255, 255, 255, 255 }
};

static const PortabilityLayer::RGBAColor gs_buttonTopRightCornerGraphicPixels[] =
{
	{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 },
	{ kMidGray, kMidGray, kMidGray, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 },
	{ 255, 255, 255, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { 0, 0, 0, 255 }
};

static const PortabilityLayer::RGBAColor gs_buttonBottomLeftCornerGraphicPixels[] =
{
	{ 0, 0, 0, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { 255, 255, 255, 255 },
	{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { kMidGray, kMidGray, kMidGray, 255 },
	{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 }
};

static const PortabilityLayer::RGBAColor gs_buttonBottomRightCornerGraphicPixels[] =
{
	{ kMidGray, kMidGray, kMidGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { 0, 0, 0, 255 },
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 },
	{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 }
};

// Pressed

static const PortabilityLayer::RGBAColor gs_buttonPressedTopLeftCornerGraphicPixels[] =
{
	{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 },
	{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 },
	{ 0, 0, 0, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }
};

static const PortabilityLayer::RGBAColor gs_buttonPressedTopRightCornerGraphicPixels[] =
{
	{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 },
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 },
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { 0, 0, 0, 255 }
};

static const PortabilityLayer::RGBAColor gs_buttonPressedBottomLeftCornerGraphicPixels[] =
{
	{ 0, 0, 0, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 },
	{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 },
	{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 }
};

static const PortabilityLayer::RGBAColor gs_buttonPressedBottomRightCornerGraphicPixels[] =
{
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { 0, 0, 0, 255 },
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 },
	{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 }
};

// Disabled

static const PortabilityLayer::RGBAColor gs_buttonDisabledTopLeftCornerGraphicPixels[] =
{
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 },
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { kLightGray, kLightGray, kLightGray, 255 },
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { kLightGray, kLightGray, kLightGray, 255 }, { kLightGray, kLightGray, kLightGray, 255 }
};

static const PortabilityLayer::RGBAColor gs_buttonDisabledTopRightCornerGraphicPixels[] =
{
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 },
	{ kLightGray, kLightGray, kLightGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 },
	{ kLightGray, kLightGray, kLightGray, 255 }, { kLightGray, kLightGray, kLightGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }
};

static const PortabilityLayer::RGBAColor gs_buttonDisabledBottomLeftCornerGraphicPixels[] =
{
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { kLightGray, kLightGray, kLightGray, 255 }, { kLightGray, kLightGray, kLightGray, 255 },
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { kLightGray, kLightGray, kLightGray, 255 },
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }
};

static const PortabilityLayer::RGBAColor gs_buttonDisabledBottomRightCornerGraphicPixels[] =
{
	{ kLightGray, kLightGray, kLightGray, 255 }, { kLightGray, kLightGray, kLightGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 },
	{ kLightGray, kLightGray, kLightGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 },
	{ kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }, { kDarkGray, kDarkGray, kDarkGray, 255 }
};



static PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3> gs_buttonCornerGraphics[4] =
{
	PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3>(gs_buttonTopLeftCornerGraphicPixels),
	PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3>(gs_buttonTopRightCornerGraphicPixels),
	PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3>(gs_buttonBottomLeftCornerGraphicPixels),
	PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3>(gs_buttonBottomRightCornerGraphicPixels),
};

static PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3> gs_buttonPressedCornerGraphics[4] =
{
	PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3>(gs_buttonPressedTopLeftCornerGraphicPixels),
	PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3>(gs_buttonPressedTopRightCornerGraphicPixels),
	PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3>(gs_buttonPressedBottomLeftCornerGraphicPixels),
	PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3>(gs_buttonPressedBottomRightCornerGraphicPixels),
};

static PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3> gs_buttonDisabledCornerGraphics[4] =
{
	PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3>(gs_buttonDisabledTopLeftCornerGraphicPixels),
	PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3>(gs_buttonDisabledTopRightCornerGraphicPixels),
	PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3>(gs_buttonDisabledBottomLeftCornerGraphicPixels),
	PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3>(gs_buttonDisabledBottomRightCornerGraphicPixels),
};

static const uint8_t gs_buttonTopLeftGraphicMask[] = { 0x2f, 0xff };
static const uint8_t gs_buttonTopRightGraphicMask[] = { 0x9b, 0xff };
static const uint8_t gs_buttonBottomLeftGraphicMask[] = { 0xec, 0xff };
static const uint8_t gs_buttonBottomRightGraphicMask[] = { 0xfa, 0x7f };


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
		if (!m_enabled || !m_visible)
			return 0;

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
		const Rect rect = m_rect;

		PixMap **pixMap = surface->m_port.GetPixMap();

		const PortabilityLayer::SimpleGraphicInstanceRGBA<3, 3> *cornerGraphics = nullptr;

		PortabilityLayer::RGBAColor leftStripeColors[2];
		PortabilityLayer::RGBAColor rightStripeColors[2];
		PortabilityLayer::RGBAColor topStripeColors[2];
		PortabilityLayer::RGBAColor bottomStripeColors[2];
		PortabilityLayer::RGBAColor centerColor;
		PortabilityLayer::RGBAColor textColor;
		PortabilityLayer::RGBAColor borderColor;

		if (!m_enabled)
		{
			cornerGraphics = gs_buttonDisabledCornerGraphics;
			topStripeColors[0] = PortabilityLayer::RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255);
			topStripeColors[1] = PortabilityLayer::RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255);
			leftStripeColors[0] = PortabilityLayer::RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255);
			leftStripeColors[1] = PortabilityLayer::RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255);
			bottomStripeColors[0] = PortabilityLayer::RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255);
			bottomStripeColors[1] = PortabilityLayer::RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255);
			rightStripeColors[0] = PortabilityLayer::RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255);
			rightStripeColors[1] = PortabilityLayer::RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255);
			centerColor = PortabilityLayer::RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255);
			textColor = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			borderColor = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
		}
		else if (inverted)
		{
			cornerGraphics = gs_buttonPressedCornerGraphics;
			topStripeColors[0] = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			topStripeColors[1] = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			leftStripeColors[0] = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			leftStripeColors[1] = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			bottomStripeColors[0] = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			bottomStripeColors[1] = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			rightStripeColors[0] = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			rightStripeColors[1] = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			centerColor = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			textColor = StdColors::White();
			borderColor = StdColors::Black();
		}
		else
		{
			cornerGraphics = gs_buttonCornerGraphics;
			topStripeColors[0] = PortabilityLayer::RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255);
			topStripeColors[1] = StdColors::White();
			leftStripeColors[0] = PortabilityLayer::RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255);
			leftStripeColors[1] = StdColors::White();
			bottomStripeColors[0] = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			bottomStripeColors[1] = PortabilityLayer::RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255);
			rightStripeColors[0] = PortabilityLayer::RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			rightStripeColors[1] = PortabilityLayer::RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255);
			centerColor = PortabilityLayer::RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255);
			textColor = StdColors::Black();
			borderColor = StdColors::Black();
		}


		cornerGraphics[0].DrawToPixMapWithMask(pixMap, gs_buttonTopLeftGraphicMask, rect.left, rect.top);
		cornerGraphics[1].DrawToPixMapWithMask(pixMap, gs_buttonTopRightGraphicMask, rect.right - 3, rect.top);
		cornerGraphics[2].DrawToPixMapWithMask(pixMap, gs_buttonBottomLeftGraphicMask, rect.left, rect.bottom - 3);
		cornerGraphics[3].DrawToPixMapWithMask(pixMap, gs_buttonBottomRightGraphicMask, rect.right - 3, rect.bottom - 3);

		for (int i = 0; i < 3; i++)
		{
			if (i == 0)
				surface->SetForeColor(borderColor);

			if (i != 0)
				surface->SetForeColor(leftStripeColors[i - 1]);

			surface->FillRect(Rect::Create(rect.top + 3, rect.left + i, rect.bottom - 3, rect.left + i + 1));

			if (i != 0)
				surface->SetForeColor(rightStripeColors[i - 1]);

			surface->FillRect(Rect::Create(rect.top + 3, rect.right - 1 - i, rect.bottom - 3, rect.right - i));

			if (i != 0)
				surface->SetForeColor(topStripeColors[i - 1]);

			surface->FillRect(Rect::Create(rect.top + i, rect.left + 3, rect.top + i + 1, rect.right - 3));

			if (i != 0)
				surface->SetForeColor(bottomStripeColors[i - 1]);

			surface->FillRect(Rect::Create(rect.bottom - 1 - i, rect.left + 3, rect.bottom - i, rect.right - 3));
		}

		surface->SetForeColor(centerColor);
		surface->FillRect(rect.Inset(3, 3));


		/*
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
			*/

		surface->SetForeColor(textColor);

		surface->SetSystemFont(12, PortabilityLayer::FontFamilyFlag_Bold);
		int32_t x = (m_rect.left + m_rect.right - static_cast<int32_t>(surface->MeasureString(m_text.ToShortStr()))) / 2;
		int32_t y = (m_rect.top + m_rect.bottom + static_cast<int32_t>(surface->MeasureFontAscender())) / 2;
		surface->DrawString(Point::Create(x, y), m_text.ToShortStr(), true);
	}
}
