#include "IconLoader.h"
#include "PLCore.h"
#include "PLCTabReducer.h"
#include "ResourceManager.h"
#include "QDStandardPalette.h"
#include "QDPixMap.h"

#include "SharedTypes.h"

// Color icons (cicn) format:
// ColorIconSerializedData (82 bytes)
// Mask bits
// B&W bits
// BEColorTableHeader (8 bytes)
// BEColorTableItem (8 bytes)
// Color bits

// ICON format is just a 32x32 bitfield

struct IconImagePrefix
{
	BEUInt32_t m_unknown;	// Seems to always be zero
	BEUInt16_t m_pitch;		// +0x8000 for color
};

struct ColorIconSerializedData
{
	IconImagePrefix m_colorPrefix;
	BEPixMap m_colorPixMap;
	IconImagePrefix m_maskPrefix;
	BEBitMap m_maskBitMap;
	IconImagePrefix m_bwPrefix;
	BEBitMap m_bwBitMap;

	uint8_t m_unused[4];
};

namespace PortabilityLayer
{
	class IconLoaderImpl final : public IconLoader
	{
	public:
		bool LoadColorIcon(const int16_t id, THandle<PixMapImpl> &outColorImage, THandle<PixMapImpl> &outBWImage, THandle<PixMapImpl> &outMaskImage) override;
		THandle<PixMapImpl> LoadSimpleColorIcon(const THandle<void> &hdl) override;
		THandle<PixMapImpl> LoadBWIcon(const THandle<void> &hdl) override;

		static IconLoaderImpl *GetInstance();

	private:
		static bool ParseColorImage(const IconImagePrefix &prefix, const BEPixMap &pixMapHeader, THandle<PixMapImpl> &outHandle, const uint8_t *&dataPtr);
		static bool ParseBWImage(const IconImagePrefix &prefix, const BEBitMap &bitMapHeader, THandle<PixMapImpl> &outHandle, const uint8_t *&dataPtr);

		static IconLoaderImpl ms_instance;
	};


	bool IconLoaderImpl::LoadColorIcon(const int16_t id, THandle<PixMapImpl> &outColorImage, THandle<PixMapImpl> &outBWImage, THandle<PixMapImpl> &outMaskImage)
	{
		THandle<const uint8_t> data = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('cicn', id).StaticCast<const uint8_t>();

		if (!data)
			return false;

		GP_STATIC_ASSERT(sizeof(ColorIconSerializedData) == 82);

		ColorIconSerializedData header;
		memcpy(&header, *data, sizeof(header));

		const uint8_t *dataPtr = (*data) + sizeof(header);

		THandle<PixMapImpl> maskImage;
		if (!ParseBWImage(header.m_maskPrefix, header.m_maskBitMap, maskImage, dataPtr))
		{
			data.Dispose();
			return false;
		}

		THandle<PixMapImpl> bwImage;
		if (!ParseBWImage(header.m_bwPrefix, header.m_bwBitMap, bwImage, dataPtr))
		{
			PixMapImpl::Destroy(maskImage);
			data.Dispose();
			return false;
		}

		THandle<PixMapImpl> colorImage;
		if (!ParseColorImage(header.m_colorPrefix, header.m_colorPixMap, colorImage, dataPtr))
		{
			PixMapImpl::Destroy(bwImage);
			PixMapImpl::Destroy(maskImage);
			data.Dispose();
			return false;
		}

		outColorImage = colorImage;
		outBWImage = bwImage;
		outMaskImage = maskImage;

		return true;
	}

	bool IconLoaderImpl::ParseColorImage(const IconImagePrefix &prefix, const BEPixMap &pixMapHeader, THandle<PixMapImpl> &outHandle, const uint8_t *&dataPtr)
	{
		if (pixMapHeader.m_componentCount != 1 || pixMapHeader.m_packType != 0)
		{
			PL_NotYetImplemented();
			return false;
		}

		const uint8_t *inData = dataPtr;

		BEColorTableHeader colorTableHeader;
		memcpy(&colorTableHeader, inData, sizeof(BEColorTableHeader));

		inData += sizeof(BEColorTableHeader);

		const size_t numItems = static_cast<uint16_t>(colorTableHeader.m_numItemsMinusOne) + 1;
		if (numItems > 256)
			return false;

		uint8_t remapping[256];
		for (int i = 0; i < 256; i++)
			remapping[i] = 0;

		for (size_t i = 0; i < numItems; i++)
		{
			BEColorTableItem ctabItem;
			memcpy(&ctabItem, inData, sizeof(BEColorTableItem));
			inData += sizeof(BEColorTableItem);

			const uint16_t index = ctabItem.m_index;
			if (index >= 256)
				return false;

			const PortabilityLayer::RGBAColor remappedColor = CTabReducer::DecodeClutItem(ctabItem);

			remapping[index] = StandardPalette::GetInstance()->MapColorLUT(remappedColor);
		}

		const Rect rect = pixMapHeader.m_bounds.ToRect();
		if (!rect.IsValid())
			return false;

		THandle<PixMapImpl> pixMap = PixMapImpl::Create(rect, GpPixelFormats::k8BitStandard);
		if (!pixMap)
			return false;

		const size_t width = rect.Width();
		const size_t height = rect.Height();
		const size_t inPitch = (prefix.m_pitch & 0x7fff);

		uint8_t *outData = static_cast<uint8_t*>((*pixMap)->GetPixelData());
		const size_t outPitch = (*pixMap)->GetPitch();

		for (size_t row = 0; row < height; row++)
		{
			if (numItems > 16)
			{
				// 8bpp
				for (size_t col = 0; col < width; col++)
				{
					const unsigned int index = inData[col];
					outData[col] = remapping[index];
				}
			}
			else if (numItems > 4)
			{
				// 4bpp
				for (size_t col = 0; col < width; col++)
				{
					const unsigned int index = (inData[col / 2] >> (4 - ((col & 1) * 4))) & 0x0f;
					outData[col] = remapping[index];
				}
			}
			else if (numItems > 2)
			{
				// 2bpp
				for (size_t col = 0; col < width; col++)
				{
					const unsigned int index = (inData[col / 4] >> (6 - ((col & 3) * 2))) & 0x03;
					outData[col] = remapping[index];
				}
			}
			else
			{
				// 1bpp
				for (size_t col = 0; col < width; col++)
				{
					const unsigned int index = (inData[col / 4] >> (7 - (col & 7))) & 0x01;
					outData[col] = remapping[index];
				}
			}

			inData += inPitch;
			outData += outPitch;
		}

		outHandle = pixMap;
		dataPtr = inData;

		return true;
	}

	bool IconLoaderImpl::ParseBWImage(const IconImagePrefix &prefix, const BEBitMap &bitMapHeader, THandle<PixMapImpl> &outHandle, const uint8_t *&dataPtr)
	{
		const Rect rect = bitMapHeader.m_bounds.ToRect();
		if (!rect.IsValid())
			return false;

		THandle<PixMapImpl> pixMap = PixMapImpl::Create(rect, GpPixelFormats::kBW1);
		if (!pixMap)
			return THandle<PixMapImpl>();

		const size_t inPitch = prefix.m_pitch;
		const size_t height = rect.Height();
		const size_t width = rect.Width();

		const uint8_t *inData = dataPtr;
		uint8_t *outData = static_cast<uint8_t*>((*pixMap)->GetPixelData());
		const size_t outPitch = (*pixMap)->GetPitch();

		for (size_t row = 0; row < height; row++)
		{
			for (size_t col = 0; col < width; col++)
			{
				if (inData[col / 8] & (0x80 >> (col & 7)))
					outData[col] = 0xff;
				else
					outData[col] = 0x00;
			}

			inData += inPitch;
			outData += outPitch;
		}

		dataPtr = inData;
		outHandle = pixMap;

		return true;
	}

	THandle<PixMapImpl> IconLoaderImpl::LoadSimpleColorIcon(const THandle<void> &hdl)
	{
		if (hdl == nullptr || hdl.MMBlock()->m_size != 1024)
			return THandle<PixMapImpl>();

		const Rect rect = Rect::Create(0, 0, 32, 32);
		THandle<PixMapImpl> pixMap = PixMapImpl::Create(rect, GpPixelFormats::k8BitStandard);
		if (!pixMap)
			return THandle<PixMapImpl>();

		const uint8_t *inData = static_cast<const uint8_t*>(*hdl);
		uint8_t *outData = static_cast<uint8_t*>((*pixMap)->GetPixelData());
		const size_t outPitch = (*pixMap)->GetPitch();

		for (size_t row = 0; row < 32; row++)
		{
			for (size_t col = 0; col < 32; col++)
				outData[col] = inData[col];

			inData += 32;
			outData += outPitch;
		}

		return pixMap;
	}

	THandle<PixMapImpl> IconLoaderImpl::LoadBWIcon(const THandle<void> &hdl)
	{
		if (hdl == nullptr || hdl.MMBlock()->m_size != 128)
			return THandle<PixMapImpl>();

		const Rect rect = Rect::Create(0, 0, 32, 32);
		THandle<PixMapImpl> pixMap = PixMapImpl::Create(rect, GpPixelFormats::kBW1);
		if (!pixMap)
			return THandle<PixMapImpl>();

		const uint8_t *inData = static_cast<const uint8_t*>(*hdl);
		uint8_t *outData = static_cast<uint8_t*>((*pixMap)->GetPixelData());
		const size_t outPitch = (*pixMap)->GetPitch();

		for (size_t row = 0; row < 32; row++)
		{
			for (size_t col = 0; col < 32; col++)
			{
				if (inData[col / 8] & (0x80 >> (col & 7)))
					outData[col] = 0xff;
				else
					outData[col] = 0x00;
			}

			inData += 4;
			outData += outPitch;
		}

		return pixMap;
	}

	IconLoaderImpl *IconLoaderImpl::GetInstance()
	{
		return &ms_instance;
	}

	IconLoaderImpl IconLoaderImpl::ms_instance;

	IconLoader *IconLoader::GetInstance()
	{
		return IconLoaderImpl::GetInstance();
	}
}
