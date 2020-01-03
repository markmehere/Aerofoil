#include "ResourceFile.h"
#include "BinarySearch.h"
#include "ByteSwap.h"
#include "MacFileMem.h"
#include "MemoryManager.h"
#include "MemReaderStream.h"
#include "ResourceCompiledRef.h"
#include "ResourceCompiledTypeList.h"

#include <algorithm>

namespace PortabilityLayer
{
	ResourceFile::ResourceFile()
		: m_resDataBlob(nullptr)
		, m_resDataBlobSize(0)
		, m_resNameBlob(nullptr)
		, m_resNameBlobSize(0)
		, m_compiledRefBlob(nullptr)
		, m_numResources(0)
		, m_compiledTypeListBlob(nullptr)
		, m_numResourceTypes(0)
	{
	}

	ResourceFile::~ResourceFile()
	{
		if (m_resNameBlob)
			delete[] m_resNameBlob;

		if (m_resDataBlob)
			delete[] m_resDataBlob;

		if (m_compiledRefBlob)
			delete[] m_compiledRefBlob;

		if (m_compiledTypeListBlob)
			delete[] m_compiledTypeListBlob;
	}

	bool ResourceFile::Load(IOStream *stream)
	{
		struct ResourceHeader
		{
			uint32_t m_resDataOffset;
			uint32_t m_resMapOffset;
			uint32_t m_resDataSize;
			uint32_t m_resMapSize;
		};

		const UFilePos_t streamSize = stream->Size();
		if (streamSize > UINT32_MAX)
			return false;

		uint32_t resForkSize = static_cast<uint32_t>(streamSize);

		ResourceHeader resourceHeader;

		if (stream->Read(&resourceHeader, sizeof(ResourceHeader)) != sizeof(ResourceHeader))
			return false;

		ByteSwap::BigUInt32(resourceHeader.m_resDataOffset);
		ByteSwap::BigUInt32(resourceHeader.m_resMapOffset);
		ByteSwap::BigUInt32(resourceHeader.m_resDataSize);
		ByteSwap::BigUInt32(resourceHeader.m_resMapSize);

		if (resourceHeader.m_resDataOffset > resForkSize)
			return false;

		if (resourceHeader.m_resMapOffset > resForkSize)
			return false;

		if (resForkSize - resourceHeader.m_resDataOffset < resourceHeader.m_resDataSize)
			return false;

		if (resForkSize - resourceHeader.m_resMapOffset < resourceHeader.m_resMapSize)
			return false;

		if (!stream->SeekStart(resourceHeader.m_resDataOffset))
			return false;

		m_resDataBlob = new uint8_t[resourceHeader.m_resDataSize];
		m_resDataBlobSize = resourceHeader.m_resDataSize;

		if (stream->Read(m_resDataBlob, resourceHeader.m_resDataSize) != resourceHeader.m_resDataSize)
			return false;

		// The format of this is slightly different from the documented format:
		// The type list offset is the offset of the type COUNT, which takes up 2 bytes.
		// Usually the offset is 28, even though the size of the resource map including the
		// count would appear to be 30.

		struct ResourceMap
		{
			uint8_t m_reserved[16 + 4 + 2];
			uint16_t m_attributes;
			uint16_t m_typeListOffset;
			uint16_t m_nameListOffset;
		};

		struct ResourceTypeListEntry
		{
			ResTypeID m_resType;
			uint16_t m_numResourcesMinusOne;
			uint16_t m_refListOffset;
		};

		struct ResourceRefListEntry
		{
			int16_t m_resID;
			int16_t m_resourceNameOffset;
			uint8_t m_attributes;
			uint8_t m_packedResDataOffset[3];
			uint32_t m_reserved;
		};

		if (!stream->SeekStart(resourceHeader.m_resMapOffset))
			return false;

		ResourceMap resourceMap;
		if (stream->Read(&resourceMap, sizeof(ResourceMap)) != sizeof(ResourceMap))
			return false;

		ByteSwap::BigUInt16(resourceMap.m_attributes);
		ByteSwap::BigUInt16(resourceMap.m_typeListOffset);
		ByteSwap::BigUInt16(resourceMap.m_nameListOffset);

		const size_t sizeFromStartOfResMap = resForkSize - resourceHeader.m_resMapOffset;
		if (resourceMap.m_typeListOffset > sizeFromStartOfResMap)
			return false;

		if (resourceMap.m_nameListOffset > sizeFromStartOfResMap)
			return false;

		const size_t typeListOffset = resourceHeader.m_resMapOffset + resourceMap.m_typeListOffset;
		const size_t sizeFromStartOfTypeList = resForkSize - typeListOffset;

		// First pass: Count the number of references we need
		if (!stream->SeekStart(typeListOffset))
			return false;

		uint16_t numTypesMinusOne;
		if (stream->Read(&numTypesMinusOne, 2) != 2)
			return false;
		ByteSwap::BigUInt16(numTypesMinusOne);

		// numTypesMinusOne can sometimes be -1, in which case there are no resources
		m_numResourceTypes = (numTypesMinusOne + 1) & 0xffff;

		if (sizeFromStartOfTypeList < 2 || (sizeFromStartOfTypeList - 2) / 8 < m_numResourceTypes)
			return false;

		uint32_t numResourcesTotal = 0;

		for (uint32_t i = 0; i < m_numResourceTypes; i++)
		{
			ResourceTypeListEntry tlEntry;
			if (stream->Read(&tlEntry, sizeof(ResourceTypeListEntry)) != sizeof(ResourceTypeListEntry))
				return false;

			ByteSwap::BigUInt16(tlEntry.m_numResourcesMinusOne);

			const uint32_t numResourcesOfThisType = tlEntry.m_numResourcesMinusOne + 1;

			if (UINT32_MAX - numResourcesTotal < numResourcesOfThisType)
				return false;

			numResourcesTotal += numResourcesOfThisType;
		}

		if (numResourcesTotal > 0)
			m_compiledRefBlob = new ResourceCompiledRef[numResourcesTotal];

		m_numResources = numResourcesTotal;

		// Second pass: Compile references
		ResourceCompiledRef *refToCompile = m_compiledRefBlob;

		if (m_numResourceTypes > 0)
			m_compiledTypeListBlob = new ResourceCompiledTypeList[m_numResourceTypes];

		for (uint32_t i = 0; i < m_numResourceTypes; i++)
		{
			if (!stream->SeekStart(typeListOffset + 2 + i * 8))
				return false;

			ResourceTypeListEntry tlEntry;
			if (stream->Read(&tlEntry, sizeof(ResourceTypeListEntry)) != sizeof(ResourceTypeListEntry))
				return false;

			ByteSwap::BigUInt16(tlEntry.m_numResourcesMinusOne);
			ByteSwap::BigUInt16(tlEntry.m_refListOffset);

			ResourceCompiledTypeList &ctl = m_compiledTypeListBlob[i];
			ctl.m_resType = tlEntry.m_resType;
			ctl.m_firstRef = refToCompile;
			ctl.m_numRefs = tlEntry.m_numResourcesMinusOne + 1;

			if (sizeFromStartOfTypeList < tlEntry.m_refListOffset)
				return false;

			// Start reading the ref list
			if (!stream->SeekStart(typeListOffset + tlEntry.m_refListOffset))
				return false;

			const uint32_t numResourcesOfThisType = tlEntry.m_numResourcesMinusOne + 1;

			ResourceCompiledRef *firstRefOfThisType = refToCompile;

			for (uint32_t i = 0; i < numResourcesOfThisType; i++)
			{
				ResourceRefListEntry refListEntry;
				if (stream->Read(&refListEntry, sizeof(ResourceRefListEntry)) != sizeof(ResourceRefListEntry))
					return false;

				ByteSwap::BigInt16(refListEntry.m_resID);
				ByteSwap::BigInt16(refListEntry.m_resourceNameOffset);

				const uint8_t *packedOffset = refListEntry.m_packedResDataOffset;

				const size_t dataSizeOffset = (packedOffset[0] << 16) + (packedOffset[1] << 8) + packedOffset[2];

				if (dataSizeOffset > resourceHeader.m_resDataSize)
					return false;

				// Needs to be at least size 4 to decode the resource size
				if (resourceHeader.m_resDataSize - dataSizeOffset < 4)
					return false;

				const size_t dataOffset = dataSizeOffset + 4;

				refToCompile->m_attributes = refListEntry.m_attributes;
				refToCompile->m_resData = m_resDataBlob + dataOffset;
				refToCompile->m_resID = refListEntry.m_resID;
				refToCompile->m_resNameOffset = refListEntry.m_resourceNameOffset;
				refToCompile->m_handle = nullptr;

				uint32_t resSize;
				memcpy(&resSize, m_resDataBlob + dataSizeOffset, 4);

				ByteSwap::BigUInt32(resSize);

				if (resSize > resourceHeader.m_resDataSize)
					return false;

				if (resourceHeader.m_resDataSize - resSize < dataOffset)
					return false;

				refToCompile++;
			}

			std::sort(firstRefOfThisType, refToCompile, CompiledRefSortPredicate);

			for (uint32_t i = 1; i < numResourcesOfThisType; i++)
			{
				if (firstRefOfThisType[i - 1].m_resID == firstRefOfThisType[i].m_resID)
					return false;
			}
		}

		if (m_numResources > 0)
		{
			bool anyNamed = false;

			size_t lastNameStart = 0;
			for (size_t i = 0; i < m_numResources; i++)
			{
				const ResourceCompiledRef &ref = m_compiledRefBlob[i];
				if (ref.m_resNameOffset < 0)
				{
					if (ref.m_resNameOffset != -1)
						return false;	// Non-compliant
				}
				else
				{
					anyNamed = true;

					const size_t candidateOffset = static_cast<size_t>(ref.m_resNameOffset);
					if (candidateOffset > lastNameStart)
						lastNameStart = candidateOffset;
				}
			}

			if (anyNamed)
			{
				const size_t nameListCapacity = sizeFromStartOfResMap - resourceMap.m_nameListOffset;
				if (lastNameStart >= nameListCapacity)
					return false;

				m_resNameBlobSize = lastNameStart + 1 + 256;
				m_resNameBlob = new uint8_t[m_resNameBlobSize];
				memset(m_resNameBlob, 0, m_resNameBlobSize);

				if (!stream->SeekStart(resourceHeader.m_resMapOffset + resourceMap.m_nameListOffset))
					return false;

				if (stream->Read(m_resNameBlob, lastNameStart + 1) != lastNameStart + 1)
					return false;

				// Figure out the length of the final string
				const size_t lastStringLength = m_resNameBlob[lastNameStart];
				if (lastStringLength > 0)
				{
					if (stream->Read(m_resNameBlob + lastNameStart + 1, lastStringLength) != lastStringLength)
						return false;
				}
			}
		}

		std::sort(m_compiledTypeListBlob, m_compiledTypeListBlob + m_numResourceTypes, CompiledTypeListSortPredicate);

		for (uint32_t i = 1; i < m_numResourceTypes; i++)
		{
			if (m_compiledTypeListBlob[i - 1].m_resType == m_compiledTypeListBlob[i].m_resType)
				return false;
		}

		return true;
	}

	bool ResourceFile::CompiledRefSortPredicate(const ResourceCompiledRef &a, const ResourceCompiledRef &b)
	{
		return a.m_resID < b.m_resID;
	}

	bool ResourceFile::CompiledTypeListSortPredicate(const ResourceCompiledTypeList &a, const ResourceCompiledTypeList &b)
	{
		return memcmp(&a.m_resType, &b.m_resType, sizeof(ResTypeID)) < 0;
	}

	int ResourceFile::CompiledRefSearchPredicate(int resID, const ResourceCompiledRef &ref)
	{
		return resID - ref.m_resID;
	}

	int ResourceFile::CompiledTypeListSearchPredicate(const ResTypeID &resTypeID, const ResourceCompiledTypeList &typeList)
	{
		return memcmp(&resTypeID, &typeList.m_resType, 4);
	}

	void ResourceFile::GetAllResourceTypeLists(ResourceCompiledTypeList *&outTypeLists, size_t &outCount) const
	{
		outTypeLists = m_compiledTypeListBlob;
		outCount = m_numResourceTypes;
	}

	const ResourceCompiledTypeList *ResourceFile::GetResourceTypeList(const ResTypeID &resType)
	{
		const ResourceCompiledTypeList *tlStart = m_compiledTypeListBlob;
		const ResourceCompiledTypeList *tlEnd = tlStart + m_numResourceTypes;

		const ResourceCompiledTypeList *tl = BinarySearch(tlStart, tlEnd, resType, CompiledTypeListSearchPredicate);
		if (tl == tlEnd)
			return nullptr;

		return tl;
	}

	MMHandleBlock *ResourceFile::GetResource(const ResTypeID &resType, int id, bool load)
	{
		const ResourceCompiledTypeList *tl = GetResourceTypeList(resType);
		if (tl == nullptr)
			return nullptr;

		ResourceCompiledRef *refStart = tl->m_firstRef;
		ResourceCompiledRef *refEnd = refStart + tl->m_numRefs;
		ResourceCompiledRef *ref = BinarySearch(refStart, refEnd, id, CompiledRefSearchPredicate);

		if (ref == refEnd)
			return nullptr;

		MMHandleBlock *handle = nullptr;
		if (ref->m_handle != nullptr)
			handle = ref->m_handle;
		else
		{
			handle = MemoryManager::GetInstance()->AllocHandle(0);
			handle->m_rmSelfRef = ref;
			ref->m_handle = handle;
		}

		if (handle->m_contents == nullptr && load)
		{
			const uint32_t resSize = ref->GetSize();
			if (resSize > 0)
			{
				void *contents = MemoryManager::GetInstance()->Alloc(resSize);
				handle->m_contents = contents;
				handle->m_size = resSize;
				memcpy(handle->m_contents, ref->m_resData, resSize);
			}
		}

		return handle;
	}
}
