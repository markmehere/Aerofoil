#pragma once

#include <stdint.h>

struct Rect;

namespace PortabilityLayer
{
	class IOStream;
	class QDPictEmitContext;

	class QDPictDecoder
	{
	public:
		QDPictDecoder();

		bool DecodePict(IOStream *stream, QDPictEmitContext *emitContext);

	private:
		int ProcessRasterOp(IOStream *stream, int pictVersion, bool isPackedFlag, bool hasRegion, bool isDirect, const Rect &drawArea, QDPictEmitContext *context);
		static bool UnpackBits8(uint8_t *dest, size_t destSize, const uint8_t *src, size_t srcSize);
		static bool UnpackBits16(uint8_t *dest, size_t destSize, const uint8_t *src, size_t srcSize);

		IOStream *m_stream;
	};
}
