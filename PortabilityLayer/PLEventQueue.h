#pragma once

#include "PLCore.h"

#include <stdint.h>

namespace PortabilityLayer
{
	class EventQueue
	{
	public:
		virtual bool Dequeue(EventRecord *evt) = 0;
		virtual EventRecord *Enqueue() = 0;

		static EventQueue *GetInstance();
	};
}
