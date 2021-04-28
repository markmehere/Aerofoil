#include "GpAudioBufferXAudio2.h"
#include "CoreDefs.h"
#include "GpWindows.h"
#include "IGpAllocator.h"

#include <string.h>
#include <new>

GpAudioBufferXAudio2 *GpAudioBufferXAudio2::Create(IGpAllocator *alloc, const void *buffer, size_t size)
{
	size_t baseSize = sizeof(GpAudioBufferXAudio2);
	baseSize = baseSize + GP_SYSTEM_MEMORY_ALIGNMENT - 1;
	baseSize -= baseSize % GP_SYSTEM_MEMORY_ALIGNMENT;

	size_t totalSize = baseSize + size;

	void *storage = alloc->Realloc(nullptr, totalSize);
	if (!storage)
		return nullptr;

	void *dataPos = static_cast<uint8_t*>(storage) + baseSize;

	memcpy(dataPos, buffer, size);
	return new (storage) GpAudioBufferXAudio2(alloc, dataPos, size);
}

void GpAudioBufferXAudio2::AddRef()
{
	InterlockedIncrement(&m_count);
}

void GpAudioBufferXAudio2::Release()
{
	if (InterlockedDecrement(&m_count) == 0)
		this->Destroy();
}

const XAUDIO2_BUFFER *GpAudioBufferXAudio2::GetXA2Buffer() const
{
	return &m_xa2Buffer;
}

GpAudioBufferXAudio2::GpAudioBufferXAudio2(IGpAllocator *alloc, const void *data, size_t size)
	: m_alloc(alloc)
	, m_data(data)
	, m_size(size)
	, m_count(1)
{
	m_xa2Buffer.Flags = 0;
	m_xa2Buffer.AudioBytes = static_cast<UINT32>(size);
	m_xa2Buffer.pAudioData = static_cast<const BYTE*>(data);
	m_xa2Buffer.PlayBegin = 0;
	m_xa2Buffer.PlayLength = 0;
	m_xa2Buffer.LoopBegin = 0;
	m_xa2Buffer.LoopLength = 0;
	m_xa2Buffer.LoopCount = 0;
	m_xa2Buffer.pContext = this;
}

GpAudioBufferXAudio2::~GpAudioBufferXAudio2()
{
}

void GpAudioBufferXAudio2::Destroy()
{
	IGpAllocator *alloc = m_alloc;
	this->~GpAudioBufferXAudio2();
	m_alloc->Realloc(this, 0);
}
