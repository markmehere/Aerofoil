#include "PLQDOffscreen.h"
#include "MemoryManager.h"
#include "MemReaderStream.h"
#include "ResourceManager.h"
#include "Rect2i.h"
#include "ResTypeID.h"
#include "QDStandardPalette.h"
#include "QDManager.h"
#include "QDGraf.h"
#include "QDPictDecoder.h"
#include "QDPictEmitContext.h"
#include "QDPictEmitScanlineParameters.h"
#include "QDPictHeader.h"
#include "QDPictOpcodes.h"
#include "QDPixMap.h"
#include "Vec2i.h"

#include <string.h>
#include <assert.h>

namespace PortabilityLayer
{
	class PixMapBlitEmitter final : public QDPictEmitContext
	{
	public:
		PixMapBlitEmitter(const Vec2i &drawOrigin, PixMapImpl *pixMap);
		~PixMapBlitEmitter();

		bool SpecifyFrame(const Rect &rect) override;
		Rect ConstrainRegion(const Rect &rect) const override;
		void Start(QDPictBlitSourceType sourceType, const QDPictEmitScanlineParameters &params) override;
		void BlitScanlineAndAdvance(const void *) override;
		bool AllocTempBuffers(uint8_t *&buffer1, size_t buffer1Size, uint8_t *&buffer2, size_t buffer2Size) override;

	private:
		PixMapImpl *m_pixMap;
		Vec2i m_drawOrigin;
		uint8_t *m_tempBuffer;
		Rect m_specFrame;

		QDPictBlitSourceType m_blitType;
		QDPictEmitScanlineParameters m_params;

		size_t m_constraintRegionWidth;
		size_t m_constraintRegionStartIndex;
		size_t m_constraintRegionEndIndex;
		size_t m_outputIndexStart;

		uint8_t m_paletteMap[256];
		bool m_sourceAndDestPalettesAreSame;
	};

	PixMapBlitEmitter::PixMapBlitEmitter(const Vec2i &drawOrigin, PixMapImpl *pixMap)
		: m_pixMap(pixMap)
		, m_drawOrigin(drawOrigin)
		, m_tempBuffer(nullptr)
		, m_sourceAndDestPalettesAreSame(false)
	{
	}

	PixMapBlitEmitter::~PixMapBlitEmitter()
	{
		if (m_tempBuffer)
			PortabilityLayer::MemoryManager::GetInstance()->Release(m_tempBuffer);
	}

	bool PixMapBlitEmitter::SpecifyFrame(const Rect &rect)
	{
		m_specFrame = rect;
		return true;
	}

	Rect PixMapBlitEmitter::ConstrainRegion(const Rect &rect) const
	{
		const Rect pixMapRect = m_pixMap->m_rect;

		const Rect2i rectInDrawSpace = Rect2i(rect) + m_drawOrigin;

		const Rect2i constrainedRectInDrawSpace = rectInDrawSpace.Intersect(Rect2i(pixMapRect));

		// If this got completely culled away, return an empty rect, but avoid int truncation
		if (!constrainedRectInDrawSpace.IsValid())
			return Rect::Create(rect.top, rect.left, rect.top, rect.left);

		// Otherwise, it should still be valid in the picture space
		const Rect2i constrainedRectInPictSpace = constrainedRectInDrawSpace - m_drawOrigin;
		return constrainedRectInPictSpace.ToShortRect();
	}

	void PixMapBlitEmitter::Start(QDPictBlitSourceType sourceType, const QDPictEmitScanlineParameters &params)
	{
		// FIXME: Detect different system palette (if we ever do that)
		if (QDPictBlitSourceType_IsIndexed(sourceType))
		{
			if (params.m_numColors == 256 && !memcmp(params.m_colors, StandardPalette::GetInstance()->GetColors(), sizeof(RGBAColor) * 256))
				m_sourceAndDestPalettesAreSame = true;
			else
			{
				assert(false);
			}
		}
		
		m_blitType = sourceType;
		m_params = params;

		m_constraintRegionWidth = params.m_constrainedRegionRight - params.m_constrainedRegionLeft;
		m_constraintRegionStartIndex = params.m_constrainedRegionLeft - params.m_scanlineOriginX;
		m_constraintRegionEndIndex = params.m_constrainedRegionRight - params.m_scanlineOriginX;

		const size_t firstCol = params.m_constrainedRegionLeft + m_drawOrigin.m_x - m_pixMap->m_rect.left;
		const size_t firstRow = params.m_firstY + m_drawOrigin.m_y - m_pixMap->m_rect.top;

		m_outputIndexStart = firstRow * m_pixMap->GetPitch() + firstCol;
	}

	void PixMapBlitEmitter::BlitScanlineAndAdvance(const void *data)
	{
		const int32_t crRight = m_params.m_constrainedRegionRight;
		const int32_t crLeft = m_params.m_constrainedRegionLeft;
		const size_t constraintRegionStartIndex = m_constraintRegionStartIndex;
		const uint8_t *dataBytes = static_cast<const uint8_t*>(data);
		const size_t outputIndexStart = m_outputIndexStart;
		const size_t planarSeparation = m_params.m_planarSeparation;
		const size_t constraintRegionWidth = m_constraintRegionWidth;

		const uint8_t *paletteMapping = nullptr;

		const uint8_t staticMapping1Bit[] = { 0, 255 };

		void *imageData = m_pixMap->GetPixelData();

		if (m_pixMap->GetPixelFormat() == PixelFormat_8BitStandard)
		{
			switch (m_blitType)
			{
			case QDPictBlitSourceType_Indexed1Bit:
				for (size_t i = 0; i < constraintRegionWidth; i++)
				{
					const size_t itemIndex = i + constraintRegionStartIndex;

					const int bitShift = 7 - (itemIndex & 7);
					const int colorIndex = (dataBytes[itemIndex / 8] >> bitShift) & 0x1;
					static_cast<uint8_t*>(imageData)[i + outputIndexStart] = paletteMapping[colorIndex];
				}
				break;
			case QDPictBlitSourceType_Indexed2Bit:
				for (size_t i = 0; i < constraintRegionWidth; i++)
				{
					const size_t itemIndex = i + constraintRegionStartIndex;

					const int bitShift = 6 - (2 * (itemIndex & 1));
					const int colorIndex = (dataBytes[itemIndex / 4] >> bitShift) & 0x3;
					static_cast<uint8_t*>(imageData)[i + outputIndexStart] = paletteMapping[colorIndex];
				}
				break;
			case QDPictBlitSourceType_Indexed4Bit:
				for (size_t i = 0; i < constraintRegionWidth; i++)
				{
					const size_t itemIndex = i + constraintRegionStartIndex;

					const int bitShift = 4 - (4 * (itemIndex & 1));
					const int colorIndex = (dataBytes[itemIndex / 2] >> bitShift) & 0xf;
					static_cast<uint8_t*>(imageData)[i + outputIndexStart] = paletteMapping[colorIndex];
				}
				break;
			case QDPictBlitSourceType_Indexed8Bit:
				if (m_sourceAndDestPalettesAreSame)
					memcpy(static_cast<uint8_t*>(imageData) + outputIndexStart, dataBytes + constraintRegionStartIndex, m_constraintRegionWidth);
				else
				{
					for (size_t i = 0; i < constraintRegionWidth; i++)
					{
						const size_t itemIndex = i + constraintRegionStartIndex;
						const uint8_t colorIndex = dataBytes[itemIndex];
						static_cast<uint8_t*>(imageData)[i + outputIndexStart] = paletteMapping[colorIndex];
					}
				}
				break;
			case QDPictBlitSourceType_1Bit:
				for (size_t i = 0; i < constraintRegionWidth; i++)
				{
					const size_t itemIndex = i + constraintRegionStartIndex;

					const int bitShift = 7 - (itemIndex & 7);
					const int colorIndex = (dataBytes[itemIndex / 8] >> bitShift) & 0x1;
					static_cast<uint8_t*>(imageData)[i + outputIndexStart] = staticMapping1Bit[colorIndex];
				}
				break;
			case QDPictBlitSourceType_RGB15:
				for (size_t i = 0; i < constraintRegionWidth; i++)
				{
					const size_t itemIndex = i + constraintRegionStartIndex;

					const uint16_t item = *reinterpret_cast<const uint16_t*>(dataBytes + itemIndex * 2);
					uint8_t &outputItem = static_cast<uint8_t*>(imageData)[i + outputIndexStart];

					outputItem = StandardPalette::GetInstance()->MapColorLUT((item >> 1) & 0xf, (item >> 6) & 0xf, (item >> 11) & 0x1f);
				}
				break;
			case QDPictBlitSourceType_RGB24_Multiplane:
				for (size_t i = 0; i < m_constraintRegionWidth; i++)
				{
					const size_t itemIndex = i + constraintRegionStartIndex;

					uint8_t &outputItem = static_cast<uint8_t*>(imageData)[i + outputIndexStart];

					const uint8_t r = dataBytes[itemIndex];
					const uint8_t g = dataBytes[itemIndex + planarSeparation];
					const uint8_t b = dataBytes[itemIndex + planarSeparation * 2];

					outputItem = StandardPalette::GetInstance()->MapColorLUT(r, g, b);
				}
				break;
			default:
				assert(false);
			}
		}

		m_outputIndexStart += m_pixMap->GetPitch();
	}

	bool PixMapBlitEmitter::AllocTempBuffers(uint8_t *&buffer1, size_t buffer1Size, uint8_t *&buffer2, size_t buffer2Size)
	{
		m_tempBuffer = static_cast<uint8_t*>(PortabilityLayer::MemoryManager::GetInstance()->Alloc(buffer1Size + buffer2Size));
		if (!m_tempBuffer)
			return false;

		buffer1 = m_tempBuffer;
		buffer2 = m_tempBuffer + buffer1Size;

		return true;
	}
}

OSErr NewGWorld(GWorldPtr *gworld, int depth, Rect *bounds, CTabHandle colorTable, GDHandle device, int flags)
{
	return PortabilityLayer::QDManager::GetInstance()->NewGWorld(gworld, depth, *bounds, colorTable, device, flags);
}

void DisposeGWorld(GWorldPtr gworld)
{
	PL_NotYetImplemented();
}

PixMapHandle GetGWorldPixMap(GWorldPtr gworld)
{
	if (!gworld)
		return nullptr;

	return gworld->m_port.GetPixMap();
}

void LockPixels(PixMapHandle pixmap)
{
	(void)pixmap;
}

PicHandle GetPicture(short resID)
{
	return reinterpret_cast<PicHandle>(PortabilityLayer::ResourceManager::GetInstance()->GetResource('PICT', resID));
}

void OffsetRect(Rect *rect, int right, int down)
{
	rect->left += right;
	rect->right += right;
	rect->top += down;
	rect->bottom += down;
}

void DrawPicture(PicHandle pict, Rect *bounds)
{
	if (!pict)
		return;

	PicPtr picPtr = *pict;

	if (!picPtr)
		return;

	const Rect picRect = picPtr->picFrame.ToRect();

	if (bounds->right - bounds->left != picRect.right - picRect.left || bounds->bottom - bounds->top != picRect.bottom - picRect.top)
	{
		// Scaled pict draw (not supported)
		assert(false);
		return;
	}

	PortabilityLayer::QDManager *qdManager = PortabilityLayer::QDManager::GetInstance();
	PortabilityLayer::QDPort *port;
	qdManager->GetPort(&port, nullptr);

	if (!port)
		return;

	PortabilityLayer::PixMapImpl *pixMap = static_cast<PortabilityLayer::PixMapImpl*>(*port->GetPixMap());

	long handleSize = GetHandleSize(reinterpret_cast<Handle>(pict));
	PortabilityLayer::MemReaderStream stream(picPtr, handleSize);

	switch (pixMap->GetPixelFormat())
	{
	case PortabilityLayer::PixelFormat_8BitStandard:
		{
			PortabilityLayer::PixMapBlitEmitter blitEmitter(PortabilityLayer::Vec2i(bounds->left, bounds->top), pixMap);
			PortabilityLayer::QDPictDecoder decoder;

			decoder.DecodePict(&stream, &blitEmitter);
		}
		break;
	default:
		// TODO: Implement higher-resolution pixel blitters
		assert(false);
		return;
	};
}

void GetGWorld(CGrafPtr *gw, GDHandle *gdHandle)
{
	PortabilityLayer::QDPort *port;
	PortabilityLayer::QDManager::GetInstance()->GetPort(&port, gdHandle);

	CGrafPtr grafPtr = reinterpret_cast<CGrafPtr>(port);
	assert(&grafPtr->m_port == port);

	*gw = grafPtr;
}

void SetGWorld(CGrafPtr gw, GDHandle gdHandle)
{
	PortabilityLayer::QDManager::GetInstance()->SetPort(&gw->m_port, gdHandle);
}
