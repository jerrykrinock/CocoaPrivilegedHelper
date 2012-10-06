CocoaPrivilegedHelper
by Jerry Krinock
-----------------------

This project provides a framework ?? for doing tasks in Cocoa with
a privilege escalation, in OS X without using deprecated methods like 
AuthorizationExecuteWithPrivileges.  It is the successor to
CPHsInCocoa.  The difference is that the it does not call
AuthorizationExecuteWithPrivileges which was deprecated and replaced with
SMJobBless in Mac OS X 10.6.  This project requires Mac OS X 10.6 or later.

If your project requires Mac OS X 10.7 or later, do not use this project.
There is an even better method, which uses XPC.

How to use the example
----------------------

Unfortunately because of the code signing, you can't just compile and run this, 
you need to make a few changes:

1. Make sure you have a code signing certificate
2. Edit CocoaPrivilegedHelperCommon.h and change kSigningCertCommonName to the 
   name of your certificate, which will be "3rd Party Mac Developer Application…
   if it's in the Mac App Store, "Developer ID…" otherwise
3. Open CocoaPrivilegedHelper-Info.plist, go to SMAuthorizedClients and change
   "Developer ID Application: Jerry Krinock" to the same
4. Open CPHDemoApp-Info.plist, go to SMPrivilegedExecutables and change
   "Developer ID Application: Jerry Krinock" to the same
5. Edit the global project settings, change 'Code Signing Identity' to 
   the name of your certificate.
   
This is necessary because all your binaries need to be signed and verified not
to have been tampered with, to avoid exposing a security hole.



Rough explanation
-----------------

There are 3 products defined here:

1. The main application
2. A privileged helper which can be used to perform privileged actions
3. A simple command-line tool which we want installed

It's CRITICAL that all 3 are codesigned. As currently set up, the requirements
are that the certificate is authorised by Apple, but you could change this.

The App asks LaunchServices to install the privileged helper, which will later 
be communicated with over a socket to perform the installation of a command-line
tool. Code signing is used to prevent tampering with the process.

Replicating this in your own projects
-------------------------------------

There are quite a lot of things that can go wrong when you're setting this up
from scratch, so if you have problems using this in your own projects, try the 
following:

1. Check you're signing all your projects with a valid certificate approved by Apple
2. Check the build settings in each of the example targets. There are several
   things there such as:
   a. Copying the helper in the app's /Library/LaunchServices and the console tool
      into resources
   b. Make sure the product name of the helper is fully qualified (com.company.blah)
   c. Make sure all the Info.plists have been updated to use the right certificate
   d. Check the command-line tools have 'Other Linker' options set to embed their
      own plist files (check the example)
3. Make sure you've updated the CPHTasks_InstallATool.h file to reflect your product
   names, certs etc. 
   
When interpreting the example, 'CPHDemoApp' is the main app, 
'CocoaPrivilegedHelper' is the reuseable helper which is installed into
LaunchServices, and 'cph-hellotool' is the command-line tool which is installed
as one of the available tasks.

Credits
-------

This project is an extension of PrivilegedHelperExample
by Steve Streeting, which can be found here:
   https://bitbucket.org/sinbad/privilegedhelperexample
and is based on his blog post here:
   http://www.stevestreeting.com/2011/11/25/escalating-privileges-on-mac-os-x-securely-and-without-using-deprecated-methods/

which is in turn derived extensively from these Apple samples:
- BetterAuthorizationSample (https://developer.apple.com/library/mac/#samplecode/BetterAuthorizationSample/Introduction/Intro.html)
- SMJobBless (https://developer.apple.com/library/mac/#samplecode/SMJobBless/Listings/ReadMe_txt.html)

My additions / changes to these samples can be considered public domain.
