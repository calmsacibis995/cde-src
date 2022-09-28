/*
 *+SNOTICE
 *
 *	$Revision: 1.341 $
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
 *	Copyright 1993, 1994, 1995, 1996 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)RoamCmds.C	1.341 06/16/97"
#endif

#ifdef __ppc
#include <DtMail/Buffer.hh>
#endif
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/FileSBP.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/PanedW.h>
#include <Xm/Form.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>
#include <Dt/Dts.h>
#include <Dt/Action.h>
#include <Dt/Wsm.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/MessageEnvelope.hh>
#include <SDtMail/MessageBody.hh>
#include <SDtMail/ContentBuffer.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/DataTypeUtility.hh>
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
#include <DtMail/AlternatesListUiItem.hh>
#include <DtMail/options_util.h>
#include "dtmailopts.h"

#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <libgen.h>
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
#include "PasswordDialogManager.h"

extern const char *XDTMAILEMPTYHEADERS;
extern const char *XDTMAILFROMHEADER;
extern const char *XDTMAILDATEHEADER;

extern forceUpdate( Widget );
extern SdmBoolean props_changed;

extern int dtmail_mapped;

RoamCmd::RoamCmd
(char *name, char *label,  int active, RoamMenuWindow *window)
: NoUndoCmd (name, label, active)
{
    _menuwindow = window;
}

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
    SdmError error;
    // Initialize the mail_error.
    error.Reset();
    _menuwindow->checkForMail(error);    
}

void 
OpenMailboxCmd::setMailboxName(char *filename)
{
    if (_container_name != NULL)
      free(_container_name);
    if (_server != NULL)
      free(_server);

    theRoamApp.fromLabelToServerMb(filename, &_server, &_container_name);
}

OpenMailboxCmd::OpenMailboxCmd(
    char *name,
    char *label,
    int active,
    RoamMenuWindow *rmw,
    char *filename
) : RoamCmd (name, label, active, rmw)
{
    _menuwindow = rmw;
    _container_name = NULL;
    _server = NULL;
    if (filename == NULL) 
      return;
      
    setMailboxName(filename);
}


OpenMailboxCmd::~OpenMailboxCmd()
{
    if (_container_name != NULL)
      free(_container_name);
    if (_server != NULL)
      free(_server);
}

void 
OpenMailboxCmd::changeContainer( char * container_name ) 
{ 
  if (_container_name) 
    free (_container_name); 
  if (container_name) 
    _container_name = strdup(container_name);
  else 
     _container_name = NULL; 
}

void 
OpenMailboxCmd::changeServer( char * server ) 
{ 
  if (_server) 
    free (_server); 
  if (server) 
    _server = strdup(server);
  else 
    _server = NULL; 
}


void
OpenMailboxCmd::doit()
{
    char *name, *newdestname, fullpath[2048];
    SdmError error;

    // Make sure file name has full path.
    if (strcmp(_container_name, "INBOX") != 0) {
	name = strdup(_container_name);
	// We really need to make sure the mailbox has a full path at this
	// point because before the mailbox is opened in mail_file_selection()
	// we check a cache to see if the mailbox is already opened. This
	// cache expects a fullpath to the mailbox. Opening the login dialog
	// here if need be (in theory) will circumvent the need to open it later...

	if (!theRoamApp.resolvePath(&name, _server)) {
		// Canceled out of login dialog
		free(name);
		return;
	}
    }
    else name = strdup("INBOX");

    // RMW::mail_file_selection will return true if the mailbox was
    // already open.
    if (_menuwindow->mail_file_selection(name, _server) ||
	_menuwindow->inList(name, _menuwindow->moveContainerList()) != -1 ||
	_menuwindow->inList(name, _menuwindow->cachedmoveContainerList()) != -1) {
	free(name);
	return;
    }

    // Dont add record file or inbox to most recently used list!
    char *path = theRoamApp.getInboxPath();
    if (path && strcmp(path, name) != 0) {
	// Not Inbox...

	SdmMailRc *mailrc;
        MailConnection *mailConnection = theRoamApp.connection();
        mailConnection->connection()->SdmMailRcFactory(error, mailrc);
        MailRcSource *mrc = (MailRcSource*)new MailRcSource (mailrc, "record");
        char *value = (char*)mrc->getValue(), *expanded_val=NULL;
        delete mrc;

	SdmUtility::ExpandPath(error, expanded_val, value, *mailrc, 
			SdmUtility::kFolderResourceName);
	free(value);

	// record file is always local, so we can check for a slash here
	if (*expanded_val != '/') {
		char *folder = theRoamApp.getFolderDir(local, Sdm_True);
		if (strcmp(folder, "/") == 0)
			sprintf(fullpath, "/%s", expanded_val);
		else
			sprintf(fullpath, "%s/%s", folder, expanded_val);
		free(folder);
	}
	else
		strcpy(fullpath, expanded_val);
	free(expanded_val);

	// Not the record file...
	if (strcmp(fullpath, name) != 0) {
		SdmUtility::GetRelativePath(error, newdestname,
			name, *_menuwindow->get_mail_rc(), 
			(_server?
			SdmUtility::kImapFolderResourceName:
			SdmUtility::kFolderResourceName) );
		if (_server) {
			char newname[2048];
			sprintf(newname, "%s(%s)", newdestname, _server);
			_menuwindow->addToRecentList(newname);
		}
		else
			_menuwindow->addToRecentList(newdestname);
		free(newdestname);
     	}
     }
     free(name);
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
    SdmError error;

    assert(_menuWindow);

    _menuWindow->open(error);
    if (error) {
        // don't need to post error.  it's done by the rmw->open call.
        return;       
    }
    _done = Sdm_True;
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
    
    _done = Sdm_True;
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


// PasswordCmd

PasswordCmd::PasswordCmd(char *name,
			char *label,
			int active,
			RoamMenuWindow *window)
			: RoamCmd (name, label, active, window)
{
    _menuWindow = window;
}

void
PasswordCmd::ok(void *)
{
}

void
PasswordCmd::cancel(void *)
{
}

void
PasswordCmd::apply(void *)
{
}

void
PasswordCmd::help(void *)
{
}

void
PasswordCmd::doit()
{
}

// FindCmd

FindCmd::FindCmd(char *name, char *label, int active, RoamMenuWindow *window )
    : RoamCmd ( name, label, active, window )
{
}

void
FindCmd::doit()
{
    _menuwindow->get_find_dialog();
}

ViewsCmd::ViewsCmd(char *name, char *label, int active, RoamMenuWindow *rmw)
    : RoamCmd ( name, label, active, rmw)
{
}

void ViewsCmd::doit()
{
    _menuwindow->get_views_dialog(Sdm_True);
}

OptionsCmd::OptionsCmd( 
		      char *name, 
		      char *label,
		      int active, 
		      Widget widget, 
		      OptionsOp op
		      ) :  NoUndoCmd( name, label, active )
{
    _op = op;
    _widget = widget;
}

void
OptionsCmd::doit()
{
    DtbOptionsDialogInfo od;
    OptCmd *oc=NULL;
    int ret = 0;
    Atom wpaWorkspace;

    oc = (OptCmd*)theRoamApp.mailOptions();
    if (oc == NULL) return; // This should never happen

    if ((od = oc->optionsDialog()) == NULL) {
        // Create the Options Dialog
	oc->execute();
	od = oc->optionsDialog();
	if (od == NULL) return; // This should never happen
    }
    else {
	if ( _widget == NULL )
		_widget = od->dialog;

        DtWsmGetCurrentWorkspace(XtDisplay(_widget),
                                     XtWindow(_widget),
                                     &wpaWorkspace);
        DtWsmSetWorkspacesOccupied ( XtDisplay(od->dialog),
                                        XtWindow (od->dialog),
                                        &wpaWorkspace,
                                        1);
    }

    // Now set the category...
	
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
	oc->setWindowAlive(Sdm_True);
	props_changed = Sdm_False;
    }
    XtManageChild(od->dialog_shellform);
    XtPopup(od->dialog, XtGrabNone);
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
    SdmError mail_error;

	mail_error.Reset();

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
    _menuwindow->list()->deleteSelected(Sdm_False);
    
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

    if (_menuwindow->list()->get_num_total_deleted_messages()) {
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
    _operation = op;
    _container_name = NULL;
    _server = NULL;

    theRoamApp.fromLabelToServerMb(name, &_server, &_container_name);
}

void
MoveCopyMenuCmd::doit()
{
    SdmError mail_error;

    // Initialize mail_error.
    mail_error.Reset();

    if (_operation == DTM_MOVE) {
      _menuwindow->list()->copySelected(mail_error, _container_name, _server, Sdm_True, Sdm_False);
      // We had an error in moving the message to a container!
    } else {
      _menuwindow->list()->copySelected(mail_error, _container_name, _server, Sdm_False, Sdm_False);
      // We had an error in copying the message to a container!
    }
    if ( mail_error ) {
      _menuwindow->postErrorDialog(mail_error);
    }
}

MoveCopyMenuCmd::~MoveCopyMenuCmd()
{
    if (_container_name != NULL)
      free(_container_name);
    if (_server != NULL)
      free(_server);
}

void 
MoveCopyMenuCmd::changeContainer( char * container_name ) 
{ 
  if (_container_name) 
    free (_container_name); 
  if (container_name) 
    _container_name = strdup(container_name);
  else 
     _container_name = NULL; 
}

void 
MoveCopyMenuCmd::changeServer( char * server ) 
{ 
  if (_server) 
    free (_server); 
  if (server) 
    _server = strdup(server);
  else 
    _server = NULL; 
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
    SdmError error;
    char *inbox_path, *inbox_server=NULL;

    inbox_path = theRoamApp.getInboxPath();
    if (inbox_path && strcmp(inbox_path, "INBOX") == 0)
    	inbox_server = theRoamApp.getInboxServer();

    if (!error) {
      _menuwindow->list()->copySelected(error, 
        inbox_path, inbox_server, Sdm_True, Sdm_True);
    }
    
    if (error) {
      _menuwindow->postErrorDialog(error);
    }
}

MoveToInboxCmd::~MoveToInboxCmd()
{
}

// Copy the selected messages to the Inbox.
CopyToInboxCmd::CopyToInboxCmd( char *name,
				char *label,
				int active,
				RoamMenuWindow *window )
    : RoamCmd (name, label, active, window)
{
    _menuwindow = window;
}
 
void
CopyToInboxCmd::doit()
{
    SdmError error;
    char *inbox_path, *inbox_server=NULL;

    inbox_path = theRoamApp.getInboxPath();
    if (inbox_path && strcmp(inbox_path, "INBOX") == 0)
    	inbox_server = theRoamApp.getInboxServer();

    _menuwindow->list()->copySelected(error, 
        inbox_path, inbox_server, Sdm_False, Sdm_True);

    if ( error ) {
      _menuwindow->postErrorDialog(error);
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
			   SdmBoolean viaDeleteList
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
    MsgScrollingList *list = _menuwindow->list();
    MsgHndArray *deleted_messages;
    SdmError mail_error;
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    
    // Initialize the mail_error.
    mail_error.Reset();

    if (_fromList) {
      // Create the Deleted Messages Dialog

      if (_undelFromList) {
          _undelFromList->popped_up();
      } else {
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
          _undelFromList->loadMsgs(mail_error,
                 deleted_messages,
                 _num_deleted);
          if (mail_error) {
	    genDialog->post_error(mail_error, DTMAILHELPCANNOTDISPLAYDELMSGS,
				  catgets(DT_catd, 3, 132, "Mailer cannot display the message headers in the Deleted Messages window."));
            return;
          }
        }
        // Display the dialog
        _undelFromList->popped_up();
      }
    } else {
      // Although we don't display the Deleted Message Dialog here, we
      // need to make sure that it gets updated for the next time
      // we bring it up.
      list->undelete_last_deleted(mail_error);
      if (mail_error) {
	genDialog->post_error(mail_error, DTMAILHELPCANNOTUNDELETELASTDELMSG, 
			      catgets(DT_catd, 3, 131, "Mailer cannot undelete the last message you marked for deletion."));
	return;
      }
    }
}

MoveCopyCmd::MoveCopyCmd( 
			  char *name, 
			  char *label,
			  SdmBoolean active, 
			  FileCallback move_callback, 
			  FileCallback copy_callback, 
			  FileCallback2 dirSearchcb, 
			  struct OpenStruct *os,
			  Widget parent,
			  char *ok_label,
			  SdmBoolean copy )
: CustomSelectFileCmd (
		 name,
		 label,
		 catgets(DT_catd, 1, 254, "Mailer - Select Mailboxes"),
		 ok_label,
// 		 catgets(DT_catd, 1, 90, "Move"),
		 active, 
		 copy ? copy_callback : move_callback, 
		 dirSearchcb, 
		 os,
		 parent)
{
    // GL - The _copy_callback now refers to the non-default button callback.  If we are
    // creating the Copy FSB, that means the ok button does a copy and the new button
    // will do a move.
    _copy_callback = copy ? move_callback : copy_callback;
    _menuWindow = ((struct OpenStruct*)os)->rmw;
    _copy_is_default = copy;
}

// We now maintain separate dialogs for the Move and Copy FSB's.  This
// is because there were problems changing the default action from move
// to copy.
void
MoveCopyCmd::doit()
{
    Widget copy_button;

    if ( _fileBrowser == NULL) {

    	CustomSelectFileCmd::doit();

    	XtVaCreateWidget("Unused Button", 
		xmPushButtonWidgetClass, _fileBrowser,
		NULL);

	if (_copy_is_default) {
	  // If the mailbox is read-only, we should grey out the Move button
	  copy_button = XtVaCreateManagedWidget(catgets(DT_catd, 1, 90, "Move"),
				xmPushButtonWidgetClass,  _fileBrowser,
				XmNsensitive, _menuWindow->mailBoxWritable(),
				NULL);
	}
	else {
	  copy_button = XtVaCreateManagedWidget(catgets(DT_catd, 1, 237, "Copy"),
				xmPushButtonWidgetClass,  _fileBrowser,
				NULL);
	}

    	XtAddCallback(copy_button,
		      XmNactivateCallback,
		      &MoveCopyCmd::fileSelectedCallback2,
		      (XtPointer) this );
    } else {
// 	SelectFileCmd::doit();
	CustomSelectFileCmd::doit();
    }
}

void
MoveCopyCmd::fileSelectedCallback2 ( Widget, XtPointer clientData, XtPointer)
{
    MoveCopyCmd * obj = (MoveCopyCmd *) clientData;
    char     *fname  = NULL;
    XmString  xmstr;  // The selected file
    int       status = 0;
    
    // Get the value
    XtVaGetValues(obj->_fileBrowser, XmNdirSpec, &xmstr, NULL);
    if ( xmstr ) {   // Make sure a file was selected
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
    // Bring the file selection dialog down.
    XtUnmanageChild ( obj->_fileBrowser );   
}

void
MoveCopyCmd::fileSelected2 (char *filename )
{
    if ( _copy_callback )
	_copy_callback ( _clientData, filename );
}

CustomSelectFileCmd::CustomSelectFileCmd( 
			  char *name, 
			  char *label,
			  char *title,
			  char *ok_label,
			  SdmBoolean active, 
			  FileCallback ok_callback, 
			  FileCallback2 dirSearchcb, 
			  void *openstruct,
			  Widget parent
			  )
: SelectFileCmd (
		 name,
		 label,
		 title,
		 ok_label,
		 active, 
		 ok_callback, 
		 dirSearchcb, 
		 openstruct,
		 this,
		 parent)
{
    _local_toggle_widget = NULL;
    _imap_toggle_widget = NULL;
    _server_textfield = NULL;
    _radioBox = NULL;
    _islocal = -1;
    _server = NULL;
    _cbs = NULL;
}

void
CustomSelectFileCmd::doit()
{
    Widget form;
    int n = 0;
    Arg args[15];
    XmString lstr;
    SdmBoolean already_created = Sdm_False;
 
    if (_fileBrowser)
        already_created = Sdm_True;
    else
        setDir();
 
    // If already created, this call will manage and bring to front
    SelectFileCmd::doit();
 
    // All the extra widgets were created already
    if (already_created) {
        // The server value may have changed via the login dialog
        setServer(NULL);
        switchToggle(isLocal() ? 1 : 0);
        return;
    }

    XtVaSetValues(_fileBrowser,
	XmNchildPlacement, XmPLACE_TOP,
	NULL);

    // Create local/IMAP widget
    form = XmCreateForm(_fileBrowser, "Work_Area",  NULL, 0);
    XtSetArg(args[n], XmNradioBehavior, True);  ++n;
    XtSetArg(args[n], XmNentryClass, xmToggleButtonWidgetClass);  ++n;
    XtSetArg(args[n], XmNnumColumns, 1);  ++n;
    XtSetArg(args[n], XmNspacing, 0);  ++n;
    XtSetArg(args[n], XmNrowColumnType, XmWORK_AREA);  ++n;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL);  ++n;
    XtSetArg(args[n], XmNpacking, XmPACK_TIGHT);  ++n;
    XtSetArg(args[n], XmNisHomogeneous, True);  ++n;
    _radioBox = XmCreateRadioBox(form, "local_imap_choice", args, n);
    lstr = XmStringCreateLocalized(catgets(DT_catd, 16, 2, "Local"));
    _local_toggle_widget = XtVaCreateManagedWidget("local_item",
                xmToggleButtonWidgetClass,
                _radioBox,
		XmNset, True,
                XmNlabelString, lstr,
                NULL);
    XmStringFree(lstr);

    lstr = XmStringCreateLocalized(catgets(DT_catd, 16, 3, "IMAP Server:"));
    _imap_toggle_widget = XtVaCreateManagedWidget("server_item",
                xmToggleButtonWidgetClass,
                _radioBox,
		XmNset, False,
                XmNlabelString, lstr,
                NULL);
    XmStringFree(lstr);
    n=0;
    XtSetArg(args[n], XmNmaxLength, 320);  ++n;
    XtSetArg(args[n], XmNeditable, True);  ++n;
    XtSetArg(args[n], XmNcursorPositionVisible, True);  ++n;
    XtSetArg(args[n], XmNcolumns, 10);  ++n;
    _server_textfield = XmCreateTextField(form, "IMAP_server_tf", args, n);

    XtVaSetValues(_server_textfield,
                XmNleftAttachment,XmATTACH_WIDGET,
		XmNleftWidget, _radioBox,
  		XmNrightAttachment,XmATTACH_FORM,
	  	XmNtopAttachment,XmATTACH_FORM,
	  	XmNbottomAttachment, XmATTACH_FORM,
	  	XmNbottomOffset, 2,
	  	XmNrightOffset, 2,
	  	XmNtopOffset, 2,
	  	XmNleftOffset, 2,
	  	NULL);


    XtAddCallback(_local_toggle_widget,
		XmNvalueChangedCallback, &CustomSelectFileCmd::localButtonCB,
		this);
    XtAddCallback(_fileBrowser,
		XmNcancelCallback, &CustomSelectFileCmd::cancelCB,
		_clientData);   // _clientData is the OpenStruct (has server name and rmw)
    XtAddCallback(_imap_toggle_widget,
		XmNvalueChangedCallback, &CustomSelectFileCmd::imapButtonCB,
		this);
    XtAddCallback(_server_textfield,
		XmNactivateCallback, (XtCallbackProc) XmProcessTraversal,
		(XtPointer) XmTRAVERSE_NEXT_TAB_GROUP);

    setServer(NULL);  
    switchToggle(isLocal() ? 1 : 0);

    XtManageChild (_fileBrowser);
    XtManageChild (form);
    XtManageChild (_radioBox);
    XtManageChild (_server_textfield);
}
void
CustomSelectFileCmd::setDir()
{
    SdmError error;
    char *folder, *value;
    SdmMailRc *mail_rc;
    SdmConnection *con = theRoamApp.connection()->connection();
    con->SdmMailRcFactory(error, mail_rc);
    assert(!error);

    if (isLocal() == 1) {
	_server = NULL;
    } else {
	mail_rc->GetValue(error, "imapfolderserver", &value);
	if (_server != NULL) free (_server);
	if (!error) {
	    _server = value; 
	    _islocal = 0;
	}
	else {
	    _server = NULL;  
	    _islocal = 1;
	    if (value)
		free(value);
	}
    }
 
    folder = theRoamApp.getFolderDir(_server ? imap : local, Sdm_True);
    setDirectory(folder);
    free(folder);
}
SdmBoolean
CustomSelectFileCmd::login()
{
    SdmBoolean ret = Sdm_True;
    if (!isLocal()) {
        thePasswordDialogManager->setImapServer(strdup(_server));
	if (!(ret = thePasswordDialogManager->loginDialog()))
		XtUnmanageChild(thePasswordDialogManager->baseWidget());
    }
    return ret;
}
char*
CustomSelectFileCmd::setLocalContext(XmFileSelectionBoxCallbackStruct *cbs)
{
	char *dir;
	setLocal(1);
        dir = resetDir(cbs);
        switchToggle(1);
	callDefSearchFunc(cbs);
	return dir;

}
void
CustomSelectFileCmd::callDefSearchFunc(XmFileSelectionBoxCallbackStruct *cbs)
{
	// Have to fake out the FSB into thinking soemthing has
	// changed, so we unset and then reset the dirSearchProc.
	// Otherwise the default function wont get called. (hack)
	FileCallback2 dp = getDefSearchProc();
	XtVaSetValues(_fileBrowser, XmNdirSearchProc, NULL, NULL);
	dp(_fileBrowser, cbs);
	XtVaSetValues(_fileBrowser,
		XmNdirSearchProc,
		CustomSelectFileCmd::doDirSearch, NULL);
}

void
CustomSelectFileCmd::cancelCB(Widget,
				XtPointer client_data,
				XtPointer)
{
    struct OpenStruct *os = (struct OpenStruct *) client_data;
    RoamMenuWindow *rmw = (RoamMenuWindow *) os->rmw;
    if (rmw == NULL) 
	return;
    if (rmw->GoAway()) {
	delete rmw;
    }
    if (theRoamApp.num_windows() == 0 ||
	theRoamApp.num_windows() == theCompose.numNotInUse())
        theRoamApp.shutdown();
}

void
CustomSelectFileCmd::imapButtonCB(Widget,
                        XtPointer client_data,
                        XtPointer )
{
    	CustomSelectFileCmd * csfc = (CustomSelectFileCmd *)client_data;
	
    	csfc->switchToggle(0);
}
void
CustomSelectFileCmd::localButtonCB(Widget,
                        XtPointer client_data,
                        XtPointer )
{
    	CustomSelectFileCmd * csfc = (CustomSelectFileCmd *)client_data;

    	csfc->switchToggle(1);
}
char *   
CustomSelectFileCmd::getServerValue()
{
        char *val = NULL;
        if (_server_textfield)
                XtVaGetValues(_server_textfield, XmNvalue, &val, NULL);
        return val;
}
void
CustomSelectFileCmd::setServerVar(char *server)
{
	if (server) {
		if (_server)
			free(_server);
		_server = strdup(server);
	}
}
void
CustomSelectFileCmd::setServer(char *server)
{
	setServerVar(server);
        if (_server_textfield)
		if (_server)
                	XtVaSetValues(_server_textfield, XmNvalue, _server, NULL);
		else
                	XtVaSetValues(_server_textfield, XmNvalue, "", NULL);
}
void
CustomSelectFileCmd::switchToggle(int on)
{
        if (_local_toggle_widget) {
                XtVaSetValues(_local_toggle_widget, XmNuserData, on, NULL);
                XtVaSetValues(_imap_toggle_widget, XmNuserData, !on, NULL);
                XtSetSensitive(_server_textfield, !on);
    		XmToggleButtonSetState(_local_toggle_widget, 
			on ? TRUE : FALSE, FALSE);
    		XmToggleButtonSetState(_imap_toggle_widget, 
			on ? FALSE : TRUE, FALSE);
        }
}
void
CustomSelectFileCmd::setLocalFromWid ()
{
	if ( _local_toggle_widget )
		XtVaGetValues(_local_toggle_widget, XmNuserData, &_islocal, NULL);
}
char *
CustomSelectFileCmd::resetDir(XmFileSelectionBoxCallbackStruct *cbs)
{
	char *tmp, *dir;

        // When the FSB local/imap setting gets toggled, we have to explicitly
	// reset the directory value or FSB has wrong dir setting

	dir = theRoamApp.getFolderDir(local, Sdm_True);
	tmp = (char*)malloc(strlen(dir)+2);
	sprintf(tmp, "%s/", dir);
	free(dir);
	dir = tmp;
	tmp = (char*)XmStringCreateLocalized(dir);
	XtVaSetValues(_fileBrowser, XmNdirectory, tmp, NULL);

#if 0
	Widget wid = XtNameToWidget(_fileBrowser, "DirText");
	XtVaSetValues(wid, XmNvalue, dir, NULL);
#endif

	if (cbs->dir)
                XmStringFree(cbs->dir);
        cbs->dir = (unsigned char*)tmp;
	return (dir);
}
SdmBoolean 
CustomSelectFileCmd::inList(const char** list, int numInList, const char *str)
{
	for (int i=0; i < numInList; i++)
		if (strcmp(list[i], str) == 0)
			return Sdm_True;
	return Sdm_False;
}
int 
CustomSelectFileCmd::myStrCmp(const char **str1, const char **str2)
{
    return(strcasecmp(*str1, *str2));
}
// Callback function when the Update or double click occurs in FSB
void
CustomSelectFileCmd::doDirSearch(
        Widget fsb, XmFileSelectionBoxCallbackStruct *cbs)
{
    SdmError err;
    SdmSortIntStrL dirInfo;
    SdmString *reference;
    SdmString *pattern;
    SdmToken token;
    int count, i, j=1, k=0, numDirs=0, numFiles=0;
    SdmMessageStore *mstore;
    char *server, *dir, *filter, *filter2;
    XmStringTable dirStringTable=NULL, fileStringTable=NULL;
    const char **dirtable=NULL, **filetable=NULL;
    CustomSelectFileCmd *cmd_ptr;

    XtVaGetValues(fsb, XmNuserData, &cmd_ptr, NULL);

    // Save the toggle state to be used in the copy/move callbacks.
    cmd_ptr->setLocalFromWid();

    // The user may have  switched the imap/local setting since the last
    // update and therefore we need to update the directory setting.
    XmStringGetLtoR(cbs->dir, XmSTRING_DEFAULT_CHARSET, &dir);

    // If the setting is local, then just call the FSB built in default
    // directory search function.
    if (cmd_ptr->isLocal()) {
            cmd_ptr->callDefSearchFunc(cbs);
            free(dir);
            return;
    }

    SdmMailRc *mail_rc;
    SdmConnection *con = theRoamApp.connection()->connection();
    con->SdmMailRcFactory(err, mail_rc);
    // Now we know its IMAP.
    // Got to have a server. Get it from the FSB server text field
    if ((server = cmd_ptr->getServerValue()) == NULL || *server == '\0') {
            // If no FSB server setting, default to resource setting
            assert(!err);
            mail_rc->GetValue(err, "imapfolderserver", &server);
            // If no resource setting, put up error dialog
            if (err) {
                    DtMailGenDialog *genDialog = theRoamApp.genDialog();
                    genDialog->setToErrorDialog(catgets(DT_catd, 2, 13, "Mailer"),
                            catgets(DT_catd, 16, 8, "You did not specify a name for the IMAP server."));
                            genDialog->post_and_return(catgets(DT_catd, 3, 76, "OK"),
                            DTMAILHELPIMAPSERVER);
                    free(dir);
                    free(cmd_ptr->setLocalContext(cbs));
                    if (server) free(server);
                    return;
            }
            // This sets on FSB textfield and object variable.
            cmd_ptr->setServer(server);
    } else {
            // This just sets object variable
            cmd_ptr->setServerVar(server);
    }

    // Since the user could have changed the server setting, reset it on the
    // Password login dialog.
    thePasswordDialogManager->setImapServer(strdup(server));
    cmd_ptr->cbs(cbs);
    if (!cmd_ptr->login()) {
            // Canceled out of login dialog, so set to local context
            dir = cmd_ptr->setLocalContext(cbs);
            free(dir);
            free(server);
            return;
    }

    // The built in FSB is unix centric - if the user inputs a relative path 
    // in the dir field, the FSB prepends the current path and passes it to this
    // routine in cbs struct. This doesnt work if non unix based system. 
    // SOOO, we verify what is in the directory text field matches what is 
    // passed to this routine. If they dont match, we try to figure out if 
    // the FSB has prepended a directory. If user has input a forward slash as 
    // first char of directory, then the FSB wont prepend any path and the 
    // directory passed in and the directory in text field should be the same.
    //  If user has input relative path or char other than forward slash, 
    // FSB will prepend current dir, so these two directories will be different.
    // In this case, we check delimiter of server with delimiter of directory 
    // in dir text field. If same, we know the user is looking at a non unix 
    // based system, so we throw out the FSB directory passed to us and use 
    // what the user has input in the directory text field.

    Widget wid = XtNameToWidget(fsb, "DirComboBox");
    if (wid)
	// This is the text widget for the directory in the combo box
    	wid = XtNameToWidget(wid, "Text");

    if (!wid)
    	wid = XtNameToWidget(fsb, "DirText");

    if (wid) {
	char *directory;

    	XtVaGetValues(wid, XmNvalue, &directory, NULL);
	if (directory == NULL || *directory == '\0')
            mail_rc->GetValue(err, "imapfolder", &directory);

	// It should be:
	//directory = theRoamApp.getFolderDir(imap, Sdm_True);
	// but then we would change to default dir which we don't want
	// So, just leave the directory text entry empty and don't force
	// any "default" directory upon the user.

	if (directory == NULL || *directory == '\0') {
	    free(dir);
            if (server)
		free(server);
            return;
	}

	// The "dir" value always has the delimiter appended to it.
	if (strncmp(dir, directory, strlen(dir)-1) == 0 || 
		strcmp(dir, directory) == 0)
		free(directory);
	else {
		// Value in the directory text field does not match value
 		// passed to this routine...
		const char *delim = thePasswordDialogManager->delimiter(server);
		if (delim == NULL) 
			free(directory);
		else if (delim[0] == directory[0]) {
			XtVaSetValues(wid, XmNvalue, directory, NULL);
			XmString lstr = XmStringCreateLocalized(directory);
			XmStringFree(cbs->dir);
			cbs->dir = lstr;
    			cmd_ptr->cbs(cbs);
			free(dir);
			dir = directory;
		}
	}
    }
    // Have to reset the directory explicitly every time. FSB doesnt do it (?)
    XtVaSetValues(fsb, XmNdirectory, cbs->dir, NULL);


    theRoamApp.osession()->osession()->SdmMessageStoreFactory(err, mstore);
    if (err) {
      DtMailGenDialog *genDialog = theRoamApp.genDialog();
      genDialog->post_error(err, DTMAILHELPIMAPERROR,
			    catgets(DT_catd, 16, 7, "An error occurred in accessing the IMAP server."));
      return;
    }

    token.SetValue("servicechannel", "cclient");
    token.SetValue("servicetype", "imap");
    token.SetValue("serviceclass", "messagestore");
    token.SetValue("serviceoption", "hostname", thePasswordDialogManager->imapserver());
    token.SetValue("serviceoption", "username", thePasswordDialogManager->user());
    token.SetValue("serviceoption", "password", thePasswordDialogManager->password());

    pattern = new SdmString("%");
    reference = new SdmString((const char*)dir);

    // Get the directories/files from the imap server
    mstore->StartUp(err);
    if (err != Sdm_EC_Success) {
            DtMailGenDialog *genDialog = theRoamApp.genDialog();
	    genDialog->post_error(err, DTMAILHELPIMAPERROR,
				  catgets(DT_catd, 16, 7, "An error occurred in accessing the IMAP server."));
            free(cmd_ptr->setLocalContext(cbs));
            delete pattern;
            delete reference;
            delete mstore;
            return;
    }
    while (mstore->Attach(err, token) != Sdm_EC_Success) {
            DtMailGenDialog *genDialog = theRoamApp.genDialog();
            genDialog->post_error(err);

            // user put in invalid userid, password, and/or server.
            // prompt the user to try again.
            if (err.IsErrorContained(Sdm_EC_CCL_CantLoginToIMAPServer)) {
            
              // we need to reset password so the loging dialog comes up.
              // this should be ok since the user can't see the characters
              // in the password anyway.
              thePasswordDialogManager->setPassword(NULL);
              if (!cmd_ptr->login()) {
                // Canceled out of login dialog, so set to local context
                dir = cmd_ptr->setLocalContext(cbs);
                free(dir);
                if (server) free(server);
		delete pattern;
		delete reference;
		delete mstore;
                return;
              } else {
                // user put in a new login data.  try the attach again.
                token.ClearValue("serviceoption", "hostname");
                token.ClearValue("serviceoption", "username");
                token.ClearValue("serviceoption", "password");
                token.SetValue("serviceoption", "hostname", thePasswordDialogManager->imapserver());
                token.SetValue("serviceoption", "username", thePasswordDialogManager->user());
                token.SetValue("serviceoption", "password", thePasswordDialogManager->password());
                err.Reset();
              }

            // some error occurred other than login error.  just return in this case.
            } else {
              free(cmd_ptr->setLocalContext(cbs));
              delete pattern;
              delete reference;
              delete mstore;
              return;
            }
    }
    if (server && strcmp(thePasswordDialogManager->imapserver(), server) != 0)
        cmd_ptr->setServer(thePasswordDialogManager->imapserver());

    mstore->ScanNamespace(err, dirInfo, *reference, *pattern);
    delete pattern;
    delete reference;

    if (err) {
            DtMailGenDialog *genDialog = theRoamApp.genDialog();
	    genDialog->post_error(err, DTMAILHELPIMAPERROR,
				  catgets(DT_catd, 16, 7, "An error occurred in accessing the IMAP server."));
            err = Sdm_EC_Success;
            mstore->Close(err);
	    delete mstore;
// Dont set local if they just entered a directory that doesnt exist...
#if 0
            free(cmd_ptr->setLocalContext(cbs));
#endif
            return;
    }
    mstore->Close(err);
    delete mstore;

    for (count = dirInfo.ElementCount(), i=0; i < count; i++) {
      if (dirInfo[i].GetNumber() & Sdm_NSA_cannotOpen)
              numDirs++;
      else
              numFiles++;
    }
    dirStringTable = (XmStringTable)XtMalloc(++numDirs * sizeof(XmString));
    dirtable = (const char**)malloc(numDirs * sizeof(const char*));
    if (numFiles > 0) {
      fileStringTable = (XmStringTable)XtMalloc(numFiles * sizeof(XmString));
      filetable  = (const char**)malloc(numFiles * sizeof(const char*));
    }

    // Always have at least a way to traverse up...
    const char*  updir = "..";
    dirtable[0] = updir;
    
    if (count > 0) {
        // Use the filter text for scanning the directories
        XtVaGetValues(XmFileSelectionBoxGetChild(fsb, XmDIALOG_FILTER_TEXT),
                 XmNvalue, &filter, NULL);
    	filter2 = getFixedMatchPattern(filter);
    	char *name = regcmp(filter2, (char *)0);
    	for (i=0; i < count; i++) {
	    SdmString tmpStr = dirInfo[i].GetString();
	    const char *t = tmpStr;
	    // Full Directory returned; strip off dir.
	    if (strncmp(t, dir, strlen(dir)) == 0)
		strcpy((char*)t, t+strlen(dir));
	    if (*t != NULL && *t != '.') {
		    if (dirInfo[i].GetNumber()&Sdm_NSA_cannotOpen
			&& !cmd_ptr->inList(dirtable, j, t))
		    		dirtable[j++] = t;
		    else if (!cmd_ptr->inList(filetable, k, t) && 
			     regex(name, (char*)t) != NULL)
					filetable[k++] = t;
                    }
    	}
    	if (name) 
		free(name);
    	qsort(dirtable, j, sizeof(char*), 
		(int (*)(const void *, const void *))myStrCmp);
    	qsort(filetable, k, sizeof(char*), 
		(int (*)(const void *, const void *))myStrCmp);
        for (i=0; i < j; i++) 
            dirStringTable[i] = XmStringCreateLocalized((char*)dirtable[i]);
        for (i=0; i < k; i++) 
            fileStringTable[i] = XmStringCreateLocalized((char*)filetable[i]);
    }
    XtVaSetValues(fsb,
            XmNdirListItems, dirStringTable,
            XmNdirListItemCount, j,
            XmNfileListItems, fileStringTable,
            XmNfileListItemCount, k,
            XmNlistUpdated, True,
            XmNdirectoryValid, False,
            NULL);

    while (j > 0)
            XmStringFree(dirStringTable[--j]);
    while (k > 0)
            XmStringFree(fileStringTable[--k]);
    free(dir);
    free(server);
    free(dirtable);
    XtFree((char*)dirStringTable);
    if (filetable)
    	free(filetable);
    if (fileStringTable) 
        XtFree((char*)fileStringTable);

}
char *
CustomSelectFileCmd::getFixedMatchPattern(char *pattern)
{
    register char *bufPtr ;
    char *          outputBuf ;
    char lastchar = '\0' ;
    int len ;

    outputBuf = (char*)calloc( 2, strlen( pattern) + 4);

    bufPtr = outputBuf ;
    *bufPtr++ = '^' ;

    while ((len = mblen( pattern, MB_CUR_MAX)) != 0) {
      if (    len <= 1    ) {
          if(    *pattern == '/'    )   
              break ;
          if(    lastchar == '\\'    )   
            *bufPtr++ = *pattern ;
          else {
            switch( *pattern ) {   
              case '.':
                *bufPtr++ = '\\' ;
                *bufPtr++ = '.' ;
                break ;
              case '?':
                *bufPtr++ = '.' ;
                break;
              case '*':
                *bufPtr++ = '.' ;
                *bufPtr++ = '*' ;
                break ;
              default:
                *bufPtr++ = *pattern ;
                break ;
            } 
          }
          lastchar = *pattern ;
          ++pattern ;
      } 
      else {
          strncpy( bufPtr, pattern, len) ;
          bufPtr += len ;
          pattern += len ;
          lastchar = '\0' ;
      } 
    }
    *bufPtr++ = '$' ;
    *bufPtr = '\0' ;

    return( outputBuf) ;
}

void
CustomSelectFileCmd::fileSelected (char *filename)
{
    // The server info is in the _clientData, which contains a handle to the
    // CustomSelectFileCmd as well as a RoamMenuWindow.
    if (_callback)
	_callback (_clientData, filename);
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
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    
    switch (status) {
      case DtACTION_INVOKED:
	break;

      case DtACTION_DONE:
	data = (PrintCmd *)clientData;
	data->_unregister_tmp_file(id);
 	if (theRoamApp.windowExists(data->_parent))
		data->_parent->message("");
	break;

      case DtACTION_FAILED:
	/*****
	sprintf(buf, catgets(DT_catd, 3, 144,
		"Drafts directory (%s) has no write permission."), draftsDir);
	*****/
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"),
		"Print action failed.");
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPERROR);
	break;

      default:
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
    SdmError	mail_error;
    MsgScrollingList	*list;
    DtActionArg	arg;
    
    DebugPrintf(1, "%s: printit\n", name());
    
    // Create tmp file.
    if ((p = tempnam("/tmp", "mail")) == NULL) {
	return;
    }
    
    mail_error.Reset();
    list = _parent->list();
    
    // Copy selected messages to a temp file
    list->copySelectedMsgs(mail_error, p);
    if (mail_error) {
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
	    SdmSystemUtility::SafeUnlink(f->file);
	    free(f->file);
	    free(f);
	    // Remove entry from list
	    _tmp_files.remove(n);
	    break;
	}
    }
    
    return;
}

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
    if (_default_trans == -1) {
	// This means get it from the resource. Resources are dynamic 
	// so we do this on the fly vs. set it up on creation.
	SdmMailRc *mailRc;
	SdmError error;
	theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
	assert(!error);
	_parent->send_message(mailRc->IsValueDefined("dontsendmimeformat"));
    }
    else
	_parent->send_message(_default_trans);

    // Exit if there are no more opened windows
    if ( theRoamApp.num_windows() == 0 ||
         theRoamApp.num_windows() == theCompose.numNotInUse() ) {
	theRoamApp.shutdown();
    }
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

// I18N - allow user to choose charset to display
// message in or send.  This button cycles thru
// a list of charsets which are displayed in the
// status area.
GetPossibleCharsetsCmd::GetPossibleCharsetsCmd(
		       char *name,
		       char *label,
		       int active,
		       void *window,
		       SdmBoolean incoming) 
: Cmd (name, label, active)
{
  _window = window;
  _incoming = incoming;
  _msg_num = -1;
  _pos = 0;
  _charset_list = NULL;
  _send_msgno = 0;
}

// I18N - allow user to choose charset
void
GetPossibleCharsetsCmd::doit()
{
DtMailGenDialog *genDialog = theRoamApp.genDialog();

  // ******************************************************
  // I18N - callback for the View->Choose Charset button
  // on the RoamMenuWindow.  
  // User can choose View->Choose Charset from menu.
  // This will choose the next possible charset to display
  // the message in.  Each time this option is chosen, a
  // new charset will be displayed.
  // ******************************************************
  if (_incoming) {
    RoamMenuWindow *rmw = (RoamMenuWindow *)_window;
    SdmError error;

    // Get the current message
    SdmMessageNumber current_msg_num = rmw->list()->current_msg_handle();
    SdmMessageStore *mbox = rmw->mailbox();
    SdmMessage *msg;
    mbox->SdmMessageFactory (error, msg, current_msg_num);
    if (error) return;

    // Get the current message envelope
    SdmMessageEnvelope *env;
    //SdmConnection *con = theRoamApp.connection()->connection();
    msg->SdmMessageEnvelopeFactory(error, env);
    if (error) return;

    // Get the current message body
    SdmMessageBodyStructure bodyStruct;
    SdmMessageBody *bp;
    msg->SdmMessageBodyFactory (error, bp, 1);
    if (error) return;
    bp->GetStructure(error, bodyStruct);
    if (error) return;

    // Get the body's content buffer
    // bp has the main body text unless it is a multipart
    // message.
    SdmContentBuffer content_buf;
    if (bodyStruct.mbs_type == Sdm_MSTYPE_multipart) {
      SdmMessage *newmsg;
      SdmMessageBody *newbp;
      SdmMessageBodyStructure newBodyStruct;
      bp->SdmMessageFactory(error, newmsg);
      if (error) return;
      newmsg->SdmMessageBodyFactory(error, newbp, 1);
      if (error) return;
      newbp->GetStructure(error, newBodyStruct);
      if (error) return;
      if (newBodyStruct.mbs_type == Sdm_MSTYPE_text) {
        newbp->GetContents(error, content_buf);
        if (error) return;
      } else {
        return;  // No body text available
      }
    } else if (bodyStruct.mbs_type == Sdm_MSTYPE_text) {
      bp->GetContents(error, content_buf);
      if (error) return;
    } else {
      return;  // No body text available
    }
 
    // Reinitialize the list of charsets to cycle thru if we're
    // displaying a new message
    if (_msg_num != current_msg_num) {
      _charset_list.ClearAllElements();
      SdmString contents;
      content_buf.GetContents (error, contents);
      if (error) return;
      SdmContentBuffer::GetPossibleCharacterSetList(error, _charset_list, 
	contents, Sdm_False);
      if (error) {
	_charset_list.AddElementToList(catgets(DT_catd, 1, 317,
	  "No Valid Character Set"));
      } else {
	_charset_list.AddElementToList(catgets(DT_catd, 1, 318,
	  "Default Character Set"));
      }
      _pos = 0;
      _msg_num = current_msg_num;
    }

    // Set the character set
    if (_charset_list[_pos] 
      == catgets(DT_catd, 1, 318, "Default Character Set")) {
      msg->SetCharacterSet (error, SdmString(""));
      if (error) return;
    } else {
      msg->SetCharacterSet (error, _charset_list[_pos]);
      if (error) return;
    }

    // Display the current message in to new charset.
    error.Reset();
    if (current_msg_num) {
        rmw->list()->display_message(error, current_msg_num);
        if (error) {
	  genDialog->post_error(error, DTMAILHELPCANNOTDISPLAYMSG,
				catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
          // don't return.  just continue.  reset error.
          error.Reset();
        }
    }

    // Echo the charset in the status area
    rmw->message((char *)(const char *)(_charset_list[_pos]));

    // cycle around the list
    if (_pos >= (_charset_list.ElementCount() - 1))
      _pos = 0;
    else
      _pos++;

  // ******************************************************
  // I18N - callback for the Format->Choose Charset button
  // on the SendMsgDialog.  
  // User can choose Format->Choose Charset from menu.
  // This will choose the next possible charset to display
  // the message in.  Each time this option is chosen, a
  // new charset will be displayed.
  // ******************************************************
  }
  else {
    SendMsgDialog *smd = (SendMsgDialog *)_window;
    AttachArea *attachArea = smd->get_editor()->attachArea();
    SdmError error;

    // Get the current message
    SdmMessage *msg = smd->getMsgHnd();
    SdmMessageNumber msgno = 0;
    if (msg) {
      msg->GetMessageNumber(error, msgno);
      if (error) error.Reset();
    }

    // Reinitialize the list of charsets to cycle thru if we're
    // composing a new message
    if (_send_msgno != msgno) {
      _charset_list.ClearAllElements();
      const char *contents = smd->get_editor()->textEditor()->get_contents(True);
      SdmContentBuffer::GetPossibleCharacterSetList(error, _charset_list, 
        contents, Sdm_True);
      if (error) {
	_charset_list.AddElementToList(catgets(DT_catd, 1, 317,
	  "No Valid Character Set"));
      } else {
	_charset_list.AddElementToList(catgets(DT_catd, 1, 318,
	  "Default Character Set"));
      }
      _pos = 0;
      _send_msgno = msgno;
    }

    // Echo the charset in the status area
    attachArea->bottomStatusMessage((char *)(const char *)(_charset_list[_pos]));

    // Set the character set
    if (_charset_list[_pos] 
      == catgets(DT_catd, 1, 318, "Default Character Set")) {
      smd->setCharacterSet (SdmString(""));
    } else {
      smd->setCharacterSet (_charset_list[_pos]);
    }

    // cycle around the list
    if (_pos >= (_charset_list.ElementCount() - 1))
      _pos = 0;
    else
      _pos++;
  }
}

void
GetPossibleCharsetsCmd::undoit()
{
}

void
OpenMsgCmd::doit()
{
    SdmError mail_error;

    // Initialize the mail_error.
    mail_error.Reset();

    _menuwindow->list()->viewInSeparateWindow(mail_error);
    if (mail_error) {
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
		NULL,
                clientData,
                NULL,
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
		NULL,
                clientData,
		NULL,
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
		NULL,
		clientData,
		NULL,
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

    XmString path = XmStringCreateLocalized(getenv("HOME"));

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
			       DtMailEditor * editor,
			       SendMsgDialog *smd,
			       Widget parent
			       ) 
:SelectFileCmd (name,
		label,
		title,
		catgets(DT_catd, 1, 95, "Save"),
		active,
		fileCB,
		NULL,
		this,
		NULL,
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

// saveText saves the message to a file as text, it doesn't save out
// the From line, so it will not be typed as a message.  The appendflag
// tells saveText whether or not to append the message to the file if
// the file already exists.  If appendflag is false and the file exists, 
// saveText will put up a popup asking whether or not it should overwrite
// the existing file.

void
SaveAsTextCmd::saveText(const char * filename, SdmBoolean appendFlag)
{
    int answer;
    char buf[2048];
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    int fd = 0;

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
    if (!appendFlag) {
	if (SdmSystemUtility::SafeAccess(filename, F_OK) == 0) {
	    sprintf(buf, catgets(DT_catd, 3, 47, "%s already exists.\nOverwrite?"),
		    filename);
	
	    genDialog->setToQuestionDialog(
					    catgets(DT_catd, 3, 48, "Mailer"),
					    buf);
	    answer = genDialog->post_and_return(DTMAILHELPMSGALREADYEXISTS);

	    if (answer == 2) { // Pressed cancel
		return;
	    }

	    if (SdmSystemUtility::SafeUnlink(filename) < 0) {
		sprintf(buf, 
			catgets(DT_catd, 3, 49, "Unable to overwrite %s.\n\
Check file permissions and retry."), 
			filename);
		genDialog->setToErrorDialog(catgets(DT_catd, 3, 50, "Mailer"), buf);
		genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOOVERWRITE);
		return;
	    }
	}

	// Create or overwrite, and then write the bits.
	// Use mode 0666 so that system umask controls all three mode fields
	fd = SdmSystemUtility::SafeOpen(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
    } else {
	// Create or append, and then write the bits.
	// Use mode 0666 so that system umask controls all three mode fields
	fd = SdmSystemUtility::SafeOpen(filename, O_RDWR | O_CREAT | O_APPEND, 0666);
    }
    if (fd < 0) {
	sprintf(buf, catgets(DT_catd, 3, 51, "Unable to create %s."), filename);
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 52, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOCREATE);
	return;
    }
    
    if (SdmSystemUtility::SafeWrite(fd, "\n", 1) < 1) {
	sprintf(buf, 
		catgets(DT_catd, 3, 53, "Unable to write to %s."), 
		filename);
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
	SdmSystemUtility::SafeClose(fd);
	SdmSystemUtility::SafeUnlink(filename);
	return;
    }
    
    char * text_buf = _text_editor->textEditor()->get_contents(False);
	if (text_buf == NULL) {
    	SdmSystemUtility::SafeClose(fd);
		return;
	}

    // Pop up a warning dialog if there is any attachment
    if(_text_editor->attachArea()->getUndeletedIconCount() > 0) {
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), 
				    catgets(DT_catd, 3, 143, "Your message contains attachments \nthat will not be saved to the text file."));
	answer = genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), 
		   catgets(DT_catd, 1, 5, "Cancel"), DTMAILHELPNOSAVEATT);
	if (answer == 2) {
	   SdmSystemUtility::SafeClose(fd);
           SdmSystemUtility::SafeUnlink(filename);
	   free(text_buf);
	   return;
	}
    }
    int len = strlen(text_buf);
    if (SdmSystemUtility::SafeWrite(fd, text_buf, len) < len) {
	sprintf(buf, catgets(DT_catd, 3, 55, "Unable to create %s."), filename);
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 56, "Mailer"), buf);
	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOCREATE);
	SdmSystemUtility::SafeClose(fd);
	SdmSystemUtility::SafeUnlink(filename);
	free(text_buf);
	return;
    }
    
    free(text_buf);
    SdmSystemUtility::SafeClose(fd);
    if (_smd != NULL)
    	_smd->setEditsMade(Sdm_False);
}

void
SaveAsTextCmd::doit()
{
    Widget append_button;

    if ( _fileBrowser == NULL) {

	SelectFileCmd::doit();

	// We need to create a dummy button in order for the Append button
	// to show up in the right place.

	XtVaCreateWidget("Unused Button",
		xmPushButtonWidgetClass, _fileBrowser,
		NULL);

	append_button = XtVaCreateManagedWidget(catgets(DT_catd, 1, 342, "Append"),
				xmPushButtonWidgetClass, _fileBrowser,
				NULL);
	
	XtAddCallback(append_button,
			XmNactivateCallback,
			&SaveAsTextCmd::appendCallback,
			(XtPointer) this );
    } else {
	SelectFileCmd::doit();
    }
}

void
SaveAsTextCmd::appendCallback ( Widget, XtPointer clientData, XtPointer)
{
    SaveAsTextCmd * obj = (SaveAsTextCmd *) clientData;
    char    *fname = NULL;
    XmString xmstr; // The selected file
    int      status = 0;

    // Get the value
    XtVaGetValues(obj->_fileBrowser, XmNdirSpec, &xmstr, NULL);
    if (xmstr) {  // Make sure a file was selected
	// Extract the first character string matching the default
	// character set from the compound string

	status = XmStringGetLtoR (xmstr,
				    XmSTRING_DEFAULT_CHARSET,
				    &fname);
	
	// If a string was successfully extracted, call
	// append to handle the file.

	if (status)
	    obj->saveText((const char *)fname, Sdm_True);
    }
    // Take down the file selection dialog.
    XtUnmanageChild(obj->_fileBrowser);
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
    SdmError error;
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    char *label, *buf_ptr;
    char filepath[MAXPATHLEN], buf[MAXPATHLEN + MAXNAMELEN];

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
      else {
        this->quit();
      }

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
    
    char *draftsDir = theRoamApp.getResource(DraftDir, Sdm_True);
    sprintf(filepath, "%s/%s", draftsDir, label);
    if (SdmSystemUtility::SafeAccess(draftsDir, F_OK) != 0) {
	if ( mkdir(draftsDir, 0700) != 0 ) {
	    sprintf(buf, catgets(DT_catd, 3, 44, "Unable to create %s."),
			draftsDir);
	    genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"), buf);
	    genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPERROR);
	    free(draftsDir);
	    doit();
	    free(label);
	    return;
	}
    }

    // If no write permission, warn the user and return
    if (SdmSystemUtility::SafeAccess(draftsDir, W_OK) != 0) {
      sprintf(buf, catgets(DT_catd, 3, 144,
			"Drafts directory (%s) has no write permission."), draftsDir);
      genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"), buf);
      genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPERROR);
      free(draftsDir);
      doit();
      free(label);
      return;
    }

    free(draftsDir);

    if (SdmSystemUtility::SafeAccess(filepath, F_OK) == 0) {
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

    // Create the draft - since this is in effect a mailbox we use mode 0600
    // as opposed to mode 0666 for security reasons as a draft could contain
    // confidential information - the user can manually change this if needed.
    //
    int fd;
    fd = SdmSystemUtility::SafeOpen(filepath, O_RDWR | O_CREAT | O_TRUNC,
				    0600);
    if (fd < 0) {
        sprintf(buf, catgets(DT_catd, 3, 51, "Unable to create %s."), label);
        genDialog->setToErrorDialog(catgets(DT_catd, 3, 52, "Mailer"), buf);
        genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOCREATE);
        this->quit();
        free(label);
        return;
    }

    SdmSystemUtility::SafeClose(fd);

    SdmMessageStore *mstore;
    SdmToken token;
    SdmSession *i_session = theRoamApp.isession()->isession();

    i_session->SdmMessageStoreFactory(error, mstore);
    if ( error ) {
        genDialog->post_error(error);
        this->quit();
        SdmSystemUtility::SafeUnlink(filepath);
      	if (label) free(label);
        return;
    }

    mstore->StartUp(error);
    if ( error ) {
        genDialog->post_error(error);
        this->quit();
        delete mstore;
        SdmSystemUtility::SafeUnlink(filepath);
      	if (label) free(label);
        return;
    }
    token.SetValue("servicechannel", "cclient");
    token.SetValue("serviceclass", "messagestore");
    token.SetValue("serviceoption", "messagestorename", filepath);
    token.SetValue("servicetype", "local");
    token.SetValue("serviceoption", "readwrite", "x");
    SdmMailRc * mailRc;
    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
    assert(!error);
    if (mailRc->IsValueDefined("dontsavemimemessageformat"))
      token.SetValue("serviceoption", "preservev3messages", "x");
      
    SdmBoolean readOnly;
    SdmMessageNumber nmsgs = 0;
    mstore->Open(error, nmsgs, readOnly, token);
    if ( error ) {
        genDialog->post_error(error);
        this->quit();
        delete mstore;
        SdmSystemUtility::SafeUnlink(filepath);
      	if (label) free(label);
        return;
    }

    SdmMessage *msg;
    mstore->SdmMessageFactory(error, msg);

    if ( error ) {
        genDialog->post_error(error);
        this->quit();
        delete mstore;
        SdmSystemUtility::SafeUnlink(filepath);
      	if (label) free(label);
        return;
    }

    _sendMsgDialog->updateHeaders();

    // Set up the envelope
    SdmMessageEnvelope *env;
    SdmStrStrL headers;
    int numOfHeaders = 0;
    int numOfEmptyHeaders = 0;
    SdmString emptyHeadersValue;
    for (SendMsgDialog::HeaderList *hl = _sendMsgDialog->getFirstHeader(); 
        hl; hl = _sendMsgDialog->getNextHeader()) 
    {
      // Write only the visible headers.
      if (hl->show == SendMsgDialog::SMD_ALWAYS || 
	hl->show == SendMsgDialog::SMD_SHOWN) {

	char* value = XmTextGetString(hl->field_widget);

	if ( !strcasecmp(hl->label, "Date") ) {
	    headers(-1).SetBothStrings(XDTMAILDATEHEADER, value);
	}

	if ( !strcasecmp(hl->label, "From") ) {
	    headers(-1).SetBothStrings(XDTMAILFROMHEADER, value);
	}

	if ( value && strlen(value) > 0 ) {
	    headers(-1).SetBothStrings(hl->label, value);
	    numOfHeaders++;
	}
	else {
	    numOfEmptyHeaders++;
	    if ( numOfEmptyHeaders == 1 )
		emptyHeadersValue += hl->label;
	    else {
		emptyHeadersValue += " ";
		emptyHeadersValue += hl->label;
	    }
	}

        if (value)
          XtFree(value);
      }
    }

    if ( numOfEmptyHeaders ) {
	headers(-1).SetBothStrings(XDTMAILEMPTYHEADERS, emptyHeadersValue);
    }

    msg->SdmMessageEnvelopeFactory(error, env);
    if ( error ) {
        genDialog->post_error(error);
        this->quit();
        delete mstore;
        SdmSystemUtility::SafeUnlink(filepath);
      	if (label) free(label);
        return;
    }
    env->SetHeaders(error, headers);
    if ( error ) {
        genDialog->post_error(error);
        this->quit();
        delete mstore;
        SdmSystemUtility::SafeUnlink(filepath);
      	if (label) free(label);
        return;
    }

    // and now set up body parts
    SdmMessage *sendMsg, *sendNestedMsg;
    SdmMessage      *nestedMsg;
    SdmMessageBody *sendBP, *sendNestedBP;
    SdmMessageBody *bp;
    int bodyCount;
    char *cont;

    // sendMsg is a message created for Compose window (and destined for "sending")
    sendMsg = _sendMsgDialog->getMsgHnd();
    sendMsg->GetBodyCount(error, bodyCount);

    if ( !error && bodyCount == 0 ) {
      msg->SdmMessageBodyFactory(error, bp,
                      (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
      if ( !error ) {
        cont = _sendMsgDialog->get_editor()->textEditor()->get_contents(False);
        SdmContentBuffer content_buf(cont);
        bp->SetContents(error, content_buf);
      }
    }
    else if (!error) { // bodyCount == 1
 
        msg->SdmMessageBodyFactory(error, bp,
            (SdmMsgStrType)Sdm_MSTYPE_multipart, "mixed");
        if ( !error ) {
          bp->SdmMessageFactory(error, nestedMsg);
        }
        if ( !error ) {
          sendMsg->SdmMessageBodyFactory(error, sendBP, 1);
        }
        if ( !error ) {
          sendBP->SdmMessageFactory(error, sendNestedMsg);
        }
        if ( !error ) {
          // nestedMsg corresponds to sendNestedMsg
          // sendNestedMsg has already created separate body parts for attachments
          // First body part is reserved for typed text in Editor window
          sendNestedMsg->GetBodyCount(error, bodyCount);
          
          if ( !error ) {
            for (int i = 1; i <= bodyCount; i++) {
              SdmMessageBodyStructure sendBodyStruct, bodyStruct;
              SdmContentBuffer contentBuf;
              SdmString contents;

              if ( i == 1 ) { // Just copy whatever is in "Compose"/Editor widget
                nestedMsg->SdmMessageBodyFactory(error, bp,
                  (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
                if (error) break;
                cont = _sendMsgDialog->get_editor()->textEditor()->get_contents(False);
                      SdmContentBuffer content_buf(cont);
                bp->SetContents(error, content_buf);
                if (error)
		  break;

		continue;
              }

              // If the attachment is deleted, don't write it out
              if ( _sendMsgDialog->get_editor()->attachArea()->isDeleted(i-2) ) {
                continue;
              }

              // Get the body part/struct from "compose" window
              sendNestedMsg->SdmMessageBodyFactory(error, sendNestedBP, i);
              if (error) break;
              sendNestedBP->GetStructure(error, sendBodyStruct);
              if (error) break;

              // Create new body part and copy "compose" stuff to it
              nestedMsg->SdmMessageBodyFactory(error, bp,
                  (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
              if (error) break;

              sendNestedBP->GetContents(error, contentBuf);
              if (error) break;
              bp->SetContents(error, contentBuf);
              if (error) break;

              bp->GetStructure(error, bodyStruct);
              if (error) break;
              bodyStruct.mbs_attachment_name = sendBodyStruct.mbs_attachment_name;
              bp->SetStructure(error, bodyStruct);
              if (error) break;

              SdmDataTypeUtility::SetContentTypesForMessage(error, *bp);
              if (error) break;
          }
        }
      }
    }

    SdmBoolean displacement;

    if (!error) {
      // Store the message in message store
      msg->CommitPendingChanges(error, displacement, Sdm_True);
    }

    if (mstore)
      delete mstore;  // closes and shutdown the store.
    
    draftsDir = theRoamApp.getResource(DraftDir, Sdm_False);
    
    if (!error) {
      sprintf(filepath, "%s/%s", draftsDir, label);
      sprintf(buf, catgets(DT_catd, 3, 116, "Draft saved to %s"), filepath);
      _sendMsgDialog->setStatus(buf);
      _sendMsgDialog->setEditsMade(Sdm_False);
    } else {
        genDialog->post_error(error);
	SdmSystemUtility::SafeUnlink(filepath);
    }

    free(label);
    free(draftsDir);

    this->quit();

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
    char *label = NULL, *buf_ptr = NULL, *value = NULL;
    char full_tname[MAXPATHLEN], expfilename[MAXPATHLEN],
	 filename[MAXPATHLEN], buf[MAXPATHLEN + MAXNAMELEN],
	 buf2[MAXPATHLEN + MAXNAMELEN];
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    int i, fd;
    SdmMessage *msg;
    SdmBoolean already_in_list = Sdm_False;
    SdmError error;
    SdmMailRc *mail_rc;
    error.Reset();
    SdmConnection *con = theRoamApp.connection()->connection();
    con->SdmMailRcFactory(error, mail_rc);
    assert(!error);

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
	char *tmpStr;
        sprintf(filename, "%s/%s", template_path, label);
	tmpStr = strdup(template_path);
    	SdmUtility::ExpandPath(error, template_path, tmpStr,
			*mail_rc, SdmUtility::kFolderResourceName);
        sprintf(expfilename, "%s/%s", template_path, label);
	free(tmpStr);
    }
    else {
	template_path = theRoamApp.getResource(TemplateDir, Sdm_False);
	sprintf(filename, "%s/%s", template_path, label);
	free(template_path);
	template_path = theRoamApp.getResource(TemplateDir, Sdm_True);
	sprintf(expfilename, "%s/%s", template_path, label);
	if (SdmSystemUtility::SafeAccess(template_path, F_OK) != 0) {
	    if ( mkdir(template_path, 0700) != 0 ) { // Could not create template dir
		sprintf(buf, catgets(DT_catd, 3, 44, "Unable to create %s."),
			template_path);
		genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"), buf);
		genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPERROR);
		free(template_path);
		_sendMsgDialog->setTemplateFullPath(NULL);
		doit();
		free(label);
		return;
	    }
	}
    }

    free(template_path);
    template_path = NULL;

    // If no write permission, warn the user and return
    template_path = theRoamApp.getResource(TemplateDir, Sdm_True);

    if (SdmSystemUtility::SafeAccess(template_path, W_OK) != 0) {
      sprintf(buf, catgets(DT_catd, 3, 145,
		"Templates directory (%s) has no write permission."), template_path);
      genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"), buf);
      genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPERROR);
      doit();
      free(template_path);
      free(label);
      return;
    }

    free(template_path);
    _sendMsgDialog->setTemplateFullPath(NULL);

    if (SdmSystemUtility::SafeAccess(expfilename, F_OK) == 0) {
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

    // Create the template - since this is in effect a mailbox we use mode 0600
    // as opposed to mode 0666 for security reasons as a draft could contain
    // confidential information - the user can manually change this if needed.
    //
    if ((fd = SdmSystemUtility::SafeOpen(expfilename, O_RDWR | O_CREAT | O_TRUNC, 0600)) < 0) {
        sprintf(buf, catgets(DT_catd, 3, 51, "Unable to create %s."), label);
        genDialog->setToErrorDialog(catgets(DT_catd, 3, 52, "Mailer"), buf);
        genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOCREATE);
        this->quit();
        free(label);
        return;
    }

    SdmSystemUtility::SafeClose(fd);

    SdmMessageStore *mstore;
    SdmToken token;
    SdmSession *i_session = theRoamApp.isession()->isession();

    i_session->SdmMessageStoreFactory(error, mstore);
    if ( error ) {
        genDialog->post_error(error);
        this->quit();
        SdmSystemUtility::SafeUnlink(expfilename);
        return;
    }
    mstore->StartUp(error);
    if ( error ) {
        genDialog->post_error(error);
        this->quit();
        delete mstore;
        SdmSystemUtility::SafeUnlink(expfilename);
        return;
    }
    token.SetValue("servicechannel", "cclient");
    token.SetValue("serviceclass", "messagestore");
    token.SetValue("serviceoption", "messagestorename", expfilename);
    token.SetValue("servicetype", "local");
    token.SetValue("serviceoption", "readwrite", "x");
    if (mail_rc->IsValueDefined("dontsavemimemessageformat"))
        token.SetValue("serviceoption", "preservev3messages", "x");

    SdmBoolean readOnly;
    SdmMessageNumber nmsgs = 0;
    mstore->Open(error, nmsgs, readOnly, token);
 
    if ( error ) {
        genDialog->post_error(error);
        this->quit();
        delete mstore;
        SdmSystemUtility::SafeUnlink(expfilename);
        if (label) free (label);
        return;
    }

    mstore->SdmMessageFactory(error, msg);

    if ( error ) {
        delete mstore;
        SdmSystemUtility::SafeUnlink(expfilename);
        if (label) free (label);
        return;
    }

    // Set up the envelope
    SdmMessageEnvelope *env;
    SdmStrStrL headers;
    int numOfHeaders = 0;
    int numOfEmptyHeaders = 0;
    SdmString emptyHeadersValue;
    for (SendMsgDialog::HeaderList *hl = _sendMsgDialog->getFirstHeader(); 
		hl; hl = _sendMsgDialog->getNextHeader()) {
	// Write only the visible headers.
        if (hl->show == SendMsgDialog::SMD_ALWAYS || 
	    hl->show == SendMsgDialog::SMD_SHOWN) {

	    value = XmTextGetString(hl->field_widget);

	    if ( !strcasecmp(hl->label, "Date") ) {
		headers(-1).SetBothStrings(XDTMAILDATEHEADER, value);
	    }

	    if ( !strcasecmp(hl->label, "From") ) {
		headers(-1).SetBothStrings(XDTMAILFROMHEADER, value);
	    }

	    if ( value && strlen(value) > 0 ) {
		headers(-1).SetBothStrings(hl->label, value);
		numOfHeaders++;
	    }
	    else {
		numOfEmptyHeaders++;
		if ( numOfEmptyHeaders == 1 )
		    emptyHeadersValue += hl->label;
		else {
		    emptyHeadersValue += " ";
		    emptyHeadersValue += hl->label;
		}
	    }

	    if (value)
		XtFree(value);
	}
    }

    if ( numOfEmptyHeaders ) {
	headers(-1).SetBothStrings(XDTMAILEMPTYHEADERS, emptyHeadersValue);
    }

    msg->SdmMessageEnvelopeFactory(error, env);
    if ( error ) {
        genDialog->post_error(error);
        this->quit();
        delete mstore;
        SdmSystemUtility::SafeUnlink(expfilename);
        if (label) free (label);
        return;
    }
    env->SetHeaders(error, headers);
    if ( error ) {
        genDialog->post_error(error);
        this->quit();
        delete mstore;
        SdmSystemUtility::SafeUnlink(expfilename);
        if (label) free (label);
        return;
    }

    // and now set up body parts
    SdmMessage *sendMsg, *sendNestedMsg;
    SdmMessage      *nestedMsg;
    SdmMessageBody *sendBP, *sendNestedBP;
    SdmMessageBody *bp;
    int bodyCount;
    char *cont;

    // sendMsg is a message created for Compose window (and destined for "sending")
    sendMsg = _sendMsgDialog->getMsgHnd();
    sendMsg->GetBodyCount(error, bodyCount);

    if ( !error && bodyCount == 0 ) {
      msg->SdmMessageBodyFactory(error, bp,
                            (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
      if (!error) {
        cont = _text_editor->get_contents(False);
        SdmContentBuffer content_buf(cont);
        bp->SetContents(error, content_buf);
      }
    }
    else if (!error) { // bodyCount == 1
 
    	msg->SdmMessageBodyFactory(error, bp,
        (SdmMsgStrType)Sdm_MSTYPE_multipart, "mixed");
        
      if (!error) {
        bp->SdmMessageFactory(error, nestedMsg);
      }

      if (!error) {
        sendMsg->SdmMessageBodyFactory(error, sendBP, 1);
      }
      if (!error) {
      	sendBP->SdmMessageFactory(error, sendNestedMsg);
      }

      if (!error) {
        // nestedMsg corresponds to sendNestedMsg
        // sendNestedMsg has already created separate body parts for attachments
        // First body part is reserved for typed text in Editor window
        sendNestedMsg->GetBodyCount(error, bodyCount);

        if (!error) {
          for (i = 1; i <= bodyCount; i++) {
              SdmMessageBodyStructure sendBodyStruct, bodyStruct;
              SdmContentBuffer contentBuf;
              SdmString contents;

              if ( i == 1 ) { // Just copy whatever is in "Compose"/Editor widget
                nestedMsg->SdmMessageBodyFactory(error, bp,
                  (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
                if (error) break;
                
                cont = _text_editor->get_contents(False);
                SdmContentBuffer content_buf(cont);
                bp->SetContents(error, content_buf);
                if (error)
		  break;
                
                continue;
              }

              // If the attachment is deleted, don't write it out
              if ( _sendMsgDialog->get_editor()->attachArea()->isDeleted(i-2) ) {
                continue;
              }

              // Get the body part/struct from "compose" window
              sendNestedMsg->SdmMessageBodyFactory(error, sendNestedBP, i);
              if (error) break;
              sendNestedBP->GetStructure(error, sendBodyStruct);
              if (error) break;

              // Create new body part and copy "compose" stuff to it
              nestedMsg->SdmMessageBodyFactory(error, bp,
                    (SdmMsgStrType)Sdm_MSTYPE_text, "plain");
              if (error) break;

              sendNestedBP->GetContents(error, contentBuf);
              if (error) break;
              bp->SetContents(error, contentBuf);
              if (error) break;

              bp->GetStructure(error, bodyStruct);
              if (error) break;
              bodyStruct.mbs_attachment_name = sendBodyStruct.mbs_attachment_name;
              bp->SetStructure(error, bodyStruct);
              if (error) break;

              SdmDataTypeUtility::SetContentTypesForMessage(error, *bp);
              if (error) break;
          }
        }
      }
    }

    SdmBoolean displacement;
    
    if (!error) {
      // Store the message in message store
      msg->CommitPendingChanges(error, displacement, Sdm_True);
    }

    if (mstore) {
      delete mstore;  // will shutdown and close store.
    }

    if (error) {
      genDialog->post_error(error);
      this->quit();
      SdmSystemUtility::SafeUnlink(expfilename);
      if (label) free (label);
    }
    
    sprintf(buf2, "%s:%s", label, filename); 

    // The template can have spaces in it! ug!
    // Make the name that we are going to store here with the backslashes
    // preceding blanks.
    char *p, *ptmp;
    strcpy(full_tname, buf2);
    for (p = full_tname; p && *p != '\0'; p++) {
	if ( isspace(*p) ) {
	    for (ptmp = p + strlen(p); p <= ptmp; ptmp--) // shift the string one place
		*(ptmp + 1) = *ptmp;
	    *p = '\\';
	    ++p;
	}
    }

    // Update the template property.
    // See if its already in the template list

    mail_rc->GetValue(error, "templates", &value);
    if (!error) {
      DtVirtArray<PropStringPair *> *templ_items = 
        new DtVirtArray<PropStringPair *>(10);
      parsePropString(value, *templ_items);
      int len = templ_items->length();
      buf_ptr = NULL;
      for (i = 0; i < len; i++) {
        if (strcmp((*templ_items)[i]->label, label) == 0) {
          SdmUtility::ExpandPath(error, buf_ptr,
             (*templ_items)[i]->value, *mail_rc, 
            SdmUtility::kFolderResourceName);

          if (strcmp(buf_ptr, expfilename) == 0) {
            // Don't add to list; its already there.
            already_in_list = Sdm_True;
            break;
          }
        }
      }
      delete templ_items;
      if (buf_ptr)
        free(buf_ptr);
    }
    free(label);

    // It's not in the list, so add to mail options pane if it exists.
    // And update the mailrc file. Also update the templates menu on 
    // compose and RMW.

    if (!already_in_list) {
      if (value && *value) {
        buf_ptr = (char*)malloc(strlen(value) + strlen(full_tname) + 2);
        sprintf(buf_ptr, "%s %s", value, full_tname);
        mail_rc->SetValue(error, "templates", buf_ptr);
            assert(!error);
        free(buf_ptr);
      }
      else  {
        mail_rc->SetValue(error, "templates", full_tname);
            assert(!error);
      }

      // Update the mailrc, the template menus and the options pane.
      OptCmd * oc = theRoamApp.mailOptions();
      DtbOptionsDialogInfo od = oc->optionsDialog(); 
      if (od && od->templates_list) {
        // Update the templates list on options pane.
        TemplateListUiItem *tmpl_ptr;
        XtVaGetValues(od->templates_list, XmNuserData, &tmpl_ptr, NULL);
        tmpl_ptr->handleIncludeButtonPress(filename);
      }

      // Update the mailrc file
      mail_rc->Update(error);
      assert(!error);

      // Update the templates menus on compose and RMW
          theRoamApp.updateTemplates();
    }

    if (value)
      free(value);

    this->quit();

    sprintf(buf, catgets(DT_catd, 3, 117, "Template saved to %s"), filename);
    _sendMsgDialog->setStatus(buf);
    _sendMsgDialog->setEditsMade(Sdm_False);
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
		  XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL,
		  NULL);

    XmStringFree(ok_str);

    XtAddCallback(XmSelectionBoxGetChild(renameDialog, XmDIALOG_HELP_BUTTON),
		 XmNhelpCallback, HelpCB, DTMAILRENAMEATTACHMENTDIALOG);

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
					 ) : Cmd (name, label, Sdm_True)
{
    _index = indx;
    
    _parent = rmw;
}

AttachmentActionCmd::AttachmentActionCmd(
					 char *name,
					 char *label,
					 ViewMsgDialog *vmd,
					 int indx
					 ) : Cmd (name, label, Sdm_True)
{
    _index = indx;
    
    _parent = vmd;
}

AttachmentActionCmd::AttachmentActionCmd(
					 char *name,
					 char *label,
					 SendMsgDialog *smd,
					 int indx
					 ) : Cmd (name, label, Sdm_True)
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
					 ) : Cmd(name, label, Sdm_True)
{
    _parent = rmw;
}

SelectAllAttachsCmd::SelectAllAttachsCmd(
					 char *name,
					 char *label,
					 ViewMsgDialog *vmd
					 ) : Cmd(name, label, Sdm_True)
{
    _parent = vmd;
}

SelectAllAttachsCmd::SelectAllAttachsCmd(
					 char *name,
					 char *label,
					 SendMsgDialog *smd
					 ) : Cmd(name, label, Sdm_False)
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
				     ) : ToggleButtonCmd(name, label, Sdm_True)
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
				   ) : ToggleButtonCmd(name, label, Sdm_True)
{
    _parent = rmw;
}

void
AbbrevHeadersCmd::doit()
{
    // If button is OFF
    if (!this->getButtonState()) {
	_parent->fullHeader(Sdm_True);
    }
    else {  // button is ON
	_parent->fullHeader(Sdm_False);
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
    // Call the goAway() method on the SMD.  Argument Sdm_True requests it
    // to check if the SMD is dirty.  Let it handle the 
    // case where text may be present in the compose window.
    if (!_compose_dialog->isMsgValid())
	return;
    else
	_compose_dialog->goAway(Sdm_True);
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
    cur_setting = (SdmBoolean) active;    
    editor->set_word_wrap(cur_setting);		
}

void
WordWrapCmd::doit()
{
    cur_setting = (SdmBoolean) ((ToggleButtonCmd *)this)->getButtonState();
    editor->set_word_wrap(cur_setting);		
}

SdmBoolean
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

  SdmError error;
  const char *logfile = NULL;

  _send = send;

}

void
LogMsgCmd::doit()
{
    if (!((ToggleButtonCmd *)this)->getButtonState()) {
	// turn off logging for this message
	_send->setLogState(Sdm_False);
    }
    else {
	// turn on logging for this message
	_send->setLogState(Sdm_True);
    }

}
char *
VacationCmd::getMonth(struct tm *tm)
{
	static char *month;
	static const char * MonthsOfTheYear[] = {
		"Jan", "Feb", "Mar",
		"Apr", "May", "Jun",
		"Jul", "Aug", "Sep",
		"Oct", "Nov", "Dec"
	};

	if (month == NULL) 
		month = (char*) malloc(5);
	*month = NULL;
	SdmSystemUtility::SafeStrftime (month, 5, "%m", tm);
	return ((char*)MonthsOfTheYear[atoi(month)-1]);
}

VacationCmd::VacationCmd(
    char *name,
    char *label
) : Cmd (name, label, Sdm_True)
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
	status = SdmSystemUtility::SafeWrite(fd, buf, len);
    } while (status < 0 && errno == EAGAIN);
	
    return(0);
}
// This routine only gets called if the user has an at job to stop vacation.
// When its time to stop vacation, this routine resets the GUI. 
void
VacationCmd::resetVacationCallback(XtPointer ptr, XtIntervalId *)
{
    SdmError error;
    SdmMailRc *mail_rc;
    error.Reset();
    SdmConnection *con = theRoamApp.connection()->connection();
    con->SdmMailRcFactory(error, mail_rc);
    assert(!error);

    theRoamApp.resetTitles();

    VacationCmd *cmd_ptr = (VacationCmd*)ptr;
    if (cmd_ptr) {
	cmd_ptr->resetRadioButton(Sdm_False);
    	cmd_ptr->_stop_timeout = 0;
    }

    mail_rc->RemoveValue(error, "vacationstopatjobid");
    mail_rc->RemoveValue(error, "vacationatjobhost");
    mail_rc->Update(error);
    assert(!error);
 
}
void
VacationCmd::startVacationCallback(XtPointer ptr, XtIntervalId *)
{
    SdmError error;
    SdmMailRc *mail_rc;
    error.Reset();
    SdmConnection *con = theRoamApp.connection()->connection();
    con->SdmMailRcFactory(error, mail_rc);
    assert(!error);

    mail_rc->RemoveValue(error, "vacationstartatjobid");

    // Dont remove the atjobhost  unless there is no stop at job.
    // It gets removed when the stop at job goes off. 
    VacationCmd *cmd_ptr = (VacationCmd*)ptr;
    if (cmd_ptr) {
	if (cmd_ptr->_stop_timeout == 0) {
    		mail_rc->RemoveValue(error, "vacationatjobhost");
	}
    	cmd_ptr->_start_timeout = 0;
    }

    mail_rc->Update(error);
    assert(!error);

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
    SdmError error;
    time_t v_starttick, v_endtick, nowtick;
    struct tm stm, etm, *nowtm;
    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    char buf[1024], *ptr = NULL, *v_buf, *month;
    int totalbytes=0; 
    SdmMailRc *mailRc;

    error.Reset();

    // Basically reset everything. 
    resetVacationItems();
    theRoamApp.resetTitles();
    SdmConnection *con = theRoamApp.connection()->connection();
    con->SdmMailRcFactory(error, mailRc);
    assert(!error);
    error.Reset();
    mailRc->GetValue(error, "vacationstartdate", &v_buf); 

    // If null, then it is set to today, which is the default.
    if (error) {
    	error.Reset();
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
    mailRc->GetValue(error, "vacationenddate", &v_buf); 

    if (error) { 
    	error.Reset();
	// Keep vacation on forever until user turns off via the GUI
    	v_endtick = -1;
    }
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
    	if (SdmSystemUtility::SafeDup2(fileno(fp), 2) == -1) {
		startVacationError("dup2() failed");
		fclose(fp);
		SdmSystemUtility::SafeUnlink(stderrfile);
		return -1;
    	}
    }

    // Just start it without the at mechanism if they want to turn it on for 'today'
    if (v_starttick == nowtick) {
	mailRc->GetValue(error, "vacationreplyinterval", &v_buf);
	addVacationToForwardFile(v_buf);
	if (!error)
		free(v_buf);
    	if (v_endtick == -1) 
		return 0;
    }
    else {


    	// Set up the 'at' string... 
    	// The 5.5 interface is nicer but doesnt work with 5.4
	month = getMonth(&stm);
    	sprintf(buf, "/usr/bin/env LC_ALL=C at %02d%02d %s %d,%4d", stm.tm_hour, stm.tm_min, 
		 month, stm.tm_mday, 1900+stm.tm_year);

    	// Sent up the 'at' job with popen. The output of the job, on execution, will
    	// go to /dev/null. By default it sends mail to the user which we dont want. 
    	if ((pfp = popen(buf, "w")) != 0) {
		mailRc->GetValue(error, "vacationreplyinterval", &v_buf);
		if (!error) {
			fprintf(pfp, "/usr/dt/bin/dtmail -v start %s", v_buf);
			free(v_buf);
		}
		else
			fprintf(pfp, "/usr/dt/bin/dtmail -v start");
		pclose(pfp);
    	}
    	else {
		startVacationError("popen() failed");
		fclose(fp);
		SdmSystemUtility::SafeUnlink(stderrfile);
		resetAtJobs();
		return -1;
    	}

   	// Read from the stderr file and get the 'at' job id.
   	// Use fgets() vs. read(). It is safer as we know that the string: "job id"
   	// occurs at the beginning of the input line. 
    	rewind(fp);
    	while (fgets(buf, 1024, fp)) {
		totalbytes += strlen(buf);
		ptr = (char *)strtok(buf, " ");
		if (ptr && *ptr) {
			// ptr should point to the string "job"... 
			if (strcmp(ptr, catgets(DT_catd, 15, 33, "job")) == 0) { 
				// The next token should be the job id...
				if (ptr = strtok(NULL, " ")) {
					mailRc->SetValue(error, 
						"vacationstartatjobid", buf+4);
					mailRc->SetValue(error, 
						"vacationatjobhost", getHost());
    					while (fgets(buf, 1024, fp)) 
						totalbytes += strlen(buf);
					break;
				}
			}
			else ptr = NULL;
		}
    	}
    	if (!ptr) {
		startVacationError("Cant get the 'at' job id from stderr");
		fclose(fp);
		SdmSystemUtility::SafeUnlink(stderrfile);
		resetAtJobs();
		return -1;
    	}
    }
    // No end 'at' job to set, so just return gracefully. User must 
    // manually turn off the vacation.
    if (v_endtick == -1) {
    	mailRc->Update(error);
        assert(!error);
	fclose(fp);
        SdmSystemUtility::SafeUnlink(stderrfile);
	return 0;
    }

    month = getMonth(&etm);
    sprintf(buf, "/usr/bin/env LC_ALL=C at %02d%02d %s %d,%4d", etm.tm_hour, etm.tm_min, 
		month, etm.tm_mday, 1900+etm.tm_year);

    if ((pfp = popen(buf, "w")) != 0) {
	fprintf(pfp, "/usr/dt/bin/dtmail -v stop");
	pclose(pfp);
    }
    else {
	startVacationError("popen() failed");
	fclose(fp);
	SdmSystemUtility::SafeUnlink(stderrfile);
	resetAtJobs();
	return -1;
    }
    // Read from the stderr file and get the 'at' job id.
    ptr = NULL;
    fseek(fp, totalbytes, 0);
    while (fgets(buf, 1024, fp )) {
	ptr = (char *)strtok(buf, " ");
	if (ptr && *ptr) {
		// ptr should point to the string "job"...
		if (strcmp(ptr, catgets(DT_catd, 15, 33, "job")) == 0) {
			 // The next token should be the job id...
			if (ptr = strtok(NULL, " ")) {
				mailRc->SetValue(error, 
					"vacationstopatjobid", buf+4);
				mailRc->SetValue(error, 
					"vacationatjobhost", getHost());
				break;
			}
		}
		else ptr = NULL;
	}
    }
    if (!ptr) {
	startVacationError("Cant get the 'at' job id from stderr");
	fclose(fp);
	SdmSystemUtility::SafeUnlink(stderrfile);
	resetAtJobs();
	return -1;
    }
    if (v_starttick != nowtick)
    	setStartVacationTimeout(v_starttick- nowtick);
    setStopVacationTimeout(v_endtick- nowtick);

    mailRc->Update(error);
    assert(!error);
    fclose(fp);
    SdmSystemUtility::SafeUnlink(stderrfile);
	
    return 0;
}

void
VacationCmd::resetAtJobs()
{
	char *value;
	SdmError error;
        SdmMailRc *mail_rc;
	SdmConnection *con = theRoamApp.connection()->connection();
	con->SdmMailRcFactory(error, mail_rc);
    	assert(!error);
        char buf[256]; 
	SdmBoolean different_hosts = Sdm_False, update = Sdm_False; 
 
        *buf = NULL;  
        mail_rc->GetValue(error,"vacationstartatjobid", &value);
        if (!error) {
		sprintf(buf, "at -r %s ", value);
		mail_rc->RemoveValue(error, "vacationstartatjobid");
		update = Sdm_True;
                free(value); 
        } 
	else error.Reset();
        mail_rc->GetValue(error, "vacationstopatjobid", &value);
        if (!error) {
		if (*buf == NULL)
			sprintf(buf, "at -r %s ", value);
		else
			strcat(buf, value);
		mail_rc->RemoveValue(error, "vacationstopatjobid");
		update = Sdm_True;
                free(value); 
        }
	else error.Reset();
	mail_rc->GetValue(error, "vacationatjobhost", &value);
        if (*buf != NULL) {
		// At jobs were started on some other host. Dont try to remove
		if (!error) {
			char *host = getHost();
			if (different_hosts = (SdmBoolean)strcmp (host, value)) {
				char buf2[1024];
				DtMailGenDialog *genDialog = theRoamApp.genDialog();
				sprintf(buf2, catgets(DT_catd, 15, 15, "You already started the vacation message on the host %s.\nGo to this host and stop your vacation message,\nor it may start or stop prematurely."), value);
				genDialog->setToErrorDialog(
					catgets(DT_catd, 3, 48, "Mailer"), buf2);
				genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPVACATIONHOSTCHANGED);
			}
		}
		// Only remove at jobs if on the host where they were started.
		if (!different_hosts) {
		  // Changed to call fork1() so that only the thread making the call
		  // is duplicated as opposed to all threads. See fork1(2) for details.
		  // If fork1() is called and the child does more than just call exec(),
		  // there is a possibility of deadlocking - buyer beware!
		  if (fork1() == 0) {
		  // exec the appropriate vacation handler
		  execl("/bin/sh", "sh", "-c", buf, (char *)0);
		  // returned - exec failed - just exit this process.
		  _exit(0);
		  }
		}
	}
	if (!error) {
		mail_rc->RemoveValue(error, "vacationatjobhost");
		update = Sdm_True;
		free(value);
	}
	if (update) {
		mail_rc->Update(error);
		assert(!error);
	}
}
void
VacationCmd::resetVacationMessage()
{
   OptCmd *oc = theRoamApp.mailOptions();
   DtbOptionsDialogInfo oH = oc->optionsDialog();
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
VacationCmd::resetRadioButton(SdmBoolean turnOn)
{
   OptCmd *oc = theRoamApp.mailOptions();
   DtbOptionsDialogInfo oH = oc->optionsDialog();
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
    resetRadioButton(Sdm_False);
}

SdmBoolean
VacationCmd::vacationAlreadyStarted()
{
	int	fd;
	char	buf[256];

	sprintf(buf, "%s/.forward", getenv("HOME"));

	if (SdmSystemUtility::SafeAccess(buf, F_OK) != 0)
		return(Sdm_False);
	
	if ((fd = SdmSystemUtility::SafeOpen(buf, O_RDONLY)) == -1)
	  return(Sdm_False);
	
	*buf = NULL;
	int len;
	while ((len = SdmSystemUtility::SafeRead(fd, buf, 255)) > 0) {
	    buf[len] = 0;
	    if ((strstr(buf, "/usr/bin/vacation")) ||
		(strstr(buf, "/usr/ucb/vacation"))) {
		SdmSystemUtility::SafeClose(fd);
		return Sdm_True;
	    }
	}
	SdmSystemUtility::SafeClose(fd);
	return(Sdm_False);
}

SdmBoolean
VacationCmd::vacationIsOn()

{
        SdmError error;
    	SdmMailRc *mail_rc;
	error.Reset();
	SdmConnection *con = theRoamApp.connection()->connection();
	con->SdmMailRcFactory(error, mail_rc);
	assert(!error);
	char *value;

        // There are two ways to determine if vacation is on:
	// 1. There is an entry in .mailrc for vacationstartatjobid
	// 2. There is an entry in the .forward  for vacation
	// Note: After vacation is started the vacationstartatjobid resource is removed
	// so vacation could be on without this resource being set. However that would mean
	// there would for sure be an entry in the .forward file for vacation.

        mail_rc->GetValue(error, "vacationstartatjobid", &value);
	if (!error) {
		free(value);
		return Sdm_True;
	}
	return(vacationAlreadyStarted());
}

void
VacationCmd::createForwardVacationStr(char *buf, char *pw_name, char *replyInterval)
{
        SdmError error;
    	SdmMailRc *mailrc;
	error.Reset();
	SdmConnection *con = theRoamApp.connection()->connection();
	con->SdmMailRcFactory(error, mailrc);
	assert(!error);

 	sprintf (buf, "\\%s, \"| /usr/bin/vacation ", pw_name);

        if (replyInterval && *replyInterval && strcmp (replyInterval, "7") != 0) {
		// 7 is the default so just ignore it.
		strcat(buf, "-t");
		strcat(buf, replyInterval);
		strcat(buf, "d ");
  	}

	// If usealternates is set and there are alternates, use -a option to
	// add these alternative adresses in vacation command so mail sent to 
	// <firstname.lastname@domain etc. will also get a vacation notice.
	if (mailrc->IsValueDefined("usealternates") == Sdm_True) {
       	  OptCmd * oc = theRoamApp.mailOptions();
	  DtbOptionsDialogInfo od = oc->optionsDialog(); 
	  if (od && od->local_name_list) {
	    AlternatesListUiItem *alt_ptr;
	    XtVaGetValues(od->local_name_list, XmNuserData, &alt_ptr, NULL);
	    DtVirtArray<PropStringPair *> *list =  alt_ptr->getItemList();

	    // The advance pane may not have initialized
	    if (list == NULL) {
	      alt_ptr->writeFromSourceToUi();
	      list = alt_ptr->getItemList();
	    }

	    if (list != NULL) {
	      int numalias = list->length();
	      for (int i = 0; i< numalias; i++) {
		strcat(buf, "-a ");
		strcat(buf, (*list)[i]->label);
		strcat(buf, " ");
	      }
	    }
	  }
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
    SdmBoolean wrote_something = Sdm_False;

    passwd pw;
    SdmSystemUtility::GetPasswordEntry(pw);

    // Create a tmp file for writing out new .forward file
    sprintf(tmpforwardfile, "%s/.forward.tmp", pw.pw_dir);
    if (SdmSystemUtility::SafeAccess(tmpforwardfile, F_OK) == 0)
        SdmSystemUtility::SafeUnlink(tmpforwardfile);

    sprintf(forwardfile, "%s/.forward", pw.pw_dir);

    if ((fwd_fp = fopen(forwardfile, "r")) == NULL)
        return;
 
    if ((tmp_fp = fopen(tmpforwardfile, "w")) == NULL) {
      fclose(fwd_fp);
      return;
    }
 
    // Make the buffer 1024. This is safe as there is no way the vacation
    // line could execeed 1024 bytes, and fgets will stop on newline.
    while (fgets(buf, 1024, fwd_fp)) {
        if (strstr(buf, "/usr/bin/vacation") == NULL &&
		 strstr(buf, "/usr/ucb/vacation") == NULL) {
                if (SdmSystemUtility::SafeWrite(fileno(tmp_fp), buf, strlen(buf)) < 0) {
                        fclose(fwd_fp);
                        fclose(tmp_fp);
                        SdmSystemUtility::SafeUnlink(tmpforwardfile);
                        return;
                }
		else wrote_something = Sdm_True;
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
    		SdmSystemUtility::SafeUnlink(tmpforwardfile);
   	}
	else 
		removeBackupFile(forwardfile);
    }
    else {
	// If nothing written, remove the .forward file and .forward.tmp file
	SdmSystemUtility::SafeUnlink(forwardfile);
    	SdmSystemUtility::SafeUnlink(tmpforwardfile);
    }
}

void
VacationCmd::addVacationToForwardFile(char *replyInterval)
{
    char forwardfile[512], buf[1024], tmpforwardfile[512];
    int	fd, fd2=0;
    SdmBoolean forward_exists = Sdm_False;
    struct stat sbuf;

    // If vacation is already in the .forward, just start vacation and we are done
    if (vacationAlreadyStarted()) {
 	if (fork1() == 0) {
		execl("/usr/bin/vacation", "vacation", "-I", (char*)0);
		_exit(0);
	}
	return;
    }
    passwd pw;
    SdmSystemUtility::GetPasswordEntry(pw);

    // Make sure there is no Forward backup from a previous Vacation startup.
    // Maybe they manually removed the .forward file instead of turning it off 
    // thru the GUI.
    sprintf(tmpforwardfile, "%s/.forward..BACKUP", pw.pw_dir);
    if (SdmSystemUtility::SafeAccess(tmpforwardfile, F_OK) == 0) 
	SdmSystemUtility::SafeUnlink (tmpforwardfile);

    // Make sure no tmp fale either
    sprintf(tmpforwardfile, "%s/.forward.tmp", pw.pw_dir);
    if (SdmSystemUtility::SafeAccess(tmpforwardfile, F_OK) == 0) 
	SdmSystemUtility::SafeUnlink(tmpforwardfile);

    sprintf(forwardfile, "%s/.forward", pw.pw_dir);

    // If forward file exists, copy to tmp file
    if (SdmSystemUtility::SafeAccess(forwardfile, F_OK) == 0) {
    	if ((fd = SdmSystemUtility::SafeOpen(forwardfile, O_RDONLY)) >= 0) {
    		if (SdmSystemUtility::SafeFStat(fd, &sbuf) >= 0) {
			SdmBoolean unmap = Sdm_True;
			forward_exists = Sdm_True;
			char *mbuffer;
			int msize = sbuf.st_size, page_size = 
				SdmSystemUtility::HardwarePageSize();
			size_t map_size = (int) (sbuf.st_size +
                            (page_size - (sbuf.st_size % page_size)));
			mbuffer = mmap(0, map_size, PROT_READ, MAP_PRIVATE|MAP_NORESERVE, fd, 0);
			if (mbuffer == (char *)-1) {
				// Couldnt map the file, so just read it in.
				mbuffer = new char[sbuf.st_size+1];
				SdmSystemUtility::SafeRead(fd, mbuffer, 
					(unsigned int)msize);
				unmap = Sdm_False;
			}
			if (mbuffer != NULL && mbuffer != (char *)-1) {
    				if ((fd2 = SdmSystemUtility::SafeOpen(tmpforwardfile,O_WRONLY | O_CREAT)) >= 0) 
				 	SdmSystemUtility::SafeWrite(fd2, mbuffer, msize);
				if (unmap)
					munmap(mbuffer, map_size);
				else
					delete mbuffer;
			}
		}
	    	SdmSystemUtility::SafeClose(fd);
	}
    }

    // Write to tmp file first
    if (fd2 || (fd2 = SdmSystemUtility::SafeOpen(tmpforwardfile, O_RDWR | O_CREAT | O_APPEND)) > 0) {
    	createForwardVacationStr(buf, pw.pw_name, replyInterval);

    	int len = strlen(buf);
    	if (SdmSystemUtility::SafeWrite(fd2, buf, len) < len) {
	    	SdmSystemUtility::SafeClose(fd2);
		SdmSystemUtility::SafeUnlink(tmpforwardfile);
	    	return;
	}
    	SdmSystemUtility::SafeClose(fd2);
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
        SdmSystemUtility::SafeUnlink(tmpforwardfile);
    }
    else {
    	if (forward_exists)
		removeBackupFile(forwardfile);
 	if (fork1() == 0) {
		execl("/usr/bin/vacation", "vacation", "-I", (char*)0);
		_exit(0);
	}
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

	SdmSystemUtility::SafeUnlink(buf); 
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
    char messagefile[256], *fullpath;
    passwd pw;
    FILE *fd;
    DtMailGenDialog	*dialog;
    char buffer[4096];
    SdmError error;
    SdmSession * d_session = theRoamApp.isession()->isession();
    SdmMailRc * mailRc;
    SdmConnection *con = theRoamApp.connection()->connection();

    error.Reset();
    con->SdmMailRcFactory(error, mailRc);
    assert(!error);

    dialog = theRoamApp.genDialog();

    SdmSystemUtility::GetPasswordEntry(pw);

    sprintf(messagefile, "%s/.vacation.msg", pw.pw_dir);
    SdmUtility::ExpandPath(error, fullpath, messagefile, *mailRc, SdmUtility::kFolderResourceName);

    _subject = NULL;
    _body = NULL;

    // File doesn't exist
    if (!(fd = fopen(fullpath, "r"))) {
      free(fullpath);
      fd = NULL;
      return;
    }

    struct stat buf;
    if (stat(fullpath, &buf) < 0) {
        sprintf(buffer,
                catgets(DT_catd, 1, 105, "Cannot open .vacation.msg file -- No write permission."));
        dialog->setToErrorDialog("Mailer", buffer);
        dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITEVACATION);
	free(fullpath);
        fclose(fd);
        return;
    }
    free(fullpath);

    int size = buf.st_size;
    char str[MAXNAMELEN];
    char *ptr;
    while (fgets(str, MAXNAMELEN , fd)) {
      if (strncasecmp(str, "subject:", 8) == 0) {
	size -= strlen(str);
	ptr = strchr(str, ':');
	if (ptr) ptr++;
	while (ptr && *ptr == ' ') ptr++;
	if (ptr && *ptr) {
	  // Take out the ending \n
	  ptr[strlen(ptr)-1] = '\0';
	  _subject = strdup(ptr);
	}
      }
      else if (strncasecmp(str, "precedence:", 11 ) == 0 ||
	       strncasecmp(str, "from:", 5) == 0 ||
	       strncasecmp(str, "to:", 3) == 0 ||
	       strncasecmp(str, "cc:", 3) == 0 ||
	       strncasecmp(str, "bcc:", 4) == 0) {
	size -= strlen(str);
	continue;
      }
      else {
	// Must be the body, copy the rest
	if (_body != NULL) free(_body);
	_body = (char *)malloc(size+1);
	*_body = '\0';
	if (str[0] != '\n') 
	  strcat(_body, str);
	while (fgets(str, MAXNAMELEN, fd)) 
	  strcat(_body, str);
      }
    }
    
    fclose(fd);
}

int
VacationCmd::handleMessageFile()
{
    DtMailGenDialog	*dialog = theRoamApp.genDialog();
    int answer=0, fd;
    char buffer[1024*4],	messagefile[256];

    SdmString buf;

    // Check if a .forward file exists.  
    passwd pw;
    SdmSystemUtility::GetPasswordEntry(pw);

    sprintf(messagefile, "%s/.vacation.msg", pw.pw_dir);

    // See if the messagefile exists.
    // If it doesn't create one and throw in the text found in the
    // properties sheet.  If no text found, use default template.

    int msg_file_exists = SdmSystemUtility::SafeAccess(messagefile, F_OK);

    if (msg_file_exists >= 0) {
	strcpy(buffer, 
		catgets(DT_catd, 1, 106, ".vacation.msg file exists.  Replace with new text?"));
	dialog->setToQuestionDialog("Mailer", buffer);
	answer = dialog->post_and_return(DTMAILHELPEXISTSVACATION);

	if (answer == 1) {
		// backup the messageFile
		this->backupFile(messagefile);
	}
    }
    
    // If the file doesn't exist or if the user has okayed creation

    if (msg_file_exists < 0 || answer == 1) {

	fd = SdmSystemUtility::SafeOpen(messagefile, O_WRONLY | O_CREAT);
	if (fd < 0) {
	    strcpy(buffer, 
		catgets(DT_catd, 1, 107, "Cannot open .vacation.msg file -- No write permission."));
	    dialog->setToQuestionDialog("Mailer", buffer);
	    answer = dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPERROR);
	    
	    // Handle dialog indicating file not writable
	    return (-1);	
	}		
	SdmSystemUtility::SafeFChmod (fd, 0644);

	if (!_subject)
	    _subject = strdup(catgets(DT_catd, 1, 108, "I am on vacation"));
	buf += "Subject: ";
	buf += _subject;
	buf += "\nPrecedence: junk\n\n";
	
	if (!_body)
	    _body = strdup(catgets(DT_catd, 1, 109, "I'm on vacation.\nYour mail regarding \"$SUBJECT\" will be read when I return.\n"));

	buf += _body;
	if (_body && _body[strlen(_body) - 1] != '\n') {
	    buf += "\n";
	}

	if (SdmSystemUtility::SafeWrite(fd, buf, buf.Length()) < buf.Length()) {
		DtMailGenDialog *genDialog = theRoamApp.genDialog();
       		sprintf(buffer,
                	catgets(DT_catd, 3, 53, "Unable to write to %s."),
                	messagefile);
        	genDialog->setToErrorDialog(catgets(DT_catd, 3, 54, "Mailer"), buffer);
        	genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), DTMAILHELPNOWRITE);
        	SdmSystemUtility::SafeClose(fd);
        	SdmSystemUtility::SafeUnlink(messagefile);
        	return 0;
        }
	SdmSystemUtility::SafeClose(fd);
    }
    return(0);
}
void
VacationCmd::resetVacationStatus()
{
  // Cleanup vacationstartatjobid  and vacationstopatjobid resources

  char *sAtIdValue, *eAtIdValue, *date=NULL;
  time_t nowTick=0, dateTick=0;
  struct tm tm;
  SdmError error;
  SdmMailRc * mail_rc;
  SdmBoolean update = Sdm_False;

  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mail_rc);
  assert(!error);

  mail_rc->GetValue(error, "vacationstartatjobid", &sAtIdValue);
  mail_rc->GetValue(error, "vacationstopatjobid", &eAtIdValue);
  SdmBoolean vacation_started = vacationAlreadyStarted();

  if (sAtIdValue && *sAtIdValue) {
        // If vacation is already started, we dont need to hang onto the
        // the start at job id
        if (vacation_started) {
                mail_rc->RemoveValue(error, "vacationstartatjobid");
                free(sAtIdValue);
                sAtIdValue = NULL;
                update = Sdm_True;
        }
        // Vacation hasnt started, but we have an at job id, check if
        // vacation start date has passed, If so then vacation has alread
        // come and gone, so remove both stop and start job at ids.
        else {
                mail_rc->GetValue(error, "vacationstartdate", &date);
  		if (!error) {
                        strptime (date, catgets(DT_catd, 1, 238, "%m %d %Y"), &tm);
                        dateTick = mktime (&tm);
                        nowTick = time((time_t *) 0);
                }
                // If no start vacation date or if the start date has passed...

                if (nowTick >= dateTick) {
                        mail_rc->RemoveValue(error, "vacationstartatjobid");
                        mail_rc->RemoveValue(error, "vacationstopatjobid");
                        free(sAtIdValue);
                        sAtIdValue = NULL;
                        if (eAtIdValue) {
                                free(eAtIdValue);
                                eAtIdValue = NULL;
			}
                        update = Sdm_True;
                }
                if (date) { free(date); date = NULL; }
        }
  }
  // If no start date but end date: This could occur only if vacation was already turned
  // on. If vacation is not on, then that means the id is obsolete.
  else if (eAtIdValue && *eAtIdValue) {
        if (!vacation_started) {
                mail_rc->RemoveValue(error, "vacationstopatjobid");
                free(eAtIdValue);
                eAtIdValue = NULL;
                update = Sdm_True;
        }
  }

  // startatjobid will exist only if there is a start job pending
  // Set timers to update dislay and delete at job ids when vacation
  // is turned on and off.

  if (sAtIdValue && *sAtIdValue) {
        if (!dateTick) {
                mail_rc->GetValue(error, "vacationstartdate", &date);
                if (!error) {
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
        mail_rc->GetValue(error, "vacationenddate", &date);
        if (!error) {
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
	char *value;
	mail_rc->GetValue(error, "vacationatjobhost", &value);
	if (!error) {
		mail_rc->RemoveValue(error, "vacationatjobhost");
		update = Sdm_True;
		free(value);
	}
  }

  if (update) {
        mail_rc->Update(error);  
  	assert(!error);
  }
 
  if (sAtIdValue) free(sAtIdValue);
  if (eAtIdValue) free(eAtIdValue);
}
#ifdef AUTOFILING 
AutoFilingCmd::AutoFilingCmd(
    char *name,
    char *label,
    char *title,
    int active,
    UIComponent *parent
) : NoUndoCmd (name, label, active)
{
 	_mainWindow = NULL;
	_parent = parent;
	_title = title;

	_open_mbox = NULL; 
	_check_mbox = NULL; 
	_window_alive = Sdm_False; 
}

void
AutoFilingCmd::doit()
{
	int i, num_items;
	char *mptr, buf[2048], *newstr=NULL;
	SdmError error;
  	SdmMailRc *mail_rc;
        error.Reset();
	SdmConnection *con = theRoamApp.connection()->connection();
        con->SdmMailRcFactory(error, mail_rc);
  	assert(!error);
 	char *value = NULL;
	SdmStringL mbox_list;
	
	if (_mainWindow == NULL) {
        	_mainWindow = (DtbAutoFilingDialogInfo)malloc(sizeof(
                               DtbAutoFilingDialogInfoRec));
	        dtbAutoFilingDialogInfo_clear(_mainWindow);
		if (Dtb_project_catd == (nl_catd)-1)
			Dtb_project_catd = 
				catopen(DTB_PROJECT_CATALOG, NL_CAT_LOCALE);
        	dtb_auto_filing_dialog_initialize(_mainWindow, _parent->baseWidget());
        	dtb_save_toplevel_widget(_parent->baseWidget());
		XtVaSetValues(_mainWindow->dialog, XmNtitle, _title, NULL);
  		Atom WM_DELETE_WINDOW=XmInternAtom(
				 XtDisplay(_mainWindow->dialog),
				"WM_DELETE_WINDOW",
				False);
    		XmAddWMProtocolCallback(_mainWindow->dialog,
                             WM_DELETE_WINDOW,
                             ( XtCallbackProc ) &AutoFilingCmd::quitCB,
                             this);

		XtVaSetValues(_mainWindow->mailbox_list, XmNuserData, this, NULL );
		XtVaSetValues(_mainWindow->dialog, XmNuserData, this, NULL );

		sprintf(buf, "? %s", catgets(DT_catd, 1, 20, "Inbox"));
		mbox_list.AddElementToList(strdup(buf));

		mail_rc->GetValue(error, "shownewmailist", &value);
		if (!error && (mptr = (char*)strtok(value, " "))) {
			do {
				error = SdmUtility::GetRelativePath(error, 
					newstr, mptr, *mail_rc);
				sprintf(buf, "? %s", (newstr && *newstr) ? newstr : mptr);
				mbox_list.AddElementToList(strdup(buf));
				if (newstr) {
					free(newstr);
					newstr=NULL;
				}
			}
			while(mptr = strtok(NULL, " "));
  		}
		options_list_init(_mainWindow->mailbox_list, &mbox_list);
		num_items = mbox_list.ElementCount();
		for(i = 0; i < num_items; i++)
			mbox_list.RemoveElement(0);
		if (value) free(value);
	}

	this->manageWindow();
	_window_alive = Sdm_True;
}
void
AutoFilingCmd::buttonCmd(buttonId buttonid)
{
	DtMailGenDialog *genDialog = theRoamApp.genDialog();

	switch (buttonid) {

          case AF_MAILBOX_OPEN: {
		int count=0;
		XmStringTable items;
		char *str, buf[2048];
		
		XtVaGetValues(_mainWindow->mailbox_list,
			XmNselectedItemCount, &count,
			XmNselectedItems, &items,
			NULL);
		if (count > 0) {
 			if (!_open_mbox)
				_open_mbox  = new OpenMailboxCmd(
					"Open...",
					catgets(DT_catd, 1, 25, "Open..."),
					Sdm_False,
					NULL,
					NULL);

			XmStringTable replace_list = NULL;
			int replace_count = 0;

			for (int i=0; i < count; i++) {

				XmStringGetLtoR(items[i], 
					XmSTRING_DEFAULT_CHARSET, &str);

				if (str && *str != ' ') {
					if (replace_list == NULL)
						replace_list = (XmStringTable)XtMalloc(count * sizeof(XmString));
					sprintf(buf, "  %s", str+2);
					replace_list[replace_count++] = XmStringCreateLocalized(buf);
				}

				_open_mbox->setMailboxName(str+2);
				_open_mbox->doit();

			}
			if (replace_list) {
				// Replace the old with the new items
				XmListReplaceItems(_mainWindow->mailbox_list,
					items,
					replace_count,
					replace_list);
				// Reselect the newly replaced items as replacing them
				// deselects them.
				for(i = 0; i < replace_count; i++) {
					XmListSelectItem(_mainWindow->mailbox_list,
						replace_list[i], 0);
    					XmStringFree(replace_list[i]);
				}
				XtFree((char *)replace_list);
			}
		}
		else {
			genDialog->setToErrorDialog(
				catgets(DT_catd, 2, 10, "Mailer"), 
				catgets(DT_catd, 15, 17, "To open a mailbox, select it and click Open."));
			genDialog->post_and_return(
				catgets(DT_catd, 3, 29, "OK"),
				DTMAILHELPAUTOFILINGOPEN);
			return;
		}
                break;
	  }
          case AF_CHECK:
		int count;
                XmStringTable items;
                char *str, buf[2048];

		XtVaGetValues(_mainWindow->mailbox_list,
			XmNselectedItemCount, &count,
			XmNselectedItems, &items,
			NULL);

		if (count > 0) {
			static RoamMenuWindow *rmw = new RoamMenuWindow(NULL);
			rmw->initialize(0, NULL);
			if (!_check_mbox) {
				_check_mbox  = new CheckForNewMailCmd(
					"Check for New Mail",
					catgets(DT_catd, 1, 19, "Check for New Mail"),
					Sdm_True,
					NULL,
					NULL);
			}

			XmStringTable replace_list = NULL, old_list;
			int replace_count = 0;

			*buf = NULL;
                        for (int i=0; i < count; i++) {

                                XmStringGetLtoR(items[i],
					XmSTRING_DEFAULT_CHARSET, &str);

#ifdef AUTOFILING
        			rmw->setMailboxName(str);
                                _check_mbox->setMailbox(str);
                                if (_check_mbox->check()) {
#endif
				if (1) { // FIX THIS
					if (*str != '*') 
						strcat (buf, "* ");
				}
				else if (*str != ' ') 
					strcat (buf, "  ");
				if (*buf) {
					strcat(buf, str+2);
					if (replace_list == NULL) {
						replace_list = (XmStringTable)XtMalloc(count * sizeof(XmString));
						old_list = (XmStringTable)XtMalloc(count * sizeof(XmString));
					}
					replace_list[replace_count] = XmStringCreateLocalized(buf);
					old_list[replace_count++] = items[i];
					*buf = NULL;
				}
                        }
			if (replace_count > 0) {
				XmListReplaceItems(_mainWindow->mailbox_list,
					old_list,
					replace_count,
					replace_list);
				for(i = 0; i < replace_count; i++) {
					XmListSelectItem(_mainWindow->mailbox_list,
						replace_list[i], 0);
    					XmStringFree(replace_list[i]);
				}
				XtFree((char *)replace_list);
				XtFree((char *)old_list);
			}
                }
		else {
			genDialog->setToErrorDialog(
				catgets(DT_catd, 2, 10, "Mailer"),
				catgets(DT_catd, 15, 16, "To check for new mail in mailboxes,\nselect the mailboxes and click Check Selected."));
			genDialog->post_and_return(
				catgets(DT_catd, 3, 29, "OK"),
				DTMAILHELPAUTOFILINGCHECK);
			return;
		}
                break;
          case AF_CANCEL:
		this->unmanageWindow();
		_window_alive = Sdm_False;
                break;
          case AF_FILING_RULES: {
		// Get the OptCmd
    		OptCmd *oc = (OptCmd*)theRoamApp.mailOptions();
    		if (!oc) return;
		DtbOptionsDialogInfo od = 
			(DtbOptionsDialogInfo)oc->optionsDialog();
    		if (!od) {
        		// Create the Options Dialog
        		oc->execute();
        		od = (DtbOptionsDialogInfo)oc->optionsDialog();
    		}
		oc->setWithCategoryStr(catgets(DT_catd, 1, 282, "Filing and Notification"));
		options_set_category_pane(od->notification_pane);

    		XtManageChild(od->dialog_shellform);
		XtPopup(od->dialog, XtGrabNone);
                break;
	  }
          case AF_HELP:
		dtb_show_help_volume_info("Mailer.sdl", DTMAILHELPAUTOFILING);
	  default:
		break;
	}
}
void
AutoFilingCmd::manageWindow()
{
	if (_mainWindow) {
		XtManageChild(_mainWindow->dialog_shellform);
		XtPopup(_mainWindow->dialog, XtGrabNone);
	}
}
void
AutoFilingCmd::unmanageWindow()
{
	if (_mainWindow) {
		XtUnmanageChild(_mainWindow->dialog_shellform);
  		XtPopdown(_mainWindow->dialog);
	}
}
void
AutoFilingCmd::quitCB( Widget, XtPointer clientData, XmAnyCallbackStruct *)
{
    AutoFilingCmd *af=( AutoFilingCmd *) clientData;
    af->unmanageWindow();
    af->setWindowAlive(Sdm_False);
}
#endif // AUTOFILING

