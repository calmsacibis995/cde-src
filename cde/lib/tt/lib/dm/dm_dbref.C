//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *  Tool Talk Database Manager (DM) - dm_dbref.cc
 *
 *  Copyright (c) 1989 Sun Microsystems, Inc.
 *
 *  Implementation of the _Tt_dbref member functions.
 */

#include <dm/dm_dbref.h>
#include <dm/dm.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>

const char *namespace_str[] =
{
	"NFS",
	"NSE",
	"STANDALONE",
	"REDIRECT",
	"LAST"
};

_Tt_dbref::
_Tt_dbref()
{
}

_Tt_dbref::
_Tt_dbref(_Tt_name_space ns, const char *path, const char *env)
{
	namespace = ns;
	if (env) {
		envname = env;	/* copy */
	} else {
		envname = _tt_dm->host_name;
	}
	relpath = path;
}

_Tt_dbref::
~_Tt_dbref()
{
}

/* 
 * Return the NFS path for this database directory
 */
_Tt_string _Tt_dbref::
db_path()
{
	_Tt_string path;
	switch (namespace) {
	      case DM_NS_NFS:
		path = envname.cat(":").cat(relpath).cat("TT_DB");
		break;
	      case DM_NS_NSE:
	      case DM_NS_STANDALONE:
	      case DM_NS_REDIRECT:
	      case DM_NS_LAST:
	      default:
		path = 0;
	}
	if (path.len() == 0) {
		return (_Tt_string)0;
	}
	return _tt_dm->resolve_abspath(path);
}

/*
 *  _Tt_string - convert this dbref to a string of the form:
 *	"<namespace>:<envname>:<relpath>"
 */

_Tt_dbref::
operator _Tt_string()
{
	int	elen = envname.len();
	char *path = (char *)malloc(strlen(namespace_str[namespace]) +
				    elen + relpath.len() + 3);
	sprintf(path, "%s:%s:%s", namespace_str[namespace],
		(char *) envname, (char *) relpath);
	_Tt_string s(path);
	free((MALLOCTYPE *)path);
	return s;
}

void _Tt_dbref::
print(FILE *fs) const
{
	fprintf(fs, "dbref - <%s, %s, %s>\n", namespace_str[namespace],
		(char *) envname, (char *) relpath);
}

int _Tt_dbref::
operator==(const _Tt_dbref_ptr dbref) const
{
	if (namespace == dbref->namespace) {
		if (envname == dbref->envname) {
			if (relpath == dbref->relpath) {
				return 1;
			}
		}
	}
	return 0;
}


