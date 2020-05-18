
//============================================================================
//----------------------------------------------------------------------------
//								  HighScores.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLHacks.h"
#include "PLKeyEncoding.h"
#include "PLNumberFormatting.h"
#include "PLScript.h"
#include "PLSound.h"
#include "PLStringCompare.h"
#include "DialogUtils.h"
#include "Externs.h"
#include "Environ.h"
#include "FileManager.h"
#include "FontFamily.h"
#include "FontManager.h"
#include "HostSystemServices.h"
#include "House.h"
#include "IOStream.h"
#include "MainWindow.h"
#include "RectUtils.h"
#include "PLStandardColors.h"
#include "PLTimeTaggedVOSEvent.h"
#include "Utilities.h"
#include "WindowManager.h"

namespace PortabilityLayer
{
	class IOStream;
}

#define kHighScoresPictID		1994
#define kHighScoresMaskID		1998
#define kHighNameDialogID		1020
#define kHighBannerDialogID		1021
#define kHighNameItem			2
#define kNameNCharsItem			5
#define kHighBannerItem			2
#define kBannerScoreNCharsItem	5


void DrawHighScores (DrawSurface *);
void UpdateNameDialog (Dialog *);
int16_t NameFilter (Dialog *dial, const TimeTaggedVOSEvent *evt);
void GetHighScoreName (short);
void UpdateBannerDialog (Dialog *);
int16_t BannerFilter(Dialog *dialog, const TimeTaggedVOSEvent *evt);
void GetHighScoreBanner (void);
Boolean OpenHighScoresFile (const VFileSpec &spec, PortabilityLayer::IOStream *&outStream);


Str31		highBanner;
Str15		highName;
short		lastHighScore;
Boolean		keyStroke;

extern	short		splashOriginH, splashOriginV;
extern	Boolean		quickerTransitions, resumedSavedGame;


//==============================================================  Functions
//--------------------------------------------------------------  DoHighScores
// Handles fading in and cleaning up the high scores screen.

void DoHighScores (void)
{
	Rect		tempRect;
	
	SpinCursor(3);
	SetPort(&workSrcMap->m_port);
	workSrcMap->SetForeColor(StdColors::Black());
	workSrcMap->FillRect(workSrcRect);
	QSetRect(&tempRect, 0, 0, 640, 480);
	QOffsetRect(&tempRect, splashOriginH, splashOriginV);
	LoadScaledGraphic(workSrcMap, kStarPictID, &tempRect);
//	if (quickerTransitions)
//		DissBitsChunky(&workSrcRect);
//	else
//		DissBits(&workSrcRect);

	DumpScreenOn(&workSrcRect);

	SpinCursor(3);

	DrawHighScores(workSrcMap);

	DumpScreenOn(&workSrcRect);

	SpinCursor(3);
//	if (quickerTransitions)
//		DissBitsChunky(&workSrcRect);
//	else
//		DissBits(&workSrcRect);

	InitCursor();
	DelayTicks(60);
	WaitForInputEvent(30);
	
	RedrawSplashScreen();
}

//--------------------------------------------------------------  DrawHighScores
// Draws the actual scores on the screen.

#define kScoreSpacing			18
#define kScoreWide				352
#define kKimsLifted				4

void DrawHighScores (DrawSurface *surface)
{
	DrawSurface	*tempMap, *tempMask;
	DrawSurface	*wasCPort;
	PLError_t		theErr;
	houseType	*thisHousePtr;
	Rect		tempRect, tempRect2;
	Str255		tempStr;
	short		scoreLeft, bannerWidth, i, dropIt;
	char		wasState;
	PortabilityLayer::RGBAColor blackColor = PortabilityLayer::RGBAColor::Create(0, 0, 0, 255);
	PortabilityLayer::RGBAColor yellowColor = PortabilityLayer::RGBAColor::Create(255, 255, 0, 255);
	PortabilityLayer::RGBAColor cyanColor = PortabilityLayer::RGBAColor::Create(0, 255, 255, 255);
	PortabilityLayer::RGBAColor whiteColor = PortabilityLayer::RGBAColor::Create(255, 255, 255, 255);
	PortabilityLayer::RGBAColor blueColor = PortabilityLayer::RGBAColor::Create(0, 0, 255, 255);
	
	scoreLeft = ((thisMac.constrainedScreen.right - thisMac.constrainedScreen.left) - kScoreWide) / 2;
	dropIt = 129 + splashOriginV;
	
	QSetRect(&tempRect, 0, 0, 332, 30);
	theErr = CreateOffScreenGWorld(&tempMap, &tempRect, kPreferredPixelFormat);
	LoadGraphic(tempMap, kHighScoresPictID);
	
	theErr = CreateOffScreenGWorld(&tempMask, &tempRect, GpPixelFormats::kBW1);	
	LoadGraphic(tempMask, kHighScoresMaskID);
	
	tempRect2 = tempRect;
	QOffsetRect(&tempRect2, scoreLeft + (kScoreWide - 332) / 2, dropIt - 60);
	
	CopyMask((BitMap *)*GetGWorldPixMap(tempMap), 
			(BitMap *)*GetGWorldPixMap(tempMask), 
			(BitMap *)*GetGWorldPixMap(workSrcMap), 
			&tempRect, &tempRect, &tempRect2);
	
	DisposeGWorld(tempMap);
	DisposeGWorld(tempMask);

	surface->SetApplicationFont(14, PortabilityLayer::FontFamilyFlag_Bold);
	
	PasStringCopy(PSTR("• "), tempStr);
	PasStringConcat(tempStr, thisHouseName);
	PasStringConcat(tempStr, PSTR(" •"));

	const Point scoreShadowPoint = Point::Create(scoreLeft + ((kScoreWide - surface->MeasureString(tempStr)) / 2) - 1, dropIt - 66);
	surface->SetForeColor(blackColor);
	surface->DrawString(scoreShadowPoint, tempStr, true);

	const Point scoreTextPoint = Point::Create(scoreLeft + ((kScoreWide - surface->MeasureString(tempStr)) / 2), dropIt - 65);
	surface->SetForeColor(cyanColor);
	surface->DrawString(scoreTextPoint, tempStr, true);

	surface->SetApplicationFont(12, PortabilityLayer::FontFamilyFlag_Bold);

	thisHousePtr = *thisHouse;
													// message for score #1
	PasStringCopy(thisHousePtr->highScores.banner, tempStr);
	bannerWidth = surface->MeasureString(tempStr);
	surface->SetForeColor(blackColor);
	const Point topScoreShadowPoint = Point::Create(scoreLeft + (kScoreWide - bannerWidth) / 2, dropIt - kKimsLifted);
	surface->DrawString(topScoreShadowPoint, tempStr, true);

	surface->SetForeColor(yellowColor);
	const Point topScoreTextPoint = Point::Create(scoreLeft + (kScoreWide - bannerWidth) / 2, dropIt - kKimsLifted - 1);
	surface->DrawString(topScoreTextPoint, tempStr, true);
	
	QSetRect(&tempRect, 0, 0, bannerWidth + 8, kScoreSpacing);
	QOffsetRect(&tempRect, scoreLeft - 3 + (kScoreWide - bannerWidth) / 2, 
			dropIt + 5 - kScoreSpacing - kKimsLifted);

	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(0, 0, 0, 255));
	surface->FrameRect(tempRect);
	QOffsetRect(&tempRect, -1, -1);
	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(255, 255, 0, 255));
	surface->FrameRect(tempRect);
	
	for (i = 0; i < kMaxScores; i++)
	{
		if (thisHousePtr->highScores.scores[i] > 0L)
		{
			Point strPos = Point::Create(0, 0);

			SpinCursor(1);
			NumToString((long)i + 1L, tempStr);		// draw placing number
			surface->SetForeColor(blackColor);
			if (i == 0)
				strPos = Point::Create(scoreLeft + 1, dropIt - kScoreSpacing - kKimsLifted);
			else
				strPos = Point::Create(scoreLeft + 1, dropIt + (i * kScoreSpacing));
			surface->DrawString(strPos, tempStr, true);
			if (i == lastHighScore)
				surface->SetForeColor(whiteColor);
			else
				surface->SetForeColor(cyanColor);
			if (i == 0)
				strPos = Point::Create(scoreLeft + 0, dropIt - 1 - kScoreSpacing - kKimsLifted);
			else
				strPos = Point::Create(scoreLeft + 0, dropIt - 1 + (i * kScoreSpacing));
			surface->DrawString(strPos, tempStr, true);
													// draw high score name
			PasStringCopy(thisHousePtr->highScores.names[i], tempStr);
			surface->SetForeColor(blackColor);
			if (i == 0)
				strPos = Point::Create(scoreLeft + 31, dropIt - kScoreSpacing - kKimsLifted);
			else
				strPos = Point::Create(scoreLeft + 31, dropIt + (i * kScoreSpacing));
			surface->DrawString(strPos, tempStr, true);
			if (i == lastHighScore)
				surface->SetForeColor(whiteColor);
			else
				surface->SetForeColor(yellowColor);
			if (i == 0)
				strPos = Point::Create(scoreLeft + 30, dropIt - 1 - kScoreSpacing - kKimsLifted);
			else
				strPos = Point::Create(scoreLeft + 30, dropIt - 1 + (i * kScoreSpacing));
			surface->DrawString(strPos, tempStr, true);
													// draw level number
			NumToString(thisHousePtr->highScores.levels[i], tempStr);
			surface->SetForeColor(blackColor);
			if (i == 0)
				strPos = Point::Create(scoreLeft + 161, dropIt - kScoreSpacing - kKimsLifted);
			else
				strPos = Point::Create(scoreLeft + 161, dropIt + (i * kScoreSpacing));
			surface->DrawString(strPos, tempStr, true);
			if (i == lastHighScore)
				surface->SetForeColor(whiteColor);
			else
				surface->SetForeColor(yellowColor);
			if (i == 0)
				strPos = Point::Create(scoreLeft + 160, dropIt - 1 - kScoreSpacing - kKimsLifted);
			else
				strPos = Point::Create(scoreLeft + 160, dropIt - 1 + (i * kScoreSpacing));
			surface->DrawString(strPos, tempStr, true);
													// draw word "rooms"
			if (thisHousePtr->highScores.levels[i] == 1)
				GetLocalizedString(6, tempStr);
			else
				GetLocalizedString(7, tempStr);
			surface->SetForeColor(blackColor);
			if (i == 0)
				strPos = Point::Create(scoreLeft + 193, dropIt - kScoreSpacing - kKimsLifted);
			else
				strPos = Point::Create(scoreLeft + 193, dropIt + (i * kScoreSpacing));
			surface->DrawString(strPos, tempStr, true);
			surface->SetForeColor(cyanColor);
			if (i == 0)
				strPos = Point::Create(scoreLeft + 192, dropIt - 1 - kScoreSpacing - kKimsLifted);
			else
				strPos = Point::Create(scoreLeft + 192, dropIt - 1 + (i * kScoreSpacing));
			surface->DrawString(strPos, tempStr, true);
													// draw high score points
			NumToString(thisHousePtr->highScores.scores[i], tempStr);
			surface->SetForeColor(blackColor);
			if (i == 0)
				strPos = Point::Create(scoreLeft + 291, dropIt - kScoreSpacing - kKimsLifted);
			else
				strPos = Point::Create(scoreLeft + 291, dropIt + (i * kScoreSpacing));
			surface->DrawString(strPos, tempStr, true);
			if (i == lastHighScore)
				surface->SetForeColor(whiteColor);
			else
				surface->SetForeColor(yellowColor);
			if (i == 0)
				strPos = Point::Create(scoreLeft + 290, dropIt - 1 - kScoreSpacing - kKimsLifted);
			else
				strPos = Point::Create(scoreLeft + 290, dropIt - 1 + (i * kScoreSpacing));
			surface->DrawString(strPos, tempStr, true);
		}
	}

	surface->SetForeColor(blueColor);

	surface->SetApplicationFont(9, PortabilityLayer::FontFamilyFlag_Bold);

	const Point textPos = Point::Create(scoreLeft + 80, dropIt - 1 + (10 * kScoreSpacing));
	GetLocalizedString(8, tempStr);
	surface->DrawString(textPos, tempStr, true);
	
	surface->SetForeColor(blackColor);
}

//--------------------------------------------------------------  SortHighScores
// This does a simple sort of the high scores.

void SortHighScores (void)
{
	scoresType	tempScores;
	houseType	*thisHousePtr;
	long		greatest;
	short		i, h, which;
	char		wasState;
	
	thisHousePtr = *thisHouse;
	
	for (h = 0; h < kMaxScores; h++)
	{
		greatest = -1L;
		which = -1;
		for (i = 0; i < kMaxScores; i++)
		{
			if (thisHousePtr->highScores.scores[i] > greatest)
			{
				greatest = thisHousePtr->highScores.scores[i];
				which = i;
			}
		}
		if (which != -1)
		{
			PasStringCopy(thisHousePtr->highScores.names[which], tempScores.names[h]);
			tempScores.scores[h] = thisHousePtr->highScores.scores[which];
			tempScores.timeStamps[h] = thisHousePtr->highScores.timeStamps[which];
			tempScores.levels[h] = thisHousePtr->highScores.levels[which];
			thisHousePtr->highScores.scores[which] = -1L;
		}
	}
	PasStringCopy(thisHousePtr->highScores.banner, tempScores.banner);
	thisHousePtr->highScores = tempScores;
}

//--------------------------------------------------------------  ZeroHighScores
// This funciton goes through and resets or "zeros" all high scores.

void ZeroHighScores (void)
{
	houseType	*thisHousePtr;
	short		i;
	char		wasState;
	
	thisHousePtr = *thisHouse;
	
	PasStringCopy(thisHouseName, thisHousePtr->highScores.banner);
	for (i = 0; i < kMaxScores; i++)
	{
		PasStringCopy(PSTR("--------------"), thisHousePtr->highScores.names[i]);
		thisHousePtr->highScores.scores[i] = 0L;
		thisHousePtr->highScores.timeStamps[i] = 0L;
		thisHousePtr->highScores.levels[i] = 0;
	}
}

//--------------------------------------------------------------  ZeroAllButHighestScore
// Like the above, but this function preserves the highest score.

void ZeroAllButHighestScore (void)
{
	houseType	*thisHousePtr;
	short		i;
	char		wasState;
	
	thisHousePtr = *thisHouse;
	
	for (i = 1; i < kMaxScores; i++)
	{
		PasStringCopy(PSTR("--------------"), thisHousePtr->highScores.names[i]);
		thisHousePtr->highScores.scores[i] = 0L;
		thisHousePtr->highScores.timeStamps[i] = 0L;
		thisHousePtr->highScores.levels[i] = 0;
	}
}

//--------------------------------------------------------------  TestHighScore
// This function is called after a game ends in order to test the…
// current high score against the high score list.  It returns true…
// if the player is on the high score list now.

Boolean TestHighScore (void)
{
	houseType	*thisHousePtr;
	short		placing, i;
	char		wasState;
	
	if (resumedSavedGame)
		return (false);

	if (IsHighScoreDisabled())
		return false;
	
	thisHousePtr = *thisHouse;
	
	lastHighScore = -1;
	placing = -1;
	
	for (i = 0; i < kMaxScores; i++)
	{
		if (theScore > thisHousePtr->highScores.scores[i])
		{
			placing = i;
			lastHighScore = i;
			break;
		}
	}

	if (IsHighScoreForceTop())
		placing = 0;
	
	if (placing != -1)
	{
		int64_t scoreTimestamp = PortabilityLayer::HostSystemServices::GetInstance()->GetTime();
		if (scoreTimestamp < 0)
			scoreTimestamp = 0;
		else if (scoreTimestamp > 0xffffffff)
			scoreTimestamp = 0xffffffff;

		FlushEvents(everyEvent, 0);
		GetHighScoreName(placing + 1);
		PasStringCopy(highName, thisHousePtr->highScores.names[kMaxScores - 1]);
		if (placing == 0)
		{
			GetHighScoreBanner();
			PasStringCopy(highBanner, thisHousePtr->highScores.banner);
		}
		thisHousePtr->highScores.scores[kMaxScores - 1] = theScore;
		thisHousePtr->highScores.timeStamps[kMaxScores - 1] = static_cast<uint32_t>(scoreTimestamp);
		thisHousePtr->highScores.levels[kMaxScores - 1] = CountRoomsVisited();
		SortHighScores();
		gameDirty = true;
	}
	
	if (placing != -1)
	{
		DoHighScores();
		return (true);
	}
	else
		return (false);
}

//--------------------------------------------------------------  UpdateNameDialog
// Redraws the "Enter High Score Name" dialog.

void UpdateNameDialog (Dialog *theDialog)
{
	short		nChars;
	
	DrawDefaultButton(theDialog);
	
	nChars = GetDialogStringLen(theDialog, kHighNameItem);
	SetDialogNumToStr(theDialog, kNameNCharsItem, (long)nChars);
}

//--------------------------------------------------------------  NameFilter
// Dialog filter for the "Enter High Score Name" dialog.

int16_t NameFilter (Dialog *dial, const TimeTaggedVOSEvent *evt)
{
	short		nChars;

	if (keyStroke)
	{
		nChars = GetDialogStringLen(dial, kHighNameItem);
		SetDialogNumToStr(dial, kNameNCharsItem, (long)nChars);
		keyStroke = false;
	}

	if (!evt)
		return -1;

	if (evt->m_vosEvent.m_eventType == GpVOSEventTypes::kKeyboardInput)
	{
		const GpKeyboardInputEvent &kbEvent = evt->m_vosEvent.m_event.m_keyboardInputEvent;

		if (kbEvent.m_eventType == GpKeyboardInputEventTypes::kDownChar || kbEvent.m_eventType == GpKeyboardInputEventTypes::kAutoChar)
		{
			PlayPrioritySound(kTypingSound, kTypingPriority);
			keyStroke = true;
			return -1;	// Don't capture, need this to forward to the editbox
		}
		else if (kbEvent.m_eventType == GpKeyboardInputEventTypes::kDown)
		{
			const intptr_t keyCode = PackVOSKeyCode(kbEvent);

			switch (keyCode)
			{
			case PL_KEY_SPECIAL(kEnter):
			case PL_KEY_NUMPAD_SPECIAL(kEnter):
				PlayPrioritySound(kCarriageSound, kCarriagePriority);
				FlashDialogButton(dial, kOkayButton);
				return kOkayButton;

			case PL_KEY_SPECIAL(kTab):
				SelectDialogItemText(dial, kHighNameItem, 0, 1024);
				return -1;
			}
		}
	}

	return -1;
}

//--------------------------------------------------------------  GetHighScoreName
// Brings up a dialog to get player's name (due to a high score).

void GetHighScoreName (short place)
{
	Dialog			*theDial;
	Str255			scoreStr, placeStr, tempStr;
	short			item;
	Boolean			leaving;

	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();
	
	InitCursor();
	NumToString(theScore, scoreStr);
	NumToString((long)place, placeStr);

	DialogTextSubstitutions substitutions(scoreStr, placeStr, thisHouseName);

	PlayPrioritySound(kEnergizeSound, kEnergizePriority);
	BringUpDialog(&theDial, kHighNameDialogID, &substitutions);
	FlushEvents(everyEvent, 0);
	SetDialogString(theDial, kHighNameItem, highName);
	SelectDialogItemText(theDial, kHighNameItem, 0, 1024);
	leaving = false;

	UpdateNameDialog(theDial);

	Window *exclStack = theDial->GetWindow();
	wm->SwapExclusiveWindow(exclStack);	// Push exclusive window for zooms

	if (doZooms)
		wm->FlickerWindowIn(theDial->GetWindow(), 64);

	while (!leaving)
	{
		item = theDial->ExecuteModal(NameFilter);
		
		if (item == kOkayButton)
		{
			GetDialogString(theDial, kHighNameItem, tempStr);
			PasStringCopyNum(tempStr, highName, 15);
			leaving = true;
		}
	}

	if (doZooms)
		wm->FlickerWindowOut(theDial->GetWindow(), 64);

	wm->SwapExclusiveWindow(exclStack);	// Pop exclusive window
	
	theDial->Destroy();
}

//--------------------------------------------------------------  UpdateBannerDialog
// Redraws the "Enter Message" dialog.

void UpdateBannerDialog (Dialog *theDialog)
{
	short		nChars;
	
	DrawDefaultButton(theDialog);
	
	nChars = GetDialogStringLen(theDialog, kHighBannerItem);
	SetDialogNumToStr(theDialog, kBannerScoreNCharsItem, (long)nChars);
}

//--------------------------------------------------------------  BannerFilter
// Dialog filter for the "Enter Message" dialog.

int16_t BannerFilter(Dialog *dial, const TimeTaggedVOSEvent *evt)
{
	short		nChars;
	
	if (keyStroke)
	{
		nChars = GetDialogStringLen(dial, kHighBannerItem);
		SetDialogNumToStr(dial, kBannerScoreNCharsItem, (long)nChars);
		keyStroke = false;
	}

	if (!evt)
		return -1;

	if (evt->m_vosEvent.m_eventType == GpVOSEventTypes::kKeyboardInput)
	{
		const GpKeyboardInputEvent &kbEvent = evt->m_vosEvent.m_event.m_keyboardInputEvent;

		if (kbEvent.m_eventType == GpKeyboardInputEventTypes::kDownChar || kbEvent.m_eventType == GpKeyboardInputEventTypes::kAutoChar)
		{
			PlayPrioritySound(kTypingSound, kTypingPriority);
			keyStroke = true;
			return -1;	// Don't capture, need this to forward to the editbox
		}
		else if (kbEvent.m_eventType == GpKeyboardInputEventTypes::kDown)
		{
			const intptr_t keyCode = PackVOSKeyCode(kbEvent);

			switch (keyCode)
			{
			case PL_KEY_SPECIAL(kEnter):
			case PL_KEY_NUMPAD_SPECIAL(kEnter):
				PlayPrioritySound(kCarriageSound, kCarriagePriority);
				FlashDialogButton(dial, kOkayButton);
				return kOkayButton;

			case PL_KEY_SPECIAL(kTab):
				SelectDialogItemText(dial, kHighBannerItem, 0, 1024);
				return -1;
			}
		}
	}

	return -1;
}

//--------------------------------------------------------------  GetHighScoreBanner
// A player who gets the #1 slot gets to enter a short message (that…
// appears across the top of the high scores list).  This dialog…
// gets that message.

void GetHighScoreBanner (void)
{
	Dialog			*theDial;
	Str255			tempStr;
	short			item;
	Boolean			leaving;

	PortabilityLayer::WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();
	
	PlayPrioritySound(kEnergizeSound, kEnergizePriority);
	BringUpDialog(&theDial, kHighBannerDialogID, nullptr);
	SetDialogString(theDial, kHighBannerItem, highBanner);
	SelectDialogItemText(theDial, kHighBannerItem, 0, 1024);
	leaving = false;

	UpdateBannerDialog(theDial);

	Window *exclStack = theDial->GetWindow();
	wm->SwapExclusiveWindow(exclStack);	// Push exclusive window for zooms

	if (doZooms)
		wm->FlickerWindowIn(theDial->GetWindow(), 64);

	while (!leaving)
	{
		item = theDial->ExecuteModal(BannerFilter);
		
		if (item == kOkayButton)
		{
			GetDialogString(theDial, kHighBannerItem, tempStr);
			PasStringCopyNum(tempStr, highBanner, 31);
			leaving = true;
		}
	}

	if (doZooms)
		wm->FlickerWindowOut(theDial->GetWindow(), 64);

	wm->SwapExclusiveWindow(exclStack);	// Pop exclusive window
	
	theDial->Destroy();
}

//--------------------------------------------------------------  OpenHighScoresFile

Boolean OpenHighScoresFile (const VFileSpec &scoreSpec, PortabilityLayer::IOStream *&scoresStream)
{
	PLError_t		theErr;

	PortabilityLayer::FileManager *fm = PortabilityLayer::FileManager::GetInstance();
	
	theErr = fm->OpenFileData(scoreSpec.m_dir, scoreSpec.m_name, PortabilityLayer::EFilePermission_Any, scoresStream);
	if (theErr == PLErrors::kFileNotFound)
	{
		theErr = fm->CreateFileAtCurrentTime(scoreSpec.m_dir, scoreSpec.m_name, 'ozm5', 'gliS');
		if (!CheckFileError(theErr, PSTR("New High Scores File")))
			return (false);
		theErr = fm->OpenFileData(scoreSpec.m_dir, scoreSpec.m_name, PortabilityLayer::EFilePermission_Any, scoresStream);
		if (!CheckFileError(theErr, PSTR("High Score")))
			return (false);
	}
	else if (!CheckFileError(theErr, PSTR("High Score")))
		return (false);
	
	return (true);
}

//--------------------------------------------------------------  WriteScoresToDisk

Boolean WriteScoresToDisk (void)
{
	scoresType	*theScores;
	VFileSpec	scoreSpec;
	long		byteCount;
	PLError_t		theErr;
	short		volRefNum;
	char		wasState;
	PortabilityLayer::IOStream	*scoresStream = nil;
	
	scoreSpec = MakeVFileSpec(PortabilityLayer::VirtualDirectories::kHighScores, thisHouseName);
	if (!OpenHighScoresFile(scoreSpec, scoresStream))
	{
		SysBeep(1);
		return (false);
	}

	if (!scoresStream->SeekStart(0))
	{
		CheckFileError(PLErrors::kIOError, PSTR("High Scores File"));
		scoresStream->Close();
		return(false);
	}
	
	byteCount = sizeof(scoresType);
	theScores = &((*thisHouse)->highScores);

	if (scoresStream->Write(theScores, byteCount) != byteCount)
	{
		CheckFileError(PLErrors::kIOError, PSTR("High Scores File"));
		scoresStream->Close();
		return(false);
	}

	if (!scoresStream->Truncate(byteCount))
	{
		CheckFileError(PLErrors::kIOError, PSTR("High Scores File"));
		scoresStream->Close();
		return(false);
	}

	scoresStream->Close();

	return (true);
}

//--------------------------------------------------------------  ReadScoresFromDisk

Boolean ReadScoresFromDisk (void)
{
	scoresType	*theScores;
	PortabilityLayer::UFilePos_t		byteCount;
	PLError_t		theErr;
	short		volRefNum;
	char		wasState;
	PortabilityLayer::IOStream *scoresStream = nil;
	
	VFileSpec	scoreSpec = MakeVFileSpec(PortabilityLayer::VirtualDirectories::kHighScores, thisHouseName);
	if (!OpenHighScoresFile(scoreSpec, scoresStream))
	{
		SysBeep(1);
		return (false);
	}
	
	byteCount = scoresStream->Size();
	
	theScores = &((*thisHouse)->highScores);

	if (scoresStream->Read(theScores, byteCount) != byteCount)
	{
		CheckFileError(PLErrors::kIOError, PSTR("High Scores File"));
		scoresStream->Close();
		return (false);
	}
	
	scoresStream->Close();

	return (true);
}

