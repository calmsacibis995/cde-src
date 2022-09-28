/*
 *+SNOTICE
 *
 *      $Revision: 1.2 $
 *
 *      RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *      The information in this document is subject to special
 *      restrictions in a confidential disclosure agreement between
 *      HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *      document outside HP, IBM, Sun, USL, SCO, or Univel without
 *      Sun's specific written approval.  This document and all copies
 *      and derivative works thereof must be returned or destroyed at
 *      Sun's request.
 *
 *      Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */
 
#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Undelete.C	1.60 06/13/97"
#else
static char *sccs__FILE__ = "@(#)Undelete.C	1.60 06/13/97";
#endif
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include "Undelete.hh"
#include "MsgScrollingList.hh"
#include "RoamMenuWindow.h"
#include "DialogShell.h"
#include "Application.h"
#include "ButtonInterface.h"
#include "RoamCmds.h"
#include "RoamApp.h"
#include "CmdList.h"
#include "MsgHndArray.hh"
#include "Help.hh"
#include "MemUtils.hh"
#include "MailMsg.h"
#include "DtMailGenDialog.hh"


UndelMsgScrollingList::UndelMsgScrollingList (RoamMenuWindow *rmw, Widget w,
						char *string)
    : MsgScrollingList(rmw, w, string) 
{
}

UndelMsgScrollingList::~UndelMsgScrollingList()
{
}

void
UndelMsgScrollingList::extended_selection(SdmError &, int)
{
}


void
UndelMsgScrollingList::deleteSelected(SdmBoolean)
{
    int  position_in_list, i;
    FORCE_SEGV_DECL(int, position_list);
    int position_count;

 
    if (!XmListGetSelectedPos(_w, &position_list, &position_count))
	return;
 
    for (i=0; i < position_count; i++ ) {
        position_in_list = *(position_list + i);
        _msgs->mark_for_delete(position_in_list - 1);
    }
 
    _msgs->compact(0);
 
 
    // Delete the items from the scrolling list
    XmListDeletePositions(_w,
                          position_list,
                          position_count);
 
    // XnListGetSelectedPos allocated memory for the array and
    // wants us to free it.  See documentation.
    free(position_list);
}
 
void
UndelMsgScrollingList::insertMsg(SdmMessageNumber tmpMH)
{
    MsgScrollingList::insertMsg(tmpMH);
}

// Checks the value for bits set in mask
#define isSet(v, m)     ((unsigned long)(v) & (unsigned long)(m))
 
void
UndelMsgScrollingList::insertMsg(
    SdmError &mail_error, 
    MsgStruct *tmpMS
)
{
    XmString item;

    _msgs->append(tmpMS);

    SdmIntStrL	intStrHeaderList;
    SdmMessageFlagAbstractFlags flagsList;

    SdmMessageStore * mbox = this->parent()->mailbox();

    mbox->GetHeaders(mail_error, intStrHeaderList, _header_info, tmpMS->message_handle);
    
    // if we didn't get any headers for the message, reset the
    // error and continue processing below.  the message scrolling
    // list will end up showing empty fields for this message.
    //
    if (mail_error == Sdm_EC_RequestedDataNotFound) {
      mail_error = Sdm_EC_Success;  // reset error.
    } 
    if (mail_error)  return;
    
    mbox->GetFlags(mail_error, flagsList, tmpMS->message_handle);
    if (mail_error)  return;

    item = formatHeader(&intStrHeaderList,
			tmpMS->message_handle,
			tmpMS->sessionNumber+1,
			IsMultipartMessage(&intStrHeaderList),
			isSet(flagsList, Sdm_MFA_Seen) ? Sdm_False : Sdm_True);

    XmListAddItem( _w , item, 0 );
    XmStringFree( item );
 
    scroll_to_bottom();
}

// loadMsgs loads in the list of deleted messages into the Deleted Message
// scrolling list.
void
UndelMsgScrollingList::loadMsgs( SdmError &mail_error,
				 MsgHndArray *deleted_msgs,
				 int count)
{
    XmString *msg_hdrs;
    int i;
    SdmIntStrLL	intStrHeaderList;
    SdmIntStrL	*intStrListPtr;
    SdmMessageFlagAbstractFlagsL flagsList;


    // If there are already items in the list, remove them.
    if (_msgs->length() > 0) {
	XmListDeleteAllItems(_w);
	// We own the MsgStruct's that live in the _msgs array so
	// it is ok for us to free them.  
	_msgs->remove_and_free_all_elements();
    }

    if (count == 0) 
      return;

    msg_hdrs = (XmString *)malloc(sizeof(XmString) * count);
    memset(msg_hdrs, 0, sizeof(XmString) * count);    

    SdmMessageStore * mbox = this->parent()->mailbox();
    SdmMessageNumberL  msgNumberList;

    for (i = 0; i < count; i++) {
      msgNumberList.AddElementToList(deleted_msgs->at(i)->message_handle);
    }

    mbox->GetHeaders(mail_error, intStrHeaderList, _header_info, msgNumberList);
    // if we didn't get any headers for any of the messages (this should be 
    // pretty rare but we need to handle it anyway), create empty lists for each
    // message and continue processing below.
    //
    if (mail_error == Sdm_EC_RequestedDataNotFound) {
      for (i = 0; i < count; i++) {
        intStrHeaderList.AddElementToList(new SdmIntStrL);
      }
      mail_error = Sdm_EC_Success;  // reset error.
    } 
    if (mail_error) return;

    mbox->GetFlags(mail_error, flagsList, msgNumberList);
    if (mail_error) return;

    for (i = 0; i < count; i ++) {
      MsgStruct *tmpMS = deleted_msgs->at(i);
      _msgs->append(tmpMS);

      intStrListPtr = intStrHeaderList[i];

      msg_hdrs[i] = formatHeader(intStrListPtr,
            tmpMS->message_handle,
            tmpMS->sessionNumber+1,
            IsMultipartMessage(intStrListPtr),
            isSet(flagsList[i], Sdm_MFA_Seen) ? Sdm_False : Sdm_True);
    }

    XmListAddItems(_w, msg_hdrs, count, 0);

    // Free the strings we added to the header array.
    for (i = 0; i < count; i++) {
      XmStringFree(msg_hdrs[i]);
    }

    free (msg_hdrs);
    intStrHeaderList.ClearAndDestroyAllElements();

    scroll_to_bottom();
    // free the memory allocated for the MsgHndArray but not the
    // MsgStructs that it contained.
//     delete deleted_msgs;
}

void
UndelFromListDialog::loadMsgs( SdmError &mail_error, 
			       MsgHndArray *deleted_msgs,
			       int count)
{
    _list->loadMsgs(mail_error, deleted_msgs, count);
}


UndelFromListDialog::UndelFromListDialog ( 
    char *name, 
    RoamMenuWindow *parent
) : DialogShell ( name, parent )
{
    assert (theApplication);
    _list = NULL;
}

UndelFromListDialog::~UndelFromListDialog()
{
}

void
UndelFromListDialog::popped_down()
{
    XtUnmapWidget(this->baseWidget());
}

void
UndelFromListDialog::popped_up()
{
  if (_workArea) {
    if (XtIsManaged(_workArea)) {
      Widget w = this->baseWidget();
      XtMapWidget(w);
      XRaiseWindow(XtDisplay(w), XtWindow(w));
    } else {
      XtManageChild(_workArea);
      XtMapWidget(this->baseWidget());
    }
  }
}

void
UndelFromListDialog::initialize()
{
    DialogShell::initialize();
}

void
UndelFromListDialog::quit()
{
    // Pop down the dialog
    this->popped_down();
}

Widget
UndelFromListDialog::createWorkArea ( Widget parent )
{

    XmString labelStr;
    Widget l1, l2, l3, l4;
    Widget form1;
    XtWidgetGeometry size;

    form1 = XmCreateForm(parent, "Work_Area",  NULL, 0);

    rowOfLabels = XtCreateManagedWidget ("rowOfLabels",
			xmFormWidgetClass, 
			form1,
			NULL, 0);

    size.request_mode = CWHeight;
    XtQueryGeometry(rowOfLabels, NULL, &size);
    XtVaSetValues(rowOfLabels,
		XmNpaneMaximum, size.height,
		XmNpaneMinimum, size.height,
		XmNrightAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
		XmNtopOffset, 5,
		XmNorientation, XmHORIZONTAL,
		XmNspacing, 120,
		XmNmarginWidth, 20,
		XmNentryAlignment, XmALIGNMENT_CENTER,
		NULL);

    labelStr = XmStringCreateLocalized(catgets(DT_catd, 1, 14,
           "Sender"));

    l1 = XtVaCreateManagedWidget("Label1", xmLabelGadgetClass,
                rowOfLabels,
                XmNlabelString, labelStr,
                NULL);

    XmStringFree(labelStr);

    labelStr = XmStringCreateLocalized(catgets(DT_catd, 1, 15,
           "Subject"));
    l2 = XtVaCreateManagedWidget("Label2",
                                xmLabelGadgetClass, rowOfLabels,
                                XmNlabelString, labelStr,
                                NULL);



    XmStringFree(labelStr);
    labelStr = XmStringCreateLocalized(catgets(DT_catd, 1, 16,
           "Date and Time"));
    l3 = XtVaCreateManagedWidget("Label3",
                        xmLabelGadgetClass, rowOfLabels,
                        XmNlabelString, labelStr,
                        NULL);

    XmStringFree(labelStr);
    labelStr = XmStringCreateLocalized(catgets(DT_catd, 1, 17,
           "Size"));
    l4 = XtVaCreateManagedWidget("Label4",
                xmLabelGadgetClass, rowOfLabels,
                XmNlabelString, labelStr,
                NULL);

    XmStringFree(labelStr);
    _list = new UndelMsgScrollingList( this->parent(), form1, "Message_List");

    SdmError error;
    SdmMailRc * mailrc;
    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailrc);
    char * value;
    int msgnums = False;

    mailrc->GetValue(error, "showmsgnum", &value);
    if (!error) {
        msgnums = True;
    }

    // Adjust labels so the align on the columns
    _list->layoutLabels(msgnums, l1, l2, l3, l4);


    rowOfButtons = XtCreateManagedWidget("RowColumn",
					xmFormWidgetClass,
					form1,
					NULL, 0);

    XtVaSetValues(XtParent(_list->get_scrolling_list()),
		XmNrightAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, rowOfLabels,
		XmNtopOffset, 3,
		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget, rowOfButtons,
		XmNbottomOffset, 7,
		NULL);

    XtVaSetValues(rowOfButtons,
		XmNorientation, XmHORIZONTAL,
		XmNpaneMaximum, size.height,
		XmNpaneMinimum, size.height,
		XmNrightAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
	        XmNfractionBase, 100,
		XmNresizable, FALSE,
		NULL);
    
    // Create undelete and close buttons.
    this->addToRowOfButtons();



    // Label should go here that says in how many days
    // deleted messages will be destroyed, and for status.
    rowOfMessageStatus = XtCreateManagedWidget("Message_Status",
				xmFormWidgetClass,
				form1, NULL, 0);
    XtVaSetValues(rowOfMessageStatus,
		XmNrightAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
		XmNtopOffset, 5,
		NULL);


    XtManageChild(_list->baseWidget());
    XtManageChild(form1);

    // return the widget
    return(form1);
}
    
void
UndelFromListDialog::insertMsg(SdmError &mail_error, MsgStruct *tmpMS)
{
    _list->insertMsg(mail_error, tmpMS);
}

void
UndelFromListDialog::updateListItems (SdmError &mail_error, int current)
{
  if (_list) {
    _list->updateListItems(mail_error, current);
  }
}

void 
UndelFromListDialog::checkDisplayProp(void)
{
  if (_list) {
    _list->checkDisplayProp();
  }
}

void
UndelFromListDialog::undelLast()
{
    int len;
    MsgHndArray *msgs = _list->get_messages();

    len = msgs->length();
    msgs->mark_for_delete(len - 1);
    msgs->compact(0);
    XmListDeletePos(_list->get_scrolling_list(), len);
}

void
UndelFromListDialog::undelSelected()
{
    FORCE_SEGV_DECL(int, position_list);
    int position_count;
    Boolean any_selected = FALSE;
    DtMailGenDialog *genDialog = theRoamApp.genDialog();

    any_selected = XmListGetSelectedPos(_list->get_scrolling_list(),
					&position_list,
					&position_count);

    // Don't do anything if there aren't any messages selected.

    if ((!any_selected) || (position_count <= 0))
      return;

    // Put the selected messages from the Deleted Messages List
    // into the RoamMenuWindow Message Scrolling List.

    // This is kind of tricky, because we have to get the list of
    // selected messages before we delete the selected messages,
    // but we have to delete the selected messages before we
    // pass them to undelete_messages().  The reason for this
    // is that deleteSelected() changes the is_deleted attribute
    // of the message handle to TRUE, which will undo undelete_messages()
    // setting is_deleted to FALSE!  Did you follow all that?

    SdmMessageNumberL msgNumberList;
    _list->selected_msg_hndls(msgNumberList);

    // Remove the selected messages from the Deleted Messages Dialog.
    _list->deleteSelected(Sdm_True);

    // Give the MsgScrollingList object a list of messages to undelete.
    MsgScrollingList *msl = parent()->list();
   
    SdmError mail_error;
    msl->undelete_messages(mail_error, msgNumberList);
    if (mail_error) {
      genDialog->post_error(mail_error, DTMAILHELPCANNOTUNDELETEMSGS,
			    catgets(DT_catd, 3, 130, "Mailer cannot undelete the messages you marked for deletion."));
      return;
    }

    free(position_list);
}

void
UndelFromListDialog::addToRowOfButtons()
{
    FORCE_SEGV_DECL(CmdInterface, ci);
    Widget w, prev_widget;

    _undelete_button = new DoUndeleteCmd("Undelete", 
	                        catgets(DT_catd, 1, 115, "Undelete"), 
	                        Sdm_True, 
	                        this);
    ci = new ButtonInterface (rowOfButtons, _undelete_button);

    w = ci->baseWidget();
    XtVaSetValues(w,
        XmNleftAttachment, XmATTACH_POSITION,
	XmNleftPosition, 35,          // centralized tUndelete and Close buttons
        XmNbottomAttachment, XmATTACH_FORM,
        NULL );
    ci->manage();
    prev_widget = w;

    _close_button = new CloseUndelCmd(
				"Close",
				catgets(DT_catd, 1, 116, "Close"), 
				TRUE, 
				this);
    ci = new ButtonInterface (rowOfButtons, _close_button);
    w = ci->baseWidget();
    XtVaSetValues(w,
        XmNleftAttachment, XmATTACH_WIDGET,
        XmNleftWidget, prev_widget,
        XmNleftOffset, 30,
        XmNbottomAttachment, XmATTACH_FORM,
        NULL );
    ci->manage();

}
void
UndelFromListDialog::expunge(void)
{
    MsgHndArray *msgs = _list->get_messages();

    for (int pos = 0; pos < _list->get_num_messages(); pos++) {
	msgs->mark_for_delete(pos);
    }

    msgs->compact(0);

    XmListDeleteAllItems(_list->get_scrolling_list());
}
