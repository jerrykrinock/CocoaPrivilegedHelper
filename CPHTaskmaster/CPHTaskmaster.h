#import <Cocoa/Cocoa.h>
#import <Security/Authorization.h>
#include "BetterAuthorizationSampleLib.h"

extern NSString* const CPHTaskmasterErrorDomain ;

/*!
 @brief     A class for accessing a privileged Helper Tool to perform authorized
 tasks WHICH HAS BECOME INCREASINGLY PROBLEMATIC IN RECENT VERSIONS OF OS X

 @details   WARNING:  This class has been giving me trouble in recent verions
 of OS X, in particular I've been getting CPHTaskmasterBlessingFailed errors
 when attempting to install the helper tool.  Because I deemed the feature
 using this class to be not worth fixing, I've stopped using it.  If you want
 to use this class, you'll need to study Apple's latest security model and
 the current requirements of SMJobBless.  I also remember seeing that there is
 now a successor to BetterAuthorizationSample, but can't find it now.  I think
 that Apple needs to update TN2095.  -- JK, 2014 09 01
 
 Designed to instantiated as a singleton, but you could have more than
 one if you really wanted to.
 */
@interface CPHTaskmaster : NSObject {
	AuthorizationRef m_authorizationRef ;
	// We can't retain the C array _commandSpecs, but by
	// requiring it to be const we insure that it won't go away.
	const BASCommandSpec* m_commandSpecs ;
	NSString* installerToolName ;
    NSString* m_privilegedHelperLabel ;
    NSString* m_stringsFilename ;
}

@property (copy) NSString* privilegedHelperLabel ;
@property (copy) NSString* stringsFilename ;
@property (assign) const BASCommandSpec* commandSpecs ;

/*!
 @brief    Installs our privileged helper tool, if current version is not
 already installed
 
 @details   This uses SMJobBless to install a tool in /Library/PrivilegedHelperTools which is
 run by launchd instead of us, with elevated privileges. This can then be used to
 perform privilieged tasks for us.
 
 We do this rather than AuthorizationExecuteWithPrivileges because that's deprecated in 10.7
 The SMJobBless approach is more secure because both ends are validated via code signing
 which is enforced by launchd - ie only tools signed with the right cert can be installed, and
 only apps signed with the right cert can install it.
 
 Although the launchd approach is primarily associated with daemons, it can be used for one-off
 tools too. We effectively invoke the privileged helper by talking to it over a private Unix socket
 (since we can't launch it directly). We still need to be careful about that invocation because
 the SMJobBless structure doesn't validate that the caller at runtime is the right application.
 
 */
- (BOOL)checkInstallPrivilegedHelperToolError_p:(NSError**)error_p ;

/*!
 @brief    To get and/or create a CPHTaskmaster singleton for an application.
 @details  Recommended unless you have > 1 authorized helper tool in an application.&nbsp; 
 The sharedTaskmaster singleton is initialized with value(s) returned by [NSApp delegate] if
 it responds to any of the optional methods in the CPHHelpee protocol.&nbsp;  
 Otherwise, the default values given in the designated initializer documentation are used.
 @result   The CPHTaskmaster singleton.
 */
+ (CPHTaskmaster*)sharedTaskmaster ;

/*!
 @brief    Designated initializer.
 @details  An instance of CPHTaskmaster needs to know what its commands are,
 what additional prompt if any to present in the authentication dialog, and the name
 of the tool that executes the tasks.
 @param    commandSpecs   A C array of BASCommandSpec structs, specifying the commands that
 the receiver will implement.  Because we can't retain a C array,
 we require it to be const we insure that it won't go away.  If this is NULL, commandSpecs
 will be set to a an array consisting of a single element of NULL components, which won't
 be very useful.  To be useful, this parameter must not be NULL.
 @param    installerToolName The name of the tool used to install the helper tool which must be
 found in Contents/MacOS.&nbsp;  If you pass nil, defaults to @"CPHHelperToolInstaller".
 @param    stringsFilename The name of a .strings file containing the prompt(s) specified in the
 rightDescriptionKey field(s) of the BASCommandSpec structs in commandSpecs.&nbsp;  If nil, defaults
 to the usual "Localizable".&nbsp;  This argument is only used for prepending a string to the " MyApp.app
 requires that you type your/administrator password" prompt in the authentication dialogs.
 @param    bundleIdentifier The bundle identifier of the bundle which contains the 
 stringsFilename.strings file in its Resources.&nbsp;  If nil, defaults to 
 [[NSBundle mainBundle] bundleIdentifier].&nbsp;  This argument is only used for prepending a string to the
 " MyApp.app requires that you type your/administrator password" prompt in the authentication dialogs.&nbsp;  
 It may be nil.
 */
- (id)initWithCommandSpecs:(const BASCommandSpec *)commandSpecs
		 installerToolName:(NSString *)installerToolName
     privilegedHelperLabel:(NSString*)privilegedHelperLabel
		   stringsFilename:(NSString*)stringsFilename
		  bundleIdentifier:(NSString*)bundleIdentifier ;

/*!
 @brief    Executes request in the Helper Tool
 @details  
 @param    request request to be executed
 @param    response_p Pointer to which the response NSDictionary* will be assigned.&nbsp;  
 *response_p will be autoreleased, but you must close any file descriptors
 that you get in it.
 @param    #error_p  Pointer to an NSError* to which any error which occurs
 will be assigned.  You may pass NULL if not interested in the error.
 
 @result   YES if successful, NO otherwise
 */
- (BOOL)executeRequest:(NSDictionary*)request
			response_p:(NSDictionary**)response_p
			   error_p:(NSError**)error_p ;

/*!
 @brief    Removes a named right from the system's Security Server's
 credentials cache, using AuthorizationRightRemove().
 
 @details  A utility method, useful in debugging
 @param    rightName The name of the right to be removed
 @result   The valued returned by AuthorizationRightRemove(), or
  paramErr if rightName is NULL.
 */
- (OSStatus)removeRight:(const char*)rightName ;

/*!
 @brief     Removes the rights specified for all commands in the
 receiver's current commandSpec from the the system's Security Server's
 credentials cache, using AuthorizationRightRemove().

 @details  A utility method, useful in debugging.&nbsp;  
 @param    error_p  On return, if an error occurred, points to
 an error giving the cause.&nbsp;  If you are not interested in the error,
 you may pass NULL.
 @result   YES upon success, otherwise NO.
*/
- (BOOL)removeRightsError_p:(NSError**)error_p ;


/*!
 @brief    Destroys the instance's authorization (_authRef) using AuthorizationFree() and then
 creates a new one using AuthorizationCreate().
 @details  A utility method, useful in debugging
 @param    rightName The name of the right to be removed
 */
- (void)recreateAuthorization ;

@end
