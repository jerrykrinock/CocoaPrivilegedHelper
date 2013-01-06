// Needed for any key constants used in the request/response dics:
#include "CPHTasks_Typicapp.h"

// Needed for COMMAND_PROC_ARGUMENTS:
#include "CPH_Constants.h"

// Provides some handy utilities for dealing with CF (CoreFoundation).
// It may not be used in all tasks but is included so you don't lose it.
#include "MoreCFQ.h"

#include <sys/types.h>
#include <sys/stat.h>

#define MAX_PATH_CHARS 2047

// Define this macro so that stat() will use 64-bit inode.  (Prior to
// this change in BookMacster 1.12, we used the transitional function
// stat64(), which is now deprecated.
#define _DARWIN_USE_64_BIT_INODE 1

OSStatus DoSetPermissions(COMMAND_PROC_ARGUMENTS) {
#pragma unused (auth)
#pragma unused (userData)
	OSStatus retval = noErr;
	
	// Pre-conditions
    
    // userData may be NULL
	assert(request != NULL);
	assert(response != NULL);
    // asl may be NULL
    // aslMsg may be NULL
	
	// Get Info from arguments and assert that it's a CFDictionaryRef
	CFDictionaryRef infos = CFDictionaryGetValue(request, CFSTR(kInfos)) ;
	assert(infos != NULL) ;
	assert(CFGetTypeID(infos) == CFDictionaryGetTypeID()) ;
	
	CFIndex nPaths = CFDictionaryGetCount(infos) ;
	
	CFMutableDictionaryRef errorDescriptions = CFDictionaryCreateMutable(
																NULL,
																nPaths,
																&kCFTypeDictionaryKeyCallBacks,
																&kCFTypeDictionaryValueCallBacks
																) ;
	CFMutableDictionaryRef originalModes = CFDictionaryCreateMutable(
																	 NULL,
																	 nPaths,
																	 &kCFTypeDictionaryKeyCallBacks,
																	 &kCFTypeDictionaryValueCallBacks
																	 ) ;
	
	CFIndex i ;
	int nSucceeded = 0 ;
	
	const void ** paths = (const void **)malloc( nPaths * sizeof(const void *)) ;
	const void ** modeNums = (const void **)malloc( nPaths * sizeof(const void *)) ;
	if ((paths != NULL) && (modeNums != NULL)) {
		CFDictionaryGetKeysAndValues(
									 infos,
									 paths,
									 modeNums
									 )
		;
		
		for (i=0; i<nPaths; i++) {
			// Process each path
			Boolean ok = true ;
			int retval ;
			
			// Get path, assert that it's a string anc convert to a C string
			CFStringRef path = paths[i] ;
			assert(CFGetTypeID(path) == CFStringGetTypeID()) ;
			char pathC[MAX_PATH_CHARS] ;
			if (ok) {
				ok = CFStringGetCString(
										path,
										pathC,
										MAX_PATH_CHARS,
										kCFStringEncodingASCII
										) ;
				if (!ok) {
					CFDictionaryAddValue(errorDescriptions, path, CFSTR("Name too long")) ;
				}
			}
			
			// Read current permissions for path, wrap as CFNumber and add to results dictionary
			if (ok) {
				struct stat rawStats ;
				retval = stat(pathC, &rawStats) ;
				ok = (retval == 0) ;
				if (ok) {
					// rawStats.st_mode is of type mode_t which is an unsigned short.
					// Unfortunately, the available kCFNumberTypes don't have unsigned short.
					// And I have found that if I give CFNumberCreate an unsigned short and
					// tell it that it's an available kCFNumberType, which has more bits,
					// the undefined bits get encoded as garbage, changing the value.
					// First assigning the unsigned short to an int fixes it.
					int originalMode = rawStats.st_mode ;
					CFNumberRef fileModeCF =  CFNumberCreate(
															 NULL,
															 kCFNumberIntType,
															 &originalMode
															 ) ;
					CFDictionaryAddValue(
										 originalModes,
										 path,
										 fileModeCF) ;
					CFRelease(fileModeCF) ;
				}
				else {
					CFStringRef errString = CFStringCreateWithFormat(
																	 NULL,
																	 NULL,
																	 CFSTR("stat64 failed.  errno: %d"),
																	 errno
																	 ) ;
					CFDictionaryAddValue(errorDescriptions, path, errString) ;
					CFQRelease(errString) ;
					ok = false ;
				}
                
                // Get new fileMode from Info, assert that it's a CFNumberRef and
                // unwrap object to get value
                mode_t fileModeValue ;
                if (ok) {
                    CFNumberRef fileModeCF = modeNums[i] ;
                    assert(fileModeCF != NULL) ;
                    assert(CFGetTypeID(fileModeCF) == CFNumberGetTypeID()) ;
                    ok = CFNumberGetValue(
                                          fileModeCF,
                                          kCFNumberIntType,
                                          &fileModeValue
                                          ) ;
                }
                
                // Set the new file mode
                if (ok) {
                    retval = chmod(pathC, fileModeValue) ;
                    
                    asl_log(
                            asl,
                            aslMsg,
                            ASL_LEVEL_DEBUG,
                            "chmod returned %d setting %o for %s",
                            retval,
                            fileModeValue,
                            pathC
                            ) ;
                    
                    if (retval < 0) {
                        CFStringRef errString = CFStringCreateWithFormat(
                                                                         NULL,
                                                                         NULL,
                                                                         CFSTR("Setting Permissions failed with errno=%d"),
                                                                         errno
                                                                         ) ;
                        CFDictionaryAddValue(errorDescriptions, path, errString) ;
                        CFQRelease(errString) ;
                    }
                    else {
                        nSucceeded++ ;
                    }
                }
            }            
		}
	}
		
	// Add the original modes to the response
	CFDictionaryAddValue(response, CFSTR(kInfos), originalModes) ;
	CFRelease(originalModes) ;
    free(paths) ;
    free(modeNums) ;
	
	// Add the errorDescriptions to the response too
	CFDictionaryAddValue(response, CFSTR(kErrorDescriptions), errorDescriptions) ;
	CFRelease(errorDescriptions) ;
	
	asl_log(
			asl,
			aslMsg,
			ASL_LEVEL_DEBUG,
			"DoSetPermissions succeeded setting %d/%d requested paths",
			nSucceeded,
			(int)nPaths
			) ;
	
	// Return the number of paths on which setting permissions failed
	retval = (OSStatus)(nPaths - nSucceeded) ;
	
	return retval ;
}	