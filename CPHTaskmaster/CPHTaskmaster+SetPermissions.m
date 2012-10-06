#import "CPHTaskmaster+SetPermissions.h"
#import "CPHTasks_Typicapp.h"

@implementation CPHTaskmaster (SetPermissionsTask)

- (BOOL)getPermissions_p:(NSDictionary**)oldPermissions_p
		  setPermissions:(NSDictionary*)newPermissions
				 error_p:(NSError**)error_p {
    NSError* error = nil ;
	
	// Create request.
	NSDictionary* request = [NSDictionary dictionaryWithObjectsAndKeys:
							 @kSetPermissionsCommand, @kBASCommandKey,
							 (CFDictionaryRef)newPermissions, @kInfos,
							 nil] ;
	
	// Execute request in helper tool.
	NSDictionary* response = nil ;
	BOOL ok = [self executeRequest:request
						response_p:&response
						   error_p:&error] ;
	if (ok)  {
		NSDictionary* failures = [response objectForKey:@kErrorDescriptions] ;
		if ([failures respondsToSelector:@selector(count)]) {
			if ([failures count] != 0) {
                NSMutableDictionary* userInfo = [NSMutableDictionary dictionary] ;
                [userInfo setObject:@"CPHTaskmaster failed"
                             forKey:NSLocalizedDescriptionKey] ;
                [userInfo setValue:[response objectForKey:@kErrorDescriptions]
                            forKey:@kErrorDescriptions] ;
                
                error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                            code:CPHTaskmasterSomePartsFailed_ErrorCode
                                        userInfo:userInfo] ;
			}
		}
	}
	
	if (oldPermissions_p) {
		NSDictionary* infos = [response objectForKey:@kInfos] ;
		*oldPermissions_p = infos ;
	}
	
    if (error && error_p) {
        *error_p = error ;
    }
	
    return ok ;
}	

- (BOOL)getPermissionNumber_p:(NSNumber**)oldPermissionNumber_p
		  setPermissionNumber:(NSNumber*)newPermissionNumber
						 path:(NSString*)path
					  error_p:(NSError**)error_p {
	NSDictionary* oldPermissions = nil ;
	NSDictionary* newPermissions = [NSDictionary dictionaryWithObject:newPermissionNumber
															   forKey:path] ;
	BOOL ok = [self getPermissions_p:&oldPermissions
					  setPermissions:newPermissions
							 error_p:error_p] ;
	if (oldPermissionNumber_p) {
		NSArray* oldPermissionValues = [oldPermissions allValues] ;
		if ([oldPermissionValues count] > 0) {
			*oldPermissionNumber_p = [oldPermissionValues objectAtIndex:0] ;
		}
	}
	
	return ok ;
}

@end