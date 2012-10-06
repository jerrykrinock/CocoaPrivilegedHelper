// Needed for any key constants used in the request/response dics and error numbers:
#include "CPHTasks_Typicapp.h"

// Needed for COMMAND_PROC_ARGUMENTS:
#include "CPH_Constants.h"

// Provides some handy utilities for dealing with CF (CoreFoundation).
// It may not be used in all tasks but is included so you don't lose it.
#include "MoreCFQ.h"

#include <unistd.h>
#include <sys/stat.h>
#include "SSYUnixPathUtils.h"

OSStatus DoStatPaths(COMMAND_PROC_ARGUMENTS) {
#pragma unused (auth)
#pragma unused (userData)
	OSStatus retval = noErr;
	
	// Pre-conditions
    
    // userData may be NULL
	assert(request != NULL);
	assert(response != NULL);
    // asl may be NULL
    // aslMsg may be NULL
	
	// Get Info from arguments and assert that it's a CFArrayRef
	CFArrayRef infos = CFDictionaryGetValue(request, CFSTR(kInfos)) ;
	assert(infos != NULL) ;
	assert(CFGetTypeID(infos) == CFArrayGetTypeID()) ;
	
	CFIndex nFiles = CFArrayGetCount(infos) ;
	CFIndex i ;
	int nSucceeded = 0 ;
	CFMutableDictionaryRef statDatas = CFDictionaryCreateMutable(
																NULL,
																nFiles,
																&kCFTypeDictionaryKeyCallBacks,
																&kCFTypeDictionaryValueCallBacks
	) ;
	CFMutableDictionaryRef errorCodes = CFDictionaryCreateMutable(
															   NULL,
															   nFiles,
															   &kCFTypeDictionaryKeyCallBacks,
															   &kCFTypeDictionaryValueCallBacks
															   ) ;
	for (i=0; i<nFiles; i++) {
		CFStringRef path = CFArrayGetValueAtIndex(infos, i) ;
		assert(CFGetTypeID(path) == CFStringGetTypeID()) ;
		
		int errorCode = 0 ;
		
		char pathC[SSY_UNIX_PATH_UTILS_MAX_PATH_CHARS] ;
		if (errorCode == 0) {
			Boolean ok = CFStringGetCString(
										 path,
										 pathC,
										 SSY_UNIX_PATH_UTILS_MAX_PATH_CHARS,
										 kCFStringEncodingASCII
										 ) ;
			if (!ok) {
				errorCode = CPHTaskmasterPathTooLong_ErrorCode ;
			}
		}
		
		if (errorCode == 0) {
			struct stat aStat ;
			int result = stat(pathC, &aStat) ;
			if (result == 0) {
				CFDataRef statData = CFDataCreate (
												   kCFAllocatorDefault,
												   (UInt8*)&aStat,
												   sizeof(aStat)
												   ) ;
				CFDictionaryAddValue(statDatas, path, statData) ;
				CFQRelease(statData) ;
			}
			else {
				errorCode = errno ;
			}
		}
				
		if (errorCode == 0) {
			nSucceeded++ ;
		}
		else {
			CFNumberRef errorNumber = CFNumberCreate(
													 kCFAllocatorDefault,
													 kCFNumberIntType,
													 &errorCode
													 ) ;
			CFDictionaryAddValue(errorCodes, path, errorNumber) ;
			CFQRelease(errorNumber) ;
		}
	}
		
	CFDictionaryAddValue(response, CFSTR(kStatDatas), statDatas) ;
	CFRelease(statDatas) ;
	CFDictionaryAddValue(response, CFSTR(kErrorCodes), errorCodes) ;
	CFRelease(errorCodes) ;
	
	asl_log(
			asl,
			aslMsg,
			ASL_LEVEL_DEBUG,
			"DoStatPaths did good %d/%d requested files",
			nSucceeded,
			(int)nFiles
			) ;
	
	// Return the number of file copy operations that failed
	retval = (OSStatus)(nFiles - nSucceeded) ;
	
	return retval ;
}	