#include "BetterAuthorizationSampleLib.h"
#include "CPH_Constants"


// ********************************
// Constants for Each of your Tasks
// ********************************

// Descriptions of each task.

// "SetAnyUserPrefs" sets some preferences for some bundle ID in /Library/Preferences.


// Declare a "command proc" (function) for each task.  (These are defined in the Helper Tool.)

OSStatus DoSetAnyUserPrefs(COMMAND_PROC_ARGUMENTS) ;

// Declare a "command name" for each task.

#define kSetAnyUserPrefsCommand				"SetAnyUserPrefs"

// Declare a "right name" for each task.

#define	kSetAnyUserPrefsRightName	"com.sheepeysystems.SetAnyUserPrefs"


// ***********************************************************
// Optional Keys used to Pass Data to and from the Helper Tool
// ***********************************************************

/*
 These keys appear in the request and response dictionaries.
 You can re-use these in different tasks, or define your own as needed.
 These are defined without the "@" prefix so that they can be used in CFSTR()
 To use create NSStrings in Cocoa methods, add the "@" prefix.
 You may pass any ^serializable^ [1] CoreFoundation object
 (CFString, CFNumber, CFBoolean, CFDate, CFData, CFArray, and CFDictionary).
 If you wish to pass nonserializable objects you must archive them into a CFData.
 Wrapping nonserializable objects into a dictionary or array is not good enough.
 [1] http://developer.apple.com/documentation/CoreFoundation/Conceptual/CFPropertyLists/CFPropertyLists.html
*/ 
#define kDictionaryPayload	"Payload"			// CFDictionary of prefs to be set

#define kBundleIdentifier   "BundleIdentifier"
