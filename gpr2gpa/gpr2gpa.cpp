#include "BMPFormat.h"
#include "CFileStream.h"
#include "GPArchive.h"
#include "MemReaderStream.h"
#include "QDPictDecoder.h"
#include "QDPictEmitContext.h"
#include "QDPictEmitScanlineParameters.h"
#include "ResourceFile.h"
#include "ResourceCompiledTypeList.h"
#include "SharedTypes.h"
#include "ZipFile.h"
#include "WaveFormat.h"

#include "zlib.h"

#include <stdio.h>
#include <vector>
#include <Windows.h>

struct PlannedEntry
{
	std::vector<uint8_t> m_contents;
	std::string m_name;
	bool m_isDirectory;

	PlannedEntry()
		: m_isDirectory(false)
	{
	}
};

 template<class T>
 void VectorAppend(std::vector<T> &vec, const T *items, size_t numItems)
 {
	 if (numItems == 0)
		 return;

	 vec.resize(vec.size() + numItems);

	 T *outLoc = &vec[vec.size() - numItems];
	 for (size_t i = 0; i < numItems; i++)
		 outLoc[i] = items[i];
 }

uint8_t FiveToEight(uint8_t v)
{
	return static_cast<uint8_t>((v << 3) | (v >> 2));
}

bool TryDeflate(const std::vector<uint8_t> &uncompressed, std::vector<uint8_t> &outCompressed)
{
	if (uncompressed.size() == 0)
		return false;

	const unsigned int bufferSize = 1024;

	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;

	int ret = deflateInit2(&stream, 9, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY);
	if (ret != Z_OK)
		return false;

	uint8_t buffer[bufferSize];

	stream.next_in = const_cast<unsigned char*>(&uncompressed[0]);
	stream.avail_in = uncompressed.size();
	stream.next_out = buffer;
	stream.avail_out = bufferSize;

	for (;;)
	{
		int deflateResult = deflate(&stream, Z_FINISH);

		size_t newDataSize = stream.next_out - buffer;
		if (newDataSize > 0)
		{
			outCompressed.resize(outCompressed.size() + newDataSize);
			memcpy(&outCompressed[outCompressed.size() - newDataSize], buffer, newDataSize);

			stream.next_out = buffer;
			stream.avail_out = bufferSize;
		}

		if (deflateResult == Z_STREAM_END)
			break;
	}

	deflateEnd(&stream);

	return true;
}

void ExportZipFile(const char *path, const std::vector<PlannedEntry> &entries)
{
	FILE *outF = nullptr;
	if (fopen_s(&outF, path, "wb"))
	{
		fprintf(stderr, "Error opening output path");
		return;
	}

	SYSTEMTIME localTime;
	GetLocalTime(&localTime);

	DWORD yearsSince1980 = localTime.wYear - 1980;
	if (yearsSince1980 < 0)
	{
		// Time machine
		yearsSince1980 = 0;
		localTime.wSecond = 0;
		localTime.wMinute = 0;
		localTime.wHour = 0;
		localTime.wDay = 1;
		localTime.wMonth = 1;
	}
	else if (yearsSince1980 > 127)
	{
		// Original author is either dead or cryofrozen
		yearsSince1980 = 127;
		localTime.wSecond = 59;
		localTime.wMinute = 59;
		localTime.wHour = 23;
		localTime.wDay = 31;
		localTime.wMonth = 12;
	}

	uint16_t msdosTime = 0;
	uint16_t msdosDate = 0;

	msdosTime |= localTime.wSecond / 2;
	msdosTime |= (localTime.wMinute << 5);
	msdosTime |= (localTime.wHour << 11);

	msdosDate |= localTime.wDay;
	msdosDate |= (localTime.wMonth << 5);
	msdosDate |= (yearsSince1980 << 9);

	std::vector<PortabilityLayer::ZipCentralDirectoryFileHeader> cdirRecords;

	for (const PlannedEntry &entry : entries)
	{
		std::vector<uint8_t> compressed;

		if (entry.m_contents.size() != 0)
		{
			if (!TryDeflate(entry.m_contents, compressed))
				compressed.resize(0);

			if (compressed.size() >= entry.m_contents.size())
				compressed.resize(0);
		}

		bool isCompressed = compressed.size() != 0;

		PortabilityLayer::ZipCentralDirectoryFileHeader cdirHeader;

		cdirHeader.m_signature = PortabilityLayer::ZipCentralDirectoryFileHeader::kSignature;
		cdirHeader.m_versionCreated = PortabilityLayer::ZipConstants::kCompressedRequiredVersion;
		cdirHeader.m_versionRequired = PortabilityLayer::ZipConstants::kStoredRequiredVersion;
		cdirHeader.m_flags = 0;
		cdirHeader.m_method = isCompressed ? PortabilityLayer::ZipConstants::kDeflatedMethod : PortabilityLayer::ZipConstants::kStoredMethod;
		cdirHeader.m_modificationTime = msdosTime;
		cdirHeader.m_modificationDate = msdosDate;
		cdirHeader.m_crc = 0;

		if (entry.m_isDirectory)
			cdirHeader.m_versionRequired = PortabilityLayer::ZipConstants::kDirectoryRequiredVersion;
		else if (isCompressed)
			cdirHeader.m_versionRequired = PortabilityLayer::ZipConstants::kCompressedRequiredVersion;

		if (entry.m_contents.size() > 0)
			cdirHeader.m_crc = crc32(0, &entry.m_contents[0], static_cast<uint32_t>(entry.m_contents.size()));

		cdirHeader.m_compressedSize = static_cast<uint32_t>(isCompressed ? compressed.size() : entry.m_contents.size());
		cdirHeader.m_uncompressedSize = static_cast<uint32_t>(entry.m_contents.size());
		cdirHeader.m_fileNameLength = static_cast<uint32_t>(entry.m_name.size());
		cdirHeader.m_extraFieldLength = 0;
		cdirHeader.m_commentLength = 0;
		cdirHeader.m_diskNumber = 0;
		cdirHeader.m_internalAttributes = 0;
		cdirHeader.m_externalAttributes = entry.m_isDirectory ? PortabilityLayer::ZipConstants::kDirectoryAttributes : PortabilityLayer::ZipConstants::kArchivedAttributes;
		cdirHeader.m_localHeaderOffset = ftell(outF);

		cdirRecords.push_back(cdirHeader);

		PortabilityLayer::ZipFileLocalHeader localHeader;

		localHeader.m_signature = PortabilityLayer::ZipFileLocalHeader::kSignature;
		localHeader.m_versionRequired = cdirHeader.m_versionRequired;
		localHeader.m_flags = 0;
		localHeader.m_method = cdirHeader.m_method;
		localHeader.m_modificationTime = cdirHeader.m_modificationTime;
		localHeader.m_modificationDate = cdirHeader.m_modificationDate;
		localHeader.m_crc = cdirHeader.m_crc;
		localHeader.m_compressedSize = cdirHeader.m_compressedSize;
		localHeader.m_uncompressedSize = cdirHeader.m_uncompressedSize;
		localHeader.m_fileNameLength = cdirHeader.m_fileNameLength;
		localHeader.m_extraFieldLength = 0;

		fwrite(&localHeader, 1, sizeof(localHeader), outF);

		fwrite(entry.m_name.c_str(), 1, entry.m_name.size(), outF);

		if (isCompressed)
			fwrite(&compressed[0], 1, compressed.size(), outF);
		else if (entry.m_contents.size() > 0)
			fwrite(&entry.m_contents[0], 1, entry.m_contents.size(), outF);
	}

	long cdirPos = ftell(outF);

	for (size_t i = 0; i < entries.size(); i++)
	{
		fwrite(&cdirRecords[i], 1, sizeof(PortabilityLayer::ZipCentralDirectoryFileHeader), outF);
		fwrite(entries[i].m_name.c_str(), 1, entries[i].m_name.size(), outF);
	}

	long cdirEndPos = ftell(outF);

	PortabilityLayer::ZipEndOfCentralDirectoryRecord endRecord;

	endRecord.m_signature = PortabilityLayer::ZipEndOfCentralDirectoryRecord::kSignature;
	endRecord.m_thisDiskNumber = 0;
	endRecord.m_centralDirDisk = 0;
	endRecord.m_numCentralDirRecordsThisDisk = static_cast<uint32_t>(entries.size());
	endRecord.m_numCentralDirRecords = static_cast<uint32_t>(entries.size());
	endRecord.m_centralDirectorySizeBytes = cdirEndPos - cdirPos;
	endRecord.m_centralDirStartOffset = cdirPos;
	endRecord.m_commentLength = 0;

	fwrite(&endRecord, 1, sizeof(endRecord), outF);

	fclose(outF);
}

class BMPDumperContext : public PortabilityLayer::QDPictEmitContext
{
public:
	bool SpecifyFrame(const Rect &rect) override;
	Rect ConstrainRegion(const Rect &rect) const override;
	void Start(PortabilityLayer::QDPictBlitSourceType sourceType, const PortabilityLayer::QDPictEmitScanlineParameters &params) override;
	void BlitScanlineAndAdvance(const void *) override;
	bool AllocTempBuffers(uint8_t *&buffer1, size_t buffer1Size, uint8_t *&buffer2, size_t buffer2Size) override;

	bool Export(std::vector<uint8_t> &outData) const;

private:
	std::vector<PortabilityLayer::RGBAColor> m_pixelData;
	size_t m_blitOrigin;
	size_t m_pitchInElements;
	Rect m_frame;
	PortabilityLayer::QDPictEmitScanlineParameters m_blitParams;
	PortabilityLayer::QDPictBlitSourceType m_blitSourceType;

	std::vector<uint8_t> m_tempBuffers;
};

bool BMPDumperContext::SpecifyFrame(const Rect &rect)
{
	if (!rect.IsValid())
		return false;

	m_frame = rect;
	m_pitchInElements = rect.Width();
	m_pixelData.resize(m_pitchInElements * rect.Height());
	for (PortabilityLayer::RGBAColor &color : m_pixelData)
		color = PortabilityLayer::RGBAColor::Create(0, 0, 0, 255);

	return true;
}

Rect BMPDumperContext::ConstrainRegion(const Rect &rect) const
{
	return m_frame.Intersect(rect);
}

void BMPDumperContext::Start(PortabilityLayer::QDPictBlitSourceType sourceType, const PortabilityLayer::QDPictEmitScanlineParameters &params)
{
	m_blitSourceType = sourceType;
	m_blitParams = params;

	int32_t relativeLeft = params.m_constrainedRegionLeft - m_frame.left;
	int32_t relativeTop = params.m_firstY - m_frame.top;

	m_blitOrigin = (static_cast<uint32_t>(relativeTop) * m_frame.Width()) + static_cast<uint32_t>(relativeLeft);
}

void BMPDumperContext::BlitScanlineAndAdvance(const void *scanlineData)
{
	const uint8_t *scanlineBytes = static_cast<const uint8_t *>(scanlineData);
	const size_t rowSize = m_blitParams.m_constrainedRegionRight - m_blitParams.m_constrainedRegionLeft;
	PortabilityLayer::RGBAColor *outRowStart = &m_pixelData[m_blitOrigin];

	m_blitOrigin += m_pitchInElements;
	const size_t planarSeparation = m_blitParams.m_planarSeparation;

	switch (m_blitSourceType)
	{
	case PortabilityLayer::QDPictBlitSourceType_1Bit:
		for (size_t i = 0; i < rowSize; i++)
		{
			if (scanlineBytes[i / 8] & (0x80 >> (i & 7)))
				outRowStart[i] = PortabilityLayer::RGBAColor::Create(0, 0, 0, 255);
			else
				outRowStart[i] = PortabilityLayer::RGBAColor::Create(255, 255, 255, 255);
		}
		break;
	case PortabilityLayer::QDPictBlitSourceType_Indexed1Bit:
		for (size_t i = 0; i < rowSize; i++)
		{
			const unsigned int colorIndex = (scanlineBytes[i / 8] >> (8 - ((i & 7) + 1) * 1)) & 1;
			outRowStart[i] = m_blitParams.m_colors[colorIndex];
		}
		break;
	case PortabilityLayer::QDPictBlitSourceType_Indexed2Bit:
		for (size_t i = 0; i < rowSize; i++)
		{
			const unsigned int colorIndex = (scanlineBytes[i / 4] >> (8 - ((i & 3) + 1) * 2)) & 3;
			outRowStart[i] = m_blitParams.m_colors[colorIndex];
		}
		break;
	case PortabilityLayer::QDPictBlitSourceType_Indexed4Bit:
		for (size_t i = 0; i < rowSize; i++)
		{
			const unsigned int colorIndex = (scanlineBytes[i / 2] >> (8 - ((i & 1) + 1) * 4)) & 15;
			outRowStart[i] = m_blitParams.m_colors[colorIndex];
		}
		break;
	case PortabilityLayer::QDPictBlitSourceType_Indexed8Bit:
		for (size_t i = 0; i < rowSize; i++)
		{
			const unsigned int colorIndex = scanlineBytes[i];
			outRowStart[i] = m_blitParams.m_colors[colorIndex];
		}
		break;
	case PortabilityLayer::QDPictBlitSourceType_RGB15:
		for (size_t i = 0; i < rowSize; i++)
		{
			const uint16_t item = *reinterpret_cast<const uint16_t*>(scanlineBytes + i * 2);
			PortabilityLayer::RGBAColor &outputItem = outRowStart[i];

			outputItem.b = FiveToEight(item & 0x1f);
			outputItem.g = FiveToEight((item >> 5) & 0x1f);
			outputItem.r = FiveToEight((item >> 10) & 0x1f);
			outputItem.a = 255;
		}
		break;
	case PortabilityLayer::QDPictBlitSourceType_RGB24_Interleaved:
		for (size_t i = 0; i < rowSize; i++)
		{
			PortabilityLayer::RGBAColor &outputItem = outRowStart[i];

			outputItem.r = scanlineBytes[i * 3 + 0];
			outputItem.g = scanlineBytes[i * 3 + 1];
			outputItem.b = scanlineBytes[i * 3 + 2];
			outputItem.a = 255;
		}
		break;
	case PortabilityLayer::QDPictBlitSourceType_RGB24_Multiplane:
		for (size_t i = 0; i < rowSize; i++)
		{
			PortabilityLayer::RGBAColor &outputItem = outRowStart[i];

			outputItem.r = scanlineBytes[i];
			outputItem.g = scanlineBytes[i + planarSeparation];
			outputItem.b = scanlineBytes[i + planarSeparation * 2];
			outputItem.a = 255;
		}
		break;
	}
}

bool BMPDumperContext::AllocTempBuffers(uint8_t *&buffer1, size_t buffer1Size, uint8_t *&buffer2, size_t buffer2Size)
{
	m_tempBuffers.resize(buffer1Size + buffer2Size);

	if (m_tempBuffers.size() == 0)
	{
		buffer1 = nullptr;
		buffer2 = nullptr;
	}
	else
	{
		buffer1 = &m_tempBuffers[0];
		buffer2 = buffer1 + buffer1Size;
	}

	return true;
}

bool BMPDumperContext::Export(std::vector<uint8_t> &outData) const
{
	outData.resize(0);

	bool couldBe15Bit = true;
	bool couldBeIndexed = true;

	PortabilityLayer::BitmapColorTableEntry colorTable[256];
	unsigned int numColors = 0;

	const size_t height = m_frame.Height();
	const size_t width = m_frame.Width();
	const size_t pitch = m_pitchInElements;

	for (size_t row = 0; row < height; row++)
	{
		const PortabilityLayer::RGBAColor *rowData = &m_pixelData[m_pitchInElements * row];

		for (size_t col = 0; col < width; col++)
		{
			const PortabilityLayer::RGBAColor &pixel = rowData[col];

			if (couldBe15Bit)
			{
				if (FiveToEight(pixel.r >> 3) != pixel.r || FiveToEight(pixel.g >> 3) != pixel.g || FiveToEight(pixel.b >> 3) != pixel.b)
				{
					couldBe15Bit = false;
					if (!couldBeIndexed)
						break;
				}
			}

			if (couldBeIndexed)
			{
				bool matched = false;
				for (unsigned int ci = 0; ci < numColors; ci++)
				{
					const PortabilityLayer::BitmapColorTableEntry &ctabEntry = colorTable[ci];

					if (ctabEntry.m_r == pixel.r && ctabEntry.m_g == pixel.g && ctabEntry.m_b == pixel.b)
					{
						matched = true;
						break;
					}
				}

				if (matched == false)
				{
					if (numColors == 256)
					{
						couldBeIndexed = false;
						if (!couldBe15Bit)
							break;
					}
					else
					{
						PortabilityLayer::BitmapColorTableEntry &ctabEntry = colorTable[numColors++];
						ctabEntry.m_r = pixel.r;
						ctabEntry.m_g = pixel.g;
						ctabEntry.m_b = pixel.b;
						ctabEntry.m_reserved = 0;
					}
				}
			}
		}

		if (!couldBeIndexed && !couldBe15Bit)
			break;
	}

	unsigned int bpp = 24;
	if (couldBeIndexed)
	{
		if (numColors <= 2)
			bpp = 1;
		else if (numColors <= 16)
			bpp = 4;
		else
			bpp = 8;
	}
	else if (couldBe15Bit)
		bpp = 16;

	const size_t minimalBitsPerRow = bpp * width;
	const size_t rowPitchBytes = ((minimalBitsPerRow + 31) / 32) * 4;	// DWORD alignment

	const size_t colorTableSize = (bpp < 16) ? numColors * 4 : 0;
	const size_t fileHeaderSize = sizeof(PortabilityLayer::BitmapFileHeader);
	const size_t infoHeaderSize = sizeof(PortabilityLayer::BitmapInfoHeader);
	const size_t ctabSize = (bpp < 16) ? (numColors * 4) : 0;
	const size_t imageDataSize = rowPitchBytes * height;
	const size_t postCTabPaddingSize = 2;
	const size_t imageFileSize = fileHeaderSize + infoHeaderSize + ctabSize + postCTabPaddingSize + imageDataSize;

	outData.reserve(imageFileSize);

	PortabilityLayer::BitmapFileHeader fileHeader;
	fileHeader.m_id[0] = 'B';
	fileHeader.m_id[1] = 'M';
	fileHeader.m_fileSize = static_cast<uint32_t>(imageFileSize);
	fileHeader.m_imageDataStart = static_cast<uint32_t>(fileHeaderSize + infoHeaderSize + ctabSize + postCTabPaddingSize);
	fileHeader.m_reserved1 = 0;
	fileHeader.m_reserved2 = 0;

	VectorAppend(outData, reinterpret_cast<const uint8_t*>(&fileHeader), sizeof(fileHeader));

	PortabilityLayer::BitmapInfoHeader infoHeader;
	infoHeader.m_thisStructureSize = sizeof(infoHeader);
	infoHeader.m_width = static_cast<uint32_t>(width);
	infoHeader.m_height = static_cast<uint32_t>(height);
	infoHeader.m_planes = 1;
	infoHeader.m_bitsPerPixel = bpp;
	infoHeader.m_compression = PortabilityLayer::BitmapConstants::kCompressionRGB;
	infoHeader.m_imageSize = static_cast<uint32_t>(imageDataSize);
	infoHeader.m_xPixelsPerMeter = 2835;
	infoHeader.m_yPixelsPerMeter = 2835;
	infoHeader.m_numColors = (bpp < 16) ? numColors : 0;
	infoHeader.m_importantColorCount = 0;

	VectorAppend(outData, reinterpret_cast<const uint8_t*>(&infoHeader), sizeof(infoHeader));

	VectorAppend(outData, reinterpret_cast<const uint8_t*>(colorTable), sizeof(PortabilityLayer::BitmapColorTableEntry) * numColors);

	for (size_t i = 0; i < postCTabPaddingSize; i++)
		outData.push_back(0);

	std::vector<uint8_t> rowPackData;
	rowPackData.resize(rowPitchBytes);

	for (size_t row = 0; row < height; row++)
	{
		for (size_t i = 0; i < rowPitchBytes; i++)
			rowPackData[i] = 0;

		const PortabilityLayer::RGBAColor *rowData = &m_pixelData[m_pitchInElements * (height - 1 - row)];

		for (size_t col = 0; col < width; col++)
		{
			const PortabilityLayer::RGBAColor &pixel = rowData[col];

			if (bpp == 24)
			{
				rowPackData[col * 3 + 0] = pixel.b;
				rowPackData[col * 3 + 1] = pixel.g;
				rowPackData[col * 3 + 2] = pixel.r;
			}
			else if (bpp == 16)
			{
				int packedValue = 0;
				packedValue |= (pixel.b >> 3);
				packedValue |= ((pixel.g >> 3) << 5);
				packedValue |= ((pixel.r >> 3) << 10);

				rowPackData[col * 2 + 0] = static_cast<uint8_t>(packedValue & 0xff);
				rowPackData[col * 2 + 1] = static_cast<uint8_t>((packedValue >> 8) & 0xff);
			}
			else
			{
				unsigned int colorIndex = 0;
				for (unsigned int ci = 0; ci < numColors; ci++)
				{
					const PortabilityLayer::BitmapColorTableEntry &ctabEntry = colorTable[ci];

					if (ctabEntry.m_r == pixel.r && ctabEntry.m_g == pixel.g && ctabEntry.m_b == pixel.b)
					{
						colorIndex = ci;
						break;
					}
				}

				if (bpp == 8)
					rowPackData[col] = colorIndex;
				else if (bpp == 4)
					rowPackData[col / 2] |= (colorIndex << (8 - (((col & 1) + 1) * 4)));
				else if (bpp == 1)
				{
					if (colorIndex)
						rowPackData[col / 8] |= (0x80 >> (col & 7));
				}
			}
		}

		VectorAppend(outData, &rowPackData[0], rowPackData.size());
	}

	return true;
}

bool ImportPICT(std::vector<uint8_t> &outBMP, const void *inData, size_t inSize)
{
	PortabilityLayer::MemReaderStream stream(inData, inSize);
	BMPDumperContext context;

	PortabilityLayer::QDPictDecoder decoder;
	if (decoder.DecodePict(&stream, &context))
	{
		if (context.Export(outBMP))
			return true;

		return false;
	}
	else
		return false;
}

size_t PadRiffChunk(size_t sz)
{
	return sz + (sz & 1);
}

void PadAlignWave(std::vector<uint8_t> &outWAV)
{
	if (outWAV.size() & 1)
		outWAV.push_back(0);
}

bool ImportSound(std::vector<uint8_t> &outWAV, const void *inData, size_t inSize)
{
	// Glider PRO has a hard-coded expectation that the sound will have exactly 20 bytes of prefix.
	// The resource format can have more than that, we'll just follow this base expectation
	const size_t hardCodedPrefixSize = 20;

	struct BufferHeader
	{
		BEUInt32_t m_samplePtr;
		BEUInt32_t m_length;
		BEFixed32_t m_sampleRate;
		BEUInt32_t m_loopStart;
		BEUInt32_t m_loopEnd;
		uint8_t m_encoding;
		uint8_t m_baseFrequency;
	};

	if (inSize < hardCodedPrefixSize)
		return false;

	const uint8_t *sndBufferData = static_cast<const uint8_t*>(inData) + hardCodedPrefixSize;
	inSize -= hardCodedPrefixSize;

	if (inSize < sizeof(BufferHeader))
		return false;

	BufferHeader header;
	memcpy(&header, sndBufferData, sizeof(header));

	sndBufferData += sizeof(header);
	inSize -= sizeof(header);

	uint32_t dataLength = header.m_length;
	if (dataLength > inSize)
		return false;

	uint32_t sampleRate = header.m_sampleRate.m_intPart;
	if (static_cast<int>(header.m_sampleRate.m_fracPart) >= 0x8000)
		sampleRate++;

	PortabilityLayer::WaveFormatChunkV1 formatChunk;

	const size_t riffTagSize = sizeof(PortabilityLayer::RIFFTag);
	const size_t waveMarkerSize = 4;
	const size_t fmtTagSize = sizeof(PortabilityLayer::RIFFTag);
	const size_t fmtContentSize = PadRiffChunk(sizeof(formatChunk));
	const size_t dataTagSize = sizeof(PortabilityLayer::RIFFTag);
	const size_t dataContentSize = PadRiffChunk(dataLength);

	// Structure:
	// riffTag
	//   waveMarker
	//   fmtTag
	//     fmtContent
	//   dataTag
	//     dataContent

	outWAV.resize(0);
	outWAV.reserve(riffTagSize + waveMarkerSize + fmtTagSize + fmtContentSize + dataTagSize + dataContentSize);

	PortabilityLayer::RIFFTag riffTag;
	riffTag.m_tag = PortabilityLayer::WaveConstants::kRiffChunkID;
	riffTag.m_chunkSize = static_cast<uint32_t>(waveMarkerSize + fmtTagSize + fmtContentSize + dataTagSize + dataContentSize);

	VectorAppend(outWAV, reinterpret_cast<const uint8_t*>(&riffTag), sizeof(riffTag));

	VectorAppend(outWAV, reinterpret_cast<const uint8_t*>("WAVE"), 4);

	PortabilityLayer::RIFFTag formatTag;
	formatTag.m_tag = PortabilityLayer::WaveConstants::kFormatChunkID;
	formatTag.m_chunkSize = sizeof(formatChunk);

	VectorAppend(outWAV, reinterpret_cast<const uint8_t*>(&formatTag), sizeof(formatTag));

	formatChunk.m_formatCode = PortabilityLayer::WaveConstants::kFormatPCM;
	formatChunk.m_numChannels = 1;
	formatChunk.m_sampleRate = sampleRate;
	formatChunk.m_bytesPerSecond = sampleRate;
	formatChunk.m_blockAlignmentBytes = 1;	// channels * bits per sample / 8
	formatChunk.m_bitsPerSample = 8;

	VectorAppend(outWAV, reinterpret_cast<const uint8_t*>(&formatChunk), sizeof(formatChunk));
	PadAlignWave(outWAV);

	PortabilityLayer::RIFFTag dataTag;
	dataTag.m_tag = PortabilityLayer::WaveConstants::kDataChunkID;
	dataTag.m_chunkSize = dataLength;

	VectorAppend(outWAV, reinterpret_cast<const uint8_t*>(&dataTag), sizeof(dataTag));

	VectorAppend(outWAV, sndBufferData, dataLength);
	PadAlignWave(outWAV);

	return true;
}

int main(int argc, const char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: gpr2gpa <input.gpr> <output.gpa>");
		return -1;
	}

	FILE *inF = nullptr;
	if (fopen_s(&inF, argv[1], "rb"))
	{
		fprintf(stderr, "Error opening input file");
		return -1;
	}

	PortabilityLayer::CFileStream cfs(inF);

	PortabilityLayer::ResourceFile *resFile = PortabilityLayer::ResourceFile::Create();
	resFile->Load(&cfs);
	cfs.Close();

	PortabilityLayer::ResourceCompiledTypeList *typeLists = nullptr;
	size_t typeListCount = 0;
	resFile->GetAllResourceTypeLists(typeLists, typeListCount);

	std::vector<PlannedEntry> contents;

	const PortabilityLayer::ResTypeID pictTypeID = PortabilityLayer::ResTypeID('PICT');
	const PortabilityLayer::ResTypeID sndTypeID = PortabilityLayer::ResTypeID('snd ');

	for (size_t tlIndex = 0; tlIndex < typeListCount; tlIndex++)
	{
		const PortabilityLayer::ResourceCompiledTypeList &typeList = typeLists[tlIndex];
		const PortabilityLayer::GpArcResourceTypeTag resTag = PortabilityLayer::GpArcResourceTypeTag::Encode(typeList.m_resType);

		const PortabilityLayer::ResourceCompiledRef *refList = typeList.m_firstRef;
		const size_t numRefs = typeList.m_numRefs;

		{
			char subName[256];
			sprintf_s(subName, "%s", resTag.m_id);

			PlannedEntry entry;
			entry.m_name = subName;
			entry.m_isDirectory = true;

			contents.push_back(entry);
		}

		for (size_t rlIndex = 0; rlIndex < numRefs; rlIndex++)
		{
			const PortabilityLayer::ResourceCompiledRef &res = refList[rlIndex];
			const void *resData = res.m_resData;
			const size_t resSize = res.GetSize();

#if 0
			if (typeList.m_resType == pictTypeID)
			{
				PlannedEntry entry;
				char resName[256];
				sprintf_s(resName, "%s/%i.bmp", resTag.m_id, static_cast<int>(res.m_resID));

				entry.m_name = resName;

				if (ImportPICT(entry.m_contents, resData, resSize))
					contents.push_back(entry);
			}
			else
#endif
			if (typeList.m_resType == sndTypeID)
			{
				PlannedEntry entry;
				char resName[256];
				sprintf_s(resName, "%s/%i.wav", resTag.m_id, static_cast<int>(res.m_resID));

				entry.m_name = resName;

				if (ImportSound(entry.m_contents, resData, resSize))
					contents.push_back(entry);
			}
			else
			{
				PlannedEntry entry;

				char resName[256];
				sprintf_s(resName, "%s/%i.bin", resTag.m_id, static_cast<int>(res.m_resID));

				entry.m_name = resName;
				entry.m_contents.resize(res.GetSize());

				memcpy(&entry.m_contents[0], resData, resSize);

				contents.push_back(entry);
			}
		}
	}

	ExportZipFile(argv[2], contents);

	resFile->Destroy();

	return 0;
}
