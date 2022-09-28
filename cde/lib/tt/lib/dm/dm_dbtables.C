//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *  Tool Talk Database Manager
 *
 *  Copyright (c) 1989 Sun Microsystems, Inc.
 *
 *  This file contains the implementation of the _Tt_dbtables
 *  member functions.
 */

#include <stddef.h>
#include <dm/dm.h>
#include <dm/dm_dbtables.h>
#include <dm/dm_key_desc.h>

#include <dm/dm_recfmts.h>

/*
 *  class _Tt_dbtables's methods definition
 *
 *  The class _Tt_dbtables implements a type which is an array of pointers to
 *  database table descriptors.  The array is indexed by enum constants defined
 *  by _Tt_dbtable_id.  The constructor initalizes the table of database tables
 *  with Tool Talk predefined tables.  A table descriptor is created for each
 *  predefined database table and is inserted into the array of database tables.
 *  The following database tables are created by this initialization function:
 *
 *	table oid_prop {
 *		indexed oid_prop {
 *			oid_key: OID_KEY_LENGTH;
 *			prop:	 MAX_PROP_LENGTH;
 *		}
 *		propvalue: variable;
 *	}
 *
 *	table oid_access {
 *		indexed oid_key: OID_KEY_LENGTH;
 *		owner: 2;  // uid_t
 *		group: 2;  // gid_t
 *		access: 2; // mode_t
 *	}
 *
 *	table oid_container {
 *		indexed oid_key: OID_KEY_LENGTH;
 *		indexed doc_key: OID_KEY_LENGTH;
 *	}
 *
 *	table docoid_path {
 *		indexed doc_key: OID_KEY_LENGTH;
 *		indexed path:    PATH_LENGTH;
 *	}
 *
 *	table msg_queue {
 *		indexed dockey:	OID_KEY_LENGTH;
 *		indexed msg_part {
 *			id:   MSG_ID_LENGTH;
 *			part: MSG_PART_LENGTH;
 *		}
 *		body: variable;
 *	}
 *
 *	table link_ends {
 *		indexed linked_dir {
 *			end_key: OID_KEY_LENGTH;
 *			dir: 1;
 *		}
 *		indexed link_key:	OID_KEY_LENGTH;
 *		linked_obj_handle:	variable;
 *		link_handle:		variable;
 *	}
 *
 *	table link_prop {
 *		indexed link_prop {
 *			link_key: OID_KEY_LENGTH;
 *			prop:	  MAX_PROP_LENGTH;
 *		}
 *		value: variable;
 *	}
 *
 *	table link_access {
 *		indexed link_key: OID_KEY_LENGTH;
 *		owner: 2;  // uid_t
 *		group: 2;  // gid_t
 *		access: 2; // mode_t
 *	}
 *
 */


_Tt_dbtables::
_Tt_dbtables()
{
	
	_Tt_table_desc	*tbl;
	_Tt_key_desc	*key;
	
	table = (_Tt_table_desc_ptr *)calloc(DM_TABLE_LAST,
					     sizeof(_Tt_table_desc_ptr));

	/* the OID_PROP table */
	tbl = new _Tt_table_desc(DM_TABLE_OID_PROP, _Tt_table_desc::RECFMT_V,
				   offsetof(Table_oid_prop,propval));
	key = new _Tt_key_desc(DM_KEY_OID_PROP);
	key->setdups();	/* duplicates for multiple-valued properties */
	key->setnumparts(2);
	key->setkeypart(0, offsetof(Table_oid_prop,objkey),
			sizeof(((Table_oid_prop *)0)->objkey), BINTYPE);
	key->setkeypart(1, offsetof(Table_oid_prop,propname),
			sizeof(((Table_oid_prop *)0)->propname), CHARTYPE);
	tbl->appendkey(key);
	insert(DM_TABLE_OID_PROP, tbl);
	
	/* the OID_ACCESS table */
	tbl = new _Tt_table_desc(DM_TABLE_OID_ACCESS, _Tt_table_desc::RECFMT_F,
				   sizeof(Table_oid_access));
	key = new _Tt_key_desc(DM_KEY_OID);
	key->setnodups();
	key->setnumparts(1);
	key->setkeypart(0, offsetof(Table_oid_access,objkey),
			sizeof(((Table_oid_access *)0)->objkey), BINTYPE);
	tbl->appendkey(key);
	insert(DM_TABLE_OID_ACCESS, tbl);
	
	/* the OID_CONTAINER table */
	tbl = new _Tt_table_desc(DM_TABLE_OID_CONTAINER,
				   _Tt_table_desc::RECFMT_F,
				   sizeof(Table_oid_container));
	key = new _Tt_key_desc(DM_KEY_OID);
	key->setnodups();
	key->setnumparts(1);
	key->setkeypart(0, offsetof(Table_oid_container,objkey),
			sizeof(((Table_oid_container *)0)->objkey), BINTYPE);
	tbl->appendkey(key);
	key = new _Tt_key_desc(DM_KEY_DOC);
	key->setdups();
	key->setnumparts(1);
	key->setkeypart(0, offsetof(Table_oid_container, dockey),
			sizeof(((Table_oid_container *)0)->dockey), BINTYPE);
	tbl->appendkey(key);
	insert(DM_TABLE_OID_CONTAINER, tbl);
	
	/* the DOCOID_PATH table */
	// special case: filepath is both variable in size
	// and indexed.  The indexes all have to be in the minimum
	// record size, so we always allocate at least MAX_KEY_LEN bytes
	// for filepath.
	tbl = new _Tt_table_desc(DM_TABLE_DOCOID_PATH,
				   _Tt_table_desc::RECFMT_V,
				   offsetof(Table_docoid_path,filepath) +
				   MAX_KEY_LEN);
	key = new _Tt_key_desc(DM_KEY_OID);
	key->setnodups();
	key->setnumparts(1);
	key->setkeypart(0, offsetof(Table_docoid_path,dockey),
			sizeof(((Table_docoid_path *)0)->dockey), BINTYPE);
	tbl->appendkey(key);
	key = new _Tt_key_desc(DM_KEY_PATH);
	key->setnodups();
	key->setnumparts(1);
	key->setkeypart(0, offsetof(Table_docoid_path,filepath),
			MAX_KEY_LEN, CHARTYPE);
	tbl->appendkey(key);
	insert(DM_TABLE_DOCOID_PATH, tbl);
	
	/* the MSG_QUEUE table */
	tbl = new _Tt_table_desc(DM_TABLE_MSG_QUEUE,
				   _Tt_table_desc::RECFMT_V,
				   offsetof(Table_msg_queue,body));
	key = new _Tt_key_desc(DM_KEY_DOC);
	key->setdups();
	key->setnumparts(1);
	key->setkeypart(0, offsetof(Table_msg_queue,dockey),
			sizeof(((Table_msg_queue *)0)->dockey), BINTYPE);
	tbl->appendkey(key);
	key = new _Tt_key_desc(DM_KEY_MSG_PART);
	key->setnodups();
	key->setnumparts(2);
	key->setkeypart(0, offsetof(Table_msg_queue,id),
			sizeof(((Table_msg_queue *)0)->id), LONGTYPE);
	key->setkeypart(1, offsetof(Table_msg_queue,part),
			sizeof(((Table_msg_queue *)0)->part), LONGTYPE);
	tbl->appendkey(key);
	insert(DM_TABLE_MSG_QUEUE, tbl);
	
	/* the LINK_ENDS table */
	tbl = new _Tt_table_desc(DM_TABLE_LINK_ENDS, _Tt_table_desc::RECFMT_V,
				 offsetof(Table_link_ends,dbrefs)+2);
	// always have at least 2 null bytes in dbrefs
	key = new _Tt_key_desc(DM_KEY_LINK_DIR);
	key->setdups();
	key->setnumparts(2);
	key->setkeypart(0, offsetof(Table_link_ends,linkkey),
			sizeof(((Table_link_ends *)0)->linkkey), BINTYPE);
	key->setkeypart(1, offsetof(Table_link_ends,direction),
			sizeof(((Table_link_ends *)0)->direction), BINTYPE);
	tbl->appendkey(key);
	key = new _Tt_key_desc(DM_KEY_END);
	key->setdups();
	key->setnumparts(1);
	key->setkeypart(0, offsetof(Table_link_ends,endkey),
			sizeof(((Table_link_ends *)0)->endkey), BINTYPE);
	tbl->appendkey(key);
	insert(DM_TABLE_LINK_ENDS, tbl);
	
	/* The LINK_PROP table */
	tbl = new _Tt_table_desc(DM_TABLE_LINK_PROP, _Tt_table_desc::RECFMT_V,
				 offsetof(Table_link_prop,propval));
	key = new _Tt_key_desc(DM_KEY_LINK_PROP);
	key->setdups();
	key->setnumparts(2);
	key->setkeypart(0, offsetof(Table_link_prop,linkkey),
			sizeof(((Table_link_prop *)0)->linkkey), BINTYPE);
	key->setkeypart(1, offsetof(Table_link_prop,propname),
			sizeof(((Table_link_prop *)0)->propname), CHARTYPE);
	tbl->appendkey(key);
	insert(DM_TABLE_LINK_PROP, tbl);
	
	/* the LINK_ACCESS table */
	tbl = new _Tt_table_desc(DM_TABLE_LINK_ACCESS, _Tt_table_desc::RECFMT_F,
				   sizeof(Table_link_access));
	key = new _Tt_key_desc(DM_KEY_OID);
	key->setnodups();
	key->setnumparts(1);
	key->setkeypart(0, offsetof(Table_link_access,linkkey),
			sizeof(((Table_link_access *)0)->linkkey), BINTYPE);
	tbl->appendkey(key);
	insert(DM_TABLE_LINK_ACCESS, tbl);
}

_Tt_dbtables::
~_Tt_dbtables()
{
	for (int i=0; i < DM_TABLE_LAST; i++) {
		table[i] = (_Tt_table_desc *)0;
	}
	(void)free((MALLOCTYPE *)table);
}


/*
 *  lookup - finds the database table descriptor with the given table id in
 *  the table of database tables.
 */

	_Tt_table_desc_ptr _Tt_dbtables::
	lookup(_Tt_dbtable_id table_id) const
{
	return table[table_id];
}

/*
 *  insert - inserts a database table descriptor with the given id in the table
 *  of database tables.
 */

void _Tt_dbtables::
insert(_Tt_dbtable_id table_id, _Tt_table_desc_ptr table_desc)
{
	table[table_id] = table_desc;
}


