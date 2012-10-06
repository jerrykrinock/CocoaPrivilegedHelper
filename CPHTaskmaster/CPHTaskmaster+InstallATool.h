#import <Cocoa/Cocoa.h>
#import "CPHTaskmaster.h"


@interface CPHTaskmaster (InstallAToolTask)

/*!
 @brief    Executes a task in a privileged helper tool which
 installs a file in a privleged location
 
 @param    error_p  On output, if an error occurs, points
 to a descriptive error.&nbsp;  Pass NULL if you're not
 interested.
 @result   YES if all copy operations succeeded, NO otherwise.
 */
- (BOOL)installAToolFromResourceName:(NSString*)resourceName
                destinationDirectory:(NSString*)destinationDirectory
                             error_p:(NSError**)error_p ;


@end
