//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *  Tool Talk Database Manager (DM) - dm_docoid_path_record.cc
 *
 *  Copyright (c) 1989 Sun Microsystems, Inc.
 *
 *  This file contains the implementation of the _Tt_docoid_path_record
 *  member functions.
 */

#include <dm/dm_docoid_path_record.h>
#include <dm/dm_recfmts.h>
#include <dm/dm.h>
#include <memory.h>
#include <string.h>
#include <isam.h>
#include <stdlib.h>
#include <stddef.h>

const int FLD_DOCOID_PATH_DOCKEY	=	0x1;
const int FLD_DOCOID_PATH_FILEPATH	=	0x2;

// docoid_path is a special table: it has a field (filepath) which
// is both variable and indexed.  Since the indexed portion of a 
// record must be in the minimum record length, we always store at least
// MAX_KEY_LEN bytes in the filepath field, padding with zeroes as
// necessary.

_Tt_docoid_path_record::
_Tt_docoid_path_record()
{
	field = (Table_docoid_path *)malloc(sizeof(Table_docoid_path));
	rec_len = offsetof(Table_docoid_path,filepath) + MAX_KEY_LEN;
}

_Tt_docoid_path_record::
~_Tt_docoid_path_record()
{
	free((MALLOCTYPE *)field);
}


/*
 * Set key_id and key_len depending on which fields are set.
 */
void _Tt_docoid_path_record::
determine_key()
{
	switch (fields_set) {
	      case 0:
		key_id = DM_KEY_NONE;
		key_len = 0;
		break;
	      case FLD_DOCOID_PATH_DOCKEY:
		key_id = DM_KEY_OID;
		key_len = sizeof(field->dockey);
		break;
	      case FLD_DOCOID_PATH_FILEPATH:
		key_id = DM_KEY_PATH;
		key_len = MAX_KEY_LEN;
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


_Tt_key_ptr _Tt_docoid_path_record::
docoid_key() const
{
	return new _Tt_key(field->dockey);
}

void _Tt_docoid_path_record::
set_docoid_key(_Tt_key_ptr oidkey)
{
	memcpy(field->dockey, oidkey->content(), sizeof(field->dockey));
	fields_set |= FLD_DOCOID_PATH_DOCKEY;
}

_Tt_string _Tt_docoid_path_record::
path() const
{
	_Tt_string result;

	if (field->filepath[MAX_KEY_LEN-1] == '\0') {
		// strip padding.
		result = field->filepath;
	} else {
		result.set((unsigned char *)field->filepath,
			   rec_len-offsetof(Table_docoid_path,filepath));
	}
	return result;
}

void _Tt_docoid_path_record::
set_path(_Tt_string _path)
{
	int l;
	memset(field->filepath,0,MAX_KEY_LEN);
	strcpy(field->filepath, (char *)_path);
	l = _path.len();
	if (l<MAX_KEY_LEN) l = MAX_KEY_LEN;
	rec_len = offsetof(Table_docoid_path,filepath)+l;
	fields_set |= FLD_DOCOID_PATH_FILEPATH;
}


void _Tt_docoid_path_record::
print(FILE *fs) const
{
	_Tt_record::print(fs);
	fprintf(fs,
		"dockey:   %s\n"
		"filepath: %s>\n",
		(char *)(_Tt_string)*docoid_key(),
		(char *)path());
}

char *_Tt_docoid_path_record::
bufadr()
{
	return (char *)field;
}

const char *_Tt_docoid_path_record::
bufadr() const
{
	return (const char *)field;
}

