#pragma once

#include "PascalStr.h"
#include "PLWidgets.h"

namespace PortabilityLayer
{
	class PopupMenuWidget final : public WidgetSpec<PopupMenuWidget>
	{
	public:
		explicit PopupMenuWidget(const WidgetBasicState &state);

		bool Init(const WidgetBasicState &state) override;
	};
}
