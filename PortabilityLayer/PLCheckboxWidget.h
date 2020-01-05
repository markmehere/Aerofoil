#pragma once

#include "PLWidgets.h"

namespace PortabilityLayer
{
	class CheckboxWidget final : public WidgetSpec<CheckboxWidget>
	{
	public:
		CheckboxWidget(const WidgetBasicState &state);
		~CheckboxWidget();

		bool Init(const WidgetBasicState &state) override;
	};
}
