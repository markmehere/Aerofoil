#pragma once

#include "PLHandle.h"
#include "ResTypeID.h"
#include <stdint.h>

class GpIOStream;

namespace PortabilityLayer
{
	class MacFileMem;
	struct MMHandleBlock;
	struct ResourceCompiledRef;
	struct ResourceCompiledTypeList;
	class ResTypeID;

	class ResourceFile final
	{
	public:
		bool Load(GpIOStream *stream);

		void GetAllResourceTypeLists(ResourceCompiledTypeList *&outTypeLists, size_t &outCount) const;

		const ResourceCompiledTypeList *GetResourceTypeList(const ResTypeID &resType);
		THandle<void> LoadResource(const ResTypeID &resType, int id);

		static ResourceFile *Create();
		void Destroy();

	private:
		ResourceFile();
		~ResourceFile();

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
