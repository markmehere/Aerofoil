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
	class ResTypeID;

	class ResourceFile
	{
	public:
		ResourceFile();
		~ResourceFile();

		bool Load(IOStream *stream);

		MMHandleBlock *GetResource(const ResTypeID &resType, int id, bool load);

	private:
		struct CompiledTypeList
		{
			ResTypeID m_resType;
			ResourceCompiledRef *m_firstRef;
			size_t m_numRefs;
		};

		uint8_t *m_resDataBlob;
		size_t m_resDataBlobSize;

		uint8_t *m_resNameBlob;
		size_t m_resNameBlobSize;

		ResourceCompiledRef *m_compiledRefBlob;
		size_t m_numResources;

		CompiledTypeList *m_compiledTypeListBlob;
		size_t m_numResourceTypes;

		static bool CompiledRefSortPredicate(const ResourceCompiledRef &a, const ResourceCompiledRef &b);
		static bool CompiledTypeListSortPredicate(const CompiledTypeList &a, const CompiledTypeList &b);

		static int CompiledRefSearchPredicate(int resID, const ResourceCompiledRef &ref);
		static int CompiledTypeListSearchPredicate(const ResTypeID &resTypeID, const CompiledTypeList &typeList);
	};
}

#endif
