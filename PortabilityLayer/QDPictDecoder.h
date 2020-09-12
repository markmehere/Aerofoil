#pragma once

#include <stdint.h>

struct Rect;
struct Point;
class GpIOStream;

namespace PortabilityLayer
{
	class QDPictEmitContext;

	class QDPictDecoder
	{
	public:
		QDPictDecoder();

		bool DecodePict(GpIOStream *stream, QDPictEmitContext *emitContext);

	private:
		int ProcessRasterOp(GpIOStream *stream, int pictVersion, bool isPackedFlag, bool hasRegion, bool isDirect, const Rect &drawArea, const Point &origin, QDPictEmitContext *context);
		static bool UnpackBits8(uint8_t *dest, size_t destSize, const uint8_t *src, size_t srcSize);
		static bool UnpackBits16(uint8_t *dest, size_t destSize, const uint8_t *src, size_t srcSize);

		GpIOStream *m_stream;
	};
}
