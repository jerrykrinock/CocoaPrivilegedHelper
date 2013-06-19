// Needed for any key constants used in the request/response dics:
#include "CPHTasks_Typicapp.h"

// Needed for COMMAND_PROC_ARGUMENTS:// Provides some handy utilities for dealing with CF (CoreFoundation).
// It may not be used in all tasks but is included so you don't lose it.
#include "MoreCFQ.h"

#include <copyfile.h>
#include <unistd.h>
#include "SSYUnixPathUtils.h"
#include "RecursiveChown.h"
CFDictionaryRef CreateErrorDictionary(
									  char* failedFunction,
									  int returnValue,
									  int daErrno,
									  char* moreInfo
									  ) ;

CFDictionaryRef CreateErrorDictionary(
									  char* failedFunction,
									  int returnValue,
									  int daErrno,
									  char* moreInfo
									  ) {
	// Create the dictionary
	CFMutableDictionaryRef error = CFDictionaryCreateMutable(
															 NULL,
															 4,
															 &kCFTypeDictionaryKeyCallBacks,
															 &kCFTypeDictionaryValueCallBacks
															 ) ;
	
	// Add the failedFunction key/value
	CFStringRef failedFunctionObject = CFStringCreateWithCString(
																 NULL,
																 failedFunction,
																 kCFStringEncodingASCII
																 ) ;
	CFDictionaryAddValue(error, CFSTR(kFailedFunction), failedFunctionObject) ;
	CFQRelease(failedFunctionObject) ;
	
	// Add the return value key/value
	CFNumberRef returnValueObject = CFNumberCreate(NULL, kCFNumberIntType, &returnValue) ;
	CFDictionaryAddValue(error, CFSTR(kReturnValue), returnValueObject) ;
	CFQRelease(returnValueObject) ;
	
	// Add the errno key/value
	CFNumberRef errnoObject = CFNumberCreate(NULL, kCFNumberIntType, &daErrno) ;
	CFDictionaryAddValue(error, CFSTR(kErrno), errnoObject) ;
	CFQRelease(errnoObject) ;
	
	// Add the moreInfo key/value, if any
	if (moreInfo != NULL) {
		CFStringRef moreInfoObject = CFStringCreateWithCString(
															   NULL,
															   moreInfo,
															   kCFStringEncodingASCII
															   ) ;
		CFDictionaryAddValue(error, CFSTR(kMoreInfo), moreInfoObject) ;
		CFQRelease(moreInfoObject) ;
	}
	
	return error ;
}

OSStatus DoCopyPaths(COMMAND_PROC_ARGUMENTS) {
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
	CFMutableDictionaryRef errors = CFDictionaryCreateMutable(
																NULL,
																nFiles,
																&kCFTypeDictionaryKeyCallBacks,
																&kCFTypeDictionaryValueCallBacks
	) ;
	for (i=0; i<nFiles; i++) {
		CFDictionaryRef info = CFArrayGetValueAtIndex(infos, i) ;
		assert(CFGetTypeID(info) == CFDictionaryGetTypeID()) ;
		
		CFStringRef sourcePath = CFDictionaryGetValue(
													  info,
													  CFSTR(kSourcePath)
													  ) ;
		assert(CFGetTypeID(sourcePath) == CFStringGetTypeID()) ;
		CFStringRef destinPath = CFDictionaryGetValue(
													  info,
													  CFSTR(kDestinPath)
													  ) ;
		assert(CFGetTypeID(destinPath) == CFStringGetTypeID()) ;

		Boolean ok = true ;
		int16_t result ;
		
		char sourcePathC[SSY_UNIX_PATH_UTILS_MAX_PATH_CHARS] ;
		if (ok) {
			ok = CFStringGetCString(
									sourcePath,
									sourcePathC,
									SSY_UNIX_PATH_UTILS_MAX_PATH_CHARS,
									kCFStringEncodingASCII
									) ;
			if (!ok) {
				CFDictionaryAddValue(response, sourcePath, CFSTR("Name too long")) ;
			}
		}
		
		char destinPathC[SSY_UNIX_PATH_UTILS_MAX_PATH_CHARS] ;
		if (ok) {
			ok = CFStringGetCString(
									destinPath,
									destinPathC,
									SSY_UNIX_PATH_UTILS_MAX_PATH_CHARS,
									kCFStringEncodingASCII
									) ;
			if (!ok) {
				CFDictionaryAddValue(response, destinPath, CFSTR("Name too long")) ;
			}
		}
		
		if (ok) {
			if (CFStringGetLength(destinPath) > 0) {
				// destinPath is a string
				if (ok) {
					copyfile_state_t state ;
					state = copyfile_state_alloc() ;
					result = copyfile(
									  sourcePathC,
									  destinPathC,
									  state,
									  COPYFILE_ALL + (1<<15)
									  ) ;
					// (1<<15) is defined as COPYFILE_RECURSIVE, but it is only available
					// in the Mac OS 10.6 SDK.  So I cheated.  If this function is run
					// under Mac OS 10.5, the (1<<15) is ignored and a nonrecursive
					// copy is performed.
					
					/* Experiment results : How copyfile(3) handles trailing slashes
					 If you put a trailing slash on the source path, it results in the
					 *contents* of the source directory being copied into the destination
					 directory.  Otherwise, it results in the source directory itself
					 being copied into the destination directory.  Trailing slash on the
					 destination path does not matter.
					 
					 In pictures, say that you have a source directory S containing a
					 file F, and want to copy this to a destination directory D.  
					 copyfile ("/S", "/D", s, COPYFILE_RECURSIVE)   ; // results in /D/S/F
					 copyfile ("/S", "/D/", s, COPYFILE_RECURSIVE)  ; // results in /D/S/F
					 copyfile ("/S/", "/D", s, COPYFILE_RECURSIVE)  ; // results in /D/F
					 copyfile ("/S/", "/D/", s, COPYFILE_RECURSIVE) ; // results in /D/F
					 */
					
					int logLevel = (result == 0) ? ASL_LEVEL_DEBUG : ASL_LEVEL_ERR ;
                    asl_log(
							asl,
							aslMsg,
							logLevel,
							"copyfile result=%d  errno=%d\nsource: %s\ndestin: %s",
							result,
							errno,
							sourcePathC,
							destinPathC
							) ;
					
					copyfile_state_free(state) ;
					
					if (result < 0) {
						CFDictionaryRef error = CreateErrorDictionary("copyfile", result, errno, NULL) ;
						CFDictionaryAddValue(errors, sourcePath, error) ;
						CFQRelease(error) ;
						ok = false ;
					}
				}
				
				uid_t destin_uid = 501 ;
				uid_t destin_gid = -1 ;  // Do not try and change the group ID.
				/*
				 Prior to BookMacster version 1.5.7, destin_gid was determined from
				 UnixOwnerIDs() like destin_uid.  However, I then found that on
				 MacMini1-jk, the owner of the Safari folder was 501, and this caused
				 the subsequent call to chown_recursive() to fail with result -1.
				 As a matter of fact, I can't even do this in Terminal with sudo, as
				 shown in this transcript…  Note that jk=501.
				 
				 MacMini2:Volumes jk$ sudo ls -al /Volumes/MacMini1-1/Users/jk/Library/ | grep Safari
				 Password:
				 drwxr-xr-x@  19 jk  501      602 Jun 11 17:29 Safari
				 MacMini2:Volumes jk$ sudo ls -al /Volumes/MacMini1-1/Users/jk/Library/Safari/Bookmarks.plist
				 -rw-r--r--  1 jk  staff  621 Jun 11 17:29 /Volumes/MacMini1-1/Users/jk/Library/Safari/Bookmarks.plist
				 MacMini2:Volumes jk$ sudo chown 501:501 /Volumes/MacMini1-1/Users/jk/Library/Safari/Bookmarks.plist
				 chown: /Volumes/MacMini1-1/Users/jk/Library/Safari/Bookmarks.plist: Operation not permitted
				 
				 You see all of the other folders in /Volumes/MacMini1-1/Users/jk/Library/
				 have gid = staff, but the Safari folder has gid=501.  Note that even with sudo,
				 the system will not let me change Bookmarks.plist gid to 501.
				 */
				if (ok) {
					result = UnixOwnerIDs(destinPathC, &destin_uid, NULL) ;
					if (result != 0) {
						char moreInfo[255] ;
						// Incorrect usage of sprintf crashed; fixed in version 18 of this tool (BookMacster 1.10.0)
						snprintf (moreInfo, 255, "destin_uid=%d  destinPath=%s", destin_uid, destinPathC) ;   
						CFDictionaryRef error = CreateErrorDictionary("UnixOwnerIDs", result, errno, moreInfo) ;
						CFDictionaryAddValue(errors, sourcePath, error) ;
						CFQRelease(error) ;
						ok = false ;
					}
				}
				
				if (ok) {
					result = chown_recursive(destinPathC, destin_uid, destin_gid) ;
					asl_log(
							asl,
							aslMsg,
							ASL_LEVEL_DEBUG,
							"chown result=%d setting [%d,%d] for %s",
							result,
							destin_uid,
							destin_gid,
							destinPathC
							) ;
					if (result != 0) {
						char moreInfo[255] ;
						// Incorrect usage of sprintf crashed; fixed in version 18 of this tool (BookMacster 1.10.0)
						snprintf (moreInfo, 255, "destin_uid=%d  destin_gid=%d  destinPath=%s", destin_uid, destin_gid, destinPathC) ;   
						CFDictionaryRef error = CreateErrorDictionary("chown", result, errno, moreInfo) ;
						CFDictionaryAddValue(errors, sourcePath, error) ;
						CFQRelease(error) ;
						ok = false ;
					}			
				}
			}
			else {
				// destinPath is an empty string, meaning we need to delete sourcePath
				int result = unlink(sourcePathC) ;
				if (result < 0) {
					CFDictionaryRef error = CreateErrorDictionary("unlink", result, errno, NULL) ;
					CFDictionaryAddValue(errors, sourcePath, error) ;
					CFQRelease(error) ;
					ok = false ;
				}
			}
		}
			
		if (ok) {
			nSucceeded++ ;
		}		
	}
		
	// Since the COMMAND_PROC_ARGUMENTS give us by BetterAuthorizationSample
	// does not have an explicit vehicle for returning error details, we
	// add our 'errors' dictionary to the 'response' dictionary.
	CFDictionaryAddValue(response, CFSTR(kErrors), errors) ;
	CFRelease(errors) ;
	
	asl_log(
			asl,
			aslMsg,
			ASL_LEVEL_DEBUG,
			"DoCopyPaths success=%d/%d requested files",
			nSucceeded,
			(int)nFiles
			) ;
	
	// Return the number of file copy operations that failed
	retval = (OSStatus)(nFiles - nSucceeded) ;
	
	return retval ;
}	