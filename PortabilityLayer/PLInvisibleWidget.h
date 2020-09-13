#pragma once

#include "PLWidgets.h"

namespace PortabilityLayer
{
	class InvisibleWidget final : public WidgetSpec<InvisibleWidget>
	{
	public:
		explicit InvisibleWidget(const WidgetBasicState &state);
		~InvisibleWidget();

		bool Init(const WidgetBasicState &state, const void *additionalData) override;

		WidgetHandleState_t ProcessEvent(void *captureContext, const TimeTaggedVOSEvent &evt) override;

	private:
		bool m_clickable;
	};
}
