//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *  Tool Talk Database Manager - dm_record.cc
 *
 *  Copyright (c) 1989 Sun Microsystems, Inc.
 *
 *  This file contains the implementation of the _Tt_record member functions.
 */

#include <dm/dm.h>
#include <dm/dm_record.h>
#include <isam.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <netinet/in.h>     


_Tt_record::
_Tt_record()
{
	reset();
}

_Tt_record::
~_Tt_record()
{
}


void _Tt_record::
reset()
{
	key_id = DM_KEY_NONE;
	key_len = 0;
	_recnum = 0;
	newp = 1;
	fields_set = 0;
}

int _Tt_record::
record_num() const
{
	return _recnum;
}

/*
 *  set_record_num - this function is used to read records by their numbers.
 *  For access control the key must also be provided.  
 */

void _Tt_record::
set_record_num(long int rec_num)
{
	_recnum = rec_num;
	fields_set |= FLD_REC_NUM;
}

/*
 *  set_recnum - set the record number.  For DM use internally.
 */

void _Tt_record::
set_recnum(long int rec_num)
{
	_recnum = rec_num;
}

int _Tt_record::
reclength() const
{
	return rec_len;
}

void _Tt_record::
set_reclength(int len)
{
	rec_len = len;
}

int _Tt_record::
newrecp() const
{
	return newp;
}

void _Tt_record::
setold()
{
	newp = 0;
}

void _Tt_record::
setnew()
{
	newp = 1;
}

// ARCHITECTURE DEPENDENCY (but not very strong)
// We can just store the int value into the record
// since version is on a word (in fact, doubleword)
// boundary.  This works on SPARC which is about as
// picky an architecture as there is.  We do have to
// account for byte order though.


void _Tt_record::
set_version_num(int ver_no)
{
	*((u_long *) bufadr()) = htonl(ver_no);
}

int _Tt_record::
version_num() const
{
	return ntohl(* (u_long *) bufadr());
}

void _Tt_record::
print(FILE *fs) const
{

	fprintf(fs,
		"<%s- rec_len = %d, key_id = %s, key_len = %d,"
		"recnum = %d, newp = %d\n",
		"_Tt_record",
		rec_len, _tt_enumname(key_id), key_len,
		_recnum, newp);

}


char *_Tt_record::
bufadr()
{
	return (char *)0;
}

const char *_Tt_record::
bufadr() const
{
	return (const char *)0;
}

void _Tt_record::
determine_key()
{
}

// It seems wrong that the key should depend on the table; the key_id should
// be enough to determine it.
_Tt_string _Tt_record::
extract_key(_Tt_table_desc_ptr table)
{
	_Tt_key_desc_ptr kd = table->findkeydesc(key_id);
	ASSERT(!kd.is_null(), "can't extract key, key id not found");
	// HACK: if all parts of a key are not contiguous this fails.
	_Tt_string result((unsigned char *)(bufadr()+kd->part_start(0)),key_len);
	return result;
}

