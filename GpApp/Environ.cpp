
//============================================================================
//----------------------------------------------------------------------------
//									Environ.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLResources.h"
#include "PLPasStr.h"
#include "DisplayDeviceManager.h"
#include "Externs.h"
#include "Environ.h"
#include "HostDisplayDriver.h"
#include "HostSystemServices.h"
#include "MenuManager.h"
#include "IGpDisplayDriver.h"
#include "WindowManager.h"

#define	kSwitchDepthAlert		130
#define kSetMemoryAlert			180
#define kLowMemoryAlert			181
#define kWNETrap				0x60
#define	kSetDepthTrap			0xA2
#define kUnimpTrap				0x9F
#define kGestaltTrap			0xAD

#define	kDisplay9Inch			1
#define	kDisplay12Inch			2
#define	kDisplay13Inch			3


typedef struct
{
	short	flags;
	long	mem1;
	long	mem2;
} sizeType;


//short GetThisVolumeRefNum (void);
//long GetThisCurrentDirectoryID (void);
//Boolean TrapExists (short);
//Boolean DoWeHaveGestalt (void);
//Boolean DoWeHaveWNE (void);
//Boolean DoWeHaveColor (void);
//Boolean DoWeHaveSystem602 (void);
//Boolean DoWeHaveSystem605 (void);
//Boolean DoWeHaveSystem7 (void);
//Boolean DoWeHaveSoundManager3 (void);
Boolean DoWeHaveQuickTime (void);
Boolean DoWeHaveDragManager (void);
//Boolean CanWeDisplay4Bit (GDHandle);
//Boolean CanWeDisplay1Bit (GDHandle);
short HowManyUsableScreens (Boolean, Boolean, Boolean);
void GetDeviceRect (Rect *);
Boolean AreWeColorOrGrayscale (void);
void SwitchDepthOrAbort (void);


macEnviron	thisMac;

extern	short		isDepthPref;
extern	Boolean		dontLoadMusic, dontLoadSounds;


//==============================================================  Functions
//--------------------------------------------------------------  GetThisVolumeRefNum
// Get a hard reference number for the current drive volume this app is on.
/*
short GetThisVolumeRefNum (void)
{
	PLError_t		theErr;
	short		vRef;
	
	theErr = GetVol(nil, &vRef);
	return (vRef);
}
*/
//--------------------------------------------------------------  GetThisCurrentDirectoryID
// Get a hard ID number for the current directory volume this app is in.
/*
long GetThisCurrentDirectoryID (void)
{
	long		dirID;
	
	dirID = LMGetCurDirStore();
	return (dirID);
}
*/
//--------------------------------------------------------------  TrapExists
// Returns whether or not a ToolBox trap exists for the users ROMs/System.
/*
Boolean TrapExists (short trapNumber)
{
	return ((NGetTrapAddress(trapNumber, ToolTrap) !=
		NGetTrapAddress(kUnimpTrap, ToolTrap)));
}
*/
//--------------------------------------------------------------  DoWeHaveGestalt

// Specifically tests for the availablity of the Gestalt() trap.
/*
Boolean DoWeHaveGestalt (void)
{
	return (TrapExists(kGestaltTrap));
}
*/
//--------------------------------------------------------------  DoWeHaveWNE  

// Specifically tests for the availablity of the WaitNextEvent() trap.
/*
Boolean DoWeHaveWNE (void)
{
	return (TrapExists(kWNETrap));
}
*/
//--------------------------------------------------------------  DoWeHaveColor  
// Determines if ROMs support Color QuickDraw (monitor not neccessarily color).
/*
Boolean DoWeHaveColor (void)
{
	SysEnvRec		thisWorld;
	
	SysEnvirons(2, &thisWorld);
	return (thisWorld.hasColorQD);
}
*/
//--------------------------------------------------------------  DoWeHaveSystem602  
// Determines if the System version is at least 6.0.2 or more recent.
/*
Boolean DoWeHaveSystem602 (void)
{
	SysEnvRec		thisWorld;
	Boolean			haveIt;
	
	SysEnvirons(2, &thisWorld);
	if (thisWorld.systemVersion >= 0x0602)
		haveIt = true;
	else
		haveIt = false;
	return (haveIt);
}
*/
//--------------------------------------------------------------  DoWeHaveSystem605  
// Determines if the System version is at least 6.0.5 or more recent.
/*
Boolean DoWeHaveSystem605 (void)
{
	SysEnvRec		thisWorld;
	Boolean			haveIt;
	
	SysEnvirons(2, &thisWorld);
	if (thisWorld.systemVersion >= 0x0605)
		haveIt = true;
	else
		haveIt = false;
	return (haveIt);
}
/
//--------------------------------------------------------------  DoWeHaveSystem7  

// Determines if the System version is at least 7.0.0 or more recent.

Boolean DoWeHaveSystem7 (void)
{
	SysEnvRec		thisWorld;
	Boolean			haveIt;
	
	SysEnvirons(2, &thisWorld);
	if (thisWorld.systemVersion >= 0x0700)
		haveIt = true;
	else
		haveIt = false;
	return (haveIt);
}

//--------------------------------------------------------------  DoWeHaveSoundManager3
// Determines if the Sound Manager version is at least 3.0.0 or more recent.
/*
Boolean DoWeHaveSoundManager3 (void)
{
//	NumVersion	version;
	Boolean		hasIt;
	
	hasIt = true;
	
	version = SndSoundManagerVersion();
	hasIt = (version.majorRev >= 3);
	
	return hasIt;
}
*/
//--------------------------------------------------------------  DoWeHaveQuickTime

Boolean DoWeHaveQuickTime (void)
{
	return true;
}

//--------------------------------------------------------------  DoWeHaveDragManager

Boolean DoWeHaveDragManager (void)
{
	return true;
}

//--------------------------------------------------------------  WhatsOurDepth  

// Determines the pixel bit depth for current device (monitor).

short WhatsOurDepth (void)
{
	GpPixelFormat_t pixelFormat;
	PortabilityLayer::HostDisplayDriver::GetInstance()->GetDisplayResolution(nil, nil, &pixelFormat);

	switch (pixelFormat)
	{
	case GpPixelFormats::k8BitCustom:
	case GpPixelFormats::k8BitStandard:
		return 8;
	case GpPixelFormats::kRGB555:
		return 16;
	case GpPixelFormats::kRGB24:
	case GpPixelFormats::kRGB32:
		return 32;
	default:
		return 0;
	}
}

void SwitchToDepth (short, Boolean)
{
}

//--------------------------------------------------------------  CanWeDisplay4Bit  
// Determines if device (monitor) capable of supporting 4 bit (16 colors/grays).
/*
Boolean CanWeDisplay4Bit (GDHandle theDevice)
{
	short		canDepth;
	Boolean		canDo;
	
	canDo = false;
	canDepth = HasDepth(theDevice, 4, 1, 0);
	if (canDepth != 0)
		canDo = true;
	
	return (canDo);
}
*/
//--------------------------------------------------------------  CanWeDisplay1Bit  
// Determines if device (monitor) capable of supporting 1 bit (black & white).
/*
Boolean CanWeDisplay1Bit (GDHandle theDevice)
{
	short		canDepth;
	Boolean		canDo;
	
	canDo = false;
	canDepth = HasDepth(theDevice, 1, 1, 0);
	if (canDepth != 0)
		canDo = true;
	
	return (canDo);
}
*/
//--------------------------------------------------------------  HowManyUsableScreens

// Counts the number of monitors that meet the depth criteria passed in.

short HowManyUsableScreens (Boolean use1Bit, Boolean use4Bit, Boolean use8Bit)
{
	return 1;
}

//--------------------------------------------------------------  FlushResolutionChange
void FlushResolutionChange(void)
{
	if (thisMac.isResolutionDirty)
	{
		GetDeviceRect(&thisMac.screen);
		thisMac.gray = thisMac.screen;
		thisMac.gray.top = 20;
		thisMac.isResolutionDirty = false;
	}
}

//--------------------------------------------------------------  CheckOurEnvirons  
// Calls all the above functions in order to fill out a sort of "spec sheet"…
// for the current Mac.

void CheckOurEnvirons (void)
{	
	thisMac.vRefNum = 0;		// TEMP
	thisMac.dirID = 0;			// TEMP
	thisMac.hasGestalt = true;	// TEMP
	thisMac.hasWNE = true;		// TEMP
	thisMac.hasColor = true;	// TEMP
	thisMac.canSwitch = true;	// TEMP
	thisMac.hasSystem7 = true;	// TEMP
	thisMac.hasSM3 = true;	// TEMP
	thisMac.hasQT = DoWeHaveQuickTime();
	thisMac.hasDrag = DoWeHaveDragManager();
	
	thisMac.can1Bit = true;
	thisMac.can4Bit = true;
	thisMac.can8Bit = true;
	thisMac.numScreens = HowManyUsableScreens(false, true, true);
	
	thisMac.wasDepth = WhatsOurDepth();
	thisMac.wasColorOrGray = AreWeColorOrGrayscale();

	thisMac.isResolutionDirty = true;
	FlushResolutionChange();
}

//--------------------------------------------------------------  HandleResolutionChange
// Installs handler
void HandleResolutionChange(uint32_t prevWidth, uint32_t prevHeight, uint32_t newWidth, uint32_t newHeight)
{
	PortabilityLayer::WindowManager::GetInstance()->HandleScreenResolutionChange(prevWidth, prevHeight, newWidth, newHeight);
	PortabilityLayer::MenuManager::GetInstance()->DrawMenuBar();
	thisMac.isResolutionDirty = true;	// Because of legacy code, we don't want to update thisMac.screen immediately, but rather, let the editor or game pick it up
}

class GpAppResolutionChangeHandler final : public PortabilityLayer::DisplayDeviceManager::IResolutionChangeHandler
{
public:
	void OnResolutionChanged(uint32_t prevWidth, uint32_t prevHeight, uint32_t newWidth, uint32_t newHeight) override
	{
		HandleResolutionChange(prevWidth, prevHeight, newWidth, newHeight);
	}

	static GpAppResolutionChangeHandler ms_instance;
};

GpAppResolutionChangeHandler GpAppResolutionChangeHandler::ms_instance;

//--------------------------------------------------------------  InstallResolutionHandler
// Installs handler
void InstallResolutionHandler(void)
{
	PortabilityLayer::DisplayDeviceManager::GetInstance()->SetResolutionChangeHandler(&GpAppResolutionChangeHandler::ms_instance);
}

//--------------------------------------------------------------  ReflectMonitor2Environs
// Tests second monitor (if available) for specific bit depth capabilities.
/*
void ReflectSecondMonitorEnvirons (Boolean use1Bit, Boolean use4Bit, Boolean use8Bit)
{
	GDHandle	tempGDevice;
	
	tempGDevice = GetDeviceList();
	while (tempGDevice != nil)
	{
		if (TestDeviceAttribute(tempGDevice, screenDevice))
			if ((use1Bit && CanWeDisplay1Bit(tempGDevice)) || 
					(use4Bit && CanWeDisplay4Bit(tempGDevice)) || 
					(use8Bit && CanWeDisplay8Bit(tempGDevice)))
				if (!TestDeviceAttribute(tempGDevice, mainScreen))
				{
					thisGDevice = tempGDevice;
					thisMac.can1Bit = CanWeDisplay1Bit(thisGDevice);
					thisMac.can4Bit = CanWeDisplay4Bit(thisGDevice);
					thisMac.can8Bit = CanWeDisplay8Bit(thisGDevice);
					thisMac.wasDepth = WhatsOurDepth();
					thisMac.wasColorOrGray = AreWeColorOrGrayscale();
					GetDeviceRect(&thisMac.screen);
					break;
				}
		tempGDevice = GetNextDevice(tempGDevice);
	}
}
*/
//--------------------------------------------------------------  HandleDepthSwitching

// Handles setting up a monitor's depth to play on.

void HandleDepthSwitching (void)
{
	if (thisMac.hasColor)
	{
		switch (isDepthPref)
		{
			case kSwitchIfNeeded:
			if ((thisMac.wasDepth != 8) && 
					((thisMac.wasDepth != 4) || (thisMac.wasColorOrGray)))
				SwitchDepthOrAbort();
			break;
			
			case kSwitchTo256Colors:
			if (thisMac.wasDepth != 8)
			{
				if (thisMac.can8Bit)
					SwitchToDepth(8, true);
				else
					SwitchDepthOrAbort();
			}
			break;
			
			case kSwitchTo16Grays:
			if ((thisMac.wasDepth != 4) || (thisMac.wasColorOrGray))
			{
				if (thisMac.can4Bit)
					SwitchToDepth(4, false);
				else
					SwitchDepthOrAbort();
			}
			break;
			
			default:
			break;
		}
	}
	
	thisMac.isDepth = WhatsOurDepth();
}

//--------------------------------------------------------------  RestoreColorDepth

// Restores a monitor to its previous depth when we quit (if we changed it).

void RestoreColorDepth (void)
{
	if ((thisMac.hasColor) && ((thisMac.wasDepth != thisMac.isDepth) || 
			(thisMac.wasColorOrGray != AreWeColorOrGrayscale())))
		SwitchToDepth(thisMac.wasDepth, true);
}

//--------------------------------------------------------------  CheckMemorySize

// Tests for a specific amount of memory available.  If the required memory…
// is not available, attempts to turn off various game features (music, etc.)…
// in order to accomodate the constrained memory available.

void CheckMemorySize (void)
{
	#define		kBaseBytesNeeded	614400L					// 600K Base memory
	#define		kPaddingBytes		204800L					// 200K Padding
	long		bytesNeeded;
	long		soundBytes, musicBytes;
	
	dontLoadMusic = false;
	dontLoadSounds = false;
	
	bytesNeeded = kBaseBytesNeeded;							// base memory
	soundBytes = SoundBytesNeeded();						// sound memory
	if (soundBytes <= 0L)
		RedAlert(kErrNoMemory);
	else
		bytesNeeded += soundBytes;
	musicBytes = MusicBytesNeeded();						// music memory
	if (musicBytes <= 0L)
		RedAlert(kErrNoMemory);
	else
		bytesNeeded += musicBytes;
	bytesNeeded += 4L * (long)thisMac.screen.bottom;		// main screen
	bytesNeeded += (((long)houseRect.right - (long)houseRect.left) * 
			((long)houseRect.bottom + 1 - (long)houseRect.top) * 
			(long)thisMac.isDepth) / 8L;					// work map
	bytesNeeded += 4L * (long)houseRect.bottom;
	bytesNeeded += (((long)houseRect.right - (long)houseRect.left) * 
			((long)houseRect.bottom + 1 - (long)houseRect.top) * 
			(long)thisMac.isDepth) / 8L;					// back map
	bytesNeeded += 4L * houseRect.bottom;
	bytesNeeded += (((long)houseRect.right - (long)houseRect.left) * 21 * 
			(long)thisMac.isDepth) / 8L;					// scoreboard map
	bytesNeeded += (6396L * (long)thisMac.isDepth) / 8L;	// more scoreboard
	bytesNeeded += (32112L * (long)thisMac.isDepth) / 8L;	// glider map
	bytesNeeded += (32112L * (long)thisMac.isDepth) / 8L;	// glider2 map
	bytesNeeded += 32064L / 8L;								// glider mask
	bytesNeeded += (912L * (long)thisMac.isDepth) / 8L;		// glider shadow
	bytesNeeded += 864L / 8L;								// shadow mask
	bytesNeeded += (304L * (long)thisMac.isDepth) / 8L;		// rubber bands
	bytesNeeded += 288L / 8L;								// bands mask
	bytesNeeded += (19344L * (long)thisMac.isDepth) / 8L;	// blower map
	bytesNeeded += 19344L / 8L;								// blower mask
	bytesNeeded += (17856L * (long)thisMac.isDepth) / 8L;	// furniture map
	bytesNeeded += 17792L / 8L;								// furniture mask
	bytesNeeded += (33264L * (long)thisMac.isDepth) / 8L;	// prizes map
	bytesNeeded += 33176L / 8L;								// prizes mask
	bytesNeeded += (2904L * (long)thisMac.isDepth) / 8L;	// points map
	bytesNeeded += 2880L / 8L;								// points mask
	bytesNeeded += (1848L * (long)thisMac.isDepth) / 8L;	// transport map
	bytesNeeded += 1792L / 8L;								// transport mask
	bytesNeeded += (3360L * (long)thisMac.isDepth) / 8L;	// switches map
	bytesNeeded += (9144L * (long)thisMac.isDepth) / 8L;	// lights map
	bytesNeeded += 9072L / 8L;								// lights mask
	bytesNeeded += (21600L * (long)thisMac.isDepth) / 8L;	// appliances map
	bytesNeeded += 21520L / 8L;								// appliances mask
	bytesNeeded += (5600L * (long)thisMac.isDepth) / 8L;	// toast map
	bytesNeeded += 5568L / 8L;								// toast mask
	bytesNeeded += (1440L * (long)thisMac.isDepth) / 8L;	// shredded map
	bytesNeeded += 1400L / 8L;								// shredded mask
	bytesNeeded += (5784L * (long)thisMac.isDepth) / 8L;	// balloon map
	bytesNeeded += 5760L / 8L;								// balloon mask
	bytesNeeded += (9632L * (long)thisMac.isDepth) / 8L;	// copter map
	bytesNeeded += 9600L / 8L;								// copter mask
	bytesNeeded += (4928L * (long)thisMac.isDepth) / 8L;	// dart map
	bytesNeeded += 4864L / 8L;								// dart mask
	bytesNeeded += (2080L * (long)thisMac.isDepth) / 8L;	// ball map
	bytesNeeded += 2048L / 8L;								// ball mask
	bytesNeeded += (1168L * (long)thisMac.isDepth) / 8L;	// drip map
	bytesNeeded += 1152L / 8L;								// drip mask
	bytesNeeded += (1224L * (long)thisMac.isDepth) / 8L;	// enemy map
	bytesNeeded += 1188L / 8L;								// enemy mask
	bytesNeeded += (2064L * (long)thisMac.isDepth) / 8L;	// fish map
	bytesNeeded += 2048L / 8L;								// fish mask
	bytesNeeded += (8960L * (long)thisMac.isDepth) / 8L;	// clutter map
	bytesNeeded += 8832L / 8L;								// clutter mask
	bytesNeeded += (23040L * (long)thisMac.isDepth) / 8L;	// support map
	bytesNeeded += (4320L * (long)thisMac.isDepth) / 8L;	// angel map
	bytesNeeded += 4224L / 8L;								// angel mask
	bytesNeeded += sizeof(roomType);
	bytesNeeded += sizeof(hotObject) * kMaxHotSpots;
	bytesNeeded += sizeof(sparkleType) * kMaxSparkles;
	bytesNeeded += sizeof(flyingPtType) * kMaxFlyingPts;
	bytesNeeded += sizeof(flameType) * kMaxCandles;
	bytesNeeded += sizeof(flameType) * kMaxTikis;
	bytesNeeded += sizeof(flameType) * kMaxCoals;
	bytesNeeded += sizeof(pendulumType) * kMaxPendulums;
	bytesNeeded += sizeof(savedType) * kMaxSavedMaps;
	bytesNeeded += sizeof(bandType) * kMaxRubberBands;
	bytesNeeded += sizeof(greaseType) * kMaxGrease;
	bytesNeeded += sizeof(starType) * kMaxStars;
	bytesNeeded += sizeof(shredType) * kMaxShredded;
	bytesNeeded += sizeof(dynaType) * kMaxDynamicObs;
	bytesNeeded += sizeof(objDataType) * kMaxMasterObjects;
	bytesNeeded += kDemoLength;		SpinCursor(1);
	
	SpinCursor(1);
}

void GetDeviceRect(Rect *rect)
{
	unsigned int width;
	unsigned int height;
	PortabilityLayer::HostDisplayDriver::GetInstance()->GetDisplayResolution(&width, &height, nil);

	SetRect(rect, 0, 0, static_cast<short>(width), static_cast<short>(height));
}

Boolean AreWeColorOrGrayscale()
{
	// ... As opposed to B&W
	return true;
}

void SwitchDepthOrAbort(void)
{
}
