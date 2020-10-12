#include "PLSound.h"

#include "HostAudioDriver.h"
#include "MemoryManager.h"
#include "HostMutex.h"
#include "HostSystemServices.h"
#include "HostThreadEvent.h"
#include "IGpAudioChannel.h"
#include "IGpAudioChannelCallbacks.h"
#include "IGpAudioDriver.h"
#include "WaveFormat.h"

#include <assert.h>

namespace PortabilityLayer
{
	namespace AudioCommandTypes
	{
		enum AudioCommandType
		{
			kBuffer,
			kCallback,
		};
	}

	typedef AudioCommandTypes::AudioCommandType AudioCommandType_t;

	struct AudioCommand
	{
		union AudioCommandParam
		{
			const void *m_ptr;
			AudioChannelCallback_t m_callback;
		};

		AudioCommandType_t m_commandType;
		AudioCommandParam m_param;
	};

	class AudioChannelImpl final : public AudioChannel, public IGpAudioChannelCallbacks
	{
	public:
		explicit AudioChannelImpl(IGpAudioChannel *channel, HostThreadEvent *threadEvent, HostMutex *mutex);
		~AudioChannelImpl();

		void Destroy(bool wait) override;
		bool AddBuffer(const void *lengthTaggedBuffer, bool blocking) override;
		bool AddCallback(AudioChannelCallback_t callback, bool blocking) override;
		void ClearAllCommands() override;
		void Stop() override;

		void NotifyBufferFinished() override;

	private:
		bool PushCommand(const AudioCommand &command, bool blocking);

		enum WorkingState
		{
			// NOTE: In all cases where the thread event is fired, the event may be fired under a mutex lock.
			// Therefore, if the mutex is to be destroyed, it must be locked first even though the thread has transitioned to idle.

			State_Idle,				// No thread is playing sound, the sound thread is out of work
			State_PlayingAsync,		// Sound thread is playing sound.  When it finishes, it will digest queue events under a lock.
			State_PlayingBlocked,	// Sound thread is playing sound.  When it finishes, it will transition to Idle and fire the thread event.
			State_FlushStarting,	// Sound thread is aborting.  When it aborts, it will transition to Idle and fire the thread event.
			State_ShuttingDown,		// Sound thread is shutting down.  When it finishes, it will transition to Idle and fire the thread event.
		};

		static const unsigned int kMaxQueuedCommands = 64;

		void DigestQueueItems();
		void DigestBufferCommand(const void *dataPointer);

		IGpAudioChannel *m_audioChannel;

		HostMutex *m_mutex;
		HostThreadEvent *m_threadEvent;

		AudioCommand m_commandQueue[kMaxQueuedCommands];
		size_t m_numQueuedCommands;
		size_t m_nextInsertCommandPos;
		size_t m_nextDequeueCommandPos;
		WorkingState m_state;
		bool m_isIdle;
	};

	AudioChannelImpl::AudioChannelImpl(IGpAudioChannel *channel, HostThreadEvent *threadEvent, HostMutex *mutex)
		: m_audioChannel(channel)
		, m_threadEvent(threadEvent)
		, m_mutex(mutex)
		, m_nextInsertCommandPos(0)
		, m_nextDequeueCommandPos(0)
		, m_numQueuedCommands(0)
		, m_state(State_Idle)
		, m_isIdle(false)
	{
		m_audioChannel->SetAudioChannelContext(this);
	}

	AudioChannelImpl::~AudioChannelImpl()
	{
		m_mutex->Destroy();
		m_threadEvent->Destroy();
		m_audioChannel->Destroy();
	}

	void AudioChannelImpl::NotifyBufferFinished()
	{
		m_mutex->Lock();

		if (m_state == State_PlayingAsync)
		{
			m_state = State_Idle;
			DigestQueueItems();
		}
		else if (m_state == State_PlayingBlocked || m_state == State_FlushStarting || m_state == State_ShuttingDown)
		{
			m_state = State_Idle;
			m_threadEvent->Signal();
		}

		m_mutex->Unlock();
	}

	void AudioChannelImpl::Destroy(bool wait)
	{
		ClearAllCommands();

		bool synchronizeMutex = false;

		if (!wait)
		{
			Stop();
			synchronizeMutex = true;
		}
		else
		{
			m_mutex->Lock();

			if (m_state == State_PlayingAsync)
			{
				m_state = State_ShuttingDown;
				m_mutex->Unlock();

				m_threadEvent->Wait();

				synchronizeMutex = true;
			}
			else
			{
				assert(m_state == State_Idle);
				m_mutex->Unlock();
			}
		}

		if (synchronizeMutex)
		{
			// If we had to stop the audio thread, then the event was fired under a lock.
			// We have to wait for the unlock to complete before destroying the mutex.
			m_mutex->Lock();
			assert(m_state == State_Idle);
			m_mutex->Unlock();
		}

		this->~AudioChannelImpl();
		PortabilityLayer::MemoryManager::GetInstance()->Release(this);
	}

	bool AudioChannelImpl::AddBuffer(const void *lengthTaggedBuffer, bool blocking)
	{
		AudioCommand cmd;
		cmd.m_commandType = AudioCommandTypes::kBuffer;
		cmd.m_param.m_ptr = lengthTaggedBuffer;

		return this->PushCommand(cmd, blocking);
	}

	bool AudioChannelImpl::AddCallback(AudioChannelCallback_t callback, bool blocking)
	{
		AudioCommand cmd;
		cmd.m_commandType = AudioCommandTypes::kCallback;
		cmd.m_param.m_ptr = reinterpret_cast<void*>(callback);

		return this->PushCommand(cmd, blocking);
	}

	void AudioChannelImpl::DigestQueueItems()
	{
		m_mutex->Lock();

		assert(m_state == State_Idle);

		while (m_numQueuedCommands > 0)
		{
			const AudioCommand &command = m_commandQueue[m_nextDequeueCommandPos];
			m_numQueuedCommands--;
			m_nextDequeueCommandPos = (m_nextDequeueCommandPos + 1) % static_cast<size_t>(kMaxQueuedCommands);

			switch (command.m_commandType)
			{
			case AudioCommandTypes::kBuffer:
				DigestBufferCommand(command.m_param.m_ptr);
				assert(m_state == State_PlayingAsync);
				m_mutex->Unlock();
				return;
			case AudioCommandTypes::kCallback:
				command.m_param.m_callback(this);

				if (m_state != State_Idle)
				{
					// Child call changed state (i.e. callback called PushBuffer, which triggered a digest)
					m_mutex->Unlock();
					return;
				}
				break;
			default:
				assert(false);
				break;
			}
		}
		m_mutex->Unlock();
	}

	void AudioChannelImpl::DigestBufferCommand(const void *dataPointer)
	{
		assert(m_state == State_Idle);

		// At this point, the buffer should already be validated and converted, and the data pointer should point at the data tag
		uint32_t length;
		memcpy(&length, dataPointer, 4);

		m_audioChannel->PostBuffer(static_cast<const uint8_t*>(dataPointer) + 4, length);
		m_state = State_PlayingAsync;
	}

	bool AudioChannelImpl::PushCommand(const AudioCommand &command, bool blocking)
	{
		bool digestOnThisThread = false;

		m_mutex->Lock();
		if (m_numQueuedCommands == kMaxQueuedCommands)
		{
			if (!blocking)
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

	void AudioChannelImpl::ClearAllCommands()
	{
		m_mutex->Lock();
		m_numQueuedCommands = 0;
		m_nextDequeueCommandPos = 0;
		m_nextInsertCommandPos = 0;
		m_mutex->Unlock();
	}

	void AudioChannelImpl::Stop()
	{
		m_mutex->Lock();
		if (m_state == State_Idle)
		{
			m_mutex->Unlock();
		}
		else if (m_state == State_PlayingAsync)
		{
			m_state = State_FlushStarting;
			m_audioChannel->Stop();
			m_mutex->Unlock();

			m_threadEvent->Wait();
		}
		else
		{
			m_mutex->Unlock();
			assert(false);
		}
	}

}

PLError_t GetDefaultOutputVolume(long *vol)
{
	short leftVol = 0x100;
	short rightVol = 0x100;

	PL_NotYetImplemented_Minor();

	*vol = (leftVol) | (rightVol << 16);

	return PLErrors::kNone;
}

PLError_t SetDefaultOutputVolume(long vol)
{
	return PLErrors::kNone;
}


namespace PortabilityLayer
{
	class SoundSystemImpl final : public SoundSystem
	{
	public:
		AudioChannel *CreateChannel() override;

		void SetVolume(uint8_t vol) override;
		uint8_t GetVolume() const override;

		static SoundSystemImpl *GetInstance();

	private:
		SoundSystemImpl();

		static SoundSystemImpl ms_instance;

		uint8_t m_volume;
	};

	AudioChannel *SoundSystemImpl::CreateChannel()
	{
		IGpAudioDriver *audioDriver = PortabilityLayer::HostAudioDriver::GetInstance();
		if (!audioDriver)
			return nullptr;

		PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();
		void *storage = mm->Alloc(sizeof(PortabilityLayer::AudioChannelImpl));
		if (!storage)
			return nullptr;

		IGpAudioChannel *audioChannel = audioDriver->CreateChannel();
		if (!audioChannel)
		{
			mm->Release(storage);
			return nullptr;
		}

		PortabilityLayer::HostMutex *mutex = PortabilityLayer::HostSystemServices::GetInstance()->CreateRecursiveMutex();
		if (!mutex)
		{
			audioChannel->Destroy();
			mm->Release(storage);
			return nullptr;
		}

		PortabilityLayer::HostThreadEvent *threadEvent = PortabilityLayer::HostSystemServices::GetInstance()->CreateThreadEvent(true, false);
		if (!threadEvent)
		{
			mutex->Destroy();
			audioChannel->Destroy();
			mm->Release(storage);
			return nullptr;
		}

		return new (storage) PortabilityLayer::AudioChannelImpl(audioChannel, threadEvent, mutex);
	}

	void SoundSystemImpl::SetVolume(uint8_t vol)
	{
		IGpAudioDriver *audioDriver = PortabilityLayer::HostAudioDriver::GetInstance();

		if (!audioDriver)
			return;

		audioDriver->SetMasterVolume(vol, 255);
		m_volume = vol;
	}

	uint8_t SoundSystemImpl::GetVolume() const
	{
		PL_NotYetImplemented_TODO("Volume");
		return m_volume;
	}

	SoundSystemImpl *SoundSystemImpl::GetInstance()
	{
		return &ms_instance;
	}

	SoundSystemImpl::SoundSystemImpl()
		: m_volume(255)
	{
	}

	SoundSystemImpl SoundSystemImpl::ms_instance;

	SoundSystem *SoundSystem::GetInstance()
	{
		return SoundSystemImpl::GetInstance();
	}
}
