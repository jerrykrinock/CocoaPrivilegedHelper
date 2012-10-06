#include "CPHTasks_SetAnyUserPrefs.h"


/*
 The kCommandSpecs array is used by both the app and the helper tool to define
 the set of supported commands to the BetterAuthorizationSampleLib module.
 
 The kCommandProcs array is used by the helper tool only.
 
 It is important that the two arrays kCommandSpecs and kCommandProcs
 have the same number of elements and that corresponding elements refer to the same tasks.
*/


const BASCommandSpec kCommandSpecs[] = {
    {	kSetAnyUserPrefsCommand,			// commandName
        kSetAnyUserPrefsRightName,			// rightName
        "default",                          // rightDefaultRule    -- by default, you have to have admin credentials (see the "default" rule in the authorization policy database, currently "/etc/authorization")
        NULL,								// rightDescriptionKey -- key for custom prompt in .strings resource
        NULL                                // userData
	},
	
    // Use null termination so that we can loop through this array without
	// having to know its count.
	{	NULL,
        NULL,
        NULL,
        NULL,
        NULL
	}
};
