#pragma once

#include <stdint.h>

#include "IGpVOSEventQueue.h"
#include "GpVOSEvent.h"

class GpVOSEventQueue final : public IGpVOSEventQueue
{
public:
	GpVOSEventQueue();
	~GpVOSEventQueue();

	const GpVOSEvent *GetNext() override;
	void DischargeOne() override;

	GpVOSEvent *QueueEvent() override;

private:
	static const size_t kMaxEvents = 10000;

	GpVOSEvent m_events[kMaxEvents];
	size_t m_firstEvent;
	size_t m_numEventsQueued;
};
