#import "CPHTaskmaster+StatPaths.h"
#import "CPHTasks_Typicapp.h"
#import <sys/stat.h>

#import "NSError+LowLevel.h"

@implementation CPHTaskmaster (StatFilesTask)


- (BOOL)statFilePaths:(NSArray*)paths
			  stats_p:(NSDictionary**)stats_p
			  error_p:(NSError**)error_p {
    BOOL ok = YES ;
    NSError* error = nil ;
    
	// Create request.
	NSDictionary* request = [NSDictionary dictionaryWithObjectsAndKeys:
							 @kStatPathsCommand, @kBASCommandKey,
							 (CFArrayRef)paths, @kInfos,
							 nil] ;
	
	// Execute request.
	NSDictionary* response = nil ;
    ok = [self executeRequest:request
						response_p:&response
						   error_p:&error] ;
	
	if (ok) {
		if (stats_p) {
			*stats_p = [response objectForKey:@kStatDatas] ;
		}
		
		NSDictionary* failures = [response objectForKey:@kErrorCodes] ;
		if ([failures respondsToSelector:@selector(count)]) {
			if ([failures count] != 0) {
                ok = NO ;
                NSDictionary* userInfo = [NSDictionary dictionaryWithObjectsAndKeys:
                                          @"Some requested paths failed", NSLocalizedDescriptionKey,
                                          [response objectForKey:@kErrorCodes], @kErrorCodes,
                                          nil] ;
                error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                            code:CPHTaskmasterSomePartsFailed_ErrorCode
                                        userInfo:userInfo] ;
			}
		}
	}
	
    if (error && error_p) {
        *error_p = error ;
    }
    
	return ok ;
}	

- (BOOL)statPath:(NSString*)path
			stat:(struct stat*)stat_p
		 error_p:(NSError**)error_p {
	// For efficiency in case the caller expects the path may not exist,
	// and has passed error_p = NULL, we don't create a local error.
	NSArray* paths = [NSArray arrayWithObject:path] ;
	NSDictionary* stats = nil ;
	BOOL ok = [self statFilePaths:paths
						  stats_p:&stats
						  error_p:error_p] ;

	NSData* statData = [stats objectForKey:path] ;
	struct stat aStat ;
	size_t statSize = sizeof(aStat) ;
	if (statData && stat_p) {
		const void* bytes = [statData bytes] ;
		memcpy(&aStat, bytes, statSize) ;
		*stat_p = aStat ;
	}
	else if (error_p != NULL) {
		NSError* underlyingError = [[*error_p userInfo] objectForKey:NSUnderlyingErrorKey] ;
		NSDictionary* underlyingUserInfo = [underlyingError userInfo] ;
		NSDictionary* errorCodes = [underlyingUserInfo objectForKey:@kErrorCodes] ;
		NSNumber* errorCodeNumber = [errorCodes objectForKey:path] ;
		NSInteger errorCode = [errorCodeNumber integerValue] ;
		if (errorCode == CPHTaskmasterPathTooLong_ErrorCode) {
			*error_p = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                           code:513315
                                       userInfo:[NSDictionary dictionaryWithObject:@"Path too long"
                                                                            forKey:NSLocalizedDescriptionKey]] ;
		}
		else {
			*error_p = [NSError errorWithPosixErrorCode:(int)errorCode] ;
		}

        *error_p = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                       code:513563
                                   userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                             @"Could get stat for path even after elevating", NSLocalizedDescriptionKey,
                                             *error_p, NSUnderlyingErrorKey,  // may be nil
                                             nil]] ;
	}

	return ok ;
}

- (NSDate*)modificationDateForPath:(NSString*)path 
						   error_p:(NSError**)error_p {
	struct stat aStat ;
    struct timespec emptyTimespec ;
    aStat.st_mtimespec = emptyTimespec ;
	NSError* error = nil ;
	NSDate* date = nil ;
	BOOL ok = [[CPHTaskmaster sharedTaskmaster] statPath:path
                                                    stat:&aStat
                                                 error_p:&error] ;
	if (ok) {
		time_t secs = aStat.st_mtimespec.tv_sec ;
		long nanosecs = aStat.st_mtimespec.tv_nsec ;
		NSTimeInterval timeSince1970 = secs + 1e-9 * nanosecs ;
		date = [NSDate dateWithTimeIntervalSince1970:timeSince1970] ;
	}
		
	return date ;
}

@end