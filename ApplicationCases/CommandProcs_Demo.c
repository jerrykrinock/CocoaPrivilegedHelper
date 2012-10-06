#include "BetterAuthorizationSampleLib.h"
#include "CPHTasks_Demo.h"

/*
 This array must have the same number of elements as kCommandSpecs, and
 corresponding elements must refer to the same tasks.

 Each element in the array must be a function name taken from a HelperToolTaskXXXX.c file.
 
 The array must end with a NULL termination sentinel.
*/
const BASCommandProc kCommandProcs[] = {
    DoGetVersion,
    DoGetUID,
    DoOpenSomeLowNumberedPorts,
	DoSetAnyUserPrefs,
	DoWriteDataToFile,
    DoInstallATool,
    DoCopyPaths,
    NULL  // array-termination sentinel
} ;
