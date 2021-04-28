#pragma once

#include "IGpAudioBuffer.h"

#include <xaudio2.h>

struct IGpAllocator;

class GpAudioBufferXAudio2 final : public IGpAudioBuffer
{
public:
	static GpAudioBufferXAudio2 *Create(IGpAllocator *alloc, const void *buffer, size_t size);

	void AddRef() override;
	void Release() override;

	const XAUDIO2_BUFFER *GetXA2Buffer() const;

private:
	GpAudioBufferXAudio2(IGpAllocator *alloc, const void *data, size_t size);
	~GpAudioBufferXAudio2();

	void Destroy();

	const void *m_data;
	size_t m_size;
	IGpAllocator *m_alloc;

	XAUDIO2_BUFFER m_xa2Buffer;

	volatile unsigned int m_count;
};
