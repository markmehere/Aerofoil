#pragma once
#ifndef __PL_MENUS_H__
#define __PL_MENUS_H__

#include "PLCore.h"

class PLPasStr;

MenuHandle GetMenu(int resID);
void InsertMenu(MenuHandle menu, int beforeID);
void DeleteMenu(int menuID);	// ???
void DrawMenuBar();
void HiliteMenu(int menu);

void EnableMenuItem(MenuHandle menu, int index);
void DisableMenuItem(MenuHandle menu, int index);
void CheckMenuItem(MenuHandle menu, int index, Boolean checked);
void SetMenuItemText(MenuHandle menu, int index, const PLPasStr &text);

#endif
