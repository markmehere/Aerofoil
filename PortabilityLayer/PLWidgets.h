#pragma once

#include "PascalStr.h"
#include "SharedTypes.h"

struct DrawSurface;
class PLPasStr;
struct TimeTaggedVOSEvent;
struct Window;

namespace PortabilityLayer
{
	namespace WidgetHandleStates
	{
		enum WidgetHandleState
		{
			kIgnored,	// Event was ignored
			kDigested,	// Event was digested by the control
			kCaptured,	// Event was digested by the control and only this control should receive events until it returns a different result
			kActivated,	// Event caused the control to activate
		};
	}

	typedef WidgetHandleStates::WidgetHandleState WidgetHandleState_t;

	struct WidgetBasicState
	{
		WidgetBasicState();

		Rect m_rect;
		PascalStr<255> m_text;
		Window *m_window;
		uint32_t m_refConstant;
		int32_t m_min;
		int32_t m_max;
		int16_t m_state;
		int16_t m_resID;
		bool m_enabled;
	};

	class Widget
	{
	public:
		virtual bool Init(const WidgetBasicState &state) = 0;
		virtual void Destroy() = 0;
		virtual WidgetHandleState_t ProcessEvent(const TimeTaggedVOSEvent &evt);
		virtual void DrawControl(DrawSurface *surface);

		virtual void SetMin(int32_t v);
		virtual void SetMax(int32_t v);

		void SetPosition(const Point &pos);
		void Resize(uint16_t width, uint16_t height);

		void SetEnabled(bool enabled);
		void SetState(int16_t state);
		int16_t GetState() const;

		void SetVisible(bool visible);
		bool IsVisible() const;

		virtual void SetString(const PLPasStr &str);
		virtual PLPasStr GetString() const;

		virtual void SetHighlightStyle(int16_t style);

		const Rect &GetRect() const;

	protected:
		explicit Widget(const WidgetBasicState &state);
		virtual ~Widget();

		virtual void OnEnabledChanged();
		virtual void OnStateChanged();

		static void BaseRelease(void *storage);
		static void *BaseAlloc(size_t sz);

		Window *m_window;
		Rect m_rect;
		int16_t m_state;
		bool m_enabled;
		bool m_visible;
	};
}

#include "PLCore.h"
#include <new>
#include <stdint.h>

namespace PortabilityLayer
{
	template<class T>
	class WidgetSpec : public Widget
	{
	public:
		explicit WidgetSpec(const WidgetBasicState &state)
			: Widget(state)
		{
		}

		void Destroy() override
		{
			static_cast<T*>(this)->~T();
			Widget::BaseRelease(static_cast<T*>(this));
		}

		static T *Create(const WidgetBasicState &state)
		{
			void *storage = Widget::BaseAlloc(sizeof(T));
			if (!storage)
				return nullptr;

			T *widgetT = new (storage) T(state);

			// Conversion check
			WidgetSpec<T> *downcastWidget = widgetT;
			(void)downcastWidget;

			Widget *widget = widgetT;
			if (!widget->Init(state))
			{
				widget->Destroy();
				return nullptr;
			}

			if (!static_cast<Window*>(state.m_window)->AddWidget(widget))
			{
				widget->Destroy();
				return nullptr;
			}

			return widgetT;
		}
	};
}
