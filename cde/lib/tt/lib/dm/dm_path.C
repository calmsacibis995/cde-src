//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *
 * dm_path.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

#include <dm/dm.h>
#include <dm/dm_path.h>

/* 
 * Return the network absolute path for the relative path.  Errors:
 *	DM_ERROR	Not a legal filename
 */
_Tt_dm_status
_tt_abs_path_of(_Tt_string rel_path, _Tt_string &abs_path)
{
	abs_path = _tt_dm->resolve_path(rel_path);
	if (abs_path.len() == 0) {
		return DM_ERROR;
	}
	return DM_OK;
}

/* 
 * Return the relative path, if any for this host, for the given absolute path
 */
_Tt_dm_status
_tt_rel_path_of(_Tt_string abs_path, _Tt_string &rel_path)
{
	rel_path = _tt_dm->resolve_abspath(abs_path);
	if (rel_path.len() == 0) {
		return DM_ERROR;
	}
	return DM_OK;
}
