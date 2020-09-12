#pragma once

#ifndef __PL_MACBINARY2_H__
#define __PL_MACBINARY2_H__

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

#endif
