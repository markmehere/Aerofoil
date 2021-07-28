#import "AerofoilAppDelegate.h"
#import "About.h"
#include "WindowManager.h"
#include "GliderDefines.h" // kPlayMode

extern short theMode;
void DoSettingsMain(void);

@interface AerofoilAppDelegate ()

@property (weak) IBOutlet NSMenuItem *aboutAerofoilMenuItem;
@property (weak) IBOutlet NSMenuItem *aboutGliderPROMenuItem;
@property (weak) IBOutlet NSMenuItem *preferencesMenuItem;

@end

@implementation AerofoilAppDelegate

- (IBAction)showAboutAerofoil:(id)sender {
	[self performAsynchronously:DoAboutFramework];
}
- (IBAction)showAboutGliderPRO:(id)sender {
	[self performAsynchronously:DoAbout];
}
- (IBAction)showPreferences:(id)sender {
	[self performAsynchronously:DoSettingsMain];
}

- (void)performAsynchronously:(void(*)())function {
	dispatch_async(dispatch_get_main_queue(), ^{
		function();
	});
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
