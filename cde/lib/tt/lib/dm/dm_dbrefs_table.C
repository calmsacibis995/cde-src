//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *  Tool Talk Database Manager (DM) - dm_dbrefs_table.cc
 *
 *  Copyright (c) 1989 Sun Microsystems, Inc.
 *
 *  Implementation of the class _Tt_dbrefs_table's member functions.
 */

#include <dm/dm_dbrefs_table.h>
#include <dm/dm.h>
#include <util/tt_assert.h>
#include <string.h>
#include <malloc.h>

/*
 *  The class _Tt_dbrefs_table implements a type of hash table of database
 *  references.  The hash table is an array of lists of entries.
 *  The hash table uses an open-hashing scheme.
 */

_Tt_dbrefs_table::
_Tt_dbrefs_table()
{
	bucket = (_Tt_dbref_list_ptr *)calloc(TT_DBREFS_TABLE_BUCKETS,
					      sizeof(_Tt_dbref_list_ptr));
	for (int i = 0; i < TT_DBREFS_TABLE_BUCKETS; i++) {
		bucket[i] = new _Tt_dbref_list();
	}
}

_Tt_dbrefs_table::
~_Tt_dbrefs_table()
{
	for (int i = 0; i < TT_DBREFS_TABLE_BUCKETS; i++) {
		bucket[i] = (_Tt_dbref_list *)0;
	}
	(void)free((MALLOCTYPE *)bucket);
}

/*
 *  lookup - finds the db reference with the given envname and relpath.
 *  Returns NULL if not found.
 */

_Tt_dbref_ptr _Tt_dbrefs_table::
lookup(_Tt_name_space ns, const char *env, const char *path)
{
	_Tt_dbref_list_cursor c(bucket[hash(env, path)]);
	if (c.is_valid()) {  // for ultrix
		while (c.next()) {
			if (strcmp(c->envname, (char *)env) == 0) { 
				if (strcmp(c->relpath, (char *)path) == 0) {
					if (c->namespace == ns) {
						return (*c);
					}
				}
			}
		}
	}
	return (_Tt_dbref *)0;
}

/*
 *  insert - creates a new database reference and inserts it into this database
 *  reference hash table.  The new database reference contains the environment
 *  name and relative path.  The namespace is the given namespace.  The pointer
 *  to the new database reference is returned.
 *  If 'env' is null, it defaults to this host name.
 *  If 'ns' is null, use the default name space.
 */

_Tt_dbref_ptr _Tt_dbrefs_table::
insert(_Tt_string ns, _Tt_string env, _Tt_string path)
{
	_Tt_name_space nsv;
	_Tt_string appended_path;

	if (ns.len()!=0) {
		// Look up namespace in table.
		for (nsv = (_Tt_name_space)0;
		     nsv < DM_NS_LAST && ns!=namespace_str[nsv];
		     nsv = (_Tt_name_space)(nsv + 1)) {};
		if (nsv==DM_NS_LAST) {
			if (DM_DEBUG(1)) {
				fprintf(stderr,
					"DM: unknown namespace %s\n",
					(char *)ns);
			}
			return (_Tt_dbref *)0;
		}
	} else {
		// apply default for namespace.
		nsv = _tt_dm->namespace;
	}
	if (env.len()==0) {
		env = _tt_dm->host_name;
	}

	/* add '/' at the end for uniformity with root dir */
		
	if (path.right(1) == "/") {
		appended_path = path;
	} else {
		appended_path = path.cat("/");
	}

	_Tt_dbref_ptr dbref = lookup(nsv, env, appended_path);
	if (dbref.is_null()) {
		if (DM_DEBUG(1)) {
			fprintf(stderr,"DM: new dbref: ns = \"%s\", "
				"env = \"%s\", path = \"%s\"\n",
				namespace_str[nsv],
				(char *)env, (char *)appended_path);
		}
		dbref = new _Tt_dbref(nsv, appended_path, env);
		bucket[hash(env, appended_path)]->push(dbref);
	}

	return dbref;
}

/*
 *  insert - creates a new database reference and inserts it into this database
 *  reference hash table.  The new database reference contains the name space,
 *  environment name and path.  The pointer to the new database reference is
 *  returned.  The input string is of the form "<namespace>:<env>:<path>".
 *  Returns DM_CONVERSION_ERROR if cannot convert the input string to a dbref.
 */

_Tt_dm_status _Tt_dbrefs_table::
insert(_Tt_string dbrefstr, _Tt_dbref_ptr &dbref)
{
	_Tt_string namespace, env, path;
	path = dbrefstr.split(':',namespace);
	path = path.split(':',env);
	dbref = insert(namespace, env, path);
	return DM_OK;
}

/*
 *  hash - hashes the environment name and relative path into a bucket number
 *  and returns the bucket number.  the hash function is:
 *        hash value = (envname + relpath) % TT_DBREFS_TABLE_BUCKETS
 */

int _Tt_dbrefs_table::
hash(const char *envname, const char *relpath) const
{
	int	hash_value = 0;
	const char *p =  (char *)envname;
	while (*p != NULL_CHAR) {
		hash_value += *p++;
	}

	p = (char *)relpath;
	while (*p != NULL_CHAR) {
		hash_value += *p++;
	}
	return (hash_value % TT_DBREFS_TABLE_BUCKETS);
}


