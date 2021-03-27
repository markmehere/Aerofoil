
//============================================================================
//----------------------------------------------------------------------------
//								 ObjectDraw2.c
//----------------------------------------------------------------------------
//============================================================================

#include "BitmapImage.h"
#include "PLResources.h"
#include "PLPasStr.h"
#include "Externs.h"
#include "FontFamily.h"
#include "FontManager.h"
#include "Environ.h"
#include "Objects.h"
#include "RectUtils.h"
#include "RenderedFont.h"
#include "ResolveCachingColor.h"
#include "ResourceManager.h"
#include "Room.h"
#include "Utilities.h"
#include "PLHacks.h"

#define k8WhiteColor			0
#define kIntenseYellowColor		5
#define kPaleVioletColor		42
#define kDarkFleshColor			58
#define k8TanColor				94
#define k8PissYellowColor		95
#define k8BrownColor			137
#define k8SkyColor				150
#define k8EarthBlueColor		170
#define k8DkRed2Color			223
#define kIntenseGreenColor		225
#define kIntenseBlueColor		235
#define k8LtstGrayColor			245
#define k8LtstGray4Color		247
#define k8LtstGray5Color		248
#define k8LtGrayColor			249
#define k8Gray2Color			251
#define k8DkGrayColor			252
#define k8DkGray2Color			253

#define kBBQMaskID				3900
#define kUpStairsMaskID			3901
#define kTrunkMaskID			3902
#define kMailboxRightMaskID		3903
#define kMailboxLeftMaskID		3904
#define kDoorInLeftMaskID		3905
#define kDoorInRightMaskID		3906
#define kWindowInLeftMaskID		3907
#define kWindowInRightMaskID	3908
#define kHipLampMaskID			3909
#define kDecoLampMaskID			3910
#define kGuitarMaskID			3911
#define kTVMaskID				3912
#define kVCRMaskID				3913
#define kStereoMaskID			3914
#define kMicrowaveMaskID		3915
#define kFireplaceMaskID		3916
#define kBearMaskID				3917
#define kVase1MaskID			3918
#define kVase2MaskID			3919
#define kManholeMaskID			3920
#define kBooksMaskID			3922
#define kCloudMaskID			3921
#define kRugMaskID				3923
#define kChimesMaskID			3924
#define kCinderMaskID			3925
#define kFlowerBoxMaskID		3926
#define kCobwebMaskID			3927
#define kCobwebPictID			3958
#define kFlowerBoxPictID		3959
#define kCinderPictID			3960
#define kChimesPictID			3961
#define kRugPictID				3962
#define kBooksPictID			3964
#define kCloudPictID			3965
#define kBulletinPictID			3966
#define kManholePictID			3967
#define kVase2PictID			3968
#define kVase1PictID			3969
#define kCalendarPictID			3970
#define kMicrowavePictID		3971
#define kBearPictID				3972
#define kFireplacePictID		3973
#define kOzmaPictID				3975
#define kWindowExRightPictID	3977
#define kWindowExLeftPictID		3978
#define kWindowInRightPictID	3979
#define kWindowInLeftPictID		3980
#define kDoorExLeftPictID		3981
#define kDoorExRightPictID		3982
#define kDoorInRightPictID		3983
#define kDoorInLeftPictID		3984
#define kMailboxRightPictID		3985
#define kMailboxLeftPictID		3986
#define kTrunkPictID			3987
#define kBBQPictID				3988
#define kStereoPictID			3989
#define kVCRPictID				3990
#define kGuitarPictID			3991
#define kTVPictID				3992
#define kDecoLampPictID			3993
#define kHipLampPictID			3994
#define kFilingCabinetPictID	3995
#define kDownStairsPictID		3996
#define kUpStairsPictID			3997

#define	kMailboxBase			296
#define kMonthStringID			1005


//==============================================================  Functions
//--------------------------------------------------------------  DrawMailboxLeft

void DrawMailboxLeft (Rect *theRect, short down)
{
	Rect		bounds;
	DrawSurface	*tempMap;
	DrawSurface	*tempMask;
	long		darkGrayC, lightWoodC, darkWoodC;
	DrawSurface	*wasCPort;
	PLError_t		theErr;
	
	if (thisMac.isDepth == 4)
	{
		darkGrayC = 13;
		lightWoodC = 9;
		darkWoodC = 11;
	}
	else
	{
		darkGrayC = k8DkGray2Color;
		lightWoodC = k8PissYellowColor;
		darkWoodC = k8BrownColor;
	}
	
	if (theRect->bottom < down + kMailboxBase)
	{
		ColorLine(backSrcMap, theRect->left + 49, theRect->bottom,
				theRect->left + 49, down + kMailboxBase, darkGrayC);
		ColorLine(backSrcMap, theRect->left + 50, theRect->bottom,
				theRect->left + 50, down + kMailboxBase + 1, lightWoodC);
		ColorLine(backSrcMap, theRect->left + 51, theRect->bottom,
				theRect->left + 51, down + kMailboxBase + 2, lightWoodC);
		ColorLine(backSrcMap, theRect->left + 52, theRect->bottom,
				theRect->left + 52, down + kMailboxBase + 3, lightWoodC);
		ColorLine(backSrcMap, theRect->left + 53, theRect->bottom,
				theRect->left + 53, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 54, theRect->bottom,
				theRect->left + 54, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 55, theRect->bottom,
				theRect->left + 55, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 56, theRect->bottom,
				theRect->left + 56, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 57, theRect->bottom,
				theRect->left + 57, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 58, theRect->bottom,
				theRect->left + 58, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 59, theRect->bottom,
				theRect->left + 59, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 60, theRect->bottom,
				theRect->left + 60, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 61, theRect->bottom,
				theRect->left + 61, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 62, theRect->bottom,
				theRect->left + 62, down + kMailboxBase + 3, darkGrayC);
	}
	
	bounds = srcRects[kMailboxLf];
	theErr = CreateOffScreenGWorld(&tempMap, &bounds);
	LoadGraphic(tempMap, kMailboxLeftPictID);
	
	theErr = CreateOffScreenGWorldCustomDepth(&tempMask, &bounds, GpPixelFormats::kBW1);
	LoadGraphic(tempMask, kMailboxLeftMaskID);
	
	CopyMask((BitMap *)*GetGWorldPixMap(tempMap), 
			(BitMap *)*GetGWorldPixMap(tempMask), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&srcRects[kMailboxLf], &srcRects[kMailboxLf], theRect);
	
	DisposeGWorld(tempMap);
	DisposeGWorld(tempMask);
//	SetPort((GrafPtr)backSrcMap);
}

//--------------------------------------------------------------  DrawMailboxRight

void DrawMailboxRight (Rect *theRect, short down)
{
	Rect		bounds;
	DrawSurface	*tempMap;
	DrawSurface	*tempMask;
	long		darkGrayC, lightWoodC, darkWoodC;
	PLError_t		theErr;
	
	if (thisMac.isDepth == 4)
	{
		darkGrayC = 13;
		lightWoodC = 9;
		darkWoodC = 11;
	}
	else
	{
		darkGrayC = k8DkGray2Color;
		lightWoodC = k8PissYellowColor;
		darkWoodC = k8BrownColor;
	}
	
	if (theRect->bottom < down + kMailboxBase)
	{
		ColorLine(backSrcMap, theRect->left + 34, theRect->bottom,
				theRect->left + 34, down + kMailboxBase, darkGrayC);
		ColorLine(backSrcMap, theRect->left + 35, theRect->bottom,
				theRect->left + 35, down + kMailboxBase + 1, lightWoodC);
		ColorLine(backSrcMap, theRect->left + 36, theRect->bottom,
				theRect->left + 36, down + kMailboxBase + 2, lightWoodC);
		ColorLine(backSrcMap, theRect->left + 37, theRect->bottom,
				theRect->left + 37, down + kMailboxBase + 3, lightWoodC);
		ColorLine(backSrcMap, theRect->left + 38, theRect->bottom,
				theRect->left + 38, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 39, theRect->bottom,
				theRect->left + 39, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 40, theRect->bottom,
				theRect->left + 40, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 41, theRect->bottom,
				theRect->left + 41, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 42, theRect->bottom,
				theRect->left + 42, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 43, theRect->bottom,
				theRect->left + 43, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 44, theRect->bottom,
				theRect->left + 44, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 45, theRect->bottom,
				theRect->left + 45, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 46, theRect->bottom,
				theRect->left + 46, down + kMailboxBase + 3, darkWoodC);
		ColorLine(backSrcMap, theRect->left + 47, theRect->bottom,
				theRect->left + 47, down + kMailboxBase + 3, darkGrayC);
	}
	
	bounds = srcRects[kMailboxRt];
	theErr = CreateOffScreenGWorld(&tempMap, &bounds);
	LoadGraphic(tempMap, kMailboxRightPictID);
	
	theErr = CreateOffScreenGWorldCustomDepth(&tempMask, &bounds, GpPixelFormats::kBW1);
	LoadGraphic(tempMask, kMailboxRightMaskID);
	
	CopyMask((BitMap *)*GetGWorldPixMap(tempMap), 
			(BitMap *)*GetGWorldPixMap(tempMask), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&srcRects[kMailboxRt], &srcRects[kMailboxRt], theRect);
	
	DisposeGWorld(tempMap);
	DisposeGWorld(tempMask);
//	SetPort((GrafPtr)backSrcMap);
}

//--------------------------------------------------------------  DrawSimpleTransport

void DrawSimpleTransport (short what, Rect *theRect)
{
	CopyMask((BitMap *)*GetGWorldPixMap(transSrcMap), 
			(BitMap *)*GetGWorldPixMap(transMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&srcRects[what], &srcRects[what], theRect);
}

//--------------------------------------------------------------  DrawInvisTransport

void DrawInvisTransport (Rect *theRect)
{
	ColorFrameRect(backSrcMap, *theRect, 32);
}

//--------------------------------------------------------------  DrawLightSwitch

void DrawLightSwitch (Rect *theRect, Boolean state)
{
	if (state)
	{
		CopyBits((BitMap *)*GetGWorldPixMap(switchSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&lightSwitchSrc[0], theRect, srcCopy);
	}
	else
	{
		CopyBits((BitMap *)*GetGWorldPixMap(switchSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&lightSwitchSrc[1], theRect, srcCopy);
	}
}

//--------------------------------------------------------------  DrawMachineSwitch

void DrawMachineSwitch (Rect *theRect, Boolean state)
{
	if (state)
		CopyBits((BitMap *)*GetGWorldPixMap(switchSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&machineSwitchSrc[0], theRect, srcCopy);
	else
		CopyBits((BitMap *)*GetGWorldPixMap(switchSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&machineSwitchSrc[1], theRect, srcCopy);
}

//--------------------------------------------------------------  DrawThermostat

void DrawThermostat (Rect *theRect, Boolean state)
{
	if (state)
		CopyBits((BitMap *)*GetGWorldPixMap(switchSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&thermostatSrc[0], theRect, srcCopy);
	else
		CopyBits((BitMap *)*GetGWorldPixMap(switchSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&thermostatSrc[1], theRect, srcCopy);
}

//--------------------------------------------------------------  DrawPowerSwitch

void DrawPowerSwitch (Rect *theRect, Boolean state)
{
	if (state)
		CopyBits((BitMap *)*GetGWorldPixMap(switchSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&powerSrc[0], theRect, srcCopy);
	else
		CopyBits((BitMap *)*GetGWorldPixMap(switchSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&powerSrc[1], theRect, srcCopy);
}

//--------------------------------------------------------------  DrawKnifeSwitch

void DrawKnifeSwitch (Rect *theRect, Boolean state)
{
	if (state)
		CopyBits((BitMap *)*GetGWorldPixMap(switchSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&knifeSwitchSrc[0], theRect, srcCopy);
	else
		CopyBits((BitMap *)*GetGWorldPixMap(switchSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&knifeSwitchSrc[1], theRect, srcCopy);
}

//--------------------------------------------------------------  DrawInvisibleSwitch

void DrawInvisibleSwitch (Rect *theRect)
{
	DrawSurface	*wasCPort;

	ColorFrameRect(backSrcMap, *theRect, kIntenseGreenColor);
}

//--------------------------------------------------------------  DrawTrigger

void DrawTrigger (Rect *theRect)
{
	ColorFrameRect(backSrcMap, *theRect, kIntenseBlueColor);
}

//--------------------------------------------------------------  DrawSoundTrigger

void DrawSoundTrigger (Rect *theRect)
{
	ColorFrameRect(backSrcMap, *theRect, kIntenseYellowColor);
}

//--------------------------------------------------------------  DrawSimpleLight

void DrawSimpleLight (short what, Rect *theRect)
{
	CopyMask((BitMap *)*GetGWorldPixMap(lightSrcMap), 
			(BitMap *)*GetGWorldPixMap(lightMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&srcRects[what], &srcRects[what], theRect);
}

//--------------------------------------------------------------  DrawFlourescent

void DrawFlourescent (Rect *theRect)
{
	Rect		partRect;
	long		grayC, gray2C, gray3C, gray4C, violetC;
		
	if (thisMac.isDepth == 4)
	{
		grayC = 7L;
		gray2C = 5L;
		gray3C = 4L;
		gray4C = 1L;
		violetC = 3L;
	}
	else
	{
		grayC = k8LtGrayColor;
		gray2C = k8LtstGray5Color;
		gray3C = k8LtstGray4Color;
		gray4C = k8LtstGrayColor;
		violetC = kPaleVioletColor;
	}
	
	ColorLine(backSrcMap, theRect->left + 16, theRect->top,
			theRect->right - 17, theRect->top, grayC);
	ColorLine(backSrcMap, theRect->left + 16, theRect->top + 1,
			theRect->right - 17, theRect->top + 1, gray2C);
	ColorLine(backSrcMap, theRect->left + 16, theRect->top + 2,
			theRect->right - 17, theRect->top + 2, gray2C);
	ColorLine(backSrcMap, theRect->left + 16, theRect->top + 3,
			theRect->right - 17, theRect->top + 3, gray3C);
	ColorLine(backSrcMap, theRect->left + 16, theRect->top + 4,
			theRect->right - 17, theRect->top + 4, gray4C);
	ColorLine(backSrcMap, theRect->left + 16, theRect->top + 5,
			theRect->right - 17, theRect->top + 5, violetC);
	ColorLine(backSrcMap, theRect->left + 16, theRect->top + 6,
			theRect->right - 17, theRect->top + 6, k8WhiteColor);
	ColorLine(backSrcMap, theRect->left + 16, theRect->top + 7,
			theRect->right - 17, theRect->top + 7, k8WhiteColor);
	ColorLine(backSrcMap, theRect->left + 16, theRect->top + 8,
			theRect->right - 17, theRect->top + 8, k8WhiteColor);
	ColorLine(backSrcMap, theRect->left + 16, theRect->top + 9,
			theRect->right - 17, theRect->top + 9, k8WhiteColor);
	ColorLine(backSrcMap, theRect->left + 16, theRect->top + 10,
			theRect->right - 17, theRect->top + 10, k8WhiteColor);
	ColorLine(backSrcMap, theRect->left + 16, theRect->top + 11,
			theRect->right - 17, theRect->top + 11, violetC);
	
	partRect = flourescentSrc1;
	ZeroRectCorner(&partRect);
	QOffsetRect(&partRect, theRect->left, theRect->top);
	
	CopyMask((BitMap *)*GetGWorldPixMap(lightSrcMap), 
			(BitMap *)*GetGWorldPixMap(lightMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&flourescentSrc1, &flourescentSrc1, &partRect);
	
	partRect = flourescentSrc2;
	ZeroRectCorner(&partRect);
	QOffsetRect(&partRect, -partRect.right, 0);
	QOffsetRect(&partRect, theRect->right, theRect->top);
	
	CopyMask((BitMap *)*GetGWorldPixMap(lightSrcMap), 
			(BitMap *)*GetGWorldPixMap(lightMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&flourescentSrc2, &flourescentSrc2, &partRect);
}

//--------------------------------------------------------------  DrawTrackLight

void DrawTrackLight (Rect *theRect)
{
	#define		kTrackLightSpacing	64
	Rect		partRect;
	long		grayC, gray2C, gray3C, gray4C;
	short		which, howMany, i, spread;
	
	if (thisMac.isDepth == 4)
	{
		grayC = 7L;
		gray2C = 8L;
		gray3C = 4L;
		gray4C = 11L;
	}
	else
	{
		grayC = k8LtGrayColor;
		gray2C = k8Gray2Color;
		gray3C = k8LtstGray4Color;
		gray4C = k8DkGrayColor;
	}
	
	ColorLine(backSrcMap, theRect->left, theRect->top - 3,
			theRect->right - 1, theRect->top - 3, gray2C);
	ColorLine(backSrcMap, theRect->left, theRect->top - 2,
			theRect->right - 1, theRect->top - 2, grayC);
	ColorLine(backSrcMap, theRect->left, theRect->top - 1,
			theRect->right - 1, theRect->top - 1, grayC);
	ColorLine(backSrcMap, theRect->left, theRect->top,
			theRect->right - 1, theRect->top, gray3C);
	ColorLine(backSrcMap, theRect->left, theRect->top + 1,
			theRect->right - 1, theRect->top + 1, gray4C);
	ColorLine(backSrcMap, theRect->left, theRect->top + 2,
			theRect->right - 1, theRect->top + 2, gray3C);
	
	partRect = trackLightSrc[0];			// left most track light
	ZeroRectCorner(&partRect);
	QOffsetRect(&partRect, theRect->left, theRect->top);
	which = 0;
	CopyMask((BitMap *)*GetGWorldPixMap(lightSrcMap), 
			(BitMap *)*GetGWorldPixMap(lightMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&trackLightSrc[which], &trackLightSrc[which], &partRect);
	
	partRect = trackLightSrc[0];			// right most track light
	ZeroRectCorner(&partRect);
	QOffsetRect(&partRect, -partRect.right, 0);
	QOffsetRect(&partRect, theRect->right, theRect->top);
	which = 2;
	CopyMask((BitMap *)*GetGWorldPixMap(lightSrcMap), 
			(BitMap *)*GetGWorldPixMap(lightMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&trackLightSrc[which], &trackLightSrc[which], &partRect);
	
	howMany = ((RectWide(theRect) - RectWide(&trackLightSrc[0])) / 
			kTrackLightSpacing) - 1;
	if (howMany > 0)
	{
		which = 0;
		spread = (RectWide(theRect) - RectWide(&trackLightSrc[0])) / (howMany + 1);
		for (i = 0; i < howMany; i++)
		{
			partRect = trackLightSrc[0];			// filler track lights
			ZeroRectCorner(&partRect);
			QOffsetRect(&partRect, theRect->left, theRect->top);
			QOffsetRect(&partRect, spread * (i + 1), 0);
			which++;
			if (which >= kNumTrackLights)
				which = 0;
			CopyMask((BitMap *)*GetGWorldPixMap(lightSrcMap), 
					(BitMap *)*GetGWorldPixMap(lightMaskMap), 
					(BitMap *)*GetGWorldPixMap(backSrcMap), 
					&trackLightSrc[which], &trackLightSrc[which], &partRect);
		}
	}
}

//--------------------------------------------------------------  DrawInvisLight

void DrawInvisLight (Rect *theRect)
{
	ColorFrameOval(backSrcMap, *theRect, 17);
}

//--------------------------------------------------------------  DrawSimpleAppliance

void DrawSimpleAppliance (short what, Rect *theRect)
{
	CopyMask((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
			(BitMap *)*GetGWorldPixMap(applianceMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&srcRects[what], &srcRects[what], theRect);
}

//--------------------------------------------------------------  DrawMacPlus

void DrawMacPlus (Rect *theRect, Boolean isOn, Boolean isLit)
{
	Rect		screen;

	if (isLit)
	{
		CopyMask((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(applianceMaskMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&srcRects[kMacPlus], &srcRects[kMacPlus], theRect);
	}
	
	screen = plusScreen1;
	ZeroRectCorner(&screen);
	QOffsetRect(&screen, theRect->left + 10, theRect->top + 7);
	if (isOn)
	{
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&plusScreen2, &screen, srcCopy);
	}
	else
	{
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&plusScreen1, &screen, srcCopy);
	}


	if (IsMacPlusGraphicBanned())
	{
		PortabilityLayer::ResolveCachingColor beigeColor = PortabilityLayer::RGBAColor::Create(255, 255, 204, 255);

		Rect paintOverRect = *theRect;
		paintOverRect.left += 8;
		paintOverRect.top += 35;
		paintOverRect.right = paintOverRect.left + 17;
		paintOverRect.bottom = paintOverRect.top + 6;

		backSrcMap->FillRect(paintOverRect, beigeColor);
	}
}

//--------------------------------------------------------------  DrawTV

void DrawTV (Rect *theRect, Boolean isOn, Boolean isLit)
{
	Rect		bounds;
	DrawSurface	*tempMap;
	DrawSurface	*tempMask;
	DrawSurface	*wasCPort;
	PLError_t		theErr;
		
	if (isLit)
	{
		bounds = srcRects[kTV];
		theErr = CreateOffScreenGWorld(&tempMap, &bounds);
		LoadGraphic(tempMap, kTVPictID);
		
		theErr = CreateOffScreenGWorldCustomDepth(&tempMask, &bounds, GpPixelFormats::kBW1);
		LoadGraphic(tempMask, kTVMaskID);
		
		CopyMask((BitMap *)*GetGWorldPixMap(tempMap), 
				(BitMap *)*GetGWorldPixMap(tempMask), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&srcRects[kTV], &srcRects[kTV], theRect);
		
		DisposeGWorld(tempMap);
		DisposeGWorld(tempMask);
//		SetPort((GrafPtr)backSrcMap);
	}
	
	bounds = tvScreen1;
	ZeroRectCorner(&bounds);
	QOffsetRect(&bounds, theRect->left + 17, theRect->top + 10);
	if (isOn)
	{
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&tvScreen2, &bounds, srcCopy);
	}
	else
	{
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&tvScreen1, &bounds, srcCopy);
	}
}

//--------------------------------------------------------------  DrawCoffee

void DrawCoffee (Rect *theRect, Boolean isOn, Boolean isLit)
{
	Rect		light;
	
	if (isLit)
	{
		CopyMask((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(applianceMaskMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&srcRects[kCoffee], &srcRects[kCoffee], theRect);
	}
	
	light = coffeeLight1;
	ZeroRectCorner(&light);
	QOffsetRect(&light, theRect->left + 32, theRect->top + 57);
	if (isOn)
	{
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&coffeeLight2, &light, srcCopy);
	}
	else
	{
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&coffeeLight1, &light, srcCopy);
	}
}

//--------------------------------------------------------------  DrawOutlet

void DrawOutlet (Rect *theRect)
{
	CopyMask((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
			(BitMap *)*GetGWorldPixMap(applianceMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&srcRects[kOutlet], &srcRects[kOutlet], theRect);
}

//--------------------------------------------------------------  DrawVCR

void DrawVCR (Rect *theRect, Boolean isOn, Boolean isLit)
{
	Rect		bounds;
	DrawSurface	*tempMap;
	DrawSurface	*tempMask;
	PLError_t		theErr;
	
	if (isLit)
	{
		bounds = srcRects[kVCR];
		theErr = CreateOffScreenGWorld(&tempMap, &bounds);
		LoadGraphic(tempMap, kVCRPictID);
		
		theErr = CreateOffScreenGWorldCustomDepth(&tempMask, &bounds, GpPixelFormats::kBW1);
		LoadGraphic(tempMask, kVCRMaskID);
		
		CopyMask((BitMap *)*GetGWorldPixMap(tempMap), 
				(BitMap *)*GetGWorldPixMap(tempMask), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&srcRects[kVCR], &srcRects[kVCR], theRect);
		
		DisposeGWorld(tempMap);
		DisposeGWorld(tempMask);
		
//		SetPort((GrafPtr)backSrcMap);
	}
	
	bounds = vcrTime1;
	ZeroRectCorner(&bounds);
	QOffsetRect(&bounds, theRect->left + 64, theRect->top + 6);
	if (isOn)
	{
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&vcrTime2, &bounds, srcCopy);
	}
	else
	{
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&vcrTime1, &bounds, srcCopy);
	}
}

//--------------------------------------------------------------  DrawStereo

void DrawStereo (Rect *theRect, Boolean isOn, Boolean isLit)
{
	Rect		bounds;
	DrawSurface	*tempMap;
	DrawSurface	*tempMask;
	PLError_t		theErr;
	
	
	if (isLit)
	{
		bounds = srcRects[kStereo];
		theErr = CreateOffScreenGWorld(&tempMap, &bounds);
		LoadGraphic(tempMap, kStereoPictID);
		
		theErr = CreateOffScreenGWorldCustomDepth(&tempMask, &bounds, GpPixelFormats::kBW1);
		LoadGraphic(tempMask, kStereoMaskID);
		
		CopyMask((BitMap *)*GetGWorldPixMap(tempMap), 
				(BitMap *)*GetGWorldPixMap(tempMask), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&srcRects[kStereo], &srcRects[kStereo], theRect);
		
		DisposeGWorld(tempMap);
		DisposeGWorld(tempMask);
		
//		SetPort((GrafPtr)backSrcMap);
	}
	
	bounds = stereoLight1;
	ZeroRectCorner(&bounds);
	QOffsetRect(&bounds, theRect->left + 56, theRect->top + 20);
	if (isOn)
	{
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&stereoLight2, &bounds, srcCopy);
	}
	else
	{
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&stereoLight1, &bounds, srcCopy);
	}
}

//--------------------------------------------------------------  DrawMicrowave

void DrawMicrowave (Rect *theRect, Boolean isOn, Boolean isLit)
{
	Rect		bounds;
	DrawSurface	*tempMap;
	DrawSurface	*tempMask;
	PLError_t		theErr;
	
	
	if (isLit)
	{
		bounds = srcRects[kMicrowave];
		theErr = CreateOffScreenGWorld(&tempMap, &bounds);
		LoadGraphic(tempMap, kMicrowavePictID);
		
		theErr = CreateOffScreenGWorldCustomDepth(&tempMask, &bounds, GpPixelFormats::kBW1);
		LoadGraphic(tempMask, kMicrowaveMaskID);
		
		CopyMask((BitMap *)*GetGWorldPixMap(tempMap), 
				(BitMap *)*GetGWorldPixMap(tempMask), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&srcRects[kMicrowave], &srcRects[kMicrowave], theRect);
		
		DisposeGWorld(tempMap);
		DisposeGWorld(tempMask);
		
//		SetPort((GrafPtr)backSrcMap);
	}
	
	bounds = microOn;
	ZeroRectCorner(&bounds);
	QOffsetRect(&bounds, theRect->left + 14, theRect->top + 13);
	if (isOn)
	{
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&microOn, &bounds, srcCopy);
		QOffsetRect(&bounds, 16, 0);
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&microOn, &bounds, srcCopy);
		QOffsetRect(&bounds, 16, 0);
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&microOn, &bounds, srcCopy);
	}
	else if (isLit)
	{
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&microOff, &bounds, srcCopy);
		QOffsetRect(&bounds, 16, 0);
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&microOff, &bounds, srcCopy);
		QOffsetRect(&bounds, 16, 0);
		CopyBits((BitMap *)*GetGWorldPixMap(applianceSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&microOff, &bounds, srcCopy);
	}
}

//--------------------------------------------------------------  DrawBalloon

void DrawBalloon (Rect *theRect)
{
	CopyMask((BitMap *)*GetGWorldPixMap(balloonSrcMap), 
			(BitMap *)*GetGWorldPixMap(balloonMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&balloonSrc[1], &balloonSrc[1], theRect);
}

//--------------------------------------------------------------  DrawCopter

void DrawCopter (Rect *theRect)
{
	CopyMask((BitMap *)*GetGWorldPixMap(copterSrcMap), 
			(BitMap *)*GetGWorldPixMap(copterMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&copterSrc[1], &copterSrc[1], theRect);
}

//--------------------------------------------------------------  DrawDart

void DrawDart (Rect *theRect, short which)
{
	if (which == kDartLf)
	{
		CopyMask((BitMap *)*GetGWorldPixMap(dartSrcMap), 
				(BitMap *)*GetGWorldPixMap(dartMaskMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&dartSrc[0], &dartSrc[0], theRect);
	}
	else
	{
		CopyMask((BitMap *)*GetGWorldPixMap(dartSrcMap), 
				(BitMap *)*GetGWorldPixMap(dartMaskMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&dartSrc[2], &dartSrc[2], theRect);
	}
}

//--------------------------------------------------------------  DrawBall

void DrawBall (short what, Rect *theRect)
{
	CopyMask((BitMap *)*GetGWorldPixMap(ballSrcMap), 
			(BitMap *)*GetGWorldPixMap(ballMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&srcRects[what], &srcRects[what], theRect);
}

//--------------------------------------------------------------  DrawFish

void DrawFish (short what, Rect *theRect)
{
	CopyMask((BitMap *)*GetGWorldPixMap(enemySrcMap), 
			(BitMap *)*GetGWorldPixMap(enemyMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&srcRects[what], &srcRects[what], theRect);
}

//--------------------------------------------------------------  DrawDrip

void DrawDrip (Rect *theRect)
{
	CopyMask((BitMap *)*GetGWorldPixMap(dripSrcMap), 
			(BitMap *)*GetGWorldPixMap(dripMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&dripSrc[3], &dripSrc[3], theRect);
}

//--------------------------------------------------------------  DrawMirror

void DrawMirror (Rect *mirror)
{
	Rect		tempRect;
	long		grayC;
	
	if (thisMac.isDepth == 4)
	{
		grayC = 13;
	}
	else
	{
		grayC = k8DkGray2Color;
	}
	
	tempRect = *mirror;
	ColorRect(backSrcMap, tempRect, k8WhiteColor);
	ColorFrameRect(backSrcMap, tempRect, grayC);
	InsetRect(&tempRect, 1, 1);
	ColorFrameRect(backSrcMap, tempRect, k8EarthBlueColor);
	InsetRect(&tempRect, 1, 1);
	ColorFrameRect(backSrcMap, tempRect, k8EarthBlueColor);
	InsetRect(&tempRect, 1, 1);
	ColorFrameRect(backSrcMap, tempRect, grayC);
}

//--------------------------------------------------------------  DrawSimpleClutter

void DrawSimpleClutter (short what, Rect *theRect)
{
	CopyMask((BitMap *)*GetGWorldPixMap(clutterSrcMap), 
			(BitMap *)*GetGWorldPixMap(clutterMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&srcRects[what], &srcRects[what], theRect);
}

//--------------------------------------------------------------  DrawFlower

void DrawFlower (Rect *theRect, short which)
{
	CopyMask((BitMap *)*GetGWorldPixMap(clutterSrcMap), 
			(BitMap *)*GetGWorldPixMap(clutterMaskMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&flowerSrc[which], &flowerSrc[which], theRect);
}

//--------------------------------------------------------------  DrawWallWindow

void DrawWallWindow (Rect *window)
{
	#define		kWindowSillThick	7
	Rect		tempRect, tempRect2;
	long		brownC, tanC, dkstRedC;
	short		halfWay;
		
	if (thisMac.isDepth == 4)
	{
		brownC = 11;
		tanC = 9;
		dkstRedC = 15;
	}
	else
	{
		brownC = k8BrownColor;
		tanC = k8TanColor;
		dkstRedC = k8DkRed2Color;
	}
	
	tempRect = *window;
	InsetRect(&tempRect, 3, 0);
	ColorRect(backSrcMap, tempRect, brownC);
	HiliteRect(backSrcMap, tempRect, tanC, dkstRedC);
	
	tempRect = *window;			// top sill
	tempRect.bottom = tempRect.top + kWindowSillThick;
	tempRect.left++;
	tempRect.right--;
	ColorRect(backSrcMap, tempRect, brownC);
	HiliteRect(backSrcMap, tempRect, tanC, dkstRedC);
	tempRect.left--;
	tempRect.right++;
	tempRect.top += 2;
	tempRect.bottom -= 2;
	ColorRect(backSrcMap, tempRect, brownC);
	HiliteRect(backSrcMap, tempRect, tanC, dkstRedC);
	
	tempRect = *window;			// bottom sill
	tempRect.top = tempRect.bottom - kWindowSillThick;
	QOffsetRect(&tempRect, 0, -4);
	tempRect.left++;
	tempRect.right--;
	ColorRect(backSrcMap, tempRect, brownC);
	HiliteRect(backSrcMap, tempRect, tanC, dkstRedC);
	tempRect.left--;
	tempRect.right++;
	tempRect.top += 2;
	tempRect.bottom -= 2;
	ColorRect(backSrcMap, tempRect, brownC);
	HiliteRect(backSrcMap, tempRect, tanC, dkstRedC);
	
	tempRect = *window;			// inside frame
	tempRect.left += 8;
	tempRect.right -= 8;
	tempRect.top += 11;
	tempRect.bottom -= 15;
	HiliteRect(backSrcMap, tempRect, dkstRedC, tanC);
	
	halfWay = (tempRect.top + tempRect.bottom) / 2;
	
	tempRect2 = tempRect;		// top pane
	tempRect2.bottom = halfWay + 2;
	InsetRect(&tempRect2, 5, 5);
	HiliteRect(backSrcMap, tempRect2, dkstRedC, tanC);
	InsetRect(&tempRect2, 1, 1);
	if (thisMac.isDepth == 4)
		ColorRect(backSrcMap, tempRect2, 5);
	else
		ColorRect(backSrcMap, tempRect2, k8SkyColor);
	
	tempRect2 = tempRect;		// bottom pane
	tempRect2.top = halfWay - 3;
	InsetRect(&tempRect2, 5, 5);
	HiliteRect(backSrcMap, tempRect2, dkstRedC, tanC);
	InsetRect(&tempRect2, 1, 1);
	if (thisMac.isDepth == 4)
		ColorRect(backSrcMap, tempRect2, 5);
	else
		ColorRect(backSrcMap, tempRect2, k8SkyColor);
	
	ColorLine(backSrcMap, tempRect2.left - 5, tempRect2.top - 7,
			tempRect2.right + 5, tempRect2.top - 7, tanC);
}

//--------------------------------------------------------------  DrawCalendar

void DrawCalendar (Rect *theRect)
{
	DateTimeRec				timeRec;
	Rect					bounds;
	THandle<BitmapImage>	thePicture;
	Str255					monthStr;
	
	thePicture = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('PICT', kCalendarPictID).StaticCast<BitmapImage>();
	if (thePicture == nil)
		RedAlert(kErrFailedGraphicLoad);
	
	bounds = (*thePicture)->GetRect();
	QOffsetRect(&bounds, -bounds.left, -bounds.top);
	QOffsetRect(&bounds, theRect->left, theRect->top);
	backSrcMap->DrawPicture(thePicture, bounds);
	thePicture.Dispose();

	PortabilityLayer::RenderedFont *appFont = GetFont(PortabilityLayer::FontPresets::kApplication9Bold);

	GetTime(&timeRec);
	GetIndString(monthStr, kMonthStringID, timeRec.month);

	const Point textPos = Point::Create(theRect->left + ((64 - appFont->MeasurePStr(monthStr)) / 2), theRect->top + 55);
	ColorText(backSrcMap, textPos, monthStr, kDarkFleshColor, appFont);
}

//--------------------------------------------------------------  DrawBulletin

void DrawBulletin (Rect *theRect)
{
	Rect					bounds;
	THandle<BitmapImage>	thePicture;
	
	thePicture = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('PICT', kBulletinPictID).StaticCast<BitmapImage>();
	if (thePicture == nil)
		RedAlert(kErrFailedGraphicLoad);
	
	bounds = (*thePicture)->GetRect();
	QOffsetRect(&bounds, -bounds.left, -bounds.top);
	QOffsetRect(&bounds, theRect->left, theRect->top);
	backSrcMap->DrawPicture(thePicture, bounds);
	thePicture.Dispose();
}

//--------------------------------------------------------------  DrawPictObject

void DrawPictObject (short what, Rect *theRect)
{
	Rect					bounds;
	THandle<BitmapImage>	thePicture;
	short					pictID;
		
	switch (what)
	{
		case kFilingCabinet:
		pictID = kFilingCabinetPictID;
		break;
		
		case kDownStairs:
		pictID = kDownStairsPictID;
		break;
		
		case kDoorExRt:
		pictID = kDoorExRightPictID;
		break;
		
		case kDoorExLf:
		pictID = kDoorExLeftPictID;
		break;
		
		case kWindowExRt:
		pictID = kWindowExRightPictID;
		break;
		
		case kWindowExLf:
		pictID = kWindowExLeftPictID;
		break;
		
		case kOzma:
		pictID = kOzmaPictID;
		break;
	}
	
	thePicture = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('PICT', pictID).StaticCast<BitmapImage>();
	if (thePicture == nil)
		RedAlert(kErrFailedGraphicLoad);
	
	bounds = srcRects[what];
	QOffsetRect(&bounds, theRect->left, theRect->top);
	backSrcMap->DrawPicture(thePicture, bounds);
	thePicture.Dispose();
}

//--------------------------------------------------------------  DrawPictWithMaskObject

void DrawPictWithMaskObject (short what, Rect *theRect)
{
	Rect		bounds;
	DrawSurface	*tempMap;
	DrawSurface	*tempMask;
	short		pictID, maskID;
	DrawSurface	*wasCPort;
	PLError_t		theErr;
	
	switch (what)
	{
		case kCobweb:
		pictID = kCobwebPictID;
		maskID = kCobwebMaskID;
		break;

		case kCloud:
		pictID = kCloudPictID;
		maskID = kCloudMaskID;
		break;
	}
	
	bounds = srcRects[what];
	theErr = CreateOffScreenGWorld(&tempMap, &bounds);
	LoadGraphic(tempMap, pictID);
	
	theErr = CreateOffScreenGWorldCustomDepth(&tempMask, &bounds, GpPixelFormats::kBW1);
	LoadGraphic(tempMask, maskID);
	
	CopyMask((BitMap *)*GetGWorldPixMap(tempMap), 
			(BitMap *)*GetGWorldPixMap(tempMask), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&srcRects[what], &srcRects[what], theRect);
	
	DisposeGWorld(tempMap);
	DisposeGWorld(tempMask);
	
//	SetPort((GrafPtr)backSrcMap);
}

//--------------------------------------------------------------  DrawPictSansWhiteObject

void DrawPictSansWhiteObject (short what, Rect *theRect)
{
	Rect		bounds;
	DrawSurface	*tempMap;
	short		pictID;
	DrawSurface	*wasCPort;
	PLError_t		theErr;
	
	switch (what)
	{
		case kBBQ:
		pictID = kBBQPictID;
		break;
		
		case kTrunk:
		pictID = kTrunkPictID;
		break;
		
		case kManhole:
		pictID = kManholePictID;
		break;
		
		case kBooks:
		pictID = kBooksPictID;
		break;
		
		case kUpStairs:
		pictID = kUpStairsPictID;
		break;
		
		case kDoorInLf:
		pictID = kDoorInLeftPictID;
		break;
		
		case kDoorInRt:
		pictID = kDoorInRightPictID;
		break;
		
		case kWindowInLf:
		pictID = kWindowInLeftPictID;
		break;
		
		case kWindowInRt:
		pictID = kWindowInRightPictID;
		break;
		
		case kHipLamp:
		pictID = kHipLampPictID;
		break;
		
		case kDecoLamp:
		pictID = kDecoLampPictID;
		break;
		
		case kGuitar:
		pictID = kGuitarPictID;
		break;
		
		case kCinderBlock:
		pictID = kCinderPictID;
		break;
		
		case kFlowerBox:
		pictID = kFlowerBoxPictID;
		break;
		
		case kFireplace:
		pictID = kFireplacePictID;
		break;
		
		case kBear:
		pictID = kBearPictID;
		break;
		
		case kVase1:
		pictID = kVase1PictID;
		break;
		
		case kVase2:
		pictID = kVase2PictID;
		break;
		
		case kRug:
		pictID = kRugPictID;
		break;
		
		case kChimes:
		pictID = kChimesPictID;
		break;
	}
	
	bounds = srcRects[what];
	theErr = CreateOffScreenGWorld(&tempMap, &bounds);
	LoadGraphic(tempMap, pictID);
	
	CopyBits((BitMap *)*GetGWorldPixMap(tempMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&srcRects[what], theRect, transparent);
	
	DisposeGWorld(tempMap);
//	SetPort((GrafPtr)backSrcMap);
}
//--------------------------------------------------------------  DrawCustPictSansWhite

void DrawCustPictSansWhite (short pictID, Rect *theRect)
{
	Rect		bounds;
	DrawSurface	*tempMap;
	PLError_t		theErr;
	
	bounds = *theRect;
	ZeroRectCorner(&bounds);
	theErr = CreateOffScreenGWorld(&tempMap, &bounds);
	LoadGraphicCustom(tempMap, pictID);
	
	CopyBits((BitMap *)*GetGWorldPixMap(tempMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&bounds, theRect, transparent);
	
	DisposeGWorld(tempMap);
//	SetPort((GrafPtr)backSrcMap);
}

