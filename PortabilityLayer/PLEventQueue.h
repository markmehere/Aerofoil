#pragma once

#include "PLCore.h"

#include <stdint.h>

struct TimeTaggedVOSEvent;

namespace PortabilityLayer
{
	class EventQueue
	{
	public:
		virtual bool Dequeue(TimeTaggedVOSEvent *evt) = 0;
		virtual const TimeTaggedVOSEvent *Peek() const = 0;
		virtual TimeTaggedVOSEvent *Enqueue() = 0;

		static EventQueue *GetInstance();
	};
}
