#include "WindowsUnicodeToolShim.h"

#include "CFileStream.h"
#include "CombinedTimestamp.h"
#include "DeflateCodec.h"
#include "MacFileInfo.h"
#include "ZipFile.h"
#include "GpAllocator_C.h"
#include "PLDrivers.h"

#include <stdio.h>
#include <string>
#include <algorithm>

int toolMain(int argc, const char **argv)
{
	GpDriverCollection *drivers = PLDrivers::GetDriverCollection();
	drivers->SetDriver<GpDriverIDs::kAlloc>(GpAllocator_C::GetInstance());

	if (argc != 2)
	{
		fprintf(stderr, "Usage: MergeGPF <file.gpf>");
		return -1;
	}

	std::string metaFileName = argv[1];
	if (metaFileName.length() < 5)
	{
		fprintf(stderr, "Usage: MergeGPF <file.gpf>");
		return -1;
	}

	std::string fnameBase = metaFileName.substr(0, metaFileName.length() - 4);

	std::string resName = fnameBase + ".gpa";
	std::string dataName = fnameBase + ".gpd";

	PortabilityLayer::MacFilePropertiesSerialized mfps;

	PortabilityLayer::ZipFileLocalHeader metaLH;

	{
		FILE *metaF = fopen_utf8(argv[1], "rb");

		PortabilityLayer::CFileStream metaStream(metaF);

		if (!mfps.ReadFromPackage(metaStream))
		{
			fprintf(stderr, "Error reading metadata");
			return -1;
		}

		metaStream.SeekStart(0);
		metaStream.Read(&metaLH, sizeof(metaLH));
		metaStream.Close();
	}

	PortabilityLayer::MacFileProperties mfp;
	mfps.Deserialize(mfp);

	mfps.Serialize(mfp);

	size_t insertedMetaFSize = sizeof(metaLH) + strlen(PortabilityLayer::MacFilePropertiesSerialized::GetPackagedName()) + sizeof(mfps.m_data);
	size_t insertedDataSize = 0;

	FILE *mergedF = fopen_utf8(argv[1], "wb");
	if (!mergedF)
	{
		fprintf(stderr, "Error reopening metadata file");
		return -1;
	}

	PortabilityLayer::CFileStream mergedStream(mergedF);

	uint32_t numFiles = 0;
	uint32_t cdirSize = 0;
	const char *metaPackagedName = PortabilityLayer::MacFilePropertiesSerialized::GetPackagedName();
	const char *dataPackagedName = "!data";
	
	PortabilityLayer::ZipCentralDirectoryFileHeader metaCDir;
	PortabilityLayer::ZipCentralDirectoryFileHeader dataCDir;

	bool hasData = false;

	// Write metadata
	{
		GpUFilePos_t metaLHStart = mergedStream.Tell();

		PortabilityLayer::ZipFileLocalHeader metaCopyLH;

		metaCopyLH.m_signature = PortabilityLayer::ZipFileLocalHeader::kSignature;
		metaCopyLH.m_versionRequired = PortabilityLayer::ZipConstants::kStoredRequiredVersion;
		metaCopyLH.m_flags = 0;
		metaCopyLH.m_method = PortabilityLayer::ZipConstants::kStoredMethod;
		metaCopyLH.m_modificationTime = metaLH.m_modificationTime;
		metaCopyLH.m_modificationDate = metaLH.m_modificationDate;
		metaCopyLH.m_crc = PortabilityLayer::DeflateContext::CRC32(0, mfps.m_data, sizeof(mfps.m_data));
		metaCopyLH.m_compressedSize = sizeof(mfps.m_data);
		metaCopyLH.m_uncompressedSize = sizeof(mfps.m_data);
		metaCopyLH.m_fileNameLength = strlen(metaPackagedName);
		metaCopyLH.m_extraFieldLength = 0;

		if (!mergedStream.WriteExact(&metaCopyLH, sizeof(metaCopyLH)) || !mergedStream.WriteExact(metaPackagedName, strlen(metaPackagedName)) || !mergedStream.WriteExact(mfps.m_data, sizeof(mfps.m_data)))
		{
			fprintf(stderr, "Error writing metadata");
			return -1;
		}

		numFiles++;
		cdirSize += sizeof(PortabilityLayer::ZipCentralDirectoryFileHeader) + strlen(metaPackagedName);


		metaCDir.m_signature = PortabilityLayer::ZipCentralDirectoryFileHeader::kSignature;
		metaCDir.m_versionCreated = PortabilityLayer::ZipConstants::kCompressedRequiredVersion;
		metaCDir.m_versionRequired = PortabilityLayer::ZipConstants::kStoredRequiredVersion;
		metaCDir.m_flags = 0;
		metaCDir.m_method = PortabilityLayer::ZipConstants::kStoredMethod;
		metaCDir.m_modificationTime = metaLH.m_modificationTime;
		metaCDir.m_modificationDate = metaLH.m_modificationDate;
		metaCDir.m_crc = metaLH.m_crc;
		metaCDir.m_compressedSize = metaLH.m_compressedSize;
		metaCDir.m_uncompressedSize = metaLH.m_uncompressedSize;
		metaCDir.m_fileNameLength = metaLH.m_fileNameLength;
		metaCDir.m_extraFieldLength = metaLH.m_extraFieldLength;
		metaCDir.m_commentLength = 0;
		metaCDir.m_diskNumber = 0;
		metaCDir.m_internalAttributes = 0;
		metaCDir.m_externalAttributes = PortabilityLayer::ZipConstants::kArchivedAttributes;
		metaCDir.m_localHeaderOffset = metaLHStart;
	}

	FILE *dataF = fopen_utf8(dataName.c_str(), "rb");
	if (dataF)
	{
		GpUFilePos_t dataLHStart = mergedStream.Tell();
		PortabilityLayer::ZipFileLocalHeader dataLH;

		dataLH.m_signature = PortabilityLayer::ZipFileLocalHeader::kSignature;
		dataLH.m_versionRequired = PortabilityLayer::ZipConstants::kCompressedRequiredVersion;
		dataLH.m_flags = 0;
		dataLH.m_method = PortabilityLayer::ZipConstants::kDeflatedMethod;
		dataLH.m_modificationTime = metaLH.m_modificationTime;
		dataLH.m_modificationDate = metaLH.m_modificationDate;
		dataLH.m_crc = 0;
		dataLH.m_compressedSize = 0;
		dataLH.m_uncompressedSize = 0;
		dataLH.m_fileNameLength = strlen(dataPackagedName);
		dataLH.m_extraFieldLength = 0;

		if (!mergedStream.WriteExact(&dataLH, sizeof(dataLH)) || !mergedStream.WriteExact(dataPackagedName, strlen(dataPackagedName)))
		{
			fprintf(stderr, "Error compressing data");
			return -1;
		}

		PortabilityLayer::DeflateContext *ctx = PortabilityLayer::DeflateContext::Create(&mergedStream, 9);

		uint8_t compressBuffer[1024];
		uint32_t crc = 0;
		size_t uncompressedSize = 0;
		GpUFilePos_t compressedDataStart = mergedStream.Tell();
		for (;;)
		{
			size_t dataRead = fread(compressBuffer, 1, sizeof(compressBuffer), dataF);

			if (dataRead == 0)
				break;

			uncompressedSize += dataRead;

			if (!ctx->Append(compressBuffer, dataRead))
			{
				fprintf(stderr, "Error compressing data");
				return -1;
			}

			crc = PortabilityLayer::DeflateContext::CRC32(crc, compressBuffer, dataRead);
		}

		if (!ctx->Flush())
		{
			fprintf(stderr, "Error compressing data");
			return -1;
		}

		ctx->Destroy();

		GpUFilePos_t compressedDataEnd = mergedStream.Tell();

		dataLH.m_crc = crc;
		dataLH.m_compressedSize = (compressedDataEnd - compressedDataStart);
		dataLH.m_uncompressedSize = uncompressedSize;

		if (!mergedStream.SeekStart(dataLHStart) || !mergedStream.Write(&dataLH, sizeof(dataLH)) || !mergedStream.SeekStart(compressedDataEnd))
		{
			fprintf(stderr, "Error compressing data");
			return -1;
		}

		numFiles++;
		cdirSize += sizeof(PortabilityLayer::ZipCentralDirectoryFileHeader) + strlen(dataPackagedName);
		hasData = true;

		fclose(dataF);

		insertedDataSize += sizeof(dataLH) + strlen(dataPackagedName) + (compressedDataEnd - compressedDataStart);

		dataCDir.m_signature = PortabilityLayer::ZipCentralDirectoryFileHeader::kSignature;
		dataCDir.m_versionCreated = PortabilityLayer::ZipConstants::kCompressedRequiredVersion;
		dataCDir.m_versionRequired = PortabilityLayer::ZipConstants::kCompressedRequiredVersion;
		dataCDir.m_flags = 0;
		dataCDir.m_method = PortabilityLayer::ZipConstants::kDeflatedMethod;
		dataCDir.m_modificationTime = dataLH.m_modificationTime;
		dataCDir.m_modificationDate = dataLH.m_modificationDate;
		dataCDir.m_crc = dataLH.m_crc;
		dataCDir.m_compressedSize = dataLH.m_compressedSize;
		dataCDir.m_uncompressedSize = dataLH.m_uncompressedSize;
		dataCDir.m_fileNameLength = dataLH.m_fileNameLength;
		dataCDir.m_extraFieldLength = dataLH.m_extraFieldLength;
		dataCDir.m_commentLength = 0;
		dataCDir.m_diskNumber = 0;
		dataCDir.m_internalAttributes = 0;
		dataCDir.m_externalAttributes = PortabilityLayer::ZipConstants::kArchivedAttributes;
		dataCDir.m_localHeaderOffset = dataLHStart;
	}

	std::vector<PortabilityLayer::ZipCentralDirectoryFileHeader> resCentralDir;
	std::vector<uint8_t> fileNameBytes;
	std::vector<size_t> fileNameSizes;

	FILE *resF = fopen_utf8(resName.c_str(), "rb");
	{
		PortabilityLayer::ZipEndOfCentralDirectoryRecord eocd;

		PortabilityLayer::CFileStream resStream(resF);

		if (!resStream.SeekEnd(sizeof(PortabilityLayer::ZipEndOfCentralDirectoryRecord)) || !resStream.ReadExact(&eocd, sizeof(eocd)) || !resStream.SeekStart(eocd.m_centralDirStartOffset))
		{
			fprintf(stderr, "Error reading res data");
			return -1;
		}

		size_t numResFiles = eocd.m_numCentralDirRecords;

		for (size_t i = 0; i < numResFiles; i++)
		{
			PortabilityLayer::ZipCentralDirectoryFileHeader cdirFile;

			if (!resStream.ReadExact(&cdirFile, sizeof(cdirFile)))
			{
				fprintf(stderr, "Error reading cdir entry");
				return -1;
			}

			size_t fileNameLength = cdirFile.m_fileNameLength;
			fileNameSizes.push_back(fileNameLength);

			if (fileNameLength > 0)
			{
				fileNameBytes.resize(fileNameBytes.size() + fileNameLength);
				if (!resStream.Read(&fileNameBytes[fileNameBytes.size() - fileNameLength], fileNameLength))
				{
					fprintf(stderr, "Error reading cdir entry");
					return -1;
				}
			}

			if (!resStream.SeekCurrent(cdirFile.m_extraFieldLength + cdirFile.m_commentLength))
			{
				fprintf(stderr, "Error reading cdir entry");
				return -1;
			}

			resCentralDir.push_back(cdirFile);

			numFiles++;
			cdirSize += sizeof(PortabilityLayer::ZipCentralDirectoryFileHeader) + fileNameLength;
		}

		for (size_t i = 0; i < resCentralDir.size(); i++)
		{
			PortabilityLayer::ZipCentralDirectoryFileHeader &cdirHeader = resCentralDir[i];

			PortabilityLayer::ZipFileLocalHeader resLH;

			if (!resStream.SeekStart(cdirHeader.m_localHeaderOffset) || !resStream.ReadExact(&resLH, sizeof(resLH)) || resLH.m_fileNameLength != cdirHeader.m_fileNameLength || resLH.m_compressedSize != cdirHeader.m_compressedSize || resLH.m_uncompressedSize != cdirHeader.m_uncompressedSize)
			{
				fprintf(stderr, "Error reading res");
				return -1;
			}

			size_t chunkSizes[] = { resLH.m_fileNameLength, resLH.m_extraFieldLength, resLH.m_compressedSize };

			resLH.m_extraFieldLength = 0;

			cdirHeader.m_localHeaderOffset = mergedStream.Tell();
			cdirHeader.m_extraFieldLength = 0;
			cdirHeader.m_commentLength = 0;

			if (!mergedStream.WriteExact(&resLH, sizeof(resLH)))
			{
				fprintf(stderr, "Error copying resource header");
				return -1;
			}

			for (int chunk = 0; chunk < 3; chunk++)
			{
				size_t chunkCopySize = chunkSizes[chunk];

				if (chunk == 1)
				{
					// Strip extra field
					if (!resStream.SeekCurrent(chunkCopySize))
					{
						fprintf(stderr, "Error copying resource");
						return -1;
					}
				}
				else
				{
					uint8_t copyBuffer[1024];
					while (chunkCopySize > 0)
					{
						size_t amountToCopy = std::min(sizeof(copyBuffer), chunkCopySize);

						if (!resStream.ReadExact(copyBuffer, amountToCopy) || !mergedStream.WriteExact(copyBuffer, amountToCopy))
						{
							fprintf(stderr, "Error copying resource");
							return -1;
						}

						chunkCopySize -= amountToCopy;
					}
				}
			}
		}

		resStream.Close();
	}

	GpUFilePos_t cdirPos = mergedStream.Tell();

	// Write metadata cdir
	if (!mergedStream.WriteExact(&metaCDir, sizeof(metaCDir)) || !mergedStream.WriteExact(metaPackagedName, strlen(metaPackagedName)))
	{
		fprintf(stderr, "Error writing directory");
		return -1;
	}

	if (hasData)
	{
		if (!mergedStream.WriteExact(&dataCDir, sizeof(dataCDir)) || !mergedStream.WriteExact(dataPackagedName, strlen(dataPackagedName)))
		{
			fprintf(stderr, "Error writing directory");
			return -1;
		}
	}

	size_t fnameBytesOffset = 0;
	for (size_t i = 0; i < resCentralDir.size(); i++)
	{
		size_t fnameSize = fileNameSizes[i];
		const PortabilityLayer::ZipCentralDirectoryFileHeader &cdir = resCentralDir[i];

		if (!mergedStream.WriteExact(&cdir, sizeof(cdir)) || (fnameSize > 0 && !mergedStream.WriteExact(&fileNameBytes[fnameBytesOffset], fnameSize)))
		{
			fprintf(stderr, "Error writing directory");
			return -1;
		}

		fnameBytesOffset += fnameSize;
	}

	PortabilityLayer::ZipEndOfCentralDirectoryRecord eocd;
	eocd.m_signature = PortabilityLayer::ZipEndOfCentralDirectoryRecord::kSignature;
	eocd.m_thisDiskNumber = 0;
	eocd.m_centralDirDisk = 0;
	eocd.m_numCentralDirRecordsThisDisk = numFiles;
	eocd.m_numCentralDirRecords = numFiles;
	eocd.m_centralDirectorySizeBytes = mergedStream.Tell() - cdirPos;
	eocd.m_centralDirStartOffset = cdirPos;
	eocd.m_commentLength = 0;

	if (!mergedStream.WriteExact(&eocd, sizeof(eocd)))
	{
		fprintf(stderr, "Error writing EOCD");
		return -1;
	}

	mergedStream.Close();

	return 0;
}
