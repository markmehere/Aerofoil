#pragma once

#include <stdint.h>
#include <stddef.h>

#include "VirtualDirectory.h"

class PLPasStr;
struct DrawSurface;
struct Point;
struct Rect;

namespace PortabilityLayer
{
	class ResTypeID;

	struct FileBrowserUI_DetailsCallbackAPI
	{
		void *m_context;

		void (*m_drawLabelsCallback)(void *context, DrawSurface *surface, const Point &basePoint);
		void (*m_drawFileDetailsCallback)(void *context, DrawSurface *surface, const Point &basePoint, const Rect &constraintRect, void *fileDetails);

		void *(*m_loadFileDetailsCallback)(void *context, VirtualDirectory_t dirID, const PLPasStr &filename);
		void (*m_freeFileDetailsCallback)(void *context, void *fileDetails);
		bool (*m_filterFileCallback)(void *context, VirtualDirectory_t dirID, const PLPasStr &filename);
		bool (*m_isDeleteValidCallback)(void *context, VirtualDirectory_t dirID, const PLPasStr &filename);
		void (*m_onDeletedCallback)(void *context, VirtualDirectory_t dirID, const PLPasStr &filename);
	};

	class FileBrowserUI
	{
	public:

		enum Mode
		{
			Mode_SaveNoDelete,
			Mode_SaveWithDelete,
			Mode_Open,
		};

		static bool Prompt(Mode mode, VirtualDirectory_t dirID, const char *extension, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &initialFileName, const PLPasStr &promptText, bool composites, const FileBrowserUI_DetailsCallbackAPI &callbackAPI);
	};
}
