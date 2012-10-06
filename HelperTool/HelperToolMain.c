#include "BetterAuthorizationSampleLib.h"
#include "SSY_ASL.h"

extern const BASCommandProc kCommandProcs[] ;
extern const BASCommandSpec kCommandSpecs[] ;

int main(int argc, char **argv)
{
#pragma unused (argc)
#pragma unused (argv)
    // Go directly into BetterAuthorizationSampleLib code.
	// It will call back to one of our procedures given in the command specs.

	int retVal = BASHelperToolMain(kCommandSpecs, kCommandProcs);
    return retVal ;
    
    // IMPORTANT: BASHelperToolMain doesn't clean up after itself,
	// so once it returns we must quit.
}
