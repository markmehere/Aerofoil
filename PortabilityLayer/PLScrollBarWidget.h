#pragma once

#include "PascalStr.h"
#include "PLWidgets.h"

struct Rect;

namespace PortabilityLayer
{
	class ScrollBarWidget final : public WidgetSpec<ScrollBarWidget>
	{
	public:
		explicit ScrollBarWidget(const WidgetBasicState &state);

		bool Init(const WidgetBasicState &state) override;

		void OnEnabledChanged() override;
		WidgetHandleState_t ProcessEvent(const TimeTaggedVOSEvent &evt) override;
		void DrawControl(DrawSurface *surface) override;

		void SetState(int16_t state) override;
		void OnStateChanged() override;

		int16_t Capture(const Point &pos, WidgetUpdateCallback_t callback) override;

		int ResolvePart(const Point &point) const override;

	private:
		bool IsHorizontal() const;
		bool Isvertical() const;

		void DrawControlHorizontal(DrawSurface *surface);
		void DrawControlVertical(DrawSurface *surface);

		void RefreshGrip();

		static void DrawBeveledBox(DrawSurface *surface, const Rect &rect);

		int16_t CaptureScrollSegment(const Point &pos, int part, WidgetUpdateCallback_t callback);
		int16_t CaptureIndicator(const Point &pos, WidgetUpdateCallback_t callback);
		void IterateScrollSegment(int part, WidgetUpdateCallback_t callback);

		int32_t m_min;
		int32_t m_max;
		int32_t m_gripSize;
		int32_t m_gripPos;
		int32_t m_laneCapacity;
	};
}
