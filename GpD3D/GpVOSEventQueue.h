#pragma once

#include <stdint.h>

#include "HostVOSEventQueue.h"
#include "GpVOSEvent.h"

class GpVOSEventQueue final : public PortabilityLayer::HostVOSEventQueue
{
public:
	GpVOSEventQueue();
	~GpVOSEventQueue();

	const GpVOSEvent *GetNext() override;
	void DischargeOne() override;

	GpVOSEvent *QueueEvent();

private:
	static const size_t kMaxEvents = 10000;

	GpVOSEvent m_events[kMaxEvents];
	size_t m_firstEvent;
	size_t m_numEventsQueued;
};
