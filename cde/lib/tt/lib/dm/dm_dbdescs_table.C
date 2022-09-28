//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *  Tool Talk Database Manager (DM) - dm_dbdescs_table.cc
 *
 *  Copyright (c) 1989 Sun Microsystems, Inc.
 *
 *  This file contains the implementation of the _Tt_dbdescs_table
 *  member functions.
 */

#include <dm/dm_enums.h>
#include <dm/dm_dbdescs_table.h>
#include <util/tt_assert.h>
#include <string.h>
#include <malloc.h>

/*
 *  class _Tt_dbdesc_table's methods definition
 *
 *  The class _Tt_dbdesc_table implements a type of hash table of database
 *  descriptors.  The hash table is an array of lists of entries.
 *  The hash table uses an open-hashing scheme.  Each hash table entry contains
 *  the information about a database such as its hostname, path, database name,
 *  table name, NetISAM file descriptor associated with the database, etc.
 */


_Tt_dbdescs_table::
_Tt_dbdescs_table()
{
	bucket = (_Tt_db_desc_list_ptr *)calloc(TT_DBDESCS_TABLE_BUCKETS,
						sizeof(_Tt_db_desc_list_ptr));
	for (int i = 0; i < TT_DBDESCS_TABLE_BUCKETS; i++) {
		bucket[i] = new _Tt_db_desc_list();
	}
	_num_isfds = 0;
}

_Tt_dbdescs_table::~_Tt_dbdescs_table()
{
	for (int i = 0; i < TT_DBDESCS_TABLE_BUCKETS; i++) {
		bucket[i] = (_Tt_db_desc_list *)0;
	}
	(void)free((MALLOCTYPE *)bucket);
}

/*
 *  lookup - finds the db descriptor with the given dbref and db_name.  Returns
 *  0 if not found.
 */

_Tt_db_desc_ptr _Tt_dbdescs_table::
lookup(_Tt_dbref_ptr dbref, _Tt_string dbname)
{
	char *db_name = (char *) dbname;
	_Tt_db_desc_list_cursor c(bucket[hash(dbref->envname, dbref->relpath,
					      db_name)]);
	while (c.next()) {
		char *e = c->envname();
		char *p = c->path();
		char *d = c->dbname();
		if ((strcmp(dbref->envname, e) == 0) &&
		    (strcmp(dbref->relpath, p) == 0) &&
		    (strcmp(db_name, d) == 0)) {
			return (*c);
		}
	}
	return 0;
}

/*
 *  insert - creates a new database table descriptor and inserts it into
 *  this database descriptor hash table.  The new database descriptor contains
 *  the host name, path, and the db name.  The pointer to the new database
 *  descriptor is returned.
 */

_Tt_db_desc_ptr _Tt_dbdescs_table::
insert(_Tt_dbref_ptr dbref, _Tt_string dbname)
{
	char *db_name = (char *) dbname;
	_Tt_db_desc_ptr dbdesc = lookup(dbref, db_name);
	if (dbdesc.is_null()) {
		dbdesc = new _Tt_db_desc(dbref, db_name);
		bucket[hash(dbref->envname, dbref->relpath, db_name)]->push(dbdesc);
	}
	return dbdesc;
}

void _Tt_dbdescs_table::
remove(_Tt_db_desc_ptr dbdesc)
{
	_Tt_db_desc_list_cursor c(bucket[hash(dbdesc->envname(), dbdesc->path(),
					      dbdesc->dbname())]);
	while (c.next()) {
		if ((*c).is_eq(dbdesc)) {
			c.remove();
			return;
		}
	}
}

int _Tt_dbdescs_table::
num_isfds()
{
	return _num_isfds;
}

void _Tt_dbdescs_table::
inc_num_isfds()
{
	_num_isfds++;
}

void _Tt_dbdescs_table::
dec_num_isfds()
{
	_num_isfds--;
}

/*
 *  hash - hashes the environment name, path, and db name into a bucket number
 *  and returns the bucket number.  the hash function is:
 *        hash value = (envname + path + db_name) % TT_DBDESCS_TABLE_BUCKETS
 */

int _Tt_dbdescs_table::
hash(_Tt_string envname, _Tt_string path, _Tt_string db_name) const
{
	int	hash_value = 0;
	char	*p = (char *) envname;
	while (*p != NULL_CHAR) {
		hash_value += *p++;
	}
	p = (char *) path;
	while (*p != NULL_CHAR) {
		hash_value += *p++;
	}
	p = (char *) db_name;
	while (*p != NULL_CHAR) {
		hash_value += *p++;
	}
	return (hash_value % TT_DBDESCS_TABLE_BUCKETS);
}


