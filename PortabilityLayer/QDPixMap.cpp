#include "QDPixMap.h"
#include "CoreDefs.h"
#include "MemoryManager.h"

#include <assert.h>

class PixMapSampler_8BitStandard
{
public:
	inline static uint8_t ReadAs8BitStandard(const void *rowData, size_t index)
	{
		return static_cast<const uint8_t*>(rowData)[index];
	}
};

template<class TSampler>
class PixMapCopier_8BitStandard
{
public:
	inline static void Copy(const void *inData, size_t inIndex, void *outData, size_t outIndex)
	{
		static_cast<uint8_t*>(outData)[outIndex] = TSampler::ReadAs8BitStandard(inData, inIndex);
	}
};

template<class TCopier>
class PixMapColBlitter
{
public:
	static void BlitRow(const void *inData, size_t inSize, void *outData, size_t outSize)
	{
		if (inSize == outSize)
		{
			for (size_t i = 0; i < inSize; i++)
				TCopier::Copy(inData, i, outData, i);
		}
		else if (inSize < outSize)
		{
			size_t remainder = 0;
			size_t inIndex = 0;
			for (size_t i = 0; i < outSize; i++)
			{
				TCopier::Copy(inData, inIndex, outData, i);

				remainder += inSize;
				if (remainder >= outSize)
				{
					remainder -= outSize;
					inIndex++;
				}
			}
		}
		else //if (outSize < inSize)
		{
			size_t remainder = 0;
			size_t outIndex = 0;
			for (size_t i = 0; i < inSize; i++)
			{
				remainder += outSize;
				if (remainder >= inSize)
				{
					TCopier::Copy(inData, i, outData, outIndex);

					remainder -= inSize;
					outIndex++;
				}
			}
		}
	}
};

template<class TCopier>
class PixMapRowBlitter
{
public:
	static void Blit(const void *inData, size_t inPitch, size_t inColCount, size_t inRowCount, void *outData, size_t outPitch, size_t outColCount, size_t outRowCount)
	{
		if (inRowCount == outRowCount)
		{
			for (size_t i = 0; i < inRowCount; i++)
			{
				PixMapColBlitter<TCopier>::BlitRow(inData, inColCount, outData, outColCount);

				inData = static_cast<const uint8_t*>(inData) + inPitch;
				outData = static_cast<uint8_t*>(outData) + outPitch;
			}
		}
		else if (inRowCount < outRowCount)
		{
			size_t remainder = 0;
			size_t inIndex = 0;
			for (size_t i = 0; i < outRowCount; i++)
			{
				PixMapColBlitter<TCopier>::BlitRow(inData, inColCount, outData, outColCount);

				remainder += inRowCount;
				if (remainder >= outRowCount)
				{
					remainder -= outRowCount;
					inData = static_cast<const uint8_t*>(inData) + inPitch;
				}

				outData = static_cast<uint8_t*>(outData) + outPitch;
			}
		}
		else //if(outRowCount < inRowCount)
		{
			size_t remainder = 0;
			size_t outIndex = 0;
			for (size_t i = 0; i < inRowCount; i++)
			{
				remainder += outRowCount;
				if (remainder >= inRowCount)
				{
					PixMapColBlitter<TCopier>::BlitRow(inData, inColCount, outData, outColCount);

					remainder -= inRowCount;
					outData = static_cast<uint8_t*>(outData) + outPitch;
				}

				inData = static_cast<const uint8_t*>(inData) + inPitch;
			}
		}
	}
};

template<class TSampler>
class PixMapBlitTargetDisambiguator
{
public:
	static void Blit(const void *inData, size_t inPitch, size_t inColCount, size_t inRowCount, void *outData, size_t outPitch, size_t outColCount, size_t outRowCount, GpPixelFormat_t destFormat)
	{
		void(*blitFunc)(const void *inData, size_t inPitch, size_t inColCount, size_t inRowCount, void *outData, size_t outPitch, size_t outColCount, size_t outRowCount);

		blitFunc = nullptr;

		switch (destFormat)
		{
		case GpPixelFormats::k8BitStandard:
			blitFunc = PixMapRowBlitter<PixMapCopier_8BitStandard<TSampler> >::Blit;
			break;
		default:
			PL_NotYetImplemented();
			break;
		}

		if (blitFunc != nullptr)
			blitFunc(inData, inPitch, inColCount, inRowCount, outData, outPitch, outColCount, outRowCount);
	}
};

class PixMapBlitSourceDisambiguator
{
public:
	static void Blit(const void *inData, size_t inPitch, size_t inColCount, size_t inRowCount, GpPixelFormat_t srcFormat, void *outData, size_t outPitch, size_t outColCount, size_t outRowCount, GpPixelFormat_t destFormat)
	{
		void(*blitFunc)(const void *inData, size_t inPitch, size_t inColCount, size_t inRowCount, void *outData, size_t outPitch, size_t outColCount, size_t outRowCount, GpPixelFormat_t srcFormat);

		blitFunc = nullptr;

		switch (srcFormat)
		{
		case GpPixelFormats::k8BitStandard:
			blitFunc = PixMapBlitTargetDisambiguator<PixMapSampler_8BitStandard>::Blit;
			break;
		default:
			PL_NotYetImplemented();
			break;
		}

		if (blitFunc != nullptr)
			blitFunc(inData, inPitch, inColCount, inRowCount, outData, outPitch, outColCount, outRowCount, srcFormat);
	}
};

namespace PortabilityLayer
{
	void PixMapImpl::Destroy(THandle<PixMapImpl> &hdl)
	{
		if (hdl)
		{
			(*hdl)->~PixMapImpl();
		}

		hdl.Dispose();
	}

	PixMapImpl::PixMapImpl(int16_t left, int16_t top, uint16_t width, uint16_t height, GpPixelFormat_t pixelFormat)
		: m_left(left)
		, m_top(top)
		, m_width(width)
		, m_height(height)
		, m_dataCapacity(0)
	{
		const Rect rect = Rect::Create(top, left, static_cast<uint16_t>(top + height), static_cast<uint16_t>(left + width));

		const size_t pitch = PitchForWidth(width, pixelFormat);

		void *dataPtr = reinterpret_cast<uint8_t*>(this) + AlignedSize();
		m_dataCapacity = PitchForWidth(width, pixelFormat) * height;

		static_cast<PixMap*>(this)->Init(rect, pixelFormat, PitchForWidth(width, pixelFormat), dataPtr);
	}

	PixMapImpl::~PixMapImpl()
	{
	}

	size_t PixMapImpl::SizeForDimensions(uint16_t width, uint16_t height, GpPixelFormat_t pixelFormat)
	{
		return AlignedSize() + PitchForWidth(width, pixelFormat) * height;
	}

	size_t PixMapImpl::AlignedSize()
	{
		const size_t szBase = sizeof(PixMapImpl) + GP_SYSTEM_MEMORY_ALIGNMENT - 1;
		const size_t szAdjusted = szBase - szBase % GP_SYSTEM_MEMORY_ALIGNMENT;

		return szAdjusted;
	}

	size_t PixMapImpl::PitchForWidth(uint16_t width, GpPixelFormat_t pixelFormat)
	{
		size_t rowByteCount = 0;

		switch (pixelFormat)
		{
		case GpPixelFormats::k8BitCustom:
		case GpPixelFormats::k8BitStandard:
		case GpPixelFormats::kBW1:
			rowByteCount = width;
			break;
		case GpPixelFormats::kRGB555:
			rowByteCount = width * 2;
			break;
		case GpPixelFormats::kRGB24:
			rowByteCount = width * 3;
			break;
		case GpPixelFormats::kRGB32:
			rowByteCount = width * 4;
			break;
		default:
			assert(false);
			return 0;
		}
		const size_t szBase = rowByteCount + GP_SYSTEM_MEMORY_ALIGNMENT - 1;
		const size_t szAdjusted = szBase - szBase % GP_SYSTEM_MEMORY_ALIGNMENT;

		return szAdjusted;
	}

	THandle<PixMapImpl> PixMapImpl::Create(const Rect &rect, GpPixelFormat_t pixelFormat)
	{
		if (!rect.IsValid())
			return THandle<PixMapImpl>();

		const uint16_t width = static_cast<uint16_t>(rect.right - rect.left);
		const uint16_t height = static_cast<uint16_t>(rect.bottom - rect.top);

		const size_t pixMapSize = PixMapImpl::SizeForDimensions(width, height, pixelFormat);
		if (pixMapSize == 0)
			return THandle<PixMapImpl>();

		MMHandleBlock *pmBlock = PortabilityLayer::MemoryManager::GetInstance()->AllocHandle(pixMapSize);
		if (!pmBlock)
			return THandle<PixMapImpl>();

		memset(pmBlock->m_contents, 0, pixMapSize);

		new (pmBlock->m_contents) PixMapImpl(rect.left, rect.top, width, height, pixelFormat);

		return THandle<PixMapImpl>(pmBlock);
	}


	THandle<PixMapImpl> PixMapImpl::ScaleTo(uint16_t width, uint16_t height)
	{
		// Stupid stuff to cover the entire numeric range
		Rect scaledRect;
		scaledRect.right = static_cast<int16_t>(width / 2);
		scaledRect.bottom = static_cast<int16_t>(height / 2);
		scaledRect.top = static_cast<int16_t>(scaledRect.bottom - height);
		scaledRect.left = static_cast<int16_t>(scaledRect.right - width);

		THandle<PixMapImpl> scaled = PixMapImpl::Create(scaledRect, m_pixelFormat);
		if (!scaled)
			return THandle<PixMapImpl>();

		PixMapImpl *destPixMap = *scaled;

		const uint16_t oldWidth = m_rect.Width();
		const uint16_t oldHeight = m_rect.Height();

		PixMapBlitSourceDisambiguator::Blit(GetPixelData(), m_pitch, m_rect.Width(), m_rect.Height(), m_pixelFormat, destPixMap->GetPixelData(), destPixMap->GetPitch(), width, height, m_pixelFormat);

		return scaled;
	}
}

void PixMap::Init(const Rect &rect, GpPixelFormat_t pixelFormat, size_t pitch, void *dataPtr)
{
	BitMap::Init(rect, pixelFormat, pitch, dataPtr);
}
