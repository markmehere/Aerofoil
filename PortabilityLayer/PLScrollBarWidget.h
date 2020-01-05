#pragma once

#include "PascalStr.h"
#include "PLWidgets.h"

namespace PortabilityLayer
{
	class ScrollBarWidget final : public WidgetSpec<ScrollBarWidget>
	{
	public:
		explicit ScrollBarWidget(const WidgetBasicState &state);

		bool Init(const WidgetBasicState &state) override;
	};
}
