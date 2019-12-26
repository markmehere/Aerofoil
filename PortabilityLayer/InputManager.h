#pragma once

struct GpKeyboardInputEvent;
struct KeyMap;

namespace PortabilityLayer
{
	class InputManager
	{
	public:
		virtual void GetKeys(KeyMap &keys16) const = 0;
		virtual void ApplyEvent(const GpKeyboardInputEvent &vosEvent) = 0;

		static InputManager *GetInstance();
	};
}
