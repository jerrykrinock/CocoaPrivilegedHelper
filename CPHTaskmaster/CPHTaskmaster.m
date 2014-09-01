#import <Cocoa/Cocoa.h>
#import "CPHTaskmaster.h"
#import "CPHHelpee.h"

#include "CPH_Constants.h"
#include "BetterAuthorizationSampleLib.h"
#import <ServiceManagement/ServiceManagement.h>

static CPHTaskmaster* sharedTaskmaster = nil ;

NSString* const SSYConstKeyOSStatusErrorCode = @"OSStatus" ;
NSString* const CPHTaskmasterErrorDomain = @"CPHTaskmasterErrorDomain" ;

const BASCommandSpec nullCommandSpecs[] = {				
	// Only the null termination, just to keep from crashing.
	{	NULL,
		NULL, 
		NULL, 
		NULL,
		NULL
	}
} ;



@implementation CPHTaskmaster

@synthesize privilegedHelperLabel = m_privilegedHelperLabel ;
@synthesize stringsFilename = m_stringsFilename ;
@synthesize commandSpecs = m_commandSpecs ;

+ (NSError*)errorWithOSStatusErrorCode:(NSInteger)code {
	NSString* domain ;
	domain = NSOSStatusErrorDomain ;
	const char* cString = GetMacOSStatusCommentString((OSStatus)code) ;
    NSString* desc = [NSString stringWithCString:cString
                                        encoding:NSUTF8StringEncoding] ;
    NSDictionary* userInfo = [NSDictionary dictionaryWithObjectsAndKeys:
                              desc, NSLocalizedDescriptionKey,
                              nil] ;
	NSError* error= [NSError errorWithDomain:domain
										code:code
									userInfo:userInfo] ;

	return error ;
}

- (void)setInstallerToolName:(NSString*)installerToolName_ {
	if (installerToolName != installerToolName_) {
        [installerToolName release];
        installerToolName = [installerToolName_ retain];
    }
}

- (NSString *)installerToolName {
    return installerToolName ;
}

- (BOOL)blessHelperWithLabel:(NSString*)label
                     error_p:(NSError**)error_p {
	BOOL ok = NO;
    NSError* error = nil ;
	AuthorizationItem authItem		= { kSMRightBlessPrivilegedHelper, 0, NULL, 0 };
	AuthorizationRights authRights	= { 1, &authItem };
	AuthorizationFlags flags		=
    kAuthorizationFlagDefaults				|
    kAuthorizationFlagInteractionAllowed	|
    kAuthorizationFlagPreAuthorize			|
    kAuthorizationFlagExtendRights;
    
	AuthorizationRef tempAuthRefForBlessing = NULL;
	
	/* Obtain the right to install privileged helper tools (kSMRightBlessPrivilegedHelper). */
	OSStatus status = AuthorizationCreate(
                                          &authRights,
                                          kAuthorizationEmptyEnvironment,
                                          flags,
                                          &tempAuthRefForBlessing);
	if (status != errAuthorizationSuccess) {
		ok = NO ;
        NSString* errorDesc = [NSString stringWithFormat:
                               @"Could not install helper, because failed to create AuthorizationRef for blessing, return code %ld",
                               (long)status] ;
        error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                    code:CPHTaskmasterFailedAutorizationCreate_ErrorCode
                                userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                          errorDesc, NSLocalizedDescriptionKey,
                                          label, @"Label",
                                          nil]] ;
	}
    else {
		/* This does all the work of verifying the helper tool against the application
		 * and vice-versa. Once verification has passed, the embedded launchd.plist
		 * is extracted and placed in /Library/LaunchDaemons and then loaded. The
		 * executable is placed in /Library/PrivilegedHelperTools.
		 */
		Boolean result = SMJobBless(
                            kSMDomainSystemLaunchd,
                            (CFStringRef)label,
                            tempAuthRefForBlessing,
                            (CFErrorRef *)&error) ;
        ok = (result == true) ;
        if (!ok) {
            NSDictionary* userInfo = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"Could not install helper, because SMJobBless failed.  Check the underlying error, and also the five requirements listed at the end of ServiceManagement.h", NSLocalizedDescriptionKey,
                                      label, @"Label",
                                      error, NSUnderlyingErrorKey,
                                      nil] ;
            error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                        code:CPHTaskmasterBlessingFailed
                                    userInfo:userInfo] ;
        }
	}
    
    AuthorizationFree(tempAuthRefForBlessing, kAuthorizationFlagDefaults);
    
    if (error && error_p) {
        *error_p = error ;
    }
    
	return ok ;
}

- (BOOL)checkInstallPrivilegedHelperToolError_p:(NSError**)error_p {
    BOOL ok = YES ;
    NSError* error = nil ;
    
    NSDictionary*	installedHelperJobData 	= (NSDictionary*)SMJobCopyDictionary(kSMDomainSystemLaunchd, (CFStringRef)[self privilegedHelperLabel]);
    BOOL needToInstall = YES;
    
    if (installedHelperJobData)
    {
        NSString* installedPath = [[installedHelperJobData objectForKey:@"ProgramArguments"] objectAtIndex:0];
        NSURL* installedPathURL = [NSURL fileURLWithPath:installedPath];
        
        NSDictionary* installedInfoPlist = (NSDictionary*)CFBundleCopyInfoDictionaryForURL( (CFURLRef)installedPathURL );
        NSString* installedBundleVersion = [installedInfoPlist objectForKey:@"CFBundleVersion"];
        [installedInfoPlist release] ;
        NSInteger installedVersion = [installedBundleVersion integerValue];
        
        NSBundle* appBundle	= [NSBundle mainBundle];
        NSURL* appBundleURL	= [appBundle bundleURL];
        
        NSURL* currentHelperToolURL	= [appBundleURL URLByAppendingPathComponent:[NSString stringWithFormat:@"Contents/Library/LaunchServices/%@", [self privilegedHelperLabel]]] ;
        
        NSDictionary* currentInfoPlist = (NSDictionary*)CFBundleCopyInfoDictionaryForURL( (CFURLRef)currentHelperToolURL );
        NSString* currentBundleVersion = [currentInfoPlist objectForKey:@"CFBundleVersion"];
 
        NSInteger currentVersion = [currentBundleVersion integerValue];
        [currentInfoPlist release] ;
        
        if ( currentVersion == installedVersion )
        {
            SecRequirementRef requirement;
            OSStatus stErr;
            
            stErr = SecRequirementCreateWithString(
                                                   (CFStringRef)[NSString stringWithFormat:@"identifier %@ and certificate leaf[subject.CN] = \"%@\"", [self privilegedHelperLabel], @kSigningCertCommonName],
                                                   kSecCSDefaultFlags,
                                                   &requirement
                                                   ) ;
            
            if ( stErr == noErr )
            {
                SecStaticCodeRef staticCodeRef;
                
                stErr = SecStaticCodeCreateWithPath(
                                                    (CFURLRef)installedPathURL,
                                                    kSecCSDefaultFlags,
                                                    &staticCodeRef
                                                    ) ;
                
                if ( stErr == noErr )
                {
                    SecStaticCodeCheckValidity( staticCodeRef, kSecCSDefaultFlags, requirement );
                    
                    needToInstall = NO;
                }

                // Memory leak fixed in BookMacster 1.17
                if (staticCodeRef != NULL) {
                    CFRelease(staticCodeRef) ;
                }
            }
            
            // Memory leak fixed in BookMacster 1.17
            if (requirement != NULL) {
                CFRelease(requirement) ;
            }

        }
    }
    [installedHelperJobData release] ;
    
    if (ok && needToInstall)
    {
        ok = [self blessHelperWithLabel:[self privilegedHelperLabel]
                                error_p:&error] ;
    }
    
    if (ok) {
        if (m_authorizationRef == NULL) {
            OSStatus    err;
            
            // If we don't do this, to assign _authRef before attempting a privileged task,
            // an assertion will burp out of BetterAuthorizationSampleLib.
            err = AuthorizationCreate(NULL, NULL, kAuthorizationFlagDefaults, &m_authorizationRef);
            if ((err != noErr) || (m_authorizationRef == NULL)) {
                ok = NO ;
                error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                            code:CPHTaskmasterFailedAutorizationCreate_ErrorCode
                                        userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                                  @"Can't install tool because failed authorization create", NSLocalizedDescriptionKey,
                                                  [NSNumber numberWithLong:(long)err], @"OSStatus",
                                                  nil]] ;
            }
            
            if (ok) {
                if ([self commandSpecs] == NULL) {
                    ok = NO ;
                    error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                                code:CPHTaskmasterNoCommandSpecsErrorCode
                                            userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                                      @"Can't install tool because no command specs", NSLocalizedDescriptionKey,
                                                      nil]] ;
                }
                
                NSString* bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier] ;
                if (bundleIdentifier == nil) {
                    ok = NO ;
                    error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                                code:CPHTaskmasterNoBundleIdentifier_ErrorCode
                                            userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                                      @"Can't install tool because no bundle ID", NSLocalizedDescriptionKey,
                                                      nil]] ;
                }
                
                BASSetDefaultRules(
                                   m_authorizationRef,
                                   [self commandSpecs],
                                   (CFStringRef)bundleIdentifier,
                                   (CFStringRef)[self stringsFilename]
                                   ) ;
            }
        }
    }
    
    if (error && error_p) {
        *error_p = error ;
    }
    
    return ok ;
}

- (id)initWithCommandSpecs:(const BASCommandSpec*)commandSpecs
		 installerToolName:(NSString*)installerToolName_
     privilegedHelperLabel:(NSString*)privilegedHelperLabel
		   stringsFilename:(NSString*)stringsFilename
		  bundleIdentifier:(NSString*)bundleIdentifier {
	self = [super init];
	if (self != nil) {
        if (commandSpecs == NULL) {
            NSLog(@"Internal Error 101-4019") ;
            [self release] ;
            return nil ;
		}
		[self setCommandSpecs:commandSpecs] ;

        if (privilegedHelperLabel == nil) {
            NSLog(@"Internal Error 101-4825") ;
            [self release] ;
            return nil ;
		}
		[self setPrivilegedHelperLabel:privilegedHelperLabel] ;

		if (installerToolName_ == nil) {
			installerToolName_ = @"CPHHelperToolInstaller" ;
		}
		[self setInstallerToolName:installerToolName_] ;
 
		if (stringsFilename == nil) {
			stringsFilename = @"Localizable" ;
		}
		[self setStringsFilename:stringsFilename] ;
	}
    
	return self ;
}


+ (CPHTaskmaster*)sharedTaskmaster {
	@synchronized(self) {
        if (sharedTaskmaster == nil) {
			// Get configuration parameters from NSApp's delegate, or assign defalts
			const BASCommandSpec* cphCommandSpecs = NULL ;
			if ([NSApp delegate]) {
				if ([[NSApp delegate] respondsToSelector:@selector(cphCommandSpecs)]) {
					cphCommandSpecs = [(NSObject <CPHHelpee> *)[NSApp delegate] cphCommandSpecs] ;
					
					NSString* cphInstallerToolName = nil ; // defaults to "CPHHelperToolInstaller"
					if ([[NSApp delegate] respondsToSelector:@selector(cphInstallerToolName)]) {
						cphInstallerToolName = [(NSObject <CPHHelpee> *)[NSApp delegate] cphInstallerToolName] ;
					}
					NSString* cphPrivilegedHelperLabel = nil ;  // No default.  Error if cphPrivilegedHelperLabel is nil
					if ([[NSApp delegate] respondsToSelector:@selector(cphPrivilegedHelperLabel)]) {
						cphPrivilegedHelperLabel = [(NSObject <CPHHelpee> *)[NSApp delegate] cphPrivilegedHelperLabel] ;
					}
					NSString* cphStringsFilename = nil ; // defaults to "Localizable"
					if ([[NSApp delegate] respondsToSelector:@selector(cphStringsFilename)]) {
						cphStringsFilename = [(NSObject <CPHHelpee> *)[NSApp delegate] cphStringsFilename] ;
					}
					NSString* cphBundleIdentifier = nil ; // defaults to main bundle
					if ([[NSApp delegate] respondsToSelector:@selector(cphBundleIdentifier)]) {
						cphBundleIdentifier = [(NSObject <CPHHelpee> *)[NSApp delegate] cphBundleIdentifier] ;
					}
					
					sharedTaskmaster = [[self alloc] initWithCommandSpecs:cphCommandSpecs
														installerToolName:cphInstallerToolName
                                                    privilegedHelperLabel:cphPrivilegedHelperLabel
														  stringsFilename:cphStringsFilename 
														 bundleIdentifier:cphBundleIdentifier] ;			
				}
				else {
					NSLog(@"Warning 340-1566.  NSApp's delegate does not implement cphCommandSpecs.") ;
					sharedTaskmaster = nil ;
				}
			}
			else {
				NSLog(@"Warning 340-1567.  CPHTaskmaster requires an app with a delegate.") ;
				sharedTaskmaster = nil ;
			}
		}
	}
    return sharedTaskmaster ;
}

- (void)dealloc {
	[installerToolName release] ;
    [m_privilegedHelperLabel release] ;
    [m_stringsFilename release] ;
    if (m_authorizationRef != NULL) {
        AuthorizationFree(m_authorizationRef, kAuthorizationFlagDefaults) ;
    }
	
	[super dealloc] ;
}

- (OSStatus)removeRight:(const char*)rightName {
	OSStatus result = paramErr ;
	if (rightName != NULL) {
		result = AuthorizationRightRemove(
										  m_authorizationRef,
										  rightName
										  ) ;
	}
	
	return result ;
}

- (BOOL)removeRightsError_p:(NSError**)error_p {
	if (!error_p) {
		NSError* error = nil ;
		error_p = &error ;
	}
	BOOL ok = YES ;

	// Make sure command has been configured
	if ([self commandSpecs] == NULL) {
		NSString* errMsg = @"No command specs!" ;
		int errCode = CPHTaskmasterNoCommandSpecsCantRemoveRightsErrorCode ;
		*error_p = [NSError errorWithDomain:@"CPHTaskmaster"
									   code:errCode
								   userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
											 [[self class] description], @"Object Class",
											 NSStringFromSelector(_cmd), @"Method",
											 errMsg, NSLocalizedDescriptionKey,
											 nil]] ;
		ok = NO ;
		goto end ;
	}
	
	int i = 0 ;
	while (YES) {
		BASCommandSpec commandSpec = ([self commandSpecs])[i] ;
		if (commandSpec.commandName == NULL) {
			// Hit the nil sentinel.  No more.
			break ;
		}
		
		// Get the rightName for the commandSpec at this index
		const char* rightName = commandSpec.rightName ;
		
		// Attempt to remove it
		OSStatus status = [self removeRight:rightName] ;
		
		// Handle error
		if (status != noErr) {
			NSError* error_ = [[self class] errorWithOSStatusErrorCode:status] ;
			NSString* errMsg = [NSString stringWithFormat:
					  @"Got Error attempting to remove right %@",
					  [NSString stringWithUTF8String:rightName]] ;
			int errCode = CPHTaskmasterErrorRemovingRightErrorCode ;
			*error_p = [NSError errorWithDomain:@"CPHTaskmaster"
										   code:errCode
									   userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
												 [[self class] description], @"Object Class",
												 NSStringFromSelector(_cmd), @"Method",
												 errMsg, NSLocalizedDescriptionKey,
												 error_, NSUnderlyingErrorKey,
												 nil]] ;
			ok = NO ;
			goto end ;
		}
		
		i++ ;
	}
	
end:
	return ok ;
}



- (void)recreateAuthorization {
    // Called when the user chooses the "Recreate" button.  This is just a testing 
    // convenience; it allows you to destroy the credentials that are stored in the cache 
    // associated with _authRef, so you can force the system to ask you for a password again.  
    // However, this isn't as convenient as you might think because the credentials might 
    // be cached globally.  See DTS Q&A 1277 "Security Credentials" for the gory details.
    //
    // <http://developer.apple.com/qa/qa2001/qa1277.html>
	OSStatus    err;
	
	// Free _authRef, destroying any credentials that it has acquired along the way. 
	
	err = AuthorizationFree(m_authorizationRef, kAuthorizationFlagDestroyRights);
	assert(err == noErr);
	m_authorizationRef = NULL;
	
	
	// Recreate it from scratch.
	
	err = AuthorizationCreate(
                              NULL,
                              NULL,
                              kAuthorizationFlagDefaults,
                              &m_authorizationRef) ;
	assert(err == noErr);
	assert( (err == noErr) == (m_authorizationRef != NULL) );    
}

- (BOOL)executeRequest:(NSDictionary*)request
			response_p:(NSDictionary**)response_p
			   error_p:(NSError**)error_p {
	BOOL ok = YES ;
    NSError* error = nil ;
    NSMutableDictionary* response = nil ;
    
	if (request == nil) {
        NSString* desc = @"Nil request" ;
        error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                    code:CPHTaskmasterRequestIsNilErrorCode
                                userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                          desc, NSLocalizedDescriptionKey,
                                          nil]] ;
		ok = NO ;
		goto end ;
	}
		
	if ([self commandSpecs] == NULL) {
        NSString* desc = @"Attempt to execute before commandSpecs have been set." ;
        error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                    code:CPHTaskmasterNoCommandSpecsCantExecuteErrorCode
                                userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                          desc, NSLocalizedDescriptionKey,
                                          nil]] ;
		ok = NO ;
		goto end ;
	}
	
//	NSString* bundleID = [[NSBundle mainBundle] bundleIdentifier] ;
	NSString* bundleID = [self privilegedHelperLabel] ;
	if (!bundleID) {
        NSString* desc = @"No bundle identifier" ;
        error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                    code:CPHTaskmasterNoBundleIdentifier_ErrorCode
                                userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                          desc, NSLocalizedDescriptionKey,
                                          nil]] ;
		ok = NO ;
		goto end ;
	}
	
	ok = [self checkInstallPrivilegedHelperToolError_p:&error] ;
    if (!ok) {
        goto end ;
    }
    
    
    OSStatus err = BASExecuteRequestInHelperTool(
                                        m_authorizationRef,
                                        [self commandSpecs],
                                        (CFStringRef)bundleID,
                                        (CFDictionaryRef)request,
                                        (CFDictionaryRef*)&response
                                        );
	
	// If all of the above went OK, it means that the IPC (inter-process communication)
	// to the helper tool, the low-level stuff, worked.  Move up to check higher
	// level, that is, check the response to see if the command's execution within 
	// the helper tool was successful.
	if (err == noErr) {
		err = BASGetErrorFromResponse((CFDictionaryRef)response) ;
        if (err != noErr) {
            ok = NO ;
            error = [[self class] errorWithOSStatusErrorCode:err] ;
            NSString* desc = [NSString stringWithFormat:
                              @"Privileged helper returned error code %ld.",
                              (long)err] ;
                
            error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                        code:CPHTaskmasterHelperReturnedError_ErrorCode
                                    userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                              desc, NSLocalizedDescriptionKey,
                                              request, @"Request",  // We did a goto end if request is nil
                                              error, NSUnderlyingErrorKey,  // may be nil sentinel
                                              nil]] ;
            goto end ;
        }
        else if (response == nil) {
            ok = NO ;
            NSString* desc = @"No data received from Privileged Helper Tool.\n" ;
            error = [NSError errorWithDomain:@"CPHTaskmaster"
                                        code:CPHTaskmasterHelperReturnedNoData_ErrorCode
                                    userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                              desc, NSLocalizedDescriptionKey,
                                              nil]] ;
        }
    }
    else {
        ok = NO ;
        NSString* desc = [NSString stringWithFormat:
                          @"BASExecuteRequestInHelperTool() returned error %ld",
                          (long)err] ;
        error = [NSError errorWithDomain:@"CPHTaskmaster"
                                    code:CPHTaskmasterBASExecuteRequestFailed_ErrorCode
                                userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                          desc, NSLocalizedDescriptionKey,
                                          nil]] ;
    }
	
end:
	// According to BASExecuteRequestInHelperTool, we must release response.
	// Also, it will be nil if the request fails.
	// But the invoker may need it.  So, we autorelease it.
	[response autorelease] ;
	
    if (response_p) {
        *response_p = response ;
    }
    
    if (error_p && error) {
        *error_p = error ;
    }
    
	return ok ;
}


@end