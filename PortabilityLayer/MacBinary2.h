#pragma once

class GpIOStream;

namespace PortabilityLayer
{
	class MacFileMem;

	namespace MacBinary2
	{
		void WriteBin(const MacFileMem *file, GpIOStream *stream);
		MacFileMem *ReadBin(GpIOStream *stream);
	};
}
