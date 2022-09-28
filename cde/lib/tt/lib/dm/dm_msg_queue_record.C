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
 *  This file contains the implementation of the _Tt_msg_queue_record
 *  member functions.  The layout of a msg queue record is:
 *	<dockey> <id> <part> <body>
 */

#include <stddef.h>
#include <dm/dm_msg_queue_record.h>
#include <dm/dm_recfmts.h>
#include <isam.h>
#include <memory.h>
#include <string.h>
#include <malloc.h>

const int FLD_MSG_QUEUE_DOCKEY	=	0x1;
const int FLD_MSG_QUEUE_ID 	=	0x2;
const int FLD_MSG_QUEUE_PART	=	0x4;
const int FLD_MSG_QUEUE_BODY	=	0x8;

_Tt_msg_queue_record::
_Tt_msg_queue_record()
{
	field = (Table_msg_queue *)malloc(sizeof(Table_msg_queue));
}

_Tt_msg_queue_record::
~_Tt_msg_queue_record()
{
	free((MALLOCTYPE *)field);
}


/*
 * Set key_id and key_len depending on which fields are set.
 */
void _Tt_msg_queue_record::
determine_key()
{
	switch (fields_set) {
	      case 0:
		key_id = DM_KEY_NONE;
		key_len = 0;
		break;
	      case FLD_MSG_QUEUE_DOCKEY:
		key_id = DM_KEY_DOC;
		key_len = sizeof(field->dockey);
		break;
	      case FLD_MSG_QUEUE_ID:
		key_id = DM_KEY_MSG_PART;
		key_len = sizeof(field->id);
		break;
	      case FLD_MSG_QUEUE_ID+FLD_MSG_QUEUE_PART:
		key_id = DM_KEY_MSG_PART;
		key_len = sizeof(field->id)+sizeof(field->part);
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
_Tt_key_ptr _Tt_msg_queue_record::
dockey() const
{
	return new _Tt_key(field->dockey);
}

void _Tt_msg_queue_record::
set_dockey(_Tt_key_ptr dockey)
{
	memcpy(field->dockey, dockey->content(), sizeof(field->dockey));
	fields_set |= FLD_MSG_QUEUE_DOCKEY;
}

// ARCHITECTURE DEPENDENCY (but not very strong)
// We can just store the int values into the record
// since they are on word 
// boundaries.  This works on SPARC which is about as
// picky an architecture as there is.  We do have to
// account for byte order though.

int _Tt_msg_queue_record::
id() const
{
	return ntohl( * (u_long *) &field->id);
}

void _Tt_msg_queue_record::
set_id(int id)
{
	* (u_long *) &field->id = htonl(id);
	fields_set |= FLD_MSG_QUEUE_ID;
}

int _Tt_msg_queue_record::
part() const
{
	return ntohl( * (u_long *) &field->part );
}

void _Tt_msg_queue_record::
set_part(int part)
{
	* (u_long *) &field->part = htonl(part);
	fields_set |= FLD_MSG_QUEUE_PART;
}

_Tt_string _Tt_msg_queue_record::
body() const
{
	_Tt_string s((unsigned char *) &field->body,
		     rec_len - offsetof(Table_msg_queue, body));
	return s;
}

void _Tt_msg_queue_record::
set_body(_Tt_string ibody)
{
	memcpy(field->body, (char *) ibody, ibody.len());
	rec_len = offsetof(Table_msg_queue,body) + ibody.len();
}

int _Tt_msg_queue_record::
max_body_length() const
{
	return sizeof(field->body);
}


void _Tt_msg_queue_record::
print(FILE *fs) const
{
	_Tt_record::print(fs);
	fprintf(fs,
		"dockey:    %s\n"
		"id:        %d\n"
		"part:      %d\n"
		"body:      ",
		(char *)(_Tt_string)*dockey(),
		id(),
		part());
	body().print();
	fprintf(fs,"\n>\n");
		
}


char *_Tt_msg_queue_record::
bufadr()
{
	return (char *)field;
}

const char *_Tt_msg_queue_record::
bufadr() const
{
	return (const char *)field;
}

