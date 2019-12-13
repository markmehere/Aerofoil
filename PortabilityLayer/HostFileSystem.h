#pragma once
#ifndef __PL_HOST_FILESYSTEM_H__
#define __PL_HOST_FILESYSTEM_H__

#include "VirtualDirectory.h"

namespace PortabilityLayer
{
	class IOStream;
	class HostDirectoryCursor;

	class HostFileSystem
	{
	public:
		virtual bool FileExists(EVirtualDirectory virtualDirectory, const char *path) = 0;
		virtual IOStream *OpenFile(EVirtualDirectory virtualDirectory, const char *path, bool writeAccess, bool create) = 0;
		virtual HostDirectoryCursor *ScanDirectory(EVirtualDirectory virtualDirectory) = 0;

		static HostFileSystem *GetInstance();
		static void SetInstance(HostFileSystem *instance);

	private:
		static HostFileSystem *ms_instance;
	};
}

#endif
