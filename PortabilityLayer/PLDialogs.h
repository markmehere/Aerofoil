#pragma once
#ifndef __PL_DIALOGS_H__
#define __PL_DIALOGS_H__

#include "PLCore.h"

class PLPasStr;
struct Control;

struct Dialog
{
	virtual void Destroy() = 0;
	virtual Window *GetWindow() const = 0;
};

struct DialogTemplate
{
	// FIXME: Audit
	Rect boundsRect;
};

enum TEMode
{
	teCenter
};

typedef Dialog *DialogPtr;

typedef THandle<DialogTemplate> DialogTHndl;


typedef Boolean(*ModalFilterUPP)(DialogPtr dial, EventRecord *event, short *item);

void DrawDialog(DialogPtr dialog);
DrawSurface *GetDialogPort(DialogPtr dialog);

void GetDialogItem(DialogPtr dialog, int index, short *itemType, THandle<Control> *itemHandle, Rect *itemRect);
void GetDialogItemText(THandle<Control> handle, StringPtr str);

void SetDialogItem(DialogPtr dialog, int index, short itemType, THandle<Control> itemHandle, const Rect *itemRect);
void SetDialogItemText(THandle<Control> handle, const PLPasStr &str);

void SelectDialogItemText(DialogPtr dialog, int item, int firstSelChar, int lastSelCharExclusive);

ModalFilterUPP NewModalFilterUPP(ModalFilterUPP func);

void ModalDialog(ModalFilterUPP filter, short *item);

void DisposeDialog(DialogPtr dialog);
void DisposeModalFilterUPP(ModalFilterUPP upp);

void ShowDialogItem(DialogPtr dialog, int item);
void HideDialogItem(DialogPtr dialog, int item);

void TETextBox(const PLPasStr &str, short len, const Rect *rect, TEMode teMode);

#endif
