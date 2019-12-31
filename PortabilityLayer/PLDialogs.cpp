#include "PLDialogs.h"

void DrawDialog(DialogPtr dialog)
{
	PL_NotYetImplemented();
}

DialogPtr GetNewDialog(int resID, void *unknown, WindowPtr behind)
{
	PL_NotYetImplemented();
	return nullptr;
}

DrawSurface *GetDialogPort(DialogPtr dialog)
{
	PL_NotYetImplemented();
	return nullptr;
}

void GetDialogItem(DialogPtr dialog, int index, short *itemType, THandle<Control> *itemHandle, Rect *itemRect)
{
	PL_NotYetImplemented();
}

void GetDialogItemText(THandle<Control> handle, StringPtr str)
{
	PL_NotYetImplemented();
}

void SetDialogItem(DialogPtr dialog, int index, short itemType, THandle<Control> itemHandle, const Rect *itemRect)
{
	PL_NotYetImplemented();
}

void SetDialogItemText(THandle<Control> handle, const PLPasStr &str)
{
	PL_NotYetImplemented();
}

void SelectDialogItemText(DialogPtr dialog, int item, int firstSelChar, int lastSelCharExclusive)
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

void DisposeDialog(DialogPtr dialog)
{
	PL_NotYetImplemented();
}

void DisposeModalFilterUPP(ModalFilterUPP upp)
{
	PL_NotYetImplemented();
}

void ShowDialogItem(DialogPtr dialog, int item)
{
	PL_NotYetImplemented();
}

void HideDialogItem(DialogPtr dialog, int item)
{
	PL_NotYetImplemented();
}

void TETextBox(const PLPasStr &str, short len, const Rect *rect, TEMode teMode)
{
	PL_NotYetImplemented();
}
