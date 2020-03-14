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

		void DrawControl(DrawSurface *surface) override;

		WidgetHandleState_t ProcessEvent(const TimeTaggedVOSEvent &evt) override;

		void OnEnabledChanged() override;

		int16_t Capture(const Point &pos, WidgetUpdateCallback_t callback) override;

	private:
		void DrawControlInternal(DrawSurface *surface, bool inverted);

		PascalStr<255> m_text;
	};
}
