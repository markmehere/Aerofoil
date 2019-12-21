#include "PLEventQueue.h"

#include <stdint.h>

namespace PortabilityLayer
{
	class EventQueueImpl final : public EventQueue
	{
	public:
		EventQueueImpl();
		~EventQueueImpl();

		bool Dequeue(EventRecord *evt) override;
		EventRecord *Enqueue() override;

		static EventQueueImpl *GetInstance();

	private:
		static const size_t kMaxEvents = 10000;

		EventRecord m_events[kMaxEvents];
		size_t m_firstEvent;
		size_t m_numQueuedEvents;

		static EventQueueImpl ms_instance;
	};

	EventQueueImpl::EventQueueImpl()
		: m_firstEvent(0)
		, m_numQueuedEvents(0)
	{
	}

	EventQueueImpl::~EventQueueImpl()
	{
	}

	bool EventQueueImpl::Dequeue(EventRecord *evt)
	{
		if (m_numQueuedEvents == 0)
			return false;

		*evt = m_events[m_firstEvent];

		m_firstEvent++;
		if (m_firstEvent == kMaxEvents)
			m_firstEvent = 0;

		m_numQueuedEvents--;

		return true;
	}

	EventRecord *EventQueueImpl::Enqueue()
	{
		if (m_numQueuedEvents == kMaxEvents)
			return nullptr;

		size_t nextEvent = m_firstEvent + m_numQueuedEvents;
		if (nextEvent >= kMaxEvents)
			nextEvent -= kMaxEvents;

		m_numQueuedEvents++;

		EventRecord *evt = m_events + nextEvent;
		memset(evt, 0, sizeof(EventRecord));

		return evt;
	}

	EventQueueImpl *EventQueueImpl::GetInstance()
	{
		return &ms_instance;
	}

	EventQueueImpl EventQueueImpl::ms_instance;

	EventQueue *EventQueue::GetInstance()
	{
		return EventQueueImpl::GetInstance();
	}
}
