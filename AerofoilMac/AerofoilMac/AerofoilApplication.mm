#import "AerofoilApplication.h"
#include "IGpVOSEventQueue.h"
#include "GpAppInterface.h"
#include "SDL.h"

@implementation AerofoilApplication

- (void)terminate:(id)sender {
	SDL_Event event;
	event.quit.type = SDL_QUIT;
	event.quit.timestamp = SDL_GetTicks();
	SDL_PushEvent(&event);
}

- (void)sendMenuItemEvent:(GpMenuItemSelectionEvent_t)itemEvent {
	GpVOSEvent event;
	event.m_eventType = GpVOSEventTypes::kMenuItemSelected;
	event.m_event.m_menuItemSelectionEvent = itemEvent;
	[self sendVOSEvent:event];
}

- (void)sendVOSEvent:(GpVOSEvent)event {
	IGpVOSEventQueue* queue = GpAppInterface_Get()->PL_GetDriverCollection()->GetDriver<GpDriverIDs::kEventQueue>();
	if (GpVOSEvent *evt = queue->QueueEvent())
		*evt = event;
}

@end
