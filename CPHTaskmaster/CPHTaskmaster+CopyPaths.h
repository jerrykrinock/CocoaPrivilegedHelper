#import <Cocoa/Cocoa.h>
#import "CPHTaskmaster.h"

extern NSString* const CPHTaskmasterPerPathErrors ;

@interface CPHTaskmaster (CopyPathsTask) 

/*!
 @brief    Copies a collection of source file paths to
 destination file paths, or deletes destination file paths.

 @details  To copy paths, uses the BSD function copyfile(3) with flags
 COPYFILE_ALL and COPYFILE_RECURSIVE.  The former means
 that it copies the file's data, POSIX "stat" metadata
 (permissions mode, times, etc.), extended attributes,
 and access control lists.  See man copyfile(3) for
 more details.  The latter means that it will copy
 directories recursively, but this only works in Mac OS 
 10.6 or later.  After copying, the owner ID (uid)
 and group ID (gid) of each copied file are set to be
 the same as the uid and gid of their new parent
 directories by chown(3).  If this method is invoked
 in Mac OS 10.5, a nonrecursive copy is performed.
 
 To delete paths, uses the BSD function unlink(3).
 
 WILL NOT CREATE ROOT PATH
 
 Regardless of OS, the given destination path must
 already exist; this method will not create needed
 directories to the destination.
 
 TRAILING SLASHES
 
 If you put a trailing slash on the source path, it results in the
 *contents* of the source directory being copied into the destination
 directory.  Otherwise, it results in the source directory itself
 being copied into the destination directory.  Trailing slash on the
 destination path does not matter.
 
 In pictures, say that you have a source directory S containing a
 file F, and want to copy this to a destination directory D.  
 copyfile ("/S", "/D", s, COPYFILE_RECURSIVE)   ; // results in /D/S/F
 copyfile ("/S", "/D/", s, COPYFILE_RECURSIVE)  ; // results in /D/S/F
 copyfile ("/S/", "/D", s, COPYFILE_RECURSIVE)  ; // results in /D/F
 copyfile ("/S/", "/D/", s, COPYFILE_RECURSIVE) ; // results in /D/F
 
 @param    paths  A dictionary of paths.  To copy, the keys are the 
 source paths and the value of each key is its desired
 destination path.  To delete a path, the key is the path to be
 deleted and the value is an empty string.  (I tried using an 
 NSNull here, but BetterAuthorizationSample failed to run the
 task due to a "Broken pipe", and gave up on that after about
 a half hour of fruitless digging.)
 @param    error_p  On output, if an error occurs, points
 to a descriptive error.  Pass NULL if you're not 
 interested.  If the error was not too serious, the -userInfo 
 of *error_p will not contain an NSUnderlyingErrorKey but
 will instead contain a value for the key
 CPHTaskmasterPerPathErrors.  This value is an
 inner dictionary whose keys are one or more of the passed-in
 paths for which errors occurred, and whose values are
 themselves inner-inner dictionaries that contain values for
 the three or four of the keys given in CPHTasks_Typicapp.h.
 The value for key kFailedFunction will be either "copyfile"
 or "unlink" depending on whether the failed path was to be copied
 or deleted, respectively.  If the reason is that the source
 path was not found, the value of kReturnValue will be an
 NSNumber of value -1 and the kErrno will be an NSNumber
 with a value of ENOENT (=2).  Actually, the value of kErrno
 is all you need to determine this, but since we don't like
 to work on the Helper Tool very often, whenever we do we
 tend to return everything except the kitchen sink.

 @result   YES if all copy operations succeeded, NO otherwise.
*/
- (BOOL)copyFilePaths:(NSDictionary*)paths
			  error_p:(NSError**)error_p ;

/*!
 @brief    A wrapper around -copyFilePaths:error_p: which
 has a simpler interface but only handles one path.

 @details  
 @param    sourcePath  
 @param    destinPath  
 @param    error_p  If not NULL and if an error occurs, upon return,
 will point to an error object encapsulating the error.  This
 error's domain will be NSPOSIXErrorDomain.  If the error is
 that sourcePath does not exist, this error's code will be EOENT (=2).
 @result   YES if the method completed successfully, otherwise NO
*/
- (BOOL)copyPath:(NSString*)sourcePath
		  toPath:(NSString*)destinPath
		 error_p:(NSError**)error_p ;

	@end
