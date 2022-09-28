//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 * Tool Talk Utility - tt_partition_redirection_map.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Defines a partition redirection map.  A global version
 * of this object is in declared db_server_svc.cc.
 */

#include "tt_db_partition_redirection_map.h"
#include "util/tt_string_map.h"
#include "util/tt_map_entry.h"
#include "util/tt_path.h"
#include "util/tt_port.h"

#define	PARTITION_REDIRECTION_MAP_STRING	"partition_map"

_Tt_db_partition_redirection_map::
_Tt_db_partition_redirection_map ()
{
	map = new _Tt_string_map((_Tt_object_table_keyfn)
				 &_Tt_map_entry::getPathAddress);
}

_Tt_db_partition_redirection_map::
~_Tt_db_partition_redirection_map ()
{
}

_Tt_string _Tt_db_partition_redirection_map::
findEntry(const _Tt_string &address)
{
	return map->findEntry(address);
}


// This function looks for the specified file in the system (only)
// database directories and returns the path of that file if the file exists.
// If the environement variable specified by "user_env" contains the path of a
// file that exists, it is used instead, otherwise we get the value of TTPATH,
// or use the default value, to search for thei file.
//
// If // the returned path is a NULL string, then no file was found.
//
_Tt_string _tt_partition_redirection_path()
{
        // give preference to the cde named setting
        _Tt_string path = _tt_get_first_set_env_var(2, "TT_PARTITION_MAP", "_SUN_TT_PARTITION_MAP");
 
        struct stat stat_buf;
 
        // If the user environment variable is set, that takes priority
        if (path.len()) {
                if (!stat((char *)path, &stat_buf)) {
                        return path;
                }
        }

        // `0' means return directories only, not xdr files.
        _Tt_string_list_ptr       tt_path_list = _tt_path(0);

        if (tt_path_list->is_empty()) {
        	path = (char *)0;
                return path;
        }

        _Tt_string_list_cursor  pathC(tt_path_list);
        _Tt_string              system_path = (char *)0;

        if (! pathC.next()) {
        	path = (char *)0;
                return path;
        }
	// Just burn  the first one, to get to the 2nd one below.

        if (pathC.next()) {
                system_path = *pathC;
                system_path = system_path.cat(PARTITION_REDIRECTION_MAP_STRING);
 
		// Try the system file...
		if (!stat((char *)system_path, &stat_buf)) {
			return system_path;
		}
        }
 
	path = (char *) NULL;
        return path;
}


void _Tt_db_partition_redirection_map::refresh ()
{
	_Tt_string path = _tt_partition_redirection_path();

	if (path.len()) {
		map->loadFile(path);
	}
}
