#include "PLWidgets.h"
#include "MemoryManager.h"

namespace PortabilityLayer
{
	WidgetBasicState::WidgetBasicState()
		: m_resID(0)
		, m_rect(Rect::Create(0, 0, 0, 0))
		, m_enabled(true)
		, m_window(nullptr)
	{
	}

	WidgetHandleState_t Widget::ProcessEvent(const TimeTaggedVOSEvent &evt)
	{
		(void)evt;

		return WidgetHandleStates::kIgnored;
	}

	void Widget::DrawControl(DrawSurface *surface)
	{
		(void)surface;
	}

	void Widget::SetEnabled(bool enabled)
	{
		m_enabled = enabled;
		OnEnabledChanged();
	}

	void Widget::SetState(int16_t state)
	{
		m_state = state;
		OnStateChanged();
	}

	void Widget::SetVisible(bool visible)
	{
		m_visible = visible;
	}

	bool Widget::IsVisible() const
	{
		return m_visible;
	}

	void Widget::SetHighlightStyle(int16_t style)
	{
		(void)style;
	}

	void Widget::SetString(const PLPasStr &str)
	{
		(void)str;
	}

	const Rect &Widget::GetRect() const
	{
		return m_rect;
	}

	Widget::Widget(const WidgetBasicState &state)
		: m_rect(state.m_rect)
		, m_window(state.m_window)
		, m_enabled(state.m_enabled)
		, m_visible(true)
		, m_state(0)
	{
	}

	Widget::~Widget()
	{
	}

	void Widget::OnEnabledChanged()
	{
	}

	void Widget::OnStateChanged()
	{
	}

	void Widget::BaseRelease(void *storage)
	{
		PortabilityLayer::MemoryManager::GetInstance()->Release(storage);
	}

	void *Widget::BaseAlloc(size_t sz)
	{
		return PortabilityLayer::MemoryManager::GetInstance()->Alloc(sz);
	}
}
