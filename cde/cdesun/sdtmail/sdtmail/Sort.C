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
#pragma ident "@(#)Sort.C	3.31 01/29/97"
#endif

#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/MessageEnvelope.hh>
#include "Sort.hh"

#define isSet(v, m)     ((unsigned long)(v) & (unsigned long)(m))

// converDate is defined in MsgScrollingList.C
extern time_t convertDate(const char* buf);


//
// Sort the mailbox according to 'howToSort'.
//
// RETURNS:
//	The location of the displayed message after the sort.
//
int
Sort::sortMessages(MsgScrollingList	*displayList,
		   SdmMessageStore	*mbox,
		   sortBy		howToSort)
{
  SdmError		 error;
  char			*header_str;
  int			 header_int;
  SdmIntStrLL		intStrHeaderList;
  SdmIntStrL    *intStrList;
  int			numberMessages;
  MsgHndArray		*msgHandles;
  SdmMessageHeaderAbstractFlags headerFlags;
  SdmMessageFlagAbstractFlagsL msgFlags;
  int displayed = displayList->get_displayed_item();
  SdmMessageNumberL msgList;
  SdmString value;
  
  static const char* kEmptyString = "";

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
      for (int i=0; i<numberMessages; i++) {
        messages[i+1].msg_struct = msgHandles->at(i);
        msgList.AddElementToList(msgHandles->at(i)->message_handle);
      }
      
      //
      // First, get the data that will be used for the sort.
      // The header that we will sort on depends on how we were
      // told to sort.
      //
      switch (howToSort) {
        case SortSender:
          headerFlags = Sdm_MHA_P_SenderPersonalInfo | Sdm_MHA_From;
          mbox->GetHeaders(error, intStrHeaderList, headerFlags, msgList);
          break;

        case SortSubject:
          headerFlags = Sdm_MHA_Subject;
          mbox->GetHeaders(error, intStrHeaderList, headerFlags, msgList);
          break;        

        case SortSize:
          headerFlags = Sdm_MHA_P_MessageSize;
          mbox->GetHeaders(error, intStrHeaderList, headerFlags, msgList);
          break;

        case SortMsgNum:
          break;
 

        case SortStatus:
          mbox->GetFlags(error, msgFlags,  msgList);
          break;

        case SortTimeDate:
          default:
          headerFlags = Sdm_MHA_P_ReceivedDate;
          mbox->GetHeaders(error, intStrHeaderList, headerFlags, msgList);
          break;
      }
      
      // if there is an error, just return for now.
      if (error) {
        delete messages;
        return displayed;
      }

      const char* p = kEmptyString, *from = NULL, *sender = NULL;
            
      // set up message record for each message.
      for(i = 0 ; i < numberMessages; i++) {
        header_str = NULL;
        header_int = 0;
        p = kEmptyString;
        
        
        switch (howToSort) {
          case SortSender:
          {
            intStrList = intStrHeaderList[i];
            from = NULL;
            sender = NULL;
            for (int j = 0; j < intStrList->ElementCount(); j++) {
              value = (*intStrList)[j].GetString();
              if (value.Length() != 0) {
                if ((*intStrList)[j].GetNumber() == Sdm_MHA_P_MessageFrom) {
                  from = (const char*)value;
                } else {
                  sender = (const char*)value;
                }
              }
            }
            if (sender || from) {
              p = (sender ? sender : from);
            }
            header_str = strdup(p);
            break;
          }

          case SortSubject:
            intStrList = intStrHeaderList[i];
            if (intStrList->ElementCount() != 0) {
              value = (*intStrList)[0].GetString();

              // I18N Decode the subject header
              SdmStrStrL decoded_str;
              SdmString concat_subj;
              int j;
              SdmMessageEnvelope::DecodeHeader(error, decoded_str, 
                  value, SdmString(""));
              // Use the orginal string if error
              if (!error){	// If error, then use the original string
                // Concatentate all the decoded headers
                for (j=0; j<decoded_str.ElementCount(); j++)
                  concat_subj += decoded_str[j].GetSecondString();
                value = concat_subj;
              }

              if (value.Length() != 0) {
                // Skip over "Re:"
                p = (const char *)value;
                if (strncasecmp(p, "Re:", 3) == 0) {
                    p += 3;
                    while (isspace(*p)) {
                      p++;
                    }    
                }
              }
            }
            header_str = strdup(p);
            break;        

          case SortSize:
            intStrList = intStrHeaderList[i];
            if (intStrList->ElementCount() != 0) {
              value = (*intStrList)[0].GetString();
              if (value) {
                header_int = (long) strtol((const char*)value, NULL, 10);
              } 
            }
            break;

          case SortStatus:
            // Want sort order to be Read, Unread, New
            // note: if a message is not seen and is recent, that means it is new.
            if ( !isSet(msgFlags[i],Sdm_MFA_Seen) && isSet(msgFlags[i], Sdm_MFA_Recent)) {
              // No Status means New
              header_int = 2;
            } else if ( isSet(msgFlags[i], Sdm_MFA_Seen) ) { // Read
              header_int = 0;
            } else {
              header_int = 1;
            }
            break;

          case SortMsgNum:
            header_int = msgHandles->at(i)->message_handle;
            break;
 

          case SortTimeDate:
            default:
            intStrList = intStrHeaderList[i];
            if (intStrList->ElementCount() != 0) {
              value = (*intStrList)[0].GetString();
              if ( value.Length() == 0 ) {
                header_int = 0;
              } else {
                header_int = convertDate((const char*)value);
              }
            }
            break;
        }

        messages[i+1].header_str = header_str;
        messages[i+1].header_int = header_int;
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
      const MsgStruct	*current;

      // Get the current message
      current = displayList->get_message_struct(displayed);

      i = messages[0].link;
      for (int offset = 0; offset < numberMessages ; offset++) {
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
      intStrHeaderList.ClearAndDestroyAllElements();
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

  // Null header means it is a numeric compare
  if (first->header_str == NULL) {
    if (first->header_int < second->header_int) {
      return (-1);
    } else if (first->header_int > second->header_int) {
      return (2);
    } else {
      // use message number for secondary sorting when values are the same.
      return (first->msg_struct->message_handle < second->msg_struct->message_handle ? -1 :
        first->msg_struct->message_handle > second->msg_struct->message_handle ?  2 :
        0);
    }
  } else {
    // XXX should this be strcoll()??
    int rc;
    if ((rc = strcmp(first->header_str, second->header_str)) != 0) {
       return (rc);
    } else {
      // use message number for secondary sorting when values are the same.
      return (first->msg_struct->message_handle < second->msg_struct->message_handle ? -1 :
        first->msg_struct->message_handle > second->msg_struct->message_handle ?  2 :
        0);
    }
  }
}
