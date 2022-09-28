/*
 *+SNOTICE
 *
 *	$Revision: 1.25 $
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

#ifndef	_MSGHNDARRAY_HH
#define	_MSGHNDARRAY_HH

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)MsgHndArray.hh	1.25 05/30/97"
#endif

#include <string.h>
#if defined(USL) && (OSMAJORVERSION == 2)
extern "C" {
#endif
#include <malloc.h>
#if defined(USL) && (OSMAJORVERSION == 2)
};
#endif

class MsgStruct {
  public:
    MsgStruct() {};
    ~MsgStruct() {};
    
    int sessionNumber;
    SdmMessageNumber message_handle;
    SdmBoolean is_deleted;
};

class MsgHndArray {
  protected:
    int _size;
    MsgStruct **_contents;
    int _num_elements;
    void grow();

  public:
    MsgHndArray(int sz = 1024, int zeroed = 1)
      {
	  _contents = new MsgStruct*[sz];
	  _size = sz;
	  if (zeroed) memset(_contents, 0, sizeof(MsgStruct *)*sz);

	  _num_elements = 0;

      }

    ~MsgHndArray()
      {
	  for (int ent = 0; ent < _num_elements; ent++) {
	      delete _contents[ent];
	  }

	  delete [] _contents;
      }

    int		length();
    MsgStruct*	at(int a_number);
    int		insert(MsgStruct* a_msg_struct);
    void	clear();
    int		index(MsgStruct* a_msg_struct);
    int		index(SdmMessageNumber a_msg_handle);
    void	remove_and_free_all_elements();
    void	remove_all_elements();
    void	remove_entry(int position);
    void	append(MsgStruct *a_msg_struct);
    void	mark_for_delete(int position);
    void	compact(int start_pos);
    void	replace(int postition, MsgStruct *a_msg_struct);
};

#endif
    
    
