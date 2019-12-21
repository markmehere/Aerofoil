#include "MenuManager.h"
#include "PLBigEndian.h"
#include "MemoryManager.h"
#include <stdint.h>

struct MenuItem
{
	uint32_t nameOffsetInStringBlob;
	int8_t iconResID;
	uint8_t key;
	uint8_t submenuID;
	uint8_t textStyle;
};

struct Menu
{
	uint16_t menuID;
	uint16_t width;
	uint16_t height;
	uint16_t commandID;
	uint32_t itemEnabledMask;

	PortabilityLayer::MMHandleBlock *stringBlobHandle;

	Menu **prevMenu;
	Menu **nextMenu;

	size_t numMenuItems;
	MenuItem menuItems[1];
};

namespace PortabilityLayer
{
	class MenuManagerImpl final : public MenuManager
	{
	public:
		MenuManagerImpl();
		~MenuManagerImpl();

		Menu **DeserializeMenu(const void *resData) const override;
		Menu **GetMenuByID(int id) const override;
		void InsertMenuBefore(Menu **insertingMenu, Menu **existingMenu) override;
		void InsertMenuAfter(Menu **insertingMenu, Menu **existingMenu) override;
		void InsertMenuAtEnd(Menu **insertingMenu) override;
		void InsertMenuAtBeginning(Menu **insertingMenu) override;

		static MenuManagerImpl *GetInstance();

	private:
		Menu **m_firstMenu;
		Menu **m_lastMenu;

		static MenuManagerImpl ms_instance;
	};

	MenuManagerImpl::MenuManagerImpl()
		: m_firstMenu(nullptr)
		, m_lastMenu(nullptr)
	{
	}

	MenuManagerImpl::~MenuManagerImpl()
	{
	}

	Menu **MenuManagerImpl::DeserializeMenu(const void *resData) const
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

		Menu *menu = static_cast<Menu*>(menuData->m_contents);
		menu->menuID = header.menuID;
		menu->width = header.width;
		menu->height = header.height;
		menu->commandID = header.commandID;
		menu->itemEnabledMask = header.enableFlags;

		uint8_t *stringDataStart = static_cast<uint8_t*>(stringData->m_contents);
		uint8_t *stringDest = stringDataStart;
		memcpy(stringDest, resBytes + 14, 1 + resBytes[14]);
		stringDest += 1 + resBytes[14];

		MenuItem *currentItem = menu->menuItems;
		for (const uint8_t *menuItemStart = menuDataStart; *menuItemStart; menuItemStart += 5 + (*menuItemStart))
		{
			uint8_t itemNameLength = *menuItemStart;

			memcpy(stringDest, menuItemStart, 1 + itemNameLength);

			currentItem->iconResID = static_cast<int8_t>(menuItemStart[1 + itemNameLength]);
			currentItem->key = menuItemStart[2 + itemNameLength];
			currentItem->submenuID = menuItemStart[3 + itemNameLength];
			currentItem->textStyle = menuItemStart[4 + itemNameLength];
			currentItem->nameOffsetInStringBlob = stringDest - stringDataStart;

			currentItem++;
			stringDest += 1 + (*menuItemStart);
		}

		menu->numMenuItems = numMenuItems;
		menu->stringBlobHandle = stringData;
		menu->prevMenu = nullptr;
		menu->nextMenu = nullptr;

		return reinterpret_cast<Menu**>(&menuData->m_contents);
	}

	Menu **MenuManagerImpl::GetMenuByID(int id) const
	{
		for (Menu **menuHandle = m_firstMenu; menuHandle; menuHandle = (*menuHandle)->nextMenu)
		{
			if ((*menuHandle)->menuID == id)
				return menuHandle;
		}

		return nullptr;
	}

	void MenuManagerImpl::InsertMenuBefore(Menu **insertingMenu, Menu **existingMenu)
	{
		Menu *insertingMenuPtr = *insertingMenu;
		Menu *existingMenuPtr = *existingMenu;

		insertingMenuPtr->prevMenu = existingMenuPtr->prevMenu;
		insertingMenuPtr->nextMenu = existingMenu;

		if (existingMenuPtr->prevMenu)
			(*existingMenuPtr->prevMenu)->nextMenu = insertingMenu;
		else
			m_firstMenu = insertingMenu;

		existingMenuPtr->prevMenu = insertingMenu;
	}

	void MenuManagerImpl::InsertMenuAfter(Menu **insertingMenu, Menu **existingMenu)
	{
		Menu *insertingMenuPtr = *insertingMenu;
		Menu *existingMenuPtr = *existingMenu;

		insertingMenuPtr->prevMenu = existingMenu;
		insertingMenuPtr->nextMenu = existingMenuPtr->nextMenu;

		if (existingMenuPtr->nextMenu)
			(*existingMenuPtr->nextMenu)->prevMenu = insertingMenu;
		else
			m_lastMenu = insertingMenu;

		existingMenuPtr->nextMenu = insertingMenu;
	}

	void MenuManagerImpl::InsertMenuAtEnd(Menu **insertingMenu)
	{
		if (m_firstMenu == nullptr)
		{
			m_firstMenu = m_lastMenu = insertingMenu;
			return;
		}

		(*m_lastMenu)->nextMenu = insertingMenu;
		(*insertingMenu)->prevMenu = m_lastMenu;
		m_lastMenu = insertingMenu;
	}

	void MenuManagerImpl::InsertMenuAtBeginning(Menu **insertingMenu)
	{
		if (m_firstMenu == nullptr)
		{
			m_firstMenu = m_lastMenu = insertingMenu;
			return;
		}

		(*m_firstMenu)->prevMenu = insertingMenu;
		(*insertingMenu)->nextMenu = m_firstMenu;
		m_firstMenu = insertingMenu;
	}

	MenuManagerImpl *MenuManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	MenuManagerImpl MenuManagerImpl::ms_instance;

	MenuManager *MenuManager::GetInstance()
	{
		return MenuManagerImpl::GetInstance();
	}
}
