#pragma once

#ifndef __PL_MACBINARY2_H__
#define __PL_MACBINARY2_H__

namespace PortabilityLayer
{
	class IOStream;
	class MacFileMem;

	namespace MacBinary2
	{
		void WriteBin(const MacFileMem *file, IOStream *stream);
		MacFileMem *ReadBin(IOStream *stream);
	};
}

#endif
