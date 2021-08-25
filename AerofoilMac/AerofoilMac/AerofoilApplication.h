#import <Cocoa/Cocoa.h>
#include "GpVOSEvent.h"

NS_ASSUME_NONNULL_BEGIN

@interface AerofoilApplication : NSApplication

- (void)sendMenuItemEvent:(GpMenuItemSelectionEvent_t)itemEvent;

@end

NS_ASSUME_NONNULL_END
