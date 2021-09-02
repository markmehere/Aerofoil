#pragma once

#include <stdint.h>
#include <stddef.h>

struct Rect;
class GpIOStream;

namespace PortabilityLayer
{
	struct RGBAColor;
	struct QDPictEmitScanlineParameters;

	enum QDPictBlitSourceType
	{
		QDPictBlitSourceType_1Bit,
		QDPictBlitSourceType_Indexed1Bit,
		QDPictBlitSourceType_Indexed2Bit,
		QDPictBlitSourceType_Indexed4Bit,
		QDPictBlitSourceType_Indexed8Bit,
		QDPictBlitSourceType_RGB15Native,
		QDPictBlitSourceType_RGB15BE,
		QDPictBlitSourceType_RGB24_Interleaved,
		QDPictBlitSourceType_RGB24_Multiplane,
	};

	bool QDPictBlitSourceType_IsIndexed(QDPictBlitSourceType sourceType);

	class QDPictEmitContext
	{
	public:
		enum ErrorCode
		{
			kMissingHeader,
			kInvalidRegionRect,
			kMalformedArguments,
			kUnusedError1,
			kUnusedError2,
			kRasterOpFailure,
			kUnsupportedClipRegionFormat,
			kMalformedOpcode,
			kUnsupportedOpcode,
		};

		virtual bool SpecifyFrame(const Rect &rect) = 0;
		virtual Rect ConstrainRegion(const Rect &rect) const = 0;
		virtual void Start(QDPictBlitSourceType sourceType, const QDPictEmitScanlineParameters &params) = 0;
		virtual void BlitScanlineAndAdvance(const void *) = 0;
		virtual bool EmitQTContent(GpIOStream *stream, uint32_t dataSize, bool isCompressed) = 0;
		virtual bool AllocTempBuffers(uint8_t *&buffer1, size_t buffer1Size, uint8_t *&buffer2, size_t buffer2Size) = 0;

		virtual void ReportError(int errorType, int errorSubtype) { }
	};
}
