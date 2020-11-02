#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	class HostThreadEvent;

	class WorkerThread
	{
	public:
		typedef void(*Callback_t)(void *context);

		static WorkerThread *Create();
		virtual void Destroy() = 0;

		virtual void AsyncExecuteTask(Callback_t callback, void *context) = 0;

	protected:
		WorkerThread();
		virtual ~WorkerThread();
	};
}
