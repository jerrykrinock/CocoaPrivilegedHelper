#include "BetterAuthorizationSampleLib.h"

@protocol CPHHelpee

- (const BASCommandSpec*)cphCommandSpecs ;

@optional

- (NSString*)cphToolName ;
- (NSString*)cphInstallerToolName ;
- (NSString*)cphPrivilegedHelperLabel ;
- (NSString*)cphStringsFilename ;
- (NSString*)cphBundleIdentifier ;

@end