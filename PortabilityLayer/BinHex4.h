#pragma once

#ifndef __PL_BINHEX4_H__
#define __PL_BINHEX4_H__

namespace PortabilityLayer
{
	class IOStream;
	class MacFileMem;

	namespace BinHex4
	{
		MacFileMem *LoadHQX(IOStream *stream);
	};
}

#endif
