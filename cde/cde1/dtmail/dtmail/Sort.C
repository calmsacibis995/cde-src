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

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Sort.C	3.9 08/09/95"
#endif

#include <stdlib.h>
#include <ctype.h>
#include <EUSCompat.h>
#include "Sort.hh"

//
// Sort the mailbox according to 'howToSort'.
//
// RETURNS:
//	The location of the displayed message after the sort.
//
int
Sort::sortMessages(MsgScrollingList	*displayList,
		   DtMail::MailBox	*mbox,
		   sortBy		howToSort)
{
  // TODO - CHECK ERROR !!!
  DtMailEnv		 error;
  char			*header_str;
  int			 header_int;
  DtMailValueSeq	value;
  int			numberMessages;
  MsgHndArray		*msgHandles;
  int displayed = displayList->get_displayed_item();

  msgHandles = displayList->get_messages();

  if (msgHandles != NULL && mbox != NULL) {

    numberMessages = msgHandles->length();
  
    if (numberMessages > 0) {

      //
      // Need list for all of the messages.
      // +2 for 2 artificial records R(0) and R(n+1).  See Knuth (5.2.4)
      // Algorithm L
      // In addition, the data must be placed in the array with the
      // dummy records at the beginning and end of the array.

      messageRecord	* messages = new messageRecord[numberMessages +2];

      register unsigned int	offset;
      register unsigned int	msgno;
      DtMail::Message		* msg = NULL;
      DtMail::Envelope	* envelope = NULL;

      //
      // Get the messages from the list.
      //
      for(msgno = 0 ; msgno < numberMessages; msgno++) {

	offset = msgno + 1;

	//
	// Get the handle and envelope and header.
	//
	messages[offset].msg_struct = msgHandles->at(msgno);

	if (howToSort != SortMsgNum) {
		// Don't need envelope to sort by MsgNum since that is
		// a front end concept
		msg = mbox->getMessage(error,
			messages[offset].msg_struct->message_handle);

		if (error.isSet()) {
			fprintf(stderr,
		"dtmail: getMessage: Could not get message # %d: %s\n",
			msgno, (const char *)error);
		}

		if (msg != NULL) {
			envelope = msg->getEnvelope(error);

			if (error.isSet()) {
			fprintf(stderr,
		"dtmail: getEnvelope: Could not get envelope for # %d: %s\n",
			msgno, (const char *)error);
			}
		}

		if (msg == NULL || envelope == NULL) {
			continue;
		}
	}

	header_str = NULL;
	header_int = 0;

	//
	// The header that we will sort on depends on how we were
	// told to sort.
	//
	switch (howToSort) {

	case SortSender:
	  envelope->getHeader(error, DtMailMessageSender, DTM_TRUE, value);
	  if (error.isSet()) {
		header_str = strdup("");
	  } else {
		// Stole from MsgScrollingList
		DtMailAddressSeq * addr_seq = NULL;
		addr_seq = (value[0])->toAddress();
		DtMailValueAddress * addr = (*addr_seq)[0];
		if (!addr) {
		  header_str = strdup("");
		}
		else if (addr->dtm_person) {
			header_str = strdup(addr->dtm_person);
		} else {
			header_str = strdup(addr->dtm_address ? addr->dtm_address : "");
		}
	  }
	  break;

	case SortSubject:
	  envelope->getHeader(error, DtMailMessageSubject, DTM_TRUE, value);
	  if (error.isSet()) {
		header_str = strdup("");
	  } else {
		// Skip over "Re:"
		const char *p;
		 p = *(value[0]);
		if (strncasecmp(p, "Re:", 3) == 0) {
			p += 3;
			while (isspace(*p)) {
				p++;
			}
		}
		header_str = strdup(p);
	  }
	  break;

	case SortSize:
	  envelope->getHeader(error, DtMailMessageContentLength, DTM_TRUE,
				value);
	  if (error.isNotSet()) {
		header_int = (int) strtol(*(value[0]), NULL, 10);
	  }
	  break;

	case SortStatus:
	  envelope->getHeader(error, DtMailMessageStatus, DTM_TRUE, value);
	  // Want sort order to be Read, Unread, New
	  if (error.isSet()) {
		// No Status means New
		header_int = 2;
	  } else {
		const char *s;
		s = *(value[0]);

		if (s == NULL) {
			// New
			header_int = 2;
		} else if (strcmp(s, "RO") == 0) {
			// Read
			header_int = 0;
		} else {
			// Unread
			header_int = 1;
		}
	  }
	  break;

	case SortMsgNum:
	  header_int = messages[offset].msg_struct->sessionNumber;
	  break;

	case SortTimeDate:
	  /*FALLTHRU*/
	default:		// Default to Time/Date.
	  // Need to convert to numeric;
	  envelope->getHeader(error, DtMailMessageReceivedTime, DTM_TRUE,
			value);
	  if (error.isSet()) {
		header_int = 0;
	  } else {
		DtMailValueDate ds;
		ds = (*(value[0])).toDate();
		header_int = (int)ds.dtm_date;
	  }
	}

	messages[offset].header_str = header_str;
	messages[offset].header_int = header_int;

	value.clear();

      }

      //
      // Sort them.
      //
      _msort((char *)messages,
	     numberMessages,
	     sizeof(messageRecord),
	     0,				// Link (offset) is at ZERO.
	     _sortCmp);

      //
      // Rearrange the pointers to msg_structs in the original
      // MsgHndArray.
      //
      int i;
      MsgStruct	*current;

      // Get the current message
      current = displayList->get_message_struct(displayed);

      i = messages[0].link;
      for (offset = 0; offset < numberMessages ; offset++) {
	msgHandles->replace(offset, messages[i].msg_struct);
	if (messages[i].msg_struct == current) {
		// Remember new offset for current message
		displayed = offset + 1;
	}
	if (messages[i].header_str != NULL) {
		free(messages[i].header_str);
	}
	i = messages[i].link;
      }

      // Now cleanup.
      delete messages;
    }
  }

  return displayed;
}

//
// msort() is a list-merge sort routine generalized from Knuth (5.2.4)
// Algorithm L.  This routine requires 2 artificial records: R0 and
// Rn+1 where n = number of elements "nel".  "offset" is the byte-offset
// of the "link" field.  "width" is the size of each record.  "base" is
// the base address of the starting record (i.e. R0.)
//
// (Code lifted from msort.c in the original mailtool).
//

#define	Record(i)	(base + (width * (i)))
#define	Link(i)		(*((int *) (Record(i) + offset)))

Sort::_msort (char	* base,
	      int	  nel,
	      int	  width,
	      int	  offset,
	      int	(*compar)(char	**one,
				  char	**two))
{
  register int	i;
  register int	t;
  register int	s;
  register int	p;
  register int	q;
  char	*k1;
  char 	*k2;

  if (nel < 2) {
    Link(0) = 1;
    return (0);
  }

  /* Prepare two lists. */
  Link(0) = 1;
  Link(nel+1) = 2;
  for (i = nel - 2; i >= 1; i--) {
    Link(i) = -(i+2);
  }
  Link(nel-1) = 0;
  Link(nel) = 0;

  while (1) {
    /* Begin new pass */
    s = 0;
    t = nel + 1;
    p = Link(s);
    q = Link(t);
    if (q == 0) {
      return (0);
    }

	int	loopCount = 0;
    while (1) {
	loopCount++;	
      /* Compare Kp: Kq */
      k1 = Record(p);
      k2 = Record(q);
      if ((*compar)(&k1, &k2) <= 0) {
	/* Advance p */
	i = abs(p);
	Link(s) = (Link(s) < 0) ? -i : i;
	s = p;
	p = Link(p);
	if (p > 0) {
	  continue;
	}

	/* Complete the sublist */
	Link(s) = q;
	s = t;
	do {
	  t = q;
	  q = Link(q);
	} while (q > 0);
      } else {
	/* Advance q */
	i = abs(q);
	Link(s) = (Link(s) < 0) ? -i : i;
	s = q;
	q = Link(q);
	if (q > 0) {
	  continue;
	}

	/* Complete the sublist */
	Link(s) = p;
	s = t;
	do {
	  t = p;
	  p = Link(p);
	} while (p > 0);
      }

      /* End of pass? */
      p = -p;
      q = -q;
      if (q == 0) {
	i = abs(p);
	Link(s) = (Link(s) < 0) ? -i : i;
	Link(t) = 0;
	break;
      }
    }
  }
}

//
// These were used in msort() only. They are #undef'ed as a precaution only.
//
#undef Link
#undef Record

//
// Sort the two records.
//
int
Sort::_sortCmp(char ** one, char ** two)
{
  //
  // Cast the pointers to the known type.
  //
  register messageRecord	* first = (messageRecord *) *one;
  register messageRecord	* second = (messageRecord *) *two;

  if (first->header_str == NULL) {
	// Null header means it is a numeric compare
	return (first->header_int < second->header_int ? -1 :
		first->header_int > second->header_int ?  2 :
		0);
  } else {
	// XXX should this be strcoll()??
	return strcmp(first->header_str, second->header_str);
  }
}
