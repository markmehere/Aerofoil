#pragma once

struct Menu;
struct IGpDisplayDriver;

namespace PortabilityLayer
{
	class MenuManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual Menu **DeserializeMenu(const void *resData) const = 0;
		virtual Menu **GetMenuByID(int id) const = 0;
		virtual void InsertMenuBefore(Menu **insertingMenu, Menu **existingMenu) = 0;
		virtual void InsertMenuAfter(Menu **insertingMenu, Menu **existingMenu) = 0;
		virtual void InsertMenuAtEnd(Menu **insertingMenu) = 0;
		virtual void InsertMenuAtBeginning(Menu **insertingMenu) = 0;
		virtual void SetMenuEnabled(Menu **menuHandle, bool enabled) = 0;
		virtual void SetItemEnabled(Menu **menu, unsigned int index, bool enabled) = 0;
		virtual void SetItemChecked(Menu **menu, unsigned int index, bool checked) = 0;

		virtual void DrawMenuBar() = 0;

		virtual void RenderFrame(IGpDisplayDriver *displayDriver) = 0;

		static MenuManager *GetInstance();
	};
}
