#pragma once

#include "PascalStr.h"
#include "PLWidgets.h"

namespace PortabilityLayer
{
	class LabelWidget final : public WidgetSpec<LabelWidget>
	{
	public:
		LabelWidget(const WidgetBasicState &state);

		bool Init(const WidgetBasicState &state, const void *additionalData) override;

		void SetString(const PLPasStr &str) override;
		PLPasStr GetString() const override;

		void DrawControl(DrawSurface *surface) override;

	private:
		PascalStr<255> m_text;
	};
}
