#include "PLEditboxWidget.h"

#include "FontFamily.h"
#include "FontManager.h"
#include "HostSystemServices.h"
#include "InputManager.h"
#include "MacRomanConversion.h"
#include "MemoryManager.h"
#include "RenderedFont.h"
#include "GpRenderedFontMetrics.h"
#include "PLKeyEncoding.h"
#include "PLQDraw.h"
#include "PLStandardColors.h"
#include "PLTimeTaggedVOSEvent.h"
#include "ResolveCachingColor.h"
#include "TextPlacer.h"
#include "Rect2i.h"

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
		, m_caratSelectionAnchor(CaratSelectionAnchor_End)
		, m_caratScrollPosition(0, 0)
		, m_caratScrollLocked(false)
		, m_hasFocus(false)
		, m_caratTimer(0)
		, m_isMultiLine(false)
		, m_isDraggingSelection(false)
		, m_dragSelectionStartChar(false)
		, m_scrollOffset(0, 0)
		, m_characterFilter(nullptr)
		, m_characterFilterContext(nullptr)
	{
	}

	EditboxWidget::~EditboxWidget()
	{
		if (m_hasFocus)
			PortabilityLayer::HostSystemServices::GetInstance()->SetTextInputEnabled(false);

		PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

		if (m_chars)
			mm->Release(m_chars);
	}

	bool EditboxWidget::Init(const WidgetBasicState &state, const void *additionalData)
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
		if (!m_visible)
			return;

		ResolveCachingColor blackColor = StdColors::Black();
		ResolveCachingColor whiteColor = StdColors::White();

		const Rect textRect = m_rect;
		const Rect innerRect = textRect.Inset(-2, -2);
		const Rect outerRect = innerRect.Inset(-1, -1);

		surface->FillRect(outerRect, blackColor);
		surface->FillRect(innerRect, whiteColor);

		PortabilityLayer::RenderedFont *sysFont = GetSystemFont(12, PortabilityLayer::FontFamilyFlag_None, true);
		int32_t ascender = sysFont->GetMetrics().m_ascent;
		int32_t lineGap = sysFont->GetMetrics().m_linegap;

		const PLPasStr str = this->GetString();

		assert(m_selStartChar <= str.Length());
		assert(m_selEndChar <= str.Length());
		assert(m_selStartChar <= m_selEndChar);

		const char *strChars = str.Chars();

		Vec2i basePoint = ResolveBasePoint();

		if (m_hasFocus && m_selStartChar != m_selEndChar)
			DrawSelection(surface, basePoint, sysFont);

		int32_t verticalOffset = (ascender + lineGap + 1) / 2;

		const Point stringBasePoint = Point::Create(basePoint.m_x, basePoint.m_y + verticalOffset);

		if (m_isMultiLine)
			surface->DrawStringWrap(stringBasePoint, m_rect, this->GetString(), blackColor, sysFont);
		else
			surface->DrawStringConstrained(stringBasePoint, this->GetString(), m_rect, blackColor, sysFont);

		if (m_hasFocus && m_selEndChar == m_selStartChar && m_caratTimer < kCaratBlinkRate)
		{
			PortabilityLayer::Vec2i caratPos = ResolveCaratPos(sysFont) + basePoint;

			int32_t caratTop = caratPos.m_y;
			int32_t caratBottom = caratTop + lineGap;
			Rect caratRect = Rect::Create(caratTop, caratPos.m_x, caratBottom, caratPos.m_x + 1);

			caratRect = caratRect.Intersect(m_rect);

			if (caratRect.IsValid())
				surface->FillRect(caratRect, blackColor);
		}
	}

	void EditboxWidget::SetString(const PLPasStr &str)
	{
		const size_t len = std::min<size_t>(m_capacity, str.Length());

		m_length = len;
		memcpy(m_chars, str.UChars(), len);

		if (m_selStartChar > len)
			m_selStartChar = len;
		if (m_selEndChar > len)
			m_selEndChar = len;

		if (m_window)
		{
			DrawSurface *surface = m_window->GetDrawSurface();

			DrawControl(surface);
		}
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
		m_caratSelectionAnchor = CaratSelectionAnchor_End;
		m_caratScrollLocked = false;

		if (m_window)
		{
			DrawSurface *surface = m_window->GetDrawSurface();
			DrawControl(surface);
		}

		PortabilityLayer::HostSystemServices::GetInstance()->SetTextInputEnabled(true);
	}

	void EditboxWidget::LoseFocus()
	{
		if (m_hasFocus)
			PortabilityLayer::HostSystemServices::GetInstance()->SetTextInputEnabled(false);

		m_hasFocus = false;
		m_selStartChar = 0;
		m_selEndChar = 0;
		m_caratSelectionAnchor = CaratSelectionAnchor_End;
		m_caratScrollLocked = false;

		Redraw();
	}

	WidgetHandleState_t EditboxWidget::ProcessEvent(void *captureContext, const TimeTaggedVOSEvent &evt)
	{
		if (m_isDraggingSelection)
			return HandleDragSelection(evt);

		if (!m_visible || !m_enabled)
			return WidgetHandleStates::kIgnored;

		if (evt.m_vosEvent.m_eventType == GpVOSEventTypes::kKeyboardInput)
		{
			if (!m_hasFocus || m_isDraggingSelection)
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
					if (m_characterFilter)
						resolvedChar = m_characterFilter(m_characterFilterContext, ch);
				}

				if (resolvedChar)
				{
					if (ch >= 0x20 && ch <= 0x7e)
						HandleCharacter(ch, keyEvent.m_repeatCount);
					else if ((ch == '\r' || ch == '\n') && m_isMultiLine)
						HandleCharacter('\r', keyEvent.m_repeatCount);

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
					else if (keyEvent.m_key.m_specialKey == GpKeySpecials::kUpArrow)
					{
						HandleUpArrow(keyEvent.m_repeatCount, isShiftHeld);
						return WidgetHandleStates::kDigested;
					}
					else if (keyEvent.m_key.m_specialKey == GpKeySpecials::kLeftArrow)
					{
						HandleLeftArrow(keyEvent.m_repeatCount, isShiftHeld);
						return WidgetHandleStates::kDigested;
					}
					else if (keyEvent.m_key.m_specialKey == GpKeySpecials::kRightArrow)
					{
						HandleRightArrow(keyEvent.m_repeatCount, isShiftHeld);
						return WidgetHandleStates::kDigested;
					}
					else if (keyEvent.m_key.m_specialKey == GpKeySpecials::kDownArrow)
					{
						HandleDownArrow(keyEvent.m_repeatCount, isShiftHeld);
						return WidgetHandleStates::kDigested;
					}
					else if (keyEvent.m_key.m_specialKey == GpKeySpecials::kDelete)
					{
						HandleForwardDelete(keyEvent.m_repeatCount);
						return WidgetHandleStates::kDigested;
					}
					else if (keyEvent.m_key.m_specialKey == GpKeySpecials::kHome)
					{
						HandleHome(isShiftHeld);
						return WidgetHandleStates::kDigested;
					}
					else if (keyEvent.m_key.m_specialKey == GpKeySpecials::kEnd)
					{
						HandleEnd(isShiftHeld);
						return WidgetHandleStates::kDigested;
					}
				}
			}
		}
		else if (evt.m_vosEvent.m_eventType == GpVOSEventTypes::kMouseInput)
		{
			const GpMouseInputEvent &mouseEvent = evt.m_vosEvent.m_event.m_mouseInputEvent;

			if (evt.IsLMouseDownEvent())
			{
				const Point pt = m_window->MouseToLocal(evt.m_vosEvent.m_event.m_mouseInputEvent);

				if (m_rect.Contains(pt))
				{
					m_window->FocusWidget(this);
					m_isDraggingSelection = true;
					return HandleDragSelection(evt);
				}
				else
					return WidgetHandleStates::kIgnored;
			}
		}

		return WidgetHandleStates::kIgnored;
	}

	Rect EditboxWidget::GetExpandedRect() const
	{
		return GetRect().Inset(-3, -3);
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
		m_caratSelectionAnchor = CaratSelectionAnchor_End;

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

		AdjustScrollToCarat();

		m_caratTimer = 0;
		Redraw();

		m_caratScrollLocked = false;
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

		AdjustScrollToCarat();
		AdjustScrollToTextBounds();

		m_caratTimer = 0;
		Redraw();

		m_caratScrollLocked = false;
	}

	void EditboxWidget::HandleForwardDelete(const uint32_t numRepeatsRequested)
	{
		const size_t numPostSelChars = m_length - m_selEndChar;
		const size_t numSelChars = m_selEndChar - m_selStartChar;
		const size_t numPreSelChars = m_selStartChar;

		size_t suffixTrim = numRepeatsRequested;
		if (numSelChars != 0)
			suffixTrim--;

		if (suffixTrim > numPostSelChars)
			suffixTrim = numPostSelChars;

		const size_t prefixKeep = numPreSelChars;
		const size_t suffixKeep = numPostSelChars - suffixTrim;

		if (suffixKeep > 0)
		{
			uint8_t *moveSrc = m_chars + m_length - suffixKeep;
			uint8_t *moveDest = m_chars + m_selStartChar;

			if (moveSrc != moveDest)
				memmove(moveDest, moveSrc, suffixKeep);
		}

		m_length = prefixKeep + suffixKeep;
		m_selStartChar = m_selEndChar = prefixKeep;

		AdjustScrollToCarat();
		AdjustScrollToTextBounds();

		m_caratTimer = 0;
		Redraw();

		m_caratScrollLocked = false;
	}

	void EditboxWidget::HandleUpArrow(const uint32_t numRepeatsRequested, bool shiftHeld)
	{
		if (!m_isMultiLine)
			return;

		size_t caratChar = ResolveCaratChar();

		PortabilityLayer::RenderedFont *rfont = GetRenderedFont();
		int32_t lineGap = rfont->GetMetrics().m_linegap;

		if (!rfont)
			return;

		if (!m_caratScrollLocked)
		{
			m_caratScrollPosition = ResolveCaratPos(rfont);
			m_caratScrollLocked = true;
		}

		Vec2i caratPos = m_caratScrollPosition;

		for (uint32_t r = 0; r < numRepeatsRequested; r++)
		{
			bool isOutOfRange = false;
			m_caratScrollPosition.m_y -= lineGap;
			caratChar = FindVerticalMovementCaratPos(m_caratScrollPosition, isOutOfRange);
			HandleKeyMoveCarat(caratChar, shiftHeld);

			if (isOutOfRange)
			{
				m_caratScrollPosition.m_y += lineGap;
				break;
			}
		}

		AdjustScrollToCarat();

		m_caratTimer = 0;
		Redraw();
	}

	void EditboxWidget::HandleDownArrow(const uint32_t numRepeatsRequested, bool shiftHeld)
	{
		if (!m_isMultiLine)
			return;

		size_t caratChar = ResolveCaratChar();

		PortabilityLayer::RenderedFont *rfont = GetRenderedFont();
		int32_t lineGap = rfont->GetMetrics().m_linegap;

		if (!rfont)
			return;

		if (!m_caratScrollLocked)
		{
			m_caratScrollPosition = ResolveCaratPos(rfont);
			m_caratScrollLocked = true;
		}

		Vec2i caratPos = m_caratScrollPosition;

		for (uint32_t r = 0; r < numRepeatsRequested; r++)
		{
			bool isOutOfRange = false;
			m_caratScrollPosition.m_y += lineGap;
			caratChar = FindVerticalMovementCaratPos(m_caratScrollPosition, isOutOfRange);
			HandleKeyMoveCarat(caratChar, shiftHeld);

			if (isOutOfRange)
			{
				m_caratScrollPosition.m_y -= lineGap;
				break;
			}
		}

		AdjustScrollToCarat();

		m_caratTimer = 0;
		Redraw();
	}

	void EditboxWidget::HandleLeftArrow(const uint32_t numRepeatsRequested, bool shiftHeld)
	{
		size_t caratChar = ResolveCaratChar();

		for (uint32_t r = 0; r < numRepeatsRequested; r++)
		{
			if (!shiftHeld && m_selStartChar != m_selEndChar)
				m_selEndChar = m_selStartChar;
			else if (caratChar > 0)
				HandleKeyMoveCarat(caratChar - 1, shiftHeld);
		}

		m_caratScrollLocked = false;

		AdjustScrollToCarat();

		m_caratTimer = 0;
		Redraw();
	}

	void EditboxWidget::HandleHome(bool shiftHeld)
	{
		const size_t originalCaratChar = ResolveCaratChar();
		size_t caratChar = originalCaratChar;

		while (caratChar > 0)
		{
			uint8_t prevChar = m_chars[caratChar - 1];
			if (prevChar == '\r')
				break;
			else
				caratChar--;
		}

		if (originalCaratChar != caratChar)
		{
			HandleKeyMoveCarat(caratChar, shiftHeld);

			m_caratScrollLocked = false;

			AdjustScrollToCarat();

			m_caratTimer = 0;
			Redraw();
		}
	}

	void EditboxWidget::HandleEnd(bool shiftHeld)
	{
		const size_t originalCaratChar = ResolveCaratChar();
		size_t caratChar = originalCaratChar;

		while (caratChar < m_length)
		{
			uint8_t nextChar = m_chars[caratChar];
			if (nextChar == '\r')
				break;
			else
				caratChar++;
		}

		if (originalCaratChar != caratChar)
		{
			HandleKeyMoveCarat(caratChar, shiftHeld);

			m_caratScrollLocked = false;

			AdjustScrollToCarat();

			m_caratTimer = 0;
			Redraw();
		}
	}


	void EditboxWidget::HandleRightArrow(const uint32_t numRepeatsRequested, bool shiftHeld)
	{
		size_t caratChar = ResolveCaratChar();

		for (uint32_t r = 0; r < numRepeatsRequested; r++)
		{
			if (!shiftHeld && m_selStartChar != m_selEndChar)
				m_selStartChar = m_selEndChar;
			else if (caratChar < m_length)
				HandleKeyMoveCarat(caratChar + 1, shiftHeld);
		}

		m_caratScrollLocked = false;

		AdjustScrollToCarat();

		m_caratTimer = 0;
		Redraw();
	}

	size_t EditboxWidget::FindVerticalMovementCaratPos(const Vec2i &desiredPos, bool &isOutOfRange) const
	{
		Vec2i basePoint = Vec2i(0, 0);

		if (desiredPos.m_y < basePoint.m_y)
		{
			isOutOfRange = true;
			return 0;
		}

		PortabilityLayer::TextPlacer placer(basePoint, m_rect.Width(), GetRenderedFont(), GetString());

		bool foundLine = false;
		size_t caratChar = 0;

		PortabilityLayer::GlyphPlacementCharacteristics characteristics;
		while (placer.PlaceGlyph(characteristics))
		{
			if (characteristics.m_glyphStartPos.m_y > desiredPos.m_y)
				break;

			if (characteristics.m_glyphStartPos.m_y == desiredPos.m_y)
			{
				foundLine = true;

				caratChar = characteristics.m_characterIndex;

				if (desiredPos.m_x <= 0)
					break;

				if (characteristics.m_character != '\r')
					caratChar++;

				if (characteristics.m_glyphStartPos.m_x <= desiredPos.m_x && characteristics.m_glyphEndPos.m_x > desiredPos.m_x)
				{
					int32_t distanceToEnd = characteristics.m_glyphEndPos.m_x - desiredPos.m_x;
					int32_t distanceToStart = desiredPos.m_x - characteristics.m_glyphStartPos.m_x;

					if (distanceToStart <= distanceToEnd)
						caratChar = characteristics.m_characterIndex;
					else
						caratChar = characteristics.m_characterIndex + 1;

					break;
				}
			}
		}

		if (foundLine)
		{
			isOutOfRange = false;
			return caratChar;
		}

		isOutOfRange = true;
		return m_length;
	}

	// Handles adjustment of the selection range and anchor when the carat is moved with shift held
	void EditboxWidget::HandleKeyMoveCarat(size_t newPos, bool shiftHeld)
	{
		if (shiftHeld)
		{
			size_t otherSelection = m_selStartChar;
			if (m_caratSelectionAnchor == CaratSelectionAnchor_Start)
				otherSelection = m_selEndChar;

			m_selStartChar = std::min<size_t>(newPos, otherSelection);
			m_selEndChar = std::max<size_t>(newPos, otherSelection);

			if (m_selStartChar == newPos)
				m_caratSelectionAnchor = CaratSelectionAnchor_Start;
			else if (m_selEndChar == newPos)
				m_caratSelectionAnchor = CaratSelectionAnchor_End;
		}
		else
		{
			m_selStartChar = newPos;
			m_selEndChar = newPos;
		}
	}

	WidgetHandleState_t EditboxWidget::HandleDragSelection(const TimeTaggedVOSEvent &evt)
	{
		if (evt.m_vosEvent.m_eventType == GpVOSEventTypes::kMouseInput)
		{
			const GpMouseInputEvent &mouseEvent = evt.m_vosEvent.m_event.m_mouseInputEvent;

			const Point pt = m_window->MouseToLocal(evt.m_vosEvent.m_event.m_mouseInputEvent);

			RenderedFont *rfont = GetRenderedFont();
			const int32_t linegap = rfont->GetMetrics().m_linegap;

			const Vec2i basePoint = ResolveBasePoint();
			const Vec2i relativePoint = Vec2i(pt.h, pt.v) - basePoint;

			int32_t relativeY = relativePoint.m_y;
			int32_t paragraph = 0;
			if (relativeY >= 0)
				paragraph = relativeY / linegap;
			else
				paragraph = -((-relativeY + (linegap - 1)) / linegap);

			bool isOutOfRange = false;
			const size_t caratPos = FindVerticalMovementCaratPos(Vec2i(relativePoint.m_x, paragraph * linegap), isOutOfRange);

			if (mouseEvent.m_eventType == GpMouseEventTypes::kDown)
			{
				m_dragSelectionStartChar = caratPos;
				m_selStartChar = caratPos;
				m_selEndChar = caratPos;
				m_caratSelectionAnchor = CaratSelectionAnchor_End;

				m_caratTimer = 0;
				Redraw();
			}
			else
			{
				if (caratPos < m_dragSelectionStartChar)
				{
					m_caratSelectionAnchor = CaratSelectionAnchor_Start;
					m_selStartChar = caratPos;
					m_selEndChar = m_dragSelectionStartChar;
				}
				else
				{
					m_caratSelectionAnchor = CaratSelectionAnchor_End;
					m_selEndChar = caratPos;
					m_selStartChar = m_dragSelectionStartChar;
				}

				AdjustScrollToCarat();

				m_caratTimer = 0;
				Redraw();

				if (mouseEvent.m_eventType == GpMouseEventTypes::kUp)
				{
					m_caratScrollLocked = false;
					m_isDraggingSelection = false;
					return WidgetHandleStates::kDigested;
				}
			}
		}

		return WidgetHandleStates::kCaptured;
	}

	void EditboxWidget::DrawSelection(DrawSurface *surface, const Vec2i &basePoint, PortabilityLayer::RenderedFont *rfont) const
	{
		PortabilityLayer::TextPlacer placer(basePoint, m_isMultiLine ? m_rect.Width() : -1, rfont, GetString());

		PortabilityLayer::Vec2i globalSelStart;
		PortabilityLayer::Vec2i globalSelEnd;
		bool endIsLineBreak = false;
		bool startSet = false;
		bool endSet = false;

		PortabilityLayer::GlyphPlacementCharacteristics characteristics;
		size_t placedIndex = 0;

		while (placer.PlaceGlyph(characteristics))
		{
			bool isTerminalForThisPara = false;
			bool isTerminalForEverything = false;
			bool isLineBreakSelected = false;

			if (characteristics.m_characterIndex == m_selStartChar)
			{
				globalSelStart = characteristics.m_glyphStartPos;
				startSet = true;
			}

			if (characteristics.m_characterIndex + 1 == m_selEndChar)
			{
				globalSelEnd = characteristics.m_glyphEndPos;
				if (characteristics.m_character == '\r')
					endIsLineBreak = true;

				endSet = true;
				break;
			}
		}

		if (!endSet || !startSet)
		{
			assert(false);
			return;
		}

		ResolveCachingColor focusColor = RGBAColor::Create(153, 153, 255, 255);

		int32_t lineGap = rfont->GetMetrics().m_linegap;
		int32_t ascender = rfont->GetMetrics().m_ascent;
		int32_t startY = basePoint.m_y;

		if (globalSelStart.m_y == globalSelEnd.m_y)
		{
			Rect selRect = Rect::Create(globalSelStart.m_y, globalSelStart.m_x, globalSelStart.m_y + lineGap, globalSelEnd.m_x).Intersect(m_rect);
			if (endIsLineBreak || (m_isMultiLine == false && m_selEndChar == m_length))
				selRect.right = m_rect.right;

			surface->FillRect(selRect, focusColor);
		}
		else
		{
			const Rect firstLineRect = Rect::Create(globalSelStart.m_y, globalSelStart.m_x, globalSelStart.m_y + lineGap, m_rect.right).Intersect(m_rect);
			surface->FillRect(firstLineRect, focusColor);

			const Rect midLinesRect = Rect::Create(globalSelStart.m_y + lineGap, m_rect.left, globalSelEnd.m_y, m_rect.right).Intersect(m_rect);
			surface->FillRect(midLinesRect, focusColor);

			Rect lastLineRect = Rect::Create(globalSelEnd.m_y, m_rect.left, globalSelEnd.m_y + lineGap, globalSelEnd.m_x).Intersect(m_rect);
			if (endIsLineBreak || (m_isMultiLine == false && m_selEndChar == m_length))
				lastLineRect.right = m_rect.right;

			surface->FillRect(lastLineRect, focusColor);
		}
	}

	// This function returns the actual coordinate of the carat relative to the top-left corner of the text
	Vec2i EditboxWidget::ResolveCaratPos(PortabilityLayer::RenderedFont *rfont) const
	{
		int32_t lineGap = rfont->GetMetrics().m_linegap;
		bool failed = false;

		PortabilityLayer::Vec2i caratPos = Vec2i(0, 0);

		const size_t caratChar = ResolveCaratChar();

		if (caratChar > 0)
		{
			PortabilityLayer::RenderedFont *rfont = GetRenderedFont();
			PortabilityLayer::TextPlacer placer(Vec2i(0, 0), m_isMultiLine ? m_rect.Width() : -1, rfont, GetString());

			PortabilityLayer::GlyphPlacementCharacteristics characteristics;
			for (size_t i = 0; i < caratChar; i++)
			{
				if (!placer.PlaceGlyph(characteristics))
				{
					failed = true;
					break;
				}
			}

			if (!failed)
			{
				if (characteristics.m_character == '\r')
					caratPos = PortabilityLayer::Vec2i(0, characteristics.m_glyphStartPos.m_y + lineGap);
				else
					caratPos = characteristics.m_glyphEndPos;
			}
		}

		return caratPos;
	}

	Vec2i EditboxWidget::ResolveBasePoint() const
	{
		return Vec2i(m_rect.left, m_rect.top) + m_scrollOffset;
	}

	size_t EditboxWidget::ResolveCaratChar() const
	{
		if (m_caratSelectionAnchor == CaratSelectionAnchor_End)
			return m_selEndChar;
		else
			return m_selStartChar;
	}

	void EditboxWidget::AdjustScrollToCarat()
	{
		PortabilityLayer::RenderedFont *rfont = GetRenderedFont();
		int32_t lineGap = rfont->GetMetrics().m_linegap;

		Vec2i caratRelativePos = m_scrollOffset + ResolveCaratPos(rfont);

		int32_t w = m_rect.Width();
		int32_t h = m_rect.Height();

		Rect2i caratRect = Rect2i(caratRelativePos, caratRelativePos + Vec2i(1, lineGap));

		Vec2i nudge = Vec2i(0, 0);
		if (caratRect.Right() > w)
			nudge.m_x = w - caratRect.Right();
		if (caratRect.Bottom() > h)
			nudge.m_y = h - caratRect.Bottom();
		if (caratRect.Left() < 0)
			nudge.m_x = -caratRect.Left();
		if (caratRect.Top() < 0)
			nudge.m_y = -caratRect.Top();

		m_scrollOffset += nudge;
	}

	void EditboxWidget::AdjustScrollToTextBounds()
	{
		if (!m_isMultiLine)
			return;

		PortabilityLayer::RenderedFont *rfont = GetRenderedFont();
		int32_t lineGap = rfont->GetMetrics().m_linegap;

		bool failed = false;

		PortabilityLayer::Vec2i caratPos = Vec2i(0, 0);

		PortabilityLayer::TextPlacer placer(Vec2i(0, 0), m_isMultiLine ? m_rect.Width() : -1, rfont, GetString());

		PortabilityLayer::GlyphPlacementCharacteristics characteristics;
		for (size_t i = 0; i < m_length; i++)
		{
			if (!placer.PlaceGlyph(characteristics))
			{
				failed = true;
				break;
			}
		}

		if (!failed)
		{
			int32_t lowerY = 0;
			if (characteristics.m_character == '\r')
				lowerY = characteristics.m_glyphStartPos.m_y + lineGap;
			else
				lowerY = characteristics.m_glyphEndPos.m_y;

			lowerY = lowerY + lineGap + m_scrollOffset.m_y;

			int32_t h = m_rect.Height();
			if (lowerY < h)
				m_scrollOffset.m_y -= lowerY - h;

			if (m_scrollOffset.m_y > 0)
				m_scrollOffset.m_y = 0;
		}
	}

	FontFamily *EditboxWidget::GetFontFamily() const
	{
		return PortabilityLayer::FontManager::GetInstance()->GetSystemFont(12, FontFamilyFlag_None);
	}

	RenderedFont *EditboxWidget::GetRenderedFont() const
	{
		return PortabilityLayer::FontManager::GetInstance()->GetRenderedFontFromFamily(GetFontFamily(), 12, true, FontFamilyFlag_None);
	}

	void EditboxWidget::SetMultiLine(bool isMultiLine)
	{
		if (m_isMultiLine != isMultiLine)
		{
			m_isMultiLine = isMultiLine;
			Redraw();
		}
	}

	void EditboxWidget::SetCharacterFilter(void *context, CharacterFilterCallback_t callback)
	{
		m_characterFilterContext = context;
		m_characterFilter = callback;
	}

	void EditboxWidget::SetCapacity(size_t capacity)
	{
		m_capacity = std::min<size_t>(255, capacity);
	}
}
