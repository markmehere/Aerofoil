#pragma once

#include "PascalStr.h"
#include "PLWidgets.h"

namespace PortabilityLayer
{
	class EditboxWidget final : public WidgetSpec<EditboxWidget>
	{
	public:
		EditboxWidget(const WidgetBasicState &state);
		~EditboxWidget();

		bool Init(const WidgetBasicState &state) override;

		void DrawControl(DrawSurface *surface) override;
		void SetString(const PLPasStr &str) override;
		PLPasStr GetString() const override;

		void GainFocus() override;
		void LoseFocus() override;

		WidgetHandleState_t ProcessEvent(const TimeTaggedVOSEvent &evt) override;

		Rect GetExpandedRect() const override;

		bool HandlesTickEvents() const;

		void SetSelection(size_t startChar, size_t endChar);

	private:
		static const unsigned int kCaratBlinkRate = 20;

		void OnTick() override;
		void Redraw();

		void HandleCharacter(uint8_t keyChar, const uint32_t numRepeatsRequested);
		void HandleBackspace(const uint32_t numRepeatsRequested);
		void HandleForwardDelete(const uint32_t numRepeatsRequested);

		uint8_t *m_chars;
		size_t m_capacity;
		size_t m_length;
		size_t m_selStartChar;
		size_t m_selEndChar;

		bool m_hasFocus;
		uint16_t m_caratTimer;
	};
}
