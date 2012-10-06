#import <Cocoa/Cocoa.h>
#import "CPHTaskmaster.h"

struct stat ;  // Needed to avoid compiler warning.  See http://stackoverflow.com/questions/6654526/semantic-warning-on-xcode-4

@interface CPHTaskmaster (StatFilesTask) 

/*!
 @brief    Returns the metadata provided by stat(3)
 for a collection of paths

 @details  Uses the BSD function stat(3).
 @param    paths  A array of paths for which stats
 are desired
 @param    stats_p  On output, points to a dictionary.
 Each key in the dictionary is one of the given paths,
 for which stat(3) succeeded, and each value is
 an NSData object whose bytes are a stat struct containing
 the stat output for that path.  That is, to extract
 a given stat struct, create a buffer of size
 sizeof(struct stat) and then fill it by sending -bytes:
 to the object in *stats_p for the given path/key.
 To extract a value from stat struct, see man stat(3) for
 a listing of the members in the stat struct.
 @param    error_p  On output, if an error occurs, points
 to a descriptive error.&nbsp;  Pass NULL if you're not 
 interested.
 For efficiency in case you expect that a path may not exist
 and pass error_p = NULL, we don't create a local error.  If
 you pass a non-NULL error_p, it should point to nil (i.e.
 there should be no "pre-existing" error).
 @result   YES if all stat operations succeeded, NO otherwise.
*/
- (BOOL)statFilePaths:(NSArray*)paths
			  stats_p:(NSDictionary**)stats_p
			  error_p:(NSError**)error_p ;

/*!
 @brief    A wrapper around -statFilePaths:error_p: which
 has a simpler interface but only handles one path.

 @param    error_p  A pointer to an NSError instance, or nil if
 you're not interested in the error.  If you pass a pointer, and
 the requested result cannot be returned, will point to an
 NSError explaning the problem upon output.
 For efficiency in case you expect that a path may not exist
 and pass error_p = NULL, we don't create a local error.  If
 you pass a non-NULL error_p, it should point to nil (i.e.
 there should be no "pre-existing" error).
 */
- (BOOL)statPath:(NSString*)path
			stat:(struct stat*)stat_p
		 error_p:(NSError**)error_p ;

	@end
