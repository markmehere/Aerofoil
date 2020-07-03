#pragma once

#include "PascalStr.h"
#include "PLWidgets.h"
#include "Vec2i.h"

struct DrawSurface;

namespace PortabilityLayer
{
	class EditboxWidget final : public WidgetSpec<EditboxWidget>
	{
	public:
		EditboxWidget(const WidgetBasicState &state);
		~EditboxWidget();

		bool Init(const WidgetBasicState &state, const void *additionalData) override;

		void DrawControl(DrawSurface *surface) override;
		void SetString(const PLPasStr &str) override;
		PLPasStr GetString() const override;

		void GainFocus() override;
		void LoseFocus() override;

		WidgetHandleState_t ProcessEvent(const TimeTaggedVOSEvent &evt) override;

		Rect GetExpandedRect() const override;

		bool HandlesTickEvents() const;

		void SetSelection(size_t startChar, size_t endChar);

		void SetMultiLine(bool isMultiLine);

	private:
		static const unsigned int kCaratBlinkRate = 20;
		static const unsigned int kMouseScrollRate = 20;

		enum CaratSelectionAnchor
		{
			CaratSelectionAnchor_Start,
			CaratSelectionAnchor_End
		};

		void OnTick() override;
		void Redraw();

		void HandleCharacter(uint8_t keyChar, const uint32_t numRepeatsRequested);
		void HandleBackspace(const uint32_t numRepeatsRequested);
		void HandleForwardDelete(const uint32_t numRepeatsRequested);

		void HandleUpArrow(const uint32_t numRepeatsRequested, bool shiftHeld);
		void HandleDownArrow(const uint32_t numRepeatsRequested, bool shiftHeld);
		void HandleLeftArrow(const uint32_t numRepeatsRequested, bool shiftHeld);
		void HandleRightArrow(const uint32_t numRepeatsRequested, bool shiftHeld);

		size_t FindVerticalMovementCaratPos(const Vec2i &desiredPos, bool &isOutOfRange) const;
		void HandleKeyMoveCarat(size_t newPos, bool shiftHeld);

		WidgetHandleState_t HandleDragSelection(const TimeTaggedVOSEvent &evt);

		void DrawSelection(DrawSurface *surface, const Vec2i &basePoint, PortabilityLayer::RenderedFont *font) const;

		Vec2i ResolveCaratPos(PortabilityLayer::RenderedFont *rfont) const;
		Vec2i ResolveBasePoint() const;
		size_t ResolveCaratChar() const;
		void AdjustScrollToCarat();
		void AdjustScrollToTextBounds();

		PortabilityLayer::FontFamily *GetFontFamily() const;
		PortabilityLayer::RenderedFont *GetRenderedFont() const;

		uint8_t *m_chars;
		size_t m_capacity;
		size_t m_length;
		size_t m_selStartChar;
		size_t m_selEndChar;
		CaratSelectionAnchor m_caratSelectionAnchor;	// Where the carat is attached to the selection range

		Vec2i m_caratScrollPosition;	// Ideal position of the carat in the editbox, but not necessarily its actual location (i.e. may be in the middle of a glyph)
		bool m_caratScrollLocked;		// If true, the vertical position 

		Vec2i m_scrollOffset;

		bool m_hasFocus;
		bool m_isMultiLine;
		bool m_isDraggingSelection;
		size_t m_dragSelectionStartChar;

		uint16_t m_caratTimer;
		uint16_t m_selectionScrollTimer;
	};
}
