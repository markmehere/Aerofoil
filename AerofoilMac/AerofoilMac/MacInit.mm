#import <Cocoa/Cocoa.h>
#import "AerofoilApplication.h"
#import "AerofoilAppDelegate.h"
#import "MacInit.h"
#include "SDL.h"

int MacInit(void) {
	// Instantiate NSApp and its delegate first, so SDL chooses those over its own implementation.
	[AerofoilApplication sharedApplication];
	[[NSBundle mainBundle] loadNibNamed:@"MainMenu" owner:NSApp topLevelObjects:nil];
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
		return -1;
	
	// Gracefully activate app.
	//
	// (SDL forcefully does this via [NSApp activateIgnoringOtherApps:YES],
	// which isn't consistent with normal Mac apps).
	[NSApp finishLaunching];
	
	return 0;
}
