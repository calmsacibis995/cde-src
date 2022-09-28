//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 * Tool Talk Utility - tt_hostname_redirection_map.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Defines a hostname redirection map.  A global version
 * of this object is in _tt_global.
 */

#include "db/tt_db_hostname_redirection_map.h"
#include "util/tt_path.h"
#include "util/tt_port.h"

#define HOSTNAME_REDIRECTION_MAP_STRING	"hostname_map"

_Tt_db_hostname_redirection_map::
_Tt_db_hostname_redirection_map ()
{
      map = new _Tt_string_map((_Tt_object_table_keyfn)
			       &_Tt_map_entry::getAddress);
}

_Tt_db_hostname_redirection_map::
~_Tt_db_hostname_redirection_map ()
{
}

_Tt_string _Tt_db_hostname_redirection_map::
findEntry(const _Tt_string &address)
{
      return map->findEntry(address);
}


// This function looks for the specified file in the user, system and network
// database directories.  It returns the path of the existing file with the
// highest precedence (user > system > network).  If the environment variable
// specified by "user_env" contains the path of a file that exists, it is
// the highest precedence; otherwise we get the value of TTPATH, or use the
// default value, to search for the file.
//
// If the returned path is a NULL string, then no file was found.
//
_Tt_string _tt_hostname_redirection_user_path()
{
	// give preference to the cde named setting
        _Tt_string user_path = _tt_get_first_set_env_var(2, "TT_HOSTNAME_MAP", "_SUN_TT_HOSTNAME_MAP");

	struct stat stat_buf;

	// If the user environment variable is set, that takes priority
	if (user_path.len()) {
		if (!stat((char *)user_path, &stat_buf)) {
			return user_path;
		}
	}

	// `0' means return directories only, not xdr files.
	_Tt_string_list_ptr 	tt_path_list = _tt_path(0);

        if (tt_path_list->is_empty()) {
		user_path = (char *)0;
                return user_path;
        }

        _Tt_string_list_cursor 	pathC(tt_path_list);
	_Tt_string 		system_path = (char *)0;
	_Tt_string 		network_path = (char *)0;

        if (! pathC.next()) {
		user_path = (char *)0;
                return user_path;
        } else {
        	user_path = *pathC;
		user_path = user_path.cat(HOSTNAME_REDIRECTION_MAP_STRING);
        }

        if (pathC.next()) {
        	system_path = *pathC;
		system_path = system_path.cat(HOSTNAME_REDIRECTION_MAP_STRING);

		if (pathC.next()) {
			// if we have both /usr/dt and /usr/openwin as the
			// network path be sure and get the last one in the
			// list, per the $TTPATH spec.
			network_path = tt_path_list->bot();
			network_path = network_path.cat(HOSTNAME_REDIRECTION_MAP_STRING);
		}
        }

	// Try the user file...
	if (user_path.len()) {
		if (!stat((char *)user_path, &stat_buf)) {
			return user_path;
		}
	}

	// Try the system file...
	if (system_path.len()) {
		if (!stat((char *)system_path, &stat_buf)) {
			return system_path;
		}
	}

	// Try the network file...
	if (network_path.len()) {
		if (!stat((char *)network_path, &stat_buf)) {
			return network_path;
		}
	}

	user_path = (char *) NULL;
	return user_path;
}


void _Tt_db_hostname_redirection_map::
refresh ()
{
	_Tt_string path = _tt_hostname_redirection_user_path();

	if (path.len() > 0) {
		map->loadFile(path);
	}
}
