#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	class IOStream;

	class DeflateCodec
	{
	public:
		static bool DecompressStream(IOStream *stream, size_t inSize, void *outBuffer, size_t outSize);
	};
}
