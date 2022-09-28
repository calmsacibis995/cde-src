/*
 *+SNOTICE
 *
 *	$Revision: 1.6 $
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
#pragma ident "@(#)MsgScrollingList.C	1.337 06/18/97"
#endif

#include <ctype.h>
#include <locale.h>
#include <assert.h>
#include <Xm/Text.h>
#include <Dt/Dts.h>
#include "RoamApp.h"
#include "RoamMenuWindow.h"
#include "Views.h"
#include "MsgScrollingList.hh"
#include "MemUtils.hh"
#include "PasswordDialogManager.h"
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/Search.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/MessageBody.hh>
#include <SDtMail/MessageEnvelope.hh>
#include <SDtMail/ServiceFunction.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/String.hh>
#include <SDtMail/SimpleTuples.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/DataTypeUtility.hh>
#include <SDtMail/MessageUtility.hh>
#include "MsgHndArray.hh"
#include "MailMsg.h"
#include "DtMailGenDialog.hh"
#include "SortCmd.hh"

#include "Help.hh"	// Remove after fixing problem with empty time headers
#include "DtMailHelp.hh"

extern force( Widget );

MsgScrollingList::MsgScrollingList(
				   RoamMenuWindow *menuwindow,
				   Widget parent,
				   char *name
				   )
  : ScrollingList(
		  parent,
		  name
		  )
{
  _current_ss = NULL;
//   _current_view_name = NULL;
  _parent=menuwindow;
  _selected_item_position=-1;
  _displayed_item_position=-1;
  _selection_on = Sdm_False;
  _xmstr_collector = NULL;
  _xtarg_collector = NULL;
  _selected_items = NULL;
  _last_msg_num = 0;

  const char* locale = setlocale(LC_TIME, NULL);
  _in_C_locale = (!locale || (strcasecmp(locale, "c")==0)) ? Sdm_True : Sdm_False;

  XtAddCallback( _w,
                 XmNextendedSelectionCallback,
                 (XtCallbackProc)
                 &MsgScrollingList::extendedSelectionCallback,
                 this );

  _msgNumL = NULL;
  _msgs = new MsgHndArray(1024);
  _deleted_messages = new MsgHndArray(1024);
  _num_new_messages = 0;
  _num_deleted_messages = 0;
  _total_deleted = 0;
  _date_width = 0;

  _session_message_number = 0;

  // Can later initialize these from the last use of the session.
  // Each folder will have some idea of which message was last
  // read.  We should select and display it at next load, no?

  _selected_item_position = 0;
  _displayed_item_position = 0;
  _displayed_msgno = 0;

  SdmMailRc *mailrc;
  SdmError mail_error;
  SdmConnection *con = theRoamApp.connection()->connection();
  assert (con != NULL);
  con->SdmMailRcFactory(mail_error, mailrc);
  assert (!mail_error);

  if (!mail_error) {
    _showTo = mailrc->IsValueDefined("showto");
    _numbered = mailrc->IsValueDefined("showmsgnum");
    _useAlternates = mailrc->IsValueDefined("usealternates");
    if (_showTo) {
      _alternates = mailrc->GetAlternates(mail_error);
      GetUsernames();
    } else {
      _alternates = NULL;
    }
  }

  _header_info = Sdm_MHA_P_MessageFrom |
	Sdm_MHA_P_ReceivedDate |
	Sdm_MHA_P_Subject |
	Sdm_MHA_P_MessageSize |
	Sdm_MHA_P_MessageType |
	Sdm_MHA_P_SenderPersonalInfo;

  if (_showTo)
  	_header_info |= Sdm_MHA_P_SenderUsername;

}

MsgScrollingList::~MsgScrollingList()
{
  if (_selected_items)
    {
      XmStringFree (_selected_items);
      _selected_items = NULL;
    }

  delete _deleted_messages;
  delete _msgs;

  if (_msgNumL != NULL) {
      delete _msgNumL;
      _msgNumL = NULL;
  }
  
  if (_alternates)
     free (_alternates);
  
//   if (_current_view_name) free(_current_view_name);
}

Widget
MsgScrollingList::get_scrolling_list()
{
  return(_w);
}

void
MsgScrollingList::select_next_item()
{
  INSERT_STACK_PROBE
  int num_msgs = 0;
  DtMailGenDialog *genDialog = theRoamApp.genDialog();
  const MsgStruct *tmpMS;
  SdmError mail_error;

  // Initialize the mail_error.
  mail_error.Reset();

  XtVaGetValues( _w,
		 XmNitemCount, &num_msgs,
		 NULL );
    
  _selected_item_position = _displayed_item_position + 1;

  if (_selected_item_position <= num_msgs && 
      _selected_item_position > 0 ) {

    tmpMS = get_message_struct(_selected_item_position);
    if (tmpMS == NULL) {
      return;
    }
    else {
      // Deselect all items currently selected.
      // display_and_select_message() will select, highlight
      // and display the "next" message.

      XmListDeselectAllItems(_w);

      this->display_and_select_message(mail_error,
				       tmpMS->message_handle);

      if (mail_error) { 
	genDialog->post_error(mail_error, DTMAILHELPCANNOTDISPLAYMSG, 
			      catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
      }
    }
  }
  else {
    return;
  }
}

void
MsgScrollingList::select_prev_item()
{
  INSERT_STACK_PROBE
  int num_msgs;
  DtMailGenDialog *genDialog = theRoamApp.genDialog();
  const MsgStruct *tmpMS;
  SdmError mail_error;

  // Initialize the mail_error.
  mail_error.Reset();

  XtVaGetValues( _w,
		 XmNitemCount, &num_msgs,
		 NULL );
    
  _selected_item_position = _displayed_item_position - 1;
    
  if (_selected_item_position >= 1) {
    tmpMS = get_message_struct(_selected_item_position);
    if (tmpMS == NULL) {
      return;
    }
    else {
      // Deselect all items currently selected.
      // display_and_select_message() will select, highlight
      // and display the "previous" message.

      XmListDeselectAllItems(_w);

      this->display_and_select_message(mail_error,
				       tmpMS->message_handle);
				       
      if (mail_error) { 
	genDialog->post_error(mail_error, DTMAILHELPCANNOTDISPLAYMSG, 
			      catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
      }
    }
  }
  else {
    return;
  }
}

SdmMessageNumber 
MsgScrollingList::msgno( int index )
{ 
  if (index <= 0) {
    return(NULL);
  }
  else {
    return _msgs->at(index-1)->message_handle; 
  }
}

const MsgStruct *
MsgScrollingList::get_message_struct( int index ) 
{ 
  if (index <= 0) {
    return(NULL);
  }
  else {
    return(_msgs->at(index-1));
  }
}

int 
MsgScrollingList::position( SdmMessageNumber msgno ) 
{ 
  return (_msgs->index(msgno))+1; 
}

void 
MsgScrollingList::insertMsg( SdmMessageNumber msg_hndl )
{
  MsgStruct *newMS;

  // A new message has come in.
  // Increase the session_message_number which keeps track of the
  // number of messages in this session for this folder (scrolling list).
  // 

  newMS = new MsgStruct();
  newMS->message_handle = msg_hndl;
  newMS->sessionNumber = _session_message_number;
  newMS->is_deleted = Sdm_False;
  _msgs->append(newMS);

  _session_message_number++;
}

// gl - It is crazy but true, setting the is_deleted flag here to
// true causes things to break when you undelete a deleted message.  We hand
// the _deleted_messages list to the UndelFromListDialog so it can load the
// appropriate messages into its own list of message numbers.  We do this from
// the load_view method.  So anyway, when the user attempts to undelete a message
// the UndelMsgScrollingList code first marks the selected message(s) as deleted
// and then calls MsgHndArray::compact which removes all messages marked as
// deleted.  If we set the is_deleted flag to true here, UndelMsgScrollingList's 
// whole message list gets blown away without its knowledge! 
void 
MsgScrollingList::insertDeletedMsg(SdmMessageNumber msg_hndl)
{
  MsgStruct *newMS;

  newMS = new MsgStruct();
  newMS->message_handle = msg_hndl;
  newMS->sessionNumber = _session_message_number;
  newMS->is_deleted = Sdm_False;
  _deleted_messages->append(newMS);

  _session_message_number++;
  _num_deleted_messages++;
}

// Check if the message is a multipart one
SdmBoolean
MsgScrollingList::IsMultipartMessage(SdmIntStrL *intStrListPtr)
{
  assert (intStrListPtr != NULL);
  for (int i = 0; i < intStrListPtr->ElementCount(); i++) {

    // multipart message is one whose message type is not text.
    if ( (*intStrListPtr)[i].GetNumber() == Sdm_MHA_P_MessageType &&
          strcasecmp((*intStrListPtr)[i].GetString(), "TEXT") == 0 ) {
          return(Sdm_False);
    }
  }

  return(Sdm_True);
}

// Checks the value for bits set in mask
#define isSet(v, m)	((unsigned long)(v) & (unsigned long)(m))

int MsgScrollingList::load_all_headers(SdmError &error, SdmMessageNumber nmsgs)
{
  XmString complete_header; // text of header w/ glyphs
  int num_items = 0;
  SdmMessageStore * mbox = this->parent()->mailbox();
  SdmBoolean first_new = Sdm_True;
  SdmMessageNumber select_msg_hndl=0;
  DtMailGenDialog *genDialog = theRoamApp.genDialog();

  // Create a class to collect the mail header XmStrings
  // then get all the items from the current list.
  _xmstr_collector = new XmStrCollector();

  // reset the sorting for this window.
  _parent->setLastSortCmd(NULL);

  // Retrieve the message_handles, and from them their headers.
  // Create an XmString and toss it into the XmStrCollector. 
  long	numOfMessages = 0;

#ifdef USE_STATUS
  SdmMessageStoreStatus mboxStatus;
  
  // Get only the number of messages in the store
  SdmMessageStoreStatusFlags mboxFlags = Sdm_MSS_Messages; 
  mbox->GetStatus(error, mboxStatus, mboxFlags);
  if (!error) {
    numOfMessages= mboxStatus.messages;
    _last_msg_num = numOfMessages;
  } else {
    _last_msg_num = 0;
    delete _xmstr_collector;
    _xmstr_collector = NULL;
    return (-1);
  }
#else
  _last_msg_num = numOfMessages = nmsgs;
#endif

  if (numOfMessages > 0) {
      SdmIntStrLL	intStrHeaderList;
      SdmIntStrL	*intStrListPtr;
      SdmMessageFlagAbstractFlagsL flagsList = 0;

      // Get the designated headers (defined through _header_info)
      // for all the messages into intStrHeaderList
      mbox->GetHeaders(error, intStrHeaderList, _header_info,
		   (SdmMessageNumber)1, (SdmMessageNumber)numOfMessages);
		   
      // if we didn't get any headers for any of the messages (this should be 
      // pretty rare but we need to handle it anyway), create empty lists for each
      // message and continue processing below.
      //
      if (error == Sdm_EC_RequestedDataNotFound) {
        for (int i = 0; i < numOfMessages; i++) {
          intStrHeaderList.AddElementToList(new SdmIntStrL);
        }
        error = Sdm_EC_Success;  // reset error.
      } 

      if (error) {
        intStrHeaderList.ClearAndDestroyAllElements();
        delete _xmstr_collector;
        _xmstr_collector = NULL;
        return (-1);
      }

      // Get the flags of all the messages in the mailbox (message store)
      // in the vector (array) of SdmMessageFlagAbstractFlagsL's
      mbox->GetFlags(error, flagsList, (SdmMessageNumber)1,
		     (SdmMessageNumber)numOfMessages);
      if (error) {
        intStrHeaderList.ClearAndDestroyAllElements();
        delete _xmstr_collector;
        _xmstr_collector = NULL;
        return (-1);
      }
      
      for (int i = 1; i <= numOfMessages; i++, num_items++) {

        // Check the pending delete flag
        if ( isSet(flagsList[i-1], Sdm_MFA_Deleted) ) {
          insertDeletedMsg(i);
          continue;
        }

        insertMsg(i);

        // Get the pointer to the vector of Int/Str pairs
        // containing header-flags/values
        intStrListPtr = intStrHeaderList[i-1];

        // formatHeader now has two message number arguments passed to it.  The
        // first is the message number wrt the messagestore, the second is the
        // message number wrt to the current view.
        complete_header = formatHeader(intStrListPtr, i, i,
                   IsMultipartMessage(intStrListPtr),
                   isSet(flagsList[i-1], Sdm_MFA_Seen) ? Sdm_False : Sdm_True);

        if ( !isSet(flagsList[i-1], Sdm_MFA_Seen) ) {

          _num_new_messages++;

	  // We want to select the last read message before the
	  // first new message.  We will select the first new
	  // message if it is the first message.  We need to be
	  // careful that we dont select a message that has been
	  // deleted.  If we just subtract 1 from i, we might be
	  // refering to a deleted message.  So, we get the previous
	  // message added to the _msgs list.
	  if (first_new) {
	      first_new = Sdm_False;
	      // Some explanation for the following code...
	      // Need to be careful when the first non-deleted message
	      // is also the first new message.  This means that there
	      // are no earlier messages that we can display, so display
	      // the first message in the _msgs list.
	      if ((i > 1) && (i-_num_deleted_messages)>1)
		  select_msg_hndl = _msgs->at(i-_num_deleted_messages-2)->message_handle;
	      else
		  select_msg_hndl = _msgs->at(0)->message_handle;
	  }
	}

        // Insert the XmString into the array.
        _xmstr_collector->AddItemToList (complete_header);
      }

      // _total_deleted represents the total number of messages
      // marked for deletion in the entire mailbox, not just the
      // current view.
      _total_deleted = _num_deleted_messages;

      // If there were no new messages, select and display the last message.
      if (first_new) {
	// Be careful here, our _msgs array might be empty!  In other words
	// all of the messages in this mailbox might be marked as deleted.
	if (_msgs->length() > 0) {
	  select_msg_hndl = _msgs->at(_msgs->length() - 1)->message_handle;
	}
      }

      // Add the items to the XmList.
      // All XmStrings are freed in the XmStrCollector destructor.
      _xtarg_collector = new XtArgCollector;

      // The first time the headers are loaded, they should all be loaded
      // at the same time.  XtVaSetValues is used for this rather than
      // XmListAddItems so that all the other resource will be set at
      // the same time.  This prevents multiple repaints.
      //
      // However, in the case where only an item or two are being added,
      // XmListAddItems should be used.  This prevents an unnecessary
      // repaint in this case.
      XmListAddItemsUnselected (_w, _xmstr_collector->GetItems(),
		      _xmstr_collector->GetNumItems(), 0);

      intStrHeaderList.ClearAndDestroyAllElements();

      display_and_select_message(error, select_msg_hndl);
      if (error) {
	genDialog->post_error(error, DTMAILHELPCANNOTDISPLAYMSG, 
			      catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
        // don't return.  just continue. 
      }

      _xtarg_collector->SetValues(_w);

      // we can free up the memory for _selected_items now
      // that the selected item has been set up for the
      // widget and it is no longer needed in _xtarg_collector.
      if (_selected_items)
      {
        XmStringFree (_selected_items);
        _selected_items = NULL;
      }

      delete _xtarg_collector;
      delete _xmstr_collector;
      _xtarg_collector = NULL;
      _xmstr_collector = NULL;
  } else {
      delete _xmstr_collector;
      _xmstr_collector = NULL;
  }
  return(num_items);
}

#define BATCH_COUNT 20

// This routine loads only a subset of the headers into the msg scrolling
// list and then sets up a workproc to handle loading the rest of them.
int
MsgScrollingList::load_headers(SdmError &error, SdmMessageNumber nmsgs)
{
    XmString complete_header;
    SdmMessageStore * mbox = this->parent()->mailbox();
    SdmMessageNumber select_msg_hndl=0;
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    SdmIntStrLL	intStrHeaderList;
    SdmIntStrL	*intStrListPtr;
    SdmMessageFlagAbstractFlagsL flagsList = 0;
    SdmMessageFlagAbstractFlagsL shortFlagsList = 0;
    SdmMessageNumber first_new_msg_index = -1;
    int msg_count=0, i, start, num_to_display;
    SdmBoolean done = Sdm_False;


    if (nmsgs <= 0)
	return 0;

    _parent->setLastSortCmd(NULL);
    _last_msg_num = nmsgs;

    _msgNumL = new SdmMessageNumberL(nmsgs);

    if (nmsgs > 0) {
	// Get the flags for all the msgs in the mailbox
	mbox->GetFlags(error, flagsList, 1, nmsgs);
	if (error)
	    return (-1);

	// Build the deleted msg list and the regular msg list.  We will
	// keep track of all the non-deleted msg numbers using the _msgNumL
	// object.  We also need to keep track of the new msg count as well
	// as where the first new msg was found.  We need this to decide the
	// initial msg to display.
	for (i=1; i<=nmsgs; i++) {
	    if ( isSet(flagsList[i-1], Sdm_MFA_Deleted) ) {
		insertDeletedMsg(i);
		continue;
	    }

	    msg_count++;
	    _msgNumL->AddElementToList((SdmMessageNumber)i);

	    insertMsg(i);
	    if ( !isSet(flagsList[i-1], Sdm_MFA_Seen) ) {
		if (first_new_msg_index == -1) {
		    // we gotta subtract _num_deleted_messages from i to get the index of
		    // the first new message in our _msgNumL array (because it doesn't have
		    // any deleted msgs in it.
		    first_new_msg_index = i - 1 - _num_deleted_messages;
		}
		_num_new_messages++;
	    }
	}
    } else {
	return 0;
    }

    _total_deleted = nmsgs - msg_count;

    // Figure out how many msgs to load.  I think a good start would
    // be double the number of visible items.
    int vis, cnt;
    XtVaGetValues(_w, XmNvisibleItemCount, &vis, NULL);
    cnt = 2 * vis;

    // figure out the range of msgs to display
    if (msg_count < cnt) {
	start = 0;
	num_to_display = msg_count;
	done = Sdm_True;
    } else if (first_new_msg_index == -1) {
	// no new msgs in the mbox, display the last n msgs
	start = msg_count - cnt;
	num_to_display = cnt;
    } else if (first_new_msg_index > (msg_count - cnt / 2)) {
	// first new msg is close to the end of the list of msgs
	start = msg_count - cnt;
	num_to_display = cnt;
    } else if (first_new_msg_index < (cnt / 2)) {
	// first new msg is close to the beginning of the list of msgs
	start = 0;
	num_to_display = cnt;
    } else {
	// somewhere in the middle
	start = first_new_msg_index - (cnt / 2);
	num_to_display = cnt;
    }

    // save the index of the first and last msg loaded into the msl
    _last_msg_index_loaded = start + num_to_display - 1;
    _first_msg_index_loaded = start;

    // copy num_to_display msgs from the big msg list (msgs) to to our small msg list
    // (msgs_to_display).  We are creating this small list of msgs so we don't have to get
    // headers for the entire mailbox.
    SdmMessageNumberL msgs_to_display(num_to_display);
    for (i=start; i < start + num_to_display; i++) {
	msgs_to_display.AddElementToList((*_msgNumL)[i]);
    }

    // This is the index of the first new msg in the msgs_to_display list
    int adjusted_new_index = first_new_msg_index - start;

    // Get the headers for our small list of msgs.
    mbox->GetHeaders(error, intStrHeaderList, _header_info, msgs_to_display);

    // if we didn't get any headers for any of the messages (this should be 
    // pretty rare but we need to handle it anyway), create empty lists for each
    // message and continue processing below.
    if (error == Sdm_EC_RequestedDataNotFound) {
        for (i = 0; i < num_to_display; i++) {
	    intStrHeaderList.AddElementToList(new SdmIntStrL);
        }
        error = Sdm_EC_Success;
    } 
    if (error) {
        intStrHeaderList.ClearAndDestroyAllElements();
        return (-1);
    }

    // Get the flags for the msgs in our small list (msgs_to_dislpay).
    flagsList.ClearAllElements();
    mbox->GetFlags(error, flagsList, msgs_to_display);
    if (error)
	return (-1);

    // Create a list of dummy XmStrings to load into the XmList widget.
    // We will replace a small subset of the dummy entries with actual
    // msg header info.  We will then setup an XtWorkProc to handle
    // loading/replacing the rest of the headers.
    XmString dummy_str = XmStringCreateLocalized("");
    XmString *item_list = (XmString *)malloc(msg_count * sizeof(XmString));
    for (i = 0; i < msg_count; i++) {
	item_list[i] = dummy_str;
    }

    // Create the XmStrings for the initial subset of headers that we want to display
    for (i = 0; i < num_to_display; i++) {
        intStrListPtr = intStrHeaderList[i];

        complete_header = formatHeader(intStrListPtr, msgs_to_display[i], msgs_to_display[i],
			       IsMultipartMessage(intStrListPtr),
			       isSet(flagsList[i], Sdm_MFA_Seen) ? Sdm_False : Sdm_True);

	// We need to remember to free these XmStrings that we are adding to our item_list.
	item_list[start + i] = complete_header;
    }

    intStrHeaderList.ClearAndDestroyAllElements();

    XmListAddItems(_w, item_list, msg_count, 1);

    // Figure out which msg to select.  If there are no new msgs, select the last
    // msg in the mailbox.
    if (adjusted_new_index < 0) {
	select_msg_hndl = msgs_to_display[num_to_display - 1];
    } else if (adjusted_new_index == 0) {
	// the first msg is new, select it
	select_msg_hndl = msgs_to_display[0];
    } else {
	// select the msg just before the first new msg
	select_msg_hndl = msgs_to_display[adjusted_new_index - 1];
    }

    display_and_select_message(error, select_msg_hndl);
    if (error) {
	genDialog->post_error(error, DTMAILHELPCANNOTDISPLAYMSG, 
	      catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
    }

    free(item_list);
    XmStringFree(dummy_str);

    // Need to set the bottom position to work around a bug in Motif that causes
    // the list to be repainted when items in the front of the list are being replaced.
    int top;
    XtVaGetValues(_w, XmNtopItemPosition, &top, NULL);
    XmListSetBottomPos(_w, top + vis - 1);

    // install a work proc to handle loading the rest of the msg headers
    if (!done) {
	XtWorkProcId id;

	// set the _state variable to Pending to indicate that all of the headers
	// are not loaded yet.  More importantly, to indicate that load_headersWP
	// has a pointer to this object and we should not actually destroy it yet.
	_state = Pending;
	id = XtAppAddWorkProc(XtWidgetToApplicationContext(_w), load_headersWP, this);
    } else {
	delete _msgNumL;
	_msgNumL = NULL;
	_state = Complete;
    }

    return((int)nmsgs);
}

//
// load_headersWP is an XtWorkProc used to finish loading the msg headers into the
// msg scrolling list.  It adds them in increments of size BATCH_COUNT, which is
// a #define.  It returns FALSE until it has completed loading all of the msg headers.
// This procedure is only run when the X event loop is idle.
Boolean
MsgScrollingList::load_headersWP(XtPointer client_data)
{
    MsgScrollingList *msl = (MsgScrollingList *)client_data;
    int i, num_to_add, msg_count, start;
    SdmError error;
    SdmIntStrLL	intStrHeaderList;
    SdmIntStrL	*intStrListPtr;
    SdmMessageFlagAbstractFlagsL flagsList = 0;
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    XmString complete_header;
    Boolean done = False;

    // Check to see if the user has quit the mailbox before we have had a
    // chance to finish loading the headers.  If so, destroy the msl and return
    // True so the workproc won't be called again.
    if (msl->_state == Destroyed) {
	msl->deleteMe();
	return True;
    }

    SdmMessageStore *mbox = msl->parent()->mailbox();

    // The MsgScrollingList::expunge method deletes the _msgNumL and sets
    // it to NULL.  The expunge routine will take care of reloading the headers.
    // Nothing left to do but exit.
    if (msl->_msgNumL == NULL)
	return True;

    // Need to figure out which msgs to add to the msg header list.  There are
    // 3 possible cases here to deal with:
    //		- there are more msgs after the ones that are already being
    //		  displayed, these are our highest priority to add.
    //		- We have added the headers for all of the msgs after the initial
    //		  subset.  Now lets start adding the ones before the initial subset.
    //		- There are fewer than BATCH number of msg headers to add from the
    //		  very beginning of the mailbox, add those remaining headers and
    //		  set done to TRUE.
    msg_count = msl->_msgNumL->ElementCount();
    if (msl->_last_msg_index_loaded < msg_count - 1) {
	start = msl->_last_msg_index_loaded + 1;
	num_to_add = ((msg_count - (msl->_last_msg_index_loaded + 1)) > BATCH_COUNT) ? BATCH_COUNT : msg_count - (msl->_last_msg_index_loaded + 1);
	msl->_last_msg_index_loaded = start + num_to_add - 1;
    } else if ((msl->_first_msg_index_loaded - BATCH_COUNT) > 0) {
	msl->_first_msg_index_loaded -= BATCH_COUNT;
	start = msl->_first_msg_index_loaded;
	num_to_add = BATCH_COUNT;
    } else {
	num_to_add = msl->_first_msg_index_loaded;
	msl->_first_msg_index_loaded = 0;
	start = 0;
	done = True;
    }

    // Create a small list of msg numbers that we will get the headers for.  This
    // should be faster than getting all headers for the mailbox.
    SdmMessageNumberL msgs_to_add(num_to_add);
    for (i=start; i < start + num_to_add; i++) {
	msgs_to_add.AddElementToList((*msl->_msgNumL)[i]);
    }

    // Get the headers (defined through _header_info) for only a chunk of the msgs
    mbox->GetHeaders(error, intStrHeaderList, msl->_header_info, msgs_to_add);
		   
    // if we didn't get any headers for any of the messages (this should be 
    // pretty rare but we need to handle it anyway), create empty lists for each
    // message and continue processing below.
    if (error == Sdm_EC_RequestedDataNotFound) {
        for (i = 0; i < num_to_add; i++) {
	    intStrHeaderList.AddElementToList(new SdmIntStrL);
        }
        error = Sdm_EC_Success;  // reset error.
    } 
    if (error) {
        intStrHeaderList.ClearAndDestroyAllElements();
	genDialog->post_error(error);
	// Should we return false here so that this workproc continues to be called?
	// If the reason this error occured is not going to fix itself, then returning
	// false is a bad idea (because the user will get popup after popup).
        return (True);
    }

    // Get the flags for the msgs we are adding the header list
    mbox->GetFlags(error, flagsList, msgs_to_add);
    if (error) {
	genDialog->post_error(error);
	return False;
    }

    msl->_xmstr_collector = new XmStrCollector();

    // loop through our small list of msgs and create an XmString for each header and
    // add them to the _xmstr_collector object.
    for (i = 0; i < num_to_add; i++) {
        intStrListPtr = intStrHeaderList[i];

        complete_header = msl->formatHeader(intStrListPtr, msgs_to_add[i], msgs_to_add[i],
			       msl->IsMultipartMessage(intStrListPtr),
			       isSet(flagsList[i], Sdm_MFA_Seen) ? Sdm_False : Sdm_True);

        msl->_xmstr_collector->AddItemToList (complete_header);
    }

    // Replace the dummy entries in the scrolling list with the real headers.
    XmListReplaceItemsPos(msl->_w, msl->_xmstr_collector->GetItems(),
			  msl->_xmstr_collector->GetNumItems(), start + 1);

    delete msl->_xmstr_collector;
    msl->_xmstr_collector = NULL;

    if (done) {
	delete msl->_msgNumL;
	msl->_msgNumL = NULL;
	msl->_state = Complete;
    }

    return (done);
}

void
MsgScrollingList::deleteMe()
{
    if (this == NULL)
	return;

    if (_state == Pending) {
	_state = Destroyed;
    } else {
	delete this;
    }
}

void
MsgScrollingList::load_view(SdmError &error,
				 const char *view_name,
				 SdmString *srch_str,
				 int msg_count,
				 int view_index)
{
  long			numOfMessages;
  int			len = _msgs->length();
  XmString		complete_header; // read status + attach + header_text.
  SdmMessageStore	*mbox = this->parent()->mailbox();
  SdmIntStrLL		intStrHeaderList;
  SdmIntStrL		*intStrListPtr;
  SdmMessageFlagAbstractFlagsL flagsList = 0;
  SdmBoolean		first_new = Sdm_True;
  SdmSearch		search_obj;
  SdmMessageNumberL	msgList;
  SdmMessageNumber	select_msg_hndl=0;
  int			i;  
  DtMailGenDialog *genDialog = theRoamApp.genDialog();


  // If the load_headersWP isn't finished yet, too bad.  We must
  // destroy the msg number list and set it to NULL so that
  // load_headersWP knows to stop.
  if (_msgNumL != NULL) {
      delete _msgNumL;
      _msgNumL = NULL;
  }

  // Need to remove all the entries from the _msgs array.  There might be
  // a quicker way to do this.  Look into it later.
  //
  _msgs->remove_and_free_all_elements();
  _deleted_messages->remove_and_free_all_elements();

  _session_message_number = 0;
  _num_new_messages = 0;
  _num_deleted_messages = 0;
  _parent->setLastSortCmd(NULL);

  _current_ss = srch_str;

  XmListDeleteAllItems(_w);

  search_obj.SetSearchString(error, *srch_str);
  if (error)
      return;
  
  // It is important that we only search through those messages
  // that we know about.  Otherwise, the scrolling list will get
  // out of sync with the messagestore.
  mbox->PerformSearch(error, msgList, search_obj, 1, _last_msg_num);
  if (error)
      return;

  numOfMessages = msgList.ElementCount();
    
  // if there are no messages, clear the message area and update the message summary.
  // code is copied from display_no_message method.
  if (numOfMessages == 0) {
    _displayed_item_position = 0;
    _selected_item_position = 0;

    parent()->get_editor()->textEditor()->clear_contents();
    parent()->get_editor()->attachArea()->clearAttachArea();
    parent()->get_editor()->attachArea()->attachment_summary(0, 0);

    _parent->delete_message(_displayed_msgno);
    _displayed_msgno = 0;
   
    parent()->message_summary(
			      _selected_item_position,
			      _session_message_number, 
			      _num_new_messages,
			      _num_deleted_messages);
    return;
  }

  // Create a class to collect the mail header XmStrings
  // then get all the items from the current list.
  _xmstr_collector = new XmStrCollector();

  // Get the designated headers (defined through _header_info)
  // for all the messages into intStrHeaderList
  mbox->GetHeaders(error, intStrHeaderList, _header_info, msgList);
  
  // if we didn't get any headers for any of the messages (this should be 
  // pretty rare but we need to handle it anyway), create empty lists for each
  // message and continue processing below.
  //
  if (error == Sdm_EC_RequestedDataNotFound) {
    for (i = 0; i < numOfMessages; i++) {
      intStrHeaderList.AddElementToList(new SdmIntStrL);
    }
    error = Sdm_EC_Success;  // reset error.
  } 

  if (error) {
    return;
  }

  // Get the flags of all the messages in the mailbox (message store)
  // in the vector (array) of SdmMessageFlagAbstractFlagsL's
  mbox->GetFlags(error, flagsList, msgList);
  if (error) {
    return;
  }

  for (i = 0; i < numOfMessages; i++) {

    // Check the pending delete flag
    if ( isSet(flagsList[i], Sdm_MFA_Deleted) ) {
      insertDeletedMsg(msgList[i]);
      continue;
    }

    insertMsg(msgList[i]);

    // Get the pointer to the vector of Int/Str pairs
    // containing header-flags/values
    intStrListPtr = intStrHeaderList[i];

    complete_header = formatHeader(intStrListPtr, msgList[i],
			   _session_message_number,
			   IsMultipartMessage(intStrListPtr),
			   isSet(flagsList[i], Sdm_MFA_Seen)  ? Sdm_False : Sdm_True);


    if ( !isSet(flagsList[i], Sdm_MFA_Seen) ) {
	_num_new_messages++;

	// We want to select the last read message before the
	// first new message.  We will select the first new
	// message if it is the first message.
	if (first_new) {
	    first_new = Sdm_False;
	    // Some explanation for the following code...
	    // Need to be careful when the first non-deleted message
	    // is also the first new message.  This means that there
	    // are no earlier messages that we can display, so display
	    // the first message in the _msgs list.
	    if ((i > 0) && (i - _num_deleted_messages > 0))
		select_msg_hndl = _msgs->at(i-_num_deleted_messages-1)->message_handle;
	    else
		select_msg_hndl = _msgs->at(0)->message_handle;
	}
    }

    // Insert the XmString into the array.
    _xmstr_collector->AddItemToList (complete_header);
  }

  // Need to make sure that the non-deleted message count in the query we
  // made above jives with the message count in the views dialog.  If they
  // don't, ask the views dialog to update that particular view.
  if ((_msgs->length() != msg_count) && (msg_count != 0)) {
    Views *vp = parent()->get_views_dialog(Sdm_False, Sdm_False);
    vp->updateView(view_index);
  }

  // If there were no new messages, select and display the last message.
  if (select_msg_hndl == 0) {
    int slot = _msgs->length() - 1;
    if (slot >= 0) {
      select_msg_hndl = _msgs->at(slot)->message_handle;
    } else {
      return;
    }
  }

  // Load the UndeleteFromList dialog with the deleted messages
  // from this view.
  UndelFromListDialog *undeleteDialog = parent()->get_undel_dialog();
  if (_num_deleted_messages > 0) {
      if (undeleteDialog) {
	  MsgHndArray *copy_list = new MsgHndArray((int)_num_deleted_messages);
	  for (i=0; i<_num_deleted_messages; i++) {
	      MsgStruct *copy_msg = new MsgStruct();

	      copy_msg->sessionNumber = _deleted_messages->at(i)->sessionNumber;
	      copy_msg->message_handle = _deleted_messages->at(i)->message_handle;
	      copy_msg->is_deleted = Sdm_False;
	      copy_list->append(copy_msg);
	  }

          undeleteDialog->loadMsgs(error, copy_list, _num_deleted_messages);
          if (error) {
	    genDialog->post_error(error, DTMAILHELPCANNOTDISPLAYDELMSGS,
				  catgets(DT_catd, 3, 132, "Mailer cannot display the message headers in the Deleted Messages window."));
            return;
          }
     }
  } else {
      // Need to call undeleteDialog->loadMsgs() even when _num_deleted_messages is
      // zero so that the UndelMsgScrollingList object will remove any deleted
      // messages it had loaded from a previous view.
      if (undeleteDialog) {
	  undeleteDialog->loadMsgs(error, (MsgHndArray *)NULL, 0);
          if (error) {
	    genDialog->post_error(error, DTMAILHELPCANNOTDISPLAYDELMSGS,
				  catgets(DT_catd, 3, 132, "Mailer cannot display the message headers in the Deleted Messages window."));
            return;
          }
      }
  }

  // Add the items to the XmList.
  // All XmStrings are freed in the XmStrCollector destructor.
  _xtarg_collector = new XtArgCollector;

  // The first time the headers are loaded, they should all be loaded
  // at the same time.  XtVaSetValues is used for this rather than
  // XmListAddItems so that all the other resource will be set at
  // the same time.  This prevents multiple repaints.
  //
  // However, in the case where only an item or two are being added,
  // XmListAddItems should be used.  This prevents an unnecessary
  // repaint in this case.
  XmListAddItemsUnselected (_w, _xmstr_collector->GetItems(),
		  _xmstr_collector->GetNumItems(), 0);

  display_and_select_message(error, select_msg_hndl);
  if (error) {
    genDialog->post_error(error, DTMAILHELPCANNOTDISPLAYMSG,
			  catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
    // don't return.  just continue.  reset error.
    error.Reset();
  }

  display_message_summary();

  // get the current title of the RoamMenuWindow and append the
  // name of the view we are loading.  Like "INBOX [Boss Mail]"
  char *mbox_name = NULL, *new_title;
  mbox_name = parent()->mailbox_fullpath();

  if (mbox_name && view_name) {
    new_title = (char *)malloc(sizeof(char) * (strlen(mbox_name) +
					       strlen(view_name) + 1 + 3));
    sprintf(new_title, "%s [%s]", mbox_name, view_name);
    parent()->title((const char *)new_title);
    free (new_title);
  } else if (view_name) {
    parent()->title(view_name);
  }

  _xtarg_collector->SetValues(_w);
  if (_selected_items)
    {
      XmStringFree (_selected_items);
      _selected_items = NULL;
    }

  delete _xtarg_collector;
  delete _xmstr_collector;
  _xtarg_collector = NULL;
  _xmstr_collector = NULL;
  intStrHeaderList.ClearAndDestroyAllElements();
}

SdmMessageNumberL*
MsgScrollingList::get_deleted_message_numbers()
{
  SdmMessageNumberL *msgNums = new SdmMessageNumberL;
  int i;

  for (i=0; i<_deleted_messages->length(); i++) {
    msgNums->AddElementToList(_deleted_messages->at(i)->message_handle);
  }
  return(msgNums);
}

SdmMessageNumberL*
MsgScrollingList::get_message_numbers()
{
  SdmMessageNumberL *msgNums = new SdmMessageNumberL;
  int i;

  for (i=0; i<_msgs->length(); i++) {
    msgNums->AddElementToList(_msgs->at(i)->message_handle);
  }
  return(msgNums);
}

void MsgScrollingList::load_new_headers(SdmError &error, SdmMessageNumber nNewMsgs)
{
  DtMailGenDialog *genDialog = theRoamApp.genDialog();
  XmString complete_header; // read status + attach + header_text.
  int num_new = 0;
  SdmMessageStore * mbox = this->parent()->mailbox();
  SdmIntStrLL	intStrHeaderList;
  SdmIntStrL	*intStrListPtr;
  SdmMessageFlagAbstractFlagsL flagsList = 0;

  if (nNewMsgs == 0)
    return;

  _xmstr_collector = new XmStrCollector();

#ifdef USE_STATUS
  SdmMessageStoreStatus mboxStatus;
  SdmMessageStoreStatusFlags mboxFlags = Sdm_MSS_Messages;
  mbox->GetStatus(error, mboxStatus, mboxFlags);
  if (error)
    return;

  long numOfMessages = mboxStatus.messages;
#else
  long numOfMessages = _last_msg_num + nNewMsgs;
#endif

  // If all new headers have already been added, just return.
  if (_last_msg_num == numOfMessages)
      return;

  SdmMessageNumber start, end;
  start = _last_msg_num+1;
  end = numOfMessages;
  
  // If the current "view" is not "all messages", then see if
  // any of the new messages match the current view criteria.
  // If so, add those to the list.
  if ((_current_ss != NULL) && (*_current_ss != SdmSearch::Sdm_STK_All)) {
      SdmMessageNumberL matching_msgs;
      SdmSearch search;
      search.SetSearchString(error, (const SdmString) *_current_ss);
      if (error) return;
      mbox->PerformSearch(error, matching_msgs, search, _last_msg_num+1,
			  (const SdmMessageNumber)numOfMessages);
      if (error) return;

      int match_count = matching_msgs.ElementCount();
      if (match_count > 0) {
        mbox->GetHeaders(error, intStrHeaderList, _header_info, matching_msgs);
        // if we didn't get any headers for any of the messages (this should be 
        // pretty rare but we need to handle it anyway), create empty lists for each
        // message and continue processing below.
        //
        if (error == Sdm_EC_RequestedDataNotFound) {
          for (int i = 0; i < match_count; i++) {
            intStrHeaderList.AddElementToList(new SdmIntStrL);
          }
          error = Sdm_EC_Success;
        } 
        if (error) {
          intStrHeaderList.ClearAndDestroyAllElements();
          return;
        }
        mbox->GetFlags(error, flagsList, matching_msgs);
        if (error) {
          intStrHeaderList.ClearAndDestroyAllElements();
          return;
        }

        for (int i = 0; i < match_count; i++) {
          if ( isSet(flagsList[i], Sdm_MFA_Deleted) ) {
            insertDeletedMsg(matching_msgs[i]);
            continue;
          }
          insertMsg(matching_msgs[i]);

          intStrListPtr = intStrHeaderList[i];
          complete_header = formatHeader(intStrListPtr, matching_msgs[i],
                 _session_message_number,
                 IsMultipartMessage(intStrListPtr),
                 isSet(flagsList[i], Sdm_MFA_Seen)  ? Sdm_False : Sdm_True);
                 
          if ( !isSet(flagsList[i], Sdm_MFA_Seen) ) 
            _num_new_messages++;

          _xmstr_collector->AddItemToList (complete_header);
        }
      }
  } else {
      // Get the designated headers (defined through _header_info)
      // for the messages from the message after the _last_msg_num message
      // and up to the current last message.
      mbox->GetHeaders(error, intStrHeaderList, _header_info,
		       _last_msg_num + 1, (SdmMessageNumber)numOfMessages);

      // if we didn't get any headers for any of the messages (this should be 
      // pretty rare but we need to handle it anyway), create empty lists for each
      // message and continue processing below.
      //
      if (error == Sdm_EC_RequestedDataNotFound) {
        for (int i = 0; i < numOfMessages-_last_msg_num; i++) {
            intStrHeaderList.AddElementToList(new SdmIntStrL);
        }
        error = Sdm_EC_Success;  // reset error.
      } 
      if (error) {
        intStrHeaderList.ClearAndDestroyAllElements();
        return;
      }
      // Get the flags of or the messages from the message after the
      // "last" message and up to the current last message.
      mbox->GetFlags(error, flagsList, _last_msg_num+1,
		     (SdmMessageNumber)numOfMessages);
      if (error) {
        intStrHeaderList.ClearAndDestroyAllElements();
        return;
      }
      
      for (int i = _last_msg_num+1, j=0; i <= numOfMessages; i++, j++) {
        if ( isSet(flagsList[j], Sdm_MFA_Deleted) ) {
            insertDeletedMsg(i);
            continue;
        }
        insertMsg(i);
        // Get the pointer to the vector of Int/Str pairs
        // containing header-flags/values
        intStrListPtr = intStrHeaderList[j];
        complete_header = formatHeader(intStrListPtr, i, i,
             IsMultipartMessage(intStrListPtr),
             isSet(flagsList[j], Sdm_MFA_Seen)  ? Sdm_False : Sdm_True);
             
        if ( !isSet(flagsList[j], Sdm_MFA_Seen) ) 
            _num_new_messages++;
        _xmstr_collector->AddItemToList (complete_header);
      }
  }

  // update the handle of the last known message number.
  _last_msg_num = numOfMessages;


  // Add the items to the XmList.
  // All XmStrings are freed in the XmStrCollector destructor.
  _xtarg_collector = new XtArgCollector;

  // The first time the headers are loaded, they should all be loaded
  // at the same time.  XtVaSetValues is used for this rather than
  // XmListAddItems so that all the other resource will be set at
  // the same time.  This prevents multiple repaints.
  //
  // However, in the case where only an item or two are being added,
  // XmListAddItems should be used.  This prevents an unnecessary
  // repaint in this case.
  XmListAddItemsUnselected (_w, _xmstr_collector->GetItems(),
		  _xmstr_collector->GetNumItems(), 0);

  do_list_vis_adjustment();

  display_message_summary();

  _xtarg_collector->SetValues(_w);
  if (_selected_items) {
    XmStringFree (_selected_items);
    _selected_items = NULL;
  }
  
  // Do auto-updating of Views window if it has been created.
  Views *vp = parent()->get_views_dialog(Sdm_False, Sdm_False);
  if (vp && vp->isMapped()) {
    SdmMessageNumberL emptyList;
    vp->updateViewsWindow(
        error,
        Sdm_True,	// regenerate
			  Sdm_False,	// unread_only
			  Range,	// type
			  start, end,
			  emptyList,
			  Sdm_True);
			  
    if (error) {
      genDialog->post_error(error, DTMAILHELPCANNOTUPDATEVIEWSTATS,
			    catgets(DT_catd, 17, 24, "Mailer cannot update the message statistics for your views."));
      // don't return.  continue processing.  reset error.
      error.Reset();
    }
  }

  delete _xtarg_collector;
  delete _xmstr_collector;
  _xtarg_collector = NULL;
  _xmstr_collector = NULL;
  intStrHeaderList.ClearAndDestroyAllElements();
}


void MsgScrollingList::do_list_vis_adjustment()
{
  int numNew = _xmstr_collector->GetNumItems();
  int visCnt, cItmCnt, pItmCnt, sItmCnt, cTopItm, cBotItm;
  int cFocItm = _selected_item_position;

  XtVaGetValues(_w,
		XmNvisibleItemCount, &visCnt,
		XmNitemCount, &cItmCnt,
		XmNselectedItemCount, &sItmCnt,
		XmNtopItemPosition, &cTopItm, NULL);
 
  pItmCnt = cItmCnt - numNew;
 
  cBotItm = cTopItm + visCnt - 1; // cTopItm...cBotItm is our window
 
  // User has chosen to view some messages and that view needs to be
  // maintained.
  if (cBotItm != pItmCnt)
    return;
 
  if (cItmCnt <= visCnt)
    return;
 
  // If no items selected, display the last n msgs
  if (sItmCnt == 0) {
    XmListSetBottomPos(_w, cItmCnt);
    return;
  }

  if ((cFocItm <= cBotItm) && (cFocItm >= cTopItm)) {
    int winM = cFocItm - cTopItm;
    if (winM <= 0) {
      return;
    }
    // All the new messages can be accomodated w/o scrolling curr selection
    if (numNew <= winM) {
      XmListSetBottomPos(_w, cItmCnt);
      return;
    } else {   // All the new messages cannot be accomodated, but we will do best fit
      int numNotShow = numNew - winM;
      XmListSetBottomPos(_w, cItmCnt - numNotShow);
      return;
    }
  } else {
    XmListSetBottomPos(_w, cItmCnt);
    return;
  }
}

void 
MsgScrollingList::deleteSelected(SdmBoolean silent)
{
  const MsgStruct *a_del_msg_struct;
  const MsgStruct *tmpMS;
  FORCE_SEGV_DECL(ViewMsgDialog, tmpView);
  DtMailGenDialog *genDialog = theRoamApp.genDialog();
  SdmMessageNumber tmpMH;
  SdmError mail_error;
  int  position_in_list, i;
  FORCE_SEGV_DECL(int, position_list);
  int position_count;
  int num_msgs;
  UndelFromListDialog *undel_dialog;
  int first_selected_pos;
  char *status_message = NULL, *str = NULL;
  SdmMessageNumberL msgNumList;

  mail_error.Reset();

  // Return if there aren't any messages selected.
  if (!XmListGetSelectedPos(_w, &position_list, &position_count)) 
    return;

  // Of the items in the list, potentially many could be selected
  // for delete.  And those selected for deleted need not be 
  // contiguous necessarily.  Note the position of the item that 
  // appears first in the list.  When the items are deleted, other
  // items if any exist will shift up to take the places of the 
  // deleted items.  You want to display the corresponding item that
  // takes the "first position".

  // Say, if you have messages A, B, C, D, E visible in that order.
  // You select B and C for delete.
  // The first selected pos is 2.
  // When B and C are deleted, D (and E...) move up.
  // The new order is A, D, E...
  // You want to display the second message which is D.

  first_selected_pos = *position_list;

  undel_dialog = parent()->get_undel_dialog();

  // If the load_headersWP routine is not finished loading all of the
  // msg headers into the scrolling list, then remove the corresponding
  // entry from the msg number list and update first and last index
  // indices.  The reason we loop from position_count down to 1 is
  // because if we remove the first element in the list all the other
  // elements in the list shift down and we are no longer referencing
  // the correct msgs.
  if (_msgNumL != NULL) {
      for (i=position_count; i>0; i--) {
	  _msgNumL->RemoveElement(position_list[i-1] - 1);
	  if ((position_list[i-1]) <= _last_msg_index_loaded) {
	      _last_msg_index_loaded--;
	      if ((position_list[i-1]) <= _first_msg_index_loaded) {
		  _first_msg_index_loaded--;
	      }
	  }
      }
  }

  for (i=0; i < position_count; i++ ) {
    position_in_list = *(position_list + i);
    a_del_msg_struct = get_message_struct(position_in_list);
    _msgs->mark_for_delete(position_in_list - 1);
    _deleted_messages->append((MsgStruct *)a_del_msg_struct);

    if (undel_dialog) {
      MsgStruct *copy = new MsgStruct();
      copy->sessionNumber = a_del_msg_struct->sessionNumber;
      copy->message_handle = a_del_msg_struct->message_handle;
      copy->is_deleted = Sdm_False;
      undel_dialog->insertMsg(mail_error, copy);
    }

    if (mail_error) {
      // need to compact otherwise, the same structure could end
      // up in both the delete list and the regulst list.
      _msgs->compact(0);
      genDialog->post_error(mail_error, DTMAILHELPCANNOTUPDATEDELMSGHDRLIST,
			    catgets(DT_catd, 3, 135, "Mailer cannot update the message headers in the Deleted Messages window."));
      return;
    }

    // See if there is a standalone view of the message.
    // If there is, quit it.

    tmpMH = a_del_msg_struct->message_handle;
    tmpView = parent()->ifViewExists(tmpMH);
    if (tmpView) {
      tmpView->quit();
    }

    // create list of message numbers for the deleted messages.
    msgNumList.AddElementToList(a_del_msg_struct->message_handle);
  }

  _msgs->compact(0);

  // get the current flags for the deleted messages.    
  SdmMessageFlagAbstractFlagsL flagsList;
  _parent->mailbox()->GetFlags(mail_error, flagsList, msgNumList);
  if (mail_error) {
    genDialog->post_error(mail_error, DTMAILHELPCANNOTDELETEMSGS,
			  catgets(DT_catd, 3, 129, "Mailer cannot delete the messages you selected."));
    return;
  }

  // Check if message is new.  If new and it is being deleted,
  // reduce the new messages count.
  for (i=0; i<flagsList.ElementCount(); i++) {
    if (!isSet(flagsList[i], Sdm_MFA_Seen)) {
      _num_new_messages--;
    }
  }

  // We must call updateViewsWindow before setting the deleted
  // flags on the messages so that we can calculate the unread
  // count correctly.
  Views *vp = parent()->get_views_dialog(Sdm_False, Sdm_False);
  if (vp && vp->isMapped()) {
    vp->updateViewsWindow(mail_error,
			  Sdm_True,	// regenerate
			  Sdm_False,	// unread_only
			  List,		// type
			  0, 0,		// start, end
			  msgNumList,	// msg list
			  Sdm_False);	// add to
			  
    if (mail_error) {
      genDialog->post_error(mail_error, DTMAILHELPCANNOTUPDATEVIEWSTATS,
			    catgets(DT_catd, 17, 24, "Mailer cannot update the message statistics for your views."));
      // don't return.  continue processing.  reset error.
      mail_error.Reset();
    }
  }

  // Update the status of the message in the persistent store.
  //
  _parent->mailbox()->SetFlags(mail_error, Sdm_MFA_Deleted, msgNumList);
  if (mail_error) {
    genDialog->post_error(mail_error, DTMAILHELPCANNOTDELETEMSGS,
			  catgets(DT_catd, 3, 129, "Mailer cannot delete the messages you selected."));
    return;
  }

  // Delete the items from the scrolling list
  XmListDeletePositions(_w,
			position_list,
			position_count);

  _num_deleted_messages += position_count;
  _total_deleted += position_count;

  num_msgs = _msgs->length();
  parent()->get_editor()->attachArea()->clearAttachArea();

  if (num_msgs == 0) {  //No more messages left; clear the editor.
    parent()->get_editor()->textEditor()->set_contents("", 1);
    // Clean up the attachment status summary as well
    parent()->get_editor()->attachArea()->attachment_summary(0, 0);

    // NOTE
    // Need to obtain the attachArea and clear it up too

    _displayed_item_position = 0;	// Not displaying any.
    _selected_item_position = 0;	// Reset...
    _parent->delete_message(_displayed_msgno);
    _displayed_msgno = 0;
   
    free(position_list);
    parent()->message_summary(
			      _selected_item_position,
			      _session_message_number, 
			      _num_new_messages,
			      _num_deleted_messages);
    return;
  }

  // If there are messages 1, 2 and 3 and 2 is selected and deleted
  // first_selected_pos will be 2;
  // after removing it, will need to display the 2nd message
  // (which will now be 3)
  // If 2 and 3 have been deleted, and there is no 2nd message,
  // display the nth message (in our example, n = 1)
    
  _selected_item_position = first_selected_pos;
  if (_selected_item_position > num_msgs) {
    _selected_item_position = num_msgs; 
  }

  // Having determined which message to display, confirm that it is
  // within the bounds.  Call display_message().
  // FYI, display_message() sets the _displayed_item_position.

  if ((_selected_item_position > 0) && (_selected_item_position <= num_msgs)) {
    XmListSelectPos(_w, _selected_item_position, Sdm_False);
    tmpMS = this->get_message_struct(_selected_item_position);
    if (tmpMS == NULL) {
      free(position_list);
      return;
    } else {
      this->display_message(mail_error, tmpMS->message_handle);
      if (mail_error) {
	genDialog->post_error(mail_error, DTMAILHELPCANNOTDISPLAYMSG,
			      catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
        // don't return.  just continue.  reset error.
        mail_error.Reset();
      }
    }
  }

  if (!silent) {
    if (position_count > 1) {
      /* NL_COMMENT
       * The following sentence means %d number of mail messages have 
       * been deleted from the mail folder.  This is the plural form 
       * of the message that gets printed if more than one message 
       * is moved.
       */
      str = catgets(DT_catd, 3, 84, "%d messages deleted"); 
    } else {
      /* NL_COMMENT
       * The following sentence means %d number of mail messages have 
       * been deleted from the mail folder.  This is the singular 
       * form of the message that gets printed if only one message 
       * is moved.
       */
      str = catgets(DT_catd, 3, 85, "%d message deleted"); 
    }
    int status_message_length = strlen(str) + 10;
    status_message = new char[status_message_length];
    memset(status_message, 0, status_message_length);
    sprintf(status_message, str, position_count);
    parent()->message(status_message);
    delete status_message;
  }

  parent()->message_summary(
			    _selected_item_position,
			    _session_message_number, 
			    _num_new_messages,
			    _num_deleted_messages);

  free(position_list);
}


SdmBoolean
inIgnoreHeaders(SdmStringL *ignoreList, const char *hdr)
{
	for (int i = 0; i < ignoreList->ElementCount(); i++) {
		if ( strcasecmp((const char *)(*ignoreList)[i], hdr) == 0 )
			return Sdm_True;
	}

	return Sdm_False;
}

static void
display_msg(SdmError &error, int fd, char *destname, SdmMessage *msg, int level, int *att)
{
    int			bodyCount;
    SdmMessage		*nestedMsg;
    SdmMessageBody	*mbody;
    SdmMessageBodyStructure bodyStruct;
    SdmContentBuffer	contents_buf;
    SdmString		contents;
    SdmString		type;
    char		buf[2048];
    char		attBuff[1024];

    error.Reset();
    
    DtMailGenDialog *genDialog = theRoamApp.genDialog();

    msg->GetBodyCount(error, bodyCount);
    if (error) return;

    for (int i = 1; i <= bodyCount; i++) {
      msg->SdmMessageBodyFactory(error, mbody, i);
      if (error) return;
      mbody->GetStructure(error, bodyStruct);
      if (error) return;
      if ( bodyStruct.mbs_type == Sdm_MSTYPE_multipart ) {
          mbody->SdmMessageFactory(error, nestedMsg);
          if (!error) {
            display_msg(error, fd, destname, nestedMsg, level + 1, att);
          } else {
            error = Sdm_EC_Success;
          }
      }
      else {
          mbody->GetContents(error, contents_buf);
          if (error) return;
          contents_buf.GetContents(error, contents);
          if (error) return;
          
          if ( level == 1 && (bodyStruct.mbs_type == Sdm_MSTYPE_none ||
               bodyStruct.mbs_type == Sdm_MSTYPE_text ||
               bodyStruct.mbs_type == Sdm_MSTYPE_message) ) 
          {
            if (SdmSystemUtility::SafeWrite(fd, contents, contents.Length()) < contents.Length()) {
                sprintf(buf,
                  catgets(DT_catd, 3, 53, "Unable to write to %s."),
                  destname);
                genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
                genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
                SdmSystemUtility::SafeClose(fd);
                SdmSystemUtility::SafeUnlink(destname);
                return;
            }
          }
          else {

            (*att)++; // Update the attachment counter

            SdmDataTypeUtility::DetermineCdeDataTypeForMessage(error, type, *mbody, &bodyStruct);

            sprintf(attBuff, "\n\n[%d] \"%s\" %s, %d bytes\n\n",
              *att, (const char *)bodyStruct.mbs_attachment_name,
              (const char *)type, contents.Length());

            // Print a short description of the attachment
	    int attBuff_Length = strlen(attBuff);
            if (SdmSystemUtility::SafeWrite(fd, attBuff, attBuff_Length) < attBuff_Length) {
                sprintf(buf,
                  catgets(DT_catd, 3, 53, "Unable to write to %s."),
                  destname);
                genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
                genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
                SdmSystemUtility::SafeClose(fd);
                SdmSystemUtility::SafeUnlink(destname);
                return;
            }
          }
      }
    }
}

int
MsgScrollingList::copyPrintSelected(SdmError &error,
				    char *destname)
{
    FORCE_SEGV_DECL(int, position_list);
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    int position_count, position, i, j;
    int			bodyCount;
    int			att = 0;
    char		buf[2048];
    SdmMessageNumber	msg;
    SdmMessageBody	*mbody;
    SdmMessageBodyStructure bodyStruct;
    SdmMessageStore     *mbox=NULL;
    SdmMessageEnvelope	*env;
    SdmContentBuffer	contents_buf;
    SdmString		contents;
    SdmString		first, second;
    SdmStringL		*ignoreList;
    SdmStrStrL		headers;
    SdmMailRc		*mailrc;
    char		*value = NULL;
    char		hdrPtr[2048];

    error.Reset();
    
    if (!XmListGetSelectedPos(_w, &position_list, &position_count)) {
      return(1);
    }

    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailrc);
    if (error) {
      return (1);
    }  

    ignoreList = mailrc->GetIgnoreList();

    mbox = parent()->mailbox();

    int fd = SdmSystemUtility::SafeOpen(destname, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) {
      sprintf(buf,
	      catgets(DT_catd, 3, 51, "Unable to create %s."),
	      destname);
      genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
      genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOCREATE);
      return(1);
    }

    // Go through the selected messages and copy them to the
    // specified container.

    for (i=0; i < position_count; i++) {
      SdmMessage *dtmsg;
      SdmMessage *nestedMsg;

      position = *(position_list + i);
      msg = msgno(position);

      if (mbox->SdmMessageFactory(error, dtmsg, (long) msg)) {
          SdmSystemUtility::SafeClose(fd);
          SdmSystemUtility::SafeUnlink(destname);
          return(0);
      }

      dtmsg->SdmMessageEnvelopeFactory(error, env);

      if ( !error ) {
          env->GetHeaders(error, headers);
          // if no headers found, just reset error and continue.
          if (error == Sdm_EC_RequestedDataNotFound) {
            error = Sdm_EC_Success;
          }
          if (!error) {
            for (j = 0; j < headers.ElementCount(); j++) {
              first = headers[j].GetFirstString();
              if ( !inIgnoreHeaders(ignoreList, (const char *)first) ) {
                  second = headers[j].GetSecondString();
                  sprintf(hdrPtr, "%s: %s\n", (const char *)first, (const char *)second);
                  if (SdmSystemUtility::SafeWrite(fd, hdrPtr, strlen(hdrPtr)) < strlen(hdrPtr)) {
                    sprintf(buf,
                      catgets(DT_catd, 3, 53, "Unable to write to %s."),
                      destname);
                    genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
                    genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
                    SdmSystemUtility::SafeClose(fd);
                    SdmSystemUtility::SafeUnlink(destname);
                    return(1);
                  }
              }
            }
          }
      }

      if (!error) {
        // Space the header part from the body by one new-line
        if (SdmSystemUtility::SafeWrite(fd, "\n", 1) < 1) {
            sprintf(buf, 
              catgets(DT_catd, 3, 53, "Unable to write to %s."),
              destname);
            genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
            genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
            SdmSystemUtility::SafeClose(fd);
            SdmSystemUtility::SafeUnlink(destname);
            return(0);
        }

        dtmsg->GetBodyCount(error, bodyCount);
        if (!error) {
          for (j = 1; j <= bodyCount; j++) {
            dtmsg->SdmMessageBodyFactory(error, mbody, j);
            if (error) break;
            mbody->GetStructure(error, bodyStruct);
            if (error) break;
            if ( bodyStruct.mbs_type == Sdm_MSTYPE_multipart ) {
              mbody->SdmMessageFactory(error, nestedMsg);
              display_msg(error, fd, destname, nestedMsg, 1, &att);
            }
            else {
              mbody->GetContents(error, contents_buf);
              if (error) break;
              contents_buf.GetContents(error, contents);
              if (SdmSystemUtility::SafeWrite(fd, contents, contents.Length()) < contents.Length()) {
                  sprintf(buf, 
                    catgets(DT_catd, 3, 53, "Unable to write to %s."),
                    destname);
                  genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
                  genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
                  SdmSystemUtility::SafeClose(fd);
                  SdmSystemUtility::SafeUnlink(destname);
                  return(0);
              }
            }
          }
        }
      }

      if ( !error && i ) {
          if (SdmSystemUtility::SafeWrite(fd, "", 1) < 1) {
            sprintf(buf, 
              catgets(DT_catd, 3, 53, "Unable to write to %s."),  
              destname);
            genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
            genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
            SdmSystemUtility::SafeClose(fd);
            SdmSystemUtility::SafeUnlink(destname);
            return(0);
          }
      }

      headers.ClearAllElements();
      parent()->delete_message(msg);
    }

    SdmSystemUtility::SafeClose(fd);
    
    if (error) {
      SdmSystemUtility::SafeUnlink(destname);
      return (1);
    } else {
      return (0);
    }
}

int
MsgScrollingList::copySelectedMsgs(SdmError &error,
				    char *destname)
{
    FORCE_SEGV_DECL(int, position_list);
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    int			position_count, position, i;
    char		buf[2048];
    SdmMessageNumber	msg;
    SdmMessageBodyStructure bodyStruct;
    SdmMessageStore     *mbox=NULL;
    SdmContentBuffer	contents_buf;
    SdmString		contents;
    SdmMailRc		*mailrc;

    error.Reset();
    
    if (!XmListGetSelectedPos(_w, &position_list, &position_count)) {
      return(1);
    }

    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailrc);
    if (error) {
      return (1);
    }  

    mbox = parent()->mailbox();

    int fd = SdmSystemUtility::SafeOpen(destname, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) {
      sprintf(buf,
	      catgets(DT_catd, 3, 51, "Unable to create %s."),
	      destname);
      genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
      genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOCREATE);
      return(1);
    }

    // Go through the selected messages and copy them to the
    // specified container.

    for (i=0; i < position_count; i++) {
      SdmMessage *dtmsg;

      position = *(position_list + i);
      msg = msgno(position);

      if (mbox->SdmMessageFactory(error, dtmsg, (long) msg)) {
          SdmSystemUtility::SafeClose(fd);
          SdmSystemUtility::SafeUnlink(destname);
          return(0);
      }

      dtmsg->GetContents(error, contents_buf);
      contents_buf.GetContents(error, contents);

      if ( !error ) {
        if (SdmSystemUtility::SafeWrite(fd, contents, contents.Length()) < contents.Length()) {
                    sprintf(buf,
                      catgets(DT_catd, 3, 53, "Unable to write to %s."),
                      destname);
          genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
          genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
          SdmSystemUtility::SafeClose(fd);
          SdmSystemUtility::SafeUnlink(destname);
          return(1);
        }
      }
      
      parent()->delete_message(msg);
    }

    SdmSystemUtility::SafeClose(fd);
    
    if (error) {
      SdmSystemUtility::SafeUnlink(destname);
      return (1);
    } else {
      return (0);
    }
}


// copySelected() will either copy or move the selected messages
// into the container called destname.  If the delete_after_copy
// flag is set to Sdm_True, it is effectively a move; otherwise, it
// is a copy.  It is assumed that the path is not relative when it gets here,
// so you have to ensure it is fully qualified on setup.
// If silent is Sdm_True then no status messages are displayed and the
// destname is not added to the copy/move cache.

int
MsgScrollingList::copySelected(SdmError &error,
			       char *destname, 
			       char *server, 
			       int delete_after_copy,
			       int silent)
{
  FORCE_SEGV_DECL(int, position_list);

  // It's a read-only mailbox, can't move. We probably wouldn't get into here
  // because all move menus should have been greyed out.
  if (delete_after_copy && _parent->mailBoxWritable() == Sdm_False) {
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), 
				catgets(DT_catd, 3, 129, "Mailer cannot delete the messages you selected."));
    genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPCANNOTDELETEMSGS);
    return 0;
  }

  int position_count, position, i;
  SdmMessageNumber msg;
  SdmMailRc *mailrc = NULL;
  SdmSession *i_session = theRoamApp.isession()->isession();
  char *status_message = NULL, *str = NULL, name[2048], *newdestname = NULL;
  SdmToken token;  
  SdmMessageStore     *mbox, *target=NULL;

  theRoamApp.busyAllWindows();

  // If there aren't any selected messages, then there isn't
  // anything for us to do.

  if (!XmListGetSelectedPos(_w, &position_list, &position_count)) {
    theRoamApp.unbusyAllWindows();
    if (delete_after_copy) 
      parent()->message(catgets(DT_catd, 3, 63,
				"Select a message to move."));
    else
      parent()->message(catgets(DT_catd, 3, 64,
				"Select a message to copy."));
    return(1);
  }

  mbox = parent()->mailbox();
  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailrc);
  if (!mailrc) {
    theRoamApp.unbusyAllWindows();
    // NL_COMMENT
    // The following is an error message.  "mailrc" is the name of the
    // mail resource file.  Translate as appropriate.
    //
    parent()->message(catgets(DT_catd, 2, 15,
			      "Error - Unable to get mailrc."));
    return(1);
  }

  error.Reset();

  // Make sure the mailbox has a full path.
  char *fn, *dir;


  name[0] = '\0';
  if (server == NULL || *server == '\0') {
        token.SetValue("servicetype", "local");

  	SdmUtility::ExpandPath(error, fn, destname, *mailrc,
	 	SdmUtility::kFolderResourceName);

  	// If local mailbox, make sure it has a full path
	if (fn[0] != '/') {
        	// Relative path, so need to get directory
		dir = theRoamApp.getFolderDir(local, Sdm_True);
		// If the dir IS the forward slash...
		if (strcmp(dir, "/") == 0)
			sprintf(name, "/%s", fn);
		else
			sprintf(name, "%s/%s", dir, fn);
		free(dir);
        }
  	else
		strcpy(name, fn);
	free(fn);
  }
  else { // Imap mailbox...
	if (strcmp(destname, "INBOX") == 0)
		strcpy(name, "INBOX");
	else {
		char *nm = strdup(destname);
    		if (!theRoamApp.resolvePath(&nm, server)) {
      			theRoamApp.unbusyAllWindows();
			free(nm);
      			return 0;
    		}
		strcpy(name, nm);
		free(nm);
	}
	token.SetValue("serviceoption", "hostname",
                thePasswordDialogManager->imapserver());
        token.SetValue("serviceoption", "username",
                thePasswordDialogManager->user());
        token.SetValue("serviceoption", "password",
                thePasswordDialogManager->password());
        token.SetValue("servicetype", "imap");
  }

  SdmString storeName;
  int len;
  if (name && ((len = strlen(name)) > 0)) {
    int j = len-1;
    int lastDelim = -1;
    while (name[j] == name[0] && j >= 0) {
      lastDelim = j;
      j--;
    }
    if (lastDelim >= 0) {
      storeName.Append(name, lastDelim);
    } else {
      storeName = name;
    }
  } 

  token.SetValue("serviceoption", "messagestorename", (const char*)storeName);

  if (mailrc->IsValueDefined("dontsavemimemessageformat"))
	token.SetValue("serviceoption", "preservev3messages", "x");

  SdmMessageNumberL msgNumList;
  for (i=0; i < position_count; i++) {
    position = *(position_list + i);
    msgNumList.AddElementToList( msgno(position) );
  }
  

  // first we'll try to copy the messages just using the message numbers.
  // this is faster than copying them one at a time.
  if (mbox->CopyMessages(error, token, msgNumList) != Sdm_EC_Success) {
  
    // The copy did not work.  it could be that the source and 
    // destination stores are not on the same server.  
    // therefore, we'll need to copy the messages
    // the hard way - one message at a time.  try to
    // open the destination store.
    //
    SdmBoolean readOnly = Sdm_False;
    SdmMessageNumber nmsgs = 0;
    error.Reset();
    target = theRoamApp.isession()->open(error, nmsgs, readOnly, token, NULL);

    // open did not work.  return with error code.
    if (error != Sdm_EC_Success || target == NULL) {
      theRoamApp.unbusyAllWindows();
      return 0;

    } else {
    
      // now we've opened the mailbox.  Perhaps the original CopyMessages
      // failed because the target mailbox did not exist.  Let's try the 
      // CopyMessages one more time to see if it works.
      //
      if (mbox->CopyMessages(error, token, msgNumList) != Sdm_EC_Success) {
      
        // Well, the second attempt at using CopyMessages did not work.
        // We need to copy the messages the hard way, one at a time.
        // Go through the selected messages and copy them to the
        // specified container one at a time.
        for (i=0; i < position_count; i++) {
          position = *(position_list + i);
          msg = msgno(position);
          SdmMessage *dtmsg;
          if (mbox->SdmMessageFactory(error, dtmsg, (long) msg)) {
            theRoamApp.isession()->close(error, target);
            theRoamApp.unbusyAllWindows();
            return(0);
          }

          error.Reset();
          if (target->AppendMessage(error, *dtmsg)) {
            parent()->postErrorDialog(error);
            theRoamApp.isession()->close(error, target);
            theRoamApp.unbusyAllWindows();
            return(0);
          }
          
          parent()->delete_message(msg);
        }
      }
    }
  }
  
  if (delete_after_copy) {
    deleteSelected();

    if (i > 1) {
      // NL_COMMENT
      // The following sentence means %d number of mail messages have 
      // been moved to the %s mail folder.  The %s is the name of a 
      // mail folder.  This is the plural form of the message that gets
      // printed if more than one message is moved.
      //
      str = catgets(DT_catd, 3, 65, "%d messages moved to %s"); 
    } else {
      // NL_COMMENT
      // The following sentence means %d number of mail messages have 
      // been moved to the %s mail folder.  The %s is the name of a 
      // mail folder.  This is the singular form of the message that
      // gets printed if only one message is moved.
      //
      str = catgets(DT_catd, 3, 66, "%d message moved to %s"); 
    }

  } else {
    if (i > 1) {
      // NL_COMMENT
      // The following sentence means %d number of mail messages have been
      // copied to the %s mail folder.  This is the plural form of the
      // message that gets printed if more than one message is copied.
      //
      str = catgets(DT_catd, 3, 67, "%d messages copied to %s"); 
    } else {
      // NL_COMMENT
      // The following sentence means %d number of mail messages have been
      // copied to the %s mail folder.  This is the singular form of the
      // message that gets printed if only one message is copied.
      //
      str = catgets(DT_catd, 3, 68, "%d message copied to %s"); 
    }
  }
    
  if (server != NULL && *server != '\0') {
      char *newdestname2 = (char*)malloc(storeName.Length()
                                         + strlen(server) + 3);
      sprintf(newdestname2, "%s(%s)", (const char*)storeName, server);
      SdmUtility::GetRelativePath(error, newdestname, newdestname2,
			  *mailrc, SdmUtility::kImapFolderResourceName);
      free(newdestname2);
  }
  else
      SdmUtility::GetRelativePath(error, newdestname, (const char*)storeName,
                                  *mailrc, SdmUtility::kFolderResourceName);
    
  status_message = (char*)malloc(strlen(str) + strlen(newdestname) + 10);
  sprintf(status_message, str, i, newdestname);

  // Dont add inbox to cache list
  if (!silent) 
    parent()->addToRecentList(newdestname);

  free(newdestname);

  if (target) {
    theRoamApp.isession()->close(error, target);
  }
  theRoamApp.unbusyAllWindows();

  parent()->message(status_message);
  free(status_message);

  return(0);
}

//-----------------------------------------------------------------------------
// This method returns a list of the currently selected messages.  This list
// must be deleted by the calling method.
//-----------------------------------------------------------------------------
 
MsgHndArray *
MsgScrollingList::selected()
{
//   FORCE_SEGV_DECL(MsgStruct, a_msg_struct);
  const MsgStruct *a_msg_struct;
  FORCE_SEGV_DECL(int, position_list);
  int  position_in_list, i;
  int position_count;
  SdmBoolean any_selected;
 
  // Find out first if any have been selected.
  // If i has been selected, how many?
  // We need the number selected so that we can allocate
  // space for that many mesasgeStructs to be returned.
 
  any_selected = (SdmBoolean) XmListGetSelectedPos(_w,
                                                   &position_list,
                                                   &position_count);
 
  // If nothing selected, return
 
  if (!any_selected) return NULL;
 
  //  Allocate memory for position_count number of messageStructs
  // in MsgHndArray.
 
  MsgHndArray *msgList = new MsgHndArray(position_count);
 
  for (i=0; i < position_count; i++ ) {
    position_in_list = *(position_list + i);
    a_msg_struct = get_message_struct(position_in_list);
    msgList->append((MsgStruct *)a_msg_struct);
  }
  return msgList;
}


void
MsgScrollingList::selected_msg_hndls( SdmMessageNumberL &msgs)
{
  FORCE_SEGV_DECL(int, position_list);
  int index, i;
  int position_count;
  SdmBoolean any_selected;

  // Find out first if any have been selected.
  // If i has been selected, how many?
  // We need the number selected so that we can allocate 
  // space for that many mesasgeStructs to be returned.

  any_selected = (SdmBoolean) XmListGetSelectedPos(_w,
						   &position_list, 
						   &position_count);

  // If nothing selected, return
  if (!any_selected)
    return;

  for (i=0; i < position_count; i++ ) {
    index = *(position_list + i) - 1;
    msgs.AddElementToList(_msgs->at(index)->message_handle);
  }
}

// Print out the contents of the message.  Used for debugging.
void
debug_display_msg(SdmMessage *msg, int level)
{
  SdmMessage		*nestedMsg;
  SdmMessageBody	*mbody;
  SdmMessageEnvelope	*env;
  SdmMessageBodyStructure bodyStruct;
  SdmStrStrL		headers;
  int			bodyCount;
  SdmError		error;
  SdmContentBuffer	contents_buf;
  SdmString		contents;
  SdmString		first, second;

  error.Reset();

  msg->SdmMessageEnvelopeFactory(error, env);
  if ( !error ) {
    env->GetHeaders(error, headers);
    printf("\n\n\tLevel %d Envelope\n", level);
    for (int i = 0; i < headers.ElementCount(); i++) {
      first = headers[i].GetFirstString();
      second = headers[i].GetSecondString();
      printf("\t%s:\t%s\n", (const char *)first, (const char *)second);
    }
  }

  printf("\n\n***** Level %d body parts *****\n", level);

  msg->GetBodyCount(error, bodyCount);
  for (int i = 1; i <= bodyCount; i++) {
    msg->SdmMessageBodyFactory(error, mbody, i);
    mbody->GetStructure(error, bodyStruct);
    if ( bodyStruct.mbs_type == Sdm_MSTYPE_multipart ) {
      mbody->SdmMessageFactory(error, nestedMsg);
      debug_display_msg(nestedMsg, (level + 1));
    }
    else {
      mbody->GetContents(error, contents_buf);
      contents_buf.GetContents(error, contents);
      printf("Body part:\t%d\n", i);
      printf("%s", (const char *)contents);
      printf("\n***** End *****\n\n");
    }
  }
}

void
MsgScrollingList::display_message(SdmError &mail_error,
				  SdmMessageNumber msg_num,
				  int item_pos)	// default value is zero
{
  DtMailGenDialog *genDialog = theRoamApp.genDialog();
  SdmMessageStore	*mbox=parent()->mailbox();
  Editor*		rmw_editor;
  int num_selected;
  mail_error.Reset();


  // Turn on the busy Cursor
  parent()->busyCursor();

  // If there is a status message displayed, clear it first.  Also clear the
  // bottom status area.
  parent()->clearStatus();
  parent()->get_editor()->attachArea()->bottomStatusMessage(" ");

  // We could have called display_message from anywhere.
  // Need to calculate what is the position of that item
  // in the scrolling list, given the DtMailMessageHandle.
  // Determine the index at the _msgs array; increment by 1
  // since array begins at 0 and XmList begins at 1.

  // if item_pos is passed in as non-zero, then just use that position.
  if (item_pos == 0) {
    _displayed_item_position = _msgs->index(msg_num) + 1;
    // check for error condition
    if (_displayed_item_position == 0) {
      parent()->normalCursor();
      return;
    }
  } else {
    _displayed_item_position = item_pos;
  }

  // Make sure the header is visible in the scrolling list.
  this->scroll_to_position(_displayed_item_position);

  // get the handle for the message.
  SdmMessage * msg;
  mbox->SdmMessageFactory(mail_error, msg, msg_num);
  if (mail_error) {
    parent()->normalCursor();
    return;
  }


  // There are multiple paths to this place:
  // 1) user has already read this message but is re-reading it;
  // 2) user has undeleted this message (implicitly, they have
  //    read this message)
  // gl - #2 is not necessarily true.  The user could have
  // deleted a range of messages which contained the message
  // we are preparing to display.  In this case it would have
  // been unread previously but is now being changed to "seen".
  //
  // 3) user has not read this message yet.
  //
  // For (1), we don't need to do anything fancy.
  // For (2), we have already reset the IsDeleted flag while 
  // undeleting.
  // For (3), we need to reset the flag in store to indicate
  // its read.

  SdmMessageFlagAbstractFlags flags = 0;
  msg->GetFlags(mail_error, flags);
  if (mail_error) {
    parent()->normalCursor();
    return;
  }

  // If the message was previously new, we need to reset the list
  // item to remove the "N" on the item.  We do this by reconstructing
  // the header line without the "N".
  //
  if (!isSet(flags, Sdm_MFA_Seen) && _parent->mailBoxWritable()) {
    const MsgStruct *ms;
    ms = get_message_struct(_displayed_item_position);

    SdmIntStrL	intStrHeaderList;

    // if we didn't get any headers for the messages reset the
    // error and continue processing below.  the message scrolling
    // list will end up showing empty fields for this message.
    //
    mbox->GetHeaders(mail_error, intStrHeaderList, _header_info,
		     (SdmMessageNumber)msg_num);
		     
    if (mail_error == Sdm_EC_RequestedDataNotFound) {
      // just continue if we couldn't get the headers.
      mail_error = Sdm_EC_Success;  
    } else if (mail_error) {
      parent()->normalCursor();
      return;
    }

    XmString complete_header;
    complete_header = formatHeader(&intStrHeaderList,
				   ms->message_handle,
				   ms->sessionNumber + 1,
				   IsMultipartMessage(&intStrHeaderList),
				   Sdm_False);

    XmListReplaceItemsPos(_w, &complete_header, 1, _displayed_item_position);

    // The default selection policy is extended_select.
    // Problem:
    // User extend selects multiple items
    // The last item selected is a "N" message
    // We display the "N" message and have to replace it without
    // the "N".
    // There is no way to replace with something else and select at 
    // the same time.  
    // Therefre, we need to explicitly select the replacement.
    // The problem is: selecting the replacement deselects the other
    // selected items.
    // To work around this, we switch temporarily to MULTIPLE_SELECT,
    // select the replacement, and switch back to extend_select.
    // This will select the replacement and *not*  drop the
    // selection on the other selected items.
    // Say "Amen" to  OSF for such convoluted thinking!

    XtVaGetValues(_w,
		  XmNselectedItemCount, &num_selected,
		  NULL);

    if (num_selected > 1) {
      // Change to MULTIPLE_SELECT.
      // Select item
      // Change back to EXTEND_SELECT
      XtVaSetValues (_w,
		     XmNselectionPolicy, XmMULTIPLE_SELECT,
		     NULL);

      // When loading mail headers, we need to make sure that
      // the XmList resources are all set at the same time to
      // avoid painting multiple times.  So here we collect
      // these resources.
      if (_xtarg_collector && _xmstr_collector) {
        // free memory for _selected_items if needed.
        if (_selected_items) {
          XmStringFree (_selected_items);
          _selected_items = NULL;
        }

        // Keep a handle to the malloced string copy so that
        // we can free it after the XtSetValues
        _selected_items = complete_header;
        complete_header = NULL;    // reset so it doesn't get destroyed below.

        _xtarg_collector->AddItemToList (XmNselectedItems,
                 (XtArgVal) &_selected_items);
        _xtarg_collector->AddItemToList (XmNselectedItemCount, 1);
        _xtarg_collector->AddItemToList (XmNselectionPolicy, 
                 XmEXTENDED_SELECT);
      }
      else {
        XmListSelectPos(_w, _displayed_item_position, Sdm_False);

        XtVaSetValues (_w,
                 XmNselectionPolicy, XmEXTENDED_SELECT,
                 NULL);
      }
    }
    else {
      if (_xtarg_collector && _xmstr_collector) {
        // free memory for _selected_items if needed.
        if (_selected_items) {
          XmStringFree (_selected_items);
          _selected_items = NULL;
        }

        _selected_items = complete_header;
        complete_header = NULL;    // reset so it doesn't get destroyed below.

        _xtarg_collector->AddItemToList (XmNselectedItems,
                 (XtArgVal) &_selected_items);
        _xtarg_collector->AddItemToList (
                 XmNselectedItemCount, 1);
      }
      else {
        XmListSelectPos(_w, _displayed_item_position, Sdm_False);
      }
    }
    
    // free memory used for complete_header.  do this only if i
    // it is not cached in _selected_items.
    if (complete_header != NULL) {
      XmStringFree(complete_header);
    }
    
    _num_new_messages--;
    // Do auto-updating of unread stat in Views window if it has
    // been created.
    Views *vp = parent()->get_views_dialog(Sdm_False, Sdm_False);
    if (vp && vp->isMapped()) {
      SdmMessageNumberL emptyList;
      vp->updateViewsWindow(mail_error,
			    Sdm_True,		// regenerate
			    Sdm_True,		// unread only
			    SingleMessage,	// type
			    msg_num, 0,		// start, end
			    emptyList,		// msg list
			    Sdm_False);		// add to existing count
    }
    
    if (mail_error) {
      genDialog->post_error(mail_error, DTMAILHELPCANNOTUPDATEVIEWSTATS,
			    catgets(DT_catd, 17, 24, "Mailer cannot update the message statistics for your views."));
      // don't return.  continue processing.  reset error.
      mail_error.Reset();
    }

    msg->SetFlags(mail_error, Sdm_MFA_Seen);
    if (mail_error) {
      parent()->normalCursor();
      return;
    }
  }

  display_message_summary();

  rmw_editor = parent()->get_editor()->textEditor();

  rmw_editor->disable_redisplay();
  rmw_editor->auto_show_cursor_off();
  rmw_editor->clear_contents();

  _parent->delete_message(_displayed_msgno);
  _displayed_msgno = msg_num;

  char * status_string = NULL;
  SdmBoolean firstBPHandled;

  firstBPHandled = parent()->get_editor()->textEditor()->set_message(
							     mail_error,
							     msg,
							     &status_string,
							     parent()->fullHeader() ?
							     Editor::HF_FULL :
							     Editor::HF_ABBREV );

  if (status_string) {
    parent()->message(status_string);
  }
  
  int num_attachments = 0;

  // If the message has attachments, then let the attach pane
  // handle attachments but not the first bodyPart (which has
  // already been handled here).
  if (firstBPHandled) {

    //  The first bodyPart has already been handled.
    // The others, beginning from the second, need to be parsed 
    // and put into the attachPane.

    parent()->get_editor()->attachArea()->parseAttachments(
                 mail_error,
                 msg, 
                 Sdm_True,
                 2, num_attachments);
  }
  else {
    // The first bodyPart was not handled.
    // It may not have been of type text.
    // The attachment pane needs to handle all the bodyParts
    // beginning with the first.

    parent()->get_editor()->attachArea()->parseAttachments(
                 mail_error,
                 msg, 
                 Sdm_True,
                 1, num_attachments);
  }


  if(mail_error) {
    parent()->normalCursor();
    return;
  }
  
  if ( num_attachments > 0 ) {
    parent()->get_editor()->manageAttachArea();
    parent()->activate_default_attach_menu();
  } else {
    parent()->deactivate_default_attach_menu();
    parent()->get_editor()->unmanageAttachArea();
  }
  
  rmw_editor->auto_show_cursor_restore();
  // Turn on text editor and manage attachPane

  rmw_editor->set_to_top();
  rmw_editor->enable_redisplay();

  parent()->normalCursor();
}

void
MsgScrollingList::display_and_select_message(SdmError &mail_error,
					     SdmMessageNumber msg_num)
{
  mail_error.Reset();

  // Need to calculate what is the position of that item
  // in the scrolling list, given the DtMailMessageHandle.
  // Determine the index at the _msgs array; increment by 1
  // since array begins at 0 and XmList begins at 1.

  _displayed_item_position = _msgs->index(msg_num) + 1;
  // check for error condition
  if (_displayed_item_position < 1) {
      display_message_summary();
      return;
  }

  _selected_item_position = _displayed_item_position;
    
  // Select this message in the scrolling list and display
  // the message.

  // When loading mail headers, we need to make sure that
  // the XmList resources are all set at the same time to
  // avoid painting multiple times.  So here we collect
  // these resources.
  if (_xtarg_collector && _xmstr_collector) {
    XmString *items = _xmstr_collector->GetItems();

    // free memory for _selected_items if needed.
    if (_selected_items) {
        XmStringFree (_selected_items);
        _selected_items = NULL;
    }

    // Keep a handle to the malloced string copy so that
    // we can free it after the XtSetValues
    _selected_items = XmStringCopy (items[_displayed_item_position - 1]);

    _xtarg_collector->AddItemToList (XmNselectedItems,
				     (XtArgVal) &_selected_items);
    _xtarg_collector->AddItemToList (XmNselectedItemCount, 1);
  } else {
    XmListSelectPos(_w, _displayed_item_position, Sdm_False);
  }

  this->display_message(mail_error, msg_num, _displayed_item_position);

  return;
}

void
MsgScrollingList::select_all_and_display_last(SdmError &error)
{
  register int	item_pos;
  DtMailGenDialog *genDialog = theRoamApp.genDialog();
  int				num_items;
  MsgHndArray		*msgHandles = get_messages();
 
  error.Reset();

  if (this->get_num_messages() == 0) return;

  XtVaSetValues (_w, XmNselectionPolicy, XmMULTIPLE_SELECT, NULL);

  //
  // We have to go to the end of the list and go backwards.
  // We display the last one, and select the rest.
  //
  // A NULL terminated list.
  //
 
  XmListDeselectAllItems(baseWidget());
  XtVaGetValues(baseWidget(), XmNitemCount, &num_items, NULL);
 
  for (item_pos = 1; item_pos < num_items; item_pos++) {
    XmListSelectPos(baseWidget(), item_pos, Sdm_False);
  }
 
  // Invoke the selection callback on the last item.
  display_and_select_message(error, 
			     msgHandles->at(item_pos-1)->message_handle);
			     
  XtVaSetValues (_w, XmNselectionPolicy, XmEXTENDED_SELECT, NULL);
  
  if (error) { 
    genDialog->post_error(error, DTMAILHELPCANNOTDISPLAYMSG, 
			  catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
  }
}

void
MsgScrollingList::select_all_and_display_last(
					      SdmError 	&error,
					      SdmMessageNumberL *handleArray,
					      unsigned int	   elements
					      )
{
  register int		handleOffset = 0;
  register int		item_pos;
  DtMailGenDialog *genDialog = theRoamApp.genDialog();

  error.Reset();

  XtVaSetValues (_w, XmNselectionPolicy, XmMULTIPLE_SELECT, NULL);
  //
  // We have to go to the end of the list and go backwards.
  // We display the last one, and select the rest.
  //
  // A NULL terminated list.
  //
  handleOffset = elements;

  XmListDeselectAllItems(baseWidget());

  while (--handleOffset >= 0 && (error == Sdm_EC_Success)) {

    item_pos = _msgs->index((*handleArray)[handleOffset]);	// Get position
    if (item_pos < 0) {
      continue;
    }
    
    //
    // Select this message in the scrolling list and IF
    // it is the last message, display it.
    //
    if (handleOffset == elements - 1) {
      display_and_select_message(error, (*handleArray)[handleOffset]);
      if (error) { 
	genDialog->post_error(error, DTMAILHELPCANNOTDISPLAYMSG, 
			      catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
      }
    } else {
      XmListSelectPos(_w, item_pos + 1, Sdm_False);
    }
  }
  XtVaSetValues (_w, XmNselectionPolicy, XmEXTENDED_SELECT, NULL);
  return;
}

void
MsgScrollingList::display_no_message()
{

  /* NL_COMMENT
   * No mail message has been selected by the user.
   */

  parent()->message(catgets(DT_catd, 2, 16, "No message selected."));
  _displayed_item_position = 0;
  _selected_item_position = 0;

  parent()->get_editor()->textEditor()->clear_contents();
  parent()->get_editor()->attachArea()->clearAttachArea();
  parent()->get_editor()->attachArea()->attachment_summary(0, 0);

  _parent->delete_message(_displayed_msgno);
  _displayed_msgno = 0;
  
}

// In need of a simple but useful optimization here.
// By the time we get to this method, we already have displayed
// the selected message in the lower section of the combo window.
// Instead of having to parse the messageHandle and construct the
// text buffer to display, we can just get the parsed-and-formatted
// text from the lower section of the combo window and stick it into
// the VMD's editor.
// This will save us the effort of parsing and formatting a message
// 

void 
MsgScrollingList::viewInSeparateWindow(SdmError &mail_error)
{
  FORCE_SEGV_DECL(const char, title);
  FORCE_SEGV_DECL(char, header_txt);
  FORCE_SEGV_DECL(MsgStruct, tmpMS);
  SdmMessageNumber msgHandle;
  SdmMessageStore	*mbox=parent()->mailbox();
  ViewMsgDialog *newview;
  Editor*		vmd_editor;
  SdmIntStrLL		intStrHeaderList;
  SdmIntStrL    *intStrList;
  SdmMessageNumberL msgNumList;
  
  mail_error.Reset();

  // If no message selected, return.

  if (this->get_selected_item() <= 0) return;

  // get the list of all messages to be view.
  selected_msg_hndls(msgNumList);
    
  // Double-check.  If none selected, return
  int len = msgNumList.ElementCount();
  if (len == 0) return;

  // get the subject headers for all the messages at once.
  Boolean noSubjectHeadersFound = Sdm_False;
  mbox->GetHeaders(mail_error, intStrHeaderList, Sdm_MHA_Subject, msgNumList);
  if (mail_error == Sdm_EC_RequestedDataNotFound) {
    noSubjectHeadersFound = Sdm_True;
    mail_error = Sdm_EC_Success;
  } else if (mail_error != Sdm_EC_Success) {
    intStrHeaderList.ClearAndDestroyAllElements();
    return;
  }

  for (int i = 0; i < len; i++) {
    msgHandle = msgNumList[i];
    newview = parent()->ifViewExists(msgHandle);
	   
    if (newview != NULL) {

      /* NL_COMMENT
       * The current mail message selected is already displayed in a separate
       * window.  Therefore this 'separate' window will be raised in front
       * of existing windows so the user can see it.
       */
      parent()->message(catgets(DT_catd, 3, 69, "View already exists.  Raising it."));
      newview->raise();
      continue;
    } 
    // No view exists.  Display it.  For feedback, set busyCursor().
	       
    parent()->busyCursor();
    newview = new ViewMsgDialog(parent());
    parent()->registerDialog( newview );
    newview->initialize();
    vmd_editor = newview->get_editor()->textEditor();

    // Set the VMD's msgHandle. This unique handle is what is
    // used to raise the VMD when the same message is double
    // clicked on later.
    newview->msgno(msgHandle);

    SdmMessage * msg;
    mbox->SdmMessageFactory(mail_error, msg, msgHandle);
    if (mail_error) {
      intStrHeaderList.ClearAndDestroyAllElements();
      newview->quit();
      parent()->normalCursor();
      return;
    }
      
    SdmString subj;
    if (noSubjectHeadersFound) {
      subj = catgets(DT_catd, 1, 322, "NO SUBJECT!");

    } 
    else {
      intStrList = intStrHeaderList[i];
      if ( intStrList && intStrList->ElementCount() > 0 && 
           (*intStrList)[0].GetString().Length() > 0 ) 
      {
        // I18N Decode the header and concatenate the segments
        // that are returned, then append them to the line.
        SdmString charset;
        msg->GetCharacterSet(mail_error, charset);
        if (mail_error) {
          intStrHeaderList.ClearAndDestroyAllElements();
          newview->quit();
          parent()->normalCursor();
          return;
        }

        SdmMessageEnvelope::DecodeHeader (mail_error, subj, 
                  (const char*)(*intStrList)[0].GetString(), charset);
        if (mail_error) {
          intStrHeaderList.ClearAndDestroyAllElements();
          newview->quit();
          parent()->normalCursor();
          return;
        }
      }
      else {
        subj = catgets(DT_catd, 1, 322, "NO SUBJECT!");
      }
    }

    newview->title(subj);
    newview->auto_show_cursor_off();
    vmd_editor->disable_redisplay();
    
    char * status_string = NULL;
    SdmBoolean firstBPHandled;
    int num_attachments = 0;

    if (parent()->fullHeader()) {
      firstBPHandled = newview->get_editor()->textEditor()->set_message(
									mail_error,
									msg, 
									&status_string,
									Editor::HF_FULL);
    }
    else{
      firstBPHandled = newview->get_editor()->textEditor()->set_message(
									mail_error,
									msg, 
									&status_string,
									Editor::HF_ABBREV);
    }
	   
    if (mail_error) {
      intStrHeaderList.ClearAndDestroyAllElements();
      newview->quit();
      parent()->normalCursor();
      return;
    }
    if (firstBPHandled) {
      //  The first bodyPart has already been handled.
      // The others, beginning from the second, need to be parsed 
      // and put into the attachPane.
      newview->get_editor()->attachArea()->parseAttachments(
                        mail_error,
                        msg, 
                        Sdm_True,
                        2, num_attachments);
      
    } else {
      // The first bodyPart was not handled.
      // It may not have been of type text.
      // The attachment pane needs to handle all the bodyParts
      // beginning with the first.
      newview->get_editor()->attachArea()->parseAttachments(
                        mail_error,
                        msg, 
                        Sdm_True,
                        1, num_attachments);
    }

    if (mail_error) {
      intStrHeaderList.ClearAndDestroyAllElements();
      newview->quit();
      parent()->normalCursor();
      return;
    }
    if (num_attachments > 0) {
      newview->get_editor()->manageAttachArea();
      newview->activate_default_attach_menu();
    } else {
      newview->get_editor()->unmanageAttachArea();
      newview->deactivate_default_attach_menu();
    }
    
    newview->set_to_top();
    newview->auto_show_cursor_restore();
    vmd_editor->enable_redisplay();
    newview->manage();
  }

  parent()->normalCursor();
  intStrHeaderList.ClearAndDestroyAllElements();
}

// defaultAction() gets called *after* extendedSelectionCallback() is
// called.  
// Display the message in extendedSelectionCallback().
// viewInSeparateWindow() in defaultAction().

void 
MsgScrollingList::defaultAction(
				Widget ,	      // w
				XtPointer ,	     // clientData
				XmListCallbackStruct *cbs
				)
{
//   FORCE_SEGV_DECL(MsgStruct, tmpMS);
  const MsgStruct *tmpMS;
  SdmError mail_error;

  // Initialize the mail_error.
  mail_error.Reset();

  _selected_item_position = cbs->item_position;

  tmpMS = get_message_struct(_selected_item_position);
  if (tmpMS == NULL) {
    return;
  }
  else {
    this->viewInSeparateWindow(mail_error);
  }
  if (mail_error) {
    parent()->postErrorDialog(mail_error);
  }
}

void
MsgScrollingList::extendedSelectionCallback(
					    Widget ,			// w
					    XtPointer clientData,
					    XmListCallbackStruct *cbs
					    )
{
  int last_clicked_on_pos;
  int above_selected_pos;
  int tmp_selected_pos;
  int i, num_selected;
  SdmError mail_error;

  // Initialize the mail_error.
  mail_error.Reset();

  SdmBoolean isSelection = Sdm_False;
  SdmBoolean selectionMade = Sdm_False;

  MsgScrollingList *obj=(MsgScrollingList *) clientData;

  // If all items have been deselected
  if (cbs->selected_item_count == 0) {
    obj->extended_selection(mail_error, 0);
    if (mail_error) {
      return;
    }
    selectionMade = Sdm_True;
  }
  else {
    if (cbs->selected_item_count > 1)
          obj->_parent->deactivate_multi_select_menu();
    else
	  obj->_parent->activate_multi_select_menu();

    last_clicked_on_pos = cbs->item_position;

    // Check to see if this was a selection or deselection
    // We do that by seeing if last_clicked_on_pos is in the 
    // selected_item_positions array.  If it is, then it is a 
    // selection; if its not there, then its a deselection.

    num_selected =  cbs->selected_item_count;
    
    for (i = 0; i < num_selected; i++) {
      if (last_clicked_on_pos == cbs->selected_item_positions[i]) {
        // Yes, it was a selection

        isSelection = Sdm_True;
        obj->extended_selection(mail_error, last_clicked_on_pos);
        if (mail_error) {
          return;
        }

        selectionMade = Sdm_True;
      }
    }

    // If it was not a selection, then we need to find the selected
    // item nearest to the deselected item.  We have a choice there -
    // we can find the nearest one above, or the nearest one below 
    // the deselected item.  Let's find the nearest one above.
    // We do that by cruising through the selected_item_positions array.
    // Motif arranges the array in ascending order, no matter what order
    // you selected the items!  If you deselect an item, the items on
    // either side of it are the ones above and below it.  We use that
    // ordering to now select and display the one above it.
    //

    if (!isSelection) {

      // If the first selected item is below the deselected item,
      // it follows that all selected items are below the deselected
      // item.  Display the first selected item and be done.

      if (cbs->selected_item_positions[0] > last_clicked_on_pos) {
        obj->extended_selection(mail_error,
              cbs->selected_item_positions[0]);
        if (mail_error) {
          return;
        }
        selectionMade = Sdm_True;
      } 
      // If the last selected item is above the deselected item,
      // it follows that all selected items are above the deselected
      // item.  Display the last selected item and be done.
      else if (cbs->selected_item_positions[num_selected - 1] < last_clicked_on_pos) 
      {
        obj->extended_selection(mail_error,
              cbs->selected_item_positions[num_selected - 1]);
        if (mail_error) {
          return;
        }
        selectionMade = Sdm_True;
      }
      // Otherwise, the deselected item must lie in between other
      // selected items.  We choose to find the closest selected
      // item above the deselected item.
      else {
        // There are selected items that are above the deselected 
        // item.
        // Need to find the one that is both above the deselected 
        // item and closest to it.
        // Iterate until you find the nearest above; select it and
        // drop out of loop when after selection.

        for (i = 0; i < (num_selected - 1) && !selectionMade; i++) {
          above_selected_pos = cbs->selected_item_positions[i];
          tmp_selected_pos = cbs->selected_item_positions[i + 1];
          if (tmp_selected_pos > last_clicked_on_pos) {
            obj->extended_selection(mail_error,
                  above_selected_pos);
            if (mail_error) {
              return;
            }
            selectionMade = Sdm_True;
          }
        }
      }
    }
  }
}

void
MsgScrollingList::extended_selection(
				     SdmError &mail_error,
				     int position 
				     )
{
  const MsgStruct *tmpMS;
  DtMailGenDialog *genDialog = theRoamApp.genDialog();

  // Disable the SaveAttachments menu item first.  
  // It gets enabled when an attachment is selected.

  _parent->all_attachments_deselected();

  if (position == 0) {  // all items deselected.
    this->display_no_message();
    _selection_on = Sdm_False;
    _parent->deactivate_default_message_menu();
    return;
  }

  // if there were no selected item(s) before and now we
  // have one/some, we need to turn on the message menu at
  // the parent level.

  if (!_selection_on) {
    _selection_on = Sdm_True;
    _parent->activate_default_message_menu();
  }
   
  // If selected message is the displayed message, return.
  // No need to redisplay the same message.

  if (position == _selected_item_position) return;

  // Retrieve message...
  // Display the selected message...
  _selected_item_position = position;

  tmpMS = get_message_struct(_selected_item_position);
  if (tmpMS == NULL) {
    return;
  }
  else {
    this->display_message(mail_error, tmpMS->message_handle);
    if (mail_error) { 
      genDialog->post_error(mail_error, DTMAILHELPCANNOTDISPLAYMSG, 
			    catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
    }
  }
}

MsgHndArray*
MsgScrollingList::get_deleted_messages()
{
  MsgHndArray *del_msgs = new MsgHndArray(_deleted_messages->length());

  for (int i=0; i<_deleted_messages->length(); i++) {
    MsgStruct *tmpMS = new MsgStruct();

    tmpMS->message_handle = _deleted_messages->at(i)->message_handle;
    tmpMS->sessionNumber = _deleted_messages->at(i)->sessionNumber;
    tmpMS->is_deleted = Sdm_False;
    del_msgs->append(tmpMS);
  }
  return(del_msgs);
}

MsgHndArray*
MsgScrollingList::get_messages()
{
  return (_msgs);

//   MsgHndArray *msgs = new MsgHndArray(_msgs->length());

//   for (int i=0; i<_msgs->length(); i++) {
//     MsgStruct *tmpMS = new MsgStruct();

//     tmpMS->message_handle = _msgs->at(i)->message_handle;
//     tmpMS->sessionNumber = _msgs->at(i)->sessionNumber;
//     tmpMS->is_deleted = Sdm_False;
//     msgs->append(tmpMS);
//   }
//   return(msgs);
}

int
MsgScrollingList::get_selected_item()
{
  return(_selected_item_position);
}

int
MsgScrollingList::get_displayed_item()
{
  return(_displayed_item_position);
}

void
MsgScrollingList::scroll_to_bottom()
{
  XmListSetBottomPos( this->baseWidget() , 0 );
}

// Scroll the list so that the item at position is in the
// middle of the scrolling list.  If the number of items
// that the scrolling list can display at one time is greater
// than the total number of items, then display them all.
// If the item at position is close to the bottom of the list
// make sure we make as many items visible as possible.
//
// If the item at position is already visible, then don't
// do anything.

void
MsgScrollingList::scroll_to_position(
				     int position
				     )
{
  int top, visible, total;
  int top_pos;

  // Determine the position of the header that we want to select.
  // If the XtArgCollector exists, then add
  // the resources to the XtArgCollector so that we can prevent
  // multiple redisplays in the XmList widget.
  if (_xmstr_collector)
    {
      XtVaGetValues( _w,
		     XmNtopItemPosition, &top,
		     XmNvisibleItemCount, &visible,
		     NULL );
      total = _xmstr_collector->GetNumItems();
    }
  else
    {
      XtVaGetValues( _w,
		     XmNtopItemPosition, &top,
		     XmNvisibleItemCount, &visible,
		     XmNitemCount, &total,
		     NULL );
    }

  if (( position < top ) || ( position >= top+visible )) {

    if ((total <= visible) || (position <= visible/2)) {
      // If we can display them all, make the first item appear
      // at the top of the list.
      top_pos = 1;
    } else if (position > (total-(visible/2))) {
      top_pos = total - visible + 1;
    } else {
      top_pos = position - visible/2 + 1;
    }

    // Determine the position of the header that we want to select.
    // If the XtArgCollector exists, then add
    // the resources to the XtArgCollector so that we can prevent
    // multiple redisplays in the XmList widget.
    if (_xtarg_collector)
      _xtarg_collector->AddItemToList (
				       XmNtopItemPosition, top_pos);
    else
      XmListSetPos (_w, top_pos);
  }
}

void
MsgScrollingList::undelete_messages(SdmError &mail_error, SdmMessageNumberL &msgNumberList)
{
  FORCE_SEGV_DECL(MsgStruct, tmpMS);
  DtMailGenDialog *genDialog = theRoamApp.genDialog();
  int  i, num_entries, entry_position, del_pos;
  SdmMessageStore *mbox=parent()->mailbox();
  SdmIntStrLL intStrHeaderList;
  SdmIntStrL  *intStrListPtr;
  SdmMessageFlagAbstractFlagsL flagsList;
  XmString complete_header;	// read status + glyph + header_text.

  // Initialize the mail_error.
  mail_error.Reset();

  num_entries = msgNumberList.ElementCount();

  if (num_entries == 0)
    return;
  
  // get the headers for the messages we are deleting.
  mbox->GetHeaders(mail_error, intStrHeaderList, _header_info, msgNumberList);
  
  // if we didn't get any headers for any of the messages (this should be 
  // pretty rare but we need to handle it anyway), create empty lists for each
  // message and continue processing below.
  //
  if (mail_error == Sdm_EC_RequestedDataNotFound) {
    for (i = 0; i < num_entries; i++) {
      intStrHeaderList.AddElementToList(new SdmIntStrL);
    }
    mail_error = Sdm_EC_Success;  // reset error.
  }

  if (mail_error) {
    intStrHeaderList.ClearAndDestroyAllElements();
    return;
  }

  // get the flags for the deleted message.  we are interested in whether 
  // the messages were "seen".
  mbox->GetFlags(mail_error, flagsList, msgNumberList);
  if (mail_error) {
    return;
  }

  // Reset the flag of the message in message store so that the  
  // message will not be expunged when the folder is quit.
  mbox->ClearFlags(mail_error, Sdm_MFA_Deleted, msgNumberList);
  if (mail_error) {
    intStrHeaderList.ClearAndDestroyAllElements();
    return;
  }

  for (i = 0; i < num_entries; i++) {
    int indx = _deleted_messages->index(msgNumberList[i]);
    tmpMS = _deleted_messages->at(indx);
    tmpMS->is_deleted = Sdm_False;

    // Remove chosen item from list of deleted messages;
    // insert it back into _msgs at the right place (which is 
    // determined by session_number of retrieved MsgStruct).
    // Insert back into scrolling list for visual display
    // at the position session_number.
    entry_position = _msgs->insert(tmpMS);

    // If the load_headersWP routine hasn't finished loading all of the
    // msg headers into the MSL, we must add these msgs to _msgNumL and
    // adjust the first and last msg index variables.
    if (_msgNumL != NULL) {
	_msgNumL->InsertElementBefore(tmpMS->message_handle, entry_position);
	if (entry_position <= _last_msg_index_loaded) {
	    _last_msg_index_loaded++;
	    if (entry_position <= _first_msg_index_loaded) {
		_first_msg_index_loaded++;
	    }
	}
    }

    // Increment by one, because the index into the scrolling
    // list is always one greater than the index into the
    // message handle array that we got the index from.
    entry_position = entry_position + 1;

    // Maintain the assumption that the item at entry_position
    // is the selected item
    _selected_item_position = entry_position;

    intStrListPtr = intStrHeaderList[i];

    complete_header = formatHeader(intStrListPtr,
				   tmpMS->message_handle,
				   tmpMS->sessionNumber + 1,
				   IsMultipartMessage(intStrListPtr),
				   isSet(flagsList[i], Sdm_MFA_Seen)  ? Sdm_False : Sdm_True);

    if ( !isSet(flagsList[i], Sdm_MFA_Seen) ) 
      _num_new_messages++;

    XmListAddItemUnselected(_w, complete_header, entry_position);
    XmStringFree(complete_header);

    // Get position of undeleted message structure in _deleted_message
    // and remove the entry from _deleted_messages
    del_pos = _deleted_messages->index(tmpMS);
    // Remove the entry from the deleted message list.  We do not free the
    // associated MsgStruct because it is still needed in the _msgs list.
    _deleted_messages->remove_entry(del_pos);
  }

  // Deselect all items currently selected.
  // display_and_select_message() will select, highlight
  // and display the last "undeleted" message.
    
  XmListDeselectAllItems(_w);

  // Do auto-updating of Views window if it has been created.
  Views *vp = parent()->get_views_dialog(Sdm_False, Sdm_False);
  if (vp && vp->isMapped()) {
    vp->updateViewsWindow(
        mail_error,
        Sdm_True,	// regenerate
			  Sdm_False,	// unread_only
			  List,		// type
			  0, 0,
			  msgNumberList, // msg list
			  Sdm_True);	// add to existing count
    
    if (mail_error) {
      genDialog->post_error(mail_error, DTMAILHELPCANNOTUPDATEVIEWSTATS,
        catgets(DT_catd, 17, 24, "Mailer cannot update the message statistics for your views."));
      // don't return.  continue processing.  reset error.
      mail_error.Reset();
    }
  }
			    
  // Display this message and select it.
  this->display_and_select_message(mail_error, 
				   tmpMS->message_handle);
  if (mail_error) { 
    genDialog->post_error(mail_error, DTMAILHELPCANNOTDISPLAYMSG,
			  catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
    // don't return.  continue below.  reset error.
    mail_error.Reset();
  }

  _num_deleted_messages -= num_entries;
  _total_deleted -= num_entries;

  // Need to decrement _num_deleted_messages if the undeleted messages
  // meet the view criteria.

  if (mail_error) {
    intStrHeaderList.ClearAndDestroyAllElements();
    return;
  }

  parent()->message_summary(_selected_item_position,
			    _session_message_number,
			    _num_new_messages,
			    _num_deleted_messages);
			    
  intStrHeaderList.ClearAndDestroyAllElements();
}

void
MsgScrollingList::undelete_last_deleted(SdmError &mail_error)
{
  FORCE_SEGV_DECL(MsgStruct, tmpMS);
  DtMailGenDialog *genDialog = theRoamApp.genDialog();
  int entry_position;
  int len;
  UndelFromListDialog *undel_dialog;
  SdmMessageStore *mbox=parent()->mailbox();
  SdmIntStrL intStrHeaderList;
  SdmMessageFlagAbstractFlags flagsList;
  XmString complete_header;	// read status + glyph + header_text.


  if (_num_deleted_messages == 0) return;

  // Initialize the mail_error.
  mail_error.Reset();

  // Delete the message from the Deleted Messages Dialog.
  undel_dialog = parent()->get_undel_dialog();
  if (undel_dialog)
    undel_dialog->undelLast();
    
  // Restore the message in RoamMenuWindow:MessageScrollingList.
  len = _deleted_messages->length();

  tmpMS = _deleted_messages->at(len - 1);
  tmpMS->is_deleted = Sdm_False;

  // get the headers for the deleted message. 
  mbox->GetHeaders(mail_error, intStrHeaderList, _header_info, tmpMS->message_handle);
  
  // if we didn't get any headers for the messages reset the
  // error and continue processing below.  the message scrolling
  // list will end up showing empty fields for this message.
  //
  if (mail_error == Sdm_EC_RequestedDataNotFound) {
    mail_error = Sdm_EC_Success;  // reset error.
  } 
  if (mail_error) {
    return;
  }

  // get the flags for the deleted message.  we need the "seen" flag below.
  mbox->GetFlags(mail_error, flagsList, tmpMS->message_handle);
  if (mail_error) {
    return;
  }

  // reset the deleted flag for the deleted message.
  mbox->ClearFlags(mail_error, Sdm_MFA_Deleted, tmpMS->message_handle);
  if (mail_error) {
    return;
  }

  // Remove chosen item from list of deleted messages;
  // insert it back into _msgs at the right place (which is 
  // determined by session_number of retrieved MsgStruct).
  // Insert back into scrolling list for visual display
  // at the position session_number.

  entry_position = _msgs->insert(tmpMS);

  // If the load_headersWP routine hasn't finished loading all of the
  // msg headers into the MSL, we must add these msgs to _msgNumL and
  // adjust the first and last msg index variables.
  if (_msgNumL != NULL) {
      _msgNumL->InsertElementBefore(tmpMS->message_handle, entry_position);
      if (entry_position <= _last_msg_index_loaded) {
	  _last_msg_index_loaded++;
	  if (entry_position <= _first_msg_index_loaded) {
	      _first_msg_index_loaded++;
	  }
      }
  }

  // Increment by one, because the index into the scrolling
  // list is always greater than the index into the
  // message handle array that we got the index from.

  entry_position = entry_position + 1;

  complete_header = formatHeader(&intStrHeaderList,
				 tmpMS->message_handle,
				 tmpMS->sessionNumber + 1,
				 IsMultipartMessage(&intStrHeaderList),
				 Sdm_False);

  // Since this msg is going to be selected, it will no longer be "new".
  // Therefore, we should just set the flag now.
  if ( !isSet(flagsList, Sdm_MFA_Seen) ) {
      mbox->SetFlags(mail_error, Sdm_MFA_Seen, tmpMS->message_handle);
  }

  _deleted_messages->remove_entry(len - 1);
  _num_deleted_messages--;
  _total_deleted--;

  XmListAddItem(_w, complete_header, entry_position);
  XmStringFree(complete_header);
    
  // Deselect all items currently selected.
  // display_and_select_message() will select, highlight
  // and display the last "undeleted" message.
  XmListDeselectAllItems(_w);

  _selected_item_position = entry_position;
  this->display_and_select_message(mail_error, tmpMS->message_handle);
  if (mail_error) { 
    genDialog->post_error(mail_error, DTMAILHELPCANNOTDISPLAYMSG,
			  catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
    // don't return.  continue below.  reset error.
    mail_error.Reset();
  }

  parent()->message_summary(_selected_item_position,
			    _session_message_number, 
			    _num_new_messages,
			    _num_deleted_messages);

  // Do auto-updating of Views window if it has been created.
  Views *vp = parent()->get_views_dialog(Sdm_False, Sdm_False);
  if (vp && vp->isMapped()) {
    SdmMessageNumberL emptyList;
    vp->updateViewsWindow(mail_error,
			  Sdm_True,			// regenerate
			  Sdm_False,			// unread_only
			  SingleMessage,		// range
			  tmpMS->message_handle, 0,	// undelete last
			  emptyList,
			  Sdm_True);
			  
    if (mail_error) {
      genDialog->post_error(mail_error, DTMAILHELPCANNOTUPDATEVIEWSTATS,
			    catgets(DT_catd, 17, 24, "Mailer cannot update the message statistics for your views."));
      // don't return.  continue processing.  reset error.
      mail_error.Reset();
    }
  }			    
}

void
MsgScrollingList::checkDisplayProp(void)
{
  SdmMessageStore * mbox;
  SdmMailRc * mailrc;
  SdmError mail_error;
  SdmBoolean state_changed = Sdm_False;
  DtMailGenDialog *genDialog = theRoamApp.genDialog();

  mbox = parent()->mailbox();
  // need to get mailrc from the connection object.
  theRoamApp.connection()->connection()->SdmMailRcFactory(mail_error, mailrc);

  if (mailrc->IsValueDefined("showto") != _showTo) {
    _showTo = _showTo ? Sdm_False : Sdm_True;    // toggle the value.
    state_changed = Sdm_True;
    if (_showTo)
    	_header_info = Sdm_MHA_P_MessageFrom |
        	Sdm_MHA_P_ReceivedDate |
        	Sdm_MHA_P_Subject |
        	Sdm_MHA_P_MessageSize |
        	Sdm_MHA_P_MessageType |
        	Sdm_MHA_P_SenderPersonalInfo |
		Sdm_MHA_P_SenderUsername;
    else
    	_header_info = Sdm_MHA_P_MessageFrom |
        	Sdm_MHA_P_ReceivedDate |
        	Sdm_MHA_P_Subject |
        	Sdm_MHA_P_MessageSize |
        	Sdm_MHA_P_MessageType |
        	Sdm_MHA_P_SenderPersonalInfo;
  } 
 
  if (mailrc->IsValueDefined("showmsgnum") != _numbered) {
    _numbered = _numbered ? Sdm_False : Sdm_True;    // toggle the value.
    state_changed = Sdm_True;
  }

  // we check the alternates only if the showto option is set.
  if (_showTo) {
    SdmError localError;
    // if the useralternates option changed, we want to always update
    // our alternates list to add or remove the alternates.
    if (mailrc->IsValueDefined("usealternates") != _useAlternates) {
      _useAlternates = _useAlternates ? Sdm_False : Sdm_True;    // toggle the value.
      if (_alternates) 
        free (_alternates);
      _alternates = mailrc->GetAlternates(localError);
      GetUsernames();
      state_changed = Sdm_True;
      
    // if the usealternates option did not change and we are currently
    // using alternates, check the list of alternates and update our
    // cached list only if the alternates list has changed.
    } else if (_useAlternates) {
      char *newAlternates = mailrc->GetAlternates(localError);
      if ((newAlternates == NULL && _alternates == NULL) || 
           (newAlternates && _alternates && strcmp(newAlternates, _alternates) == 0)) 
      {
    	if (_usernames.ElementCount() == 0) {
            GetUsernames();
            state_changed = Sdm_True;
        }
        free (newAlternates);
      } else {
	if (_alternates)
	    free (_alternates);
        _alternates = newAlternates;
        GetUsernames();
        state_changed = Sdm_True;
      }

    // if we are not using the alternates, we should have at least
    // one name in the alternates list, the login.  We the list
    // is empty, that means that we need to add the login name to
    // the alternates list.  The call to GetUsernames
    // will add the login name to the alternates list.
    } else if (_usernames.ElementCount() == 0) {
      GetUsernames();
      state_changed = Sdm_True;
    }
  }

  // if the state changed, then update the message scrolling list and
  // the deleted list (if it exists).
  if (state_changed) {
    SdmError error;
    updateListItems(error, -1);

    if (error) {
      genDialog->post_error(error, DTMAILHELPCANNOTUPDATEMSGHDRLIST,
			    catgets(DT_catd, 3, 133, "Mailer cannot update the Message Header List."));
    }
 
    UndelFromListDialog * del_dialog = _parent->get_undel_dialog();
    if (del_dialog) {
      // calling checkDiaplayProp for the dialog below will 
      // cause the delete scrolling list to check the properties
      // and update it's list accordingly.
      del_dialog->checkDisplayProp();
    }
  }
}

//
// Update the scrolling list. Current is the index of the message
// to position the scrolling list to.  -1 to keep it as is.
//
void
MsgScrollingList::updateListItems (SdmError &error, int current)
{
  SdmMessageStore * mbox = NULL;
  int		nmsgs;

  error.Reset();

  mbox = parent()->mailbox();

  if (current < 0) {
    current = _displayed_item_position;
  }


  nmsgs = _msgs->length();
  if (nmsgs == 0)
    return;

  assert (mbox != NULL);

  //
  // We need to build a new list of strings to display
  // in the scrolling list.  Initialize that now.
  //
  XmString * newList;
  newList = new XmString[nmsgs];
  memset (newList, 0, nmsgs * sizeof (XmString *));

  // create list of messages that we are to display
  SdmMessageNumberL msgList;
  for (int i=0; i<nmsgs; i++) {
    msgList.AddElementToList(_msgs->at(i)->message_handle);
  }

  SdmIntStrLL	intStrHeaderList;
  SdmIntStrL	*intStrListPtr;
  SdmMessageFlagAbstractFlagsL flagsList = 0;

  // Get the designated headers (defined through _header_info)
  // for all the messages into intStrHeaderList
  mbox->GetHeaders(error, intStrHeaderList, _header_info, msgList);

  // if we didn't get any headers for any of the messages (this should be 
  // pretty rare but we need to handle it anyway), create empty lists for each
  // message and continue processing below.
  //
  if (error == Sdm_EC_RequestedDataNotFound) {
    for (i = 0; i < msgList.ElementCount(); i++) {
      intStrHeaderList.AddElementToList(new SdmIntStrL);
    }
    error = Sdm_EC_Success;  // reset error.
  } 
  if (error) {
    intStrHeaderList.ClearAndDestroyAllElements();
    return;
  }

  // Get the flags of all the messages in the mailbox (message store)
  // in the vector (array) of SdmMessageFlagAbstractFlagsL's
  mbox->GetFlags(error, flagsList, msgList);
  if (error) {
    intStrHeaderList.ClearAndDestroyAllElements();
    return;
  }

  // Loop through _msgs and create new strings to display in the
  // scrolling list. This is inefficient and dominates the time
  // spent in sort.  It may be worth while finding a way to just
  // rearrange the existing strings.
  for (int m = 0; m < nmsgs; m++) {
    intStrListPtr = intStrHeaderList[m];
    newList [m] = formatHeader(intStrListPtr,
		       _msgs->at(m)->message_handle,
		       _msgs->at(m)->sessionNumber + 1,
		       IsMultipartMessage(intStrListPtr),
		       isSet(flagsList[m], Sdm_MFA_Seen)  ? Sdm_False : Sdm_True);
  }

  XmListReplaceItemsPos(_w, newList, nmsgs, 1);
  for (int fr = 0; fr < nmsgs; fr++) {
    XmStringFree(newList[fr]);
  }

  // Update current message
  _selected_item_position = current;
  _displayed_item_position = current;
  scroll_to_position(_displayed_item_position);
  XmListSelectPos(_w, _displayed_item_position, Sdm_False);

  delete newList;
  intStrHeaderList.ClearAndDestroyAllElements();
}

static const char * DaysOfTheWeek[] = {
"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char * MonthsOfTheYear[] = {
"Jan", "Feb", "Mar",
"Apr", "May", "Jun",
"Jul", "Aug", "Sep",
"Oct", "Nov", "Dec"
};

static int
matchDay(const char * start, const char * end)
{
    int len = end - start + 1;

    for (int i = 0; i < 7; i++) {
	if (strncmp(DaysOfTheWeek[i], start, len) == 0) {
	    return(i);
	}
    }

    return(-1);
}

static int
matchMonth(const char * start, const char * end)
{
    int len = end - start + 1;

    for (int i = 0; i < 12; i++) {
	if (strncmp(MonthsOfTheYear[i], start, len) == 0) {
	    return(i);
	}
    }

    return(-1);
}

static void
parseTime(const char * start, const char * end, tm & val)
{
    int size = end - start + 1;

    // Time will be in the form hh:mm:ss where seconds are optional.

    char num_buf[10];
    strncpy(num_buf, start, 2);
    num_buf[2] = 0;

    val.tm_hour = strtol(num_buf, NULL, 10);

    strncpy(num_buf, &start[3], 2);
    num_buf[2] = 0;

    val.tm_min = strtol(num_buf, NULL, 10);

    if (size > 6) {
	strncpy(num_buf, &start[6], 2);
	num_buf[2] = 0;
	val.tm_sec = strtol(num_buf, NULL, 10);
    }
    else {
	val.tm_sec = 0;
    }

    return;
}

static const char * TZNames[] = {
"EST", "CST", "MST", "PST"
};

static const char * TZNamesDST[] = {
"EDT", "CDT", "MDT", "PDT"
};

static time_t
parseTZ(const char * start, const char * end)
{
    int size = end - start + 1;

    // There are at 3 possibilities that we understand. There
    // is the single letter military time zone. In that case
    // Z is 0 UTC. A-M is -1 to -12, skipping J. N-Y is +1 to +12
    // from UTC.
    //
    // Lets start with that one because it is the easiest.

    if (size == 1) {
	int hours_from = 0;
	if (*start >= 'A' && *start <= 'I') {
	    hours_from = *start - 'A' + 1;
	}
	else if (*start >= 'L' && *start <= 'M') {
	    hours_from = *start - 'K' + 10;
	}
	else if (*start >= 'N' && *start <= 'Y') {
	    hours_from = ('N' - *start) - 1;
	}

	return(hours_from * 3600);
    }

    // The next option is one of the ANSI standard time zones. These
    // are three letter abbrievations that tell us where DST in in effect.
    // So, if we have a length of three, lets see if it is in the table.
    if (size == 3) {
	// First normal zones.
	int i;
	for (i = 0; i < 4; i++) {
	    if (strncmp(start, TZNames[i], 3) == 0) {
		return((5 + i) * -3600);
	    }
	}

	// Now DST zones
	for (i = 0; i < 4; i++) {
	    if (strncmp(start, TZNames[i], 3) == 0) {
		return((4 + i) * -3600);
	    }
	}
    }

    // Finally we understand +/- HHMM from UTC.
    if (size == 5) {
	int sign = (*start == '+') ? 1 : -1;

	char num_buf[10];
	strncpy(num_buf, &start[1], 2);
	num_buf[2] = 0;
	int hours = strtol(num_buf, NULL, 10);

	strncpy(num_buf, &start[3], 2);
	num_buf[2] = 0;
	int minutes = strtol(num_buf, NULL, 10);

	return(sign * ((hours * 3600) + (minutes * 60)));
    }

    // We have no idea at this point, and it is very unlikely that the
    // text is meaningful to the reader either. Set the zone to UTC and
    // punt. It is also possible that the text is "UT" or "GMT" in which
    // case offset 0 is the right answer.

    return(0);
}

time_t
convertDate(const char *date_buf)
{
//     DtMailValueDate date;
//     const char * pos = _value;
    time_t new_date = 0;
    tm new_time;

    memset(&new_time, 0, sizeof(new_time));

    // Before doing anything, check to see if _value is valid.
    // Some messages have no Date string.  Return date with zeroed fields
    // in those cases.

    if (!date_buf || (strlen(date_buf) == 0)) return (new_date);

    // Find the first non-blank
    for (; *date_buf && isspace((unsigned char)*date_buf); date_buf++) {
	continue;
    }

    // There are usually no more than 6 tokens in an RFC date. We will
    // have a few extras just in case we are given a wierd string.
    const char *token_begin[12];
    const char *token_end[12];
    int	n_tokens = 0;

    // Look for the end of each token. Date tokens are white space
    // separated.
    while (*date_buf) {
	token_begin[n_tokens] = date_buf;
	for (; *date_buf && !isspace((unsigned char)*date_buf); date_buf++) {
	    continue;
	}

	if (*date_buf) {
	    token_end[n_tokens++] = date_buf - 1;
	}
	else {
	    token_end[n_tokens++] = date_buf;
	}

	for (; *date_buf && isspace((unsigned char)*date_buf); date_buf++) {
	    continue;
	}
	// This means the message is most likely corrupted so just bail out
	if (n_tokens == 12) 
		break;
    }

    // Some dates will have a comma after the day of the week. We
    // want to remove that. It will always be the first token if
    // we have the day of the week.
    if (*token_end[0] == ',') {
	token_end[0]--;
    }

    if (n_tokens < 2) {
	return(new_date);
    }

    // There are two possible formats, and many variations, that we
    // will see in an RFC message. They are:
    //
    // Tue Oct 12 10:36:10 1993
    // Tue, 12 Oct 1993 10:35:05 PDT
    //
    // The first is the 821 format put on by sendmail. The second is
    // one of the many variants of the 822 format. The big difference
    // we must detect is "mon dd time year" vs "dd mon year time tz"
    //
    // The first qualifier is usually the day of the week. For our purposes,
    // we will simply throw it away. This information will be recomputed
    // based on the date and time.

    int this_token = 0;

    int day = matchDay(token_begin[this_token], token_end[this_token]);
    if (day >= 0) {
	// Ignore the day.
	this_token += 1;
    }

    // This token should either be a numeric day, or an alpha month.
    // Lets see if it is a month. If so, we know what the rest of
    // the date will look like.

    int month = matchMonth(token_begin[this_token], token_end[this_token]);
    if (month >= 0) {
	new_time.tm_mon = month;

	// Now should be the day of the month.
	char num_buf[20];
	this_token += 1;

	if (this_token == n_tokens) {
	    return(new_date);
	}

	strncpy(num_buf, token_begin[this_token], 2);
	num_buf[2] = 0;
	new_time.tm_mday = strtol(num_buf, NULL, 10);

	this_token += 1;
	if (this_token == n_tokens) {
	    return(new_date);
	}

	parseTime(token_begin[this_token], token_end[this_token], new_time);

	this_token += 1;
	if (this_token == n_tokens) {
	    return(new_date);
	}


	// Sometimes the Unix date will include the time zone.
	//
	if (isalpha(*token_begin[this_token])) {
	    this_token += 1;
	    if (this_token == n_tokens) {
		return(new_date);
	    }
	}

	// The year, which is either 2 or 4 digits.
	int t_size = token_end[this_token] - token_begin[this_token] + 1;
	strncpy(num_buf, token_begin[this_token], t_size);
	num_buf[t_size] = 0;
	new_time.tm_year = strtol(num_buf, NULL, 10);
	if (new_time.tm_year > 1900) {
	    new_time.tm_year -= 1900;
	}

	new_time.tm_isdst = -1;
	new_date = SdmSystemUtility::SafeMktime(&new_time);
// 	date.dtm_tz_offset_secs = timezone;
    }
    else {
	// In this format, we should have a day of the month.
	char num_buf[20];
	strncpy(num_buf, token_begin[this_token], 2);
	num_buf[2] = 0;
	new_time.tm_mday = strtol(num_buf, NULL, 10);

	this_token += 1;
	if (this_token == n_tokens) {
	    return(new_date);
	}

	// Now the month name.
	new_time.tm_mon = matchMonth(token_begin[this_token], token_end[this_token]);

	this_token += 1;
	if (this_token == n_tokens) {
	    return(new_date);
	}

	// The year, which is either 2 or 4 digits.
	int t_size = token_end[this_token] - token_begin[this_token] + 1;
	strncpy(num_buf, token_begin[this_token], t_size);
	num_buf[t_size] = 0;
	new_time.tm_year = strtol(num_buf, NULL, 10);
	if (new_time.tm_year > 1900) {
	    new_time.tm_year -= 1900;
	}

	this_token += 1;
	if (this_token == n_tokens) {
	    return(new_date);
	}

	// The time, in the specified time zone.
	parseTime(token_begin[this_token], token_end[this_token], new_time);

	this_token += 1;
	if (this_token == n_tokens) {
	    return(new_date);
	}

	time_t offset = parseTZ(token_begin[this_token], token_end[this_token]);

	time_t orig_zone = timezone;
	timezone = offset;
	new_time.tm_isdst = 1;

	new_date = SdmSystemUtility::SafeMktime(&new_time);
// 	date.dtm_tz_offset_secs = offset;

	timezone = orig_zone;
    }

    return(new_date);
}


/*
 * Copyright (c) 1996, by Sun Microsystems, Inc.
 * All rights reserved.
 */
#pragma	ident	"@(#)dtcoltobytes.c 1.1	96/11/07  SMI"

/*
 *  void __dt_coltobytes(char *mbstring, int width_col, int prec_col,
 *			int *width_bytes, int *prec_bytes)
 *
 *  This routine is intended to be used with XPG4 behavior of
 *  printf "%<width>.<precision>s" format.  It helps provide Solaris
 *  printf %s behavior for column-aligned output in an XPG4 environment.
 *
 *  This routine takes a multibyte string, positive integer width in
 *  screen columns, and positive integer precision in screen columns.
 *  It passes back the number of bytes to use as a width argument
 *  in width_bytes, and the number of bytes to use as a precision
 *  argument in prec_bytes.  The number of bytes returned are such
 *  that they include only complete multibyte characters, and also such
 *  that the number of screen display columns for width or precision
 *  are not exceeded.
 *
 *  For Solaris %s compatibilty, if illegal multibyte characters are
 *  detected, they are skipped over, and each byte is counted as 1
 *  screen column width.  If wcwidth detects a non-printing wide
 *  character. its width is counted as 1 screen column width.
 *
 *  If number of bytes for width in screen columns or precision in
 *  screen columns is not desired, then width_col or prec_col should
 *  be set to -1.  If width_col or prec_col is set to -1, then
 *  width_bytes or prec_bytes may be set to NULL.
 *
 */
#include <wchar.h>
#include <stdlib.h>
#include <limits.h>

#define	_CALC_WIDTH	1	/* call to __do_calc is for width */
#define	_CALC_PRECISION	2	/* call to __do_calc is for precision */

void __dt_coltobytes(char *, int, int, int *, int *);
static int __do_calc(char *, int, int *, int);

void
__dt_coltobytes(char *string, int width_col, int prec_col,
	int *width_bytes, int *prec_bytes)
{
	int cols = 0;
	int p_bytes = 0;	/* bytes for precision */
	int w_bytes = 0;	/* bytes for width */
	int dummy;

	/* Calculate precision columns to bytes */
	if (prec_col >= 0) {
		p_bytes = __do_calc(string, prec_col, &cols, _CALC_PRECISION);
		*prec_bytes = p_bytes;
	}

	/* Calculate width columns to bytes */
	if (width_col >= 0) {
		if (prec_col >= 0) {
			/*
			 * Both width and precision were specified, so
			 * need to take precision into account
			 */
			if (width_col > cols) {
				w_bytes = p_bytes + width_col - cols;
			} else {
				w_bytes = p_bytes;
			}
		} else {
			/* Only width was specified */
			w_bytes = __do_calc(string, width_col, &dummy,
						_CALC_WIDTH);
		}
		*width_bytes = w_bytes;
	}
}

/*
 *  calculate # of bytes
 *  type = _CALC_WIDTH for width
 *         _CALC_PRECISION for precision
 */
static int
__do_calc(char *string, int ncolumns, int *retcols, int type)
{
	int cols, nbytes, ncol;
	wchar_t wchr;
	int totbytes;

	cols = ncolumns;
	totbytes = 0;

	while (cols) {
		if ((nbytes = mbtowc(&wchr, string, MB_LEN_MAX)) == -1) {
			/*
			 * printf(3) skips over (and prints) illegal bytes
			 * and calculates display as 1 column.
			 */
#ifdef DEBUG
			printf("mbtowc 0x%x 0x%x 0x%x ... returned error\n",
				*(string), *(string+1), *(string+2));
#endif
			nbytes = 1;
			ncol = 1;
		} else {
			if (nbytes == 0)		/* end of string */
				break;
			if ((ncol = wcwidth(wchr)) == -1) {
				/*
				 * printf(3) calculates 1 column for
				 * non-printing characters
				 */
				ncol = 1;
			}
		}
		/* Don't exceed the number of columns requested */
		if ((cols - ncol) < 0)
			break;
		cols -= ncol;
		string += nbytes;
		totbytes += nbytes;
	}
	/* For width calculation, calculate length for pad characters */
	if (type == _CALC_WIDTH) {
		totbytes += cols;
	}
	*retcols = ncolumns - cols;	/* return # columns occupied */
	return (totbytes);
}


  //
  // ATTENTION!	This function is a performance bottleneck.  Make sure
  // 		that any change you make here are as efficient as possible.
  //
#define	MAX_DATE	25
#define	MAX_TO		40

XmString
MsgScrollingList::formatHeader(SdmIntStrL *info,
			       int msg_num,
			       int view_msg_num,
			       SdmBoolean multi_part,
			       SdmBoolean new_msg)
{
  char buf[256];
  memset(buf, 0, sizeof(buf));
  const char *from=NULL;
  const char *from_alt=NULL;
  const char *user=NULL;
  char *subject;
  SdmString concat_subj;
  SdmString concat_from;
  SdmString concat_from_alt;
  SdmString concat_to;
  SdmString concat_user;
  int contentLength;
  char contentStr[20];
  char date[MAX_DATE];
  char new_date[MAX_DATE];
  SdmString messageOriginalHeader;
  static const char *kEmptyString = " ";
  static int kEmptyString_length = 1;	// make sure this matches the length of kEmptyString!!
  static char *new_str;
  static int new_str_length;
  static XmString attachment_glyph = NULL;
  static XmString no_str = NULL;
  static unsigned char attach_symbol[16];
  SdmBoolean showto = Sdm_False;

  memset(date, 0, MAX_DATE);
  memset(contentStr, 0, 20);

  if ( !attachment_glyph ) {
    char *tmp_str;
    attach_symbol[0] = 168; // Diamond character
    attach_symbol[1] = 0;
    attachment_glyph = XmStringCreate((char *)attach_symbol, "attach");
    no_str = XmStringCreateLocalized ((String)kEmptyString);
    tmp_str = catgets (DT_catd, 1, 114, "N");
    new_str_length = strlen(tmp_str);
    new_str = new char [new_str_length + 1];
    strcpy (new_str, tmp_str);
  }

  // IMAP is incapable of handling a message header that begins
  // with From (space) (NOTE: not a From:).
  // Thus, if a message has only a From but does not have a From:
  // or a Reply-To: or Received-From:, IMAP doesn't tell us who
  // it is from.  In such cases, we set it to "???".

  // We are interested in the following flags:
  // 		Sdm_MHA_P_MessageFrom
  //		Sdm_MHA_P_ReceivedDate
  //		Sdm_MHA_P_ContentLength
  //		Sdm_MHA_Subject
  //


  // I18N - decode the from, from_alt, to, and subject headers here
  SdmError error;
  SdmMessageStore *mbox = parent()->mailbox();
  SdmString defaultCharset("");

  int i;
  for (i = 0; i < info->ElementCount(); i++) {
    int messageFlags = (*info)[i].GetNumber();
    error = Sdm_EC_Success;
    messageOriginalHeader = (*info)[i].GetString();
    if ( isSet(messageFlags, Sdm_MHA_P_SenderPersonalInfo) ) {
      // I18N Decode the from header
      SdmMessageEnvelope::DecodeHeader(error, concat_from, messageOriginalHeader, defaultCharset);
    } 
    if ( isSet(messageFlags, Sdm_MHA_P_MessageFrom) ) {
      from_alt = (const char *)messageOriginalHeader;
      // I18N Decode the from_alt header
      SdmMessageEnvelope::DecodeHeader(error, concat_from_alt, messageOriginalHeader, defaultCharset);
    } 
    else if ( isSet(messageFlags, Sdm_MHA_P_ReceivedDate) ) {
      strncpy(date, (const char *)messageOriginalHeader, MAX_DATE - 1);
      date[MAX_DATE-1] = '\0';
    }
    else if (isSet(messageFlags, Sdm_MHA_P_MessageSize)) {
      strncpy(contentStr, (const char *)messageOriginalHeader, 20 - 1);
    } 
    else if ( isSet(messageFlags, Sdm_MHA_P_Subject) ) {
      // I18N Decode the subject header
      SdmMessageEnvelope::DecodeHeader(error, concat_subj, messageOriginalHeader, defaultCharset);
    }
    else if (_showTo && isSet(messageFlags, Sdm_MHA_P_SenderUsername) ) {
      // I18N Decode the to header
      SdmMessageEnvelope::DecodeHeader(error, concat_user, messageOriginalHeader, defaultCharset);
    }
  }
  // I18N end
  SdmStrStrL xUnixFrom;
  if (concat_from.Length() == 0) {
    if (concat_from_alt.Length() != 0) {
      from = (const char*)from_alt;
    } else {    // As a last resort, try to get the unix "From " line string
      mbox->GetHeader(error, xUnixFrom, "x-unix-from", (SdmMessageNumber)msg_num);
      if (xUnixFrom.ElementCount() != 0) {
        from = (const char*)xUnixFrom[0].GetSecondString();
        char *p;
        // IMPORTANT NOTE: To avoid unnecessary duplication, the following
        // code writes DIRECTLY into the internal SdmString object.  BE VERY
        // CAREFUL about applying other SdmString operations to this object,
        // since the SdmString believes its length to still be the original
        // length.
        if (p = strchr(from, ' '))
          *p = '\0';
      }
      else
        from = "???";
    }
  } else {
    from = (const char*)concat_from;
  }

  if (_showTo) {
    if (concat_user.Length() != 0) {
      user = (const char*)concat_user;
    } else {
      mbox->GetHeader(error, xUnixFrom, "x-unix-from", (SdmMessageNumber)msg_num);
      if (xUnixFrom.ElementCount() != 0) {
        concat_user = xUnixFrom[0].GetSecondString();
        if (concat_user.Length() != 0) {
          user = (const char*)concat_user;
          char *p;
          // IMPORTANT NOTE: To avoid unnecessary duplication, the following
          // code writes DIRECTLY into the internal SdmString object.  BE VERY
          // CAREFUL about applying other SdmString operations to this object,
          // since the SdmString believes its length to still be the original
          // length.
          if ((p = strchr(user, '@')) != NULL || (p = strchr(user, ' ')) != NULL)
            *p = '\0';
        }
      }
    }

    if (IsUserMyself(user) == Sdm_True) {
      SdmStrStrL hdr;
      mbox->GetHeader(error, hdr, "to", (SdmMessageNumber)msg_num);
      if (hdr.ElementCount() != 0) {
        messageOriginalHeader = hdr[0].GetSecondString();
        // I18N Decode the to header
        SdmMessageEnvelope::DecodeHeader(error, concat_to, messageOriginalHeader, defaultCharset);
        if (concat_to.Length() != 0) {
          from = (const char*)concat_to;
          showto = Sdm_True;
        }
      }
    }
  }  
      
  // Prepare a proper "Subject" header to display
  // A message may or may not have a subject header.
  // If it does have a subject header, the header may consist of one
  // or more lines which have to be dealt with properly here.
  //  
  if (concat_subj.Length() > 0) {
    const char *from_real_subj = (const char*)concat_subj;
    subject = new char[concat_subj.Length()+2];
    char *to_tmp_subj = subject;
    char cur_subj_char;
 
    // Get the BE store of header.  It may contain newlines or
    // tab chars which can munge the scrolling list's display!
    //   

    while (cur_subj_char = *from_real_subj++) {
      if ( (cur_subj_char == '\t') || (cur_subj_char == '\r')) {
        continue;
      }  
      if (cur_subj_char == '\n') {
        // Newline seen - if space follows newline, ignore it as that is how
        // continuation lines in headers are inserted (e.g. text<newline><space>text)
        //
        if (*from_real_subj == ' ') {
          from_real_subj++;
        }
        continue;
      }  
      *to_tmp_subj++ = cur_subj_char;
    }
    *to_tmp_subj = '\0';
  } else {
    subject = strdup(kEmptyString);
  }

  if (*contentStr != NULL) {
    contentLength = (int) strtol(contentStr, NULL, 10);
  }
  else {
    contentLength = 0;
  }

  if (contentLength < 1000) {
    sprintf(contentStr, "%d", contentLength);
  }
  else if (contentLength < 1000000) {
    sprintf(contentStr, "%dK", contentLength / 1000);
  }
  else {
    sprintf(contentStr, "%dM", contentLength / 1000000);
  }

  int width_bytes=35, prec_bytes=35;

  // We need to localize the date string returned by the c-client.
  // Convert the date from char * to time_t.  We only need to do this
  // if the locale is set to something other than "C".
  if (!_in_C_locale) {
      tm epoch;
      time_t numeric_date;

#if (SunOS >= 56)
      // Starting in 2.6,  printf width and precision fields are now
      // XPG4 compliant.  This means they no longer represent screen
      // columns, but rather the number of bytes needed to display
      // the given string.
      __dt_coltobytes(subject, 35, 35, &width_bytes, &prec_bytes);
#endif
      numeric_date = convertDate(date);
      SdmSystemUtility::SafeLocaltime(&numeric_date, epoch);
#ifdef sun
      SdmSystemUtility::SafeStrftime(new_date, sizeof(new_date), 
	     catgets(DT_catd, 1, 222, "%a %b %e %k:%M"), &epoch);
#else
      SdmSystemUtility::SafeStrftime(new_date, sizeof(new_date), 
	     catgets(DT_catd, 1, 223, "%a %b %e %H:%M"), &epoch);
#endif
  } else {
      strcpy(new_date, date);
  }
  

  // If we are to print the message_number in the header_list,
  // use msg_num as the first element in the sprintf.
  // Introduce a %d at the beginning though.

  if (showto)
        sprintf(buf, " To %-15.15s %-*.*s %-*.*s %-5.5s",
		from,
		width_bytes,
		prec_bytes,
		subject,
		_date_width, 
		_date_width, 
		new_date,
		contentStr);
   else
        sprintf(buf, " %-18.18s %-*.*s %-*.*s %-5.5s",
		from,
		width_bytes,
		prec_bytes,
		subject,
		_date_width,
		_date_width,
		new_date,
		contentStr);

  delete [] subject;


  // Create the final header form - need to prepend:
  // <number><new><attachments><BUF> This code is written to minimize
  // the number of data copies done to speed performance.

  XmString complete_header;

  if (multi_part == Sdm_True) {
    XmString concat_header, concat_attach, concat_new;
    char str_new[32];

    // Concatenate all the strings before creating XmStrings

    if (_numbered) {
      if (new_msg)
	sprintf(str_new, "%4d %s", view_msg_num, new_str);
      else
	sprintf(str_new, "%4d  ", view_msg_num);
    }
    else {
      if (new_msg)
	sprintf(str_new, " %s", new_str);
      else
	strcpy(str_new, "  ");
    }

    // Ok, prefix created - create XmStrings and concatenate it all

    concat_new = XmStringCreateLocalized (str_new);
    concat_attach = XmStringConcat(concat_new, attachment_glyph);
    XmStringFree(concat_new);

    concat_header = XmStringCreateLocalized (buf);
    complete_header = XmStringConcat (concat_attach, concat_header);
    XmStringFree(concat_attach);
    XmStringFree(concat_header);
  }
  else {
    char *str_header;
    str_header = new char [32 + new_str_length + kEmptyString_length + strlen (buf) + 1];
    if (_numbered) {
      if (new_msg)
	sprintf(str_header, "%4d %s %s", view_msg_num, new_str, buf);
      else
	sprintf(str_header, "%4d   %s", view_msg_num, buf);
    }
    else {
      if (new_msg)
	sprintf(str_header, " %s %s", new_str, buf);
      else
	sprintf(str_header, "   %s", buf);
    }
    complete_header = XmStringCreateLocalized (str_header);
    delete str_header;
  }

  return(complete_header);

}

void 
MsgScrollingList::GetUsernames()
{
  _usernames.ClearAllElements();

  passwd pw;
  SdmSystemUtility::GetPasswordEntry(pw);
  _usernames(-1) = pw.pw_name;        // include user login (eg. esthert)

  if (_useAlternates) {
    SdmMessageUtility::AddAlternates(_usernames);
  }
}


SdmBoolean 
MsgScrollingList::IsUserMyself(const char *user)
{
  if (user != NULL) {
    for (int i=0; i<_usernames.ElementCount(); i++) {
      if (_usernames[i] == user) 
        return Sdm_True;
    }
  }
  return Sdm_False;
}

void
MsgScrollingList::clearMsgs()
{
  if ( _msgs->length() > 0 )
    _msgs->clear();
}

void
MsgScrollingList::display_message_summary()
{
  parent()->message_summary(
    _selected_item_position,
    _session_message_number, 
    _num_new_messages,
    _num_deleted_messages);
}

SdmMessageNumber MsgScrollingList::current_msg_handle()
{
  SdmMessageNumber msg_number;

  if ( _displayed_item_position > 0 )
    msg_number=msgno( _displayed_item_position );
  else
    msg_number=NULL;

  return msg_number;
}

SdmBoolean
MsgScrollingList::warnAboutDestroy()
{
  // If the number of deleted messages in the current view is less
  // than the number of total deleted messages in the entire mailbox,
  // return True.
  if (_num_deleted_messages < _total_deleted)
      return(Sdm_True);
  else
      return(Sdm_False);
}

void
MsgScrollingList::expunge(int num_deleted, SdmMessageNumberL &msgList)
{
    _last_msg_num -= num_deleted;
    _deleted_messages->remove_and_free_all_elements();

    // Destroy the msg number list that the load_headersWP routine
    // uses.  This will cause that workproc to stop adding headers
    // to the MSL.  We are doing this because msg numbers are being
    // changed by this routine so the ones in _msgNumL are no longer
    // valid.
    if (_msgNumL != NULL) {
	delete _msgNumL;
	_msgNumL = NULL;
    }

    // The following if block is needed to keep the new message view
    // stats correct.  The "new" view has a tendancy to get out of sync
    // with the messages that are actually loaded in the MSL.
    if (_current_ss != NULL) {
      if (!strcmp((const char *)*_current_ss, SdmSearch::Sdm_STK_Unread)) {
	Views *vp = parent()->get_views_dialog(Sdm_False, Sdm_False);
	// We shouldn't need to check for a valid views object here since
	// _current_ss wouldn't be set if we didn't have a Views object.
	if (vp) {
	  vp->updateNewViewStats();
	}
      }
    }
    // resetSessionNums() must come after the call to vp->updateNewViewStats()
    // to get the correct view stats for the new message view.
    resetSessionNums();
    _num_deleted_messages = 0;
    _total_deleted = 0;

    // close views for any deleted messages that are displayed. 
    ViewMsgDialog* tmpView; 
    for (int i=0; i < msgList.ElementCount(); i++) {
      tmpView = parent()->ifViewExists(msgList[i]);
      if (tmpView) {
        tmpView->quit();
      }
    }

    // if the list was sorted, we need to re-sort it because
    // resetSessionNums renumbers everything from 1 to n. 
    if (_parent->lastSortCmd() != NULL) {
      SortCmd *cmd = (SortCmd*)_parent->lastSortCmd();
      // we want to just use the sorter and sorting style from the
      // command.  don't call doit for the command because it
      // changes _selected_item_position and _displayed_item_position
      // incorrectly and updates the message header list unnecessarily.
      cmd->getSorter()->sortMessages (this, _parent->mailbox(), cmd->getSortStyle());

      SdmMailRc * mailrc;
      SdmError error;
      theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailrc);
      if ( error ) {
          theRoamApp.genDialog()->post_error(error);
      } else {
        // 
        // update the message scrolling list to reflect the newly sorted messages.
        // the reason why we need to resort the list is that new mail could have come in
        // when the list was sorted.  The new mail is added to the bottom of the list and
        // is not in sorting order within the list.  When we resort the message numbers,
        // the message numbers in the message list is correct but if there was new mail,
        // the messages in the message scrolling list is not reflecting this order because
        // the new mail is in the bottom.  Therefore, we need to redisplay the message
        // scrolling list so that it reflects the newly sorted message numbers.
        //
        // we only want to update the list if we are not showing the message number.
        // in the case when we are showing the message number, the message list
        // will get updated in the RoamMenuWindow::expunge so we don't need to do it here.
        //
        if (mailrc->IsValueDefined("showmsgnum") == Sdm_False) {
          // Need to update scrolling list to display
          // resequenced msg nums
          updateListItems(error, -1);

          if (error) {
	    theRoamApp.genDialog()->post_error(error, DTMAILHELPCANNOTUPDATEMSGHDRLIST,
					       catgets(DT_catd, 3, 133, "Mailer cannot update the Message Header List."));
          }
        }
      }
    }

    // need to update the message number because our message could have
    // been renumbered after the expunge.
    _displayed_msgno = current_msg_handle();
  
    display_message_summary();
}

int
MsgScrollingList::resetSessionNums(void)
{
  int	m = 0, length = _msgs->length();
  MsgStruct *ms;

  // if the current view is not "ALL" messages, PerformSearch to get
  // current message handles that match view criteria.  Update _msgs
  // list with the new message handles.
  if ((_current_ss != NULL) && (*_current_ss != SdmSearch::Sdm_STK_All)) {
      SdmError error;
      SdmMessageNumberL matching_msgs;
      SdmMessageStore *mbox = parent()->mailbox();
      SdmSearch search;

      search.SetSearchString(error, (const SdmString) *_current_ss);
      if (!error) {        
        mbox->PerformSearch(error, matching_msgs, search, 1, _last_msg_num);
      }
      if (!error) {
        // Special case: when the MSL's length is != length of matching_msgs
        // list this means that there are messages in the view that
        // no longer belong there.  By belong I mean that they no
        // longer meet the view criteria.  An example would be a read
        // message in the "new" view.  I think the best thing to do
        // here is to reload the view.
        int new_len = matching_msgs.ElementCount();
	if (length != new_len) {
          theRoamApp.busyAllWindows(catgets(DT_catd, 3, 140,
                    "Reloading View..."));
          load_view(error, NULL, _current_ss);
          theRoamApp.unbusyAllWindows();
          assert(!error);
          return(-1);
        }
        for (m = 0; m < length; m++) {
          ms = _msgs->at(m);
          ms->sessionNumber = m;
          ms->message_handle = matching_msgs[m];
        }
      } else {
        _msgs->remove_and_free_all_elements();
      }
  } else {
    for (m = 0; m < length; m++) {
      ms = _msgs->at(m);
      ms->sessionNumber = m;
      ms->message_handle = m + 1;
    }
  }
  _session_message_number = m;
  return _session_message_number;
}

//
// Layout out the row of labels above the scrolling list
//
void
MsgScrollingList::layoutLabels(
       int    msgnums, 
       Widget sender,
       Widget subject,
       Widget date,
       Widget size)
{
  // Width of fields.  +1 for spaces
  int	num_width = 5,
  sender_width = 18 + 1,
  subject_width = 35 + 1,
  date_width;
  int	char_width;		// Width of a single character
  int	n = 0;
  XmString	xmstr;
  XmFontList	font_list;

  // Calculate the width of date format to allocate the label 
  // width dynamically
  struct tm *tm;
  time_t clock;
  char   buf[40];

  clock = time((time_t *) 0);
  tm = localtime(&clock);  
#ifdef sun
  SdmSystemUtility::SafeStrftime(buf,
         sizeof(buf), 
         catgets(DT_catd, 1, 222, "%a %b %d %k:%M"), 
         tm);
#else
  SdmSystemUtility::SafeStrftime(buf, 
         sizeof(buf), 
         catgets(DT_catd, 1, 223, "%a %b %d %H:%M"), 
         tm);
#endif

  _date_width = strlen(buf);       // width of date format
  date_width = _date_width + 1;    // plus one space in label

  // List uses a fixed width font. Therefore all characters are the
  // same size.  So we use a space to determine the width of a char
  xmstr = XmStringCreateLocalized(" ");
  XtVaGetValues(_w, XmNfontList, &font_list, NULL);
  char_width = XmStringWidth(font_list, xmstr);


  n = 0;      // horizontal position in pixels

  // If the scrollbar is on the left, move header over.
  unsigned char sbp = XmBOTTOM_RIGHT;
  // XXXX - default really depends on XmNstringDirection
  XtVaGetValues(XtParent(_w), XmNscrollBarPlacement, &sbp, NULL);
  if (sbp == XmTOP_LEFT || sbp == XmBOTTOM_LEFT) {
    Widget sb = NULL;
    Dimension width = 0, spacing = 0;

    XtVaGetValues(XtParent(_w), XmNverticalScrollBar, &sb, NULL);
    if (sb != NULL) {
      XtVaGetValues(sb, XmNwidth, &width, NULL);
      XtVaGetValues(sb, XmNspacing, &spacing, NULL);
    }
    n += width + spacing;
  }

  if (msgnums) {
    // Numbering is on
    n += num_width * char_width;
  }
  n += 4 * char_width;	// Margin.
  n += 1;			// XXX - I think this should be highlightThickness
  XtVaSetValues(sender, XmNx, n, NULL);
  n += sender_width * char_width;
  XtVaSetValues(subject, XmNx, n, NULL);
  n += subject_width * char_width;
  XtVaSetValues(date, XmNx, n, NULL);
  n += date_width * char_width;
  XtVaSetValues(size, XmNx, n, NULL);

  XmStringFree(xmstr);

  return;
}
