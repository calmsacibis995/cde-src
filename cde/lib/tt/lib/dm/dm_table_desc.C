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
 *  This file contains the implementation of the _Tt_table_desc member
 *  functions.
 */

#include <dm/dm_table_desc.h>
#include <dm/dm.h>
#include <util/tt_assert.h>
#include <memory.h>


_Tt_table_desc::
_Tt_table_desc()
{
}

_Tt_table_desc::
_Tt_table_desc(_Tt_dbtable_id table_id, Recfmt r, short min_len)
{
	id = table_id;
	recfmt = r;
	min_rec_len = min_len;
	_keys = new _Tt_key_desc_list();
}

_Tt_table_desc::
~_Tt_table_desc()
{
}

_Tt_dbtable_id _Tt_table_desc::
tableid() const
{
	return id;
}

int _Tt_table_desc::
variable_recp() const
{
	return recfmt==RECFMT_V;
}

short _Tt_table_desc::
minreclen() const
{
	return min_rec_len;
}

_Tt_key_desc_list_ptr _Tt_table_desc::
keys()
{
	return _keys;
}

/*
 *  findkeydesc - this function takes the name of a key belongs to this table,
 *  finds and returns the key descriptor associated with the given key name.
 *  If the key is DM_KEY_NONE, _tt_dm->no_key_desc is returned.
 *  NULL is returned if the given key name is not found.
 */

_Tt_key_desc_ptr _Tt_table_desc::
findkeydesc(_Tt_key_id key_id)
{
	if (key_id==DM_KEY_NONE) {
		return _tt_dm->no_key_desc;
	}
	
	_Tt_key_desc_list_cursor c(_keys);
	while (c.next()) {
		if (c->keyid() == key_id) {
			return (*c);
		}
	}
	return 0;
}

int _Tt_table_desc::
primarykeyp() const
{
	
	return (_keys->count() == 1);
}

void _Tt_table_desc::
appendkey(_Tt_key_desc_ptr key)
{
	_keys->append(key);
}


