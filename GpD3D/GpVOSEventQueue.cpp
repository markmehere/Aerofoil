#include "GpVOSEventQueue.h"

#include <assert.h>

GpVOSEventQueue::GpVOSEventQueue()
	: m_firstEvent(0)
	, m_numEventsQueued(0)
{
}

GpVOSEventQueue::~GpVOSEventQueue()
{
}

const GpVOSEvent *GpVOSEventQueue::GetNext()
{
	if (m_numEventsQueued)
		return m_events + m_firstEvent;

	return nullptr;
}

void GpVOSEventQueue::DischargeOne()
{
	assert(m_numEventsQueued > 0);

	m_numEventsQueued--;
	m_firstEvent++;
	if (m_firstEvent == kMaxEvents)
		m_firstEvent = 0;
}

GpVOSEvent *GpVOSEventQueue::QueueEvent()
{
	if (m_numEventsQueued == kMaxEvents)
		return nullptr;

	size_t nextEvent = m_firstEvent + m_numEventsQueued;
	if (nextEvent >= kMaxEvents)
		nextEvent -= kMaxEvents;

	m_numEventsQueued++;

	return m_events + nextEvent;
}
