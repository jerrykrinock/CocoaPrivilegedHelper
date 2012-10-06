#import <Cocoa/Cocoa.h>
#import "CPHTaskmaster.h"


@interface CPHTaskmaster (SetPermissionsTask) 

/*!
 @param    permissions  An array of permissions to which
 @param    error_p  On output, if an error occurs, points
 to a descriptive error.&nbsp;  Pass NULL if you're not 
 interested.
 @result   YES if all copy operations succeeded, NO otherwise.
 */
/*!
 @brief    Reads and returns the existing BSD permissions on a
 given set of filesystem paths, and sets given new permissions.
 
 @param    oldPermissions_p  If a pointer to an NSDictionary is provided
 on input, on output will point to an dictionary of permissions originally
 found for the paths given as keys in newPermissions..&nbsp;  Values
 will be wrapped as NSNumbers and can be recovered by sending
 -[NSNumber unsignedShortValue].
 @param    newPermissions  An dictionary whose keys are strings
 representing filesystem paths and whose values are NSNumbers with
 unsigned short integer values equal to the new permissions to be set
 for its key.&nbsp;  Note: To create the NSNumber values, use
 -[NSNumber numberWithUnsignedShort:].
 @param    error_p  On output, if an error occurs, points
 to a descriptive error.&nbsp;  Pass NULL if you're not 
 interested.
 @result   YES if all copy operations succeeded, NO otherwise.
 */
- (BOOL)getPermissions_p:(NSDictionary**)oldPermissions_p
		  setPermissions:(NSDictionary*)newPermissions
				 error_p:(NSError**)error_p ;

- (BOOL)getPermissionNumber_p:(NSNumber**)oldPermission_p
		  setPermissionNumber:(NSNumber*)newPermission
						 path:(NSString*)path
					  error_p:(NSError**)error_p ;

@end
