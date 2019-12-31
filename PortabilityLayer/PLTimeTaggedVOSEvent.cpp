#include "PLTimeTaggedVOSEvent.h"

// Helpers for common cases
bool TimeTaggedVOSEvent::IsKeyDownEvent() const
{
	if (m_vosEvent.m_eventType != GpVOSEventTypes::kKeyboardInput)
		return false;

	const GpKeyboardInputEvent &keyboardEvent = m_vosEvent.m_event.m_keyboardInputEvent;

	return keyboardEvent.m_eventType == GpKeyboardInputEventTypes::kDown;
}

bool TimeTaggedVOSEvent::IsLMouseDownEvent() const
{
	if (m_vosEvent.m_eventType != GpVOSEventTypes::kMouseInput)
		return false;

	const GpMouseInputEvent &mouseEvent = m_vosEvent.m_event.m_mouseInputEvent;

	return mouseEvent.m_eventType == GpMouseEventTypes::kDown && mouseEvent.m_button == GpMouseButtons::kLeft;
}
