#include "PLSound.h"

#include "ClientAudioChannelContext.h"
#include "HostAudioChannel.h"
#include "HostAudioDriver.h"
#include "MemoryManager.h"
#include "HostMutex.h"
#include "HostSystemServices.h"
#include "HostThreadEvent.h"

#include <assert.h>

namespace PortabilityLayer
{
	class AudioChannelImpl : public SndChannel, public ClientAudioChannelContext
	{
	public:
		explicit AudioChannelImpl(PortabilityLayer::HostAudioChannel *channel, SndCallBackUPP callback, HostThreadEvent *threadEvent, HostMutex *mutex);
		~AudioChannelImpl();

		bool PushCommand(const SndCommand &command, bool blocking);

		void NotifyBufferFinished() override;

	private:
		enum WorkingState
		{
			State_Idle,				// No thread is playing sound, the sound thread is out of work
			State_PlayingAsync,		// Sound thread is playing sound.  When it finishes, it will digest queue events under a lock.
			State_PlayingBlocked,	// Sound thread is playing sound.  When it finishes, it will transition to Idle and fire the thread event.
			State_FlushStarting,	// Sound thread is aborting.  When it aborts, it will transition to Idle and fire the thread event.
		};

		static const unsigned int kMaxQueuedCommands = 64;

		void DigestQueueItems();
		void DigestBufferCommand(const void *dataPointer);

		PortabilityLayer::HostAudioChannel *m_audioChannel;
		SndCallBackUPP m_callback;

		HostMutex *m_mutex;
		HostThreadEvent *m_threadEvent;

		SndCommand m_commandQueue[kMaxQueuedCommands];
		size_t m_numQueuedCommands;
		size_t m_nextInsertCommandPos;
		size_t m_nextDequeueCommandPos;
		WorkingState m_state;
		bool m_isIdle;
	};

	AudioChannelImpl::AudioChannelImpl(PortabilityLayer::HostAudioChannel *channel, SndCallBackUPP callback, HostThreadEvent *threadEvent, HostMutex *mutex)
		: m_audioChannel(channel)
		, m_callback(callback)
		, m_threadEvent(threadEvent)
		, m_mutex(mutex)
		, m_nextInsertCommandPos(0)
		, m_nextDequeueCommandPos(0)
		, m_numQueuedCommands(0)
		, m_state(State_Idle)
		, m_isIdle(false)
	{
		m_audioChannel->SetClientAudioChannelContext(this);
	}

	AudioChannelImpl::~AudioChannelImpl()
	{
		m_mutex->Destroy();
		m_threadEvent->Destroy();
	}

	void AudioChannelImpl::NotifyBufferFinished()
	{
		m_mutex->Lock();

		if (m_state == State_PlayingAsync)
		{
			m_state = State_Idle;
			DigestQueueItems();
		}
		else if (m_state == State_PlayingBlocked || m_state == State_FlushStarting)
		{
			m_state = State_Idle;
			m_threadEvent->Signal();
		}

		m_mutex->Unlock();
	}

	void AudioChannelImpl::DigestQueueItems()
	{
		m_mutex->Lock();

		assert(m_state == State_Idle);

		while (m_numQueuedCommands > 0)
		{
			const SndCommand &command = m_commandQueue[m_nextDequeueCommandPos];
			m_numQueuedCommands--;
			m_nextDequeueCommandPos = (m_nextDequeueCommandPos + 1) % static_cast<size_t>(kMaxQueuedCommands);

			switch (command.cmd)
			{
			case nullCmd:
				break;
			case bufferCmd:
				DigestBufferCommand(reinterpret_cast<const void*>(command.param2));
				assert(m_state == State_PlayingAsync);
				m_mutex->Unlock();
				return;
			case callBackCmd:
				{
					SndCommand commandCopy = command;
					m_callback(this, &commandCopy);
				}
				break;
			default:
			case flushCmd:
			case quietCmd:
				assert(false);	// These shouldn't be in the queue
				break;
			}
		}
		m_mutex->Unlock();
	}

	void AudioChannelImpl::DigestBufferCommand(const void *dataPointer)
	{
		struct BufferHeader
		{
			BEUInt32_t m_samplePtr;
			BEUInt32_t m_length;
			BEFixed32_t m_sampleRate;
			BEUInt32_t m_loopStart;
			BEUInt32_t m_loopEnd;
			uint8_t m_encoding;
			uint8_t m_baseFrequency;
		};

		BufferHeader bufferHeader;

		PL_STATIC_ASSERT(sizeof(BufferHeader) >= 22);

		memcpy(&bufferHeader, dataPointer, 22);

		const uint32_t length = bufferHeader.m_length;

		m_audioChannel->PostBuffer(static_cast<const uint8_t*>(dataPointer) + 22, length);
		m_state = State_PlayingAsync;
	}

	bool AudioChannelImpl::PushCommand(const SndCommand &command, bool failIfFull)
	{
		bool digestOnThisThread = false;

		m_mutex->Lock();
		if (m_numQueuedCommands == kMaxQueuedCommands)
		{
			if (failIfFull)
			{
				m_mutex->Unlock();
				return false;
			}
			else
			{
				assert(m_state == State_PlayingAsync);

				if (m_numQueuedCommands == kMaxQueuedCommands)
				{
					m_state = State_PlayingBlocked;
					m_mutex->Unlock();

					m_threadEvent->Wait();

					m_mutex->Lock();

					assert(m_state == State_Idle);
					digestOnThisThread = true;
				}
			}
		}
		else
		{
			if (m_state == State_Idle)
				digestOnThisThread = true;
		}

		m_commandQueue[m_nextInsertCommandPos] = command;
		m_nextInsertCommandPos = (m_nextInsertCommandPos + 1) % static_cast<size_t>(kMaxQueuedCommands);
		m_numQueuedCommands++;
		m_mutex->Unlock();

		if (digestOnThisThread)
			DigestQueueItems();

		return true;
	}
}

OSErr GetDefaultOutputVolume(long *vol)
{
	short leftVol = 0x100;
	short rightVol = 0x100;

	PL_NotYetImplemented_Minor();

	*vol = (leftVol) | (rightVol << 16);

	return noErr;
}

OSErr SetDefaultOutputVolume(long vol)
{
	return noErr;
}


SndCallBackUPP NewSndCallBackProc(SndCallBackProc callback)
{
	return callback;
}

void DisposeSndCallBackUPP(SndCallBackUPP upp)
{
}

OSErr SndNewChannel(SndChannelPtr *outChannel, SndSynthType synthType, int initFlags, SndCallBackUPP callback)
{
	PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();
	void *storage = mm->Alloc(sizeof(PortabilityLayer::AudioChannelImpl));
	if (!storage)
		return mFulErr;

	PortabilityLayer::HostAudioDriver *audioDriver = PortabilityLayer::HostAudioDriver::GetInstance();
	PortabilityLayer::HostAudioChannel *audioChannel = audioDriver->CreateChannel();
	if (!audioChannel)
	{
		mm->Release(storage);
		return genericErr;
	}

	PortabilityLayer::HostMutex *mutex = PortabilityLayer::HostSystemServices::GetInstance()->CreateMutex();
	if (!mutex)
	{
		audioChannel->Destroy();
		mm->Release(storage);
		return genericErr;
	}

	PortabilityLayer::HostThreadEvent *threadEvent = PortabilityLayer::HostSystemServices::GetInstance()->CreateThreadEvent(true, false);
	if (!threadEvent)
	{
		mutex->Destroy();
		audioChannel->Destroy();
		mm->Release(storage);
		return genericErr;
	}

	*outChannel = new (storage) PortabilityLayer::AudioChannelImpl(audioChannel, callback, threadEvent, mutex);
	
	return noErr;
}

OSErr SndDisposeChannel(SndChannelPtr channel, Boolean flush)
{
	if (flush)
	{
		SndCommand cmd;
		cmd.cmd = flushCmd;
		cmd.param1 = cmd.param2 = 0;

		SndDoImmediate(channel, &cmd);

		cmd.cmd = quietCmd;
		cmd.param1 = cmd.param2 = 0;
		SndDoImmediate(channel, &cmd);
	}

	PortabilityLayer::AudioChannelImpl *audioChannelImpl = static_cast<PortabilityLayer::AudioChannelImpl*>(channel);
	audioChannelImpl->~AudioChannelImpl();

	PortabilityLayer::MemoryManager::GetInstance()->Release(audioChannelImpl);

	return noErr;
}

OSErr SndDoCommand(SndChannelPtr channel, const SndCommand *command, Boolean failIfFull)
{
	PortabilityLayer::AudioChannelImpl *audioChannelImpl = static_cast<PortabilityLayer::AudioChannelImpl*>(channel);

	if (!audioChannelImpl->PushCommand(*command, failIfFull == 0))
		return queueFull;

	return noErr;
}

OSErr SndDoImmediate(SndChannelPtr channel, const SndCommand *command)
{
	PL_NotYetImplemented();
	return noErr;
}

