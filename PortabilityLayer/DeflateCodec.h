#pragma once

#include <stdint.h>

class GpIOStream;

namespace PortabilityLayer
{
	class DeflateCodec
	{
	public:
		static bool DecompressStream(GpIOStream *stream, size_t inSize, void *outBuffer, size_t outSize);
	};
}
