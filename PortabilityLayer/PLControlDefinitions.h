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
int FindControl(Point point, WindowPtr window, ControlHandle *outControl);


void SetControlValue(ControlHandle control, int value);
void SetControlMaximum(ControlHandle control, int value);
void MoveControl(ControlHandle control, int x, int y);
void SizeControl(ControlHandle control, int width, int height);
int GetControlValue(ControlHandle control);
ControlHandle NewControl(WindowPtr window, const Rect *rect, const PLPasStr &label, Boolean visible, int value, int minValue, int maxValue, int cdef, long userdata);
ControlActionUPP NewControlActionUPP(ControlActionProc proc);
void DisposeControlActionUPP(ControlActionUPP upp);
Boolean TrackControl(ControlHandle control, Point point, ControlActionUPP proc);
Boolean TrackControl(PortabilityLayer::Widget *control, Point point, ControlActionUPP proc);
long GetControlReference(ControlHandle control);	// Returns userdata
ControlHandle GetNewControl(int resID, WindowPtr window);
void HiliteControl(ControlHandle control, int unknown);
void HiliteControl(PortabilityLayer::Widget *widget, int unknown);

#endif
