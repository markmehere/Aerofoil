#pragma once
#ifndef __PL_MENUS_H__
#define __PL_MENUS_H__

#include "PLCore.h"

class PLPasStr;

MenuHandle GetMenu(int resID);
void InsertMenu(MenuHandle menu, int beforeID);

void EnableMenuItem(MenuHandle menu, int index);
void DisableMenuItem(MenuHandle menu, int index);
void CheckMenuItem(MenuHandle menu, int index, Boolean checked);
void SetMenuItemText(MenuHandle menu, int index, const PLPasStr &text);

void AppendMenuItem(MenuHandle menu, int8_t iconResID, uint8_t key, uint8_t submenuID, uint8_t textStyle, bool enabled, bool checked, const PLPasStr &text);

#endif
