#pragma once

#include <stdint.h>
#include <stddef.h>

class GpIOStream;

namespace PortabilityLayer
{
	class DeflateContext
	{
	public:
		static DeflateContext *Create(GpIOStream *stream, int compressionLevel);

		virtual void Destroy() = 0;
		virtual bool Append(const void *buffer, size_t size) = 0;

		virtual bool Flush() = 0;

		static uint32_t CRC32(uint32_t inputValue, const void *buffer, size_t bufferLength);
	};

	class InflateContext
	{
	public:
		static InflateContext *Create();

		virtual void Destroy() = 0;

		virtual bool Append(const void *buffer, size_t size, size_t &sizeConsumed) = 0;
		virtual bool Read(void *buffer, size_t size, size_t &sizeRead) = 0;

		virtual bool Reset() = 0;
	};

	class DeflateCodec
	{
	public:
		static bool DecompressStream(GpIOStream *stream, size_t inSize, void *outBuffer, size_t outSize);
	};
}
