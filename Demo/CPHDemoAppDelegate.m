#import <Cocoa/Cocoa.h>

#include <unistd.h>
#include <netinet/in.h>

#import <Security/Security.h>
#import <Security/Authorization.h>
#import <Security/Security.h>
#import <Security/SecCertificate.h>
#import <Security/SecCode.h>
#import <Security/SecStaticCode.h>
#import <Security/SecCodeHost.h>
#import <Security/SecRequirement.h>

#import "CPHDemoAppDelegate.h"
#import "CPHTaskmaster+InstallATool.h"
#import "CPH_Constants.h"
#import "SSYDictionaryEntry.h"
#import "CPHTaskmaster.h"
#import "CPHTaskmaster+DemoTasks.h"
#import "CPHTaskmaster+CopyPaths.h"

extern const BASCommandSpec kCommandSpecs[] ;

@interface NSTextView (LogError)

@end

@implementation NSTextView (LogError)

- (void)logError:(NSError*)error {
    do {
        NSString* text ;
        
        text =  [NSString stringWithFormat:@"Error %ld in %@: ",
                 (long)[error code],
                 [error domain]] ;
        [self insertText:text] ;
        
        text = [error localizedDescription] ;
        if (!text) {
            text = @" Sorry, no error description" ;
        }
        [self insertText:text] ;
        [self insertText:@"\n"] ;
        
        error = [[error userInfo] objectForKey:NSUnderlyingErrorKey] ;
        if (error) {
            [self insertText:@"But there was an underlying error \u2026"] ;
        }
    } while (error != nil) ;
}

@end


@implementation CPHDemoAppDelegate

- (const BASCommandSpec*)cphCommandSpecs {
	return kCommandSpecs ;
}

- (NSString*)cphPrivilegedHelperLabel {
    return @"com.sheepsystems.CocoaPrivilegedHelper" ;
}


// Called when the user clicks the "GetVersion" button.  This is the simplest
// possible BetterAuthorizationSample operation, in that it doesn't handle any failures.
- (IBAction)doGetVersion:(id)sender {
#pragma unused (sender)
	NSError* error = nil ;
	NSString* version ;
	BOOL ok = [[CPHTaskmaster sharedTaskmaster] getVersion_p:&version
                                                               error_p:&error] ;
	
	// Log our results.
    if (ok == YES) {
        [textViewLog insertText:[NSString stringWithFormat:@"version = %@\n", version]] ;
    }
	else {
        [textViewLog logError:error] ;
    }
}

// Called when the user clicks the "GetUIDs" button.  This is a typical BetterAuthorizationSample
// privileged operation implemented in Objective-C.
- (IBAction)doGetUIDs:(id)sender {
#pragma unused (sender)
	NSError *error = nil ;
	NSString *ruid, *euid ;
	
    BOOL ok = [[CPHTaskmaster sharedTaskmaster] getRUID:&ruid
                                                             EUID:&euid
                                                          error_p:&error] ;
	
    // Log our results.
    
    if (ok == YES) {
        [textViewLog insertText:[NSString stringWithFormat:@"RUID = %@, EUID=%@\n", ruid, euid]] ;
	}
	else {
        [textViewLog logError:error] ;
	}
}

// Calls helper to open three low-numbered ports (which can't otherwise by opened by
// non-privileged code).
- (IBAction)doOpenSomeLowNumberedPorts:(id)sender {
#pragma unused (sender)
	NSError *error = nil ;
	NSArray *lowNumberedPorts ;
	
    BOOL ok = [[CPHTaskmaster sharedTaskmaster] openSomeLowPortNumbers_p:&lowNumberedPorts
                                                                           error_p:&error] ;
	
    // Log our results.
    if (ok == YES) {
        [textViewLog insertText:
		 [NSString stringWithFormat:@"ports[0] = %d, port[1] = %d, port[2] = %d\n",
		  [[lowNumberedPorts objectAtIndex:0] intValue],
		  [[lowNumberedPorts objectAtIndex:1] intValue],
		  [[lowNumberedPorts objectAtIndex:2] intValue]]];
	}
	else {
        [textViewLog logError:error] ;
	}
}

- (NSMutableDictionary *)anyUserPrefsDic {
    if (_anyUserPrefsDic == nil) {
		_anyUserPrefsDic = [[NSMutableDictionary alloc] init] ;
	}
	
	return _anyUserPrefsDic ;
}

- (void)setAnyUserPrefsDic:(NSMutableDictionary *)value {
    if (_anyUserPrefsDic != value) {
        [_anyUserPrefsDic release];
        _anyUserPrefsDic = [value retain];
    }
}

- (IBAction)doSetSystemPrefs:(id)sender {
#pragma unused (sender)
	NSError *error = nil ;
	NSMutableDictionary *anyUserPrefsDic = [self anyUserPrefsDic] ;
	
    BOOL ok = [[CPHTaskmaster sharedTaskmaster] setAnyUserPrefs:anyUserPrefsDic
                                                       inBundleIdentifier:[textBundleID stringValue]
                                                                  error_p:&error] ;
	if (ok == YES) {
        [textViewLog insertText:@"No error writing system prefs\n"];
	}
	else {
        [textViewLog logError:error] ;
	}
	
}

- (IBAction)doWriteTextToFile:(id)sender {
#pragma unused (sender)
	NSError *error = nil ;
	NSString *text = [textToFile string] ;
	NSString *filePath = [pathToFile stringValue] ;
	
	NSData *data = [text dataUsingEncoding:NSUTF8StringEncoding] ;
	
    BOOL ok = [[CPHTaskmaster sharedTaskmaster] writeData:data
															 toFile:filePath
															error_p:&error] ;
	if (ok == YES) {
        [textViewLog insertText:@"No error writing data to file\n"];
	}
}

- (IBAction)doCopyFile:(id)sender
{
#pragma unused (sender)
	NSError *error = nil ;
	NSString *source = [NSHomeDirectory() stringByAppendingPathComponent:[sourcePath stringValue]] ;
	NSString *destin = [destinPath stringValue] ;

    BOOL ok = [[CPHTaskmaster sharedTaskmaster] copyPath:source
                                                  toPath:destin
                                                 error_p:&error] ;
	if (ok == YES) {
        [textViewLog insertText:@"No error copying file\n"];
	}
    else {
        NSString* text = [NSString stringWithFormat:
                          @"Copying file resulted in error: %@", error] ;
        [textViewLog insertText:text] ;
    }
}

- (IBAction)doInstallATool:(id)sender
{
    NSError* error = nil ;
    NSString* toolName = @"cph-hellotool" ;
    NSString* destinationDirectory = @"/usr/local/bin" ;
    NSString* text ;

    BOOL ok = YES ;
        ok = [[CPHTaskmaster sharedTaskmaster] installAToolFromResourceName:toolName
                                                       destinationDirectory:destinationDirectory
                                                                    error_p:&error] ;
       if (ok) {
            text = [NSString stringWithFormat:
                    @"Tool named '%@' was installed in %@",
                    toolName,
                    destinationDirectory] ;
        }
        else {
            text = [error localizedDescription] ;
        }
    
    [textViewLog insertText:text] ;
    [textViewLog insertText:@"\n"] ;
}

- (IBAction)recreateRights:(id)sender {
#pragma unused (sender)
	[[CPHTaskmaster sharedTaskmaster] recreateAuthorization] ;
}

- (IBAction)removeRightsForAllCommands:(id)sender {
#pragma unused (sender)
	int i=0 ;
	while (kCommandSpecs[i].commandName != NULL) {
		// Not all commands have rightName, but all commands have commandName
		char *rightName = (char*)kCommandSpecs[i].rightName ;
		[[CPHTaskmaster sharedTaskmaster] removeRight:rightName] ;
		[textViewLog insertText:[NSString stringWithFormat:@"Removed right for %s\n", rightName]] ;
		i++ ;
	}
}

-(id)init {
	if ((self = [super init])) {
		// This has nothing to do with demo-ing authorization.  It is only used in
		// binding the Prefs Key/Value table in the window to our _anyUserPrefsDic ivar.
		// Just something for the Cocoa geeks to enjoy.
		id transformer = [[DicToReadableValuesArray alloc] init] ;
		[NSValueTransformer setValueTransformer:transformer
										forName:@"DicToReadableValuesArrayTransformer"] ;
		[transformer release] ;
	}
	
	return self;
}

- (void)dealloc {
	[_anyUserPrefsDic release] ;
	
	[super dealloc] ;
}



@end
