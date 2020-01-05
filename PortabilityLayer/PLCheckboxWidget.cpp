#include "PLCheckboxWidget.h"

namespace PortabilityLayer
{
	CheckboxWidget::CheckboxWidget(const WidgetBasicState &state)
		: WidgetSpec<CheckboxWidget>(state)
	{
	}

	CheckboxWidget::~CheckboxWidget()
	{
	}

	bool CheckboxWidget::Init(const WidgetBasicState &state)
	{
		(void)state;

		return true;
	}
}
