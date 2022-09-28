//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *  Tool Talk Database Manager - dm_db.cc
 *
 *  Copyright (c) 1989 Sun Microsystems, Inc.
 *
 *  This file contains the implementation of the _Tt_db member functions.
 */

#include <dm/dm_db.h>
#include <dm/dm.h>


/*
 *  _Tt_db class
 */

_Tt_db::
_Tt_db()
{
}

_Tt_db::
~_Tt_db()
{
}

_Tt_dm_status _Tt_db::errcode = DM_OK;

/*
 *  _Tt_db::open - open the database for a specified table given a
 *  nodeid (<key, dbref>).  Error code is set to:
 *  	DM_OK  -  successful
 *	DM_CONVERSION_ERROR	- error in converting the nodeid to an oid
 *	DM_OPEN_FAILED             -  the open failed.
 *	DM_INVALID_VERSION_NUMBER  -  the database version numbers do not match.
 */

_Tt_dm_status _Tt_db::
open(_Tt_string nodeid, _Tt_dbtable_id table_id)
{
	_Tt_oid_ptr oid(new _Tt_oid());
	if (oid->open(nodeid) != DM_OK) {
		return (errcode = oid->error_code());
	}
	return open(oid->dbref(), table_id);
}

/*
 *  _Tt_db::open_path - open the database for a specified table given a
 *  document path.  Error code is set to:
 *  	DM_OK  -  successful
 *	DM_PATHMAP_FAILED	   -  the path to partition mapping failed.
 *	DM_OPEN_FAILED             -  the open failed.
 *	DM_INVALID_VERSION_NUMBER  -  the database version numbers do not match.
 */

_Tt_dm_status _Tt_db::
open_path(_Tt_string path, _Tt_dbtable_id table_id)
{
	if (DM_DEBUG(5)) {
		printf("_Tt_db::open_path - path = %s\n", (char *) path);
	}
	_Tt_dbref_ptr dbref;
	/* maps from file path to dbref (database host:path partition) */
	if (_tt_dm->docpath_to_dbref(path, dbref) != DM_OK) {
		return (errcode = _tt_dm->error_code());
	}
	return open(dbref, table_id);
}

/*
 *  _Tt_db::open - open the database for a specified table given an
 *  oid.  Error code is set to:
 *  	DM_OK  -  successful
 *	DM_OPEN_FAILED             -  the open failed.
 *	DM_INVALID_VERSION_NUMBER  -  the database version numbers do not match.
 */

_Tt_dm_status _Tt_db::
open_oid(_Tt_oid_ptr oid, _Tt_dbtable_id table_id)
{
	return open(oid->dbref(), table_id);
}

/*
 *  _Tt_db::open - open the database for a specified table given a dbref.
 *  Set manual locking mode and checks the database version number.  Error code
 *  is set to:
 *  	DM_OK  -  successful
 *	DM_OPEN_FAILED             -  the open failed.
 *	DM_CREATE_FAILED           -  the create failed.
 *	DM_INVALID_VERSION_NUMBER  -  the database version numbers do not match.
 */

_Tt_dm_status _Tt_db::
open(_Tt_dbref_ptr dbref, _Tt_dbtable_id table_id)
{
	/* create the database descriptor object */
	if (_tt_dm->create_dbdesc(dbref, table_id, db) != DM_OK) {
		return (errcode = _tt_dm->error_code());
	}
	/* open the database */
	if (db->open() != DM_OK) {
		return (errcode = db->error_code());
	}
	return DM_OK;
}

_Tt_dm_status _Tt_db::
read(_Tt_record_ptr rec)
{
	if (db.is_null()) {
		return (errcode = DM_READ_FAILED);
	}
	if (db->read(rec) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
readfirst(_Tt_record_ptr rec)
{
	if (db.is_null()) {
		return (errcode = DM_READ_FAILED);
	}
	if (db->readfirst(rec) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
readnext(_Tt_record_ptr rec)
{
	if (db.is_null()) {
		return (errcode = DM_READ_FAILED);
	}
	if (db->readnext(rec) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
write(_Tt_record_ptr rec)
{
	if (db.is_null()) {
		return (errcode = DM_WRITE_FAILED);
	}
	if (db->write(rec) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
remove(_Tt_record_ptr rec)
{
	if (db.is_null()) {
		return (errcode = DM_DELETE_FAILED);
	}
	if (db->remove(rec) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
close()
{
	if (db.is_null()) {
		return (errcode = DM_CLOSE_FAILED);
	}
	if (db->close() != DM_OK) {
		return (errcode = db->error_code());
	}
	db = (_Tt_db_desc *)0;
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
setuid(_Tt_oid_ptr oid, uid_t uid)
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->setuid(oid, uid) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
setgroup(_Tt_oid_ptr oid, gid_t group)
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->setgroup(oid, group) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
setmode(_Tt_oid_ptr oid, mode_t mode)
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->setmode(oid, mode) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
getaccess(_Tt_oid_ptr oid, uid_t &uid, gid_t &group, mode_t &mode)
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->getaccess(oid, uid, group, mode) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
test_and_set(_Tt_record_ptr rec)
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->test_and_set(rec) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

#ifdef UNUSED
// XXX: Much of this old lib/dm code is apparently not used any more.
// Until we make an organized effort to get rid of it, I will just
// ifdef out things as I find them.  RFM 7/16/93
_Tt_dm_status _Tt_db::
readspec(_Tt_oid_ptr oid, _Tt_prop_array &props)
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->readspec(oid, props) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
writespec(_Tt_oid_ptr oid, _Tt_prop_array props)
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->writespec(oid, props) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
addsession(_Tt_oid_ptr oid, _Tt_string session)
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->addsession(oid, session) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

_Tt_dm_status _Tt_db::
delsession(_Tt_oid_ptr oid, _Tt_string session)
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->delsession(oid, session) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}
#endif /* UNUSED */

_Tt_dm_status _Tt_db::
gettype(_Tt_oid_ptr oid, _Tt_string typeprop, _Tt_string &typename)
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->gettype(oid, typeprop, typename) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

/*
 *  _Tt_db::transaction_write - 'write' a record into the transaction
 *  records buffer.  If succeeds returns DM_OK, otherwise returns DM_ERROR.
 */

_Tt_dm_status _Tt_db::
transaction_write(_Tt_record_ptr rec)
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->transaction_write(rec) != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

/*
 *  _Tt_db::transaction_abort - delete all transaction records in transaction
 *  buffer.  If succeeds returns DM_OK, otherwise returns DM_ERROR.
 */

_Tt_dm_status _Tt_db::
transaction_abort()
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->transaction_abort() != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}

/*
 *  _Tt_db::transaction_commit - sends all the transaction records in
 *  the transaction records buffer to the db server.  If the transaction
 *  succeeds returns DM_OK, otherwise returns DM_ERROR.
 */

_Tt_dm_status _Tt_db::
transaction_commit()
{
	if (db.is_null()) {
		return (errcode = DM_ERROR);
	}
	if (db->transaction_commit() != DM_OK) {
		return (errcode = db->error_code());
	}
	return (errcode = DM_OK);
}
