#pragma once

#include "PascalStr.h"
#include "PLWidgets.h"

namespace PortabilityLayer
{
	class CheckboxWidget final : public WidgetSpec<CheckboxWidget>
	{
	public:
		CheckboxWidget(const WidgetBasicState &state);
		~CheckboxWidget();

		bool Init(const WidgetBasicState &state, const void *additionalData) override;

		void DrawControl(DrawSurface *surface) override;
		void SetString(const PLPasStr &str) override;
		PLPasStr GetString() const override;

		void OnStateChanged() override;
		void OnEnabledChanged() override;
		int16_t Capture(const Point &pos, WidgetUpdateCallback_t callback) override;

		WidgetHandleState_t ProcessEvent(const TimeTaggedVOSEvent &evt);

	private:
		void DrawControlInternal(DrawSurface *surface, bool inverted);

		PascalStr<255> m_text;
		bool m_haveMouseDown;
	};
}
