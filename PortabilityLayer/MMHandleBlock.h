#pragma once
#ifndef __PL_MM_HANDLE_BLOCK_H__
#define __PL_MM_HANDLE_BLOCK_H__

#include <stdint.h>

#include "CoreDefs.h"

namespace PortabilityLayer
{
	struct ResourceArchiveRef;
	struct ResourceCompiledRef;

	struct MMHandleBlock
	{
		explicit MMHandleBlock(void *contents, size_t size);

		void **AsHandle();

		void *m_contents;	// This must be the first field
		ResourceArchiveRef *m_rmSelfRef;

		size_t m_size;

	private:
		MMHandleBlock() GP_DELETED;
	};
}

#endif
