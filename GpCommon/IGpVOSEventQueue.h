#pragma once

struct GpVOSEvent;

struct IGpVOSEventQueue
{
	virtual const GpVOSEvent *GetNext() = 0;
	virtual void DischargeOne() = 0;

	virtual GpVOSEvent *QueueEvent() = 0;
};
