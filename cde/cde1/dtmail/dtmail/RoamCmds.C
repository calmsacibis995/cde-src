/*
 *+SNOTICE
 *
 *	$Revision: 1.11 $
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
 *	Copyright 1993, 1994, 1995 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)RoamCmds.C	1.170 30 May 1995"
#endif

#ifdef __ppc
#include <DtMail/Buffer.hh>
#endif
#include <stdio.h>
#include <EUSCompat.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>
#include <Xm/Text.h>
#include <Xm/FileSBP.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/PanedW.h>
#include <Xm/Form.h>
#include <Dt/Dts.h>
#include <Dt/Action.h>
#include <DtMail/IO.hh>
#include "RoamMenuWindow.h"
#include "SendMsgDialog.h"
#include "Undelete.hh"
#include "RoamCmds.h"
#include "Application.h"
#include "RoamApp.h"
#include "DtMailWDM.hh"
#include "FindDialog.h"
#include "MsgScrollingList.hh"
#include "MsgHndArray.hh"
#include "MemUtils.hh"
#include "MailMsg.h"
#include "EUSDebug.hh"
#include "DtMailGenDialog.hh"
#include <DtMail/DtMailError.hh>
#include <DtMail/DtMailP.hh>
#ifndef __ppc
#include <DtMail/Buffer.hh>
#endif
#include "DtMailHelp.hh"
#include "Help.hh"
#include <Dt/Help.h>
#include "Attachment.h"
#include "AttachArea.h"
#include "DtMailEditor.hh"
#include <DtMail/OptCmd.h>
#include "TemplateDialog.h"
#include "DraftDialog.h"
#include <DtMail/ListUiItem.hh>
#include <DtMail/TemplateListUiItem.hh>

#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/systeminfo.h>
#include <sys/types.h>

#if defined(NEED_MMAP_WRAPPER)
extern "C" {
#endif
#include <sys/mman.h>
#if defined(NEED_MMAP_WRAPPER)
}
#endif

#include <sys/file.h>

extern forceUpdate( Widget );
extern Boolean props_changed;

RoamCmd::RoamCmd
(char *name, char *label,  int active, RoamMenuWindow *window)
: NoUndoCmd (name, label, active)
{
    _menuwindow = window;
}

#ifdef DEAD_WOOD
SearchCmd::SearchCmd(
		     char *name, 
		     char *label,
		     int active, 
		     RoamMenuWindow *window
		     ) : InterruptibleCmd (name, label, active)
{
    _menuwindow = window;
    _criteria = NULL;
}

void
SearchCmd::execute ( 
		     TaskDoneCallback callback, 
		     void *clientData 
		     )
{
    InterruptibleCmd::execute( callback, clientData );
}

void
SearchCmd::execute()
{
    
    _menuwindow->list()->clearMsgs();
    _menuwindow->busyCursor();
    
    if ( !_criteria ) {
	_criteria=( char * )realloc(_criteria, strlen( this->name()) + 1);
	strcpy(_criteria, this->name());
    }
    
    InterruptibleCmd::execute();
}

void
SearchCmd::doit()
{
    int 	count;
    DtMailEnv	mail_error;
    
    // Initialize the mail_error.
    mail_error.clear();
    
    MsgScrollingList *list=_menuwindow->list();
    
    
    // load_headers will retrieve all of the message headers and
    // add the handles to the list.
    //
    count = list->load_headers(mail_error);
    
    _menuwindow->normalCursor();
    
    if (count == 0) {
	_menuwindow->message(catgets(DT_catd, 3, 46, "Empty container"));
	_done = TRUE;
	return;
    }
    
    list->scroll_to_bottom();
    _done=TRUE;
}      

void
SearchCmd::undoit()
{
    // Just print a message that allows us to trace the execution
    
    DebugPrintf(1, "%s: undoit\n", name());
}       

void
SearchCmd::updateMessage (char *msg)
{
    InterruptibleCmd::updateMessage(msg);
}
#endif /* DEAD_WOOD */

CheckForNewMailCmd::CheckForNewMailCmd( 
					char *name, 
					char *label,
					int active, 
					RoamMenuWindow *window
					) :  NoUndoCmd ( name, label, active )
{
    _menuwindow = window;
}

void
CheckForNewMailCmd::doit()
{
    DtMailEnv error;
    // Initialize the mail_error.
    error.clear();
    _menuwindow->checkForMail(error);
    
}

OpenMailboxCmd::OpenMailboxCmd(
    char *name,
    char *label,
    int active,
    RoamMenuWindow *rmw,
    char *filename
) : RoamCmd (name, label, active, rmw)
{
    if (filename[0] != '/') { 
    	DtMail::Session * d_session = theRoamApp.session()->session();
    	DtMailEnv error;
	char *expanded_name = d_session->expandPath(error, filename);
	if (expanded_name[0] != '/') { 
		char *full_dirname = theRoamApp.getFolderDir(TRUE);
		char *dirname = (char*)malloc(strlen(full_dirname) + strlen(expanded_name) +2);    
		sprintf(dirname, "%s/%s", full_dirname, expanded_name);
		free(expanded_name); 
		free(full_dirname);
		_container_name = dirname;
        }
	else
		_container_name = expanded_name;	
    }
    else
    	_container_name = filename;
    _menuwindow = rmw;
}

void
OpenMailboxCmd::doit()
{
    char * buf = NULL;
    DtMailObjectSpace space;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMailEnv error;
    RoamMenuWindow *rmw = NULL;

    d_session->queryImpl(error,
                         d_session->getDefaultImpl(error),
                         DtMailCapabilityInboxName,
                         &space,
                         &buf);

    if (strcmp(buf, _container_name) == 0) {
    	rmw = theRoamApp.inboxWindow();
    	if (rmw) 
		rmw->manage();
    }
    if (!rmw)
	_menuwindow->mail_file_selection(_container_name);
}

// 
// OpenContainerCmd methods implementation.
// For the most part, we treat container->open() as a benign thing.
//

OpenContainerCmd::OpenContainerCmd (
				    char *name, 
				    char *label,
				    int active, 
				    RoamMenuWindow *window
				) : RoamInterruptibleCmd (name, label, active)
{
    _menuWindow = window;
}

// Parent's execute() ends up calling derived class's doit()
void
OpenContainerCmd::execute()
{
    RoamInterruptibleCmd::execute();
}

void
OpenContainerCmd::execute( 
			   RoamTaskDoneCallback rtd_callback, 
			   void *clientData
			   )
{
    RoamInterruptibleCmd::execute(rtd_callback, clientData);
}

// Tell the RMW to open.  The RMW may end up calling its convert()
// which depends on ConvertContainerCmd's doit...
// By the time RMW->open() returns, the conversion would have
// been done too.
// This is the case of a RinterruptibleCmd derived class
// getting its work done in its doit() in one call.
//

void
OpenContainerCmd::doit()
{
    DtMailEnv error;
    
    assert(_menuWindow);
    
    // Initialize the mail_error.
    error.clear();
    
    _menuWindow->open(error, _open_create_flag, _open_lock_flag);
    if (error.isSet()) {
        // Post a dialog indicating error and exit?
        return;         // for now. Should exit instead?
    }
    _done = TRUE;
}      

void
OpenContainerCmd::undoit()
{
    // Just print a message that allows us to trace the execution
    
    DebugPrintf(1, "%s: undoit\n", name());
}       

void
OpenContainerCmd::check_if_done()
{
    // Have nothing fancy to do here.  Since we do not want a dialog
    // in any case, set it to true...
    
    _done = TRUE;
    
}


void
OpenContainerCmd::post_dialog()
{
    // Empty.  We don't want a dialog on open...
}

void
OpenContainerCmd::unpost_dialog()
{
    // Empty.  We don't post, and we don't unpost.
}
void
OpenContainerCmd::updateMessage (char *msg)
{
    RoamInterruptibleCmd::updateMessage(msg);
}

void
OpenContainerCmd::set_create_lock_flags(
					DtMailBoolean create,
					DtMailBoolean lock
					)
{
    _open_create_flag = create;
    _open_lock_flag   = lock;
}


// ConvertContainerCmd
// Here be dragons!

#ifdef DEAD_WOOD
ConvertContainerCmd::ConvertContainerCmd(
					 char *name, 
					 char *label,
					 int active, 
					 RoamMenuWindow *window
				 ) : RoamInterruptibleCmd (name, label, active)
{
    _menuWindow = window;
    _num_converted = 0;
    _num_to_be_converted = 0;
    _dialog = NULL;
}

void
ConvertContainerCmd::execute()
{
    if (!_dialog) {
	_dialog = new DtMailWDM("Convert");
    }
    
    RoamInterruptibleCmd::execute();
}

void
ConvertContainerCmd::execute ( RoamTaskDoneCallback rtd_callback, 
			       void *clientData)
{
    if (!_dialog) {
	_dialog = new DtMailWDM("Convert");
    }
    
    RoamInterruptibleCmd::execute(rtd_callback, clientData);
}

// Here be bigger dragons!
// The doit() calls the session->convert().
// And returns right away!
// the ses->convert() however ends up calling the conv_cb for every
// message that it has converted.
// So, we now have two loops working in parallel:
// 1) the parent()'s execute() class which called this doit() and is now
//    calling check_if_done() periodically via its workProc;
// 2) the session->convert() which is calling the _conv_cb() for every
//    message that it converts.  In the _conv_cb(), we do the following:
//    a) force update the dialog and see if it was interrupted;
//    b) if not interrupted, set_convert_data() where we set _done if 
//       we are really done.
//  

void
ConvertContainerCmd::doit()
{
    assert(_menuWindow);
    
    MailSession *ses = theRoamApp.session();
    DtMailEnv mail_error;
    
    // Initialize the mail_error.
    mail_error.clear();
    
//    ses->convert(mail_error, _src, _dest, _conv_cb, _menuWindow);
    
    if (mail_error.isSet()) {
	_menuWindow->postErrorDialog(mail_error);
    }
}      

void
ConvertContainerCmd::set_convert_data(
				      int converted,
				      int to_be_converted
				      )
{
    _num_converted = converted;
    _num_to_be_converted = to_be_converted;
    
    if ((_num_converted == _num_to_be_converted) && !_interrupted) {
	_done = TRUE;
    }
}

int
ConvertContainerCmd::get_num_converted()
{
    return(_num_converted);
}


void
ConvertContainerCmd::check_if_done()
{
    if (_interrupted) {
	_done = FALSE;
    }
    else if (_num_converted == _num_to_be_converted) {
	_done = TRUE;
    }
}

void
ConvertContainerCmd::updateDialog(
				  char *msg
				  )
{
    forceUpdate(_dialog->baseWidget());
    _dialog->updateDialog( msg );  
}

void
ConvertContainerCmd::updateAnimation()
{
    forceUpdate(_dialog->baseWidget());
    _dialog->updateAnimation();
}

void
ConvertContainerCmd::post_dialog()
{
    Dimension x, y, wid, ht;
    
    char * buf = new char[25];
    
    sprintf(buf, "Converted: %3d%", 0);
    
    _dialog->post ("Mailer",
		   buf,
		   _menuWindow->baseWidget(),
		   (void *) this,
		   NULL, 
		   &RoamInterruptibleCmd::interruptCallback );
    
    XtVaGetValues(_dialog->baseWidget(),
		  XmNx, &x,
		  XmNy, &y,
		  XmNwidth, &wid,
		  XmNheight, &ht,
		  NULL);
    
}

void
ConvertContainerCmd::unpost_dialog()
{
    _dialog->unpost();
}

void
ConvertContainerCmd::undoit()
{
    // Just print a message that allows us to trace the execution
    
    DebugPrintf(1, "%s: undoit\n", name());
}       

void
ConvertContainerCmd::set_data(
			      char *path1,
			      char *path2,
			      ConversionStatusCB cb
			      )
{
    _src = path1;
    _dest = path2;
    _conv_cb = cb;
}

char *
ConvertContainerCmd::get_destination_name()
{
    return strdup(_dest);
}

#endif // DEAD_WOOD

// Here be sheep!
// FindCmd

FindCmd::FindCmd( 
		  char *name, 
		  char *label,
		  int active,
		  RoamMenuWindow *window 
		  ) : RoamCmd ( name, label, active, window )
{
}

void
FindCmd::doit()
{
    
    _menuwindow->get_find_dialog();
    
//  SearchCmd::doit();
    
}

OptionsCmd::OptionsCmd( 
		      char *name, 
		      char *label,
		      int active, 
		      OptionsOp op
		      ) :  NoUndoCmd( name, label, active )
{
    _op = op;
}

void
OptionsCmd::doit()
{
    DtbOptionsDialogInfo od;
    OptCmd *oc=NULL;
    int ret = 0;

    if (!(od = (DtbOptionsDialogInfo) theRoamApp.optionsDialog())) {
        // Create the Options Dialog
        oc = (OptCmd*)theRoamApp.mailOptions();
        if (oc) {
                oc->execute();
                od = (DtbOptionsDialogInfo) theRoamApp.optionsDialog();
        }
        else
                return;
    }
    // Now set the category...
    if (od) {
        if (!oc)
                oc = (OptCmd*)theRoamApp.mailOptions();
	
        switch (_op) {
	  case OPT_BASIC: 
        	if ((ret = options_set_category_pane(od->msg_header_pane)) != 2)
        		oc->setWithCategoryStr(catgets(DT_catd, 1, 272, "Basic"));
		
		break;
	  case OPT_TB:
                if ((ret = options_set_category_pane(od->toolbar_pane)) != 2)
                	oc->setWithCategoryStr(catgets(DT_catd, 1, 273, "Toolbar"));
		break;
	  case OPT_MV:
                if ((ret = options_set_category_pane(od->msg_view_pane)) != 2)
                	oc->setWithCategoryStr(catgets(DT_catd, 1, 274, "Message View"));
		break;
	  case OPT_MF:
                if ((ret = options_set_category_pane(od->msg_filing_pane)) != 2)
                	oc->setWithCategoryStr(catgets(DT_catd, 1, 275, "Move Menu Setup"));
		break;
// AUTOFILING change this back to Filing and Notification when we put back the changes
	  case OPT_NO:
                if ((ret = options_set_category_pane(od->notification_pane)) !=2)
                	oc->setWithCategoryStr(catgets(DT_catd, 1, 282, "Notification"));
		break;
	  case OPT_CW:
                if ((ret = options_set_category_pane(od->compose_win_pane)) != 2)
                	oc->setWithCategoryStr(catgets(DT_catd, 1, 276, "Compose Window"));
		break;
	  case OPT_SIG:
                if ((ret = options_set_category_pane(od->signature_pane)) != 2)
                	oc->setWithCategoryStr(catgets(DT_catd, 1, 277, "Signature"));
		break;
	  case OPT_VAC:
                if ((ret = options_set_category_pane(od->vacation_pane)) != 2)
                	oc->setWithCategoryStr(catgets(DT_catd, 1, 278, "Vacation Message"));
		break;
	  case OPT_TD:
                if ((ret = options_set_category_pane(od->templates_pane)) != 2)
                	oc->setWithCategoryStr(catgets(DT_catd, 1, 279, "Templates"));
		break;
	  case OPT_AL:
                if ((ret = options_set_category_pane(od->aliases_pane)) != 2)
                	oc->setWithCategoryStr(catgets(DT_catd, 1, 280, "Aliases"));
		break;
	  case OPT_AD:
                if ((ret = options_set_category_pane(od->advanced_pane)) != 2)
                	oc->setWithCategoryStr(catgets(DT_catd, 1, 281, "Advanced"));
		break;
	  default: 
        	if ((ret = options_set_category_pane(od->msg_header_pane)) != 2)
        		oc->setWithCategoryStr(catgets(DT_catd, 1, 272, "Basic"));
        }

   	if (ret != 2 && oc) {
		oc->update_pane(oc->currentPane());
		oc->setWindowAlive(TRUE);
	}
        XtManageChild(od->dialog_shellform);
        XtPopup(od->dialog, XtGrabNone);
	props_changed = FALSE;
    }
}

ChooseCmd::ChooseCmd( 
		      char *name, 
		      char *label,
		      int active, 
		      RoamMenuWindow *window
		      ) :  NoUndoCmd( name, label, active )
{
    _menuwindow = window;
}

void
ChooseCmd::doit()
{
    
}

SelectAllCmd::SelectAllCmd(
			    char *name,
			    char *label,
			    int active,
			    RoamMenuWindow *window
			    ) : Cmd ( name, label, active )
{
    _menuwindow = window;
}

void
SelectAllCmd::doit()
{
    DtMailEnv mail_error;

    _menuwindow->list()->select_all_and_display_last(mail_error);
}

void
SelectAllCmd::undoit()
{
}


DeleteCmd::DeleteCmd( 
		      char *name, 
		      char *label,
		      int active, 
		      RoamMenuWindow *window
		      ) :  Cmd ( name, label, active )
{
    _menuwindow = window;
}

void
DeleteCmd::doit()
{
    _menuwindow->list()->deleteSelected(FALSE);
    
}

void
DeleteCmd::undoit()
{
    
}

DestroyCmd::DestroyCmd(
			char *name, 
			char *label, 
			int active, 
			RoamMenuWindow *window
			) : Cmd(name, label, active)
{
    _menuwindow = window;
}

void
DestroyCmd::doit()
{
    // Call Expunge only if there are deleted messages.

    if (_menuwindow->list()->get_num_deleted_messages()) {
	_menuwindow->expunge();
    }
}

void
DestroyCmd::undoit()
{
}


MoveCopyMenuCmd::MoveCopyMenuCmd(
    char *name,
    char *label,
    int active,
    RoamMenuWindow *window,
    MoveCopyOpenOp op
) : RoamCmd ( name, label, active, window )
{
    _menuwindow = window;
    _container_name = name;
    _operation = op;
}

void
MoveCopyMenuCmd::doit()
{
    DtMailEnv mail_error;

    // Initialize mail_error.
    mail_error.clear();

    if (_operation == DTM_MOVE) {
	_menuwindow->list()->copySelected(mail_error, _container_name, TRUE, FALSE);
	if (mail_error.isSet()) {
	    // We had an error in moving the message to a container!
	}
    } else {
	_menuwindow->list()->copySelected(mail_error, _container_name, FALSE, FALSE);
	if (mail_error.isSet()) {
	    // We had an error in copying the message to a container!
	}
    }
}

MoveCopyMenuCmd::~MoveCopyMenuCmd()
{
}

    
// Move the messages that are selected in the RoamMenuWindow to the Inbox.
MoveToInboxCmd::MoveToInboxCmd(
			       char *name,
			       char *label,
			       int active,
			       RoamMenuWindow *window
			       ) : RoamCmd (name, label, active, window)
{
    _menuwindow = window;
}

void
MoveToInboxCmd::doit()
{
    DtMailEnv mail_error;
    
    // Initialize mail_error.
    mail_error.clear();
    
    // Get a handle to the Inbox.
    char * mail_file = NULL;
    DtMailObjectSpace space;
    DtMail::Session * d_session = theRoamApp.session()->session();
    
    d_session->queryImpl(mail_error,
			 d_session->getDefaultImpl(mail_error),
			 DtMailCapabilityInboxName,
			 &space,
			 &mail_file);
    _menuwindow->list()->copySelected(mail_error, mail_file, TRUE, TRUE);
    if (mail_error.isSet()) {
	// We had an error in moving the messages to the Inbox!
    }
}

MoveToInboxCmd::~MoveToInboxCmd()
{
}

// Copy the selected messages to the Inbox.
CopyToInboxCmd::CopyToInboxCmd(
                             char *name,
			     char *label,
                             int active,
                             RoamMenuWindow *window
                             ) : RoamCmd (name, label, active, window)
{
    _menuwindow = window;
}
 
void
CopyToInboxCmd::doit()
{
    DtMailEnv mail_error;
 
    // Initialize mail_error.
    mail_error.clear();
 
    // Get a handle to the Inbox.
    char * mail_file = NULL;
    DtMailObjectSpace space;
    DtMail::Session * d_session = theRoamApp.session()->session();
 
    d_session->queryImpl(mail_error,
                         d_session->getDefaultImpl(mail_error),
                         DtMailCapabilityInboxName,
                         &space,
                         &mail_file);
    _menuwindow->list()->copySelected(mail_error, mail_file, FALSE, TRUE);
    if (mail_error.isSet()) {
	// We ad an error in copying the messages to the Inbox!
    }
}

CopyToInboxCmd::~CopyToInboxCmd()
{
}


// This is hooked up the Undelete button in the Deleted
// Messages List dialog box.

DoUndeleteCmd::DoUndeleteCmd( 
			      char *name, 
			      char *label,
			      int active, 
			      UndelFromListDialog *undelDialog
			      ) :  Cmd ( name, label, active )
{
    _undelDialog = undelDialog;
}

void
DoUndeleteCmd::doit()
{
    // Undelete the selected messages.
    _undelDialog->undelSelected();
}

void
DoUndeleteCmd::undoit()
{
    // nothing
}

DoUndeleteCmd::~DoUndeleteCmd()
{
}

// This is hooked up to the Close button in the Deleted Messages
// List dialog box.

CloseUndelCmd::CloseUndelCmd( 
			      char *name, 
			      char *label,
			      int active,
			      UndelFromListDialog *undelDialog
			      ) :  Cmd ( name, label, active )
{
    _undelDialog = undelDialog;
}

void
CloseUndelCmd::doit()
{
    // Close the dialog.
    _undelDialog->popped_down();
}

void
CloseUndelCmd::undoit()
{
    // nothing
}


CloseUndelCmd::~CloseUndelCmd()
{
}

UndeleteCmd::UndeleteCmd ( 
			   char *name, 
			   char *label,
			   int active, 
			   RoamMenuWindow *window,
			   Boolean viaDeleteList
			   ) : ChooseCmd  ( name, label, active, window )
{
    _menuwindow = window;
    _undelFromList = NULL;
    _fromList = viaDeleteList;
}

UndeleteCmd::~UndeleteCmd()
{
}

void
UndeleteCmd::doit()
{
    FORCE_SEGV_DECL(MsgStruct, tmpMS);
    MsgScrollingList *list = _menuwindow->list();
    MsgHndArray *deleted_messages;
    DtMailEnv mail_error;
    
    // Initialize the mail_error.
    mail_error.clear();
    
    
    if (_fromList) {
	// Create the Deleted Messages Dialog
	
	if (_undelFromList) {
	    // Hack for user testing.  If the dialog is up, we destroy it.
	    XtDestroyWidget(_undelFromList->baseWidget());
	}
//	if (!_undelFromList) {
	_undelFromList = new UndelFromListDialog(
	                 catgets(DT_catd, 1, 227, "Mailer - Deleted Messages"), 
	                 _menuwindow);	
	_undelFromList->initialize();
	
	// Check for existing list of deleted messages
	_num_deleted = list->get_num_deleted_messages();
	
	// If there are deleted messages, put them in the Deleted
	// Messages List.
	
	if (_num_deleted > 0) {
	    deleted_messages = list->get_deleted_messages();
	    _undelFromList->loadMsgs(
				mail_error, 
				deleted_messages, 
				_num_deleted);
	    if (mail_error.isSet()) {
		// Post an exception here!
		_menuwindow->postErrorDialog(mail_error);
	    }
		
	}
	// Display the dialog
	
	_undelFromList->popped_up();
    } else {
	// Although we don't display the Deleted Message Dialog here, we
	// need to make sure that it gets updated for the next time
	// we bring it up.
	list->undelete_last_deleted();
    }
}


#ifdef DEAD_WOOD
SaveCmd::SaveCmd ( char *name, 
		   char *label, 
		   int active, 
		   RoamMenuWindow *window 
		) : RoamCmd ( name, label, active, window )
{
    
}

void
SaveCmd::doit()
{
    
    assert(_menuwindow->mailbox());
}
#endif /* DEAD_WOOD */



MoveCopyCmd::MoveCopyCmd( 
			  char *name, 
			  char *label,
			  Boolean copy,
			  int active, 
			  FileCallback move_callback, 
			  FileCallback copy_callback, 
			  RoamMenuWindow * menu_window,
			  Widget parent
			  )
: SelectFileCmd (
		 name,
		 label,
		 catgets(DT_catd, 1, 254, "Mailer - Select Mailboxes"),
		 "Move",
		 active, 
		 move_callback, 
		 menu_window,
		 parent)
{
    _copy_is_default = copy;
    _copy_callback = copy_callback;
    _menuwindow = menu_window;
}

MoveCopyCmd::~MoveCopyCmd()
{
}

void
MoveCopyCmd::doit()
{
    XmString move;
    Widget filter_button;
    Widget ok_button;
    Widget copy_button, unused_button;
    Widget action_area;
    DtMailEnv error;
    char *full_dirname;
    
    full_dirname = theRoamApp.getFolderDir(TRUE);

    XmString path = XmStringCreateLocalized(full_dirname);
    free(full_dirname);
    if (!_fileBrowser) {
	SelectFileCmd::doit();
	// Customize buttons for MoveCopy dialog
	move = XmStringCreateLocalized(catgets(DT_catd, 1, 90, "Move"));
	
	filter_button = 
	    XmFileSelectionBoxGetChild(_fileBrowser, XmDIALOG_APPLY_BUTTON);
	ok_button =
	    XmFileSelectionBoxGetChild(_fileBrowser, XmDIALOG_OK_BUTTON);
	action_area = XtParent(ok_button);
	unused_button = XtVaCreateWidget("Unused Button",
					 xmPushButtonWidgetClass,	_fileBrowser,
					 NULL);
	copy_button = XtVaCreateManagedWidget(catgets(DT_catd, 1, 237, "Copy"),
					      xmPushButtonWidgetClass,	_fileBrowser,
					      NULL);
	printHelpId("Copy", copy_button);
	/* add help callback */
	XtAddCallback(copy_button, XmNhelpCallback, 
		      HelpCB, "dtmailViewmainWindowWork-AreapanedWform2RowColumnMoveCopy");
	XtAddCallback( copy_button,
		       XmNactivateCallback,
		       &MoveCopyCmd::fileSelectedCallback2,
		       (XtPointer) this );

	XtVaSetValues(_fileBrowser,
		      XmNokLabelString,	move,
		      XmNdirectory, path,
		      NULL);

	if (_copy_is_default) {
	    XtVaSetValues(_fileBrowser,
			  XmNdefaultButton, copy_button,
			  NULL);
	}
	
	if (_menuwindow->mailbox()->mailBoxWritable(error) == DTM_FALSE) {
	    XtVaSetValues(ok_button, XmNsensitive, FALSE);
	}

	XmStringFree(move);

    } else {
	SelectFileCmd::doit();
    }
    XmStringFree(path);
}

void
MoveCopyCmd::fileSelectedCallback2 ( 
				     Widget	,
				     XtPointer	clientData,
				     XtPointer	callData
				     )
{
    MoveCopyCmd * obj = (MoveCopyCmd *) clientData;
    XmFileSelectionBoxCallbackStruct *cb =
        (XmFileSelectionBoxCallbackStruct *) callData;
    char     *name   = NULL;
    char     *dir_str    = NULL;
    char     *fname  = NULL;
    XmString  xmstr;  // The selected file
    int       status = 0;
    
    // Get the value
    XtVaGetValues(obj->_fileBrowser, XmNdirSpec, &xmstr, NULL);
    if ( xmstr )   // Make sure a file was selected
      {
	  // Extract the first character string matching the default
	  // character set from the compound string
	  
	  status = XmStringGetLtoR (xmstr,
				    XmSTRING_DEFAULT_CHARSET,
				    &fname );
	  
	  // If a string was successfully extracted, call
	  // fileSelected2 to handle the file.
	  
	  if (status)
	      obj->fileSelected2 (fname);
      }
    XtUnmanageChild ( obj->_fileBrowser );   // Bring the file selection dialog down.
}

void
MoveCopyCmd::fileSelected2 (char *filename )
{
    if ( _copy_callback )
	_copy_callback ( _clientData, filename );
}


NextCmd::NextCmd( 
		  char *name, 
		  char *label,
		  int active, 
		  RoamMenuWindow *window 
		  ) : RoamCmd ( name, label, active, window )
{
}

void
NextCmd::doit()
{
    _menuwindow->list()->select_next_item();
}


PrevCmd::PrevCmd( 
		  char *name, 
		  char *label,
		  int active, 
		  RoamMenuWindow *window 
		  ) : RoamCmd ( name, label, active, window )
{
}

void
PrevCmd::doit()
{
    _menuwindow->list()->select_prev_item();
}

PrintCmd::PrintCmd ( 
		     char *name, 
		     char *label,
		     int active, 
		     int silent,
		     RoamMenuWindow *window 
		 ) : ChooseCmd ( name, label, active, window ), _tmp_files(5)
{
    _parent = window;
    _silent = silent;
}

void
PrintCmd::doit()
{
    // The entire implementation of print was broken. It has
    // be removed until a proper implementation can be provided.
    // dlp 10/04/93
    
    printit(_silent);
    return;
}

void
PrintCmd::actioncb(
		   DtActionInvocationID id,
		   XtPointer     clientData,
		   DtActionArg *,
		   int,
		   int           status
		   )
{
    PrintCmd	*data;
    
    switch (status) {
      case DtACTION_INVOKED:
	break;
      default:
	data = (PrintCmd *)clientData;
	data->_parent->message("");
	data->_unregister_tmp_file(id);
	break;
    }
    
    return;
}

void
PrintCmd::printit(
		  int silent
		  )
{
    char *env_str;
    char *silent_str = "DTPRINTSILENT";
    char *p;
    static char env_buf[32];	// Must be static for putenv()
    DtActionInvocationID	id;
    DtMailEnv	mail_error;
    MsgScrollingList	*list;
    DtActionArg	arg;
    
    DebugPrintf(1, "%s: printit\n", name());
    
    // Create tmp file.
    if ((p = tempnam("/tmp", "mail")) == NULL) {
	return;
    }
    
    mail_error.clear();
    list = _parent->list();
    
    // Copy selected messages to a temp file
    list->copySelected(mail_error, p, FALSE, TRUE);
    if (mail_error.isSet()) {
	_parent->postErrorDialog(mail_error);
	free(p);
	return;
    }
    
    _parent->message(catgets(DT_catd, 1, 91, "Printing..."));
    
    // Assume dtlp eventually gets invoked by the print action and use
    // the DTPRINTSILENT environment variable to control whether or not
    // the dtlp GUI pops up.  The default print action uses dtlp and
    // if it doesn't this won't hurt anything anyway.
    env_str = getenv(silent_str);
    if (silent) {
	sprintf(env_buf, "%s=%s", silent_str, "True");
    } else {
	sprintf(env_buf, "%s=%s", silent_str, "False");
    }
    putenv(env_buf);
    
    arg.argClass = DtACTION_FILE;
    arg.u.file.name = p;
    id = DtActionInvoke(
			_parent->baseWidget(),
			"Print",
			&arg,
			1,
			NULL,NULL,NULL,
			1,
			(DtActionCallbackProc)&PrintCmd::actioncb,
			this
			);
    
    // Invoke Print action
    
    // Restore environment
    if (env_str != NULL) {
	sprintf(env_buf, "%s=%s", silent_str, env_str);
    } else {
	// Default value is False
	sprintf(env_buf, "%s=False", silent_str);
    }
    putenv(env_buf);
    
    // Add tmp file to list of tmp files.  Later when the action is complete
    // we will unlink it and remove it from this list.
    _register_tmp_file(p, id);
    free(p);
    return;
}

int
PrintCmd::_register_tmp_file(
			     const char		*name,
			     DtActionInvocationID	id
			     )
{
    struct tmp_file	*f;
    
    // Allocate struct to hold id and temp file
    if ((f = (struct tmp_file *)malloc(sizeof(struct tmp_file))) == NULL) {
	return -1;
    }
    
    // Save file name and action id
    f->file = strdup(name);
    f->id = id;
    
    // Add to list of temp files
    _tmp_files.append(f);
    
    return 0;
}

void
PrintCmd::_unregister_tmp_file(
			       DtActionInvocationID	id
			       )
{
    int n;
    struct tmp_file *f;
    
    // Find the temp file that was used by the Action specified by id
    for (n = _tmp_files.length() - 1; n >= 0; n--) {
	f = _tmp_files[n];
	if (f->id == id) {
	    // Found the file.  Unlink and free data structs
	    unlink(f->file);
	    free(f->file);
	    free(f);
	    // Remove entry from list
	    _tmp_files.remove(n);
	    break;
	}
    }
    
    return;
}

#ifdef DEAD_WOOD
PopupCmd::PopupCmd ( 
		     char *name, 
		     char *label,
		     int active,
		     PopupWindow * (RoamMenuWindow::* member) (void), 
		     RoamMenuWindow *myparent 
		     ) : NoUndoCmd ( name, label, active )
{
    parent=myparent;
    pmpopup=member;
}

void
PopupCmd::doit()
{
    PopupWindow *popup=(parent->*pmpopup)();
//  popup->manage();
}
#endif /* DEAD_WOOD */

// OnItemCmd brings up the Help On Item help.
OnItemCmd::OnItemCmd ( char * name, 
		       char *label, 
		       int active, 
		       UIComponent *window ) 
: NoUndoCmd (name, label, active)
{
    _parent = window;
}

void
OnItemCmd::doit()
{
    int status = DtHELP_SELECT_ERROR;
    Widget widget = _parent->baseWidget();
    Widget selWidget = NULL;
    
    // Display the appropriate help information for the selected item.
    
    status = DtHelpReturnSelectedWidgetId(widget, NULL, &selWidget);
    
    switch ((int) status) {
      case DtHELP_SELECT_ERROR:
	printf("Selection Error, cannot continue\n");
	break;
      case DtHELP_SELECT_VALID:
	while (selWidget != NULL) {
	    if ((XtHasCallbacks(selWidget, XmNhelpCallback)
		 == XtCallbackHasSome)) {
		XtCallCallbacks((Widget) selWidget, XmNhelpCallback, NULL);
		break;
	    } else {
		selWidget = XtParent(selWidget);
	    }
	}
	break;
      case DtHELP_SELECT_ABORT:
	printf("Selection Aborted by user.\n");
	break;
      case DtHELP_SELECT_INVALID:
	printf("You must select a component within your app.\n");
	break;
      default:
	;
	// Empty
    }
    
}

OnAppCmd::OnAppCmd ( char * name, 
		     char *label,
		     int active, 
		     UIComponent *window ) 
: NoUndoCmd (name, label, active)
{
    _parent = window;
}

void
OnAppCmd::doit()
{
    DisplayMain (_parent->baseWidget(), "Mailer", DTMAILWINDOWID);
}

TasksCmd::TasksCmd ( char * name, 
		     char *label,
		     int active, 
		     UIComponent *window )
: NoUndoCmd (name, label, active)
{
    _parent = window;
}

void
TasksCmd::doit()
{
    DisplayMain (_parent->baseWidget(), "Mailer", HELP_MAILER_TASKS);
}

ReferenceCmd::ReferenceCmd ( char * name, 
			     char *label,
			     int active, 
			     UIComponent *window )
: NoUndoCmd (name, label, active)
{
    _parent = window;
}

void
ReferenceCmd::doit()
{
    DisplayMain (_parent->baseWidget(), "Mailer", HELP_MAILER_REFERENCE);
}

MessagesCmd::MessagesCmd ( char * name,
			   char * label,
			   int active,
			   UIComponent *window)
: NoUndoCmd (name, label, active)
{
    _parent = window;
}

void
MessagesCmd::doit()
{
    DisplayMain (_parent->baseWidget(), "Mailer", HELP_MAILER_MESSAGES);
}


UsingHelpCmd::UsingHelpCmd ( char * name, 
			     char *label,
			     int active, 
			     UIComponent *window )
: NoUndoCmd (name, label, active)
{
    _parent = window;
}

void
UsingHelpCmd::doit()
{
    DisplayMain (_parent->baseWidget(), "Help4Help", "_HOMETOPIC");
}

RelNoteCmd::RelNoteCmd ( char * name, 
			 char *label,
			 int active, 
			 UIComponent *window 
		     ) : NoUndoCmd (name, label, active )
{
    _parent = window;
    _genDialog = NULL;
}

void
RelNoteCmd::doit()
{
    if (!_genDialog) {
	_genDialog = new DtMailGenDialog("AboutBox", _parent->baseWidget());
    }
    
    _genDialog->setToAboutDialog();
    
    _genDialog->post_and_return(catgets(DT_catd, 1, 92, "OK"), NULL);
}

RelNoteCmd::~RelNoteCmd()
{
    delete _genDialog;
}

SendCmd::SendCmd(
		 char *name, 
		 char *label,
		 int active, 
		 SendMsgDialog *parent,
		 int trans_type) 
: NoUndoCmd( name, label, active )
{
    _parent=parent;
    _default_trans = trans_type;
}

void
SendCmd::doit()
{
    if (!_parent->isMsgValid())
	return;
    else
	_parent->send_message( this->name(), _default_trans );
}

// JT - Added methods below

OpenMsgCmd::OpenMsgCmd(
		       char *name,
		       char *label,
		       int active,
		       RoamMenuWindow *window) 
: RoamCmd (name, label, active, window)
{
}

void
OpenMsgCmd::doit()
{
    DtMailEnv mail_error;

    // Initialize the mail_error.
    mail_error.clear();

    _menuwindow->list()->viewInSeparateWindow(mail_error);
    if (mail_error.isSet()) {
        _menuwindow->postErrorDialog(mail_error);
    }

}

// Attachment Cmds stuff

SaveAttachCmd::SaveAttachCmd ( 
			       char *name, 
			       char *label,
			       char * title,
			       int active, 
			       FileCallback save_callback,
			       RoamMenuWindow *clientData,
			       Widget parent
			       )
:SelectFileCmd (name,
		label,
		title,
		catgets(DT_catd, 1, 93, "Save"),
		active,
		save_callback,
		clientData,
		parent )
{
   _parent = clientData;
   _name = NULL;
}

SaveAttachCmd::SaveAttachCmd ( 
			       char *name, 
			       char *label,
			       char * title,
			       int active, 
			       FileCallback save_callback,
			       ViewMsgDialog *clientData,
			       Widget parent
			       )
:SelectFileCmd (name,
		label,
		title,
		catgets(DT_catd, 1, 93, "Save"),
		active,
		save_callback,
		clientData,
		parent )
{
   _parent = clientData;
   _name = NULL;
}

SaveAttachCmd::SaveAttachCmd ( 
			       char *name, 
			       char *label,
			       char * title,
			       int active, 
			       FileCallback save_callback,
			       SendMsgDialog *clientData,
			       Widget parent
			       )
:SelectFileCmd (name,
		label,
		title,
		catgets(DT_catd, 1, 93, "Save"),
		active,
		save_callback,
		clientData,
		parent )
{
    _parent = clientData;
    _name = NULL;
}

SaveAttachCmd::~SaveAttachCmd()
{
    if (_name) {
	XmStringFree(_name);
    }
}


void
SaveAttachCmd::doit()
{
    char *full_dirname = theRoamApp.getFolderDir(TRUE);

    XmString path = XmStringCreateLocalized(full_dirname);
    free(full_dirname);

    if (!_fileBrowser) {
	SelectFileCmd::doit();
	
	// Customize buttons for SaveAttach dialog
	XmString save = 
	    XmStringCreateLocalized(catgets(DT_catd, 1, 94, "Save"));
	Widget ok_button =
	    XmFileSelectionBoxGetChild(_fileBrowser, XmDIALOG_OK_BUTTON);

        XtVaSetValues(_fileBrowser, 
			XmNokLabelString, save, 
			XmNdirectory, path,
			NULL);
	XmStringFree(save);
    } else {
	SelectFileCmd::doit();
    }

    DtMailEditor *editor = _parent->get_editor();
    AttachArea *aa = editor->attachArea();
    XmString attachmentName = aa->getSelectedAttachName();
    XtVaSetValues(_fileBrowser, XmNtextString, attachmentName, NULL);
    XtAddCallback ( _fileBrowser, XmNapplyCallback,
		    &SaveAttachCmd::updateCallback,
		    (XtPointer) this);

    if (_name) {
	XmStringFree(_name);
    }

    _name = XmStringCopy(attachmentName);
    XmStringFree(attachmentName);
    XmStringFree(path);
}

// Attachment Cmds stuff


void SaveAttachCmd::updateCallback(Widget, XtPointer clientData, XtPointer )
{
    SaveAttachCmd *obj = (SaveAttachCmd *)clientData;
    
    XtVaSetValues(obj->_fileBrowser, XmNtextString, obj->_name, NULL);
}

SaveAsTextCmd::SaveAsTextCmd ( 
			       char *name, 
			       char *label,
			       char *title,
			       int active, 
			       Editor * editor,
			       SendMsgDialog *smd,
			       Widget parent
			       ) 
:SelectFileCmd (name,
		label,
		title,
		catgets(DT_catd, 1, 95, "Save"),
		active,
		fileCB,
		this,
		parent )
{
    _text_editor = editor;
    _smd = smd;
}

void
SaveAsTextCmd::fileCB(void * client_data, char * selection)
{
    SaveAsTextCmd * self = (SaveAsTextCmd *)client_data;

    self->saveText(selection);
}

void
SaveAsTextCmd::saveText(const char * filename)
{
    int answer;
    char buf[2048];
    DtMailGenDialog *genDialog = theRoamApp.genDialog();

    // Make sure there is a filename specified
    if (!*filename) {
	    genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"), 
			 catgets(DT_catd, 3, 97, "Specify the name of the file to save this message"));
	    answer = genDialog->post_and_return(DTMAILHELPNOFILENAME);
            if (answer == 1)  // Pressed OK, we remap SaveAsText dialog
		doit();
         
	    return;
	}
    

    // Is it already there?
    //
    if (SafeAccess(filename, F_OK) == 0) {
	sprintf(buf, catgets(DT_catd, 3, 47, "%s already exists.\nOverwrite?"),
		filename);
	
	genDialog->setToQuestionDialog(
					catgets(DT_catd, 3, 48, "Mailer"),
					buf);
	answer = genDialog->post_and_return(DTMAILHELPMSGALREADYEXISTS);

	if (answer == 2) { // Pressed cancel
	    return;
	}

	if (unlink(filename) < 0) {
	    sprintf(buf, 
		    catgets(DT_catd, 3, 49, "Unable to overwrite %s.\n\
Check file permissions and retry."), 
		    filename);
	    genDialog->setToErrorDialog(catgets(DT_catd, 3, 50, "Mailer"), buf);
	    genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOOVERWRITE);
	    return;
	}
    }

    // Create or truncate, and then write the bits.
    //
    int fd = SafeOpen(filename, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
	sprintf(buf, catgets(DT_catd, 3, 51, "Unable to create %s."), filename);
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 52, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOCREATE);
	return;
    }
    
    if (SafeWrite(fd, "\n", 1) < 1) {
	sprintf(buf, 
		catgets(DT_catd, 3, 53, "Unable to write to %s."), 
		filename);
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
	SafeClose(fd);
	unlink(filename);
	return;
    }
    
    char * text_buf = _text_editor->get_contents();
	if (text_buf == NULL) {
    	SafeClose(fd);
		return;
	}
    int len = strlen(text_buf);
    if (SafeWrite(fd, text_buf, len) < len) {
	sprintf(buf, catgets(DT_catd, 3, 55, "Unable to create %s."), filename);
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 56, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOCREATE);
	SafeClose(fd);
	unlink(text_buf);
	free(text_buf);
	return;
    }
    
    free(text_buf);
    SafeClose(fd);
    if (_smd != NULL)
    	_smd->setEditsMade(FALSE);
}

void
SaveAsTextCmd::doit()
{
    SelectFileCmd::doit();
}
SaveAsDraftCmd::SaveAsDraftCmd ( 
			       char *name, 
			       char *label,
			       int active, 
			       SendMsgDialog *smd,
			       Widget parent
			       ) 
: NoUndoCmd( name, label, active )
{
    _sendMsgDialog = smd;
    _draftDialog = NULL;
    _parent = parent;
}
void
SaveAsDraftCmd::saveDraft()
{
    char *tmp_buf=NULL;

    if (_draftDialog == NULL) {
        _draftDialog = (DraftModuleDialogInfo)malloc(sizeof(
                               DraftModuleDialogInfoRec));
        DraftDialogInfo_clear(_draftDialog);
        draft_dialog_initialize(_draftDialog, _parent);
        dtb_save_toplevel_widget(_parent);
    	XtVaSetValues(_draftDialog->save_button, XmNuserData, this, NULL);
    	XtVaSetValues(_draftDialog->cancel_button, XmNuserData, this, NULL);
        XtSetMappedWhenManaged(_draftDialog->draftDialog, False);
    }
    tmp_buf =_sendMsgDialog->getDraftFile();
    if (tmp_buf) {
        XtVaSetValues(_draftDialog->draft_name,
                XmNvalue, tmp_buf,
                NULL);
    }              
    else
        XtVaSetValues(_draftDialog->draft_name,
                  XmNvalue,  "",
                  NULL);
                   
    XtManageChild(_draftDialog->dialog_shellform);
    XtPopup(_draftDialog->draftDialog, XtGrabNone);
}

void
SaveAsDraftCmd::getNameAndSave()
{
    DtMailEnv error;
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    char *label, *buf_ptr;
    char filepath[1024], buf[1024];
    DtMail::Session * d_session = theRoamApp.session()->session();

    label = XmTextGetString(_draftDialog->draft_name);
    if (label == NULL || *label == NULL) {
        genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"),
                         catgets(DT_catd, 3, 115, "Specify a draft name"));
        int answer = genDialog->post_and_return(DTMAILHELPNOFILENAME);
        if (answer == 1) {  // Pressed OK, we remap dialog
                buf_ptr = _sendMsgDialog->getDraftFile();
                if (buf_ptr) free(buf_ptr);
                _sendMsgDialog->setDraftFile(0);
                doit();
        }
        else this->quit();
	if (label) free(label);
        return;
    }
    
    if (*label == '/' || *label == '$' || *label == '~') {
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"),
		 catgets(DT_catd, 1, 300, "Do not specify a folder\nname when saving the file."));
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOFOLDER);
	doit();
	free (label);
	return;
    }
    char *draftsDir = theRoamApp.getResource(DraftDir, TRUE);
    sprintf(filepath, "%s/%s", draftsDir, label);
    if (SafeAccess(draftsDir, F_OK) != 0) 
	mkdir(draftsDir, 0700);
    free(draftsDir);

    if (SafeAccess(filepath, F_OK) == 0) {
        sprintf(buf, catgets(DT_catd, 3, 47, 
		"%s already exists.\nOverwrite?"), label);
        genDialog->setToQuestionDialog(catgets(DT_catd, 3, 48, "Mailer"), 
		buf);
        int answer = genDialog->post_and_return(DTMAILHELPMSGALREADYEXISTS);
        if (answer == 2) { // Pressed cancel
            doit();
	    free(label);
            return;
        }
        this->quit();
    }
       
    int fd;
    if ((fd = SafeOpen(filepath, O_RDWR | O_CREAT | O_TRUNC, 0600)) < 0) {
        sprintf(buf, catgets(DT_catd, 3, 51, "Unable to create %s."), label);
        genDialog->setToErrorDialog(catgets(DT_catd, 3, 52, "Mailer"), buf);
        genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOCREATE);
        this->quit();
	free(label);
        return;
    }

    _sendMsgDialog->updateMsgHnd();
    DtMail::Message *msgHandle = _sendMsgDialog->getMsgHnd();

    RFCWriteMessage(error, theRoamApp.session()->session(), filepath, msgHandle, TRUE);
    draftsDir = theRoamApp.getResource(DraftDir, FALSE);
    sprintf(filepath, "%s/%s", draftsDir, label);
    free(label);
    free(draftsDir);
    sprintf(buf, catgets(DT_catd, 3, 116, "Draft saved to %s"), filepath);

    this->quit();

    _sendMsgDialog->setStatus(buf);
    _sendMsgDialog->setEditsMade(FALSE);
    SafeClose(fd);
}
void
SaveAsDraftCmd::quit()
{
    char *buf = _sendMsgDialog->getDraftFile();
    if (buf) free(buf);
    buf = XmTextGetString(_draftDialog->draft_name);
    _sendMsgDialog->setDraftFile(buf);
    XtUnmanageChild(_draftDialog->dialog_shellform);
    XtPopdown(_draftDialog->draftDialog);
}
void
SaveAsDraftCmd::doit()
{
    this->saveDraft();
}
SaveAsTemplateCmd::SaveAsTemplateCmd ( 
			       char *name, 
			       char *label,
			       int active, 
			       Editor * editor,
			       SendMsgDialog *smd,
			       Widget parent
			       ) 
: NoUndoCmd( name, label, active )
{
    _text_editor = editor;
    _templateDialog = NULL;
    _parent = parent;
    _sendMsgDialog = smd;
}

void
SaveAsTemplateCmd::saveTemplate()
{
    DtMailEnv error;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMail::MailRc *mail_rc = d_session->mailRc(error);
    char *tmp_buf=NULL, *expanded_path=NULL;
 
    if (_templateDialog == NULL) {
	_templateDialog = (DtbModuleDialogInfo)malloc(sizeof(
                               DtbModuleDialogInfoRec));
	dtbTemplTemplateDialogInfo_clear(_templateDialog);
	dtb_template_dialog_initialize(_templateDialog, _parent);
	dtb_save_toplevel_widget(_parent);
    	XtSetMappedWhenManaged(_templateDialog->templateDialog, False);
    }
    XmString label_str = XmStringCreateLocalized(catgets(DT_catd, 1, 94, "Save"));
    XtVaSetValues(_templateDialog->save_button, XmNuserData, this,
                        XmNlabelString, label_str,
                        NULL);  
    XmStringFree(label_str);
    XtVaSetValues(_templateDialog->cancel_button, XmNuserData, this, NULL);
    XtVaSetValues(_templateDialog->templateDialog,
                XmNtitle, catgets(DT_catd, 1, 292, "Mailer - Save As Template"),
                NULL);
    XtRemoveAllCallbacks (_templateDialog->save_button, XmNactivateCallback);
    XtRemoveAllCallbacks (_templateDialog->cancel_button, XmNactivateCallback);
    XtAddCallback(_templateDialog->save_button,
                XmNactivateCallback, save_butCB, this);
    XtAddCallback(_templateDialog->cancel_button,
                XmNactivateCallback, cancel_butCB, this);

    tmp_buf =_sendMsgDialog->getTemplateFile();
    if (tmp_buf) {
	XtVaSetValues(_templateDialog->templ_name,
		XmNvalue, tmp_buf,
		NULL);
    }
    else
    	XtVaSetValues(_templateDialog->templ_name,
                  XmNvalue,  "",
                  NULL);

    XtManageChild(_templateDialog->dialog_shellform);
    XtPopup(_templateDialog->templateDialog, XtGrabNone);
}
void
SaveAsTemplateCmd::doit()
{
    this->saveTemplate();
}
void
SaveAsTemplateCmd::quit()
{
    char *buf = _sendMsgDialog->getTemplateFile();
    if (buf)  free(buf);
    buf = XmTextGetString(_templateDialog->templ_name);
    _sendMsgDialog->setTemplateFile(buf);
    XtUnmanageChild(_templateDialog->dialog_shellform);
    XtPopdown(_templateDialog->templateDialog);
}
void
SaveAsTemplateCmd::getNameAndSave()
{
    char *label, *buf_ptr;
    char full_tname[1024], expfilename[1024], filename[1024], buf[1024], buf2[1024];
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    int i, fd;
    Boolean already_in_list = FALSE;
    DtMailEnv error;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMail::MailRc *mail_rc = d_session->mailRc(error);

    label = XmTextGetString(_templateDialog->templ_name);
    if (label == NULL || *label == NULL) {
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"),
			 catgets(DT_catd, 3, 98, "Specify a template name"));
       	int answer = genDialog->post_and_return(DTMAILHELPNOFILENAME);
        if (answer == 1) {  // Pressed OK, we remap dialog
    		buf_ptr = _sendMsgDialog->getTemplateFile();
    		if (buf_ptr) free(buf_ptr);
    		_sendMsgDialog->setTemplateFile(0);
                doit(); 
	}
	else this->quit();
	if (label) free(label);
        return;
    }
    if (*label == '/' || *label == '$' || *label == '~') {
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"),
		 catgets(DT_catd, 1, 300, "Do not specify a folder\nname when saving the file."));
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOFOLDER);
	doit(); 
	free(label);
	return;
    }

    char *template_path = _sendMsgDialog->getTemplateFullPath(); 
    if (template_path) {
        sprintf(filename, "%s/%s", template_path, label);
    	template_path = d_session->expandPath(error, template_path); 
        sprintf(expfilename, "%s/%s", template_path, label);
    }
    else {
    	template_path = theRoamApp.getResource(TemplateDir, FALSE);
        sprintf(filename, "%s/%s", template_path, label);
	free(template_path);
    	template_path = theRoamApp.getResource(TemplateDir, TRUE);
        sprintf(expfilename, "%s/%s", template_path, label);
    	if (SafeAccess(template_path, F_OK) != 0) 
                	mkdir(template_path, 0700);
    }
    free(template_path);

    if (SafeAccess(expfilename, F_OK) == 0) {
        sprintf(buf, catgets(DT_catd, 3, 47, "%s already exists.\nOverwrite?"),
                label);
        genDialog->setToQuestionDialog(catgets(DT_catd, 3, 48, "Mailer"), buf);
        int answer = genDialog->post_and_return(DTMAILHELPMSGALREADYEXISTS);
 
        if (answer == 2) { // Pressed cancel
	    doit(); 
	    free(label);
            return;
	}
	this->quit();
    }

    if ((fd = SafeOpen(expfilename, O_RDWR | O_CREAT | O_TRUNC, 0600)) < 0) {
        sprintf(buf, catgets(DT_catd, 3, 51, "Unable to create %s."), label);
        genDialog->setToErrorDialog(catgets(DT_catd, 3, 52, "Mailer"), buf);
        genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOCREATE);
	this->quit();
	free(label);
        return;
    }

    // Go though the headers and write them out with the associated values
    char *value = NULL;
    for (SendMsgDialog::HeaderList *hl = _sendMsgDialog->getFirstHeader(); 
		hl; hl = _sendMsgDialog->getNextHeader()) {
	// Write only the visible headers.
        if (hl->show == SendMsgDialog::SMD_ALWAYS || 
		hl->show == SendMsgDialog::SMD_SHOWN) {
    		buf2[0] = NULL;
		if (hl->label != NULL) {
			strcat(buf2, hl->label);
			strcat(buf2, ": ");
			value = XmTextGetString(hl->field_widget);
			if (value != NULL) {
				strcat(buf2, value);
				free(value);
			}
			strcat(buf2, "\n");
			if (SafeWrite(fd, buf2, strlen(buf2)) < 1) {
        			sprintf(buf, catgets( DT_catd, 3, 53, 
					"Unable to write to %s."), label);
        			genDialog->setToErrorDialog(
					catgets(DT_catd, 3, 54, "Mailer"), buf);
        			genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
        			SafeClose(fd);
        			unlink(expfilename);
				this->quit();
				free(label);
        			return;
			}
    		}
        }
    }

    // Get the body of the compose
    char *text_ptr = _text_editor->get_contents();
    if (text_ptr == NULL) {
	strcpy(buf, catgets( DT_catd, 3, 109, 
			"Error occurred in saving Template.\nPartial template saved."));
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPTEMPLATENOTEXT);
        SafeClose(fd);
	this->quit();
	free(label);
	return;
    }

    int len = strlen(text_ptr);
    buf_ptr = (char*)malloc(len + 3);
    sprintf(buf_ptr, "\n%s\n", text_ptr);
    free(text_ptr);
    if (SafeWrite(fd, buf_ptr, len+2) < len+2) {
        sprintf(buf, catgets(DT_catd, 3, 53, "Unable to write to %s."), label);
        genDialog->setToErrorDialog(catgets(DT_catd, 3, 56, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
        SafeClose(fd);
        free(buf_ptr);
	free(label);
	this->quit();
        return;
    }
    free(buf_ptr);
    SafeClose(fd);

    sprintf(buf2, "%s:%s", label, filename); 

    // The template can have spaces in it! ug!
    // Make the name that we are going to store here with the backslashes
    // preceding blanks.
    if (buf_ptr = (char *)strtok(buf2, " ")) {
	strcpy(full_tname, buf_ptr);
	while(buf_ptr = (char *)strtok(NULL, " ")) {
		strcat(full_tname, "\\ ");
		strcat(full_tname, buf_ptr);
	}
    }
    else
	strcat(full_tname, buf2);

    // Update the template property.
    // See if its already in the template list

    value = NULL;
    mail_rc->getValue(error, "templates", (const char **)&value);
    if (value && *value) {
	DtVirtArray<PropStringPair *> *templ_items = 
		new DtVirtArray<PropStringPair *>(10);
	parsePropString(value, *templ_items);
	len = templ_items->length();
	buf_ptr = NULL;
	for (i = 0; i < len; i++) {
		if (strcmp((*templ_items)[i]->label, label) == 0) {
			buf_ptr = d_session->expandPath(error,
				 (*templ_items)[i]->value);
			if (strcmp(buf_ptr, expfilename) == 0) {
				// Dont add to list; its already there.
				already_in_list = TRUE;
				break;
			}
		}
	}
	delete templ_items;
	if (buf_ptr)
		free(buf_ptr);
    }
    free(label);

    // Its not in the list, so add to mail options pane if it exists.
    // And update the mailrc file. Also update the templates menu on 
    // compose and RMW.

    if (!already_in_list) {
	if (value && *value) {
		buf_ptr = (char*)malloc(strlen(value) + strlen(full_tname) + 2);
		sprintf(buf_ptr, "%s %s", value, full_tname);
		mail_rc->setValue(error, "templates", buf_ptr);
		free(buf_ptr);
	}
	else 
		mail_rc->setValue(error, "templates", full_tname);

	// Update the mailrc, the template menus and the options pane.
	DtbOptionsDialogInfo oHandle = 
			(DtbOptionsDialogInfo)theRoamApp.optionsDialog();
	if (oHandle && oHandle->templates_list) {
		// Update the templates list on options pane.
		TemplateListUiItem *tmpl_ptr;
		XtVaGetValues(oHandle->templates_list, 
			XmNuserData, &tmpl_ptr, NULL);
		tmpl_ptr->handleIncludeButtonPress(filename);
	}

	// Update the mailrc file
	mail_rc->update(error);
	// Update the templates menus on compose and RMW
    	theRoamApp.updateTemplates();
    }

    if (value)
	free(value);

    this->quit();

    sprintf(buf, catgets(DT_catd, 3, 117, "Template saved to %s"), filename);
    _sendMsgDialog->setStatus(buf);
    _sendMsgDialog->setEditsMade(FALSE);
}

DeleteAttachCmd::DeleteAttachCmd( 
				  char *name, 
				  char *label,
				  int active,
				  SendMsgDialog *smd
				  
				  ) : Cmd ( name, label, active )
{
    _parent = smd;
}

void
DeleteAttachCmd::doit()
{
    _parent->delete_selected_attachments();
    
}

void
DeleteAttachCmd::undoit()
{
}

UndeleteAttachCmd::UndeleteAttachCmd( 
				      char *name, 
				      char *label,
				      int active,
				      SendMsgDialog *smd
				      
				      ) : Cmd ( name, label, active )
{
    _parent = smd;
}

void
UndeleteAttachCmd::doit()
{
    _parent->undelete_last_deleted_attachment();
    
}

void
UndeleteAttachCmd::undoit()
{
}

RenameAttachCmd::RenameAttachCmd ( 
				   char *name, 
				   char *label,
				   int active,
				   SendMsgDialog *smd
				   ) : Cmd ( name, label, active )
{
    Widget renameDialog;
    XmString message;
    
    _parent = smd;
    renameDialog = XmCreatePromptDialog(
					smd->baseWidget(), 
					"renameDialog", 
					NULL, 
					0
					);
    
    message = XmStringCreateLocalized(catgets(DT_catd, 1, 96, "Empty"));
    XtVaSetValues(renameDialog, XmNselectionLabelString, message, NULL);
    XmStringFree(message);
    XmString ok_str = XmStringCreateLocalized(catgets(DT_catd, 1, 97, "Rename"));
    XtVaSetValues(XtParent(renameDialog),
		  XmNtitle, catgets(DT_catd, 1, 98, "Mailer - Rename"),
		  NULL);
    XtVaSetValues(renameDialog,
		  XmNokLabelString, ok_str,
		  NULL);

    XmStringFree(ok_str);

    XtAddCallback(XmSelectionBoxGetChild(renameDialog, XmDIALOG_HELP_BUTTON),
		 XmNhelpCallback, HelpCB, "RenameAnAttachment");

    _parent->get_editor()->attachArea()->setRenameDialog(renameDialog);
    XtAddCallback(renameDialog, XmNcancelCallback, 
		  &RenameAttachCmd::cancelCallback,
		  (XtPointer) this );
    XtAddCallback(renameDialog, XmNokCallback, 
		  &RenameAttachCmd::okCallback,
		  (XtPointer) this );
}

void RenameAttachCmd::doit()
{
    Widget renameDialog;
    XmString oldAttachName = NULL;
    XmString message;
    char	buf[512];
    AttachArea *aa;
    
    if (!_parent->renameAttachmentOK()) {
	return;
    }
    
    aa = _parent->get_editor()->attachArea();
    
    oldAttachName = aa->getSelectedAttachName();
    
    if (oldAttachName == NULL) return;
    
    renameDialog = aa->getRenameDialog();
    
    sprintf(buf, catgets(DT_catd, 3, 57, "Rename attachment as"));
    
    message = XmStringCreateLocalized(buf);
    
    XtVaSetValues(renameDialog, 
		  XmNselectionLabelString, message,
		  XmNtextString, oldAttachName,
		  NULL);
    
//     XtFree(buf);
    XmStringFree(message);
    XmStringFree(oldAttachName);
    
    XtManageChild(renameDialog);
}      

void RenameAttachCmd::undoit()
{
    // Just print a message that allows us to trace the execution
    
}       

void RenameAttachCmd::cancelCallback ( 
				       Widget, 
				       XtPointer clientData, 
				       XtPointer callData 
				       )
{
    RenameAttachCmd *obj = (RenameAttachCmd *) clientData;
    
    obj->cancel( callData );
}

void RenameAttachCmd::cancel( XtPointer )
{
    AttachArea* aa;
    
    aa = _parent->get_editor()->attachArea();
    
    Widget renameDialog = aa->getRenameDialog();
    
    XtUnmanageChild(renameDialog);
}

void RenameAttachCmd::okCallback ( 
				   Widget, 
				   XtPointer clientData, 
				   XtPointer callData 
				   )
{
    RenameAttachCmd *obj = (RenameAttachCmd *) clientData;
    obj->ok( callData );
}

void RenameAttachCmd::ok( XtPointer callData )
{
    XmSelectionBoxCallbackStruct *cbs = 
	(XmSelectionBoxCallbackStruct *)callData;
    AttachArea *aa;
    
    aa = _parent->get_editor()->attachArea();
    
    Widget renameDialog = aa->getRenameDialog();
    
    XtUnmanageChild(renameDialog);
    
    aa->setSelectedAttachName(cbs->value);
    
}

AttachmentActionCmd::AttachmentActionCmd(
					 char *name,
					 char *label,
					 RoamMenuWindow *rmw,
					 int indx
					 ) : Cmd (name, label, TRUE)
{
    _index = indx;
    
    _parent = rmw;
}

AttachmentActionCmd::AttachmentActionCmd(
					 char *name,
					 char *label,
					 ViewMsgDialog *vmd,
					 int indx
					 ) : Cmd (name, label, TRUE)
{
    _index = indx;
    
    _parent = vmd;
}

AttachmentActionCmd::AttachmentActionCmd(
					 char *name,
					 char *label,
					 SendMsgDialog *smd,
					 int indx
					 ) : Cmd (name, label, TRUE)
{
    _index = indx;
    
    _parent = smd;
}


void
AttachmentActionCmd::doit()
{
    _parent->invokeAttachmentAction(_index);
}

void
AttachmentActionCmd::undoit()
{
}

SelectAllAttachsCmd::SelectAllAttachsCmd(
					 char *name,
					 char *label,
					 RoamMenuWindow *rmw
					 ) : Cmd(name, label, TRUE)
{
    _parent = rmw;
}

SelectAllAttachsCmd::SelectAllAttachsCmd(
					 char *name,
					 char *label,
					 ViewMsgDialog *vmd
					 ) : Cmd(name, label, TRUE)
{
    _parent = vmd;
}

SelectAllAttachsCmd::SelectAllAttachsCmd(
					 char *name,
					 char *label,
					 SendMsgDialog *smd
					 ) : Cmd(name, label, FALSE)
{
    _parent = smd;
}

void
SelectAllAttachsCmd::doit()
{
    _parent->selectAllAttachments();
}

void
SelectAllAttachsCmd::undoit()
{
    // 
}

ShowAttachPaneCmd::ShowAttachPaneCmd(
				     char *name,
				     char *label,
				     AbstractEditorParent *aep
				     ) : ToggleButtonCmd(name, label, TRUE)
{
    _parent = aep;
}

void
ShowAttachPaneCmd::doit()
{
    // If button is OFF
    if (!this->getButtonState()) {
	_parent->hideAttachArea();
    }
    else {  // button is ON
	_parent->showAttachArea();
    }
}

void
ShowAttachPaneCmd::undoit()
{
    // 
}

AbbrevHeadersCmd::AbbrevHeadersCmd(
				   char *name,
				   char *label,
				   RoamMenuWindow *rmw
				   ) : ToggleButtonCmd(name, label, TRUE)
{
    _parent = rmw;
}

void
AbbrevHeadersCmd::doit()
{
    // If button is OFF
    if (!this->getButtonState()) {
	_parent->fullHeader(TRUE);
    }
    else {  // button is ON
	_parent->fullHeader(FALSE);
    }
}

void
AbbrevHeadersCmd::undoit()
{
    // 
}

CloseCmd::CloseCmd( 
    char *name, 
    char *label,
    int active, 
    Widget w, 
    SendMsgDialog *s ) 
    : NoUndoCmd(name, label, active)
{
    _compose_dialog = s;
    menubar_w = w;
}

void
CloseCmd::doit()
{
    // Call the goAway() method on the SMD.  Argument TRUE requests it
    // to check if the SMD is dirty.  Let it handle the 
    // case where text may be present in the compose window.
    if (!_compose_dialog->isMsgValid())
	return;
    else
	_compose_dialog->goAway(TRUE);

}

EditUndoCmd::EditUndoCmd( 
    char *name, 
    char *label,
    int active, 
    AbstractEditorParent *w ) 
    : NoUndoCmd( name, label, active )
{
    editor = w->get_editor()->textEditor();
}

void
EditUndoCmd::doit()
{
	editor->undo_edit();
}

EditCutCmd::EditCutCmd( 
    char *name, 
    char *label,
    int active, 
    AbstractEditorParent *w
) 
    : NoUndoCmd( name, label, active )
{
    editor = w->get_editor()->textEditor();

    // className() is a virtual method
    if (w->className() == "SendMsgDialog") {
	_compose_dialog = (SendMsgDialog *)w;
    }
    else {
	_compose_dialog = NULL;
    }
}

void
EditCutCmd::doit()
{
    editor->cut_selection();
	
    if (_compose_dialog) {
	// Turn Paste on
	_compose_dialog->activate_edit_paste();
	_compose_dialog->activate_edit_paste_indented();
	_compose_dialog->activate_edit_paste_bracketed();
    }
}

EditCopyCmd::EditCopyCmd( 
    char *name, 
    char *label,
    int active, 
    AbstractEditorParent *w ) 
    : NoUndoCmd( name, label, active )
{
    editor = w->get_editor()->textEditor();
    
    // className() is a virtual method
    if (w->className() == "SendMsgDialog") {
	_compose_dialog = (SendMsgDialog *)w;
    }
    else {
	_compose_dialog = NULL;
    }
}

void
EditCopyCmd::doit()
{
    editor->copy_selection();
    if (_compose_dialog) {
	// Turn Paste on
	_compose_dialog->activate_edit_paste();
	_compose_dialog->activate_edit_paste_indented();
	_compose_dialog->activate_edit_paste_bracketed();
    }
}

EditPasteCmd::EditPasteCmd( 
    char *name, 
    char *label,
    int active, 
    AbstractEditorParent *w ) 
    : NoUndoCmd( name, label, active )
{
    editor = w->get_editor()->textEditor();
}

void
EditPasteCmd::doit()
{
	editor->paste_from_clipboard();
}

EditPasteSpecialCmd::EditPasteSpecialCmd( 
    char *name, 
    char *label,
    int active, 
    AbstractEditorParent *w,
    Editor::InsertFormat format) 
    : NoUndoCmd( name, label, active )
{
    editor = w->get_editor()->textEditor();
    insert_format = format;
}

void
EditPasteSpecialCmd::doit()
{
	editor->paste_special_from_clipboard(insert_format);
}


EditClearCmd::EditClearCmd( 
    char *name, 
    char *label,
    int active, 
    AbstractEditorParent *w ) 
    : NoUndoCmd( name, label, active )
{
	editor = w->get_editor()->textEditor();
    // this->deactivate();
}

void
EditClearCmd::doit()
{
    editor->clear_selection();
	// Turn Paste on
	// _edit_paste->activate();
}

EditDeleteCmd::EditDeleteCmd( 
    char *name, 
    char *label,
    int active, 
    AbstractEditorParent *w ) 
    : NoUndoCmd( name, label, active )
{
    editor = w->get_editor()->textEditor();
	// this->deactivate();
}

void
EditDeleteCmd::doit()
{
    editor->delete_selection();
	// Turn Paste off
	// _edit_paste->deactivate();
}

EditSelectAllCmd::EditSelectAllCmd( 
    char *name, 
    char *label,
    int active, 
    AbstractEditorParent *w ) 
    : NoUndoCmd( name, label, active )
{
    editor = w->get_editor()->textEditor();
}

void
EditSelectAllCmd::doit()
{
	editor->select_all();
}



WordWrapCmd::WordWrapCmd( 
    char *name, 
    char *label,
    int active, 
    AbstractEditorParent *w 
) : ToggleButtonCmd( name, label, active )
{
    editor = w->get_editor()->textEditor();
    cur_setting = active;    
    editor->set_word_wrap(cur_setting);		
}

void
WordWrapCmd::doit()
{
    cur_setting = ((ToggleButtonCmd *)this)->getButtonState();
    editor->set_word_wrap(cur_setting);		
}

Boolean
WordWrapCmd::wordWrap()
{
    return(cur_setting);
}

FindChangeCmd::FindChangeCmd( 
    char *name, 
    char *label,
    int active, 
    AbstractEditorParent *w ) 
    : NoUndoCmd( name, label, active )
{
    editor = w->get_editor()->textEditor();
}

void
FindChangeCmd::doit()
{
	editor->find_change();
}

SpellCmd::SpellCmd( 
    char *name, 
    char *label,
    int active, 
    AbstractEditorParent *w ) 
    : NoUndoCmd( name, label, active )
{
    editor = w->get_editor()->textEditor();
}

void
SpellCmd::doit()
{
	editor->spell();
}

FormatCmd::FormatCmd( 
    char *name, 
    char *label,
    int active, 
    AbstractEditorParent *w ) 
    : NoUndoCmd( name, label, active )
{
    editor = w->get_editor()->textEditor();
}

void
FormatCmd::doit()
{
	editor->format();
}

LogMsgCmd::LogMsgCmd( 
    char *name, 
    char *label,
    int active, 
    SendMsgDialog * send 
) : ToggleButtonCmd( name, label, active )
{
  // Go to props and find out the default, ie. to log or not to log.
  // But for now, just look in .mailrc to see if "record" is set.

  DtMailEnv error;
  const char *logfile = NULL;

  _send = send;

}

void
LogMsgCmd::doit()
{
    if (!((ToggleButtonCmd *)this)->getButtonState()) {
	// turn off logging for this message
	_send->setLogState(DTM_FALSE);
    }
    else {
	// turn on logging for this message
	_send->setLogState(DTM_TRUE);
    }

}
char *
VacationCmd::getMonth(struct tm *tm)
{
	static char *month;

	if (month == NULL) 
		month = (char*) malloc(5);
	*month = NULL;
	SafeStrftime (month, 5, "%b", tm);
	return (month);
}


VacationCmd::VacationCmd(
    char *name,
    char *label
) : Cmd (name, label, TRUE)
{

    _subject = NULL;
    _body = NULL;
    _stop_timeout = 0;
    _start_timeout = 0;
}

void
VacationCmd::doit()
{
}
void
VacationCmd::setStartVacationTimeout(time_t tick)
{
    // We do this to remove the startatjobid.
    _start_timeout = XtAppAddTimeOut(
	 theApplication->appContext(),
	 tick * 1000,
	 startVacationCallback,
	 (XtPointer) this );
}
void
VacationCmd::setStopVacationTimeout(time_t tick)
{
    // We do this to reset the titles on the RMW when the vacation is turned off via `at' job
    _stop_timeout = XtAppAddTimeOut(
	 theApplication->appContext(),
	 tick * 1000,
	 resetVacationCallback,
	 (XtPointer) this );
}

static unsigned long
writeToFileDesc(const char * buf, int len, va_list args)
{
    int fd = va_arg(args, int);
    int cnt = va_arg(args, int);
    int status = 0;

    do {
	status = SafeWrite(fd, buf, len);
    } while (status < 0 && errno == EAGAIN);
	
    return(0);
}
// This routine only gets called if the user has an at job to stop vacation.
// When its time to stop vacation, this routine resets the GUI. 
void
VacationCmd::resetVacationCallback(XtPointer ptr, XtIntervalId *)
{
    DtMailEnv error;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMail::MailRc *mail_rc = d_session->mailRc(error);

    theRoamApp.resetTitles();

    VacationCmd *cmd_ptr = (VacationCmd*)ptr;
    if (cmd_ptr) {
    	cmd_ptr->resetRadioButton(FALSE);
    	cmd_ptr->_stop_timeout = 0;
    }

    mail_rc->removeValue(error, "vacationstopatjobid");
    mail_rc->removeValue(error, "vacationatjobhost");
    mail_rc->update(error);
 
}
void
VacationCmd::startVacationCallback(XtPointer ptr, XtIntervalId *)
{
    DtMailEnv error;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMail::MailRc *mail_rc = d_session->mailRc(error);

    mail_rc->removeValue(error, "vacationstartatjobid");

    // Dont remove the atjobhost  unless there is no stop at job.
    // It gets removed when the stop at job goes off. 
    VacationCmd *cmd_ptr = (VacationCmd*)ptr;
    if (cmd_ptr) {
	if (cmd_ptr->_stop_timeout == 0)
    		mail_rc->removeValue(error, "vacationatjobhost");
    	cmd_ptr->_start_timeout = 0;
    }

    mail_rc->update(error);

}

void
VacationCmd::startVacationError(char * error)
{
    char buf[1024];
    DtMailGenDialog *genDialog = theRoamApp.genDialog();

    sprintf(buf, catgets(DT_catd, 15, 8, "A system error occurred setting up the vacation notifier:\n%s\nThe vacation notifier will not take effect."), error);
    genDialog->setToErrorDialog(catgets(DT_catd, 3, 50, "Mailer"), buf);
    genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"),
                DTMAILHELPVACATIONSETUPERROR);
}
char *
VacationCmd::getHost()
{
	static char host[1024];

	if (host == NULL || *host == NULL) 
                sysinfo(SI_HOSTNAME, host, 1024);
	return host;
}
int
VacationCmd::startVacation()
{
    FORCE_SEGV_DECL(DtMail::Session, m_session);
    DtMailEnv error;
    time_t v_starttick, v_endtick, nowtick;
    struct tm stm, etm, *nowtm;
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    char buf[1024], *ptr = NULL, *v_buf = NULL, *month;
    int totalbytes=0; 

    error.clear();

    // Basically reset everything. 
    resetVacationItems();
    theRoamApp.resetTitles();

    m_session = theRoamApp.session()->session();
    m_session->mailRc(error)->getValue(error, "vacationstartdate", (const char **)&v_buf); 

    // If null, then it is set to today, which is the default.
    if (!v_buf || !*v_buf) {
	strcpy(buf, catgets(DT_catd, 15, 12, "You have not entered a start date.\nThe vacation notifier will not take effect."));
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 50, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), 
		DTMAILHELPVACATIONNOSTARTDATE);
	if (v_buf) free(v_buf);
	return -1;
    }
    else {
        // Convert from date string to tm struct
    	if (strptime (v_buf, catgets(DT_catd, 1, 238, "%m %d %Y"), &stm) == NULL) {
		strcpy(buf, catgets(DT_catd, 15, 10, "The start date format is incorrect.\nUse the format shown next to the End Date.\nThe vacation notifier will not take effect."));
		genDialog->setToErrorDialog(catgets(DT_catd, 3, 50, "Mailer"), buf);
		genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), 
			DTMAILHELPVACATIONINVALIDSTARTDATE);
		if (v_buf) free(v_buf);
		return -1;
	}
    }
    if (v_buf) {
	free (v_buf); 
	v_buf = NULL;
    }

    nowtick = time((time_t *) 0);

    // Get the now tm struct as we need to see if they are starting vacation 'today'
    // because if so, we just start it without using an 'at' job.
    nowtm = localtime(&nowtick);
    if (stm.tm_mon == nowtm->tm_mon && stm.tm_mday == nowtm->tm_mday &&
                stm.tm_year == nowtm->tm_year)
    	v_starttick = nowtick;
    else {
    	// We set the start date to be the first second of the day.
    	// tm_isdst == -1 tells kernel to take daylight savings time into account.
    	stm.tm_min = 1;
    	stm.tm_hour = 0;
    	stm.tm_isdst = -1;
    	v_starttick = mktime (&stm);
    }

    if (nowtick > v_starttick) {
	strcpy(buf, catgets(DT_catd, 15, 6, "The start date has already passed.\nThe vacation notifier will not take effect."));
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 50, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), 
		DTMAILHELPVACATIONSTARTDATEPASSED);
	return -1;
    }

    // Set up the end date.
    m_session->mailRc(error)->getValue(error, "vacationenddate", (const char**)&v_buf); 

    if (!v_buf || !*v_buf) 
	// Keep vacation on forever until user turns off via the GUI
    	v_endtick = -1;
    else { 
	if (strptime (v_buf, catgets(DT_catd, 1, 238, "%m %d %Y"), &etm) == NULL) {
		strcpy(buf, catgets(DT_catd, 15, 11, "The end date format is incorrect.\nUse the format shown next to the End Date.\nThe vacation notifier will not take effect."));
		genDialog->setToErrorDialog(catgets(DT_catd, 3, 50, "Mailer"), buf);
		genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), 
			DTMAILHELPVACATIONINVALIDENDDATE);
		if (v_buf) free(v_buf);
		return -1;
	}
	// If enddate same as start date, end vacation on last tick
        // of that day, otherwise end on first tick of that day
        if (etm.tm_mon == stm.tm_mon && etm.tm_mday == stm.tm_mday &&
                etm.tm_year == stm.tm_year) {
                etm.tm_min = 59;
                etm.tm_hour = 23;
        }
        else {
                etm.tm_min = 1;
                etm.tm_hour = 0;
        }
        etm.tm_isdst = -1;
	v_endtick = mktime (&etm);
    }
    if (v_buf) {
	free (v_buf); 
	v_buf = NULL;
    }

    if (v_endtick != -1 && v_endtick < v_starttick) {
	strcpy(buf, catgets(DT_catd, 15, 7, "The end date occurs before the start date.\nThe vacation notifier will not take effect."));
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 50, "Mailer"), buf);
        genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), 
		DTMAILHELPVACATIONENDDATEBEFORESTART);
	return -1;
    }


    // This means we are using popen to start the at job so need to create a file
    FILE *pfp, *fp;
    char stderrfile[20];
    if (v_starttick != nowtick || v_endtick != -1) {

	strcpy(stderrfile, "/tmp/.dtmailXXXXXX");
    	mktemp(stderrfile);
    	if ((fp = fopen(stderrfile, "w+")) == NULL) {
		startVacationError("open() failed");
		return -1;
    	}

    	// Duplicate the stderr to a file, so we can read the 'at' file job id.
    	if (SafeDup2(fileno(fp), 2) == -1) {
		startVacationError("dup2() failed");
		fclose(fp);
		SafeUnlink(stderrfile);
		return -1;
    	}
    }

    // Just start it without the at mechanism if they want to turn it on for 'today'
    if (v_starttick == nowtick) {
	m_session->mailRc(error)->getValue(error, "vacationreplyinterval", (const char **)&v_buf);
	addVacationToForwardFile(v_buf);
	if (v_buf) free(v_buf);
    	if (v_endtick == -1) 
		return 0;
    }
    else {


    	// Set up the 'at' string... 
    	// The 5.5 interface is nicer but doesnt work with 5.4
	month = getMonth(&stm);
    	sprintf(buf, "at %02d%02d %s %d,%4d", stm.tm_hour, stm.tm_min, 
		 month, stm.tm_mday, 1900+stm.tm_year);

    	// Sent up the 'at' job with popen. The output of the job, on execution, will
    	// go to /dev/null. By default it sends mail to the user which we dont want. 
    	if ((pfp = popen(buf, "w")) != 0) {
		m_session->mailRc(error)->getValue(error, "vacationreplyinterval", 
				(const char **)&v_buf);
		if (v_buf && *v_buf)
			fprintf(pfp, "/usr/dt/bin/dtmail -v start %s", v_buf);
		else
			fprintf(pfp, "/usr/dt/bin/dtmail -v start");
		if (v_buf) free(v_buf);
		pclose(pfp);
    	}
    	else {
		startVacationError("popen() failed");
		fclose(fp);
		SafeUnlink(stderrfile);
		resetAtJobs();
		return -1;
    	}

   	// Read from the stderr file and get the 'at' job id.
   	// Use fgets() vs. read(). It is safer as we know that the string: "job id"
   	// occurs at the beginning of the input line. 
    	rewind(fp);
    	while (fgets(buf, 1024, fp)) {
		totalbytes += strlen(buf);
		if (strncmp(buf, "job ", 4) == 0) { 
			// Get the at job id 
			if (ptr = strchr(buf+4, ' ')) {
				*ptr = '\0';
				m_session->mailRc(error)->setValue(error, 
					"vacationstartatjobid", buf+4);
				m_session->mailRc(error)->setValue(error, 
					"vacationatjobhost", getHost());
    				while (fgets(buf, 1024, fp)) 
					totalbytes += strlen(buf);
				break;
			}
			else ptr = NULL;
		}
    	}
    	if (!ptr) {
		startVacationError("Cant get the 'at' job id from stderr");
		fclose(fp);
		SafeUnlink(stderrfile);
		resetAtJobs();
		return -1;
    	}
    }
    // No end 'at' job to set, so just return gracefully. User must 
    // manually turn off the vacation.
    if (v_endtick == -1) {
    	m_session->mailRc(error)->update(error);
	fclose(fp);
        SafeUnlink(stderrfile);
	return 0;
    }

    month = getMonth(&etm);
    sprintf(buf, "at %02d%02d %s %d,%4d", etm.tm_hour, etm.tm_min, 
		month, etm.tm_mday, 1900+etm.tm_year);

    if ((pfp = popen(buf, "w")) != 0) {
	fprintf(pfp, "/usr/dt/bin/dtmail -v stop");
	pclose(pfp);
    }
    else {
	startVacationError("popen() failed");
	fclose(fp);
	SafeUnlink(stderrfile);
	resetAtJobs();
	return -1;
    }
    // Read from the stderr file and get the 'at' job id.
    ptr = NULL;
    fseek(fp, totalbytes, 0);
    while (fgets(buf, 1024, fp )) {
	if (strncmp(buf, "job ", 4) == 0) {
		// Get the at job id 
		if (ptr = strchr(buf+4, ' ')) {
			*ptr = '\0';
			m_session->mailRc(error)->setValue(error, 
				"vacationstopatjobid", buf+4);
			m_session->mailRc(error)->setValue(error, 
				"vacationatjobhost", getHost());
			break;
		}
		else ptr = NULL;
	}
    }
    if (!ptr) {
	startVacationError("Cant get the 'at' job id from stderr");
	fclose(fp);
	SafeUnlink(stderrfile);
	resetAtJobs();
	return -1;
    }
    if (v_starttick != nowtick)
    	setStartVacationTimeout(v_starttick- nowtick);
    setStopVacationTimeout(v_endtick- nowtick);

    m_session->mailRc(error)->update(error);
    fclose(fp);
    SafeUnlink(stderrfile);
	
    return 0;
}

void
VacationCmd::resetAtJobs()
{
	char *value=NULL;
	DtMailEnv error;
        DtMail::Session * d_session = theRoamApp.session()->session();
        DtMail::MailRc *mail_rc = d_session->mailRc(error);
        char buf[256]; 
	Boolean different_hosts = FALSE, update = FALSE; 
 
        *buf = NULL;  
        mail_rc->getValue(error, "vacationstartatjobid", (const char **)&value);
        if (value) {
		if (*value) { 
                	sprintf(buf, "at -r %s ", value);
			mail_rc->removeValue(error, "vacationstartatjobid");
			update = TRUE;
		}
                free(value); 
                value = NULL;
        } 
        mail_rc->getValue(error, "vacationstopatjobid", (const char **)&value);
        if (value) {
		if (*value) {
			if (*buf == NULL)
                		sprintf(buf, "at -r %s ", value);
			else
                		strcat(buf, value);
                	mail_rc->removeValue(error, "vacationstopatjobid");
			update = TRUE;
		}
                free(value); 
                value = NULL;
        }

	mail_rc->getValue(error, "vacationatjobhost", (const char **)&value);
        if (*buf != NULL) {
		// At jobs were started on some other host. Dont try to remove
		if (value && *value) {
			char *host = getHost();
			if (different_hosts = strcmp (host, value)) {
				char buf2[1024];
				DtMailGenDialog *genDialog = theRoamApp.genDialog();
				sprintf(buf2, catgets(DT_catd, 15, 15, "You already started the vacation message on the host %s.\nGo to this host and stop your vacation message,\nor it may start or stop prematurely."), value);
				genDialog->setToErrorDialog(
					catgets(DT_catd, 3, 48, "Mailer"), buf2);
				genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPVACATIONHOSTCHANGED);
			}
		}
		// Only remove at jobs if on the host where they were started.
		if (!different_hosts)
			if (fork() == 0)
                        	execl("/bin/sh", "sh", "-c", buf, (char *)0);
	}
	if (value) {
		if (*value) {
			mail_rc->removeValue(error, "vacationatjobhost");
			update = TRUE;
		}
		free(value);
	}
		
	if (update)
		mail_rc->update(error);
}
void
VacationCmd::resetVacationMessage()
{
   DtbOptionsDialogInfo oH = (DtbOptionsDialogInfo)theRoamApp.optionsDialog();
   if (oH) {
	if (_subject)
		XtVaSetValues(oH->subject_tf,
                      XmNvalue, _subject,
                      NULL);
	if (_body)
		XtVaSetValues(oH->vacation_msg_tp,
                      XmNvalue, _body,
                      NULL);
   }
}
void
VacationCmd::resetRadioButton(Boolean turnOn)
{
   DtbOptionsDialogInfo oH = (DtbOptionsDialogInfo)theRoamApp.optionsDialog();
   if (oH) {
	XtVaSetValues(oH->vacation_rb_items.On_item, XmNset, turnOn, NULL);
    	XtVaSetValues(oH->vacation_rb_items.Off_item, XmNset, !turnOn, NULL);
   }
}
void
VacationCmd::resetVacationItems()
{
    resetAtJobs();

    this->removeVacationFromForwardFile();

    if (_stop_timeout != 0) {
	XtRemoveTimeOut(this->_stop_timeout);
	_stop_timeout = 0;
    }
    if (_start_timeout != 0) {
	XtRemoveTimeOut(this->_start_timeout);
	_start_timeout = 0;
    }
}

void
VacationCmd::stopVacation()
{
    resetVacationItems();
    resetRadioButton(FALSE);
}
Boolean
VacationCmd::vacationAlreadyStarted()
{
	int	fd;
	char	buf[256];

	sprintf(buf, "%s/.forward", getenv("HOME"));

	if (SafeAccess(buf, F_OK) != 0)
		return(FALSE);
	
	fd = SafeOpen(buf, O_RDONLY);
	
	*buf = NULL;
	int len;
	while ((len = SafeRead(fd, buf, 255)) > 0) {
	    buf[len] = 0;
	    if ((strstr(buf, "/usr/bin/vacation")) ||
		(strstr(buf, "/usr/ucb/vacation"))) {
		SafeClose(fd);
		return TRUE;
	    }
	}
	SafeClose(fd);
	return(FALSE);
}

Boolean
VacationCmd::vacationIsOn()

{
        DtMailEnv error;
    	DtMail::Session * d_session = theRoamApp.session()->session();
    	DtMail::MailRc *mail_rc = d_session->mailRc(error);
	char *value;

        // There are two ways to determine if vacation is on:
	// 1. There is an entry in .mailrc for vacationstartatjobid
	// 2. There is an entry in the .forward  for vacation
	// Note: After vacation is started the vacationstartatjobid resource is removed
	// so vacation could be on without this resource being set. However that would mean
	// there would for sure be an entry in the .forward file for vacation.

        mail_rc->getValue(error, "vacationstartatjobid", (const char **)&value);
	if (error.isNotSet() && value && *value)
		return TRUE;

	return(vacationAlreadyStarted());
}

void
VacationCmd::createForwardVacationStr(char *buf, char *pw_name, char *replyInterval)
{
	sprintf (buf, "\\%s, \"| /usr/bin/vacation ", pw_name);

        if (replyInterval && *replyInterval && strcmp (replyInterval, "7") != 0) {
		// 7 is the default so just ignore it.
		strcat(buf, "-t");
		strcat(buf, replyInterval);
		strcat(buf, "d ");
  	}
	strcat(buf, pw_name);
	strcat(buf, "\"\n");
}

void
VacationCmd::removeVacationFromForwardFile()
{

    // If vacation not in forward file, then just return
    if (!vacationAlreadyStarted()) 
        return;

    char buf[80], forwardfile[512], tmpforwardfile[512];
    FILE *fwd_fp=NULL, *tmp_fp=NULL;
    Boolean wrote_something = FALSE;

    passwd pw;
    GetPasswordEntry(pw);

    // Create a tmp file for writing out new .forward file
    sprintf(tmpforwardfile, "%s/.forward.tmp", pw.pw_dir);
    if (SafeAccess(tmpforwardfile, F_OK) == 0)
        unlink(tmpforwardfile);

    sprintf(forwardfile, "%s/.forward", pw.pw_dir);

    if ((fwd_fp = fopen(forwardfile, "r")) == NULL)
        return;
 
    if ((tmp_fp = fopen(tmpforwardfile, "w")) == NULL)
        return;
 
    // Make the buffer 1024. This is safe as there is no way the vacation
    // line could execeed 1024 bytes, and fgets will stop on newline.
    while (fgets(buf, 1024, fwd_fp)) {
        if ((strstr(buf, "/usr/bin/vacation") == 0) &&
                (strstr(buf, "/usr/ucb/vacation") == 0)) {
                if (SafeWrite(fileno(tmp_fp), buf, strlen(buf)) < 0) {
                        fclose(fwd_fp);
                        fclose(tmp_fp);
                        unlink(tmpforwardfile);
                        return;
                }
		else wrote_something = TRUE;
        }
    }

    fclose(fwd_fp);
    fclose(tmp_fp);

    // Backup the file before renaming the tmp file in case of error
    if (wrote_something) {
    	this->backupFile(forwardfile);
    	// If rename fails then recover the forward file. 
    	if (rename (tmpforwardfile, forwardfile) < 0) {
		recoverForwardFile(forwardfile);
    		unlink(tmpforwardfile);
   	}
	else 
		removeBackupFile(forwardfile);
    }
    else {
	// If nothing written, remove the .forward file and .forward.tmp file
	unlink(forwardfile);
    	unlink(tmpforwardfile);
    }
}

void
VacationCmd::addVacationToForwardFile(char *replyInterval)
{
    char forwardfile[512], buf[1024], tmpforwardfile[512];
    int	fd;
    Boolean forward_exists = FALSE;

    // If vacation is already in the .forward, just start vacation and we are done
    if (vacationAlreadyStarted()) {
    	system("/usr/bin/vacation -I");
	return;
    }
    passwd pw;
    GetPasswordEntry(pw);

    // Make sure there is no Forward backup from a previous Vacation startup.
    // Maybe they manually removed the .forward file instead of turning it off 
    // thru the GUI.
    sprintf(tmpforwardfile, "%s/.forward..BACKUP", pw.pw_dir);
    if (SafeAccess(tmpforwardfile, F_OK) == 0) 
	unlink (tmpforwardfile);

    // Make sure no tmp file either
    sprintf(tmpforwardfile, "%s/.forward.tmp", pw.pw_dir);
    if (SafeAccess(tmpforwardfile, F_OK) == 0) 
	unlink(tmpforwardfile);

    sprintf(forwardfile, "%s/.forward", pw.pw_dir);

    // If forward file exists, copy to tmp file
    if (SafeAccess(forwardfile, F_OK) == 0) {
	sprintf(buf, "cp %s %s", forwardfile, tmpforwardfile);
	forward_exists = TRUE;
	system (buf);
    }

    // Write to tmp file first
    if ((fd = SafeOpen(tmpforwardfile, O_RDWR | O_CREAT | O_APPEND)) > 0) {
    	createForwardVacationStr(buf, pw.pw_name, replyInterval);

    	int len = strlen(buf);
    	if (SafeWrite(fd, buf, len) < len) {
	    	SafeClose(fd);
		unlink(tmpforwardfile);
	    	return;
	}
    	SafeClose(fd);
    }
    else return;

    // Forward file exists, so back it up just to be safe. 
    if (forward_exists)
	this->backupFile(forwardfile);

    // Now, rename the tmp forward file to forward file. If it fails then restore 
    // the original forward file.
    if (rename(tmpforwardfile, forwardfile) < 0) {
    	if (forward_exists)
		recoverForwardFile(forwardfile);
        unlink(tmpforwardfile);
    }
    else {
    	if (forward_exists)
		removeBackupFile(forwardfile);
    	system("/usr/bin/vacation -I");
    }
}

int
VacationCmd::backupFile(
    char *file
)
{
	char buf[512];

	sprintf(buf, "%s..BACKUP", file);

	if (rename(file, buf) < 0) 
	    return(-1);

	return(0);
}
void
VacationCmd::removeBackupFile(
    char	*file
)
{
	char buf[512];

	sprintf(buf, "%s..BACKUP", file);

	unlink(buf); 
}

int
VacationCmd::recoverForwardFile(
    char	*file
)
{
	char buf[512];

	sprintf(buf, "%s..BACKUP", file);

	if (rename(buf, file) < 0) 
		return(-1);

	return(0);
}

void
VacationCmd::parseVacationMessage()
{
    char		messagefile[256];
    passwd pw;
    int		fd;

    DtMailGenDialog	*dialog;
    char dialog_text[1024*4];
    DtMailEnv error;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMailBuffer mbuf;

    dialog = theRoamApp.genDialog();

    GetPasswordEntry(pw);

    memset(messagefile, 0, sizeof(messagefile));
    sprintf(messagefile, "%s/.vacation.msg", pw.pw_dir);
    char * fullpath = d_session->expandPath(error, messagefile);

    _subject = NULL;
    _body = NULL;

    fd = SafeOpen(fullpath, O_RDONLY);
    free(fullpath);

    if (fd < 0) // File doesn't exist
	return;

    struct stat buf;
    if (SafeFStat(fd, &buf) < 0) {

	sprintf(dialog_text, 
		catgets(DT_catd, 1, 105, "Cannot open .vacation.msg file -- No write permission."));
	dialog->setToQuestionDialog("Mailer", dialog_text);
	dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITEVACATION);
	    
	SafeClose(fd);
	return;
    }		

    int page_size = (int)sysconf(_SC_PAGESIZE);
    size_t map_size = (int) (buf.st_size + 
			    (page_size - (buf.st_size % page_size)));

    int free_buf = 0;
    mbuf.size = buf.st_size;
    mbuf.buffer = mmap(0, map_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mbuf.buffer == (char *)-1) {
	free_buf = 1;
	mbuf.buffer = new char[mbuf.size];
	if (mbuf.buffer == NULL) {
	    dialog->setToErrorDialog(catgets(DT_catd, 3, 59, "No Memory"),
				     catgets(DT_catd, 3, 60, "There is not enough memory to load the existing .vacation.msg file."));
	    dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOLOADVACATION);
	    SafeClose(fd);

	    _subject = NULL;
	    _body = NULL;

	    return;
	}

	if (SafeRead(fd, mbuf.buffer, (unsigned int)mbuf.size) < mbuf.size) {
	    dialog->setToErrorDialog(catgets(DT_catd, 3, 61, "Mailer"),
				     catgets(DT_catd, 3, 62, "The existing .vacation.msg file appears to be corrupt."));
	    dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPCORRUPTVACATION);
	    SafeClose(fd);
	    delete mbuf.buffer;

	    return;
	}
    }

    // Now we ask the library to parse it. If this fails for any reason, this
    // is not a message, so we give up.
    //
    DtMail::Message * msg = d_session->messageConstruct(error,
							DtMailBufferObject,
							&mbuf,
							NULL,
							NULL,
							NULL);
    SafeClose(fd);

    if (error.isSet()) 
	return;
    DtMail::Envelope * env = msg->getEnvelope(error);
    DtMailHeaderHandle hnd;

    int hcount = 0;
    char * name;
    DtMailValueSeq value;

    for (hnd = env->getFirstHeader(error, &name, value); error.isNotSet() && hnd;
	    hnd = env->getNextHeader(error, hnd, &name, value)) {
	    
	    if (!strcmp(name, "Subject") == 0) {
		continue;
	    }
	    else {

		int max_len = 0;
		for (int slen = 0; slen < value.length(); slen++) {
		    max_len += strlen(*(value[slen]));
		}

		char * new_str = new char[max_len + (value.length() * 3)];

		strcpy(new_str, "");
		for (int copy = 0; copy < value.length(); copy++) {
		    if (copy != 0) {
			strcat(new_str, " ");
		    }

		    strcat(new_str, *(value[copy]));
		}

		_subject = strdup(new_str);
		value.clear();
		free(name);
		break;
	    }
    }

    DtMail::BodyPart * bp = msg->getFirstBodyPart(error);
    if (error.isNotSet()) {
	unsigned long length;
    	void *body = NULL;

      	bp->getContents(error,
			    &body,
			    &length,
			    NULL,
			    NULL,
			    NULL,
			    NULL);
    	if (body)
		_body = strdup((char*)body);
    }
}

int
VacationCmd::handleMessageFile()
{
    DtMailGenDialog	*dialog = theRoamApp.genDialog();
    int answer=0, fd;
    char dialog_text[1024*4],	messagefile[256];

    BufferMemory buf(4096);

    // Check if a .forward file exists.  
    passwd pw;
    GetPasswordEntry(pw);

    sprintf(messagefile, "%s/.vacation.msg", pw.pw_dir);

    // See if the messagefile exists.
    // If it doesn't create one and throw in the text found in the
    // properties sheet.  If no text found, use default template.

    int msg_file_exists = SafeAccess(messagefile, F_OK);

    if (msg_file_exists >= 0) {
	strcpy(dialog_text, 
		catgets(DT_catd, 1, 106, ".vacation.msg file exists.  Replace with new text?"));
	dialog->setToQuestionDialog("Mailer", dialog_text);
	answer = dialog->post_and_return(DTMAILHELPEXISTSVACATION);

	if (answer == 1) {
		// backup the messageFile
		this->backupFile(messagefile);
	}
    }
    
    // If the file doesn't exist or if the user has okayed creation

    if (msg_file_exists < 0 || answer == 1) {

	fd = SafeOpen(messagefile, O_WRONLY | O_CREAT);
	if (fd < 0) {
	    strcpy(dialog_text, 
		catgets(DT_catd, 1, 107, "Cannot open .vacation.msg file -- No write permission."));
	    dialog->setToQuestionDialog("Mailer", dialog_text);
	    answer = dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPERROR);
	    
	    // Handle dialog indicating file not writable
	    return (-1);	
	}		
	SafeFChmod (fd, 0644);

	if (!_subject)
	    _subject = strdup(catgets(DT_catd, 1, 108, "I am on vacation"));
        buf.appendData("Subject: ", 9);
        buf.appendData(_subject, strlen(_subject));
	buf.appendData("\nPrecedence: junk\n\n", 19);
	
	if (!_body)
	    _body = strdup(catgets(DT_catd, 1, 109, "I'm on vacation.\nYour mail regarding \"$SUBJECT\" will be read when I return.\n"));

	buf.appendData(_body, strlen(_body));
	if (_body && _body[strlen(_body) - 1] != '\n') 
	    buf.appendData("\n", 1);

	buf.iterate(writeToFileDesc, fd);
	
	SafeClose(fd);
    }
    return(0);
}
void
VacationCmd::resetVacationStatus()
{
  // Cleanup vacationstartatjobid  and vacationstopatjobid resources

  char *sAtIdValue=NULL, *eAtIdValue=NULL, *date=NULL;
  time_t nowTick=0, dateTick=0;
  struct tm tm;
  DtMailEnv error;
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMail::MailRc * mail_rc;
  Boolean update = FALSE;

  if (d_session)
        mail_rc = d_session->mailRc(error);
  else
        return;

  mail_rc->getValue(error, "vacationstartatjobid", (const char **)&sAtIdValue);
  mail_rc->getValue(error, "vacationstopatjobid", (const char **)&eAtIdValue);
  Boolean vacation_started = vacationAlreadyStarted();

  if (sAtIdValue && *sAtIdValue) {
        // If vacation is already started, we dont need to hang onto the
        // the start at job id
        if (vacation_started) {
                mail_rc->removeValue(error, "vacationstartatjobid");
                free(sAtIdValue);
                sAtIdValue = NULL;
                update = TRUE;
        }
        // Vacation hasnt started, but we have an at job id, check if
        // vacation start date has passed, If so then vacation has alread
        // come and gone, so remove both stop and start job at ids.
        else {
                mail_rc->getValue(error, "vacationstartdate", (const char **)&date);
                if (date && *date) {
                        strptime (date, catgets(DT_catd, 1, 238, "%m %d %Y"), &tm);
                        dateTick = mktime (&tm);
                        nowTick = time((time_t *) 0);
                }
                // If no start vacation date or if the start date has passed...

                if (nowTick >= dateTick) {
                        mail_rc->removeValue(error, "vacationstartatjobid");
                        mail_rc->removeValue(error, "vacationstopatjobid");
                        free(sAtIdValue);
                        sAtIdValue = NULL;
                        if (eAtIdValue) {
                                free(eAtIdValue);
                                eAtIdValue = NULL;
			}
                        update = TRUE;
                }
                if (date) { free(date); date = NULL; }
        }
  }
  // If no start date but end date: This could occur only if vacation was already turned
  // on. If vacation is not on, then that means the id is obsolete.
  else if (eAtIdValue && *eAtIdValue) {
        if (!vacation_started) {
                mail_rc->removeValue(error, "vacationstopatjobid");
                free(eAtIdValue);
                eAtIdValue = NULL;
                update = TRUE;
        }
  }

  // startatjobid will exist only if there is a start job pending
  // Set timers to update dislay and delete at job ids when vacation
  // is turned on and off.

  if (sAtIdValue && *sAtIdValue) {
        if (!dateTick) {
                mail_rc->getValue(error, "vacationstartdate", (const char **)&date);
                if (date && *date) {
                        strptime (date, catgets(DT_catd, 1, 238, "%m %d %Y"), &tm);
                        dateTick = mktime (&tm);
                }
                if (date) { free(date); date = NULL; }
        }
        if (dateTick) {
                if (!nowTick)
                        nowTick = time((time_t *) 0);
                setStartVacationTimeout(dateTick - nowTick);
        }
  }
  dateTick = 0;
  if (eAtIdValue && *eAtIdValue) {
        mail_rc->getValue(error, "vacationenddate", (const char **)&date);
        if (date && *date) {
                strptime (date, catgets(DT_catd, 1, 238, "%m %d %Y"), &tm);
                dateTick = mktime (&tm);
        }
        if (date)  free(date);
        if (dateTick) {
                if (!nowTick)
                        nowTick = time((time_t *) 0);
                setStopVacationTimeout(dateTick - nowTick);
        }
  }
  // Reset vacationhost if no start or stop at jobs
  if ((!eAtIdValue || !(*eAtIdValue)) && (!sAtIdValue || !(*sAtIdValue))) {
	char *value=NULL;
	mail_rc->getValue(error, "vacationatjobhost", (const char **)&value);
	if (value) {
		if (*value) {
        		mail_rc->removeValue(error, "vacationatjobhost");
        		update = TRUE;
		}
		free(value);
	}
  }

  if (update) 
        mail_rc->update(error);  
 
  if (sAtIdValue) free(sAtIdValue);
  if (eAtIdValue) free(eAtIdValue);
}


