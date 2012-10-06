// Needed for any key constants used in the request/response dics:
#include "CPHTasks_Demo.h"

// Needed for COMMAND_PROC_ARGUMENTS:
#include "CPH_Constants.h"

// Provides some handy utilities for dealing with CF (CoreFoundation).
// It may not be used in all tasks but is included so you don't lose it.
#include "MoreCFQ.h"


OSStatus DoWriteDataToFile(COMMAND_PROC_ARGUMENTS) {
#pragma unused (auth)
#pragma unused (userData)
	OSStatus retval = noErr;
	
	// Pre-conditions
    
    // userData may be NULL
	assert(request != NULL);
	assert(response != NULL);
    // asl may be NULL
    // aslMsg may be NULL
	
	// Get data to write and assert that it's a CFDataRef
	CFDataRef data = CFDictionaryGetValue(request, CFSTR(kData)) ;
	assert(data != NULL) ;
	assert(CFGetTypeID(data) == CFDataGetTypeID()) ;
	
	// Get path and assert that it's a CFStringRef
	CFStringRef filePath = CFDictionaryGetValue(request, CFSTR(kPath)) ;
	assert(filePath != NULL) ;
	assert(CFGetTypeID(filePath) == CFStringGetTypeID()) ;
	
	CFURLRef url = CFURLCreateWithFileSystemPath (
												  kCFAllocatorDefault,
												  filePath,
												  kCFURLPOSIXPathStyle,
												  false
												  ) ;
	
	SInt32 errorCode ;
	Boolean ok = CFURLWriteDataAndPropertiesToResource (
														url,
														data,
														NULL,
														&errorCode
														) ;
	if (!ok) {
		retval = errorCode ;
	}
	
	asl_log(asl,
			aslMsg,
			ASL_LEVEL_DEBUG,
			"DoWriteDataToFile result ok=%d for %s",
			ok,
			CFStringGetCStringPtr(
								  filePath,
								  CFStringGetSystemEncoding()
								  )
			) ;
	
	// Clean up
	CFQRelease(url) ;
	
	return retval ;
}	

