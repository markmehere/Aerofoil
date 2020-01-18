#include "MenuManager.h"
#include "DisplayDeviceManager.h"
#include "FontFamily.h"
#include "FontManager.h"
#include "HostDisplayDriver.h"
#include "HostFont.h"
#include "IGpDisplayDriver.h"
#include "MemoryManager.h"
#include "ResourceManager.h"
#include "SimpleGraphic.h"
#include "PLBigEndian.h"
#include "PLCore.h"
#include "PLPasStr.h"
#include "PLResources.h"
#include "PLTimeTaggedVOSEvent.h"
#include "PLQDOffscreen.h"
#include "RenderedFont.h"
#include "QDGraf.h"
#include "QDManager.h"
#include "QDPixMap.h"
#include "RGBAColor.h"
#include "Vec2i.h"

#include <stdint.h>
#include <assert.h>
#include <algorithm>

namespace
{
	static const int kMidGray = 187;

	const PortabilityLayer::RGBAColor gs_barTopLeftCornerGraphicPixels[] =
	{
		{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 85, 85, 85, 255 }, { 170, 170, 170, 255 },
		{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 85, 85, 85, 255 }, { 255, 255, 255, 255 }, { 255, 255, 255, 255 },
		{ 0, 0, 0, 255 }, { 85, 85, 85, 255 }, { 255, 255, 255, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { kMidGray, kMidGray, kMidGray, 255 },
		{ 85, 85, 85, 255 }, { 255, 255, 255, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { kMidGray, kMidGray, kMidGray, 255 },
		{ 170, 170, 170, 255 }, { 255, 255, 255, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { kMidGray, kMidGray, kMidGray, 255 },
	};

	const PortabilityLayer::RGBAColor gs_barTopRightCornerGraphicPixels[] =
	{
		{ 170, 170, 170, 255 }, { 85, 85, 85, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 },
		{ 255, 255, 255, 255 }, { 255, 255, 255, 255 }, { 85, 85, 85, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 },
		{ kMidGray, kMidGray, kMidGray, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { 255, 255, 255, 255 }, { 85, 85, 85, 255 }, { 0, 0, 0, 255 },
		{ kMidGray, kMidGray, kMidGray, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { 255, 255, 255, 255 }, { 85, 85, 85, 255 },
		{ kMidGray, kMidGray, kMidGray, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { kMidGray, kMidGray, kMidGray, 255 }, { 255, 255, 255, 255 }, { 85, 85, 85, 255 },
	};

	const PortabilityLayer::RGBAColor gs_barBrightColor = { 255, 255, 255, 255 };
	const PortabilityLayer::RGBAColor gs_barMidColor = { kMidGray, kMidGray, kMidGray, 255 };
	const PortabilityLayer::RGBAColor gs_barDarkColor = { 102, 102, 102, 255 };
	const PortabilityLayer::RGBAColor gs_barBottomEdgeColor = { 0, 0, 0, 255 };
	const PortabilityLayer::RGBAColor gs_barNormalTextColor = { 0, 0, 0, 255 };
	const PortabilityLayer::RGBAColor gs_barHighlightTextColor = { 255, 255, 255, 255 };

	const PortabilityLayer::RGBAColor gs_barHighlightBrightColor = { 153, 204, 255, 255 };
	const PortabilityLayer::RGBAColor gs_barHighlightMidColor = { 51, 102, 204, 255 };
	const PortabilityLayer::RGBAColor gs_barHighlightDarkColor = { 0, 51, 102, 255 };

	PortabilityLayer::SimpleGraphicInstanceRGBA<5, 5> gs_barTopLeftCornerGraphic(gs_barTopLeftCornerGraphicPixels);
	PortabilityLayer::SimpleGraphicInstanceRGBA<5, 5> gs_barTopRightCornerGraphic(gs_barTopRightCornerGraphicPixels);
}

struct MenuItem
{
	uint32_t nameOffsetInStringBlob;
	int8_t iconResID;
	uint8_t key;
	uint8_t submenuID;
	uint8_t textStyle;
	bool enabled;
	bool checked;

	uint16_t layoutYOffset;
	uint16_t layoutHeight;
};

struct Menu
{
	uint16_t menuID;
	uint16_t width;
	uint16_t height;
	uint16_t commandID;
	bool enabled;
	bool isIcon;
	bool haveMenuLayout;

	size_t layoutWidth;
	size_t layoutHeight;

	PortabilityLayer::MMHandleBlock *stringBlobHandle;

	THandle<Menu> prevMenu;
	THandle<Menu> nextMenu;

	// Refreshed on layout
	size_t cumulativeOffset;
	size_t unpaddedTitleWidth;
	unsigned int menuIndex;

	size_t numMenuItems;

	// This must be the last item
	MenuItem menuItems[1];
};

namespace PortabilityLayer
{
	class MenuManagerImpl final : public MenuManager
	{
	public:
		MenuManagerImpl();
		~MenuManagerImpl();

		virtual void Init() override;
		virtual void Shutdown() override;

		THandle<Menu> DeserializeMenu(const void *resData) const override;
		THandle<Menu> GetMenuByID(int id) const override;

		void InsertMenuBefore(const THandle<Menu> &insertingMenu, const THandle<Menu> &existingMenu) override;
		void InsertMenuAfter(const THandle<Menu> &insertingMenu, const THandle<Menu> &existingMenu) override;
		void InsertMenuAtEnd(const THandle<Menu> &insertingMenu) override;
		void InsertMenuAtBeginning(const THandle<Menu> &insertingMenu) override;

		void RemoveMenu(const THandle<Menu> &menu) override;

		void SetMenuEnabled(const THandle<Menu> &menuHandle, bool enabled) override;
		void SetItemEnabled(const THandle<Menu> &menu, unsigned int index, bool enabled) override;
		void SetItemChecked(const THandle<Menu> &menu, unsigned int index, bool checked) override;
		bool SetItemText(const THandle<Menu> &menu, unsigned int index, const PLPasStr &str) override;

		bool IsPointInMenuBar(const Vec2i &point) const override;
		void MenuSelect(const Vec2i &initialPoint, int16_t *outMenu, uint16_t *outItem) override;

		void DrawMenuBar() override;
		void SetMenuVisible(bool isVisible) override;

		void RenderFrame(IGpDisplayDriver *displayDriver) override;

		static MenuManagerImpl *GetInstance();

	private:
		class MenuSelectionState
		{
		public:
			MenuSelectionState();
			~MenuSelectionState();

			void HandleSelectionOfMenu(MenuManagerImpl *mm, Menu **menuHdl, bool &outNeedRedraw);
			void Dismiss();

			THandle<Menu> GetSelectedMenu() const;
			DrawSurface *GetRenderedMenu() const;
			const unsigned int *GetSelectedItem() const;

			void SelectItem(size_t item);
			void ClearSelection();

		private:
			void RenderMenu(Menu *menu);

			THandle<Menu> m_currentMenu;
			DrawSurface *m_menuGraf;
			unsigned int m_itemIndex;
			bool m_haveItem;
		};

		void RefreshMenuBarLayout();
		void RefreshMenuLayout(Menu *menu);
		void ProcessMouseMoveTo(const Vec2i &point);
		void ProcessMouseMoveToMenuBar(const Vec2i &point);
		void ProcessMouseMoveToMenu(const Vec2i &point);

		static const unsigned int kIconResID = 128;
		static const unsigned int kMenuFontSize = 12;
		static const unsigned int kMenuBarIconYOffset = 2;
		static const unsigned int kMenuBarTextYOffset = 14;
		static const unsigned int kMenuBarHeight = 20;
		static const unsigned int kMenuBarItemPadding = 6;
		static const unsigned int kMenuBarInitialPadding = 16;

		static const unsigned int kMenuItemHeight = 18;
		static const unsigned int kMenuItemTextYOffset = 13;
		static const unsigned int kMenuSeparatorHeight = 6;

		static const unsigned int kMenuItemRightPadding = 8;
		static const unsigned int kMenuItemLeftPadding = 16 + 2 + 2;	// 2 for left border, 16 for icon, 2 for spacing

		static const int kMenuFontFlags = PortabilityLayer::FontFamilyFlag_Bold;

		DrawSurface *m_menuBarGraf;

		THandle<Menu> m_firstMenu;
		THandle<Menu> m_lastMenu;
		bool m_haveMenuBarLayout;
		bool m_haveIcon;
		bool m_menuBarVisible;

		uint8_t m_iconColors[16 * 16];
		uint8_t m_iconMask[32];

		SimpleGraphic *m_iconGraphic;

		MenuSelectionState m_menuSelectionState;

		static MenuManagerImpl ms_instance;
	};

	MenuManagerImpl::MenuManagerImpl()
		: m_menuBarGraf(nullptr)
		, m_haveMenuBarLayout(false)
		, m_haveIcon(false)
		, m_iconGraphic(nullptr)
		, m_menuBarVisible(false)
	{
	}

	MenuManagerImpl::~MenuManagerImpl()
	{
	}

	void MenuManagerImpl::Init()
	{
	}

	void MenuManagerImpl::Shutdown()
	{
		PortabilityLayer::QDManager *qdManager = PortabilityLayer::QDManager::GetInstance();

		if (m_menuBarGraf)
			qdManager->DisposeGWorld(m_menuBarGraf);

		if (m_iconGraphic)
		{
			m_iconGraphic->~SimpleGraphic();
			free(m_iconGraphic);
		}

		// GP TODO: Dispose of menus properly
	}

	THandle<Menu> MenuManagerImpl::DeserializeMenu(const void *resData) const
	{
		PortabilityLayer::MemoryManager *mm = PortabilityLayer::MemoryManager::GetInstance();

		const uint8_t *resBytes = static_cast<const uint8_t*>(resData);

		struct MenuDataSerialized
		{
			BEUInt16_t menuID;
			BEUInt16_t width;
			BEUInt16_t height;
			BEUInt16_t commandID;
			BEUInt16_t padding;
			BEUInt32_t enableFlags;
		};

		MenuDataSerialized header;
		memcpy(&header, resBytes, sizeof(header));

		const uint8_t titleLength = resBytes[14];

		const uint8_t *menuDataStart = resBytes + 14 + 1 + titleLength;

		size_t stringDataLength = 1 + titleLength;
		size_t numMenuItems = 0;

		for (const uint8_t *menuItemStart = menuDataStart; *menuItemStart; menuItemStart += 5 + (*menuItemStart))
		{
			numMenuItems++;
			stringDataLength += 1 + (*menuItemStart);
		}

		MMHandleBlock *stringData = mm->AllocHandle(stringDataLength);
		if (!stringData)
		{
			mm->ReleaseHandle(stringData);
			return nullptr;
		}

		MMHandleBlock *menuData = mm->AllocHandle(sizeof(Menu) + sizeof(MenuItem) * (numMenuItems - 1));
		if (!menuData)
		{
			mm->ReleaseHandle(stringData);
			return nullptr;
		}

		uint32_t enableFlags = header.enableFlags;

		Menu *menu = static_cast<Menu*>(menuData->m_contents);
		menu->menuID = header.menuID;
		menu->width = header.width;
		menu->height = header.height;
		menu->commandID = header.commandID;
		menu->enabled = ((enableFlags & 1) != 0);
		menu->menuIndex = 0;
		menu->cumulativeOffset = 0;
		menu->unpaddedTitleWidth = 0;
		menu->isIcon = false;
		menu->haveMenuLayout = false;

		uint8_t *stringDataStart = static_cast<uint8_t*>(stringData->m_contents);
		uint8_t *stringDest = stringDataStart;
		memcpy(stringDest, resBytes + 14, 1 + resBytes[14]);
		stringDest += 1 + resBytes[14];

		enableFlags >>= 1;

		MenuItem *currentItem = menu->menuItems;
		for (const uint8_t *menuItemStart = menuDataStart; *menuItemStart; menuItemStart += 5 + (*menuItemStart))
		{
			uint8_t itemNameLength = *menuItemStart;

			memcpy(stringDest, menuItemStart, 1 + itemNameLength);

			currentItem->iconResID = static_cast<int8_t>(menuItemStart[1 + itemNameLength]);
			currentItem->key = menuItemStart[2 + itemNameLength];
			currentItem->submenuID = menuItemStart[3 + itemNameLength];
			currentItem->textStyle = menuItemStart[4 + itemNameLength];
			currentItem->nameOffsetInStringBlob = static_cast<uint32_t>(stringDest - stringDataStart);
			currentItem->enabled = ((enableFlags & 1) != 0);
			currentItem->checked = false;
			currentItem->layoutYOffset = 0;

			enableFlags >>= 1;

			currentItem++;
			stringDest += 1 + (*menuItemStart);
		}

		menu->numMenuItems = numMenuItems;
		menu->stringBlobHandle = stringData;
		menu->prevMenu = nullptr;
		menu->nextMenu = nullptr;
		menu->layoutWidth = 0;
		menu->layoutHeight = 0;

		return THandle<Menu>(menuData);
	}

	THandle<Menu> MenuManagerImpl::GetMenuByID(int id) const
	{
		for (THandle<Menu> menuHandle = m_firstMenu; menuHandle; menuHandle = (*menuHandle)->nextMenu)
		{
			if ((*menuHandle)->menuID == id)
				return menuHandle;
		}

		return THandle<Menu>();
	}

	void MenuManagerImpl::InsertMenuBefore(const THandle<Menu> &insertingMenu, const THandle<Menu> &existingMenu)
	{
		m_haveMenuBarLayout = false;

		Menu *insertingMenuPtr = *insertingMenu;
		Menu *existingMenuPtr = *existingMenu;

		insertingMenuPtr->prevMenu = existingMenuPtr->prevMenu;
		insertingMenuPtr->nextMenu = existingMenu;

		if (existingMenuPtr->prevMenu)
			(*existingMenuPtr->prevMenu)->nextMenu = insertingMenu;
		else
			m_firstMenu = insertingMenu;

		existingMenuPtr->prevMenu = insertingMenu;

		DrawMenuBar();
	}

	void MenuManagerImpl::InsertMenuAfter(const THandle<Menu> &insertingMenu, const THandle<Menu> &existingMenu)
	{
		m_haveMenuBarLayout = false;

		Menu *insertingMenuPtr = *insertingMenu;
		Menu *existingMenuPtr = *existingMenu;

		insertingMenuPtr->prevMenu = existingMenu;
		insertingMenuPtr->nextMenu = existingMenuPtr->nextMenu;

		if (existingMenuPtr->nextMenu)
			(*existingMenuPtr->nextMenu)->prevMenu = insertingMenu;
		else
			m_lastMenu = insertingMenu;

		existingMenuPtr->nextMenu = insertingMenu;

		DrawMenuBar();
	}

	void MenuManagerImpl::InsertMenuAtEnd(const THandle<Menu> &insertingMenu)
	{
		m_haveMenuBarLayout = false;

		if (m_firstMenu == nullptr)
		{
			m_firstMenu = m_lastMenu = insertingMenu;
			return;
		}

		(*m_lastMenu)->nextMenu = insertingMenu;
		(*insertingMenu)->prevMenu = m_lastMenu;
		m_lastMenu = insertingMenu;

		DrawMenuBar();
	}

	void MenuManagerImpl::InsertMenuAtBeginning(const THandle<Menu> &insertingMenu)
	{
		m_haveMenuBarLayout = false;

		if (m_firstMenu == nullptr)
		{
			m_firstMenu = m_lastMenu = insertingMenu;
			return;
		}

		(*m_firstMenu)->prevMenu = insertingMenu;
		(*insertingMenu)->nextMenu = m_firstMenu;
		m_firstMenu = insertingMenu;

		DrawMenuBar();
	}

	void MenuManagerImpl::RemoveMenu(const THandle<Menu> &menu)
	{
		Menu *menuPtr = *menu;
		if (menuPtr->stringBlobHandle)
			PortabilityLayer::MemoryManager::GetInstance()->ReleaseHandle(menuPtr->stringBlobHandle);

		if (menuPtr->prevMenu)
			(*menuPtr->prevMenu)->nextMenu = menuPtr->nextMenu;

		if (menuPtr->nextMenu)
			(*menuPtr->nextMenu)->prevMenu = menuPtr->prevMenu;

		if (m_firstMenu == menu)
			m_firstMenu = menuPtr->nextMenu;

		if (m_lastMenu == menu)
			m_lastMenu = menuPtr->prevMenu;

		menu.Dispose();

		DrawMenuBar();
	}

	void MenuManagerImpl::SetMenuEnabled(const THandle<Menu> &menuHandle, bool enabled)
	{
		Menu *menu = *menuHandle;

		menu->enabled = enabled;

		DrawMenuBar();
	}

	void MenuManagerImpl::SetItemEnabled(const THandle<Menu> &menuHandle, unsigned int index, bool enabled)
	{
		Menu *menu = *menuHandle;

		if (index >= menu->numMenuItems)
			return;

		menu->menuItems[index].enabled = enabled;

		DrawMenuBar();
	}

	void MenuManagerImpl::SetItemChecked(const THandle<Menu> &menuHandle, unsigned int index, bool checked)
	{
		Menu *menu = *menuHandle;

		if (index >= menu->numMenuItems)
			return;

		menu->menuItems[index].checked = checked;
	}

	bool MenuManagerImpl::SetItemText(const THandle<Menu> &menu, unsigned int index, const PLPasStr &str)
	{
		Menu *menuPtr = *menu;

		PortabilityLayer::MMHandleBlock *oldHandle = menuPtr->stringBlobHandle;
		size_t oldSize = oldHandle->m_size;

		const uint8_t *oldStrBlob = static_cast<const uint8_t*>(oldHandle->m_contents);

		const size_t newSize = oldSize - oldStrBlob[menuPtr->menuItems[index].nameOffsetInStringBlob] + str.Length() + 1;

		THandle<uint8_t> newHandle = THandle<uint8_t>(PortabilityLayer::MemoryManager::GetInstance()->AllocHandle(newSize));
		if (!newHandle)
			return false;

		uint8_t *strWritePtr = *newHandle;

		uint32_t writeOffset = 0;
		for (size_t i = 0; i < menuPtr->numMenuItems; i++)
		{
			MenuItem &menuItem = menuPtr->menuItems[i];

			const PLPasStr copyStr = (i == index) ? str : PLPasStr(oldStrBlob + menuItem.nameOffsetInStringBlob);

			menuItem.nameOffsetInStringBlob = writeOffset;

			strWritePtr[writeOffset] = copyStr.Length();
			writeOffset++;

			memcpy(strWritePtr + writeOffset, copyStr.UChars(), copyStr.Length());
			writeOffset += copyStr.Length();
		}

		PortabilityLayer::MemoryManager::GetInstance()->ReleaseHandle(menuPtr->stringBlobHandle);

		menuPtr->stringBlobHandle = newHandle.MMBlock();

		return true;
	}

	bool MenuManagerImpl::IsPointInMenuBar(const Vec2i &point) const
	{
		return point.m_y >= 0 && static_cast<uint32_t>(point.m_y) < kMenuBarHeight;
	}

	void MenuManagerImpl::MenuSelect(const Vec2i &initialPoint, int16_t *outMenu, uint16_t *outItem)
	{
		RefreshMenuBarLayout();

		ProcessMouseMoveTo(initialPoint);

		if (m_menuSelectionState.GetSelectedMenu() == nullptr)
		{
			if (outMenu)
				*outMenu = 0;

			if (outItem)
				*outItem = 0;

			return;
		}

		TimeTaggedVOSEvent evt;
		bool canDismiss = false;
		while (!canDismiss)
		{
			if (WaitForEvent(&evt, 1))
			{
				if (evt.m_vosEvent.m_eventType == GpVOSEventTypes::kMouseInput)
				{
					switch (evt.m_vosEvent.m_event.m_mouseInputEvent.m_eventType)
					{
					case GpMouseEventTypes::kMove:
						ProcessMouseMoveTo(PortabilityLayer::Vec2i(evt.m_vosEvent.m_event.m_mouseInputEvent.m_x, evt.m_vosEvent.m_event.m_mouseInputEvent.m_y));
						break;
					case GpMouseEventTypes::kDown:
						if (evt.m_vosEvent.m_event.m_mouseInputEvent.m_button == GpMouseButtons::kLeft)
							ProcessMouseMoveTo(PortabilityLayer::Vec2i(evt.m_vosEvent.m_event.m_mouseInputEvent.m_x, evt.m_vosEvent.m_event.m_mouseInputEvent.m_y));
						break;
					case GpMouseEventTypes::kUp:
						if (evt.m_vosEvent.m_event.m_mouseInputEvent.m_button == GpMouseButtons::kLeft)
							canDismiss = true;
						break;
					}
				}
			}
		}

		if (outMenu)
			*outMenu = 0;

		if (outItem)
			*outItem = 0;

		if (Menu **menuHdl = m_menuSelectionState.GetSelectedMenu())
		{
			if (const unsigned int *selectedItem = m_menuSelectionState.GetSelectedItem())
			{
				if (outMenu)
					*outMenu = (*menuHdl)->menuID;

				if (outItem)
					*outItem = (*selectedItem) + 1;
			}
		}

		m_menuSelectionState.Dismiss();
		this->DrawMenuBar();
	}

	void MenuManagerImpl::DrawMenuBar()
	{
		if (!m_haveIcon)
		{
			ResourceManager *resManager = ResourceManager::GetInstance();

			Handle icsHandle = GetResource('ics#', kIconResID);
			Handle ics8Handle = GetResource('ics8', kIconResID);

			if (icsHandle && ics8Handle)
			{
				typedef SimpleGraphicInstanceStandardPalette<16, 16> GraphicType_t;

				void *storage = static_cast<GraphicType_t*>(malloc(sizeof(GraphicType_t)));
				if (storage)
				{
					memcpy(m_iconMask, static_cast<const uint8_t*>(*icsHandle) + 32, 32);
					memcpy(m_iconColors, static_cast<const uint8_t*>(*ics8Handle), 16 * 16);

					GraphicType_t *graphic = new (storage) GraphicType_t(m_iconColors);

					m_iconGraphic = graphic;

				}
			}

			if (icsHandle)
				icsHandle.Dispose();

			if (ics8Handle)
				ics8Handle.Dispose();

			m_haveIcon = true;
		}

		unsigned int width;
		GpPixelFormat_t pixelFormat;
		PortabilityLayer::HostDisplayDriver::GetInstance()->GetDisplayResolution(&width, nullptr, &pixelFormat);

		PortabilityLayer::QDManager *qdManager = PortabilityLayer::QDManager::GetInstance();

		const Rect menuRect = Rect::Create(0, 0, kMenuBarHeight, width);

		if (m_menuBarGraf == nullptr)
		{
			if (qdManager->NewGWorld(&m_menuBarGraf, pixelFormat, menuRect, nullptr) != 0)
				return;
		}

		DrawSurface *graf = m_menuBarGraf;

		assert(graf);

		if (static_cast<unsigned int>(graf->m_port.GetRect().right) != width)
		{
			if (!graf->m_port.Resize(menuRect))
				return;
		}

		RefreshMenuBarLayout();

		DrawSurface *oldGraf = GetGraphicsPort();

		SetGraphicsPort(m_menuBarGraf);

		PortabilityLayer::QDState *qdState = qdManager->GetState();

		graf->SetForeColor(gs_barMidColor);
		graf->FillRect(menuRect);
		graf->SetForeColor(gs_barBrightColor);

		// Top stripe
		{
			const Rect rect = Rect::Create(0, 0, 1, static_cast<int16_t>(width) - 1);
			m_menuBarGraf->FillRect(rect);
		}

		// Left stripe
		{
			const Rect rect = Rect::Create(0, 0, kMenuBarHeight - 1, 1);
			m_menuBarGraf->FillRect(rect);
		}

		qdState->SetForeColor(gs_barDarkColor);

		// Bottom stripe
		{
			const Rect rect = Rect::Create(kMenuBarHeight - 2, 1, kMenuBarHeight - 1, width);
			m_menuBarGraf->FillRect(rect);
		}

		// Right stripe
		{
			const Rect rect = Rect::Create(0, width - 1, kMenuBarHeight - 1, width);
			m_menuBarGraf->FillRect(rect);
		}

		qdState->SetForeColor(gs_barBottomEdgeColor);

		// Bottom edge
		{
			const Rect rect = Rect::Create(kMenuBarHeight - 1, 0, kMenuBarHeight, width);
			m_menuBarGraf->FillRect(rect);
		}

		PixMapHandle pixMap = m_menuBarGraf->m_port.GetPixMap();

		// Round corners
		gs_barTopLeftCornerGraphic.DrawToPixMap(pixMap, 0, 0);
		gs_barTopRightCornerGraphic.DrawToPixMap(pixMap, static_cast<int16_t>(width) - static_cast<int16_t>(gs_barTopRightCornerGraphic.m_width), 0);

		Menu **selectedMenuHdl = m_menuSelectionState.GetSelectedMenu();

		if (selectedMenuHdl)
		{
			Menu *menu = *selectedMenuHdl;

			const size_t xCoordinate = menu->cumulativeOffset + (menu->menuIndex * 2) * kMenuBarItemPadding + kMenuBarInitialPadding - kMenuBarItemPadding;
			const size_t width = menu->unpaddedTitleWidth + kMenuBarItemPadding * 2;

			const int16_t left = static_cast<int16_t>(xCoordinate);
			const int16_t right = static_cast<int16_t>(xCoordinate + width);

			// Top edge
			qdState->SetForeColor(gs_barHighlightBrightColor);
			{
				const Rect rect = Rect::Create(0, left, 1, right);
				m_menuBarGraf->FillRect(rect);
			}

			// Middle
			qdState->SetForeColor(gs_barHighlightMidColor);
			{
				const Rect rect = Rect::Create(1, left, kMenuBarHeight - 2, right);
				m_menuBarGraf->FillRect(rect);
			}

			qdState->SetForeColor(gs_barHighlightDarkColor);
			{
				const Rect rect = Rect::Create(kMenuBarHeight - 2, left, kMenuBarHeight - 1, right);
				m_menuBarGraf->FillRect(rect);
			}
		}

		// Text items
		qdState->SetForeColor(gs_barNormalTextColor);
		m_menuBarGraf->SetSystemFont(kMenuFontSize, PortabilityLayer::FontFamilyFlag_Bold);

		{
			Menu **menuHdl = m_firstMenu;
			while (menuHdl)
			{
				Menu *menu = *menuHdl;

				if (menu->stringBlobHandle)
				{
					size_t xCoordinate = menu->cumulativeOffset + (menu->menuIndex * 2) * kMenuBarItemPadding + kMenuBarInitialPadding;

					if (menu->isIcon)
					{
						if (m_iconGraphic)
							m_iconGraphic->DrawToPixMapWithMask(pixMap, m_iconMask, static_cast<int16_t>(xCoordinate), kMenuBarIconYOffset);
					}
					else
					{
						if (menuHdl != selectedMenuHdl)
						{
							const Point itemPos = Point::Create(static_cast<int16_t>(xCoordinate), kMenuBarTextYOffset);
							graf->DrawString(itemPos, PLPasStr(static_cast<const uint8_t*>(menu->stringBlobHandle->m_contents)), true);
						}
					}
				}

				menuHdl = menu->nextMenu;
			}
		}

		if (selectedMenuHdl)
		{
			Menu *menu = *selectedMenuHdl;

			if (menu->stringBlobHandle && !menu->isIcon)
			{
				qdState->SetForeColor(gs_barHighlightTextColor);
				size_t xCoordinate = menu->cumulativeOffset + (menu->menuIndex * 2) * kMenuBarItemPadding + kMenuBarInitialPadding;

				const Point itemPos = Point::Create(static_cast<int16_t>(xCoordinate), kMenuBarTextYOffset);
				graf->DrawString(itemPos, PLPasStr(static_cast<const uint8_t*>(menu->stringBlobHandle->m_contents)), true);
			}
		}

		SetGraphicsPort(oldGraf);

		m_menuBarGraf->m_port.SetDirty(QDPortDirtyFlag_Contents);
	}

	void MenuManagerImpl::SetMenuVisible(bool isVisible)
	{
		if (isVisible && !m_menuBarVisible)
			DrawMenuBar();

		m_menuBarVisible = isVisible;
	}

	void MenuManagerImpl::RenderFrame(IGpDisplayDriver *displayDriver)
	{
		if (!m_menuBarVisible)
			return;

		if (m_menuBarGraf)
		{
			m_menuBarGraf->PushToDDSurface(displayDriver);

			if (m_menuBarGraf->m_ddSurface)
			{
				const PixMap *pixMap = *m_menuBarGraf->m_port.GetPixMap();
				const size_t width = pixMap->m_rect.right - pixMap->m_rect.left;
				const size_t height = pixMap->m_rect.bottom - pixMap->m_rect.top;
				displayDriver->DrawSurface(m_menuBarGraf->m_ddSurface, 0, 0, width, height);
			}
		}

		if (DrawSurface *renderedMenu = m_menuSelectionState.GetRenderedMenu())
		{
			renderedMenu->PushToDDSurface(displayDriver);

			Menu *selectedMenu = *m_menuSelectionState.GetSelectedMenu();
			const PixMap *pixMap = *renderedMenu->m_port.GetPixMap();

			const size_t xCoordinate = kMenuBarInitialPadding + selectedMenu->menuIndex * kMenuBarItemPadding * 2 + selectedMenu->cumulativeOffset - kMenuBarItemPadding;

			displayDriver->DrawSurface(renderedMenu->m_ddSurface, xCoordinate, kMenuBarHeight, pixMap->m_rect.right, pixMap->m_rect.bottom);
		}
	}

	void MenuManagerImpl::RefreshMenuBarLayout()
	{
		if (m_haveMenuBarLayout)
			return;

		PortabilityLayer::FontManager *fontManager = PortabilityLayer::FontManager::GetInstance();

		PortabilityLayer::FontFamily *fontFamily = PortabilityLayer::FontManager::GetInstance()->GetSystemFont(kMenuFontSize, kMenuFontFlags);
		if (!fontFamily)
			return;

		PortabilityLayer::RenderedFont *rfont = PortabilityLayer::FontManager::GetInstance()->GetRenderedFontFromFamily(fontFamily, kMenuFontSize, true, kMenuFontFlags);
		if (!rfont)
			return;

		unsigned int index = 0;
		size_t measuredWidth = 0;

		Menu **menuHdl = m_firstMenu;
		while (menuHdl)
		{
			Menu *menu = *menuHdl;

			menu->menuIndex = index++;
			menu->cumulativeOffset = measuredWidth;

			const PLPasStr pascalStr = PLPasStr(static_cast<const uint8_t*>(menu->stringBlobHandle->m_contents));

			if (pascalStr.Length() == 1 && pascalStr.UChars()[0] == 0x14)
			{
				menu->unpaddedTitleWidth = 16;
				menu->isIcon = true;
			}
			else
				menu->unpaddedTitleWidth = rfont->MeasureString(pascalStr.UChars(), pascalStr.Length());

			measuredWidth += menu->unpaddedTitleWidth;

			menuHdl = menu->nextMenu;
		}

		m_haveMenuBarLayout = true;
	}

	void MenuManagerImpl::RefreshMenuLayout(Menu *menu)
	{
		if (menu->haveMenuLayout)
			return;

		PortabilityLayer::FontManager *fontManager = PortabilityLayer::FontManager::GetInstance();

		PortabilityLayer::FontFamily *fontFamily = PortabilityLayer::FontManager::GetInstance()->GetSystemFont(kMenuFontSize, kMenuFontFlags);
		if (!fontFamily)
			return;

		PortabilityLayer::RenderedFont *rfont = PortabilityLayer::FontManager::GetInstance()->GetRenderedFontFromFamily(fontFamily, kMenuFontSize, true, kMenuFontFlags);
		if (!rfont)
			return;

		const uint8_t *strBlob = static_cast<const uint8_t*>(menu->stringBlobHandle->m_contents);

		size_t cumulativeHeight = 0;
		size_t width = menu->width;

		const size_t numItems = menu->numMenuItems;
		for (size_t i = 0; i < numItems; i++)
		{
			MenuItem &item = menu->menuItems[i];
			item.layoutYOffset = static_cast<uint16_t>(cumulativeHeight);
			item.layoutHeight = kMenuItemHeight;

			const uint8_t *itemName = strBlob + item.nameOffsetInStringBlob;
			const PLPasStr itemNamePStr = PLPasStr(itemName);

			const size_t nameWidth = rfont->MeasureString(itemNamePStr.UChars(), itemNamePStr.Length());

			const size_t paddedWidth = nameWidth + kMenuItemLeftPadding + kMenuItemRightPadding;

			width = std::max<size_t>(width, paddedWidth);

			cumulativeHeight += item.layoutHeight;
		}

		menu->haveMenuLayout = true;
		menu->layoutWidth = width;
		menu->layoutHeight = cumulativeHeight;
	}

	void MenuManagerImpl::ProcessMouseMoveTo(const Vec2i &point)
	{
		if (point.m_y < 0)
			return;

		if (point.m_y < static_cast<int>(kMenuBarHeight))
			ProcessMouseMoveToMenuBar(point);
		else
			ProcessMouseMoveToMenu(point);
	}

	void MenuManagerImpl::ProcessMouseMoveToMenuBar(const Vec2i &point)
	{
		if (point.m_x < 0)
			return;

		const uint32_t mouseXCoordinate = static_cast<uint32_t>(point.m_x);

		Menu **selectedMenu = nullptr;

		Menu **menuHdl = m_firstMenu;

		while (menuHdl)
		{
			Menu *menu = *menuHdl;

			const size_t menuLeftXCoordinate = kMenuBarInitialPadding + menu->cumulativeOffset + menu->menuIndex * kMenuBarItemPadding * 2 - kMenuBarItemPadding;
			const size_t menuRightXCoordinate = menuLeftXCoordinate + menu->unpaddedTitleWidth + kMenuBarItemPadding * 2;

			if (mouseXCoordinate >= menuLeftXCoordinate && mouseXCoordinate < menuRightXCoordinate)
			{
				selectedMenu = menuHdl;
				break;
			}

			menuHdl = menu->nextMenu;
		}

		if (selectedMenu)
		{
			bool needRedraw = false;
			m_menuSelectionState.HandleSelectionOfMenu(this, menuHdl, needRedraw);

			if (needRedraw)
				DrawMenuBar();
		}
	}

	void MenuManagerImpl::ProcessMouseMoveToMenu(const Vec2i &point)
	{
		Menu **selectedMenuHandle = m_menuSelectionState.GetSelectedMenu();

		if (selectedMenuHandle)
		{
			Menu *menu = *selectedMenuHandle;
			const int32_t xCoordinate = static_cast<int32_t>(kMenuBarInitialPadding + menu->menuIndex * kMenuBarItemPadding * 2 + menu->cumulativeOffset - kMenuBarItemPadding);

			const Vec2i localPoint = point - Vec2i(xCoordinate, kMenuBarHeight);

			if (localPoint.m_x < 0 || localPoint.m_y < 0 || static_cast<size_t>(localPoint.m_x) >= menu->layoutWidth || static_cast<size_t>(localPoint.m_y) >= menu->layoutHeight)
				return;

			const size_t localY = localPoint.m_y;

			for (size_t i = 0; i < menu->numMenuItems; i++)
			{
				const MenuItem &item = menu->menuItems[i];

				if (localY >= item.layoutYOffset && localY - item.layoutYOffset < item.layoutHeight)
				{
					m_menuSelectionState.SelectItem(i);
					return;
				}
			}
		}

		m_menuSelectionState.ClearSelection();
	}

	MenuManagerImpl *MenuManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	MenuManagerImpl MenuManagerImpl::ms_instance;

	MenuManagerImpl::MenuSelectionState::MenuSelectionState()
		: m_menuGraf(nullptr)
		, m_haveItem(false)
		, m_itemIndex(0)
	{
	}

	MenuManagerImpl::MenuSelectionState::~MenuSelectionState()
	{
		Dismiss();
	}

	void MenuManagerImpl::MenuSelectionState::HandleSelectionOfMenu(MenuManagerImpl *mm, Menu **menuHdl, bool &outNeedRedraw)
	{
		outNeedRedraw = false;

		if (!menuHdl)
			return;

		if (menuHdl != m_currentMenu)
		{
			Dismiss();
			m_currentMenu = menuHdl;

			Menu *menu = *menuHdl;

			outNeedRedraw = true;

			mm->RefreshMenuLayout(menu);

			RenderMenu(menu);
		}
	}

	void MenuManagerImpl::MenuSelectionState::Dismiss()
	{
		if (m_menuGraf)
		{
			DisposeGWorld(m_menuGraf);
			m_menuGraf = nullptr;
		}

		m_currentMenu = nullptr;
		m_haveItem = false;
	}

	THandle<Menu> MenuManagerImpl::MenuSelectionState::GetSelectedMenu() const
	{
		return m_currentMenu;
	}

	DrawSurface *MenuManagerImpl::MenuSelectionState::GetRenderedMenu() const
	{
		return m_menuGraf;
	}

	const unsigned int *MenuManagerImpl::MenuSelectionState::GetSelectedItem() const
	{
		if (!m_haveItem)
			return nullptr;

		return &m_itemIndex;
	}

	void MenuManagerImpl::MenuSelectionState::SelectItem(size_t item)
	{
		if (m_haveItem && m_itemIndex == item)
			return;

		m_haveItem = true;
		m_itemIndex = static_cast<unsigned int>(item);

		if (m_currentMenu)
			RenderMenu(*m_currentMenu);
	}

	void MenuManagerImpl::MenuSelectionState::ClearSelection()
	{
		if (!m_haveItem)
			return;

		m_haveItem = false;

		if (m_currentMenu)
			RenderMenu(*m_currentMenu);
	}

	void MenuManagerImpl::MenuSelectionState::RenderMenu(Menu *menu)
	{
		PortabilityLayer::QDManager *qdManager = PortabilityLayer::QDManager::GetInstance();

		const Rect menuRect = Rect::Create(0, 0, static_cast<int16_t>(menu->layoutHeight), static_cast<int16_t>(menu->layoutWidth));

		if (m_menuGraf == nullptr)
		{
			GpPixelFormat_t pixelFormat;
			PortabilityLayer::HostDisplayDriver::GetInstance()->GetDisplayResolution(nullptr, nullptr, &pixelFormat);

			if (qdManager->NewGWorld(&m_menuGraf, pixelFormat, menuRect, nullptr) != 0)
				return;
		}

		DrawSurface *surface = m_menuGraf;

		DrawSurface *oldGraf = GetGraphicsPort();

		SetGraphicsPort(m_menuGraf);

		QDState *qdState = qdManager->GetState();

		qdState->SetForeColor(gs_barMidColor);

		{
			const Rect rect = Rect::Create(0, 0, menu->layoutHeight, menu->layoutWidth);
			surface->FillRect(rect);
		}

		m_menuGraf->SetSystemFont(kMenuFontSize, PortabilityLayer::FontFamilyFlag_Bold);

		const uint8_t *strBlob = static_cast<const uint8_t*>(menu->stringBlobHandle->m_contents);

		Point itemPos = Point::Create(kMenuItemLeftPadding, 0);

		qdState->SetForeColor(gs_barNormalTextColor);

		for (size_t i = 0; i < menu->numMenuItems; i++)
		{
			if (m_haveItem && i == m_itemIndex)
				continue;

			const MenuItem &item = menu->menuItems[i];

			itemPos.v = item.layoutYOffset + kMenuItemTextYOffset;

			surface->DrawString(itemPos, PLPasStr(strBlob + item.nameOffsetInStringBlob), true);
		}

		if (m_haveItem)
		{
			const MenuItem &selectedItem = menu->menuItems[m_itemIndex];
			qdState->SetForeColor(gs_barHighlightMidColor);
			const Rect itemRect = Rect::Create(selectedItem.layoutYOffset, 0, selectedItem.layoutYOffset + selectedItem.layoutHeight, menu->layoutWidth);
			surface->FillRect(itemRect);

			qdState->SetForeColor(gs_barHighlightTextColor);

			const MenuItem &item = menu->menuItems[m_itemIndex];

			itemPos.v = item.layoutYOffset + kMenuItemTextYOffset;

			surface->DrawString(itemPos, PLPasStr(strBlob + item.nameOffsetInStringBlob), true);
		}

		m_menuGraf->m_port.SetDirty(QDPortDirtyFlag_Contents);

		SetGraphicsPort(oldGraf);
	}

	MenuManager *MenuManager::GetInstance()
	{
		return MenuManagerImpl::GetInstance();
	}
}
