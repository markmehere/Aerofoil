#pragma once

#include "PascalStr.h"
#include "PLWidgets.h"

namespace PortabilityLayer
{
	class RadioButtonWidget final : public WidgetSpec<RadioButtonWidget>
	{
	public:
		RadioButtonWidget(const WidgetBasicState &state);
		~RadioButtonWidget();

		bool Init(const WidgetBasicState &state) override;

		void DrawControl(DrawSurface *surface) override;
		void SetString(const PLPasStr &str) override;
		PLPasStr GetString() const override;

		void OnStateChanged() override;

	private:
		PascalStr<255> m_text;
	};
}
