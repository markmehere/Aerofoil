#import "AerofoilApplication.h"
#include "SDL.h"

extern int SDL_SendQuit(void);

@implementation AerofoilApplication

- (void)terminate:(id)sender {
	// TODO: Use Aerofoil method instead of private SDL method
	SDL_SendQuit();
}

@end
