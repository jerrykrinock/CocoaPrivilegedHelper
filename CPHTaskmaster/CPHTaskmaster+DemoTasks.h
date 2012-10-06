#import <Cocoa/Cocoa.h>
#import "CPHTaskmaster.h"


@interface CPHTaskmaster (Tasks) 

- (BOOL)getVersion_p:(NSString**)version
			 error_p:(NSError**)error_p ;

- (BOOL)getRUID:(NSString**)ruid_p
		   EUID:(NSString**)euid_p
		error_p:(NSError**)error_p ;

- (BOOL)openSomeLowPortNumbers_p:(NSArray**)lowNumberedPorts_p	
					error_p:(NSError**)error_p ;

- (BOOL)setAnyUserPrefs:(NSDictionary*)prefs
inBundleIdentifier:(NSString*)bundleIdentifier
				error_p:(NSError**)error_p ;

- (BOOL)writeData:(NSData*)data
		   toFile:(NSString*)path
		  error_p:(NSError**)error_p ;

@end
