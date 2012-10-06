#include "BetterAuthorizationSampleLib.h"
#include "CPH_Constants.h"


// ********************************
// Constants for Each of your Tasks
// ********************************

// Descriptions of each task.

// "SetAnyUserPrefs" sets some preferences for some bundle ID in /Library/Preferences.
// "CopyPaths" copies an array of files to new paths.  Either the original files or
//     the new files, and/or their parent directories, may require elevated privilges.
// "SetPermissions" executes chmod(2) on a given path.

// Declare a "command proc" (function) for each task.  (These are defined in the Helper Tool.)

OSStatus DoSetAnyUserPrefs(COMMAND_PROC_ARGUMENTS) ;
OSStatus DoCopyPaths(COMMAND_PROC_ARGUMENTS) ;
OSStatus DoStatPaths(COMMAND_PROC_ARGUMENTS) ;
OSStatus DoSetPermissions(COMMAND_PROC_ARGUMENTS) ;

// Declare a "command name" for each task.

#define kSetAnyUserPrefsCommand				"SetAnyUserPrefs"
#define kCopyPathsCommand                   "CopyPaths"
#define kStatPathsCommand                   "StatPaths"
#define kSetPermissionsCommand              "SetPermissions"

// Declare a "right name" for each task.

#define	kSetAnyUserPrefsRightName	"com.sheepsystems.SetAnyUserPrefs"
#define	kCopyPathsRightName	        "com.sheepsystems.CopyPaths"
#define	kStatPathsRightName	        "com.sheepsystems.StatPaths"
#define kSetPermissionsRightName    "com.sheepsystems.SetPermissions"

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
#define kDictionaryPayload	"Payload"			  // CFDictionary of prefs to be set
#define kInfos              "Infos"               // CFArray of information for commands 
#define kSourcePath         "SourcePath"          // In a CopyPaths Info dictionary, the source path
#define kDestinPath         "DestinPath"          // In a CopyPaths Info dictionary, the destin path, or an empty string to delete (unlink) the SourcePath 
#define kErrorDescriptions  "Error Descriptions"  // CFDictionary of paths with values = error descriptions.
#define kErrors             "Errors"              // CFDictionary with keys = paths and values = error dictionaries.  See below.
#define kErrorCodes         "Error Codes"         // CFDictionary of paths with values = error codes.
											      //     Values -3 < error code < 102 are unix errno values
#define kStatDatas          "StatDatas"           // CFDictionary of CFData objects of stat structs
#define kBundleIdentifier   "BundleIdentifier"    // string
// Keys used in error dictionaries:
#define kFailedFunction     "Failed Function"     // string
#define kReturnValue        "Return Value"        // number
#define kErrno              "Errno"               // number
#define kMoreInfo           "More Info"           // string (optional)

