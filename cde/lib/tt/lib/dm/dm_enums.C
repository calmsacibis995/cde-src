//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *
 * dm_enums.cc
 *
 * Implementations of _tt_enumname() functions for each DM enumeration type.
 *
 * A "switch" statement is used for each function.  This is somewhat
 * harder to type than an array declaration or a chain of conditionals,
 * but it has the advantage that if the enums are reordered no change 
 * need be made here; also, cfront warns if there are fewer cases in 
 * a switch on an enum variable than there are elements in the enum;
 * therefore, we'll get a warning if we add a enum element but forget
 * to update this code.
 * 
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

#include "dm/dm_enums.h"
     
/* following macro saves a lot of typing */
#if defined(__STDC__)
#define STRINGIFY(s) #s
#else
#define STRINGIFY(s) "s"
#endif

#define TTC(x) case x: return STRINGIFY(x)

const char *
_tt_enumname(_Tt_dm_status x)
{
	switch(x) {
	TTC(DM_OK);
	TTC(DM_ERROR);
	TTC(DM_INIT_FAILED);
	TTC(DM_DBDESC_EXISTS);
	TTC(DM_DB_EXISTS);
	TTC(DM_CREATE_FAILED);
	TTC(DM_OPEN_FAILED);
	TTC(DM_NO_RECORD);
	TTC(DM_READ_FAILED);
	TTC(DM_WRITE_FAILED);
	TTC(DM_DELETE_FAILED);
	TTC(DM_CLOSE_FAILED);
	TTC(DM_UNKNOWN_DBTABLE);
	TTC(DM_UNKNOWN_INDEX);
	TTC(DM_INVALID_VERSION_NUMBER);
	TTC(DM_PATHMAP_FAILED);
	TTC(DM_UPDATE_MFS_INFO_FAILED);
	TTC(DM_CLEAR_LOCKS_FAILED);
	TTC(DM_RECORD_LOCKED);
	TTC(DM_NO_MFS);
	TTC(DM_UNKNOWN_FS);
	TTC(DM_CONVERSION_ERROR);
	TTC(DM_RECORD_SET);
	TTC(DM_ACCESS_DENIED);
	      default:
		return "! _Tt_dm_status";
	}
}



const char *
_tt_enumname(_Tt_dbtable_id x) {
      switch (x) {
	TTC(DM_TABLE_OID_PROP);
	TTC(DM_TABLE_OID_ACCESS);
	TTC(DM_TABLE_OID_CONTAINER);
	TTC(DM_TABLE_DOCOID_PATH);
	TTC(DM_TABLE_MSG_QUEUE);
	TTC(DM_TABLE_LINK_ENDS);
	TTC(DM_TABLE_LINK_PROP);
	TTC(DM_TABLE_LINK_ACCESS);
	TTC(DM_TABLE_LAST);
	    default:
	      return "! _Tt_dbtable_id";
      }
}

const char *
_tt_enumname(_Tt_key_id x) {
	switch (x) {
	TTC(DM_KEY_UNDEFINED);	/* for records read iterator */
	TTC(DM_KEY_NONE);
	TTC(DM_KEY_OID_PROP);
	TTC(DM_KEY_OID);
	TTC(DM_KEY_DOC);
	TTC(DM_KEY_PATH);
	TTC(DM_KEY_MSG_PART);
	TTC(DM_KEY_LINK_DIR);
	TTC(DM_KEY_END);
	TTC(DM_KEY_LINK_PROP);
	      default:
	      return "! _Tt_key_id";	
	}
}

const char *
_tt_enumname(_Tt_name_space x) {
	switch (x) {
	TTC(DM_NS_NFS);
	TTC(DM_NS_NSE);
	TTC(DM_NS_STANDALONE);  
	TTC(DM_NS_REDIRECT);
	TTC(DM_NS_LAST);
	      default:
		return "! _Tt_name_space";	
	}
}

const char *
_tt_enumname(_Tt_propvalue_state x) {
	switch (x) {
	TTC(DM_PVS_OLD);
	TTC(DM_PVS_NEW);
	TTC(DM_PVS_DELETE);
	      default:
		return "! _Tt_propvalue_state";
	}
}
