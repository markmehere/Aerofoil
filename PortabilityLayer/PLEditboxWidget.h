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

	private:
		uint8_t *m_chars;
		size_t m_capacity;
		size_t m_length;
		size_t m_selStartChar;
		size_t m_selEndChar;
	};
}
