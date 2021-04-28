#pragma once

#include "FilePermission.h"
#include "CoreDefs.h"
#include "GpFileCreationDisposition.h"
#include "PLErrorCodes.h"
#include "VirtualDirectory.h"

#include <stdint.h>

class PLPasStr;

class GpIOStream;

namespace PortabilityLayer
{
	class ResTypeID;
	struct MacFileProperties;
	struct FileBrowserUI_DetailsCallbackAPI;
	class ZipFileProxy;

	class CompositeFile
	{
	public:
		virtual PLError_t OpenData(EFilePermission filePermission, GpFileCreationDisposition_t disposition, GpIOStream *&outStream) = 0;
		virtual PLError_t OpenResources(GpIOStream *&outStream, ZipFileProxy *&outProxy, bool &outIsProxyShared) = 0;
		virtual const MacFileProperties &GetProperties() const = 0;

		virtual VirtualDirectory_t GetDirectory() const = 0;
		virtual PLPasStr GetFileName() const = 0;
		virtual bool IsDataReadOnly() const = 0;

		virtual void Close() = 0;

	protected:
		inline CompositeFile() {}
		inline ~CompositeFile() {}
	};

	class FileManager
	{
	public:
		virtual CompositeFile *OpenCompositeFile(VirtualDirectory_t dirID, const PLPasStr &filename) = 0;
		virtual PLError_t OpenNonCompositeFile(VirtualDirectory_t dirID, const PLPasStr &filename, const char *extension, EFilePermission filePermission, GpFileCreationDisposition_t creationDisposition, GpIOStream *&outStream) = 0;

		virtual bool CompositeFileExists(VirtualDirectory_t dirID, const PLPasStr &filename) = 0;
		virtual bool NonCompositeFileExists(VirtualDirectory_t dirID, const PLPasStr &filename, const char *extension) = 0;

		GP_ASYNCIFY_PARANOID_VIRTUAL bool DeleteNonCompositeFile(VirtualDirectory_t dirID, const PLPasStr &filename, const char *ext) GP_ASYNCIFY_PARANOID_PURE;
		GP_ASYNCIFY_PARANOID_VIRTUAL bool DeleteCompositeFile(VirtualDirectory_t dirID, const PLPasStr &filename) GP_ASYNCIFY_PARANOID_PURE;

		GP_ASYNCIFY_PARANOID_VIRTUAL PLError_t CreateFile(VirtualDirectory_t dirID, const PLPasStr &filename, const MacFileProperties &mfp) GP_ASYNCIFY_PARANOID_PURE;
		GP_ASYNCIFY_PARANOID_VIRTUAL PLError_t CreateFileAtCurrentTime(VirtualDirectory_t dirID, const PLPasStr &filename, const ResTypeID &fileCreator, const ResTypeID &fileType) GP_ASYNCIFY_PARANOID_PURE;

		virtual PLError_t RawOpenFileData(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, bool ignoreMeta, GpFileCreationDisposition_t createDisposition, GpIOStream *&outStream) = 0;
		virtual PLError_t RawOpenFileResources(VirtualDirectory_t dirID, const PLPasStr &filename, EFilePermission filePermission, bool ignoreMeta, GpFileCreationDisposition_t createDisposition, GpIOStream *&outStream) = 0;

		GP_ASYNCIFY_PARANOID_VIRTUAL bool PromptSaveFile(VirtualDirectory_t dirID, const char *extension, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &initialFileName, const PLPasStr &promptText, bool composites, const FileBrowserUI_DetailsCallbackAPI &callbackAPI) GP_ASYNCIFY_PARANOID_PURE;
		GP_ASYNCIFY_PARANOID_VIRTUAL bool PromptOpenFile(VirtualDirectory_t dirID, const char *extension, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &promptText, bool composites, const FileBrowserUI_DetailsCallbackAPI &callbackAPI) GP_ASYNCIFY_PARANOID_PURE;

		static FileManager *GetInstance();
	};
}
