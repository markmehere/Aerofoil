#include "PLControlDefinitions.h"
#include "PLArrayView.h"
#include "PLArrayViewIterator.h"
#include "PLWidgets.h"

int FindControl(Point point, WindowPtr window, PortabilityLayer::Widget **outControl)
{
	// Returns clicked part
	ArrayView<PortabilityLayer::Widget*> widgets = window->GetWidgets();

	for (ArrayViewIterator<PortabilityLayer::Widget*const> it = widgets.begin(), itEnd = widgets.end(); it != itEnd; ++it)
	{
		PortabilityLayer::Widget *widget = *it;
		const Rect widgetRect = widget->GetRect();
		if (widgetRect.Contains(point))
		{
			int part = widget->ResolvePart(point);
			if (part != 0)
			{
				*outControl = widget;
				return part;
			}
		}
	}

	*outControl = nullptr;
	return 0;
}
