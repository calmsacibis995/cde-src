//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *
 * mp_c_pattern.cc
 *
 * _Tt_pattern member functions called only on client side.  Since there are
 * so few, I didn't bother making a _Tt_c_pattern class, but by breaking
 * this out into a separate file we can avoid defining _tt_c_mp in the
 * main mp_pattern.cc file, so that inadvertent references to it get
 * caught at compile time.
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 */

#include "mp/mp_c.h"
#include "mp/mp_pattern.h"
#include "api/c/api_error.h"
#include "util/tt_port.h"

//
// Constructor for a _Tt_pattern. Calls the base constructor and in
// client-mode, sets the id of the pattern.
//
_Tt_pattern::
_Tt_pattern()
{
	if (! _tt_mp->in_server()) {
		set_id(_tt_c_mp->default_procid()->id());
	}
	base_constructor();
}

//  
// Called by _Tt_procid::add_pattern only in client mode.
// 
// Sets the id for this pattern which is unique. The uniqueness of the id
// is guaranteed by giving the procid of the "sender" which is the procid
// which is registering this pattern. To further distinguish among
// different patterns registered by the same sender, there is a counter
// maintainted by _Tt_mp and returned by generate_pattern_id. Thus the id
// consists of <counter>:<sender_id>
// 
//  XXX: note that _Tt_procid::sender has a dependency on the format of
//  this message.
// 
void _Tt_pattern::
set_id(const _Tt_string &sender)
{
	char		id[256];

	sprintf(id,"%d:%s",_tt_mp->generate_pattern_id(),(char *)sender);
	_pattern_id = id;
}

//
// If this pattern is file scoped, record this session on this pattern's
// files.
//
Tt_status _Tt_pattern::
join_files(const _Tt_string &sessID) const
{
	if (! ((scopes() & (1<<TT_FILE)) || (scopes() & (1<<TT_BOTH)))) {
		// Not file-scoped
		return TT_OK;
	}
	Tt_status	worstErr = TT_OK;
	_Tt_db_results	dbStatus;
	Tt_status	status;
	_Tt_c_file_ptr	file;
	_Tt_string_list_cursor fileC( _files );
	while (fileC.next()) {
		status = _tt_mp->find_file( *fileC, file, 1 );
		if (status != TT_OK) {
			worstErr = status;
			continue;
		}
		dbStatus = file->addSession( sessID );
		status = _tt_get_api_error( dbStatus, _TT_API_FILE );
		if (status == TT_ERR_INTERNAL) {
			_tt_syslog( 0, LOG_ERR,
				    "_Tt_db_file::addSession(): %d", dbStatus);
		}
		if (status != TT_OK) {
			worstErr = status;
			continue;
		}
	}
	return worstErr;
}
