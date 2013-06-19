// Needed for any key constants used in the request/response dics:
#include "CPHTasks_Demo.h"

// Needed for COMMAND_PROC_ARGUMENTS:// Provides some handy utilities for dealing with CF (CoreFoundation).
// It may not be used in all tasks but is included so you don't lose it.
#include "MoreCFQ.h"


OSStatus DoSetAnyUserPrefs(COMMAND_PROC_ARGUMENTS) {
#pragma unused (auth)
#pragma unused (userData)
	OSStatus retval = noErr;
	
	// Pre-conditions
    
    // userData may be NULL
	assert(request != NULL);
	assert(response != NULL);
    // asl may be NULL
    // aslMsg may be NULL
	
	// Get bundleIdentifier to which prefs will be written and assert that it's a string
	CFStringRef bundleIdentifier = CFDictionaryGetValue(request, CFSTR(kBundleIdentifier)) ;
	assert(bundleIdentifier != NULL) ;
	assert(CFGetTypeID(bundleIdentifier) == CFStringGetTypeID()) ;

	// Get payload and assert that it's a dictionary
	CFDictionaryRef payload = CFDictionaryGetValue(request, CFSTR(kDictionaryPayload)) ;
	assert(payload != NULL) ;
	assert(CFGetTypeID(payload) == CFDictionaryGetTypeID()) ;
	
	// Get count, then keys and values as separate arrays
	// (Now is when you ^really^ wish you had Cocoa and -keyEnumerator !)
	CFIndex nKeys = CFDictionaryGetCount(payload) ;
	CFArrayRef keys = NULL ;
	CFArrayRef values = NULL ;
	CFQArrayCreateWithDictionaryKeys(payload, &keys) ;
	CFQArrayCreateWithDictionaryValues(payload, &values) ;

	// Write keys and values to AnyUser prefs 
	CFIndex i ;
	for (i=0; i<nKeys; i++) {
		CFPropertyListRef key = CFArrayGetValueAtIndex(keys, i) ; 
		CFPropertyListRef value = CFArrayGetValueAtIndex(values, i) ; 
		
		// If value is our null placeholder (an empty NSData),
		// change it back to a real NULL.
		// We used an empty NSData to symbolize nil because
		// NSNull is not serializable.
		if (CFGetTypeID((CFDictionaryRef)value) == CFDataGetTypeID()) {
			if (CFDataGetLength((CFDataRef)value) == 0) {
				// Tell CFPreferencesSetValue to remove key from preferences.
				value = NULL ;
			}
		}
		
		// Set in prefs
		CFPreferencesSetValue (key,
							   value,
							   bundleIdentifier,
							   kCFPreferencesAnyUser,
							   kCFPreferencesCurrentHost
		) ;
	}
	// Actually, the above can be done without privileges. 
	// But it's not written to the disk yet.
	// Here's what we usually need admin privileges for...
	Boolean ok = CFPreferencesSynchronize(
										  bundleIdentifier,
										  kCFPreferencesAnyUser,
										  kCFPreferencesCurrentHost
										  ) ;
	if (!ok) {
		retval = permErr ;
	}
	
	asl_log(asl,
			aslMsg,
			ASL_LEVEL_DEBUG,
			"DoSetAnyUserPrefs result ok=%d for %s",
			ok,
			CFStringGetCStringPtr(
								  bundleIdentifier,
								  CFStringGetSystemEncoding()
								  )
			) ;
	
	// Clean up
	CFQRelease(keys) ;
	CFQRelease(values) ;
	
	return retval ;
}	
