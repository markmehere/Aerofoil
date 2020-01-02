#include "PLInvisibleWidget.h"

namespace PortabilityLayer
{
	InvisibleWidget::InvisibleWidget(const WidgetBasicState &state)
		: WidgetSpec<InvisibleWidget>(state)
	{
	}

	InvisibleWidget::~InvisibleWidget()
	{
	}

	bool InvisibleWidget::Init(const WidgetBasicState &state)
	{
		(void)state;

		return true;
	}
}
