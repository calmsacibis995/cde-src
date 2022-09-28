//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/* @(#)tt_file_system_entry_utils.C	1.7 93/09/07
 * Tool Talk Utility - tt_file_system_entry_utils.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Implements the _Tt_file_system_entry utilities.
 *
 */

#include "util/tt_file_system_entry.h"
#include "util/tt_file_system_entry_utils.h"

implement_list_of(_Tt_file_system_entry)

_Tt_file_system_entry::
_Tt_file_system_entry ()
{
}

_Tt_file_system_entry::
_Tt_file_system_entry(
	const _Tt_string &hostname,
	const _Tt_string &mountpt,
	const _Tt_string &partition,
	int		 islocal,
	int		 isloopback,
	int		 iscachefs
)
{
	entryHostname = hostname;
	entryMountPoint = mountpt;
	entryPartition = partition;
	localFlag = islocal;
	loopBackFlag = isloopback;
	cachefsFlag = iscachefs;
}

_Tt_file_system_entry::
~_Tt_file_system_entry ()
{
}

     
