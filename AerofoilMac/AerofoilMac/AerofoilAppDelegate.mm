#import "AerofoilAppDelegate.h"
#import "AerofoilApplication.h"
#include "WindowManager.h"
#include "GliderDefines.h" // kPlayMode

extern short theMode;

@interface AerofoilAppDelegate ()

@property (weak) IBOutlet NSMenuItem *aboutAerofoilMenuItem;
@property (weak) IBOutlet NSMenuItem *aboutGliderPROMenuItem;
@property (weak) IBOutlet NSMenuItem *preferencesMenuItem;

@end

@implementation AerofoilAppDelegate

- (IBAction)showAboutAerofoil:(id)sender {
	[NSApp sendMenuItemEvent:GpMenuItemSelectionEvents::kAboutAerofoil];
}
- (IBAction)showAboutGliderPRO:(id)sender {
	[NSApp sendMenuItemEvent:GpMenuItemSelectionEvents::kAboutGliderPRO];
}
- (IBAction)showPreferences:(id)sender {
	[NSApp sendMenuItemEvent:GpMenuItemSelectionEvents::kPreferences];
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
	if (menuItem == _aboutAerofoilMenuItem || menuItem == _aboutGliderPROMenuItem) {
		return ![self menuItemsDisabled];
	} else if (menuItem == _preferencesMenuItem) {
		return ![self menuItemsDisabled];
	} else {
		return NO;
	}
}

- (BOOL)menuItemsDisabled {
	PortabilityLayer::WindowManager* wm = PortabilityLayer::WindowManager::GetInstance();
	return theMode == kPlayMode || wm->IsExclusiveWindowVisible();
}

@end
