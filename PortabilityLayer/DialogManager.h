#pragma once

#include <stdint.h>

struct Dialog;
struct Window;

namespace PortabilityLayer
{
	class DialogManager
	{
	public:
		virtual Dialog *LoadDialog(int16_t resID, Window *behindWindow) = 0;

		static DialogManager *GetInstance();
	};
}
