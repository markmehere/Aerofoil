#pragma once

#include <stdint.h>

#include "VirtualDirectory.h"

class PLPasStr;
struct DrawSurface;
struct Point;
struct Rect;

namespace PortabilityLayer
{
	struct FileBrowserUI_DetailsCallbackAPI
	{
		void(*m_drawLabelsCallback)(DrawSurface *surface, const Point &basePoint);
		void(*m_drawFileDetailsCallback)(DrawSurface *surface, const Point &basePoint, const Rect &constraintRect, void *fileDetails);

		void *(*m_loadFileDetailsCallback)(VirtualDirectory_t dirID, const PLPasStr &filename);
		void(*m_freeFileDetailsCallback)(void *fileDetails);
	};

	class FileBrowserUI
	{
	public:

		enum Mode
		{
			Mode_Save,
			Mode_Open,
		};

		static bool Prompt(Mode mode, VirtualDirectory_t dirID, char *path, size_t &outPathLength, size_t pathCapacity, const PLPasStr &initialFileName, const PLPasStr &promptText, const FileBrowserUI_DetailsCallbackAPI &callbackAPI);
	};
}
