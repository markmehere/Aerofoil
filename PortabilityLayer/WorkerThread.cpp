#include "WorkerThread.h"
#include "IGpThreadEvent.h"
#include "IGpSystemServices.h"

#include "PLDrivers.h"

#include <stdlib.h>
#include <new>

namespace PortabilityLayer
{
	class WorkerThreadImpl final : public WorkerThread
	{
	public:
		WorkerThreadImpl();

		bool Init();
		void Destroy() override;

		void AsyncExecuteTask(Callback_t callback, void *context) override;

	private:
		~WorkerThreadImpl() override;

		static int StaticThreadFuncThunk(void *context);
		int ThreadFunc();

		IGpThreadEvent *m_wakeSignal;
		IGpThreadEvent *m_wakeConsumedSignal;

		Callback_t m_waitingCallback;
		void *m_waitingContext;

		bool m_terminated;
		bool m_threadRunning;
	};
}

void PortabilityLayer::WorkerThreadImpl::Destroy()
{
	this->~WorkerThreadImpl();
	free(this);
}

void PortabilityLayer::WorkerThreadImpl::AsyncExecuteTask(PortabilityLayer::WorkerThread::Callback_t callback, void *context)
{
	m_waitingCallback = callback;
	m_waitingContext = context;
	m_wakeSignal->Signal();
	m_wakeConsumedSignal->Wait();
}

PortabilityLayer::WorkerThreadImpl::WorkerThreadImpl()
	: m_wakeSignal(nullptr)
	, m_wakeConsumedSignal(nullptr)
	, m_waitingCallback(nullptr)
	, m_waitingContext(nullptr)
	, m_terminated(false)
	, m_threadRunning(false)
{
}

PortabilityLayer::WorkerThreadImpl::~WorkerThreadImpl()
{
	if (m_threadRunning)
	{
		m_terminated = true;
		m_wakeSignal->Signal();
		m_wakeConsumedSignal->Wait();
	}

	if (m_wakeSignal)
		m_wakeSignal->Destroy();
	if (m_wakeConsumedSignal)
		m_wakeConsumedSignal->Destroy();
}


int PortabilityLayer::WorkerThreadImpl::StaticThreadFuncThunk(void *context)
{
	return static_cast<PortabilityLayer::WorkerThreadImpl*>(context)->ThreadFunc();
}

int PortabilityLayer::WorkerThreadImpl::ThreadFunc()
{
	m_wakeSignal->Wait();
	m_wakeConsumedSignal->Signal();

	for (;;)
	{
		m_wakeSignal->Wait();

		if (m_terminated)
		{
			m_wakeConsumedSignal->Signal();
			return 0;
		}
		else
		{
			Callback_t callback = m_waitingCallback;
			void *context = m_waitingContext;
			m_wakeConsumedSignal->Signal();

			callback(context);
		}
	}
}

bool PortabilityLayer::WorkerThreadImpl::Init()
{
	IGpSystemServices *sysServices = PLDrivers::GetSystemServices();

	m_wakeSignal = sysServices->CreateThreadEvent(true, false);
	if (!m_wakeSignal)
		return false;

	m_wakeConsumedSignal = sysServices->CreateThreadEvent(true, false);
	if (!m_wakeConsumedSignal)
		return false;

	if (!sysServices->CreateThread(PortabilityLayer::WorkerThreadImpl::StaticThreadFuncThunk, this))
		return false;

	m_threadRunning = true;
	m_wakeSignal->Signal();
	m_wakeConsumedSignal->Wait();

	return true;
}


PortabilityLayer::WorkerThread::WorkerThread()
{
}

PortabilityLayer::WorkerThread::~WorkerThread()
{
}

PortabilityLayer::WorkerThread *PortabilityLayer::WorkerThread::Create()
{
	void *storage = malloc(sizeof(PortabilityLayer::WorkerThreadImpl));
	if (!storage)
		return nullptr;

	PortabilityLayer::WorkerThreadImpl *thread = new (storage) PortabilityLayer::WorkerThreadImpl();
	if (!thread->Init())
	{
		thread->Destroy();
		return nullptr;
	}

	return thread;
}
