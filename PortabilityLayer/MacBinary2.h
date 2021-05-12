#pragma once

class GpIOStream;
struct IGpAllocator;

namespace PortabilityLayer
{
	class MacFileMem;
	struct MacFileInfo;

	namespace MacBinary2
	{
		static const int kHeaderSize = 128;

		void SerializeHeader(unsigned char *headerBytes, const MacFileInfo &macFileInfo);

		void WriteBin(const MacFileMem *file, GpIOStream *stream);
		MacFileMem *ReadBin(GpIOStream *stream, IGpAllocator *alloc);
	};
}
