#import "CPHTaskmaster+DemoTasks.h"
#import "CPHTasks_Demo.h"
#include <unistd.h>     // for close() in -openSomeLowPortNumbers_p:error_p
#include <netinet/in.h> // for sock... stuff in -openSomeLowPortNumbers_p:error_p

@implementation CPHTaskmaster (Tasks)

- (BOOL)getVersion_p:(NSString**)version_p
			 error_p:(NSError**)error_p {
	if (!error_p) {
		NSError* error = nil ;
		error_p = &error ;
	}
	
    assert (version_p != NULL) ;
    
    // Create the request.  The request always contains the kBASCommandKey that 
    // describes the command to do. 
	NSDictionary* request = [NSDictionary dictionaryWithObject:@kGetVersionCommand
														forKey:@kBASCommandKey] ;
	
	NSDictionary* response ;
	BOOL ok = [self executeRequest:request
						response_p:&response
						   error_p:error_p] ;
    
	if (!ok) {goto end;}
	
	// Process response to get desired data
	*version_p = [response objectForKey:@kNumberPayload] ;
	
end:
	return ok ;
}

- (BOOL)getRUID:(NSString**)ruid_p
		   EUID:(NSString**)euid_p
		error_p:(NSError**)error_p {
	if (!error_p) {
		NSError* error = nil ;
		error_p = &error ;
	}
	
	assert (ruid_p != NULL) ;
	assert (euid_p != NULL) ;
	
    // Create the request.  The request always contains the kBASCommandKey that 
    // describes the command to do. 
	NSDictionary *request = [NSDictionary dictionaryWithObject:@kGetUIDsCommand
														forKey:@kBASCommandKey] ;
	
	NSDictionary* response ;
	BOOL ok = [self executeRequest:request
						response_p:&response
						   error_p:error_p] ;
	
	if (!ok) {goto end;}
	
	// Process response to get desired data
	*ruid_p = [(NSDictionary *)response objectForKey:@kNumberRUID] ;
	*euid_p = [(NSDictionary *)response objectForKey:@kNumberEUID] ;
	
end:
	return ok ;
}	

#define kNumberOfLowNumberedPorts 3

- (BOOL)openSomeLowPortNumbers_p:(NSArray**)lowNumberedPorts_p	
						 error_p:(NSError**)error_p {	
	if (!error_p) {
		NSError* error = nil ;
		error_p = &error ;
	}
	
	// Check arguments.    
    assert(lowNumberedPorts_p != NULL);
    
    // Create request.
	NSDictionary* request = [NSDictionary dictionaryWithObject:@kOpenSomeLowNumberedPortsCommand
														forKey:@kBASCommandKey] ;
	
	// Execute request.
	NSDictionary* response ;
	BOOL ok = [self executeRequest:request
						response_p:&response
						   error_p:error_p] ;
	
	// ok == YES guarantees that the *response_p is non-nil.
	if (!ok) {goto end;}
	
	
	// Process response to get descriptors[] (file descriptors)
	CFArrayRef      descArray;
	CFIndex         arrayIndex;
	CFIndex         arrayCount;
	CFNumberRef     thisNum;
	
	descArray = (CFArrayRef) CFDictionaryGetValue((CFDictionaryRef)response, CFSTR(kBASDescriptorArrayKey));
	assert( descArray != NULL );
	assert( CFGetTypeID(descArray) == CFArrayGetTypeID() );
	
	arrayCount = CFArrayGetCount(descArray);
	assert(arrayCount == kNumberOfLowNumberedPorts);
	
	int descriptors[kNumberOfLowNumberedPorts] = { -1, -1, -1 };
	for (arrayIndex = 0; arrayIndex < kNumberOfLowNumberedPorts; arrayIndex++) {
		thisNum = CFArrayGetValueAtIndex(descArray, arrayIndex);
		assert(thisNum != NULL);
		assert( CFGetTypeID(thisNum) == CFNumberGetTypeID() );
		
		ok = CFNumberGetValue(thisNum, kCFNumberIntType, &descriptors[arrayIndex]);
		if (!ok) {
			NSString* msg = [NSString stringWithFormat:@"Could not get number value from descriptors for index %ld", (long)arrayIndex] ;
			*error_p = [NSError errorWithDomain:@"CPHTaskmaster"
										   code:28970
									   userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
												 [[self class] description], @"Object Class",
												 NSStringFromSelector(_cmd), @"Method",
												 msg, NSLocalizedDescriptionKey,
												 nil]] ;
			break ;
		}
	}
    
	if (!ok) {goto end;}
	
	// If this were a real program, we would do something with the file descriptors at this point
	// Instead, we just get their port numbers to send back to the invoker and then close them.

	// Process descriptors[] to get ports[] (port numbers)
	uint16_t    ports[kNumberOfLowNumberedPorts];
	int         portIndex;
	for (portIndex = 0; portIndex < kNumberOfLowNumberedPorts; portIndex++) {
		int                 sockErr;
		struct sockaddr_in  boundAddr;
		socklen_t           boundAddrLen;
		
		memset(&boundAddr, 0, sizeof(boundAddr));
		boundAddrLen = sizeof(boundAddr);
		
		sockErr = getsockname(descriptors[portIndex], (struct sockaddr *) &boundAddr, &boundAddrLen);
		assert(sockErr == 0);
		assert(boundAddrLen == sizeof(boundAddr));
		uint16_t portNumber = ntohs(boundAddr.sin_port) ;
		ports[portIndex] = portNumber;
	}
	
	*lowNumberedPorts_p = [NSArray arrayWithObjects:
						   [NSNumber numberWithUnsignedInt:(unsigned int) ports[0]],
						   [NSNumber numberWithUnsignedInt:(unsigned int) ports[1]],
						   [NSNumber numberWithUnsignedInt:(unsigned int) ports[2]],
						   nil] ;
	
	// Close the file descriptors.
	for (portIndex = 0; portIndex < kNumberOfLowNumberedPorts; portIndex++) {
		assert(close(descriptors[portIndex]) == 0) ;
	}
	
end:
	return ok ;
}

- (BOOL)setAnyUserPrefs:(NSDictionary*)prefs
	 inBundleIdentifier:(NSString*)bundleIdentifier
				error_p:(NSError**)error_p {
    BOOL ok = YES ;
    NSError* error = nil ;

	if ([prefs count] < 1) {
        ok = NO ;
		error = [NSError errorWithDomain:@"CPHTaskmaster"
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
		error = [NSError errorWithDomain:@"CPHTaskmaster"
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
	
	if (!ok) {
        goto end;
    }
	
	// Process response 
	// No response and thus no processing required for this task
	
end:
    if (error && error_p) {
        *error_p = error ;
    }
    
	return ok ;
}	


- (BOOL)writeData:(NSData*)data
		   toFile:(NSString*)path
		  error_p:(NSError**)error_p {
    BOOL ok = YES ;
    NSError* error = nil ;
    
	if (data == nil) {
		// You might consider this an error.
		// If so, assign *error_p here.
		goto end ;
	}
	
    if ([path length] < 1) {
        ok = NO ;
		error = [NSError errorWithDomain:@"CPHTaskmaster"
                                    code:CPHTaskmasterNoPayload_ErrorCode
                                userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                          [[self class] description], @"Object Class",
                                          NSStringFromSelector(_cmd), @"Method",
                                          @"No key/value pairs to set in prefs", NSLocalizedDescriptionKey,
                                          nil]] ;
		goto end ;
	}
	
    if (![path hasPrefix:@"/"]) {
        ok = NO ;
		error = [NSError errorWithDomain:@"CPHTaskmaster"
                                    code:CPHTaskmasterNoBundleIdentifier_ErrorCode
                                userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                          [[self class] description], @"Object Class",
                                          NSStringFromSelector(_cmd), @"Method",
                                          @"No key/value pairs to set in prefs", NSLocalizedDescriptionKey,
                                          nil]] ;
		goto end ;
	}
	
	// Create request.
	NSDictionary* request = [NSDictionary dictionaryWithObjectsAndKeys:
							 @kWriteDataToFileCommand, @kBASCommandKey,
							 (CFDataRef)data, @kData,
							 (CFStringRef)path, @kPath,
							 nil] ;
	
	// Execute request.
	ok = [self executeRequest:request
                   response_p:NULL
                      error_p:&error] ;
	
	if (!ok) {
		goto end;
	}
	
	// Process response 
	// No response and thus no processing required for this task
	
end:
    if (error && error_p) {
        *error_p = error ;
    }
    
	return ok ;
}	

@end