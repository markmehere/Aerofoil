#import "AerofoilApplication.h"
#include "SDL.h"

@implementation AerofoilApplication

- (void)terminate:(id)sender {
	SDL_Event event;
	event.quit.type = SDL_QUIT;
	event.quit.timestamp = SDL_GetTicks();
	SDL_PushEvent(&event);
}

@end
