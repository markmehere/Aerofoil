#pragma once

namespace GpFileCreationDispositions
{
	enum GpFileCreationDisposition
	{
		kCreateOrOverwrite,	// If exists: Overwrite. If not exists: Create.
		kCreateNew,			// If exists: Fail.      If not exists: Create.
		kCreateOrOpen,		// If exists: Open.      If not exists: Create.
		kOpenExisting,		// If exists: Open.      If not exists: Fail.
		kOverwriteExisting,	// If exists: Overwrite. If not exists: Fail.
	};
}

typedef GpFileCreationDispositions::GpFileCreationDisposition GpFileCreationDisposition_t;
