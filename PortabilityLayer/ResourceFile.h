#pragma once
#ifndef __PL_RESOURCE_FILE_H__
#define __PL_RESOURCE_FILE_H__

#include "ResTypeID.h"
#include <stdint.h>

namespace PortabilityLayer
{
	class IOStream;
	class MacFileMem;
	struct MMHandleBlock;
	struct ResourceCompiledRef;
	struct ResourceCompiledTypeList;
	class ResTypeID;

	class ResourceFile
	{
	public:
		ResourceFile();
		~ResourceFile();

		bool Load(IOStream *stream);

		void GetAllResourceTypeLists(ResourceCompiledTypeList *&outTypeLists, size_t &outCount) const;

		const ResourceCompiledTypeList *GetResourceTypeList(const ResTypeID &resType);
		MMHandleBlock *GetResource(const ResTypeID &resType, int id, bool load);

	private:

		uint8_t *m_resDataBlob;
		size_t m_resDataBlobSize;

		uint8_t *m_resNameBlob;
		size_t m_resNameBlobSize;

		ResourceCompiledRef *m_compiledRefBlob;
		size_t m_numResources;

		ResourceCompiledTypeList *m_compiledTypeListBlob;
		size_t m_numResourceTypes;

		static bool CompiledRefSortPredicate(const ResourceCompiledRef &a, const ResourceCompiledRef &b);
		static bool CompiledTypeListSortPredicate(const ResourceCompiledTypeList &a, const ResourceCompiledTypeList &b);

		static int CompiledRefSearchPredicate(int resID, const ResourceCompiledRef &ref);
		static int CompiledTypeListSearchPredicate(const ResTypeID &resTypeID, const ResourceCompiledTypeList &typeList);
	};
}

#endif
