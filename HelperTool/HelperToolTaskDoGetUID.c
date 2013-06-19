// Needed for any key constants used in the request/response dics:
#include "CPHTasks_Demo.h"

// Needed for COMMAND_PROC_ARGUMENTS:// Provides some handy utilities for dealing with CF (CoreFoundation).
// It may not be used in all tasks but is included so you don't lose it.
#include "MoreCFQ.h"

// Needed in this particular task for geteuid() and getruid()
#include <unistd.h>


OSStatus DoGetUID(COMMAND_PROC_ARGUMENTS)
    // Implements the kGetUIDsCommand.  Gets the process's three UIDs and 
    // adds them to the response dictionary.
{	
#pragma unused (auth)
#pragma unused (userData)
	OSStatus					retval = noErr;
    int                         err;
	uid_t						euid;
	uid_t						ruid;
	CFNumberRef					values[2];
	long long					tmp;
	
	// Pre-conditions
	
	////////// Jerry assert(auth != NULL);
    // userData may be NULL
	assert(request != NULL);
	assert(response != NULL);
    // asl may be NULL
    // aslMsg may be NULL
	
    // Get the UIDs.
    
	euid = geteuid();
	ruid = getuid();
	
	err = asl_log(asl, aslMsg, ASL_LEVEL_DEBUG, "euid=%ld, ruid=%ld", (long) euid, (long) ruid);
    assert(err == 0);
	
    // Add them to the response.
    
	tmp = euid;
	values[0] = CFNumberCreate(NULL, kCFNumberLongLongType, &tmp);
	tmp = ruid;
	values[1] = CFNumberCreate(NULL, kCFNumberLongLongType, &tmp);
	
	if ( (values[0] == NULL) || (values[1] == NULL) ) {
		retval = coreFoundationUnknownErr;
    } else {
        CFDictionaryAddValue(response, CFSTR(kNumberRUID), values[0]);
        CFDictionaryAddValue(response, CFSTR(kNumberEUID), values[1]);
	}
	
	if (values[0] != NULL) {
		CFRelease(values[0]);
	}
	if (values[1] != NULL) {
		CFRelease(values[1]);
	}

	return retval;
}

