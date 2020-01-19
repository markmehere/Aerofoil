#pragma once

#include "PLWidgets.h"
#include "PLHandle.h"

struct BitmapImage;

namespace PortabilityLayer
{
	class ImageWidget final : public WidgetSpec<ImageWidget>
	{
	public:
		ImageWidget(const WidgetBasicState &state);
		~ImageWidget();

		bool Init(const WidgetBasicState &state) override;
		void DrawControl(DrawSurface *surface) override;

	private:
		THandle<BitmapImage> m_pict;
	};
}
