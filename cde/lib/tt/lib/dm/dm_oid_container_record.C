//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *  Tool Talk Database Manager (DM) - dm_oid_container_record.cc
 *
 *  Copyright (c) 1989 Sun Microsystems, Inc.
 *
 *  This file contains the implementation of the _Tt_oid_container_record
 *  member functions.
 */

#include <memory.h>
//#include <malloc.h>
#include "dm/dm_oid_container_record.h"
#include "dm/dm_recfmts.h"



const int FLD_OID_CONTAINER_OBJKEY = 0x1;
const int FLD_OID_CONTAINER_DOCKEY = 0x2;

_Tt_oid_container_record::
_Tt_oid_container_record()
{
	field = (Table_oid_container *)malloc(sizeof(Table_oid_container));
	rec_len = sizeof(Table_oid_container);
}

_Tt_oid_container_record::
~_Tt_oid_container_record()
{
	free((MALLOCTYPE *)field);
}


/*
 * Set key_id and key_len depending on which fields are set.
 */
void _Tt_oid_container_record::
determine_key()
{
	switch (fields_set) {
	      case 0:
		key_id = DM_KEY_NONE;
		key_len = 0;
		break;
	      case FLD_OID_CONTAINER_OBJKEY:
		key_id = DM_KEY_OID;
		key_len = sizeof(field->objkey);
		break;
	      case FLD_OID_CONTAINER_DOCKEY:
		key_id = DM_KEY_DOC;
		key_len = sizeof(field->dockey);
		break;
	      case FLD_OID_CONTAINER_OBJKEY+FLD_OID_CONTAINER_DOCKEY:
		key_id = DM_KEY_OID;
		key_len = sizeof(field->objkey) + sizeof(field->dockey);
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

_Tt_key_ptr _Tt_oid_container_record::
oidkey() const
{
	return new _Tt_key(field->objkey);
}

void _Tt_oid_container_record::
set_oidkey(_Tt_key_ptr oidkey)
{
	memcpy(field->objkey, oidkey->content(), sizeof(field->objkey));
	fields_set |= FLD_OID_CONTAINER_OBJKEY;
}

_Tt_key_ptr _Tt_oid_container_record::
dockey() const
{
	return new _Tt_key(field->dockey);
}

void _Tt_oid_container_record::
set_dockey(_Tt_key_ptr dockey)
{
	memcpy(field->dockey, dockey->content(), sizeof(field->dockey));
	fields_set |= FLD_OID_CONTAINER_DOCKEY;
}

void _Tt_oid_container_record::
print(FILE *fs) const
{
	char		*okey =	(char *)(_Tt_string)*oidkey();

	_Tt_record::print(fs);
	fprintf(fs,
		"objkey:    %s\n"
		"dockey:    %s>\n",
		okey,
		(char *)(_Tt_string)*dockey());
}


char *_Tt_oid_container_record::
bufadr()
{
	return (char *)field;
}

const char *_Tt_oid_container_record::
bufadr() const
{
	return (const char *)field;
}

