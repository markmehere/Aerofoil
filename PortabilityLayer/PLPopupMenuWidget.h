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

		bool Init(const WidgetBasicState &state, const void *additionalData) override;

		WidgetHandleState_t ProcessEvent(void *captureContext, const TimeTaggedVOSEvent &evt) override;
		int16_t Capture(void *captureContext, const Point &pos, WidgetUpdateCallback_t callback) override;
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
