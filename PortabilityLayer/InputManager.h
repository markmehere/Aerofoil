#pragma once

#include "GpVOSEvent.h"

struct GpGamepadInputEvent;
struct GpKeyboardInputEvent;
struct GpMouseInputEvent;
struct KeyDownStates;

namespace PortabilityLayer
{
	class InputManager
	{
	public:
		virtual const KeyDownStates *GetKeys() const = 0;
		virtual void ApplyKeyboardEvent(const GpKeyboardInputEvent &vosEvent) = 0;
		virtual void ApplyGamepadEvent(const GpGamepadInputEvent &vosEvent) = 0;
		virtual void ApplyMouseEvent(const GpMouseInputEvent &vosEvent) = 0;
		virtual int16_t GetGamepadAxis(unsigned int playerNum, GpGamepadAxis_t gamepadAxis) = 0;
		virtual void ClearState() = 0;

		static InputManager *GetInstance();
	};
}
