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
 *  This file contains the implementation of the _Tt_oid_prop_record
 *  member functions.
 */

#include <dm/dm_oid_prop_record.h>
#include <dm/dm_recfmts.h>
#include <dm/dm_key.h>
#include <dm/dm.h>
#include <isam.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

const int FLD_OID_PROP_OBJKEY 	= 	0x1;
const int FLD_OID_PROP_PROPNAME	= 	0x2;
const int FLD_OID_PROP_PROPVAL	=	0x4;

_Tt_oid_prop_record::
_Tt_oid_prop_record()
{
	field = (Table_oid_prop *)malloc(sizeof(Table_oid_prop));
	rec_len = offsetof(Table_oid_prop,propval);
}

_Tt_oid_prop_record::
~_Tt_oid_prop_record()
{
	free((MALLOCTYPE *)field);
}

/*
 * Set key_id and key_len depending on which fields are set.
 */
void _Tt_oid_prop_record::
determine_key()
{
	switch (fields_set) {
	      case 0:
		key_id = DM_KEY_NONE;
		key_len = 0;
		break;
	      case FLD_OID_PROP_OBJKEY:
		key_id = DM_KEY_OID_PROP;
		key_len = sizeof(field->objkey);
		break;
	      case FLD_OID_PROP_OBJKEY+FLD_OID_PROP_PROPNAME:
	      case FLD_OID_PROP_OBJKEY+FLD_OID_PROP_PROPNAME+FLD_OID_PROP_PROPVAL:
		key_id = DM_KEY_OID_PROP;
		key_len = sizeof(field->objkey) + sizeof(field->propname);
		break;
	      default:
		if (fields_set & FLD_REC_NUM) {
			key_id = DM_KEY_NONE;
			key_len = 0;
		} else {
			key_id = DM_KEY_UNDEFINED;
			key_len = 0;
		}
		break;
	}
}


_Tt_key_ptr _Tt_oid_prop_record::
oidkey() const
{
	return new _Tt_key(field->objkey);
}

void _Tt_oid_prop_record::
set_oidkey(_Tt_key_ptr oidkey)
{
	memcpy(field->objkey, oidkey->content(), sizeof(field->objkey));
	fields_set |= FLD_OID_PROP_OBJKEY;
}

_Tt_string _Tt_oid_prop_record::
propname() const
{
	_Tt_string result;
	int l;
	if (field->propname[sizeof(field->propname)-1]!=NULL_CHAR) {
		// name is of maximal length, has no null at end
		l = sizeof(field->propname);
	} else {
		l = strlen(field->propname);
	}
	result.set((unsigned char *)field->propname,l);
	return result;
}

void _Tt_oid_prop_record::
set_propname(_Tt_string prop_name)
{
	memset(field->propname,0,sizeof(field->propname));
	strncpy(field->propname, (char *) prop_name, sizeof(field->propname));
	fields_set |= FLD_OID_PROP_PROPNAME;
}

_Tt_string _Tt_oid_prop_record::
value() const
{
	_Tt_string result;
	result.set((unsigned char *)(field->propval),
		   rec_len - (offsetof(Table_oid_prop,propval)));
	return result;
}

void _Tt_oid_prop_record::
set_value(_Tt_string _value)
{
	memcpy(field->propval, (char *)_value, _value.len());
	fields_set |= FLD_OID_PROP_PROPVAL;
	rec_len = offsetof(Table_oid_prop,propval) + _value.len();
};



void _Tt_oid_prop_record::
print(FILE *fs) const
{
	char *ok = (char *)(_Tt_string)*oidkey();
	char *pn = (char *)propname();

	if (!ok) ok = "(nil)";
	if (!pn) pn = "(nil)";
	
	_Tt_record::print(fs);
	fprintf(fs,
		"objkey:   %s\n"
		"propname:  %s\n"
		"value:     ",
		ok, pn);
	value().print(fs);
	fprintf(fs,"\n>\n");
}


char *_Tt_oid_prop_record::
bufadr()
{
	return (char *)field;
}

const char *_Tt_oid_prop_record::
bufadr() const
{
	return (const char *)field;
}

