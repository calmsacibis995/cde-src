//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 * ToolTalk Database Manager - dm_oid.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Implementation of class _Tt_oid
 */

#include <dm/dm_oid.h>
#include <dm/dm.h>
#include <memory.h>
#include <string.h>

_Tt_dm_status _Tt_oid::errcode = DM_OK;

_Tt_oid::
_Tt_oid()
{
}

_Tt_oid::
_Tt_oid(_Tt_dbref_ptr dbref)
{
	_key = new _Tt_key((short) DEFAULT_KEY_VERSION);
	_dbref = dbref;
}

_Tt_oid::
_Tt_oid(_Tt_dbref_ptr dbref, _Tt_key_ptr key)
{
	_dbref = dbref;
	_key = key;
}

_Tt_oid::
~_Tt_oid()
{
}

/*
 *  open - converts an oid string in the form "<key>:<dbref>" to an oid object,
 *  where the <dbref> is of the form "<ns>:<env>:<path>".  If successful, returns
 *  DM_OK.  Otherwise, returns DM_CONVERSION_ERROR.
 */

_Tt_dm_status _Tt_oid::
open(_Tt_string oid_str)
{
	_Tt_string keystr, dbrefstr;
	// HACK: for compatibility, check for "old style" oid strings
	// with the 16-byte binary key in front.  Eventually, all these
	// should go away and be replaced with the true string form.
	// RFM 12/17/90
	if (oid_str.len()>0 && oid_str[0]=='\0') {
		// old style oid string
		if (DM_DEBUG(1)) {
			fprintf(stderr,
				"DM: %s:%d: WARNING: old style oid string:\n",
				__FILE__, __LINE__);
			oid_str->print(stderr);
		}
		keystr = oid_str.left(OID_KEY_LENGTH);
		dbrefstr = oid_str.right(oid_str.len()-OID_KEY_LENGTH-1);
	} else {
		// proper, new style oid string
		dbrefstr = oid_str.split(':',keystr);
	}
	if (keystr.len()==0 || dbrefstr.len()==0) {
		if (DM_DEBUG(1)) {
			fprintf(stderr,
				"DM: couldn't convert oid string %s to oid.\n",
				(char *)oid_str);
		}
		return DM_CONVERSION_ERROR;
	}
	// HACK: Should do more checking: keystr->key conversion *could* fail.
	_key = new _Tt_key(keystr);
	errcode = _tt_dm->create_dbref(dbrefstr, _dbref);
	if (DM_DEBUG(1) && errcode != DM_OK) {
		fprintf(stderr,
			"DM: couldn't convert oid string %s to dbref.  "
			"Returning %s.\n",
			(char *)oid_str,
			_tt_enumname(errcode));
	}
	return errcode;
}

/*
 *  create - creates an oid from a relative path.  Returns one of the codes:
 *	DM_OK  -  successful
 *	DM_PATHMAP_FAILED - path mapping failed
 */

_Tt_dm_status _Tt_oid::
create(_Tt_string path)
{
	_key = new _Tt_key((short) DEFAULT_KEY_VERSION);
	/* maps from file path to dbref (database host:path partition) */
	if (_tt_dm->docpath_to_dbref(path, _dbref) == DM_OK) {
		return (errcode = DM_OK);
	} else {
		return (errcode = _tt_dm->error_code());
	}
	
}

/*
 *  create - creates an oid from a relative path and a specified key.  Returns one
 *  of the codes:
 *	DM_OK  -  successful
 *	DM_PATHMAP_FAILED - path mapping failed
 */

_Tt_dm_status _Tt_oid::
create(_Tt_string path, _Tt_key_ptr key)
{
	_key = key;
	/* maps from file path to dbref (database host:path partition) */
	if (_tt_dm->docpath_to_dbref(path, _dbref) == DM_OK) {
		return (errcode = DM_OK);
	} else {
		return (errcode = _tt_dm->error_code());
	}
	
}

_Tt_dbref_ptr _Tt_oid::
dbref()
{
	return _dbref;
}

_Tt_key_ptr _Tt_oid::
key() const
{
	return _key;
}

// HACK: Not implemented.  Not clear why this class should have it..
//_Tt_string
//_Tt_oid::otype()
//{
//}

/*
 *  _Tt_string - converts an oid to a string.  The equivalent string
 *  has the following layout:
 *	"<key>:<dbref>"
 */

_Tt_oid::
operator _Tt_string()
{
	_Tt_string s,d;
	s = _Tt_string(*_key);
	d = _Tt_string(*_dbref);
	s = s.cat(":");
	s = s.cat(d);
	return s;
}

int _Tt_oid::
operator==(const _Tt_oid_ptr nodeid) const
{
	_Tt_key		*kptr = _key.c_pointer();

	return *kptr == *(nodeid->_key);
}

void _Tt_oid::
print(FILE *fs) const
{
	fprintf(fs, "oid - {\n       ");
	if (_dbref.is_null()) {
		fprintf(fs, "dbref - null\n       ");
	} else {
		_dbref->print(fs);
	}
	fprintf(fs, "       ");
	_key->print(fs);
	fprintf(fs, "      }\n");
}


