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
 *  This file contains the implementation of the _Tt_oid_access_record
 *  member functions.
 */

#include <dm/dm_oid_access_record.h>
#include <dm/dm_recfmts.h>     
#include <memory.h>
#include <malloc.h>
#include <sys/types.h>
#include <netinet/in.h>     

const int FLD_OID_ACCESS_OBJKEY = 0x1;
const int FLD_OID_ACCESS_OWNER = 0x2;     

_Tt_oid_access_record::
_Tt_oid_access_record()
{
	field = (Table_oid_access *)malloc(sizeof(Table_oid_access));
	rec_len = sizeof(Table_oid_access);
}

_Tt_oid_access_record::
~_Tt_oid_access_record()
{
	free((MALLOCTYPE *)field);
}


/*
 * Set key_id and key_len depending on which fields are set.
 */
void _Tt_oid_access_record::
determine_key()
{
	switch (fields_set) {
	      case 0:
		key_id = DM_KEY_NONE;
		key_len = 0;
		break;
	      case FLD_OID_ACCESS_OBJKEY:
		key_id = DM_KEY_OID;
		key_len = sizeof(field->objkey);
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


_Tt_key_ptr _Tt_oid_access_record::
oidkey() const
{
	return new _Tt_key(field->objkey);
}

void _Tt_oid_access_record::
set_oidkey(_Tt_key_ptr oidkey)
{
	memcpy(field->objkey, oidkey->content(), sizeof(field->objkey));
	fields_set |= FLD_OID_ACCESS_OBJKEY;
}

uid_t _Tt_oid_access_record::
owner() const
{
	short n_uid;
	memcpy((char *)&n_uid, (char *)&field->owner, sizeof(n_uid));
	return (uid_t)ntohs(n_uid);
}

void 
_Tt_oid_access_record::
set_owner(uid_t _owner)
{
	short n_uid = (short)_owner;
	memcpy((char *)&field->owner, (char *)htons(n_uid), sizeof(n_uid));
	fields_set |= FLD_OID_ACCESS_OWNER;
}

gid_t _Tt_oid_access_record::
group() const
{
	short n_gid;
	memcpy((char *)&n_gid, (char *)&field->group, sizeof(n_gid));
	return (gid_t)ntohs(n_gid);
}

void 
_Tt_oid_access_record::
set_group(gid_t _group)
{
	short n_gid = (short)_group;
	memcpy((char *)&field->group, (char *)htons(n_gid), sizeof(n_gid));
	fields_set |= FLD_OID_ACCESS_OWNER;
}

mode_t _Tt_oid_access_record::
mode() const
{
	short n_mode;
	memcpy((char *)&n_mode, (char *)&field->mode, sizeof(n_mode));
	return (mode_t)ntohs(n_mode);
}

void 
_Tt_oid_access_record::
set_mode(mode_t _mode)
{
	short n_mode = (short)_mode;
	memcpy((char *)&field->mode, (char *)htonl(n_mode), sizeof(n_mode));
	fields_set |= FLD_OID_ACCESS_OWNER;
}

void _Tt_oid_access_record::
print(FILE *fs) const
{
	_Tt_record::print(fs);
	fprintf(fs,
		"objkey:    %s\n"
		"owner:     %d\n",
		(char *)(_Tt_string)*oidkey(),
		owner());
}


char *_Tt_oid_access_record::
bufadr()
{
	return (char *)field;
}

const char *_Tt_oid_access_record::
bufadr() const
{
	return (const char *)field;
}
