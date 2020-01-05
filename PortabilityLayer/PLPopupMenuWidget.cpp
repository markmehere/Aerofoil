#include "PLPopupMenuWidget.h"

namespace PortabilityLayer
{
	PopupMenuWidget::PopupMenuWidget(const WidgetBasicState &state)
		: WidgetSpec<PopupMenuWidget>(state)
	{
	}

	bool PopupMenuWidget::Init(const WidgetBasicState &state)
	{
		return true;
	}
}
