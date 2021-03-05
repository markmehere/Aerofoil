#pragma once

#include "IGpInputDriver.h"
#include "GpInputDriverProperties.h"
#include "GpVOSEvent.h"

union SDL_Event;

struct IGpInputDriverSDLGamepad : public IGpInputDriver
{
	virtual void ProcessSDLEvent(const SDL_Event &evt) = 0;

	static IGpInputDriverSDLGamepad *GetInstance();
};
