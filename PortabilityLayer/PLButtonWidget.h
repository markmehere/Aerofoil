#pragma once

#include "PascalStr.h"
#include "PLWidgets.h"

namespace PortabilityLayer
{
	class ButtonWidget final : public WidgetSpec<ButtonWidget>
	{
	public:
		explicit ButtonWidget(const WidgetBasicState &state);

		bool Init(const WidgetBasicState &state) override;

		WidgetHandleState_t ProcessEvent(Window *window, const TimeTaggedVOSEvent &evt);

	private:
		bool m_haveMouseDown;
		PascalStr<255> m_text;
	};
}
