#pragma once

#include "PLWidgets.h"

namespace PortabilityLayer
{
	class InvisibleWidget final : public WidgetSpec<InvisibleWidget>
	{
	public:
		InvisibleWidget(const WidgetBasicState &state);
		~InvisibleWidget();

		bool Init(const WidgetBasicState &state) override;
	};
}
