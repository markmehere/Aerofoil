#pragma once

#ifndef __PL_BINHEX4_H__
#define __PL_BINHEX4_H__

class GpIOStream;

namespace PortabilityLayer
{
	class MacFileMem;

	namespace BinHex4
	{
		MacFileMem *LoadHQX(GpIOStream *stream);
	};
}

#endif
