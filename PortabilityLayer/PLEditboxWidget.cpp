#include "PLEditboxWidget.h"

#include "FontFamily.h"
#include "InputManager.h"
#include "MacRomanConversion.h"
#include "MemoryManager.h"
#include "PLKeyEncoding.h"
#include "PLStandardColors.h"
#include "PLTimeTaggedVOSEvent.h"

#include <algorithm>

namespace PortabilityLayer
{
	EditboxWidget::EditboxWidget(const WidgetBasicState &state)
		: WidgetSpec<EditboxWidget>(state)
		, m_capacity(255)
		, m_length(0)
		, m_chars(nullptr)
		, m_selStartChar(0)
		, m_selEndChar(0)
		, m_hasFocus(false)
		, m_caratTimer(0)
	{
	}

	EditboxWidget::~EditboxWidget()
	{
		PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

		if (m_chars)
			mm->Release(m_chars);
	}

	bool EditboxWidget::Init(const WidgetBasicState &state)
	{
		PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();
		m_capacity = 255;

		m_chars = static_cast<uint8_t*>(mm->Alloc(m_capacity * sizeof(m_chars[0])));
		if (!m_chars)
			return false;

		return true;
	}

	void EditboxWidget::EditboxWidget::DrawControl(DrawSurface *surface)
	{
		const Rect textRect = m_rect;
		const Rect innerRect = textRect.Inset(-2, -2);
		const Rect outerRect = innerRect.Inset(-1, -1);

		surface->SetForeColor(StdColors::Black());
		surface->FillRect(outerRect);
		surface->SetForeColor(StdColors::White());
		surface->FillRect(innerRect);

		surface->SetSystemFont(12, PortabilityLayer::FontFamilyFlag_None);
		int32_t ascender = surface->MeasureFontAscender();
		int32_t lineGap = surface->MeasureFontLineGap();

		const PLPasStr str = this->GetString();

		assert(m_selStartChar <= str.Length());
		assert(m_selEndChar <= str.Length());
		assert(m_selStartChar <= m_selEndChar);

		const char *strChars = str.Chars();

		size_t preSelWidth = 0;
		size_t selWidth = 0;
		//size_t postSelWidth = 0;
		if (m_selStartChar > 0)
			preSelWidth = surface->MeasureString(PLPasStr(static_cast<uint8_t>(m_selStartChar), strChars));

		if (m_selEndChar > m_selStartChar)
			selWidth = surface->MeasureString(PLPasStr(static_cast<uint8_t>(m_selEndChar - m_selStartChar), strChars + m_selStartChar));

		//if (m_selEndChar < str.Length())
		//	postSelWidth = surface->MeasureString(PLPasStr(static_cast<uint8_t>(m_selEndChar - str.Length()), strChars + m_selEndChar));

		Point basePoint = Point::Create(textRect.left, (textRect.top + textRect.bottom + ascender + 1) / 2);

		if (m_hasFocus && selWidth > 0)
		{
			Rect selRect = Rect::Create(m_rect.top, static_cast<int16_t>(basePoint.h + preSelWidth), m_rect.bottom, m_rect.right);
			if (m_selEndChar != str.Length())
				selRect.right = static_cast<int16_t>(basePoint.h + preSelWidth + selWidth);

			selRect = selRect.Intersect(m_rect);

			if (selRect.IsValid())
			{
				PortabilityLayer::RGBAColor focusColor = PortabilityLayer::RGBAColor::Create(153, 153, 255, 255);
				surface->SetForeColor(focusColor);
				surface->FillRect(selRect);
			}
		}

		surface->SetForeColor(StdColors::Black());
		surface->DrawStringConstrained(basePoint, this->GetString(), true, m_rect);

		if (m_hasFocus && m_selEndChar == m_selStartChar && m_caratTimer < kCaratBlinkRate)
		{
			int16_t caratTop = (textRect.top + textRect.bottom - lineGap + 1) / 2;
			int16_t caratBottom = (textRect.top + textRect.bottom + lineGap + 1) / 2;
			int16_t caratH = static_cast<int16_t>(basePoint.h + preSelWidth);
			Rect caratRect = Rect::Create(caratTop, caratH, caratBottom, caratH + 1);

			caratRect = caratRect.Intersect(m_rect);

			if (caratRect.IsValid())
				surface->FillRect(caratRect);
		}
	}

	void EditboxWidget::SetString(const PLPasStr &str)
	{
		const size_t len = std::min<size_t>(m_capacity, str.Length());

		m_length = len;
		memcpy(m_chars, str.UChars(), len);

		if (m_window)
		{
			DrawSurface *surface = m_window->GetDrawSurface();

			DrawControl(surface);
		}

		if (m_selStartChar > len)
			m_selStartChar = len;
		if (m_selEndChar > len)
			m_selEndChar = len;
	}

	PLPasStr EditboxWidget::GetString() const
	{
		const uint8_t len = static_cast<uint8_t>(std::min<size_t>(255, m_length));
		return PLPasStr(len, reinterpret_cast<const char*>(m_chars));
	}

	void EditboxWidget::GainFocus()
	{
		m_hasFocus = true;
		m_selStartChar = 0;
		m_selEndChar = this->GetString().Length();

		if (m_window)
		{
			DrawSurface *surface = m_window->GetDrawSurface();
			DrawControl(surface);
		}
	}

	void EditboxWidget::LoseFocus()
	{
		m_hasFocus = false;
		m_selStartChar = 0;
		m_selEndChar = 0;

		Redraw();
	}

	WidgetHandleState_t EditboxWidget::ProcessEvent(const TimeTaggedVOSEvent &evt)
	{
		if (!m_visible || !m_enabled)
			return WidgetHandleStates::kIgnored;

		if (evt.m_vosEvent.m_eventType == GpVOSEventTypes::kKeyboardInput)
		{
			if (!m_hasFocus)
				return WidgetHandleStates::kIgnored;

			const GpKeyboardInputEvent &keyEvent = evt.m_vosEvent.m_event.m_keyboardInputEvent;

			if (keyEvent.m_eventType == GpKeyboardInputEventTypes::kAutoChar || keyEvent.m_eventType == GpKeyboardInputEventTypes::kDownChar)
			{
				// Resolve character
				bool resolvedChar = false;
				uint8_t ch = 0;

				if (keyEvent.m_keyIDSubset == GpKeyIDSubsets::kASCII)
				{
					ch = static_cast<uint8_t>(keyEvent.m_key.m_asciiChar);
					resolvedChar = true;
				}
				else if (keyEvent.m_keyIDSubset == GpKeyIDSubsets::kUnicode)
				{
					uint32_t codePoint = keyEvent.m_key.m_unicodeChar;

					if (codePoint < 0xffff)
						resolvedChar = MacRoman::FromUnicode(ch, keyEvent.m_key.m_unicodeChar);
				}

				if (resolvedChar)
				{
					if (ch >= 0x20 && ch <= 0x7e)
						HandleCharacter(ch, keyEvent.m_repeatCount);

					return WidgetHandleStates::kDigested;
				}
			}
			else if (keyEvent.m_eventType == GpKeyboardInputEventTypes::kAuto || keyEvent.m_eventType == GpKeyboardInputEventTypes::kDown)
			{
				const KeyDownStates *downStates = PortabilityLayer::InputManager::GetInstance()->GetKeys();
				const bool isShiftHeld = downStates->m_special.Get(GpKeySpecials::kLeftShift) || downStates->m_special.Get(GpKeySpecials::kRightShift);

				if (keyEvent.m_keyIDSubset == GpKeyIDSubsets::kSpecial)
				{
					if (keyEvent.m_key.m_specialKey == GpKeySpecials::kBackspace)
					{
						HandleBackspace(keyEvent.m_repeatCount);
						return WidgetHandleStates::kDigested;
					}
					else if (keyEvent.m_key.m_specialKey == GpKeySpecials::kDelete)
					{
						return WidgetHandleStates::kDigested;
					}
				}
			}
		}

		return WidgetHandleStates::kIgnored;
	}

	void EditboxWidget::Redraw()
	{
		if (m_window)
		{
			DrawSurface *surface = m_window->GetDrawSurface();
			DrawControl(surface);
		}
	}

	bool EditboxWidget::HandlesTickEvents() const
	{
		return true;
	}

	void EditboxWidget::SetSelection(size_t startChar, size_t endChar)
	{
		if (startChar > m_length)
			startChar = m_length;

		if (endChar < startChar)
			endChar = startChar;

		if (endChar > m_length)
			endChar = m_length;

		m_selStartChar = startChar;
		m_selEndChar = endChar;

		m_caratTimer = 0;
		Redraw();
	}

	void EditboxWidget::OnTick()
	{
		if (m_hasFocus)
		{
			m_caratTimer++;
			if (m_caratTimer == kCaratBlinkRate)
				Redraw();
			else if (m_caratTimer == kCaratBlinkRate * 2)
			{
				m_caratTimer = 0;
				Redraw();
			}
		}
	}

	void EditboxWidget::HandleCharacter(uint8_t ch, const uint32_t numRepeatsRequested)
	{
		const size_t numPostSelChars = m_length - m_selEndChar;
		const size_t numSelChars = m_selEndChar - m_selStartChar;
		const size_t numPreSelChars = m_selStartChar;

		const size_t lengthWithSelectionRemoved = m_length - numSelChars;
		const size_t availableInsertions = m_capacity - lengthWithSelectionRemoved;

		const size_t numInsertions = std::min<size_t>(availableInsertions, numRepeatsRequested);

		if (m_selEndChar != m_length)
		{
			size_t moveSize = m_length - m_selEndChar;
			uint8_t *moveSrc = m_chars + m_selEndChar;
			uint8_t *moveDest = m_chars + m_selStartChar + numInsertions;
			if (moveSrc != moveDest)
				memmove(moveDest, moveSrc, numPostSelChars);
		}

		uint8_t *insertPos = m_chars + m_selStartChar;

		for (size_t r = 0; r < numInsertions; r++)
			insertPos[r] = ch;

		// Reset selection
		m_selStartChar += numInsertions;
		m_selEndChar = m_selStartChar;

		// Reset length;
		m_length = numPreSelChars + numInsertions + numPostSelChars;

		m_caratTimer = 0;
		Redraw();
	}

	void EditboxWidget::HandleBackspace(uint32_t numRepeatsRequested)
	{
		const size_t numPostSelChars = m_length - m_selEndChar;
		const size_t numSelChars = m_selEndChar - m_selStartChar;
		const size_t numPreSelChars = m_selStartChar;

		size_t prefixTrim = numRepeatsRequested;
		if (numSelChars != 0)
			prefixTrim--;

		if (prefixTrim > numPreSelChars)
			prefixTrim = numPreSelChars;

		const size_t prefixKeep = numPreSelChars - prefixTrim;
		const size_t suffixKeep = numPostSelChars;

		if (suffixKeep > 0)
		{
			uint8_t *moveSrc = m_chars + m_selEndChar;
			uint8_t *moveDest = m_chars + prefixKeep;

			if (moveSrc != moveDest)
				memmove(moveDest, moveSrc, suffixKeep);
		}

		m_length = prefixKeep + suffixKeep;
		m_selStartChar = m_selEndChar = prefixKeep;

		m_caratTimer = 0;
		Redraw();
	}
}
