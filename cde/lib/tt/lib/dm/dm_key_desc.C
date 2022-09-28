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
 *  This file contains the implementation of the _Tt_db_desc member functions.
 */

#include "dm/dm_key_desc.h"
#include <isam.h>
#include <malloc.h>     

_Tt_key_desc::
~_Tt_key_desc()
{
	free((MALLOCTYPE *)kd);
}


_Tt_key_desc::
_Tt_key_desc(_Tt_key_id key_id)
{
	id = key_id;
	kd = (keydesc *)malloc(sizeof(keydesc));
	kd->k_flags = ISNODUPS;
	kd->k_nparts = 0;		/* no key */
}

_Tt_key_id _Tt_key_desc::
keyid() const
{
	return id;
}

const keydesc * _Tt_key_desc::
key_desc() const
{
	return kd;
}

keydesc * _Tt_key_desc::
key_desc()
{
	return kd;
}

int _Tt_key_desc::
dupsp() const
{
	return (kd->k_flags == ISDUPS);
}

void _Tt_key_desc::
setdups()
{
	kd->k_flags = ISDUPS;
}

void _Tt_key_desc::
setnodups()
{
	kd->k_flags = ISNODUPS;
}

short _Tt_key_desc::
num_parts() const
{
	return kd->k_nparts;
}

void _Tt_key_desc::
setnumparts(short nparts)
{
	kd->k_nparts = nparts;
}

void _Tt_key_desc::
setkeypart(short part_no, short kp_start, short kp_len, short kp_type)
{
	kd->k_part[part_no].kp_start = kp_start;
	kd->k_part[part_no].kp_leng = kp_len;
	kd->k_part[part_no].kp_type = kp_type;
}

short _Tt_key_desc::
part_start(short part_no) const
{
	return kd->k_part[part_no].kp_start;
}

short _Tt_key_desc::
part_len(short part_no) const
{
	return kd->k_part[part_no].kp_leng;
}

short _Tt_key_desc::
part_type(short part_no) const
{
	return kd->k_part[part_no].kp_type;
}

short _Tt_key_desc::
maxlength() const
{
	short max_len = 0;
	for (int i = 0; i < kd->k_nparts ; i++) {
		max_len += kd->k_part[i].kp_leng;
	}
	return max_len;
}

void _Tt_key_desc::
print(FILE *fs) const
{
	fprintf(fs, " -*- key desc <id = %d, flags = %d, num parts = %d> -*-\n",
		id, kd->k_flags, kd->k_nparts);
	for (int i = 0; i < kd->k_nparts; i++) {
		fprintf(fs, "    [%d] - start = %d, len = %d, type = %d\n",
			i, kd->k_part[i].kp_start, kd->k_part[i].kp_leng,
			kd->k_part[i].kp_type);
	}

}


