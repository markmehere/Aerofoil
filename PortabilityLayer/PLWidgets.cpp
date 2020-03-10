#include "PLWidgets.h"
#include "MemoryManager.h"
#include "PLControlDefinitions.h"

namespace PortabilityLayer
{
	WidgetBasicState::WidgetBasicState()
		: m_resID(0)
		, m_rect(Rect::Create(0, 0, 0, 0))
		, m_window(nullptr)
		, m_refConstant(0)
		, m_min(0)
		, m_max(0)
		, m_state(0)
		, m_enabled(true)
	{
	}

	WidgetHandleState_t Widget::ProcessEvent(const TimeTaggedVOSEvent &evt)
	{
		(void)evt;

		return WidgetHandleStates::kIgnored;
	}

	int16_t Widget::Capture(const Point &pos, WidgetUpdateCallback_t callback)
	{
		return 0;
	}

	void Widget::DrawControl(DrawSurface *surface)
	{
		(void)surface;
	}

	void Widget::SetMin(int32_t v)
	{
	}

	void Widget::SetMax(int32_t v)
	{
	}

	void Widget::SetPosition(const Point &pos)
	{
		uint16_t width = m_rect.Width();
		uint16_t height = m_rect.Height();

		m_rect.left = pos.h;
		m_rect.top = pos.v;

		Resize(width, height);
	}

	void Widget::Resize(uint16_t width, uint16_t height)
	{
		m_rect.right = m_rect.left + width;
		m_rect.bottom = m_rect.top + height;
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

	int16_t Widget::GetState() const
	{
		return m_state;
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

	bool Widget::HandlesTickEvents() const
	{
		return false;
	}

	Rect Widget::GetExpandedRect() const
	{
		return GetRect();
	}

	int Widget::ResolvePart(const Point &point) const
	{
		return kControlButtonPart;
	}

	void Widget::GainFocus()
	{
	}

	void Widget::LoseFocus()
	{
	}

	void Widget::SetString(const PLPasStr &str)
	{
		(void)str;
	}

	PLPasStr Widget::GetString() const
	{
		return PSTR("");
	}

	uint32_t Widget::GetReferenceConstant() const
	{
		return m_referenceConstant;
	}

	const Rect &Widget::GetRect() const
	{
		return m_rect;
	}

	Window *Widget::GetWindow() const
	{
		return m_window;
	}

	Widget::Widget(const WidgetBasicState &state)
		: m_rect(state.m_rect)
		, m_window(state.m_window)
		, m_enabled(state.m_enabled)
		, m_referenceConstant(state.m_refConstant)
		, m_state(state.m_state)
		, m_visible(true)
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

	void Widget::OnTick()
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
