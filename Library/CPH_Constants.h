#ifndef _COMMANDPROCARGUMENTS_H
#define _COMMANDPROCARGUMENTS_H


// A convenient constant for inserting the long list of arguments required in a BASCommandProc
// All the "command procs" that implement your tasks must have the same signature
// This is the list of arguments for the "command procs".
// (Backslash causes #define to continue onto next line)

// This argument signature is required by BetterAuthorizationSample.  Don't change it.

#define COMMAND_PROC_ARGUMENTS                \
AuthorizationRef			auth,     \
const void *                userData, \
CFDictionaryRef				request,  \
CFMutableDictionaryRef      response, \
aslclient                   asl,      \
aslmsg                      aslMsg


// CHANGE THIS LINE to use your codesign cert
#define kSigningCertCommonName "Developer ID Application: Jerry Krinock"

#define CPHTaskmasterInitialErrorCode                        251000
#define CPHTaskmasterNoCommandSpecsCantRemoveRightsErrorCode 251001
#define CPHTaskmasterErrorRemovingRightErrorCode             251002
#define CPHTaskmasterRequestIsNilErrorCode                   251003
#define CPHTaskmasterNoCommandSpecsCantExecuteErrorCode      251004
#define CPHTaskmasterNoCommandSpecsErrorCode                 251005
#define CPHTaskmasterNilHelperToolNameErrorCode              251006
#define CPHTaskmasterHelperReturnedError_ErrorCode           251007
#define CPHTaskmasterHelperReturnedNoData_ErrorCode          251008
#define CPHTaskmasterRequestExecutionFailed_ErrorCode        251009
#define CPHTaskmasterSomePartsFailed_ErrorCode               251010
#define CPHTaskmasterBASExecuteRequestFailed_ErrorCode       251011
#define CPHTaskmasterErrorFromResponse_ErrorCode             251012
#define CPHTaskmasterNoPayload_ErrorCode                     251013
#define CPHTaskmasterNoBundleIdentifier_ErrorCode            251014
#define CPHTaskmasterPathTooLong_ErrorCode                   251015
#define CPHTaskmasterMissingResource_ErrorCode               251016
#define CPHTaskmasterFailedAutorizationCreate_ErrorCode      251017
#define CPHTaskmasterFailedStaticCodeCheckValidity           251018
#define CPHTaskmasterBlessingFailed                          251019
#endif