#include "PLScrollBarWidget.h"

namespace PortabilityLayer
{
	ScrollBarWidget::ScrollBarWidget(const WidgetBasicState &state)
		: WidgetSpec<ScrollBarWidget>(state)
	{
	}

	bool ScrollBarWidget::Init(const WidgetBasicState &state)
	{
		return true;
	}
}
