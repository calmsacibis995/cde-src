/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM) - dm_db.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 *  The class _Tt_db implements a descriptor that contains information
 *  about a database's location and its table layout.
 */

#ifndef  _TT_DM_DB_H
#define  _TT_DM_DB_H

#include <dm/dm_db_desc.h>

class _Tt_db : public _Tt_object {
      public:
	_Tt_db();
	virtual ~_Tt_db();
	_Tt_dm_status		open(_Tt_string nodeid,
				     _Tt_dbtable_id table_id);
	_Tt_dm_status		open(_Tt_dbref_ptr dbref,
				     _Tt_dbtable_id table_id);
	_Tt_dm_status		open_oid(_Tt_oid_ptr oid,
				     _Tt_dbtable_id table_id);
	_Tt_dm_status		open_path(_Tt_string path,
					  _Tt_dbtable_id table_id);
	_Tt_dm_status		read(_Tt_record_ptr rec);
	_Tt_dm_status		readfirst(_Tt_record_ptr rec);
	_Tt_dm_status		readnext(_Tt_record_ptr rec);
	void			reset_read() { db->reset_read(); }
	_Tt_dm_status		test_and_set(_Tt_record_ptr rec);
	_Tt_dm_status		write(_Tt_record_ptr rec);
	_Tt_dm_status		remove(_Tt_record_ptr rec);
	_Tt_dm_status		close();
	_Tt_dm_status		setuid(_Tt_oid_ptr oid, uid_t uid = -1);
	_Tt_dm_status		setgroup(_Tt_oid_ptr oid, gid_t group);
	_Tt_dm_status		setmode(_Tt_oid_ptr oid, mode_t mode);
	_Tt_dm_status		getaccess(_Tt_oid_ptr oid, uid_t &uid,
					  gid_t &group, mode_t &mode);
#ifdef UNUSED
	_Tt_dm_status		readspec(_Tt_oid_ptr oid,
					 _Tt_prop_array &props);
	_Tt_dm_status		writespec(_Tt_oid_ptr oid,
					  _Tt_prop_array props);
	_Tt_dm_status		addsession(_Tt_oid_ptr oid,
					   _Tt_string session);
	_Tt_dm_status		delsession(_Tt_oid_ptr oid,
					   _Tt_string session);
#endif /* UNUSED */	
	_Tt_dm_status		gettype(_Tt_oid_ptr oid, _Tt_string typeprop,
					_Tt_string &typename);
	_Tt_dm_status		error_code()	{ return errcode; }
	_Tt_string		error_string() 	{ return errcode; }
	_Tt_dm_status		transaction_write(_Tt_record_ptr rec);
	_Tt_dm_status		transaction_abort();
	_Tt_dm_status		transaction_commit();
      private:
	static	_Tt_dm_status	errcode;
	_Tt_db_desc_ptr		db;
};

declare_ptr_to(_Tt_db)

#endif /* _TT_DM_DB_DESC_H */
