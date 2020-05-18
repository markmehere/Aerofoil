#pragma once

#include "PLWidgets.h"
#include "PLHandle.h"

namespace PortabilityLayer
{
	class PixMapImpl;

	class IconWidget final : public WidgetSpec<IconWidget>
	{
	public:
		IconWidget(const WidgetBasicState &state);
		~IconWidget() override;

		bool Init(const WidgetBasicState &state, const void *additionalData) override;

		void DrawControl(DrawSurface *surface) override;

		WidgetHandleState_t ProcessEvent(const TimeTaggedVOSEvent &evt) override;

	private:
		THandle<PixMapImpl> m_iconImage;
		THandle<PixMapImpl> m_iconMask;
	};
}
