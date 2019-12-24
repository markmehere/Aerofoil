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
#include "PLQDOffscreen.h"
#include "RenderedFont.h"
#include "QDGraf.h"
#include "QDManager.h"
#include "QDPixMap.h"
#include "RGBAColor.h"

#include <stdint.h>
#include <assert.h>

namespace
{
	const PortabilityLayer::RGBAColor gs_barTopLeftCornerGraphicPixels[] =
	{
		{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 85, 85, 85, 255 }, { 170, 170, 170, 255 },
		{ 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 85, 85, 85, 255 }, { 255, 255, 255, 255 }, { 255, 255, 255, 255 },
		{ 0, 0, 0, 255 }, { 85, 85, 85, 255 }, { 255, 255, 255, 255 }, { 221, 221, 221, 255 }, { 221, 221, 221, 255 },
		{ 85, 85, 85, 255 }, { 255, 255, 255, 255 }, { 221, 221, 221, 255 }, { 221, 221, 221, 255 }, { 221, 221, 221, 255 },
		{ 170, 170, 170, 255 }, { 255, 255, 255, 255 }, { 221, 221, 221, 255 }, { 221, 221, 221, 255 }, { 221, 221, 221, 255 },
	};

	const PortabilityLayer::RGBAColor gs_barTopRightCornerGraphicPixels[] =
	{
		{ 170, 170, 170, 255 }, { 85, 85, 85, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 },
		{ 255, 255, 255, 255 }, { 255, 255, 255, 255 }, { 85, 85, 85, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 255 },
		{ 221, 221, 221, 255 }, { 221, 221, 221, 255 }, { 255, 255, 255, 255 }, { 85, 85, 85, 255 }, { 0, 0, 0, 255 },
		{ 221, 221, 221, 255 }, { 221, 221, 221, 255 }, { 221, 221, 221, 255 }, { 255, 255, 255, 255 }, { 85, 85, 85, 255 },
		{ 221, 221, 221, 255 }, { 221, 221, 221, 255 }, { 221, 221, 221, 255 }, { 255, 255, 255, 255 }, { 85, 85, 85, 255 },
	};

	const PortabilityLayer::RGBAColor gs_barBrightColor = { 255, 255, 255, 255 };
	const PortabilityLayer::RGBAColor gs_barMidColor = { 221, 221, 221, 255 };
	const PortabilityLayer::RGBAColor gs_barDarkColor = { 102, 102, 102, 255 };
	const PortabilityLayer::RGBAColor gs_barBottomEdgeColor = { 0, 0, 0, 255 };
	const PortabilityLayer::RGBAColor gs_barNormalTextColor = { 0, 0, 0, 255 };

	const PortabilityLayer::RGBAColor gs_barHighlightBrightColor = { 153, 153, 255, 255 };
	const PortabilityLayer::RGBAColor gs_barHighlightMidColor = { 102, 102, 204, 255 };
	const PortabilityLayer::RGBAColor gs_barHighlightDarkColor = { 51, 51, 102, 255 };

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
};

struct Menu
{
	uint16_t menuID;
	uint16_t width;
	uint16_t height;
	uint16_t commandID;
	bool enabled;
	bool isIcon;

	PortabilityLayer::MMHandleBlock *stringBlobHandle;

	Menu **prevMenu;
	Menu **nextMenu;

	// Refreshed on layout
	size_t cumulativeOffset;
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

		Menu **DeserializeMenu(const void *resData) const override;
		Menu **GetMenuByID(int id) const override;
		void InsertMenuBefore(Menu **insertingMenu, Menu **existingMenu) override;
		void InsertMenuAfter(Menu **insertingMenu, Menu **existingMenu) override;
		void InsertMenuAtEnd(Menu **insertingMenu) override;
		void InsertMenuAtBeginning(Menu **insertingMenu) override;
		void SetMenuEnabled(Menu **menuHandle, bool enabled) override;
		void SetItemEnabled(Menu **menu, unsigned int index, bool enabled) override;
		void SetItemChecked(Menu **menu, unsigned int index, bool checked) override;

		void DrawMenuBar() override;

		void RenderFrame(IGpDisplayDriver *displayDriver) override;

		static MenuManagerImpl *GetInstance();

	private:
		void RefreshMenuLayout();

		static const unsigned int kIconResID = 128;
		static const unsigned int kMenuFontSize = 12;
		static const unsigned int kMenuBarIconYOffset = 2;
		static const unsigned int kMenuBarTextYOffset = 14;
		static const unsigned int kMenuBarHeight = 20;
		static const unsigned int kMenuBarItemPadding = 6;
		static const unsigned int kMenuBarInitialPadding = 16;
		static const int kMenuFontFlags = PortabilityLayer::FontFamilyFlag_Bold;

		CGraf *m_menuBarGraf;

		Menu **m_firstMenu;
		Menu **m_lastMenu;
		bool m_haveMenuLayout;
		bool m_haveIcon;

		uint8_t m_iconColors[16 * 16];
		uint8_t m_iconMask[32];

		SimpleGraphic *m_iconGraphic;

		static MenuManagerImpl ms_instance;
	};

	MenuManagerImpl::MenuManagerImpl()
		: m_menuBarGraf(nullptr)
		, m_firstMenu(nullptr)
		, m_lastMenu(nullptr)
		, m_haveMenuLayout(false)
		, m_haveIcon(false)
		, m_iconGraphic(nullptr)
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

		uint32_t enableFlags = header.enableFlags;

		Menu *menu = static_cast<Menu*>(menuData->m_contents);
		menu->menuID = header.menuID;
		menu->width = header.width;
		menu->height = header.height;
		menu->commandID = header.commandID;
		menu->enabled = ((enableFlags & 1) != 0);
		menu->menuIndex = 0;
		menu->cumulativeOffset = 0;
		menu->isIcon = false;

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

			enableFlags >>= 1;

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
		m_haveMenuLayout = false;

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
		m_haveMenuLayout = false;

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
		m_haveMenuLayout = false;

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
		m_haveMenuLayout = false;

		if (m_firstMenu == nullptr)
		{
			m_firstMenu = m_lastMenu = insertingMenu;
			return;
		}

		(*m_firstMenu)->prevMenu = insertingMenu;
		(*insertingMenu)->nextMenu = m_firstMenu;
		m_firstMenu = insertingMenu;
	}

	void MenuManagerImpl::SetMenuEnabled(Menu **menuHandle, bool enabled)
	{
		Menu *menu = *menuHandle;

		menu->enabled = enabled;
	}

	void MenuManagerImpl::SetItemEnabled(Menu **menuHandle, unsigned int index, bool enabled)
	{
		Menu *menu = *menuHandle;

		if (index >= menu->numMenuItems)
			return;

		menu->menuItems[index].enabled = enabled;
	}

	void MenuManagerImpl::SetItemChecked(Menu **menuHandle, unsigned int index, bool checked)
	{
		Menu *menu = *menuHandle;

		if (index >= menu->numMenuItems)
			return;

		menu->menuItems[index].checked = checked;
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
				ReleaseResource(icsHandle);

			if (ics8Handle)
				ReleaseResource(ics8Handle);

			m_haveIcon = true;
		}

		unsigned int width;
		GpPixelFormat_t pixelFormat;
		PortabilityLayer::HostDisplayDriver::GetInstance()->GetDisplayResolution(&width, nullptr, &pixelFormat);

		PortabilityLayer::QDManager *qdManager = PortabilityLayer::QDManager::GetInstance();

		const Rect menuRect = Rect::Create(0, 0, kMenuBarHeight, width);

		if (m_menuBarGraf == nullptr)
		{
			int depth = 0;

			switch (pixelFormat)
			{
			case GpPixelFormats::k8BitStandard:
				depth = 8;
				break;
			default:
				PL_NotYetImplemented();
				return;
			}

			if (qdManager->NewGWorld(&m_menuBarGraf, depth, menuRect, nullptr, nullptr, 0) != 0)
				return;
		}

		CGraf *graf = m_menuBarGraf;

		assert(graf);

		if (static_cast<unsigned int>(graf->m_port.GetRect().right) != width)
		{
			if (!graf->m_port.Resize(menuRect))
				return;
		}


		RefreshMenuLayout();

		CGraf *oldGraf;
		GDHandle oldDevice;
		GetGWorld(&oldGraf, &oldDevice);

		SetGWorld(m_menuBarGraf, nullptr);

		PortabilityLayer::QDState *qdState = qdManager->GetState();

		qdState->SetForeColor(gs_barMidColor);
		PaintRect(&menuRect);

		qdState->SetForeColor(gs_barBrightColor);

		// Top stripe
		{
			const Rect rect = Rect::Create(0, 0, 1, static_cast<int16_t>(width) - 1);
			PaintRect(&rect);
		}

		// Left stripe
		{
			const Rect rect = Rect::Create(0, 0, kMenuBarHeight - 1, 1);
			PaintRect(&rect);
		}

		qdState->SetForeColor(gs_barDarkColor);

		// Bottom stripe
		{
			const Rect rect = Rect::Create(kMenuBarHeight - 2, 1, kMenuBarHeight - 1, width);
			PaintRect(&rect);
		}

		// Right stripe
		{
			const Rect rect = Rect::Create(0, width - 1, kMenuBarHeight - 1, width);
			PaintRect(&rect);
		}

		qdState->SetForeColor(gs_barBottomEdgeColor);

		// Bottom edge
		{
			const Rect rect = Rect::Create(kMenuBarHeight - 1, 0, kMenuBarHeight, width);
			PaintRect(&rect);
		}

		PixMapHandle pixMap = m_menuBarGraf->m_port.GetPixMap();

		// Round corners
		gs_barTopLeftCornerGraphic.DrawToPixMap(pixMap, 0, 0);
		gs_barTopRightCornerGraphic.DrawToPixMap(pixMap, static_cast<int16_t>(width) - static_cast<int16_t>(gs_barTopRightCornerGraphic.m_width), 0);

		qdState->SetForeColor(gs_barNormalTextColor);
		TextFont(systemFont);
		TextSize(kMenuFontSize);

		// Text items
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
							m_iconGraphic->DrawToPixMapWithMask(pixMap, m_iconMask, xCoordinate, kMenuBarIconYOffset);
					}
					else
					{
						qdState->m_penPos.h = xCoordinate;
						qdState->m_penPos.v = kMenuBarTextYOffset;
						DrawString(PLPasStr(static_cast<const uint8_t*>(menu->stringBlobHandle->m_contents)));
					}
				}

				menuHdl = menu->nextMenu;
			}
		}

		SetGWorld(oldGraf, oldDevice);

		m_menuBarGraf->m_port.SetDirty(QDPortDirtyFlag_Contents);
	}

	void MenuManagerImpl::RenderFrame(IGpDisplayDriver *displayDriver)
	{
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
	}

	void MenuManagerImpl::RefreshMenuLayout()
	{
		if (m_haveMenuLayout)
			return;

		PortabilityLayer::FontManager *fontManager = PortabilityLayer::FontManager::GetInstance();

		PortabilityLayer::FontFamily *fontFamily = PortabilityLayer::FontManager::GetInstance()->GetSystemFont(kMenuFontSize, kMenuFontFlags);
		if (!fontFamily)
			return;

		PortabilityLayer::RenderedFont *rfont = PortabilityLayer::FontManager::GetInstance()->GetRenderedFontFromFamily(fontFamily, kMenuFontSize, kMenuFontFlags);
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
				measuredWidth += 16;
				menu->isIcon = true;
			}
			else
				measuredWidth += rfont->MeasureString(pascalStr.UChars(), pascalStr.Length());

			menuHdl = menu->nextMenu;
		}

		m_haveMenuLayout = true;
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
