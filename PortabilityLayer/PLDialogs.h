#pragma once
#ifndef __PL_DIALOGS_H__
#define __PL_DIALOGS_H__

#include "PLCore.h"

template<class T>
class ArrayView;

class PLPasStr;
struct Control;

struct DialogItem
{
	virtual Rect GetRect() const = 0;
};

struct Dialog
{
	virtual void Destroy() = 0;
	virtual Window *GetWindow() const = 0;
	virtual ArrayView<DialogItem*const> GetItems() const = 0;
};

enum TEMode
{
	teCenter
};

typedef Boolean(*ModalFilterUPP)(Dialog *dial, EventRecord *event, short *item);

void DrawDialog(Dialog *dialog);
DrawSurface *GetDialogPort(Dialog *dialog);

void GetDialogItem(Dialog *dialog, int index, short *itemType, THandle<Control> *itemHandle, Rect *itemRect);
void GetDialogItemText(THandle<Control> handle, StringPtr str);

void SetDialogItem(Dialog *dialog, int index, short itemType, THandle<Control> itemHandle, const Rect *itemRect);
void SetDialogItemText(THandle<Control> handle, const PLPasStr &str);

void SelectDialogItemText(Dialog *dialog, int item, int firstSelChar, int lastSelCharExclusive);

ModalFilterUPP NewModalFilterUPP(ModalFilterUPP func);

void ModalDialog(ModalFilterUPP filter, short *item);

void DisposeDialog(Dialog *dialog);
void DisposeModalFilterUPP(ModalFilterUPP upp);

void ShowDialogItem(Dialog *dialog, int item);
void HideDialogItem(Dialog *dialog, int item);

void TETextBox(const PLPasStr &str, short len, const Rect *rect, TEMode teMode);

#endif
