#pragma once

class GpIOStream;
struct IGpAllocator;

namespace PortabilityLayer
{
	class MacFileMem;

	namespace BinHex4
	{
		MacFileMem *LoadHQX(GpIOStream *stream, IGpAllocator *alloc);
	};
}
