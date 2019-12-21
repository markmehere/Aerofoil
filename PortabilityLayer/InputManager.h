#pragma once

namespace PortabilityLayer
{
	class InputManager
	{
	public:
		virtual void GetKeys(unsigned char *keys16) const = 0;

		static InputManager *GetInstance();
	};
}
