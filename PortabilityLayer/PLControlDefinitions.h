#pragma once
#ifndef __PL_CONTROLDEFINITIONS_H__
#define __PL_CONTROLDEFINITIONS_H__

#include "PLCore.h"

namespace PortabilityLayer
{
	class Widget;
}

struct Control
{
};

typedef THandle<Control> ControlHandle;

typedef void(*ControlActionProc)(ControlHandle control, short part);
typedef ControlActionProc ControlActionUPP;

enum ControlParts
{
	kControlUpButtonPart = 1,
	kControlDownButtonPart,
	kControlPageUpPart,
	kControlPageDownPart,
	kControlIndicatorPart,
	kControlButtonPart,
};


int FindControl(Point point, WindowPtr window, PortabilityLayer::Widget **outControl);	// Returns part

#endif
