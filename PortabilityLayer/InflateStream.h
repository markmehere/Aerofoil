#pragma once

#include "GpIOStream.h"

namespace PortabilityLayer
{
	namespace InflateStream
	{
		GpIOStream *Create(GpIOStream *stream, GpUFilePos_t start, size_t compressedSize, size_t decompressedSize);
	};
}
