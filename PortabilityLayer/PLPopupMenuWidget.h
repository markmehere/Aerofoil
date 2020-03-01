#pragma once

#include "PascalStr.h"
#include "PLWidgets.h"

struct Menu;

namespace PortabilityLayer
{
	class PopupMenuWidget final : public WidgetSpec<PopupMenuWidget>
	{
	public:
		explicit PopupMenuWidget(const WidgetBasicState &state);

		bool Init(const WidgetBasicState &state) override;

		WidgetHandleState_t ProcessEvent(const TimeTaggedVOSEvent &evt);
		int16_t Capture(const Point &pos, WidgetUpdateCallback_t callback);
		void DrawControl(DrawSurface *surface) override;

		void OnStateChanged() override;

		PLPasStr GetString() const override;

		const THandle<Menu> &GetMenu() const;

	protected:
		~PopupMenuWidget();

	private:
		THandle<Menu> m_menu;
	};
}
