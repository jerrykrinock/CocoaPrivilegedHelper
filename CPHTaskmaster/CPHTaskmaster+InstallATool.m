#import "CPHTaskmaster+InstallATool.h"
#import "CPHTasks_InstallATool.h"
#import <ServiceManagement/ServiceManagement.h>

@implementation CPHTaskmaster (InstallAToolTask)

- (BOOL)installAToolFromPath:(NSString*)srcPath
                      toPath:(NSString*)destinationDirectory
                   error_p:(NSError**)error_p {
    BOOL ok = YES ;
    NSError*        error = nil ;
    OSStatus        err;
    NSString *      bundleID;
    NSDictionary *  request;
    CFDictionaryRef response;
    
    response = NULL;
    
    request = [NSDictionary dictionaryWithObjectsAndKeys:
               @kInstallAToolCommand, @kBASCommandKey,
               srcPath, @kInstallAToolSrcPath,
               destinationDirectory, @kInstallAToolDestinationDirectory,
               [srcPath lastPathComponent], @kInstallAToolName, nil];
    assert(request != NULL);
    
    bundleID = [self privilegedHelperLabel] ;
    
    // Execute it.
    
    AuthorizationRef auth;
    if (AuthorizationCreate(NULL, NULL, kAuthorizationFlagDefaults, &auth)) {
        ok = NO ;
    }

	err = BASExecuteRequestInHelperTool(
                                        auth,
                                        kCommandSpecs,
                                        (CFStringRef) bundleID,
                                        (CFDictionaryRef) request,
                                        &response
                                        );
    
    // If the above went OK, it means that the IPC to the helper tool worked.  We
    // now have to check the response dictionary to see if the command's execution
    // within the helper tool was successful.
    
    NSString* errorDesc = nil ;
    NSInteger errorCode = 0 ;
    if (err == noErr) {
        err = BASGetErrorFromResponse(response);
        if (err != noErr) {
            errorCode = CPHTaskmasterErrorFromResponse_ErrorCode ;
            errorDesc = @"Executed OK but error in response" ;
        }
    }
    else {
        ok = NO ;
        errorCode = CPHTaskmasterBASExecuteRequestFailed_ErrorCode;
        errorDesc = @"Error executing request" ;
    }
    if (errorCode != 0) {
        ok = NO ;
        error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                    code:errorCode
                                userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                          errorDesc, NSLocalizedDescriptionKey,
                                          nil]] ;
    }
    
    if (response) {
        CFRelease(response);
    }
    
    if (error_p && error) {
        *error_p = error ;
    }
    
    return ok ;
}

- (BOOL)installAToolFromResourceName:(NSString*)resourceName
                destinationDirectory:(NSString*)destinationDirectory
                             error_p:(NSError**)error_p {
    BOOL ok = YES ;
    NSError* error = nil ;
    
    ok = [self checkInstallPrivilegedHelperToolError_p:&error] ;

    AuthorizationRef auth = NULL ;
    if (ok) {
        if (AuthorizationCreate(NULL, NULL, kAuthorizationFlagDefaults, &auth)) {
            ok = NO ;
        }
    }
    
    if (ok) {
        BASSetDefaultRules(auth,
                           kCommandSpecs,
                           CFBundleGetIdentifier(CFBundleGetMainBundle()),
                           NULL); // No separate strings file, use Localizable.strings
        
        NSBundle* bundle = [NSBundle mainBundle] ;
        NSString* toolPath = [bundle pathForResource:resourceName
                                                             ofType:@""];
        
        if (toolPath) {
            ok = [self installAToolFromPath:toolPath
                                     toPath:destinationDirectory
                                    error_p:&error] ;
        }
        else {
            ok = NO ;
            NSString* desc = [NSString stringWithFormat:
                    @"Error: Tool named '%@' is apparently missing from this app's Resources.  "
                    @"Check the bundle %@.",
                    resourceName,
                    bundle] ;
            error = [NSError errorWithDomain:CPHTaskmasterErrorDomain
                                        code:CPHTaskmasterMissingResource_ErrorCode
                                    userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                                              desc, NSLocalizedDescriptionKey,
                                              nil]] ;
        }
        
    }

    if (auth) {
        AuthorizationFree(auth, kAuthorizationFlagDefaults) ;
    }

    if (error && error_p) {
        *error_p = error ;
    }
    
    return ok ;
}

@end



