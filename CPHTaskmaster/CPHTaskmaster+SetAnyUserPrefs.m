#import "CPHTaskmaster+SetAnyUserPrefs.h"
#import "CPHTasks_Demo.h"

@implementation CPHTaskmaster (Tasks)

- (BOOL)setAnyUserPrefs:(NSDictionary*)prefs
	 inBundleIdentifier:(NSString*)bundleIdentifier
				error_p:(NSError**)error_p {
    BOOL ok = YES ;
    NSError* error = nil ;
    
	if ([prefs count] < 1) {
        ok = NO ;
		error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                    code:CPHTaskmasterNoPayload_ErrorCode
                                userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                          [[self class] description], @"Object Class",
                                          NSStringFromSelector(_cmd), @"Method",
                                          @"No key/value pairs to set in prefs", NSLocalizedDescriptionKey,
                                          nil]] ;
		goto end ;
	}
	
    if ([bundleIdentifier length] < 1) {
        ok = NO ;
		error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                    code:CPHTaskmasterNoBundleIdentifier_ErrorCode
                                userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                          [[self class] description], @"Object Class",
                                          NSStringFromSelector(_cmd), @"Method",
                                          @"No bundle identifier given to set", NSLocalizedDescriptionKey,
                                          nil]] ;
		goto end ;
	}
	
	// Create request.
	NSDictionary* request = [NSDictionary dictionaryWithObjectsAndKeys:
							 @kSetAnyUserPrefsCommand, @kBASCommandKey,
							 prefs, @kDictionaryPayload,
							 bundleIdentifier, @kBundleIdentifier,
							 nil] ;
	
	// Execute request.
	ok = [self executeRequest:request
                   response_p:NULL
                      error_p:&error] ;
	
	// Process response
	// No response and thus no processing required for this task
	
end:
    if (error && error_p) {
        *error_p = error ;
    }
    
	return ok ;
}	


@end