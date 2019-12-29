
//============================================================================
//----------------------------------------------------------------------------
//								Externs.h
//----------------------------------------------------------------------------
//============================================================================


#pragma once


#include "PLMenus.h"

namespace PortabilityLayer
{
	class ScanlineMask;
}

#define kPreferredDepth				8
#define kPreferredPixelFormat		(GpPixelFormats::k8BitStandard)


#define	kNilPointer					0L
#define	kPutInFront					(PL_GetPutInFrontWindowPtr())
#define	kNormalUpdates				TRUE
#define kOneKilobyte				1024
#define kOkayButton					1
#define kCancelButton				2
#define kControlActive				0
#define kControlInactive			255
#define kAsynch						TRUE
#define kSynch						FALSE

#define	kErrUnnaccounted			1
#define	kErrNoMemory				2
#define kErrDialogDidntLoad			3
#define kErrFailedResourceLoad		4
#define kErrFailedGraphicLoad		5
#define kErrFailedOurDirect			6
#define kErrFailedValidation		7
#define kErrNeedSystem7				8
#define kErrFailedGetDevice			9
#define kErrFailedMemoryOperation	10
#define kErrFailedCatSearch			11
#define kErrNeedColorQD				12
#define kErrNeed16Or256Colors		13

#define iAbout					1
#define iNewGame				1
#define iTwoPlayer				2
#define iOpenSavedGame			3
#define iLoadHouse				5
#define iQuit					7
#define iEditor					1
#define iHighScores				3
#define iPrefs					4
#define iHelp					5
#define iNewHouse				1
#define iSave					2
#define iHouse					4
#define iRoom					5
#define iObject					6
#define iCut					8
#define iCopy					9
#define iPaste					10
#define iClear					11
#define iDuplicate				12
#define iBringForward			14
#define iSendBack				15
#define iGoToRoom				17
#define iMapWindow				19
#define iObjectWindow			20
#define iCoordinateWindow		21

//--------------------------------------------------------------  Structs
/*
typedef	short		SICN[16];
typedef	SICN		*SICNList;
typedef	SICNList	*SICNHand;
*/

typedef struct
{
	Str32		wasDefaultName;
	Str15		wasLeftName, wasRightName;
	Str15		wasBattName, wasBandName;
	Str15		wasHighName;
	Str31		wasHighBanner;
//	long		encrypted, fakeLong;
	long		wasLeftMap, wasRightMap;
	long		wasBattMap, wasBandMap;
	long		wasGPLeftMap, wasGPRightMap;
	long		wasGPBattMap, wasGPBandMap;
	long		wasGPFlipMap;
	long		wasGPFaceLeftMap, wasGPFaceRightMap;
	short		wasVolume;
	short		prefVersion;
	short		wasMaxFiles;
	short		wasEditH, wasEditV;
	short		wasMapH, wasMapV;
	short		wasMapWide, wasMapHigh;
	short		wasToolsH, wasToolsV;
	short		wasLinkH, wasLinkV;
	short		wasCoordH, wasCoordV;
	short		isMapLeft, isMapTop;
	short		wasNumNeighbors;
	short		wasDepthPref;
	short		wasToolGroup;
	short		smWarnings;
	short		wasFloor, wasSuite;
	Boolean		wasZooms, wasMusicOn;
	Boolean		wasAutoEdit, wasDoColorFade;
	Boolean		wasMapOpen, wasToolsOpen;
	Boolean		wasCoordOpen, wasQuickTrans;
	Boolean		wasIdleMusic, wasGameMusic;
	Boolean		wasEscPauseKey;
	Boolean		wasDoAutoDemo, wasScreen2;
	Boolean		wasDoBackground, wasHouseChecks;
	Boolean		wasPrettyMap, wasBitchDialogs;
} prefsInfo;

//--------------------------------------------------------------  Prototypes

void DoAbout (void);									// --- About.c

void LoadCursors (void);								// --- AnimCursor.c
void DisposCursors (void);
void IncrementCursor (void);
void DecrementCursor (void);
void SpinCursor (short);
void BackSpinCursor (short);

void ColorText (StringPtr, long);						// --- ColorUtils.c
void ColorRect (Rect *, long);
void ColorOval (Rect *, long);
void ColorRegion (PortabilityLayer::ScanlineMask *scanlineMask, long colorIndex);
void ColorLine (short, short, short, short, long);
void HiliteRect (Rect *, short, short);
void ColorFrameRect (Rect *, long);
void ColorFrameWHRect (short, short, short, short, long);
void ColorFrameOval (Rect *, long);
void LtGrayForeColor (void);
void GrayForeColor (void);
void DkGrayForeColor (void);
void RestoreColorsSlam (void);

void MonitorWait (void);								// --- DebugUtils.c
void DisplayRect (Rect *);
void FlashRect (Rect *);
void CheckLegitRect(Rect *, Rect *);
void DisplayLong (long);
void DisplayShort (short);
void FlashLong (long);
void FlashShort (short);
void DoBarGraph (short, short, short, short);
short BetaOkay (void);
void DebugNum (long);
void DisplayCTSeed (CGrafPtr);
void FillScreenRed (void);
void DumpToResEditFile (Ptr, long);

void HandleEvent (void);								// --- Event.c
void HiliteAllWindows (void);
void IgnoreThisClick (void);

short WhatsOurDepth (void);								// --- Environs.c
void SwitchToDepth (short, Boolean);
void CheckOurEnvirons (void);
//void ReflectSecondMonitorEnvirons (Boolean, Boolean, Boolean);
void HandleDepthSwitching (void);
void RestoreColorDepth (void);
void CheckMemorySize (void);
void SetAppMemorySize (long);

Boolean CheckFileError (short, const PLPasStr &);				// --- File Error.c

Boolean SavePrefs (prefsInfo *, short);					// --- Prefs.c
Boolean LoadPrefs (prefsInfo *, short);

void PasStringCopy (StringPtr, StringPtr);				// --- StringUtils.c
short WhichStringFirst (StringPtr, StringPtr);
void PasStringCopyNum (StringPtr, StringPtr, short);
void PasStringConcat (StringPtr, const PLPasStr &);
void GetLineOfText (StringPtr, short, StringPtr);
void WrapText (StringPtr, short);
void GetFirstWordOfString (StringPtr, StringPtr);
void CollapseStringToWidth (StringPtr, short);
void GetChooserName (StringPtr);
StringPtr GetLocalizedString (short, StringPtr);

Point MyGetGlobalMouse (void);							// --- Utilities.c
void ToolBoxInit (void);
void FindOurDevice (void);
short RandomInt (short);
long RandomLong (long);
void InitRandomLongQUS (void);
UInt32 RandomLongQUS (void);
//void CenterAlert (short);
void RedAlert (short);
//void CreateOffScreenBitMap (Rect *, GrafPtr *);
//void CreateOffScreenPixMap (Rect *, CGrafPtr *);
//void KillOffScreenPixMap (CGrafPtr);
//void KillOffScreenBitMap (GrafPtr);
void LoadGraphic (short);
void LoadScaledGraphic (short, Rect *);
//void PlotSICN (Rect *, SICNHand, long);
void LargeIconPlot (Rect *, short);
void DrawCIcon (short, short, short);
char KeyMapOffsetFromRawKey (char);
long LongSquareRoot (long);
//void HideMenuBarOld (void);
//void ShowMenuBarOld (void);
Boolean WaitForInputEvent (short);
void WaitCommandQReleased (void);
void GetKeyName (intptr_t, StringPtr);
Boolean OptionKeyDown (void);
long ExtractCTSeed (CGrafPtr);
//void ForceCTSeed (CGrafPtr, long);
void DelayTicks (long);
void UnivGetSoundVolume (short *, Boolean);
void  UnivSetSoundVolume (short, Boolean);

Boolean ValidInstallation (Boolean);					// --- Validate.c

void GetWindowLeftTop (WindowPtr, short *, short *);	// --- WindowUtils.c
void GetWindowRect (WindowPtr, Rect *);
void GetLocalWindowRect (WindowPtr, Rect *);
//void FlagWindowFloating (WindowPtr);
//Boolean	IsWindowFloating (WindowPtr);
void OpenMessageWindow (const PLPasStr&);
void SetMessageWindowMessage (StringPtr);
void CloseMessageWindow (void);
void CloseThisWindow (WindowPtr *);

#ifdef powerc
//	extern pascal void SetSoundVol(short level);		// for old Sound Manager
//	extern pascal void GetSoundVol(short *level)
//	THREEWORDINLINE(0x4218, 0x10B8, 0x0260);
#endif

#include "GliderDefines.h"
#include "GliderStructs.h"
#include "GliderVars.h"
#include "GliderProtos.h"
