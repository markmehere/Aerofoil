#pragma once

#include "PLWidgets.h"

namespace PortabilityLayer
{
	class InvisibleWidget final : public WidgetSpec<InvisibleWidget>
	{
	public:
		explicit InvisibleWidget(const WidgetBasicState &state);
		~InvisibleWidget();

		bool Init(const WidgetBasicState &state) override;

		WidgetHandleState_t ProcessEvent(const TimeTaggedVOSEvent &evt) override;

	private:
		bool m_clickable;
	};
}
