#pragma once

template<class T>
class THandle;

#include <stdint.h>

struct IGpDisplayDriver;
struct Menu;

namespace PortabilityLayer
{
	struct Vec2i;

	class MenuManager
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual THandle<Menu> DeserializeMenu(const void *resData) const = 0;
		virtual THandle<Menu> GetMenuByID(int id) const = 0;

		virtual void InsertMenuBefore(const THandle<Menu> &insertingMenu, const THandle<Menu> &existingMenu) = 0;
		virtual void InsertMenuAfter(const THandle<Menu> &insertingMenu, const THandle<Menu> &existingMenu) = 0;
		virtual void InsertMenuAtEnd(const THandle<Menu> &insertingMenu) = 0;
		virtual void InsertMenuAtBeginning(const THandle<Menu> &insertingMenu) = 0;

		virtual void RemoveMenu(const THandle<Menu> &menu) = 0;

		virtual void SetMenuEnabled(const THandle<Menu> &menuHandle, bool enabled) = 0;
		virtual void SetItemEnabled(const THandle<Menu> &menu, unsigned int index, bool enabled) = 0;
		virtual void SetItemChecked(const THandle<Menu> &menu, unsigned int index, bool checked) = 0;

		virtual bool IsPointInMenuBar(const Vec2i &point) const = 0;

		virtual void MenuSelect(const Vec2i &initialPoint, int16_t *outMenu, uint16_t *outItem) = 0;

		virtual void DrawMenuBar() = 0;
		virtual void SetMenuVisible(bool isVisible) = 0;

		virtual void RenderFrame(IGpDisplayDriver *displayDriver) = 0;

		static MenuManager *GetInstance();
	};
}
