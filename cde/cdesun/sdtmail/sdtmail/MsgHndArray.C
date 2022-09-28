/*
 *+SNOTICE
 *
 *	$Revision: 1.2 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)MsgHndArray.C	1.23 05/20/97"
#endif

#include <SDtMail/Sdtmail.hh>
#include "MsgHndArray.hh"
#include "MemUtils.hh"

MsgStruct*
MsgHndArray::at(int indx)
{
    return(_contents[indx]);
}

void
MsgHndArray::clear()
{
    memset(_contents, 0, sizeof(MsgStruct *)*_size);
}

int
MsgHndArray::length()
{
    return(_num_elements);
}

int
MsgHndArray::index(MsgStruct *a_msg_struct)
{
    int tmp;

    for (tmp = 0; tmp < _num_elements; tmp++)
      {
	  if ((_contents[tmp]->sessionNumber == a_msg_struct->sessionNumber) &&
	      (_contents[tmp]->message_handle == a_msg_struct->message_handle))
	    {
		return(tmp);
	    }
      }
    return(-1);
}

int
MsgHndArray::index(SdmMessageNumber a_message_handle)
{
    int tmp;

    for (tmp = 0; tmp < _num_elements; tmp++)
      {
	  if (_contents[tmp]->message_handle == a_message_handle)
	    {
		return(tmp);
	    }
      }
    return(-1);
}

void
MsgHndArray::remove_and_free_all_elements()
{
    int i;

    for (i=0; i<_num_elements; i++) {
	delete _contents[i];
	_contents[i] = NULL;
    }
    _num_elements = 0;
}

void
MsgHndArray::remove_all_elements()
{
    int i;

    for (i=0; i<_num_elements; i++) {
	_contents[i] = NULL;
    }
    _num_elements = 0;
}

void
MsgHndArray::remove_entry(
    int position
)
{
    int i;

    if ((position < 0) || (position >= _num_elements)) return;

    for (i = position; i < (_num_elements - 1); i++) {
	_contents[i] = _contents[i+1];
    }

    // decrement _num_elements and then reset the last element to NULL
    _num_elements--;
    _contents[_num_elements] = NULL;
}

void
MsgHndArray::grow()
{
    int orig_size = _size;

    // Note: If _size is less than 4 we are going to have
    // problems.  (_size >> 2) is 0 in this case.  That is
    // why we check for this case and increase the size by 1.
    _size += (_size > 4) ? (_size >> 2) : 1;

    _contents = (MsgStruct **)realloc(_contents, _size * sizeof(MsgStruct *));
    memset(_contents+orig_size, 0, (_size-orig_size) * sizeof(MsgStruct *));
}

int
MsgHndArray::insert(
    MsgStruct *a_msg_struct
)
{
    int i, j, return_val;
    int sess_num;
    SdmBoolean found;
    FORCE_SEGV_DECL(MsgStruct, tmpMS1);

    found = Sdm_False;
    sess_num = a_msg_struct->sessionNumber;

    // grow the array if array is filled.
    if (_num_elements >= _size) {
      grow();
    }

    for(i = 0; i < _num_elements; i++) {
	if (_contents[i]->sessionNumber > sess_num) {
            tmpMS1 = _contents[i];
	    _contents[i] = a_msg_struct;
	    return_val = i;
	    found = Sdm_True;
	    break;
	}
    }
    if (found == Sdm_True) {
	for (j = _num_elements; j > i+1; j--) {
	    _contents[j] = _contents[j-1];
	}
        _contents[i+1] = tmpMS1;
	_num_elements++;
    }
     else {
	 _contents[_num_elements] = a_msg_struct;
	 return_val = _num_elements;
	 _num_elements++;
     }
    return(return_val);
}

void
MsgHndArray::append(
    MsgStruct *a_msg_struct
)
{
    // grow the array if needed.
    if (_num_elements >= _size) 
      grow();

    _contents[_num_elements] = a_msg_struct;
    _num_elements++;
}

void
MsgHndArray::mark_for_delete(int indx)
{
    _contents[indx]->is_deleted = Sdm_True;
}

void
MsgHndArray::compact(
    int start_pos
)
{
    FORCE_SEGV_DECL(MsgStruct, tmpMS);
    int i;

    if ((_num_elements <= 0) || (start_pos < 0) || (start_pos >= _num_elements))
	return;

    for (i = _num_elements - 1; i >= start_pos; i--) {
	tmpMS = _contents[i];
	if (tmpMS->is_deleted) {
	    tmpMS->is_deleted = Sdm_False;
	    remove_entry(i);
// 	    compact(i);
	}
    }
}

//
// Replace the MsgStruct at 'position' with the passed MsgStruct.
// The MsgStruct previously located at 'position' is *not* destroyed.
//
void
MsgHndArray::replace(
	int position,
	MsgStruct *a_msg_struct
)
{
    if (position < 0 || position >= _num_elements) {
	return;
    }

    _contents[position] = a_msg_struct;

    return;
}
