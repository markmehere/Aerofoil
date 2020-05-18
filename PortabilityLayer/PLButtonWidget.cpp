#include "PLButtonWidget.h"
#include "PLCore.h"
#include "PLControlDefinitions.h"
#include "PLRegions.h"
#include "PLTimeTaggedVOSEvent.h"
#include "PLStandardColors.h"
#include "FontFamily.h"
#include "SimpleGraphic.h"

#include <algorithm>

static const int kLightGray = 238;
static const int kMidGray = 221;
static const int kMidDarkGray = 170;
static const int kDarkGray = 102;

#ifdef CLR
#undef CLR
#endif

#define CLR(n) { n, n, n, 255}

static const PortabilityLayer::RGBAColor gs_buttonTopLeftCornerGraphicPixels[] =
{
	CLR(0), CLR(0), CLR(0),
	CLR(0), CLR(0), CLR(kMidGray),
	CLR(0), CLR(kMidGray), CLR(255)
};

static const PortabilityLayer::RGBAColor gs_buttonTopRightCornerGraphicPixels[] =
{
	CLR(0), CLR(0), CLR(0),
	CLR(kMidGray), CLR(0), CLR(0),
	CLR(255), CLR(kMidGray), CLR(0)
};

static const PortabilityLayer::RGBAColor gs_buttonBottomLeftCornerGraphicPixels[] =
{
	CLR(0), CLR(kMidGray), CLR(255),
	CLR(0), CLR(0), CLR(kMidGray),
	CLR(0), CLR(0), CLR(0)
};

static const PortabilityLayer::RGBAColor gs_buttonBottomRightCornerGraphicPixels[] =
{
	CLR(kMidGray), CLR(kDarkGray), CLR(0),
	CLR(kDarkGray), CLR(0), CLR(0),
	CLR(0), CLR(0), CLR(0)
};

// Pressed

static const PortabilityLayer::RGBAColor gs_buttonPressedTopLeftCornerGraphicPixels[] =
{
	CLR(0), CLR(0), CLR(0),
	CLR(0), CLR(0), CLR(kDarkGray),
	CLR(0), CLR(kDarkGray), CLR(kDarkGray)
};

static const PortabilityLayer::RGBAColor gs_buttonPressedTopRightCornerGraphicPixels[] =
{
	CLR(0), CLR(0), CLR(0),
	CLR(kDarkGray), CLR(0), CLR(0),
	CLR(kDarkGray), CLR(kDarkGray), CLR(0)
};

static const PortabilityLayer::RGBAColor gs_buttonPressedBottomLeftCornerGraphicPixels[] =
{
	CLR(0), CLR(kDarkGray), CLR(kDarkGray),
	CLR(0), CLR(0), CLR(kDarkGray),
	CLR(0), CLR(0), CLR(0)
};

static const PortabilityLayer::RGBAColor gs_buttonPressedBottomRightCornerGraphicPixels[] =
{
	CLR(kDarkGray), CLR(kDarkGray), CLR(0),
	CLR(kDarkGray), CLR(0), CLR(0),
	CLR(0), CLR(0), CLR(0)
};

// Disabled

static const PortabilityLayer::RGBAColor gs_buttonDisabledTopLeftCornerGraphicPixels[] =
{
	CLR(kDarkGray), CLR(kDarkGray), CLR(kDarkGray),
	CLR(kDarkGray), CLR(kDarkGray), CLR(kLightGray),
	CLR(kDarkGray), CLR(kLightGray), CLR(kLightGray)
};

static const PortabilityLayer::RGBAColor gs_buttonDisabledTopRightCornerGraphicPixels[] =
{
	CLR(kDarkGray), CLR(kDarkGray), CLR(kDarkGray),
	CLR(kLightGray), CLR(kDarkGray), CLR(kDarkGray),
	CLR(kLightGray), CLR(kLightGray), CLR(kDarkGray)
};

static const PortabilityLayer::RGBAColor gs_buttonDisabledBottomLeftCornerGraphicPixels[] =
{
	CLR(kDarkGray), CLR(kLightGray), CLR(kLightGray),
	CLR(kDarkGray), CLR(kDarkGray), CLR(kLightGray),
	CLR(kDarkGray), CLR(kDarkGray), CLR(kDarkGray)
};

static const PortabilityLayer::RGBAColor gs_buttonDisabledBottomRightCornerGraphicPixels[] =
{
	CLR(kLightGray), CLR(kLightGray), CLR(kDarkGray),
	CLR(kLightGray), CLR(kDarkGray), CLR(kDarkGray),
	CLR(kDarkGray), CLR(kDarkGray), CLR(kDarkGray)
};

// Default boundary

static const PortabilityLayer::RGBAColor gs_buttonDefaultTopLeftCornerGraphicPixels[] =
{
	CLR(0), CLR(0), CLR(0), CLR(0), CLR(0),
	CLR(0), CLR(0), CLR(0), CLR(0), CLR(255),
	CLR(0), CLR(0), CLR(0), CLR(255), CLR(kMidGray),
	CLR(0), CLR(0), CLR(255), CLR(kMidGray), CLR(kMidGray),
	CLR(0), CLR(255), CLR(kMidGray), CLR(kMidGray), CLR(0),
};

static const PortabilityLayer::RGBAColor gs_buttonDefaultTopRightCornerGraphicPixels[] =
{
	CLR(0), CLR(0), CLR(0), CLR(0), CLR(0),
	CLR(255), CLR(0), CLR(0), CLR(0), CLR(0),
	CLR(kMidGray), CLR(kMidGray), CLR(0), CLR(0), CLR(0),
	CLR(kMidGray), CLR(kMidGray), CLR(kMidGray), CLR(0), CLR(0),
	CLR(0), CLR(kMidGray), CLR(kMidGray), CLR(kDarkGray), CLR(0),
};

static const PortabilityLayer::RGBAColor gs_buttonDefaultBottomLeftCornerGraphicPixels[] =
{
	CLR(0), CLR(255), CLR(kMidGray), CLR(kMidGray), CLR(0),
	CLR(0), CLR(0), CLR(kMidGray), CLR(kMidGray), CLR(kMidGray),
	CLR(0), CLR(0), CLR(0), CLR(kMidGray), CLR(kMidGray),
	CLR(0), CLR(0), CLR(0), CLR(0), CLR(kDarkGray),
	CLR(0), CLR(0), CLR(0), CLR(0), CLR(0),
};

static const PortabilityLayer::RGBAColor gs_buttonDefaultBottomRightCornerGraphicPixels[] =
{
	CLR(0), CLR(kMidGray), CLR(kMidGray), CLR(kDarkGray), CLR(0),
	CLR(kMidGray), CLR(kMidGray), CLR(kDarkGray), CLR(0), CLR(0),
	CLR(kMidGray), CLR(kDarkGray), CLR(0), CLR(0), CLR(0),
	CLR(kDarkGray), CLR(0), CLR(0), CLR(0), CLR(0),
	CLR(0), CLR(0), CLR(0), CLR(0), CLR(0),
};

static const PortabilityLayer::RGBAColor gs_buttonRadioGraphicPixels[] =
{
	CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),
	CLR(0),CLR(0),CLR(0),CLR(255),CLR(255),CLR(255),CLR(255),CLR(255),CLR(255),CLR(0),CLR(0),CLR(0),
	CLR(0),CLR(0),CLR(255),CLR(255),CLR(kLightGray),CLR(kLightGray),CLR(kLightGray),CLR(kLightGray),CLR(kMidGray),CLR(kMidGray),CLR(0),CLR(0),
	CLR(0),CLR(255),CLR(255),CLR(kLightGray),CLR(kLightGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kDarkGray),CLR(0),
	CLR(0),CLR(255),CLR(kLightGray),CLR(kLightGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidDarkGray),CLR(kDarkGray),CLR(0),
	CLR(0),CLR(255),CLR(kLightGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidDarkGray),CLR(kDarkGray),CLR(0),
	CLR(0),CLR(255),CLR(kLightGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidDarkGray),CLR(kDarkGray),CLR(0),
	CLR(0),CLR(255),CLR(kLightGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidDarkGray),CLR(kMidDarkGray),CLR(kDarkGray),CLR(0),
	CLR(0),CLR(255),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidGray),CLR(kMidDarkGray),CLR(kMidDarkGray),CLR(kDarkGray),CLR(kDarkGray),CLR(0),
	CLR(0),CLR(0),CLR(kMidGray),CLR(kMidGray),CLR(kMidDarkGray),CLR(kMidDarkGray),CLR(kMidDarkGray),CLR(kMidDarkGray),CLR(kDarkGray),CLR(kDarkGray),CLR(0),CLR(0),
	CLR(0),CLR(0),CLR(0),CLR(kDarkGray),CLR(kDarkGray),CLR(kDarkGray),CLR(kDarkGray),CLR(kDarkGray),CLR(kDarkGray),CLR(0),CLR(0),CLR(0),
	CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),CLR(0),
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

static PortabilityLayer::SimpleGraphicInstanceRGBA<5, 5> gs_buttonDefaultCornerGraphics[4] =
{
	PortabilityLayer::SimpleGraphicInstanceRGBA<5, 5>(gs_buttonDefaultTopLeftCornerGraphicPixels),
	PortabilityLayer::SimpleGraphicInstanceRGBA<5, 5>(gs_buttonDefaultTopRightCornerGraphicPixels),
	PortabilityLayer::SimpleGraphicInstanceRGBA<5, 5>(gs_buttonDefaultBottomLeftCornerGraphicPixels),
	PortabilityLayer::SimpleGraphicInstanceRGBA<5, 5>(gs_buttonDefaultBottomRightCornerGraphicPixels),
};

static const uint8_t gs_buttonTopLeftGraphicMask[] = { 0x2f, 0xff };
static const uint8_t gs_buttonTopRightGraphicMask[] = { 0x9b, 0xff };
static const uint8_t gs_buttonBottomLeftGraphicMask[] = { 0xec, 0xff };
static const uint8_t gs_buttonBottomRightGraphicMask[] = { 0xfa, 0x7f };

static const uint8_t gs_buttonDefaultTopLeftGraphicMask[] = { 0x08, 0xCE, 0xFF, 0xFF };
static const uint8_t gs_buttonDefaultTopRightGraphicMask[] = { 0x86, 0x39, 0xEF, 0xFF };
static const uint8_t gs_buttonDefaultBottomLeftGraphicMask[] = { 0xFB, 0xCE, 0x30, 0xFF };
static const uint8_t gs_buttonDefaultBottomRightGraphicMask[] = { 0xFF, 0xB9, 0x88, 0x7F };

static const uint8_t gs_buttonRadioGraphicMask[] = { 0x1f, 0x83, 0xFC, 0x7F, 0xEF, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xF7, 0xFE, 0x3F, 0xC1, 0xF8 };
static PortabilityLayer::SimpleGraphicInstanceRGBA<12, 12> gs_buttonRadioGraphic(gs_buttonRadioGraphicPixels);


namespace PortabilityLayer
{

	ButtonWidget::AdditionalData::AdditionalData()
		: m_buttonStyle(kButtonStyle_Button)
	{
	}

	ButtonWidget::ButtonWidget(const WidgetBasicState &state)
		: WidgetSpec<ButtonWidget>(state)
		, m_text(state.m_text)
		, m_haveHighlightOverride(false)
		, m_buttonStyle(kButtonStyle_Button)
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
		if (m_window)
			DrawControl(m_window->GetDrawSurface());
	}

	void ButtonWidget::OnStateChanged()
	{
		if (m_window)
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

	bool ButtonWidget::Init(const WidgetBasicState &state, const void *additionalDataPtr)
	{
		const AdditionalData &additionalData = *static_cast<const AdditionalData *>(additionalDataPtr);

		m_buttonStyle = additionalData.m_buttonStyle;

		return true;
	}

	void ButtonWidget::DrawControl(DrawSurface *surface)
	{
		DrawControlInternal(surface, false);
	}


	void ButtonWidget::SetString(const PLPasStr &str)
	{
		m_text = PascalStr<255>(str);
	}

	PLPasStr ButtonWidget::GetString() const
	{
		return m_text.ToShortStr();
	}

	void ButtonWidget::DrawControlInternal(DrawSurface *surface, bool inverted)
	{
		switch (m_buttonStyle)
		{
		case kButtonStyle_Button:
			DrawAsButton(surface, inverted);
			break;
		case kButtonStyle_CheckBox:
			DrawAsCheck(surface, inverted);
			break;
		case kButtonStyle_Radio:
			DrawAsRadio(surface, inverted);
			break;
		default:
			break;
		}
	}

	void ButtonWidget::DrawAsButton(DrawSurface *surface, bool inverted)
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
		else if (inverted || m_haveHighlightOverride)
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

		surface->SetForeColor(textColor);

		surface->SetSystemFont(12, PortabilityLayer::FontFamilyFlag_Bold);
		int32_t x = (m_rect.left + m_rect.right - static_cast<int32_t>(surface->MeasureString(m_text.ToShortStr()))) / 2;
		int32_t y = (m_rect.top + m_rect.bottom + static_cast<int32_t>(surface->MeasureFontAscender())) / 2;
		surface->DrawString(Point::Create(x, y), m_text.ToShortStr(), true);
	}

	void ButtonWidget::DrawAsCheck(DrawSurface *surface, bool inverted)
	{
		if (!m_rect.IsValid())
			return;

		surface->SetForeColor(StdColors::White());
		surface->FillRect(m_rect);

		uint16_t checkFrameSize = std::min<uint16_t>(12, std::min(m_rect.Width(), m_rect.Height()));
		int16_t top = (m_rect.top + m_rect.bottom - static_cast<int16_t>(checkFrameSize)) / 2;

		const Rect checkRect = Rect::Create(top, m_rect.left, top + static_cast<int16_t>(checkFrameSize), m_rect.left + static_cast<int16_t>(checkFrameSize));

		RGBAColor checkColor;
		RGBAColor checkEraseColor;
		RGBAColor textColor;
		if (!m_enabled)
		{
			surface->SetForeColor(RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255));
			surface->FillRect(checkRect);
			surface->SetForeColor(RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255));
			surface->FillRect(checkRect.Inset(1, 1));

			checkColor = RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255);
			checkEraseColor = RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255);
			textColor = RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255);
		}
		else if (inverted)
		{
			surface->SetForeColor(StdColors::Black());
			surface->FillRect(checkRect);
			surface->SetForeColor(RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255));
			surface->FillRect(checkRect.Inset(1, 1));

			checkColor = StdColors::White();
			checkEraseColor = RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255);
			textColor = StdColors::Black();
		}
		else
		{
			surface->SetForeColor(StdColors::Black());
			surface->FillRect(checkRect);
			surface->SetForeColor(RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255));
			surface->FillRect(checkRect.Inset(1, 1));

			surface->SetForeColor(RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255));
			surface->FillRect(Rect::Create(checkRect.top + 2, checkRect.right - 2, checkRect.bottom - 2, checkRect.right - 1));
			surface->FillRect(Rect::Create(checkRect.bottom - 2, checkRect.left + 2, checkRect.bottom - 1, checkRect.right - 1));

			surface->SetForeColor(StdColors::White());
			surface->FillRect(Rect::Create(checkRect.top + 1, checkRect.left + 1, checkRect.top + 2, checkRect.right - 2));
			surface->FillRect(Rect::Create(checkRect.top + 2, checkRect.left + 1, checkRect.bottom - 2, checkRect.left + 2));

			checkColor = StdColors::Black();
			checkEraseColor = RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255);
			textColor = StdColors::Black();
		}

		if (m_state)
		{
			const Rect checkmarkRect = checkRect.Inset(3, 3);

			if (checkmarkRect.IsValid())
			{
				surface->SetForeColor(checkColor);
				surface->FillRect(checkmarkRect);

				if (checkmarkRect.Width() >= 5)
				{
					int32_t eraseSpan = checkmarkRect.Width() - 4;
					int16_t coordinateOffset = 0;

					surface->SetForeColor(checkEraseColor);

					while (eraseSpan > 0)
					{
						surface->FillRect(Rect::Create(checkmarkRect.top + coordinateOffset, checkmarkRect.left + 2 + coordinateOffset, checkmarkRect.top + 1 + coordinateOffset, checkmarkRect.right - 2 - coordinateOffset));
						surface->FillRect(Rect::Create(checkmarkRect.top + 2 + coordinateOffset, checkmarkRect.left + coordinateOffset, checkmarkRect.bottom - 2 - coordinateOffset, checkmarkRect.left + 1 + coordinateOffset));
						surface->FillRect(Rect::Create(checkmarkRect.bottom - 1 - coordinateOffset, checkmarkRect.left + 2 + coordinateOffset, checkmarkRect.bottom - coordinateOffset, checkmarkRect.right - 2 - coordinateOffset));
						surface->FillRect(Rect::Create(checkmarkRect.top + 2 + coordinateOffset, checkmarkRect.right - 1 - coordinateOffset, checkmarkRect.bottom - 2 - coordinateOffset, checkmarkRect.right - coordinateOffset));

						eraseSpan -= 2;
						coordinateOffset++;
					}
				}
			}
		}

		surface->SetForeColor(textColor);

		surface->SetSystemFont(12, FontFamilyFlag_Bold);
		int32_t textV = (m_rect.top + m_rect.bottom + surface->MeasureFontAscender()) / 2;
		surface->DrawString(Point::Create(m_rect.left + checkFrameSize + 2, textV), m_text.ToShortStr(), true);
	}


	void ButtonWidget::DrawAsRadio(DrawSurface *surface, bool inverted)
	{
		if (!m_rect.IsValid())
			return;

		surface->SetForeColor(StdColors::White());
		surface->FillRect(m_rect);

		uint16_t checkFrameSize = std::min<uint16_t>(12, std::min(m_rect.Width(), m_rect.Height()));
		int16_t top = (m_rect.top + m_rect.bottom - static_cast<int16_t>(checkFrameSize)) / 2;

		const Rect checkRect = Rect::Create(top, m_rect.left, top + static_cast<int16_t>(checkFrameSize), m_rect.left + static_cast<int16_t>(checkFrameSize));

		RGBAColor radioColor;
		RGBAColor textColor;
		if (!m_enabled)
		{
			surface->SetForeColor(RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255));
			surface->FillEllipse(checkRect);
			surface->SetForeColor(RGBAColor::Create(kLightGray, kLightGray, kLightGray, 255));
			surface->FillEllipse(checkRect.Inset(1, 1));

			radioColor = RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255);
			textColor = RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255);
		}
		else if (inverted)
		{
			surface->SetForeColor(StdColors::Black());
			surface->FillEllipse(checkRect);
			surface->SetForeColor(RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255));
			surface->FillEllipse(checkRect.Inset(1, 1));

			radioColor = StdColors::Black();
			textColor = StdColors::Black();
		}
		else
		{
			gs_buttonRadioGraphic.DrawToPixMapWithMask(surface->m_port.GetPixMap(), gs_buttonRadioGraphicMask, checkRect.left, checkRect.top);

			radioColor = StdColors::Black();
			textColor = StdColors::Black();
		}

		if (m_state)
		{
			const Rect checkmarkRect = checkRect.Inset(3, 3);

			if (checkmarkRect.IsValid())
			{
				surface->SetForeColor(radioColor);
				surface->FillEllipse(checkmarkRect);
			}
		}

		surface->SetForeColor(textColor);

		surface->SetSystemFont(12, FontFamilyFlag_Bold);
		int32_t textV = (m_rect.top + m_rect.bottom + surface->MeasureFontAscender()) / 2;
		surface->DrawString(Point::Create(m_rect.left + checkFrameSize + 2, textV), m_text.ToShortStr(), true);
	}

	void ButtonWidget::DrawDefaultButtonChrome(const Rect &rectRef, DrawSurface *surface)
	{
		const Rect rect = rectRef;
		PixMap **pixMap = surface->m_port.GetPixMap();

		gs_buttonDefaultCornerGraphics[0].DrawToPixMapWithMask(pixMap, gs_buttonDefaultTopLeftGraphicMask, rect.left - 3, rect.top - 3);
		gs_buttonDefaultCornerGraphics[1].DrawToPixMapWithMask(pixMap, gs_buttonDefaultTopRightGraphicMask, rect.right - 2, rect.top - 3);
		gs_buttonDefaultCornerGraphics[2].DrawToPixMapWithMask(pixMap, gs_buttonDefaultBottomLeftGraphicMask, rect.left - 3, rect.bottom - 2);
		gs_buttonDefaultCornerGraphics[3].DrawToPixMapWithMask(pixMap, gs_buttonDefaultBottomRightGraphicMask, rect.right - 2, rect.bottom - 2);

		RGBAColor upperLeftStripeColors[3] =
		{
			RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255),
			StdColors::White(),
			StdColors::Black(),
		};

		RGBAColor bottomRightStripeColors[3] =
		{
			RGBAColor::Create(kMidGray, kMidGray, kMidGray, 255),
			RGBAColor::Create(kDarkGray, kDarkGray, kDarkGray, 255),
			StdColors::Black(),
		};


		for (int i = 0; i < 3; i++)
		{
			surface->SetForeColor(upperLeftStripeColors[i]);
			surface->FillRect(Rect::Create(rect.top - 1 - i, rect.left + 2, rect.top - i, rect.right - 2));
			surface->FillRect(Rect::Create(rect.top + 2, rect.left - 1 - i, rect.bottom - 2, rect.left - i));
		}

		for (int i = 0; i < 3; i++)
		{
			surface->SetForeColor(bottomRightStripeColors[i]);
			surface->FillRect(Rect::Create(rect.bottom + i, rect.left + 2, rect.bottom + i + 1, rect.right - 2));
			surface->FillRect(Rect::Create(rect.top + 2, rect.right + i, rect.bottom - 2, rect.right + i + 1));
		}
	}

	void ButtonWidget::SetHighlightStyle(int16_t style, bool enabled)
	{
		if (style == kControlButtonPart)
		{
			if (m_haveHighlightOverride != enabled)
			{
				m_haveHighlightOverride = enabled;
				DrawControl(&m_window->m_surface);
			}
		}
	}
}
