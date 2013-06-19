// Needed for any key constants used in the request/response dics:
#include "CPHTasks_Demo.h"

// Needed for COMMAND_PROC_ARGUMENTS:// Provides some handy utilities for dealing with CF (CoreFoundation).
// It may not be used in all tasks but is included so you don't lose it.
#include "MoreCFQ.h"

// Needed in this particular task
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 A helper routine for DoOpenSomeLowNumberedPorts.  Opens a TCP port and 
stashes the resulting descriptor in descArray.
*/
static OSStatus OpenAndBindDescAndAppendToArray(
	uint16_t					port,               // in host byte order
	CFMutableArrayRef			descArray,
    aslclient                   asl,
    aslmsg                      aslMsg
) {
	OSStatus                    retval;
	int							err;
	int							desc;
	CFNumberRef					descNum;
	
	// Pre-conditions
	
	assert(port != 0);
	assert(descArray != NULL);
    // asl may be NULL
    // aslMsg may be NULL
	
	descNum = NULL;
	
    retval = noErr;
	desc = socket(AF_INET, SOCK_STREAM, 0);
    if (desc < 0) {
        retval = BASErrnoToOSStatus(errno);
    }
	if (retval == noErr) {
		descNum = CFNumberCreate(NULL, kCFNumberIntType, &desc);
		if (descNum == NULL) {
			retval = coreFoundationUnknownErr;
		}
	}
	if (retval == 0) {
		struct sockaddr_in addr;

		memset(&addr, 0, sizeof(addr));
		addr.sin_len    = sizeof(addr);
		addr.sin_family = AF_INET;
		addr.sin_port   = htons(port);
		
        static const int kOne = 1;

        err = setsockopt(desc, SOL_SOCKET, SO_REUSEADDR, (void *)&kOne, sizeof(kOne));
        if (err < 0) {
            retval = BASErrnoToOSStatus(errno);
        }

        if (retval == noErr) {
            err = bind(desc, (struct sockaddr *) &addr, sizeof(addr));
            if (err < 0) {
                retval = BASErrnoToOSStatus(errno);
            }
        }
	}
	if (retval == noErr) {
		CFArrayAppendValue(descArray, descNum);
	}
    if (retval == noErr) {
        err = asl_log(asl, aslMsg, ASL_LEVEL_DEBUG, "Opened port %u", (unsigned int) port);
    } else {
        errno = BASOSStatusToErrno(retval);                         // so that %m can pick it up
        err = asl_log(asl, aslMsg, ASL_LEVEL_ERR, "Failed to open port %u: %m", (unsigned int) port);
    }
    assert(err == 0);
	
	// Clean up.
	
	if ( (retval != noErr) && (desc != -1) ) {
		err = close(desc);
		assert(err == 0);
	}
	if (descNum != NULL) {
		CFRelease(descNum);
	}
	
	return retval;
}

OSStatus DoOpenSomeLowNumberedPorts(COMMAND_PROC_ARGUMENTS) {	
#pragma unused (auth)
#pragma unused (userData)
	OSStatus					retval = noErr;
	CFMutableArrayRef			descArray = NULL;
	
	// Pre-conditions
    
    // userData may be NULL
	assert(request != NULL);
	assert(response != NULL);
    // asl may be NULL
    // aslMsg may be NULL
	
	descArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (descArray == NULL) {
		retval = coreFoundationUnknownErr;
	}
	
	if (retval == noErr) {
		retval = OpenAndBindDescAndAppendToArray(130, descArray, asl, aslMsg);
	}
	if (retval == noErr) {
		retval = OpenAndBindDescAndAppendToArray(131, descArray, asl, aslMsg);
	}
	if (retval == noErr) {
		retval = OpenAndBindDescAndAppendToArray(132, descArray, asl, aslMsg);
	}
	 
	if (retval == noErr) {
        CFDictionaryAddValue(response, CFSTR(kBASDescriptorArrayKey), descArray);
	}
	
    // Clean up.
    
	if (retval != noErr) {
		BASCloseDescriptorArray(descArray);
	}
	if (descArray != NULL) {
		CFRelease(descArray);
	}
	
	return retval;
}
