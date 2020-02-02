#pragma once

#include "PLCore.h"

namespace PortabilityLayer
{
	class DialogItem;
}

template<class T>
class ArrayView;

class PLPasStr;
struct Control;
struct Dialog;

typedef int16_t(*DialogFilterFunc_t)(Dialog *dialog, const TimeTaggedVOSEvent &evt);

struct DialogTextSubstitutions
{
	Str255 m_strings[4];

	DialogTextSubstitutions();
	explicit DialogTextSubstitutions(const PLPasStr& str0);
	DialogTextSubstitutions(const PLPasStr& str0, const PLPasStr& str1);
	DialogTextSubstitutions(const PLPasStr& str0, const PLPasStr& str1, const PLPasStr& str2);
	DialogTextSubstitutions(const PLPasStr& str0, const PLPasStr& str1, const PLPasStr& str2, const PLPasStr& str3);

private:
	void Init(size_t numItems, const PLPasStr *items);
};

struct Dialog
{
	virtual void Destroy() = 0;

	virtual Window *GetWindow() const = 0;
	virtual ArrayView<const PortabilityLayer::DialogItem> GetItems() const = 0;

	virtual void SetItemVisibility(unsigned int itemIndex, bool isVisible) = 0;

	virtual int16_t ExecuteModal(DialogFilterFunc_t filterFunc) = 0;
};

typedef Boolean(*ModalFilterUPP)(Dialog *dial, EventRecord *event, short *item);

void DrawDialog(Dialog *dialog);
DrawSurface *GetDialogPort(Dialog *dialog);

void GetDialogItemText(THandle<Control> handle, StringPtr str);

void SetDialogItem(Dialog *dialog, int index, short itemType, THandle<Control> itemHandle, const Rect *itemRect);
void SetDialogItemText(THandle<Control> handle, const PLPasStr &str);

void SelectDialogItemText(Dialog *dialog, int item, int firstSelChar, int lastSelCharExclusive);

void ModalDialog(ModalFilterUPP filter, short *item);

void ShowDialogItem(Dialog *dialog, int item);
void HideDialogItem(Dialog *dialog, int item);
