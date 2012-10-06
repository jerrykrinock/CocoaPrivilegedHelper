#import <Cocoa/Cocoa.h>
#import "CPHHelpee.h"

@interface CPHDemoAppDelegate : NSObject <NSApplicationDelegate, CPHHelpee>
{
    IBOutlet NSTextView* textViewLog;
	IBOutlet NSTextField* textBundleID ;
	IBOutlet NSTextView* textToFile ;
	IBOutlet NSTextField* pathToFile ;
	IBOutlet NSTextField* sourcePath ;
	IBOutlet NSTextField* destinPath ;
    
	NSMutableDictionary* _anyUserPrefsDic ;
}


//@property (assign) IBOutlet NSWindow *window;

- (IBAction)doGetVersion:(id)sender;
- (IBAction)doGetUIDs:(id)sender;
- (IBAction)doOpenSomeLowNumberedPorts:(id)sender;
- (IBAction)doSetSystemPrefs:(id)sender ;
- (IBAction)doWriteTextToFile:(id)sender ;
- (IBAction)doCopyFile:(id)sender ;
- (IBAction)doInstallATool:(id)sender;

- (IBAction)recreateRights:(id)sender;
- (IBAction)removeRightsForAllCommands:(id)sender ;

@end
