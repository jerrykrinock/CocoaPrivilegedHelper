#include "BetterAuthorizationSampleLib.h"
#include "CPH_Constants.h"


// ********************************
// Constants for Each of your Tasks
// ********************************

// Descriptions of each task.

// "GetVersion" gets a version number from the helper tool.
// "GetUIDs" gets the important process UIDs (RUID and EUID) of the helper tool.
// "OpenSomeLowNumberedPorts" opens some low-numbered ports on our behalf.
// "SetAnyUserPrefs" sets some preferences for some bundle ID in /Library/Preferences.
// "WriteDataToFile" writes data to a file that you may not have write access to.


// Declare a "command proc" (function) for each task.  (These are defined in the Helper Tool.)

OSStatus DoGetVersion(COMMAND_PROC_ARGUMENTS) ;
OSStatus DoGetUID(COMMAND_PROC_ARGUMENTS) ;
OSStatus DoOpenSomeLowNumberedPorts(COMMAND_PROC_ARGUMENTS) ;
OSStatus DoSetAnyUserPrefs(COMMAND_PROC_ARGUMENTS) ;
OSStatus DoWriteDataToFile(COMMAND_PROC_ARGUMENTS) ;
OSStatus DoInstallATool(COMMAND_PROC_ARGUMENTS) ;
OSStatus DoCopyPaths(COMMAND_PROC_ARGUMENTS) ;

// Declare a "command name" for each task.

#define kGetVersionCommand					"GetVersion"
#define kGetUIDsCommand						"GetUIDs"
#define kOpenSomeLowNumberedPortsCommand	"OpenSomeLowNumberedPorts"
#define kSetAnyUserPrefsCommand				"SetAnyUserPrefs"
#define kWriteDataToFileCommand				"WriteDataToFile"
#define kInstallAToolCommand                "InstallATool"
#define kCopyPathsCommand                   "CopyPaths"

// Declare a "right name" for each task.

// kGetVersionRightName				The trivial GetVersion does not require authorization, so we don't define a right for it.
#define	kUIDRightName				"com.sheepsystems.CPH_TasksInCocoa.GetUIDs"
#define	kLowNumberedPortsRightName	"com.sheepsystems.CPH_TasksInCocoa.OpenSomeLowNumberedPorts"
#define	kSetAnyUserPrefsRightName	"com.sheepsystems.CPH_TasksInCocoa.SetAnyUserPrefs"
#define	kWriteDataToFileRightName 	"com.sheepsystems.CPH_TasksInCocoa.WriteDataToFile"
#define kInstallAToolRightName      "com.sheepsystems.CPH_TasksInCocoa.InstallATool"
#define kCopyPathsRightName         "com.sheepsystems.CPH_TasksInCocoa.CopyPaths"

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
#define kNumberPayload		"Version"           // CFNumber
#define kNumberRUID			"RUID"              // CFNumber
#define kNumberEUID			"EUID"              // CFNumber
#define kDictionaryPayload	"Payload"			// CFDictionary of prefs to be set
#define kBundleIdentifier	"BundleIdentifier"  // name of .plist file in which to set prefs
#define kData				"Data"				// CFData to write
#define kPath				"Path"				// POSIX path to write the file to

