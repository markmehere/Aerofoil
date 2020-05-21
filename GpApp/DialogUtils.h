//============================================================================
//----------------------------------------------------------------------------
//								DialogUtils.h
//----------------------------------------------------------------------------
//============================================================================


#include "PLDialogs.h"

namespace PortabilityLayer
{
	class ResolveCachingColor;
}

struct DialogTextSubstitutions;

void BringUpDialog (Dialog **theDialog, short dialogID, const DialogTextSubstitutions *);
//void GetPutDialogCorner (Point *);
//void GetGetDialogCorner (Point *);
//void CenterDialog (short);
void GetDialogRect (Rect *, short);
//void TrueCenterDialog (short);
//void CenterAlert (short);
//void ZoomOutDialogRect (short);
//void ZoomOutAlertRect (short);
void FlashDialogButton (Dialog *, short);
void DrawDefaultButton (Dialog *);
void GetDialogString (Dialog *, short, StringPtr);
void SetDialogString (Dialog *, short, const PLPasStr&);
short GetDialogStringLen (Dialog *, short);
void GetDialogItemValue (Dialog *, short, short *);
void SetDialogItemValue (Dialog *, short, short);
void ToggleDialogItemValue (Dialog *, short);
void SetDialogNumToStr (Dialog *, short, long);
void GetDialogNumFromStr (Dialog *, short, long *);
void GetDialogItemRect (Dialog *, short, Rect *);
void SetDialogItemRect (Dialog *, short, Rect *);
void OffsetDialogItemRect (Dialog *, short, short, short);
void SelectFromRadioGroup (Dialog *, short, short, short);
//void AddMenuToPopUp (Dialog *, short, MenuHandle);
void GetPopUpMenuValue (Dialog *, short, short *);
void SetPopUpMenuValue (Dialog *, short, short);
void MyEnableControl(Dialog *, short);
void MyDisableControl(Dialog *, short);
void DrawDialogUserText (Dialog *, short, StringPtr, Boolean);
void DrawDialogUserText2 (Dialog *, short, StringPtr);
void LoadDialogPICT (Dialog *, short, short);
void FrameDialogItem (Dialog *, short, PortabilityLayer::ResolveCachingColor &color);
void FrameDialogItemC (Dialog *, short, long);
void FrameOvalDialogItem (Dialog *, short, PortabilityLayer::ResolveCachingColor &color);
void BorderDialogItem (Dialog *theDialog, short item, short sides, short thickness, const uint8_t *pattern);
void ShadowDialogItem (Dialog *, short, short);
void EraseDialogItem (Dialog *, short);
