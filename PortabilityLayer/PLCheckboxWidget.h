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

		bool Init(const WidgetBasicState &state) override;

		void DrawControl(DrawSurface *surface) override;
		void SetString(const PLPasStr &str) override;
		PLPasStr GetString() const override;

		void OnStateChanged() override;

		WidgetHandleState_t ProcessEvent(const TimeTaggedVOSEvent &evt);

	private:
		PascalStr<255> m_text;
		bool m_haveMouseDown;
	};
}
