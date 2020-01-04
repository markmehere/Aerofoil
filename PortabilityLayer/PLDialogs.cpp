#include "PLDialogs.h"

void DrawDialog(Dialog *dialog)
{
	PL_NotYetImplemented();
}

Dialog *GetNewDialog(int resID, void *unknown, WindowPtr behind)
{
	PL_NotYetImplemented();
	return nullptr;
}

DrawSurface *GetDialogPort(Dialog *dialog)
{
	PL_NotYetImplemented();
	return nullptr;
}

void GetDialogItem(Dialog *dialog, int index, short *itemType, THandle<Control> *itemHandle, Rect *itemRect)
{
	PL_NotYetImplemented();
}

void GetDialogItemText(THandle<Control> handle, StringPtr str)
{
	PL_NotYetImplemented();
}

void SetDialogItem(Dialog *dialog, int index, short itemType, THandle<Control> itemHandle, const Rect *itemRect)
{
	PL_NotYetImplemented();
}

void SetDialogItemText(THandle<Control> handle, const PLPasStr &str)
{
	PL_NotYetImplemented();
}

void SelectDialogItemText(Dialog *dialog, int item, int firstSelChar, int lastSelCharExclusive)
{
	PL_NotYetImplemented();
}

ModalFilterUPP NewModalFilterUPP(ModalFilterUPP func)
{
	return func;
}

void ModalDialog(ModalFilterUPP filter, short *item)
{
	PL_NotYetImplemented();
}

void DisposeModalFilterUPP(ModalFilterUPP upp)
{
	PL_NotYetImplemented();
}

void ShowDialogItem(Dialog *dialog, int item)
{
	PL_NotYetImplemented();
}

void HideDialogItem(Dialog *dialog, int item)
{
	PL_NotYetImplemented();
}
