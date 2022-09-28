/*
 *+SNOTICE
 *
 *	$Revision:$
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

#ifndef SORT_HH
#define SORT_HH

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Sort.hh	1.6 08/09/96"
#endif

#include "FindDialog.h"
#include "RoamMenuWindow.h"

//
// What method to use to sort the message list.
//
  enum sortBy {
    SortTimeDate = 1,
    SortSender,
    SortSubject,	
    SortSize,
    SortStatus,
    SortMsgNum
};

//
// Used for sorting the mailbox.
//
class Sort {

public:


  //
  // Sorts the array of message handles.
  //
  int sortMessages(MsgScrollingList	*display_list,
  		    SdmMessageStore	*mbox,
		    sortBy		howToSort);

private:

  //
  // This is the message record that is sorted (not returned). It
  // is for internal use only and is never passed up to the calling
  // public member functions.
  //
  // The variable 'link' is used to sort the array in place. After
  // msort() (below) is called, the 'link' variable is set to the
  // link-order (1,2,...,N). Un-like qsort(), msort() sorts in place
  // and does not do memcpy(), msort() - updates the 'link' variable.
  // 
  
  struct messageRecord {
	int			link;		// Needed by msort(). (see source).
	char		*header_str;
	long			header_int;
	MsgStruct	*msg_struct;
  };

  //
  // Similar to qsort().
  //
  int	_msort(char		* base,		// Base of array.
	       int		  nel,		// Number of elements in array.
	       int		  width,	// Size of each element.
	       int		  offset,	// Offset to'link'(see source).

	       // Compare function.
	       int		(*compar)(char ** one, char ** two));

  //
  // This is the function that msort() calls to compare two records.
  //
  static int	_sortCmp(char **one, char **two);
};

#endif // SORT_HH
