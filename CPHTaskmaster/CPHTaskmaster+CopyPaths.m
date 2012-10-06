#import "CPHTaskmaster+CopyPaths.h"
#import "CPHTasks_Typicapp.h"
#import "NSError+LowLevel.h"

NSString* const CPHTaskmasterPerPathErrors = @"CPHTaskmasterPerPathErrors" ;



@implementation CPHTaskmaster (CopyPathsTask)

- (BOOL)copyFilePaths:(NSDictionary*)paths
			  error_p:(NSError**)error_p {
	if (!error_p) {
		NSError* error = nil ;
		error_p = &error ;
	}
	
	NSMutableArray* infos = [[NSMutableArray alloc] init] ;
	
	for (NSString* path in paths) {
		
		
		NSDictionary* info = [NSDictionary dictionaryWithObjectsAndKeys:
							  path, @kSourcePath,
							  [paths objectForKey:path], @kDestinPath,
							  nil] ;
		
		[infos addObject:info] ;
	}
	
	// Create request.
	NSDictionary* request = [NSDictionary dictionaryWithObjectsAndKeys:
							 @kCopyPathsCommand, @kBASCommandKey,
							 (CFArrayRef)infos, @kInfos,
							 nil] ;
	[infos release] ;
	
	// Execute request.
	NSDictionary* response = nil ;
	NSError* underlyingError = nil ;
	BOOL ok = [self executeRequest:request
						response_p:&response
						   error_p:&underlyingError] ;
	
	NSInteger errCode = 0 ;
	if (!ok) {
		errCode = CPHTaskmasterRequestExecutionFailed_ErrorCode ;
	}
	else {
		NSDictionary* errors = [response objectForKey:@kErrors] ;
		if ([errors respondsToSelector:@selector(count)]) {
			if ([errors count] != 0) {
				errCode = CPHTaskmasterSomePartsFailed_ErrorCode ;
			}
		}
	}
	
	if (errCode != 0) {
        if (error_p) {
            NSMutableDictionary* userInfo = [NSMutableDictionary dictionary] ;
            [userInfo setObject:@"CPHTaskmaster executeRequest::: failed"
                         forKey:NSLocalizedDescriptionKey] ;
            [userInfo setValue:paths
                        forKey:@"Paths"] ;
            NSDictionary* perPathErrors = [response objectForKey:@kErrors] ;
            if (perPathErrors) {
                [userInfo setObject:perPathErrors
                             forKey:CPHTaskmasterPerPathErrors] ;
            }
            else {
                [userInfo setValue:underlyingError
                            forKey:NSUnderlyingErrorKey] ;
            }
            *error_p = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                           code:errCode
                                       userInfo:userInfo] ;
        }
	}
	
	return ok ;
}	

- (BOOL)copyPath:(NSString*)sourcePath
		  toPath:(NSString*)destinPath
		 error_p:(NSError**)error_p {
	NSError* error = nil ;
	NSDictionary* paths = [NSDictionary dictionaryWithObjectsAndKeys:
						   destinPath,   // value (destin)
						   sourcePath,   // key   (source)
						   nil] ;
	BOOL ok = [self copyFilePaths:paths
						  error_p:&error] ;
	if (!ok) {
		NSDictionary* perPathErrors = [[error userInfo] objectForKey:CPHTaskmasterPerPathErrors] ;
		if (perPathErrors) {
			NSDictionary* errorDictionary = [perPathErrors objectForKey:sourcePath] ;
			if (errorDictionary) {
				error = [NSError errorWithPosixErrorCode:(int)[[errorDictionary objectForKey:@kErrno] integerValue]] ;
			}
		}
	}
	
	if (error && error_p) {
		*error_p = error ;
	}
	
	return ok ;
}

@end