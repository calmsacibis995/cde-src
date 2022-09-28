//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $XConsortium: tt_path.C /main/cde1_maint/2 1995/10/04 13:36:49 gtsang $ 			 				
/* @(#)tt_path.C	1.26 93/07/30
 * Tool Talk Utility - tt_path.cc
 *
 * Copyright (c) 1990, 1993 by Sun Microsystems, Inc.
 *
 * Implementation for filepath utility functions
 *
 */
#if defined(ultrix) || defined(CRAY)
#include <sys/types.h>
#endif
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util/tt_assert.h"
#include "util/tt_string.h"
#include "util/tt_file_system_entry.h"
#include "util/tt_file_system.h"
#include "util/tt_host.h"
#include "util/tt_global_env.h"
#include "util/tt_host_equiv.h"

#if defined(OPT_BUG_USL) || defined(OPT_BUG_UXP)
#define S_ISLNK(mode)   ((mode & 0xF000) == S_IFLNK)
#endif

// Not everybody has realpath() in libc, sometimes we have to define
// it ourselves, see realpath*.c in tt/lib

#if !defined(OPT_HAS_REALPATH)
	extern "C" { char *_tt_internal_realpath(char*, char*); }
#endif

#if defined(OPT_BUG_HPUX)
extern "C" {
        char *realpath(char *, char *);
}
#endif


// Use the system realpath on OS's that have it, otherwise
// use the ToolTalk implementation of it.
char *
_tt_get_realpath(char  *pathname, char  *finalpath)
{
#ifdef OPT_HAS_REALPATH
	return realpath(pathname, finalpath);
#else
	return _tt_internal_realpath(pathname, finalpath);
#endif
}

/*
 *  _Tt_dirname - given a path name, returns the char index to the end of
 *  the path's longest prefix (directory), i.e., excluding the basename and the
 *  last '/'.  e.g., given "/home3/dynamo/tan/text", returns 16
 *  (/home3/dynamo/tan^).
 */

int _Tt_dirname(const char *path, int len)
{
	int path_len = len ? len : strlen(path);
	while (path[path_len] != '/') {	/* discard base name */
		ASSERT(path_len >= 0, "Invalid path");
		--path_len;
	}
	return (path_len - 1);		/* discard the suffix '/' */
}

/*
 *  _Tt_basename - given a path name, returns the char index to the beginning of
 *  the path's suffix, e.g., given "/home3/dynamo/tan/text", returns 18
 *  (/home3/dynamo/tan/^).
 */

int _Tt_basename(const char *path, int len)
{
	return (_Tt_dirname(path, len) + 2);
}

/*
 * _tt_dir_entries() - Return a new list of paths, with one entry for each
 *	entry in the directory <path>, each entry consisting of
 *	<path> appended by a slash and the name of the entry.
 *	Returns an empty list if <path> is not a directory.
 *	If !follow_symlinks, returns an empty list if <path> is a symlink.
 */
_Tt_string_list_ptr
_tt_dir_entries(const _Tt_string &path, bool_t follow_symlinks )
{
	DIR		       *dirp;
	_Tt_string_list_ptr	entries(new _Tt_string_list);

	if (! follow_symlinks) {
		struct stat	lstat_buf;
		int		lstat_status;

		lstat_status = lstat( (char *)path, &lstat_buf );
		if (( lstat_status == 0) && S_ISLNK(lstat_buf.st_mode)) {
			return entries;
		}
	}
	dirp = opendir( (char *)path );
	if (dirp == NULL) {
		return entries;
	}

	_Tt_string	epath;
	_Tt_string	ename;

	while (TRUE) {
		struct dirent  *entry;

		entry = readdir( dirp );
		if (entry == NULL) {
			break;
		}
		ename = entry->d_name;
		if ((ename == ".") || (ename == "..")) {
			continue;
		}
		epath = (char *)path;
		epath = epath.cat("/").cat( ename );
		entries->push( epath );
	}
	int closedir_err = closedir( dirp );
	ASSERT(closedir_err == 0, "Could not close directory");
	return entries;

} /* dir_entries() */

/*
 * Returns the real path (resolves symbolic links, etc) of the specified path,
 * even if the file in the specified path does not exist.
 */
_Tt_string
_tt_realpath(const _Tt_string &path)
{
  _Tt_string temp_path = path;

  if (!path.len()) {
    return temp_path;
  }

  if (temp_path[0] != '/') {
    // A relative path, make it absolute.
    char wd[MAXPATHLEN+1];
    if (getcwd(wd, sizeof(wd))) {
      temp_path = _Tt_string(wd).cat("/").cat(path);
    }
  }
  
  _Tt_string right, left, dir, base;
  left = temp_path;
  
  _Tt_string temp_real_path(MAXPATHLEN);
  char *p = _tt_get_realpath((char *)temp_path, (char *)temp_real_path);
  
  while ((p == 0) && (left.len() > 0)) {
    // Realpath failed; drop right components until it works.
    base = left.rsplit('/', dir);
    
    // Takes care of special case of "/file" where
    // file does not exist.
    if ((dir.len() <= 0) && (left[0]=='/')) {
      dir = "/";
    } 
    left = dir;
    
    if (right.len() > 0) {
      right = base.cat("/").cat(right);
    } else {
      right = base;
    } 
    p = _tt_get_realpath((char *)left, (char *)temp_real_path);
  }
  
  _Tt_string real_path;

  // Only "/" matches a real path...
  if (p == 0) {
    // Use the original path
    real_path = temp_path;
  }
  // Else parts of the path don't really exist...
  else if (right.len() > 0) {
    // Append the fake stuff on
    real_path = (char *)temp_real_path;

    if (real_path [real_path.len()-1] != '/') {
      real_path = real_path.cat("/").cat(right);
    }
    else {
      real_path = real_path.cat(right);
    }
  }
  // Else the entire path exists, now we have the "real" one...
  else {
    real_path = (char *)temp_real_path;
  }
  
  return real_path;
}

int
_tt_isdir(const _Tt_string &path)
{
	if (path.len() == 0) {
		return 0;
	}
	struct stat stat_buf;
	if (stat((char *)path, &stat_buf) != 0) {
		return 0;
	}
	return S_ISDIR(stat_buf.st_mode);
}

/*
 * Returns the the best guess of the absolute network path of the specified
 * path without using the TT DB Server.  This is accomplished by using
 * _tt_realpath on the specified path and then by finding best mount
 * table entry match to the path and exchanging the local partition path
 * with the mounted partition path.
 */
_Tt_string _tt_local_network_path(const _Tt_string &path)
{
	_Tt_string network_path;

	if (path.len()) {
		_Tt_string real_path = _tt_realpath(path);
#ifdef notdef
printf("DEBUG _tt_local_network_path: real_path initialized to %s\n",
	(char *) real_path);
#endif
		_Tt_file_system file_system;
		_Tt_file_system_entry_ptr entry =
			file_system.bestMatchToPath(real_path);

		_Tt_string hostname = entry->getHostname();
#ifdef notdef
printf("DEBUG _tt_local_network_path: hostname = %s\n",
	(char *) hostname);
#endif
		_Tt_string loop_back_mount_point =
				entry->getLoopBackMountPoint();
#ifdef notdef
printf("DEBUG _tt_local_network_path: loop_back_mount_point = %s\n",
	(char *) loop_back_mount_point == NULL ? "(null)" : (char *) loop_back_mount_point);
#endif
		_Tt_string mount_point;
		if (loop_back_mount_point.len() > 0) {
			mount_point = loop_back_mount_point;
#ifdef notdef
printf("DEBUG _tt_local_network_path: ! isLocal 1: mount_point = %s\n",
	(char *) mount_point);
#endif
		} else {
			mount_point = entry->getMountPoint();
#ifdef notdef
printf("DEBUG _tt_local_network_path: ! isLocal 2: mount_point = %s\n",
	(char *) mount_point);
#endif
		}

		if (entry->isLocal()) {
			if (loop_back_mount_point.len() > 0) {
#ifdef notdef
printf("DEBUG _tt_local_network_path: isLocal 2: mount_point = %s\n",
	(char *) mount_point);
#endif
				// Get the path info after the loop back
				// mount point path.
				real_path = real_path.right(real_path.len() -
							    mount_point.len());
#ifdef notdef
printf("DEBUG _tt_local_network_path: isLocal 2: real_path = %s\n",
	(char *) real_path);
#endif
				// Replace the loop back mount point path
				// with the mount point path.
				if (mount_point != "/") {
					real_path = mount_point.cat(real_path);
				}
#ifdef notdef
printf("DEBUG _tt_local_network_path: isLocal 3: real_path = %s\n",
	(char *) real_path);
#endif
			}
		} else {
			_Tt_string partition = entry->getPartition();
#ifdef notdef
printf("DEBUG _tt_local_network_path: ! isLocal: partition = %s\n",
	(char *) partition);
#endif
			// Get the path info after the mount point path
			real_path =
			real_path.right(real_path.len()-mount_point.len());
#ifdef notdef
printf("DEBUG _tt_local_network_path: ! isLocal: real_path = %s\n",
	(char *) real_path);
#endif
			// Replace the mount point path with the exported
			// partition path.
			real_path = partition.cat(real_path);
#ifdef notdef
printf("DEBUG _tt_local_network_path: ! isLocal: real_path = %s\n",
	(char *) real_path);
#endif
		}
		network_path = hostname.cat(":").cat(real_path);
#ifdef notdef
printf("DEBUG _tt_local_network_path: network_path = %s\n",
	(char *) network_path);
#endif
	}

	return network_path;
}


/* Converts a path from the network form to a local form:
 *
 *	From:
 *
 *		hostname:/path
 *
 *	To:
 *
 *		/local_path
 *
 * If the path cannot be mapped out through the local mount tables,
 * it converted to the form:
 *
 *		/net/hostname/path
 */
_Tt_string
_tt_network_path_to_local_path(const _Tt_string &network_path)
{
// We initialize _tt_global here if necessary. When this call is exposed,
// we don\'t want to require that tt_open be called first.	
	
	if (_tt_global==0) {
		_tt_global = new _Tt_global;
	}

	_Tt_string local_path;
	
	if (!network_path.len()) {
		return local_path;
	}
	
	_Tt_string hostname;
	_Tt_string path;
	_Tt_host_ptr localtthost;
	
	path = network_path;
	path = path.split(':', hostname);

	if (!_tt_global->get_local_host(localtthost)) {
		// If we don\'t even know who we are, we must not be networked
		// (I have no evidence this ever happens...)
		local_path = path;
	} else {
		_Tt_host_equiv_ptr eq_p = new _Tt_host_equiv;

		if (eq_p->hostname_equiv(localtthost->name(), hostname) == 1) {
			// If we own the file, we can just use this path
			local_path = path;
		} else {
			_Tt_file_system file_system;
			_Tt_file_system_entry_ptr entry;
			entry = file_system.findMountEntry(network_path);
			if (entry.is_null()) {

				char * tmp_path = getenv("DTMOUNTPOINT");

				// Can\'t find a usable mount. In desperation,
				// try to invoke the automounter.  If the
				// DTMOUNTPOINT env. variable exists, use
				// it as the root of the path, otherwise /net.

				if (tmp_path == (char *)0) {
					local_path = "/net/";
				} else {
					local_path = tmp_path;

					// Be user friendly; ensure there
					// is a trailing slash...
					if (strcmp((char *) local_path.right(1), "/") != 0) {
						local_path = local_path.cat('/');
					}
				}
				// Ensure we don't use qualified hostname.
				hostname = eq_p->prefix_host(hostname,
							    localtthost->name());

				local_path = local_path.cat(hostname).cat(path);
			} else {
				_Tt_string mount_point = entry->getMountPoint();
				_Tt_string partition = entry->getPartition();
				
				// Get the path info after the partition path
				if (strcmp((char *) partition, "/") != 0) {
					path = path.right(path.len()-partition.len());
				}
				
				// Replace the partition path with the mount point path
				local_path = mount_point.cat(path);
			}
		}
	}
	
	path = _tt_realpath(local_path);

#ifdef OPT_AUTOMOUNT_PATH_FIX
	//
	// Extract the automount point which is introduced by a
	// call to realpath() when an NFS automounter is used:
	//      /DTAUTOMOUNTPOINT/DTMOUNTPOINT/host/path
	//
	// And convert to:
	//                       /DTMOUNTPOINT/host/path
	//

	// Fetch automount prefix
	_Tt_string automnt_prefix = getenv("DTAUTOMOUNTPOINT");
	if (!automnt_prefix.len()) {
	    // use default
	    automnt_prefix = "/tmp_mnt/";
	}
	else {
	    // fix up users prefix.   Providing an ending slash
	    // delineates the directory element.
	    if (strcmp((char *) automnt_prefix.right(1), "/") != 0) {
		automnt_prefix = automnt_prefix.cat('/');
	    }
	}

	// Pull out automount prefix from start of path
	int index = path.index(automnt_prefix);
	if (index == 0) {
	    // the path starts with the prefix, so rip it out
	    // minus the one trailing prefix slash.
	    path = path.right(path.len() - (automnt_prefix.len() - 1));
	}
#endif /* OPT_AUTOMOUNT_PATH_FIX */

	return path;
}


bool_t _tt_is_network_path (const _Tt_string &path)
{
  int slash_index = path.index('/');
  if (slash_index > -1) {
    int colon_index = path.index(':');
    if ((colon_index > 0) && (slash_index == colon_index+1)) {
      return TRUE;
    }
  }  

  return FALSE;
}


// Return a list of _Tt_strings where each string contains
// an element of TTPATH.  Or if TTPATH is undefined,  the tuple:
//
//	"$HOME/.tt/"
//	"/etc/tt/"
//	"/usr/dt/appconfig/tttypes/"
//	"/usr/openwinhome/etc/tt/"
//
// The paramater supply_xdrs is so we can use this both to supply
// a list of directories (above), as well as a list of files (the
// above  directories terminatd in "types.xdr".
//
_Tt_string_list_ptr _tt_path(int supply_xdrs)
{

        _Tt_string_list_ptr pathlist = new _Tt_string_list;

        _Tt_string path = getenv("TTPATH");

        if (path.len() <= 0) {
	// No TTPATH set, use the default...
                _Tt_string home = getenv("HOME");

		if (supply_xdrs) {
			pathlist->append(home.cat("/.tt/types.xdr"));
			pathlist->append(_Tt_string("/etc/tt/types.xdr"));
			pathlist->append(_Tt_string("/usr/dt/appconfig/tttypes/types.xdr"));

			home = getenv("OPENWINHOME");
			if (home.len() == 0) {
				home = "/usr/openwin";
			}
			pathlist->append(home.cat(_Tt_string("/etc/tt/types.xdr")));
		} else  {
			pathlist->append(home.cat("/.tt/"));
			pathlist->append(_Tt_string("/etc/tt/"));
			pathlist->append(_Tt_string("/usr/dt/appconfig/tttypes/"));

			home = getenv("OPENWINHOME");
			if (home.len() == 0) {
				home = "/usr/openwin";
			}
			pathlist->append(home.cat(_Tt_string("/etc/tt/")));
		}

        } else {
	// parse the user:system:network from path variable

                int n = 0;
                _Tt_string pathname;
                while (n >= 0) {
                        n = path.index(':');
                        if (n > 0) {
                                pathname = path.left(n);
                                if (pathname.len() == 0) break;
                                path = path.right(path.len() - n - 1);
                        } else {
                                pathname = path;
                                path = 0;
                        }

			if (supply_xdrs) {
				if (_tt_isdir(pathname)) {
					pathname = pathname.cat("/types.xdr");
				}
				pathlist->append( pathname );
			}
                }
        }

        return pathlist;
}
