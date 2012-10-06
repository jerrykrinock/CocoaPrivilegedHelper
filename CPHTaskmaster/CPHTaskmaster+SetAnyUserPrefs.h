#import <Cocoa/Cocoa.h>
#import "CPHTaskmaster.h"


@interface CPHTaskmaster (Tasks) 

- (BOOL)setAnyUserPrefs:(NSDictionary*)prefs
	 inBundleIdentifier:(NSString*)bundleIdentifier
				error_p:(NSError**)error_p ;

@end
