#pragma once
#ifndef __PL_FILE_MANAGER_H__
#define __PL_FILE_MANAGER_H__

#include "FilePermission.h"
#include "CoreDefs.h"
#include "FilePos.h"
#include "PLErrorCodes.h"
#include "VirtualDirectory.h"

#include <stdint.h>

class PLPasStr;

namespace PortabilityLayer
{
	class IOStream;
	class ResTypeID;
	struct MacFileProperties;

	class FileManager
	{
	public:
		virtual bool FileExists(VirtualDirectory_t dirID, const PLPasStr &filename) = 0;
		virtual bool DeleteFile(VirtualDirectory_t dirID, const PLPasStr &filename) = 0;

		virtual PLError_t CreateFile(VirtualDirectory_t dirID, const PLPasStr &filename, const MacFileProperties &mfp) = 0;
		virtual PLError_t CreateFileAtCurrentTime(VirtualDirectory_t dirID, const PLPasStr &filename, const ResTypeID &fileCreator, const ResTypeID &fileType) = 0;

		virtual PLError_t OpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, IOStream *&outStream) = 0;
		virtual PLError_t OpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, IOStream *&outStream) = 0;
		virtual bool ReadFileProperties(VirtualDirectory_t dirID, const PLPasStr &filename, MacFileProperties &properties) = 0;

		virtual PLError_t RawOpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, bool ignoreMeta, IOStream *&outStream) = 0;
		virtual PLError_t RawOpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, bool ignoreMeta, IOStream *&outStream) = 0;

		static FileManager *GetInstance();
	};
}

#endif
