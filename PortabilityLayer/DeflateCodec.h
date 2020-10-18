#pragma once

#include <stdint.h>

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
	};

	class DeflateCodec
	{
	public:
		static bool DecompressStream(GpIOStream *stream, size_t inSize, void *outBuffer, size_t outSize);
	};
}
