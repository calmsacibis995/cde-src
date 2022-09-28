/*
 *+SNOTICE
 *
 *	$Revision: 1.21 $
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
#pragma ident "@(#)RoamMenuWindow.C	1.508 06/23/97"
#endif

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>
#if !defined(USL)
#include <strings.h>
#endif
#include <sys/utsname.h>
#ifdef SunOS
#include <sys/systeminfo.h>
#endif
#include <ctype.h>
#include <signal.h>
#include <X11/cursorfont.h>
#include <Xm/Protocols.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/RepType.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/FileSB.h>
#include <Xm/SelectioB.h>

#include <Dt/Dnd.h>
#include <Dt/Editor.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/MessageEnvelope.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/Utility.hh>

#include <DtMail/OptCmd.h>

// Pop #include <DtMail/DtMailTypes.h>
// Pop #include <DtMail/IO.hh>
// Pop #include <DtMail/PropSource.hh>
#include "SelectFileCmd.h"
#include "RoamMenuWindow.h"
#include "RoamApp.h"
#include "ButtonInterface.h"
#include "WorkingDialogManager.h"
#include "MemUtils.hh"
#include "Help.hh"
#include "DtMailHelp.hh"
#include "MailMsg.h"
#include "SendMsgDialog.h"
#include "AttachArea.h"
#include "Attachment.h"
#include "PasswordDialogManager.h"
#include <dirent.h>

#include "ComposeCmds.hh"
#include "EUSDebug.hh"

#include "SortCmd.hh"

extern force( Widget );

extern "C" {
extern int _DtGetLock( Display *display, char *lock_name) ;
extern void _DtReleaseLock( Display *display, char *lock_name) ;
extern int _DtTestLock( Display *display, char *lock_name) ;
}


DtVirtArray<long> *RoamMenuWindow::_toolbar_indices = NULL;

static const char * NormalIcon = "DtMail";
static const char * EmptyIcon = "DtMnone";
static const char * NewMailIcon = "DtMnew";
static const char * MailDragIcon = "DtMmsg";

// Used for mapping dynamic menus
static struct clientdata {
        char *filename;
        MoveCopyOpenOp op;
        time_t timestamp;
        int checksum;
};
typedef struct clientdata MenuClientData;

extern nl_catd	DtMailMsgCat;
#define MAXIMUM_PATH_LENGTH	2048
#define MAX_STATUS_LEN	128

char *
RoamMenuWindow:: _supported[] = {
  "TARGETS", "MESSAGES", "STRING", "DELETE"
};
enum {TARGETS,MESSAGES, STR, DEL};

XtResource
RoamMenuWindow::_resources[] = {
  {
    "interval",
    "Interval",
    XtRInt,
    sizeof( int ),
    XtOffset ( RoamMenuWindow *, _check_mail_interval ),
    XtRString,
    ( XtPointer ) "60000",
  },
  {
    "fullheader",
    "FullHeader",
    XtRBoolean,
    sizeof( SdmBoolean ),
    XtOffset ( RoamMenuWindow *, _fullHeader ),
    XtRBoolean,
    ( XtPointer ) 0,
  },
  {
    "mailfiles",
    "MailFiles",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamMenuWindow *, _mail_files ),
    XtRString,
    ( XtPointer ) NULL,
  },

};


void print_msgs_list(MsgNumberStruct *list, 
	MsgNumberStruct *tail, long count) 
{
  fprintf(stderr, "\ncount = %d\nlist = ", count);
  MsgNumberStruct *mns = list;
  MsgNumberStruct *next = NULL;
  while (mns != NULL) {
    fprintf(stderr, "%d,", mns->msgno);
    mns = mns->next;
  }
  fprintf(stderr, "\n");
  fprintf(stderr, "tail = %d\n", (tail?tail->msgno:0));
}

// We need to maintain a constant WM_CLASS_NAME for SQE test
// suites to run consistently for all RMWs.  We do that by
// maintaining a constant "name", as in WM_CLASS_NAME, for all
// RMWs by passing "dtmail" to a RMW's parent
// at creation time.  We distinguish individual RMWs via the
// title which indicates what mail container they are currently
// looking at.  The title is passed here as name; its dynamic
// (i.e., each RMW has its own "name", as in WM_NAME).
// If you are going to change the name, talk to SQE
// and get their consent for whatever change you are making.
//
// Note also that the name chosen must match whatever is specified
// in dtwm.fp (front panel configuration file) as the CLIENT_NAME
// for the Mail CONTROL. This is so PUSH_RECALL works.
// Typically this is the executable name.

RoamMenuWindow::RoamMenuWindow ( const char *name,
				 Tt_message tt_msg,
				 SdmToken *token,
				 SdmBoolean tmp_file) :
    MenuWindow ( "dtmail", tt_msg )
{

#ifdef DOLATER
  DtMailObjectSpace space;
#endif // DOLATER
  SdmSession *d_session = theRoamApp.isession()->isession();

  _tmp_file = tmp_file;
  _token = token;
  _async = Sdm_False;
  _is_opening_and_loading = Sdm_False;

  // Initialize private variables

  _mbox_image = NULL;
  _mbox_mask  = NULL;
  _numDialogs = 0;
  _mailbox    = NULL;
  _list	= NULL;
  _genDialog  = NULL;
  _findDialog = NULL;
  _viewsDialog = NULL;
  _fullHeader = Sdm_False;
  _dialogs    = NULL;
  _mailbox_fullpath = NULL;
  _startIconic = 0;
  _server = NULL;
    
  // Set the _mailbox_name here.  
  // In the absence of the RMW's title being set via title(),
  // the _mailbox_name is what will be used in initialize().

  if (name)
    _mailbox_name = strdup(name);
  else
    _mailbox_name = NULL;
  _is_loaded = Sdm_False;
  if (_token) {
	// Check to see if the token passed in specifies "imap".
	if (_token->CheckValue("servicetype", "imap")) {
		_is_local = Sdm_False;
	} else 
		_is_local = Sdm_True;
  } else {
    SdmMailRc *mailRc;
    SdmError mail_error;
    theRoamApp.connection()->connection()->SdmMailRcFactory(mail_error, mailRc);
    assert (mailRc != NULL);

    if (mailRc->IsValueDefined("imapinboxserver")) {
	_is_local = Sdm_False;
    } else {
	_is_local = Sdm_True;
    }
  }
  _go_away = Sdm_False;
  _is_writable = Sdm_True;
  _server_disconnected = Sdm_False;
  _restart = Sdm_False;
  _lock_status = 0;
  _passwd_dialog_up = Sdm_False;
  _requiredConversion = Sdm_False;

  _SUN_MAIL_STATUS = 0;

  _hide_show = NULL;
  _message = NULL;
  _message_summary = NULL;
  rowOfLabels = NULL;
  rowOfButtons = NULL;
  rowOfMessageStatus = NULL;
  
  _separator= new SeparatorCmd( "Separator", Sdm_True );
    
  // Mailbox Menu
    
  _check_new_mail = NULL;
  _show_views = NULL;
  _msg_select_all = NULL;
  _open_inbox	= NULL;
  _new_container = NULL;
  _rename_mailbox = NULL;
  _delete_mailbox = NULL;
  _logfile = NULL;
  _open	= NULL;
  _quit	= NULL;

  // Message Menu
  _open_msg = NULL;
  _edit_copy = NULL;
  _select_all = NULL;
  _save_msg = NULL;
  _save_msg_as = NULL;
  _print_msg = NULL;
  _print = NULL;
    
  _delete_msg = NULL;
  _undeleteLast = NULL;
  _undeleteFromList = NULL;
  _destroyDeletedMessages = NULL;

  _copy_to = NULL;
  _copy_inbox = NULL;
  _copyMenu = NULL;
  _first_cached_copy_item = 0;
  _copy_containerlist = NULL;
  _cached_copy_containerlist = NULL;

  _first_cached_open_item = 0;
  _openmailboxcmd_list = new DtVirtArray<OpenMailboxCmd *> (3);
  _cached_open_containerlist = NULL;

  _openMenu = NULL;

  // Cascade Widgets
  _message_cascade = NULL;
  _file_cascade = NULL;
  _compose_cascade = NULL;
  _move_cascade = NULL;
    
  // View Menu
    
  _abbrevHeaders = NULL;
  _charset = NULL;
  _previous = NULL;
  _next = NULL;
  _find = NULL;
  _sortTD = NULL;
  _sortSender = NULL;	
  _sortSubject = NULL;	
  _sortSize = NULL;	
  _sortStatus = NULL;
  _lastSortCmd = NULL;
    
  // Options Menu
  _opt_basic = NULL;
  _opt_tb = NULL;
  _opt_mv = NULL;
  _opt_mf = NULL;
  _opt_no = NULL;
  _opt_cw = NULL;
  _opt_sig = NULL;
  _opt_vac = NULL;
  _opt_td = NULL;
  _opt_al = NULL;
  _opt_ad = NULL;
    
  // Attachment menu
    
  _save_attachment = NULL;
  _select_all_attachments = NULL;

  _attachmentActionsList = NULL;
  _attachmentMenuPR = NULL;
  _attachmentMenuList = NULL;

  _attachment_popup_title = NULL;
  _attachmentPopupMenuList = NULL;
  _textPopupMenuList = NULL;
  _text_popup_title = NULL;

  // Message Popup

  _menuPopupMsgs = NULL;
  _msgsPopupMoveMenu = NULL;
  _msgsPopupMenu = NULL;
  _msgsPopupMenuList = NULL;
  _msgs_popup_title = NULL;
    
  // Compose Menu
    
  _comp_new = NULL;
  _forward = NULL;
  _forwardBody = NULL;
    

  _replySender = NULL;
  _replyAll = NULL;
  _replySinclude = NULL;
  _replyAinclude = NULL;
  _templates = NULL;
  _drafts = NULL;
    
  my_editor = NULL;
    
  // Move Menu
  _moveMenu = NULL;
  _move_menu_cmdList = NULL;
  _move_popup_menu_cmdList = NULL;
  _move_inbox = NULL;
  _move_containerlist = NULL;
  _cached_move_containerlist = NULL;
  _cached_list_size = 0;
  _max_cached_list_size = 0;
  _user_list_size = 0;
  _first_cached_move_item = 0;
  _filemenu2 = NULL;
  _folder = theRoamApp.getFolderDir(local, Sdm_True);
  _imapfolder = theRoamApp.getFolderDir(imap, Sdm_True);
  _logfilename = NULL;
  _movecopymenucmd_list = new DtVirtArray<MoveCopyMenuCmd *> (3);

  // Help Menu
  _overview = NULL;
  _tasks = NULL;
  _reference = NULL;
  _messages = NULL;
  _on_item = NULL;
  _using_help = NULL;
  _about_mailer = NULL;
    
  _move_copy_button = NULL;
  _open_other_mboxes = NULL;
  _move_other_mboxes = NULL;
  _move_popup_other_mboxes = NULL;
  _copy_other_mboxes = NULL;
  _mailbox_cmdList = NULL;
  _message_cmdList = NULL;
  _compose_cmdList = NULL;
  _view_cmdList = NULL;
  _option_cmdList = NULL;
  _help_cmdList = NULL;

  _toolbar_cmds = NULL;
  _toolbar_icons = NULL;
  _toolbar_inactive_icons = NULL;
  _toolbar_mailrc_string = NULL;
  _toolbar_use_icons = Sdm_False;

  _mailbox_index = 0;
  _isstartup = Sdm_True;
  _openContainerCmd = NULL; 

  _recent_msgs_head = NULL;
  _recent_msgs_tail = NULL;
  _recent_msgs_cnt = 0;
  _expunging = Sdm_False;
   
  // See if this RMW is for the inbox 
    
  // We need to find out what the name of the inbox is, so that we
  // can compare it with the name of the mailbox we are opening.

  char *inbox_name;
  inbox_name = theRoamApp.GetInbox();

  // We should be a little more diligent at comparing files than
  // just a strcmp.  We should follow any links for both files.

  SdmBoolean inbox_server = Sdm_False;
  if (!_is_local) {

    if (_token) {
        const char *inbox_server_name = (const char*)theRoamApp.getInboxServer();
        const char *server_name = NULL;
        SdmString server;

        _token->FetchValue(server, "serviceoption", "hostname");
        server_name = (const char *)server;
    
        if (inbox_server_name && (server.Length() == 0 || strcmp(server_name, inbox_server_name) == 0))
          inbox_server = Sdm_True;
     } else {
	// in this case we will be defaulting to the inbox_server_name
        inbox_server = Sdm_True;
     }
  }

  if (_mailbox_name && inbox_name && strcmp(inbox_name, _mailbox_name) == 0 &&
      ((_is_local && strcmp(_mailbox_name, "INBOX") != 0) || inbox_server)) {
    _inbox = Sdm_True;
  }
  else {
    _inbox = Sdm_False;
  }
}

RoamMenuWindow::~RoamMenuWindow() 
{
//   delete	_list;
  // Release the lock on the new mail property.
  if (_lock_status)
      _DtReleaseLock(XtDisplay(baseWidget()), "_SUN_MAIL_STATUS");
  if (_list != NULL)
    _list->deleteMe();

  SdmError	error;

  // Initialize the mail_error.
  error.Reset();

  if (_tmp_file) {
    SdmSystemUtility::SafeUnlink(_mailbox_fullpath);
  }

  theRoamApp.isession()->close(error, _mailbox);
  if ( error ) {
    // Cannot return error to caller since there's
    // no caller.  What do we do?
    // exit(0)??
  }

  _openCmdlist->deleteMe();

  // Mailbox Menu
    
  delete	_check_new_mail;
  delete	_show_views;
  delete	_open_inbox;
  delete  _msg_select_all;
  if (_new_container->getClientData() != NULL) {
    free ((struct OpenStruct*)_new_container->getClientData());
  }
  delete	_new_container;
  if (_delete_mailbox->getClientData() != NULL) {
    free ((struct OpenStruct*)_delete_mailbox->getClientData());
  }
  delete	_delete_mailbox;
  if (_rename_mailbox->getClientData() != NULL) {
    free ((struct OpenStruct*)_rename_mailbox->getClientData());
  }
  delete	_rename_mailbox;
  delete	_logfile;
  if (_open->getClientData() != NULL) {
    free ((struct OpenStruct*)_open->getClientData());
  }
  delete	_open;
  
  if (_open_other_mboxes && _open_other_mboxes->getClientData() != NULL) {
    MenuClientData* mcd = (MenuClientData*) _open_other_mboxes->getClientData();
    if (mcd->filename != NULL) 
      free (mcd->filename);
    free (mcd);
  }
  _open_other_mboxes->deleteMe();
  
  if (_move_other_mboxes && _move_other_mboxes->getClientData() != NULL) {
    MenuClientData* mcd = (MenuClientData*) _move_other_mboxes->getClientData();
    if (mcd->filename != NULL) 
      free (mcd->filename);
    free (mcd);
  }
  _move_other_mboxes->deleteMe();

  if (_move_popup_other_mboxes && _move_popup_other_mboxes->getClientData() != NULL) {
    MenuClientData* mcd = (MenuClientData*) _move_popup_other_mboxes->getClientData();
    if (mcd->filename != NULL) 
      free (mcd->filename);
    free (mcd);
  }
  _move_popup_other_mboxes->deleteMe();
    
  if (_copy_other_mboxes && _copy_other_mboxes->getClientData() != NULL) {
    MenuClientData* mcd = (MenuClientData*) _copy_other_mboxes->getClientData();
    if (mcd->filename != NULL) 
      free (mcd->filename);
    free (mcd);
  }
  _copy_other_mboxes->deleteMe();
  delete	_destroyDeletedMessages;
  delete	_quit;

  _mailbox_cmdList->deleteMe();
  _message_cmdList->deleteMe();
  _compose_cmdList->deleteMe();
  _view_cmdList->deleteMe();
  _option_cmdList->deleteMe();
  _help_cmdList->deleteMe();

  // View Menu
    
  delete	_next;
  delete	_previous;
  delete	_abbrevHeaders;
  delete	_charset;
  delete	_sortTD;
  delete	_sortSender;	
  delete	_sortSubject;	
  delete	_sortSize;	
  delete	_sortStatus;
    
  // Message Menu
    
  delete	_open_msg;
  delete	_edit_copy;
  delete	_select_all;
  delete	_save_msg;
  delete	_save_msg_as;
  _copy_to->deleteMe();
  delete	_copy_inbox;
  delete	_print_msg;
  delete	_print;
  delete	_find;
  delete	_delete_msg;
  delete	_undeleteLast;
  delete	_undeleteFromList;

  // Move menu

  _move_menu_cmdList->deleteMe();
  _move_popup_menu_cmdList->deleteMe();
  delete	_move_inbox;
  delete	_move_containerlist;
  delete	_cached_move_containerlist;
  if (_move_copy_button->getClientData() != NULL) {
    free ((struct OpenStruct*)_move_copy_button->getClientData());
  }
  delete  _move_copy_button;

  if (_movecopymenucmd_list != NULL) {
    while (_movecopymenucmd_list->length()) {
      delete (*_movecopymenucmd_list)[0];
      _movecopymenucmd_list->remove(0);
    }
    delete (_movecopymenucmd_list);
  }
  
  delete _cached_open_containerlist;
  if (_openmailboxcmd_list != NULL) {
    while (_openmailboxcmd_list->length()) {
      delete (*_openmailboxcmd_list)[0];
      _openmailboxcmd_list->remove(0);
    }
    delete (_openmailboxcmd_list);
  }



  // Copy menu
  delete _copy_containerlist;
  delete _cached_copy_containerlist;
  
  // Options Menu
  delete _opt_basic;  
  delete _opt_tb; 
  delete _opt_mv; 
  delete _opt_mf; 
  delete _opt_no;
  delete _opt_cw;
  delete _opt_sig;    
  delete _opt_vac; 
  delete _opt_td; 
  delete _opt_al; 
  delete _opt_ad;

  // Attachment menu
    
  delete	_save_attachment;
  delete	_select_all_attachments;

  if (_attachmentActionsList != NULL) {
    _attachmentMenuPR->removeCommands(NULL, _attachmentActionsList);
    _menuPopupAtt->removeCommands(_attachmentPopupMenu, 
                                  _attachmentActionsList);
                                  
    while (_attachmentActionsList->size()) {
      delete (*_attachmentActionsList)[0];
      _attachmentActionsList->remove(0);
    }
    _attachmentActionsList->deleteMe();
  }
  delete  _attachmentMenuPR;
  _attachmentMenuList->deleteMe();
  _attachmentPopupMenuList->deleteMe();
  delete  _attachment_popup_title;
  if (_textPopupMenuList != NULL) {
    _textPopupMenuList->deleteMe();
  }
  delete  _text_popup_title;
  _msgsPopupMenuList->deleteMe();
  delete  _msgs_popup_title;
  delete 	_menuPopupMsgs;
    
  // Compose Menu
    
  delete	_comp_new;
  delete	_forward;
  delete	_forwardBody;
  
  if (_templates != NULL) {
    while (_templates->size()) {
      delete (*_templates)[0];
      _templates->remove(0);
    }
    _templates->deleteMe();
  }

  if (_drafts && _drafts->getClientData() != NULL) {
    RAMenuClientData* mcd = (RAMenuClientData*) _drafts->getClientData();
    for (int i=0; i<2; i++) {
      if (mcd[i].filename != NULL) 
        free (mcd[i].filename);
    }
    free (mcd);
  }
  _drafts->deleteMe();
    
  delete	_replySender;
  delete	_replyAll;
  delete	_replySinclude;
  delete	_replyAinclude;
    
  delete	my_editor;
    
    
  // Help Menu
  delete	_overview;
  delete	_tasks;
  delete	_reference;
  delete	_messages;
  delete	_on_item;
  delete	_using_help;
  delete	_about_mailer;
    
  delete	_openContainerCmd;
  
  // Attachment
    
  // Generic
    
  delete	_genDialog;
  delete [] _dialogs;
    
  // Mailbox Image
  if (_mbox_image)
    XmDestroyPixmap(XtScreen(baseWidget()), _mbox_image);
  if (_mbox_mask)
    XmDestroyPixmap(XtScreen(baseWidget()), _mbox_mask);
  if (_mailbox_fullpath)
    free(_mailbox_fullpath);
  if (_mailbox_name)
    free(_mailbox_name);
  if (_logfilename)
    free(_logfilename);

  if (_findDialog)
    delete _findDialog;

  if (_viewsDialog)
    delete _viewsDialog;
 
  theRoamApp.unregisterMailView(_mailbox_index);
  _toolbar_cmds->deleteMe();
  delete _toolbar_icons;
  delete _toolbar_inactive_icons;
  free (_toolbar_mailrc_string);
  _toolbar_mailrc_string = NULL;
  delete _separator;
  delete _token;
  if (_folder) free(_folder);
  if (_imapfolder) free(_imapfolder);
  if (_server) free(_server);

  MsgNumberStruct *mns = _recent_msgs_head;
  MsgNumberStruct *next = NULL;
  while (mns != NULL) {
    next = mns->next;
    delete mns;
    mns = next;
  }
}

void
RoamMenuWindow::setToNormalIcon()
{
    setIconName(NormalIcon);

    //  We want to let the front panel know when we detect
    // new mail.  We only do this if we have a lock on
    // the atom "_SUN_MAIL_STATUS".
    if (_lock_status) {
	MailStatus *new_mail_data;

	new_mail_data = (MailStatus *) malloc(sizeof(MailStatus));
	new_mail_data->new_mail = False;
	XChangeProperty(XtDisplay(baseWidget()),
			XtWindow(baseWidget()),
			_SUN_MAIL_STATUS, _SUN_MAIL_STATUS,
			32, PropModeReplace,
			(unsigned char *) new_mail_data,
			1);   // The "1" means we are passing one element.
	free(new_mail_data);
    }
}

void
RoamMenuWindow::setToEmptyIcon()
{
    setIconName(EmptyIcon);

    //  We want to let the front panel know when we detect
    // new mail.  We only do this if we have a lock on
    // the atom "_SUN_MAIL_STATUS".
    if (_lock_status) {
	MailStatus *new_mail_data;

	new_mail_data = (MailStatus *) malloc(sizeof(MailStatus));
	new_mail_data->new_mail = False;
	XChangeProperty(XtDisplay(baseWidget()),
			XtWindow(baseWidget()),
			_SUN_MAIL_STATUS, _SUN_MAIL_STATUS,
			32, PropModeReplace,
			(unsigned char *) new_mail_data,
			1);   // The "1" means we are passing one element.
	free(new_mail_data);
    }
}

void
RoamMenuWindow::initialize(int argc, char **argv)
{
    XmRepTypeInstallTearOffModelConverter();

    MenuWindow::initialize(argc, argv);

    if (!this->baseWidget()) {
      fprintf(stderr, 
        catgets(DT_catd, 2, 3,
        "Unable to initialize windows. Exiting.\n"));
      exit(1);
    }

    setStatus(catgets(DT_catd, 3, 2, "Initializing..."));

//     busyCursor();

    // GL - Why are we calling XInternAtom here?  We don't even keep
    // the value that is returned.  This is meaningless code if you
    // don't save the return value.
//     XInternAtom( XtDisplay(this->baseWidget()) ,"STRING", Sdm_False);
//     XInternAtom( XtDisplay(this->baseWidget()) ,"MESSAGES", Sdm_False);

    getResources( _resources, XtNumber ( _resources ) );

    // Mailbox image & mask
    {
	Screen *screen = XtScreen(baseWidget());
	char    icon_name[256];

	sprintf(icon_name, "%s.m", MailDragIcon);
	_mbox_image = XmGetPixmap(screen, icon_name,
	    BlackPixelOfScreen(screen), WhitePixelOfScreen(screen));

	sprintf(icon_name, "%s.m_m.bm", MailDragIcon);
	_mbox_mask = XmGetPixmapByDepth(screen, icon_name, 1, 0, 1);

	if (_mbox_image == XmUNSPECIFIED_PIXMAP ||
	    _mbox_mask == XmUNSPECIFIED_PIXMAP) {

	    if (_mbox_image && _mbox_image != XmUNSPECIFIED_PIXMAP)
        	XmDestroyPixmap(screen, _mbox_image);

	    if (_mbox_mask && _mbox_mask != XmUNSPECIFIED_PIXMAP)
        	XmDestroyPixmap(screen, _mbox_mask);

	    _mbox_image = _mbox_mask = NULL;
	}
    }
    
    if (inbox()) {
	Display *display = XtDisplay(baseWidget());
	_SUN_MAIL_STATUS = XmInternAtom(display, "_SUN_MAIL_STATUS", False);

	_lock_status = _DtGetLock(display, "_SUN_MAIL_STATUS");
    }

    // Add an event handler for structureNotify.
    // This EH will handle exposure, configure notifies ...

    XtAddEventHandler( this->baseWidget(), StructureNotifyMask,
		       Sdm_False,
		       ( XtEventHandler ) &RoamMenuWindow::structurenotify,
		       (XtPointer) this );

    _openContainerCmd = new OpenContainerCmd("openContainer", "openContainer",
                                             Sdm_True, this);

    _genDialog = new DtMailGenDialog("Dialog", _main, 
                       XmDIALOG_PRIMARY_APPLICATION_MODAL);

    // register ourselves with the RoamApp object and then record what number
    // mailbox we are.
    setMailboxIndex(theRoamApp.registerMailView(this));

    // Defer creation of toolbar until window is opened, if started iconic.
    // Otherwise the window will get managed and appear open.
    if (!startedIconic()) {
        manage();
    	buildToolbar();
    }

    // Now that the toolbar buttons have been created and managed, we
    // will manage all of the children of form2 (a widget local to createWorkArea).
    // These children are rowOfLabels, rowOfButtons, and the list widget.
    WidgetList children;
    int num_children;
    XtVaGetValues(XtParent(rowOfButtons),
		  XmNchildren, &children,
		  XmNnumChildren, &num_children,
		  NULL);
    XtManageChildren(children, num_children);

    if (!startedIconic()) {
      manage();
    }

    // Initialize this variable for use in propsChanged()
    SdmError error;
    SdmMailRc *mailrc;
    MailConnection *mailConnection = theRoamApp.connection();
    mailConnection->connection()->SdmMailRcFactory(error, mailrc);
    MailRcSource *mrc = (MailRcSource*)new MailRcSource (mailrc, "record");
    _logfilename = (char*)mrc->getValue();
    setToNormalIcon();
}

SdmBoolean
RoamMenuWindow::inbox()
{
    return(_inbox);
}

void
RoamMenuWindow::checkForMail(SdmError &error)
{

    SdmMessageNumber r_nummessages;

    // First order of business - busy out active windows
    theRoamApp.busyAllWindows(catgets(DT_catd, 3, 86, "Checking for new mail..."));

    _mailbox->CheckForNewMessages(error, r_nummessages);
    if (!error && r_nummessages > 0) {
        this->_list->load_new_headers(error, r_nummessages);
    }
    
    if (error) {
      _genDialog->post_error(error, DTMAILHELPCANNOTCHECKNEWMAIL, 
			     catgets(DT_catd, 2, 26, "Mailer cannot check if you have new email."));
    }

    // Last order of business - unbusy out active windows
    theRoamApp.unbusyAllWindows();
}

Widget
RoamMenuWindow::createWorkArea ( Widget parent )
{
    Widget form1, panedW;
    Widget form2, form3;
    Dimension x, y, width, height, bwid;
//     XtWidgetGeometry size;
    VacationCmd *vacation_cmd = theRoamApp.vacation();

    form1 = XmCreateForm(parent, "Work_Area",  NULL, 0);
    XtVaSetValues(form1, XmNrightAttachment,XmATTACH_FORM,
                  XmNleftAttachment,XmATTACH_FORM,
                  XmNtopAttachment,XmATTACH_FORM,
                  XmNbottomAttachment, XmATTACH_FORM, NULL);

    XtAddCallback(form1, XmNhelpCallback, HelpCB, DTMAILWINDOWID);

    panedW = XtVaCreateWidget("panedW", xmPanedWindowWidgetClass,
			      form1,
			      XmNrightAttachment,XmATTACH_FORM, 
			      XmNleftAttachment,XmATTACH_FORM, 
			      XmNtopAttachment,XmATTACH_FORM,
			      XmNbottomAttachment, XmATTACH_FORM,
			      XmNsashIndent, -30,
			      XmNsashWidth, 18,
			      XmNsashHeight, 12,
			      NULL );

//     XtVaSetValues(panedW,
// 		  XmNrightAttachment,XmATTACH_FORM, 
// 		  XmNleftAttachment,XmATTACH_FORM, 
// 		  XmNtopAttachment,XmATTACH_FORM,
// 		  XmNbottomAttachment, XmATTACH_FORM,
// 		  XmNsashIndent, -30,
// 		  XmNsashWidth, 18,
// 		  XmNsashHeight, 12,
// 		  NULL );


    form2 = XtCreateWidget("form2", xmFormWidgetClass,
			   panedW, NULL, 0);

    rowOfLabels = XtCreateWidget("RowOfLabels", 
				 xmFormWidgetClass,
				 form2,
				 NULL, 0);

    XtAddCallback(rowOfLabels, XmNhelpCallback, HelpCB,
		  DTMAILWINDOWROWOFLABELSID);


//     size.request_mode = CWHeight;
//     XtQueryGeometry(rowOfLabels, NULL, &size);

    XtVaSetValues(rowOfLabels, 
		  XmNrightAttachment,XmATTACH_FORM, 
		  XmNleftAttachment,XmATTACH_FORM, 
		  XmNtopAttachment,XmATTACH_FORM,
		  XmNtopOffset, 5,
		  XmNorientation, XmHORIZONTAL,
		  XmNspacing, 120,
		  XmNmarginWidth, 10,
		  XmNentryAlignment, XmALIGNMENT_CENTER,
		  NULL);

    _list = new MsgScrollingList( this, form2, "Message_List");

    msgListDragSetup();

    SdmError error;
    SdmMailRc *mailrc;
    MailConnection *mailConnection = theRoamApp.connection();
    SdmSession * d_session = theRoamApp.isession()->isession();
    mailConnection->connection()->SdmMailRcFactory(error, mailrc);
    assert(!error);
    int	msgnums = Sdm_False;
    int header_lines;

    error.Reset();

    if (mailrc->IsValueDefined("showmsgnum"))
	msgnums = Sdm_True;

    this->addToRowOfLabels(_list, msgnums);

    // See if the header size has changed.
    //
    char *value;
    mailrc->GetValue(error, "headerlines", &value);
    if ( error ) {
    	header_lines = 15;
        error.Reset();
    }
    else 
    	header_lines = (int) strtol(value, NULL, 10);
    // Dont ask me why i have to add 5 to this, but it works!
    // Only do this if text buttons, not icons.
    if (mailrc->IsValueDefined("toolbarusetext")) 
    	_list->visibleItems(header_lines+5);
    else
    	_list->visibleItems(header_lines+2);
	
    XtAddCallback( _list->get_scrolling_list(),
		   XmNhelpCallback, HelpCB, DTMAILMSGLISTID);

    XtVaSetValues(_list->get_scrolling_list(), XmNuserData, this, NULL);

    rowOfButtons = XtVaCreateWidget("Toolbar", 
				    xmRowColumnWidgetClass,
				    form2,
				    XmNorientation, XmHORIZONTAL,
				    XmNpacking, XmPACK_TIGHT,
				    XmNspacing, 6,
				    XmNrightAttachment, XmATTACH_FORM, 
				    XmNleftAttachment, XmATTACH_FORM, 
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNresizeHeight, Sdm_True,
				    XmNresizeWidth, Sdm_False,
				    XmNadjustLast, Sdm_False,
				    NULL );

    XtVaSetValues(XtParent(_list->get_scrolling_list()),
		  XmNrightAttachment,XmATTACH_FORM, 
		  XmNleftAttachment,XmATTACH_FORM, 
		  XmNtopAttachment,XmATTACH_WIDGET,
		  XmNtopWidget, rowOfLabels,
		  XmNtopOffset, 3,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, rowOfButtons,
		  XmNbottomOffset, 7,
		  NULL );

    form3 = XtCreateWidget("form3", xmFormWidgetClass,
			   panedW, NULL, 0);

    rowOfMessageStatus = XtVaCreateWidget("Message_Status",
				  xmFormWidgetClass,
				  form3,
				  XmNrightAttachment,XmATTACH_FORM, 
				  XmNleftAttachment,XmATTACH_FORM, 
				  XmNtopAttachment,XmATTACH_FORM,
				  XmNtopOffset, 5,
				  NULL );

    this->addToRowOfMessageStatus();

    my_editor = new DtMailEditor(form3, this);

    my_editor->initialize();
    my_editor->attachArea()->setOwnerShell(this);
    my_editor->unmanageAttachArea();

    if (value) free(value);
    mailrc->GetValue(error, "toolcols", &value);
    if ( !error && value && *value){
       int cols = (int) strtol(value, NULL, 10);
       Widget twid = my_editor->textEditor()->get_editor();
       XtVaSetValues(twid, DtNcolumns, cols, NULL);
    }
    if (value) free(value);

    // DtMailEditor contains a widget that contains the textEditor and 
    // attachArea.  Get that widget...

    Widget wid = my_editor->container();

    XtVaSetValues(wid,
		  XmNrightAttachment,XmATTACH_FORM, 
		  XmNleftAttachment,XmATTACH_FORM, 
		  XmNtopAttachment,XmATTACH_WIDGET, 
		  XmNtopWidget, rowOfMessageStatus,
		  XmNbottomAttachment, XmATTACH_FORM,
		  NULL );

    _mailbox_fullpath = strdup(_mailbox_name);
    // strip consecutive ending path delimiter at end of full path name.
    int len;
    if (_mailbox_fullpath && ((len = strlen(_mailbox_fullpath)) > 0)) {
      int j = len-1;
      int lastDelim = -1;
      while (_mailbox_fullpath[j] == _mailbox_fullpath[0] && j >= 0) {
        lastDelim = j;
        j--;
      }
      if (lastDelim >= 0) {
        _mailbox_fullpath[lastDelim] = '\0';
      }
    }
       

    // Set the appShell's title to be _mailbox_fullpath

    if (vacation_cmd != NULL && vacation_cmd->vacationIsOn()) {
	char * new_title = new char[strlen(_mailbox_fullpath) +
		     strlen(catgets(DT_catd, 1, 3, "Vacation")) + 10];
	sprintf(new_title, "%s [%s]", _mailbox_fullpath,
		     catgets(DT_catd, 1, 3, "Vacation"));
	title(new_title);
	delete [] new_title;
    } else {
	title(_mailbox_fullpath);
    }

    // Set the icon name to be the folder name.  Extract foldername
    // from full path.

    char *fname = strrchr(_mailbox_fullpath, '/');

    if (fname == NULL) {
	// path has no slash in it.
	setIconTitle(_mailbox_fullpath);
    } else {
	// Extract the filename from it.
	fname++;
	setIconTitle(fname);
    }

    // manage the children of form3 all together for perf reasons
    WidgetList children;
    int count;
    XtVaGetValues(form3,
		  XmNchildren, &children,
		  XmNnumChildren, &count,
		  NULL);
    XtManageChildren(children, count);
    XtVaGetValues(form2,
		  XmNchildren, &children,
		  XmNnumChildren, &count,
		  NULL);
    XtManageChildren(children, count);

    // Manage the 
    XtManageChild(_list->baseWidget());

    Widget w[2];
    w[0] = form2;
    w[1] = form3;
    XtManageChildren(w, 2);
    XtManageChild(panedW);
    XtManageChild(form1);
    
    XtVaGetValues(_main,
		  XmNx, &x,
		  XmNy, &y,
		  XmNwidth, &width,
		  XmNheight, &height,
		  XmNborderWidth, &bwid,
		  NULL);

    _x = x;
    _y = y;
    _width = width;
    _height = height;
    _border_width = bwid;
    return form1;
}

// msgListTransferCallback
//
// Handles the transfer of data that is dragged and dropped to the
// MsgScrollingList.  Files and buffers are transferred by inserting 
// them as messages into the scrolling list.
//
void
RoamMenuWindow::msgListTransferCallback(
	Widget,
	XtPointer	clientData,
	XtPointer	callData)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *) clientData;
    DtDndTransferCallbackStruct *transferInfo =
			(DtDndTransferCallbackStruct *) callData;
    DtDndContext        *dropData = transferInfo->dropData;
    char		*filepath;
    int			i, ii, numItems;
    SdmMessageStore	*mbox, *tmpmbox;
    SdmToken		token;
    SdmError		mail_error;
    SdmMessage		*msg;
    SdmMessageStoreStatus mboxStatus;
    long		numOfMessages;
    static const char	*temp = "mbox.XXXXXX";
    char		fileName[16];
    char		*tmpFile = fileName;
    char		fullFileName[MAXPATHLEN + 16];
    MailSession *session = theRoamApp.isession();
    SdmMailRc * mailRc;

    // Initialize the mail_error.
    mail_error.Reset();

    numItems = dropData->numItems;

    switch (dropData->protocol) {
	case DtDND_FILENAME_TRANSFER:

	    token.SetValue("servicechannel", "cclient");
	    token.SetValue("serviceclass", "messagestore");
	    token.SetValue("servicetype", "local");

	    // Set the hostname of the IMAP server.
	    //token.SetValue("serviceoption", "hostname",
	    //		thePasswordDialogManager->ImapServer());
	    token.SetValue("serviceoption", "username",
			    thePasswordDialogManager->User());
	    token.SetValue("serviceoption", "password",
			    thePasswordDialogManager->password());
	    theRoamApp.connection()->connection()->SdmMailRcFactory(mail_error, mailRc);
	    assert(!mail_error);
	    if (mailRc->IsValueDefined("dontsavemimemessageformat"))
		    token.SetValue("serviceoption", "preservev3messages", "x");

    	    // Loop through the files that were dropped onto the msg list

    	    for (ii = 0; ii < numItems; ii++) {

		// Try to open the file as a mail container.
		filepath = dropData->data.files[ii];
		token.SetValue("serviceoption", "readonly", "x");
		token.SetValue("serviceoption", "messagestorename",
				dropData->data.files[ii]);

		SdmBoolean readOnly = Sdm_False;
                SdmMessageNumber nmsgs = 0;
		tmpmbox = session->open(mail_error, nmsgs, readOnly, token, rmw);

		if (tmpmbox && !mail_error) {
		    // Make sure we didn't open the file read only.
		    // We were able to open the container, so now we get the 
		    // current mailbox and copy all the messages into it.
		    mbox = rmw->mailbox();
#ifdef USE_STATUS
		    tmpmbox->GetStatus(mail_error, mboxStatus,
				(SdmMessageStoreStatusFlags)Sdm_MSS_Messages);
#endif
		    if (!mail_error) {
#ifdef USE_STATUS
			numOfMessages = mboxStatus.messages;
#else
                        numOfMessages = nmsgs;
#endif
			// Copy messages from tmpmbox to mbox
			for (i = 1; i <= numOfMessages; i++) {
			    tmpmbox->SdmMessageFactory(mail_error, msg, i);
			    if (mail_error) break;
			    mbox->AppendMessage(mail_error, *msg);
			    if (mail_error) break;
			}
		    }
		} 

		// Reject the drop if an error occurred.
		if (mail_error || !tmpmbox) {
		    transferInfo->status = DtDND_FAILURE;
		    if (tmpmbox) {
			session->close(mail_error, tmpmbox);
			delete tmpmbox;
		    }

		    return;

		} else {
		    rmw->checkForMail(mail_error);
		    transferInfo->status = DtDND_SUCCESS;
		    if (tmpmbox) {
			session->close(mail_error, tmpmbox);
			delete tmpmbox;
		    }
		}
	    }
	    break;

	case DtDND_BUFFER_TRANSFER:
	    // Loop through the buffers that were dropped onto the msg list

	    token.SetValue("servicechannel", "cclient");
	    token.SetValue("serviceclass", "messagestore");
	    token.SetValue("servicetype", "local");
	    theRoamApp.connection()->connection()->SdmMailRcFactory(mail_error, mailRc);
	    assert(!mail_error);
	    if (mailRc->IsValueDefined("dontsavemimemessageformat"))
		token.SetValue("serviceoption", "preservev3messages", "x");

	    for (ii = 0; ii < numItems; ii++) {

		int fd;
		char *cwd;

		// Turn the dropped data into a mail buffer to pass to
		// the mailbox constructor.
		strcpy(fileName, temp);
		mktemp(fileName);
		cwd = getcwd(NULL, MAXPATHLEN);
		strcpy(fullFileName, cwd);
		strcat(fullFileName, "/");
		strcat(fullFileName, tmpFile);
		free(cwd);

		// Turn the dropped data into a mail buffer to pass to
		// the mailbox constructor.

		// Create the auto save file - since this is a behind
		// the scenes kind of deal and it is an e-mail, we use
		// mode 0600 as opposed to mode 0666 for security reasons.

		if ( (fd = SdmSystemUtility::SafeOpen(tmpFile, O_CREAT | O_WRONLY, 0600)) == -1 ) {
		    transferInfo->status = DtDND_FAILURE;
		    return;
		}

		token.SetValue("serviceoption", "readonly", "x");
		token.SetValue("serviceoption", "messagestorename", fullFileName);

		// Write the DnD buffer data into a temporary file
		write(fd, (char *) transferInfo->dropData->data.buffers->bp,
		transferInfo->dropData->data.buffers->size);

		SdmSystemUtility::SafeClose(fd);

		// Convert the buffer into a mailbox object.
		session->isession()->SdmMessageStoreFactory(mail_error, tmpmbox);
		if (!mail_error) {
		  tmpmbox->StartUp(mail_error);
		} 
                SdmMessageNumber nmsgs = 0;
                SdmBoolean readOnly;
		if (!mail_error) {
		  tmpmbox->Open(mail_error, nmsgs, readOnly, token);
		}

		// Reject the drop if we weren't able to convert it to a mailbox
		if ( !tmpmbox || mail_error ) {
		    transferInfo->status = DtDND_FAILURE;
		    if (tmpmbox) {
			session->close(mail_error, tmpmbox);
			delete tmpmbox;  // this will shutdown and close temp mailbox.
		    }
		    SdmSystemUtility::SafeUnlink(tmpFile);
		    return;

		} else {
		    // Parse the dropped data into the tmpmbox.
		    mbox = rmw->mailbox();
#ifdef USE_STATUS
		    tmpmbox->GetStatus(mail_error, mboxStatus,
			(SdmMessageStoreStatusFlags)Sdm_MSS_Messages);
#endif
		    if (!mail_error) {
#ifdef USE_STATUS
			numOfMessages = mboxStatus.messages;
#else
                        numOfMessages = nmsgs;
#endif
			// Copy messages from tmpmbox to mbox
			for (i = 1; i <= numOfMessages; i++) {
			    tmpmbox->SdmMessageFactory(mail_error, msg, i);
			    if (mail_error) break;
			    mbox->AppendMessage(mail_error, *msg);
			    if (mail_error) break;
			}
		    }

			// Destroy tmpmbox.  this will shutdown and close it.
			session->close(mail_error, tmpmbox);
			delete tmpmbox;
			SdmSystemUtility::SafeUnlink(tmpFile);

			// return failure if any error occurred.
			if (mail_error) {			
			  transferInfo->status = DtDND_FAILURE;
			  return;
			}
			else {
			  transferInfo->status = DtDND_SUCCESS;
			}

			rmw->checkForMail(mail_error);
		    }
		}
		break;

	    default:
		transferInfo->status = DtDND_FAILURE;
		return;
    }
}

// msgListDropRegister
//
// Register the message list to accept file & buffer drops
//
void
RoamMenuWindow::msgListDropRegister()
{
    static XtCallbackRec transferCBRec[] = { 
	{&RoamMenuWindow::msgListTransferCallback, NULL}, {NULL, NULL} };

    // Pass the RoamMenuWindow object (this) as clientData.
    transferCBRec[0].closure = this;

    DtDndVaDropRegister(_list->get_scrolling_list(),
	    DtDND_FILENAME_TRANSFER | DtDND_BUFFER_TRANSFER,
	    (unsigned char)(XmDROP_MOVE | XmDROP_COPY), transferCBRec,
	    DtNtextIsBuffer, Sdm_True,
	    NULL);
}

// msgListDropEnable
//
// Enable the message list for drops by restoring the operation
//
void
RoamMenuWindow::msgListDropEnable()
{
    Arg	args[1];

    XtSetArg(args[0], XmNdropSiteOperations, XmDROP_MOVE | XmDROP_COPY);
    XmDropSiteUpdate(_list->get_scrolling_list(), args, 1);
}

// msgListDropDisable
//
// Disable the message list for drops by setting the operation to noop
//
void
RoamMenuWindow::msgListDropDisable()
{
    Arg	args[1];

    XtSetArg(args[0], XmNdropSiteOperations, XmDROP_NOOP);
    XmDropSiteUpdate(_list->get_scrolling_list(), args, 1);
}


// msgListConvertCallback
//
// Provide the msg list as a mailbox for the drag
//
void
RoamMenuWindow::msgListConvertCallback(
    Widget	/* dragContext */,
    XtPointer	clientData,
    XtPointer	callData)
{
    DtDndConvertCallbackStruct *convertInfo =
			    (DtDndConvertCallbackStruct *) callData;
    DtDndBuffer		*buffer = &(convertInfo->dragData->data.buffers[0]);
    RoamMenuWindow 	*rmw = (RoamMenuWindow *)clientData;
    MsgScrollingList	*msgList;
    Widget		listW;
    SdmMessageStore	*mbox;
    SdmMessage		*msg;
    SdmMessageNumber	msgHandle;
    int			*pos_list, pos_count, ii;
    unsigned long	bufSize = 0;
    char 		*bufPtr;
    SdmError		error;

    error.Reset();

    switch (convertInfo->reason) {
	case DtCR_DND_CONVERT_DATA:
	{

	    msgList = rmw->list();
	    listW = msgList->get_scrolling_list();
	    mbox = rmw->mailbox();

	    if (!XmListGetSelectedPos(listW, &pos_list, &pos_count)) {
			convertInfo->status = DtDND_FAILURE;
			return;
	    }

	    //mbufList = (SdmString *)XtMalloc(pos_count*sizeof(SdmString));
	    // Create a list of strings with pos_count elements.
	    SdmStringL 	mbufList(pos_count); 
	    SdmContentBuffer content_buf;

	    for (ii = 0; ii < pos_count; ii++) {
		msgHandle = msgList->msgno(pos_list[ii]);
		mbox->SdmMessageFactory(error, msg, msgHandle);
		if ( error ) {
		    convertInfo->status = DtDND_FAILURE;
		    return;
		}

		msg->GetContents(error, content_buf);
		if ( error ) {
		    convertInfo->status = DtDND_FAILURE;
		    return;
		}

		content_buf.GetContents(error, mbufList[ii]);
		if ( error ) {
		    convertInfo->status = DtDND_FAILURE;
		    return;
		}
		bufSize += mbufList[ii].Length() + 1;
		
		rmw->delete_message(msgHandle);
	    }

	    //bufPtr = new char[bufSize];
	    bufPtr = (char *)malloc(bufSize);

	    buffer->bp = bufPtr;
	    buffer->size = (int)bufSize;
	    buffer->name = "Mailbox";

	    for (ii = 0; ii < pos_count; ii++) {
		strncpy(bufPtr, (const char *)mbufList[ii], 
				(unsigned int)mbufList[ii].Length());
		bufPtr += mbufList[ii].Length();
		*bufPtr = '\n';
		bufPtr++;
	    }

	    XtFree((char *)pos_list);
	    break;

	}
	case DtCR_DND_CONVERT_DELETE:
	    // The drag and drop succeeded, so we can now delete the messages
	    // from the scrolling list.
	    rmw->list()->deleteSelected();
	    break;
    }
}

// msgListDragFinishCallback
//
// Clean up from the convert callback and restore state
//
void
RoamMenuWindow::msgListDragFinishCallback(
	Widget 		/* widget */,
	XtPointer 	clientData,
	XtPointer 	callData)
{
    DtDndDragFinishCallbackStruct *finishInfo =
		(DtDndDragFinishCallbackStruct *) callData;
    DtDndContext 	*dragData = finishInfo->dragData;
    RoamMenuWindow 	*rmw = (RoamMenuWindow *) clientData;
    MsgScrollingList	*msgList = rmw->list();
    SdmError		mail_error;
    int			ii;

    mail_error.Reset();

    // Re-enable drops on this message list
	if (rmw->mailBoxWritable() == Sdm_True) {
		rmw->msgListDropEnable();
	}

	if (finishInfo->sourceIcon)
		XtDestroyWidget(finishInfo->sourceIcon);

	// Free any memory allocated for the drag.
	for (ii = 0; ii < dragData->numItems; ii++) {
		XtFree((char *)dragData->data.buffers[ii].bp);
    }
}

// getDragIcon
//
// Use the mailbox pixmap to create a drag icon
//
Widget
RoamMenuWindow::getDragIcon(
	Widget 		widget)
{
	if (_mbox_image && _mbox_mask) {
		return DtDndCreateSourceIcon(widget, _mbox_image, _mbox_mask);
	} else {
		return NULL;
	}
}

// msgListDragStart
//
// Translation start a drag from the msg list
//
XtActionProc
RoamMenuWindow::msgListDragStart(
	Widget		widget,
	XEvent		*event,
	String		* /* params */,
	Cardinal	* /* numParams */)
{
    static XtCallbackRec convertCBRec[] = { 
	{&RoamMenuWindow::msgListConvertCallback, NULL},{NULL, NULL} };
    static XtCallbackRec dragFinishCBRec[] = {
	{&RoamMenuWindow::msgListDragFinishCallback, NULL}, {NULL, NULL} };
    RoamMenuWindow 	*rmw = NULL;
    unsigned char	operations;
    Widget 		drag_icon;
    SdmError 		mail_error;

    mail_error.Reset();

    XtVaGetValues(widget, XmNuserData, &rmw, NULL);

    if (rmw == NULL)
	return NULL;

    drag_icon = rmw->getDragIcon(widget);
    //drag_icon = NULL;

    rmw->msgListDropDisable();

    // Choose the drag operations based on the writeability of the mailbox

#ifdef POP
!!!!!!!!!!!!!!! Should be fixed
#endif
    if (rmw->mailBoxWritable() == Sdm_True) {
        // RW Folder.  Permit Copy and Move.
	operations = (unsigned char)(XmDROP_COPY | XmDROP_MOVE);
    } else {
        // RO Folder.  Permit only Copy drags.
	operations = (unsigned char)XmDROP_COPY;
    }

    // Start the drag

    convertCBRec[0].closure = (XtPointer) rmw;
    dragFinishCBRec[0].closure = (XtPointer) rmw;

    if (DtDndVaDragStart(widget, event, DtDND_BUFFER_TRANSFER, 1,
			operations, convertCBRec, dragFinishCBRec,
			DtNsourceIcon, drag_icon,
			NULL) == NULL) {
    }

    return NULL;
}

// msgListDragSetup
//
// Override default list translations to start our own drags
//
void
RoamMenuWindow::msgListDragSetup()
{
    Widget msgList = _list->get_scrolling_list();
    static char	translations[] = "\
		~c ~s ~m ~a <Btn1Down>:\
		    process-press(ListBeginSelect,MsgListDragStart)\n\
		c ~s ~m ~a <Btn1Down>:\
		    process-press(ListBeginToggle,MsgListDragStart)";
    static char	btn2_translations[] = "\
		~c ~s ~m ~a <Btn2Down>:\
		    process-press(ListBeginSelect,MsgListDragStart)\n\
		c ~s ~m ~a <Btn2Down>:\
		    process-press(ListBeginToggle,MsgListDragStart)\n\
		<Btn2Motion>:ListButtonMotion()\n\
		~c ~s ~m ~a <Btn2Up>:ListEndSelect()\n\
		c ~s ~m ~a <Btn2Up>:ListEndToggle()";
    int		btn1_transfer = 0;
    XtTranslations	new_translations;
    static XtActionsRec	actionTable[] = {
	{"MsgListDragStart", 
	(XtActionProc) &RoamMenuWindow::msgListDragStart}
    };

    XtAppAddActions(theRoamApp.appContext(), actionTable, 1);
    new_translations = XtParseTranslationTable(translations);
    XtOverrideTranslations(msgList, new_translations);

    XtVaGetValues(
	(Widget)XmGetXmDisplay(XtDisplayOfObject(msgList)),
	"enableBtn1Transfer", &btn1_transfer,
	NULL);
    
    if (btn1_transfer != Sdm_True) {
	new_translations = XtParseTranslationTable(btn2_translations);
	XtOverrideTranslations(msgList, new_translations);
    }
}

void
RoamMenuWindow::open_and_load(SdmError &error)
{
    _is_opening_and_loading = Sdm_True;
    _openContainerCmd->execute();

    // if it required conversion, let conversion handle the error (its a
    // callback routine and we have "lost control" of what it returns anyway.)
    // 
    // if it did not require conversion, then open() should have succeeded.
    // If it did succeed, then its _mailbox should be set by now.  If its not
    // set, then either open() failed somewhere or it was cancelled by the
    // user.  We now have an error condition.
    if ((!_requiredConversion) && (_mailbox == NULL)) {
      error = (SdmErrorCode)Sdm_EC_Fail;
      
    } else {
      // If it required conversion, then let the conversion process handle
      // errors if any occur.  Reset the SdmError for this method.
      if (_requiredConversion) {
        error.Reset();
      }

      // If it required conversion, and conversion succeeded, then the mailbox
      // was also loaded and _is_loaded gets set to Sdm_True.  If it required no
      // conversion, then it implies that we already have a container in hand
      // and we just load it (implicitly setting _is_loaded to Sdm_True)
      if (!_requiredConversion && !_is_loaded && !_async) {
        this->load_mailbox(error);
        if (!error) {
          _is_loaded = Sdm_True;
        }
      }
    }
    _is_opening_and_loading = Sdm_False;
}

void
RoamMenuWindow::okcb(void *)
{
  printf("Logic Error: In RoamMenuWindow::okcb\n");
  return;
}

// If the user hits the Cancel button in the Login Dialog, we want to
// bring up a file selection box.  The file selection box should let
// them open the mailbox of their choice.
void
RoamMenuWindow::cancelcb(struct OpenStruct *open_struct)
{
  XtUnmanageChild(thePasswordDialogManager->baseWidget());
  RoamMenuWindow *rmw = open_struct->rmw;
  // Dont open this if opening from restoreSession. 
  // The assumption is the inbox is already setup.
  // This makes for a cleaner UI on restore session startup
  if (theRoamApp.firstLogin() && theRoamApp.sessionFile() == NULL) {
	// The FSB is displayed and its the first login. This mean that the 
	// user is canceling out of login dialog. So just return.
	if (open_struct != NULL && open_struct->command != NULL) {
		CustomSelectFileCmd* csfc = 
			(CustomSelectFileCmd*)open_struct->command;
		if (strcmp(csfc->className(), "CustomSelectFileCmd") == 0 &&
			csfc->fileBrowser() &&
			XtIsManaged(csfc->fileBrowser())) {
			if (csfc->cbs())
				free(csfc->setLocalContext(csfc->cbs()));
			return;
		}
	}
	rmw->setGoAway(Sdm_True);
	CustomSelectFileCmd *fsb = new CustomSelectFileCmd(
		"Select Mailbox",
		catgets(DT_catd, 1, 65, "Select Mailbox..."),
		catgets(DT_catd, 1, 26, "Mailer - Open Mailbox"),
		catgets(DT_catd, 1, 340, "Open Mailbox"),
		Sdm_True,
		RoamMenuWindow::file_selection_callback,
		CustomSelectFileCmd::doDirSearch,
		open_struct, rmw->baseWidget());
	open_struct->command = fsb;
	fsb->setLocal(TRUE);
	fsb->doit();
  } 
  else 
	delete rmw;
}
void
RoamMenuWindow::resetToken()
{
      	_token->ClearValue("serviceoption", "username");
      	_token->SetValue("serviceoption", "username", 
			thePasswordDialogManager->user()); 
      	_token->ClearValue("serviceoption", "password");
      	_token->SetValue("serviceoption", "password", 
			thePasswordDialogManager->password());
}

// Ideally, open() should set the error if the user cancels the open.  
// And we should error after open() returns at the caller's end...
// We need to treat the inbox and other mailboxes differently, especially
// in the case of IMAP, since we have an imapinboxserver and an imapfolderserver.
void
RoamMenuWindow::open(SdmError &error)
{
  char *value=NULL;
  SdmError localError;
  SdmString server=NULL;

  // Check to see if this is an IMAP or a local mailbox.  If it's the inbox
  // and "imapinboxserver" is set, or it it's another mailbox and
  // "imapfolderserver" is set, then it's IMAP.  Otherwise, it's local.  If
  // there is no local inbox and the imapinboxserver variable is not set,
  // and we are trying to open the inbox for the first time, we should bring
  // up a dialog asking the user if whether they want local or imap.
  SdmMailRc * mailRc;
  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
  assert(!error);

  if (inbox()) {
    mailRc->GetValue(error, "imapinboxserver", &value);

    // If imapinboxserver isn't set, check for a local mailbox.
    if (error) {
      struct stat info;
      error.Reset();

      if (SdmSystemUtility::SafeStat(_mailbox_fullpath, &info) != 0) {
        // "imapinboxserver" isn't set and there is no local inbox, so
        // we have to ask the user what he/she prefers: IMAP or local.
        char *helpId = NULL;
        int answer = 0;

        if (theRoamApp.FirstInboxOpen()) { // Is it the first time we're opening the inbox?
          char promptstr[250];
          theRoamApp.FirstInboxOpen(Sdm_False);
          sprintf(promptstr, catgets(DT_catd, 3, 124, 
                                     "Your email setup does not indicate whether you have a\n\
local or IMAP mailbox.  Specify your choice.  If you do\n\
not know the answer, click Local."));
          _genDialog->setToQuestionDialog(catgets(DT_catd, 3, 125, "Mailer - Specify Startup Mailbox"), promptstr);
          helpId = DTMAILHELPSTARTUPMAILBOX;
          answer = _genDialog->post_and_return(catgets(DT_catd, 3, 125, "Local"),
                                               catgets(DT_catd, 1, 5, "Cancel"),
                                               catgets(DT_catd, 3, 126, "IMAP"), helpId);
        } else {  // if we opened the inbox before, we'll assume local.
          answer = 1;
        }
        if (answer == 1) { // Local
          // If we get here it means that no inbox exists, so we
          // have to create it.
          MailSession *session = theRoamApp.isession();
          SdmMessageStore *mbox;

          if (!_token) {
            _token = new SdmToken;
          }
          _token->SetValue("serviceclass", "messagestore");
          _token->SetValue("servicetype", "local");
          session->isession()->SdmMessageStoreFactory(error, mbox);
          if (error) {
            return;
          }
          if (mbox->StartUp(error) != Sdm_EC_Success) {
            delete mbox;
            return;
          }
          if (mbox->Attach(error, *_token) != Sdm_EC_Success) {
            delete mbox;
            return;
          }
          if (mbox->Create(error, _mailbox_fullpath) != Sdm_EC_Success) {
            delete mbox;
            return;
          }
          if (mbox->Close(error) != Sdm_EC_Success) {
            delete mbox;
            return;
          }
          setLocal(Sdm_True);

        } else if (answer == 3) { // IMAP
          setLocal(Sdm_False);
        } else {
          return;
        }
      } else {
        // There is a local inbox, so we set local to true.
        theRoamApp.FirstInboxOpen(Sdm_False);
        setLocal(Sdm_True);
      }
    } else {
      // "imapinboxserver is set, so we know it's not local.
      theRoamApp.FirstInboxOpen(Sdm_False);
      setLocal(Sdm_False);
    }
  } else { // It's not the inbox.
    if (_token) {
      if (_token->CheckValue("servicetype", "imap")) {
        setLocal(Sdm_False);
      } else {
        setLocal(Sdm_True);
      }
    } else {
      mailRc->GetValue(error, "imapfolderserver", &value);
      if (error) { // Must be local
        error.Reset();
        setLocal(Sdm_True);
      } else {
        setLocal(Sdm_False);
      }
    }
  }

  if (!_token) 
    _token = new SdmToken;


  _token->SetValue("servicechannel", "cclient");
  _token->SetValue("serviceclass", "messagestore");
  _token->SetValue("serviceoption", "messagestorename", _mailbox_fullpath);
  if (mailRc->IsValueDefined("dontsavemimemessageformat"))
    	_token->SetValue("serviceoption", "preservev3messages", "x");
  if (mailRc->IsValueDefined("logdebug"))
    	_token->SetValue("serviceoption", "debug", "x");

  if (isLocal()) {
    	_token->SetValue("servicetype", "local");
    	// if error occurs here, just return it.
    	open_internal(error);
    	if (error == Sdm_EC_Success) 
		theRoamApp.firstLogin(Sdm_False);
  } else {
    	_token->SetValue("servicetype", "imap");
    	// A pre-specified hostname in the token overrides any derived setting.
    	if (_token->FetchValue(server, "serviceoption", "hostname")) {
      		// Reset the title to include the IMAP server name
      		char *path = getTitle();
      		char *new_title = new char [strlen(server) + strlen(path) + 2];
      		sprintf(new_title, "%s:%s", (const char*)server, path);
      		title(new_title);
      		delete [] new_title;
      		// This may not be null if rmw was started by session manager
      		if (_server == NULL)
			_server = strdup((const char*)server);
     	}

    	// first check to see if server, password, and user information is 
	// already available.  if it is, let's try to open the mailbox with 
	// this information.
    	if (thePasswordDialogManager->user() != NULL && 
         	thePasswordDialogManager->password() != NULL) {
		if (server && *server)
			thePasswordDialogManager->setImapServer(strdup(server));
		else if (value)
			thePasswordDialogManager->setImapServer(strdup(value));
 		resetToken();
		open_internal(localError);
		if (localError == Sdm_EC_Success) {
			theRoamApp.firstLogin(Sdm_False);
			return;
		}
	
    	}
    
    	// we were unable to open the mailbox.  post the password dialog.
    	struct OpenStruct *open_struct;
    	open_struct = (struct OpenStruct *) malloc(sizeof(OpenStruct));
    	open_struct->rmw = this;
    	open_struct->command = NULL;
  
    	// If the login dialog is already managed, wait until it becomes 
    	// unmanaged before we bring it up again: we dont want multiple 
    	// instances of this dialog on the desktop, because there is only
    	// PasswordDialogManager object and things get overwritten and very
    	// convoluted if we allow the creation of more than one dialog. In the 
    	// case of a restoreSession() we could have multiple imap mailboxes 
    	// opening concurrently. So we wait here until the login dialog is 
    	// brought down then we post it again for the next imap mailbox, etc.
    	// Also we try to open the mailbox with the last login data supplied, 
    	// so we dont redundantly display the login dialog again unnecessarily.
    	if (thePasswordDialogManager->baseWidget())
		if (XtIsManaged(thePasswordDialogManager->baseWidget())) {
			while (XtIsManaged( thePasswordDialogManager->baseWidget()))
				XtAppProcessEvent(XtWidgetToApplicationContext(
				thePasswordDialogManager->baseWidget()), XtIMXEvent);
			if (server && *server)
				thePasswordDialogManager->setImapServer(strdup(server));
			resetToken();
			open_internal(localError);
			if (localError == Sdm_EC_Success) { 
				theRoamApp.firstLogin(Sdm_False);
				return;
			}
     		}
			
    	// Bring up the password dialog - the PasswordDialogManager::okcb will
    	// set any additional token fields as required.
	if (server && *server)
		thePasswordDialogManager->setImapServer(strdup(server));
	else if (value)
		thePasswordDialogManager->setImapServer(strdup(value));
    	thePasswordDialogManager->post(catgets(DT_catd, 16, 1, "Mailer - Login"), 
		catgets(DT_catd, 16, 1, "Mailer - Login"),
		theRoamApp.baseWidget(), (void *) open_struct,
		(DialogCallback) &RoamMenuWindow::okcb,
		(DialogCallback) &PasswordCmd::apply,
		(DialogCallback) &RoamMenuWindow::cancelcb, 
		(DialogCallback) &PasswordCmd::help);
    	_requiredConversion = Sdm_True;
    }
    return;
}

#ifdef OPEN_ASYNC
void
RoamMenuWindow::finishOpen(SdmMessageStore *mbox)
{
  SdmError error;

  _mailbox = mbox;

  if (mailBoxWritable() == Sdm_True) {
	msgListDropRegister();
  }

  load_mailbox(error);
  if (error) {
    postErrorDialog(error);
  }
}

#endif

void
RoamMenuWindow::open_internal(SdmError &error)
{
  MailSession *ses = theRoamApp.isession();
  char buf[512];
  int answer = 0;
  char *helpId;

  // GL - 4-8-97 We now check to see if the mbox is already open in the
  // RoamMenuWindow::mail_file_selection method.  It is done there to avoid
  // creating a RMW object and all the widgets associated with it.
  // 
  // Check to see if the mbox is already open.  If so, simply map-raise the
  // original RoamMenuWindow that opened it.
//   SdmString hostname;
//   _token->FetchValue(hostname, "serviceoption", "hostname");
//   already_open = ses->isMboxOpen(_mailbox_fullpath, hostname);

  if (!isLocal())
    get_editor()->attachArea()->bottomStatusMessage
      (catgets(DT_catd, 16, 6, "Connecting to the IMAP server..."));

  // Force the display of the status message
  XmUpdateDisplay(baseWidget());

  // Try to open this folder, but don't take the lock and don't create it.
  SdmBoolean readOnly = Sdm_False;
  _mailbox = ses->open(error, _initial_nmsgs, readOnly, *_token, this);

  setWritable(readOnly ? Sdm_False : Sdm_True);

  if (error) {
    if ((SdmErrorCode)error == Sdm_EC_CannotAccessMessageStore) {
      if (error.IsErrorContained(Sdm_EC_NullsDetectedInMail)) {
        sprintf(buf, catgets(DT_catd, 2, 27,
                             "Mailer cannot open this mailbox (%s)\nbecause it contains null characters."),
                _mailbox_fullpath);

        _genDialog->setToQuestionDialog(catgets(DT_catd, 1, 6, "Mailer"), buf);
            
        helpId = DTMAILHELPOPENFAILSFROMNULLS;
        answer = _genDialog->post_and_return(catgets(DT_catd, 3, 9, "OK"), helpId);
        return;
      }
    }
    // This is a real error. Punt!
    this->postErrorDialog(error);
  } else { // No error.
    char *new_title = NULL;
    
    // check to see if the server value changed.  (it could have been
    // changed in the password dialog).  If it the server changed,
    // update the title and save the new server name.
    if (!isLocal()) {
      SdmString server;
      if (_token->FetchValue(server, "serviceoption", "hostname") &&        
          (_server == NULL || server != _server)) 
      {
        new_title = new char [strlen(server) + strlen(_mailbox_fullpath) + 2];
        sprintf(new_title, "%s:%s", (const char*)server, _mailbox_fullpath);
        // This may not be null if the rmw was started by the session manager
        if (_server != NULL) {
          free (_server);
        }
        _server = strdup(server);
      }
    }

    // if the title has been changed 
    // because the server name changed, we update it here.
    if (new_title) {
        title(new_title);
        delete [] new_title;
    }
      
    if (mailBoxWritable() == Sdm_True) {
      msgListDropRegister();
    } else {
      SdmMailRc * mailrc;
      SdmError localError;
      theRoamApp.connection()->connection()->SdmMailRcFactory(localError, mailrc);
      if (localError == Sdm_EC_Success && mailrc->IsValueDefined("expert")) {
          setupReadOnlyMailbox(Sdm_False);
      } else  {
          setupReadOnlyMailbox(Sdm_True);
      }
    }
    
    get_editor()->attachArea()->bottomStatusMessage(" ");
    

    load_mailbox(error);
    if (error) {
      // if error occurs, just return it.
      this->postErrorDialog(error);
      return;
    }
  }
  return;
}



void RoamMenuWindow::setupReadOnlyMailbox(SdmBoolean postWarning)
{
  char *new_title = NULL;
  char *helpId;

  // We need to disable the editable menu options if the mail
  // box is readonly.
  //
  _delete_msg->deactivate();
  _undeleteLast->deactivate();
  _undeleteFromList->deactivate();
  _destroyDeletedMessages->deactivate();
  _check_new_mail->deactivate();

  // WARNING!!  Do not be tempted to set the sensitivity of a widget here!!!
  // Since we now use work procedures (XtAppAddWorkProc) to create all of the menu
  // items and menu panes, the widgets (like _moveMenu or _msgsPopupMoveMenu)
  // will not exist at this point in the process.  The work procs will run after the gui
  // is up and running and the Xt event loop has no more events to process.
  // The correct way to do this is by using the deactivate
  // method for the Cmd or CmdList object you are dealing with.
  _move_popup_menu_cmdList->deactivate();
  _move_menu_cmdList->deactivate();

  if (_server) {
    new_title = new char[strlen(_server) + strlen(_mailbox_fullpath) + 30];
    sprintf(new_title, "%s:%s [Read Only]", _server, _mailbox_fullpath);
  } else {
    new_title = new char[strlen(_mailbox_fullpath) + 30];
    sprintf(new_title, "%s [Read Only]", _mailbox_fullpath);
  }

  // set to the new title in the window.
  title(new_title);
  delete [] new_title;
    
  if (postWarning) {
    // post a warning message about read-only mailbox
    char* str = catgets(DT_catd, 3, 146, "Mailer has opened a read-only mailbox in which you can read\nmessages.  You cannot change this mailbox or check for new\nmail from it.");
    helpId = DTMAILHELPREADONLYMAILBOX;
    _genDialog->setToWarningDialog(catgets(DT_catd, 3, 54, "Mailer"), str);
    _genDialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), helpId);
  }
}

void RoamMenuWindow::createMenuPanes()
{
  construct_file_menu();
  construct_message_menu();
  construct_compose_menu();
  construct_view_menu();
  construct_options_menu();
  construct_move_menu();
  construct_help_menu();
  construct_message_popup();
  construct_text_popup();
  construct_attachment_popup();
  create_toolbar_cmdlist();

  // We are done with startup menu creation, so we can go back to normal
  // without using XtAppAddWorkProc which seems to mess up the recently visisted menu
  _isstartup = Sdm_False;
}

void
RoamMenuWindow::create_toolbar_cmdlist()
{
  // Whenever you add a command to the list of possible toolbar commands
  // you MUST add the corresponding icon file name to the _toolbar_icons
  // array.  Also, all commands that have an insensitive icon associated
  // with them should be added before all other commands.  This is because
  // the normal and insesitive pixmaps for a given command must occupy the
  // same position in their respective arrays.
  _toolbar_cmds = new CmdList();
  _toolbar_icons = new DtVirtArray<char *>(24);
  _toolbar_inactive_icons = new DtVirtArray<char *>(5);

  _toolbar_cmds->add( _undeleteLast );
  _toolbar_icons->append("DtMudl.s.pm");
  _toolbar_inactive_icons->append("DtMudlI.s.pm");

  _toolbar_cmds->add( _undeleteFromList );
  _toolbar_icons->append("DtMudfl.s.pm");
  _toolbar_inactive_icons->append("DtMuflI.s.pm");

  _toolbar_cmds->add( _delete_msg );
  _toolbar_icons->append("DtMd.s.pm");
  _toolbar_inactive_icons->append("DtMdI.s.pm");

  _toolbar_cmds->add( _destroyDeletedMessages );
  _toolbar_icons->append("DtMddm.s.pm");
  _toolbar_inactive_icons->append("DtMddmI.s.pm");

  _toolbar_cmds->add( _check_new_mail );
  _toolbar_icons->append("DtMcfnm.s.pm");
  _toolbar_inactive_icons->append("DtMcnmI.s.pm");

#ifdef AUTOFILING
  _toolbar_cmds->add( theRoamApp.autoFiling() );
  _toolbar_icons->append("DtMsmnm.s.pm");
#endif // AUTOFILING

  _toolbar_cmds->add( _quit );
  _toolbar_icons->append("DtMc.s.pm");

  // No toolbar glyph for this command at this time.
  //     _toolbar_cmds->add( _print_msg );
  //     _toolbar_icons->append("DtMpri.s.pm");

  _toolbar_cmds->add( _print );
  _toolbar_icons->append("DtMpo.s.pm");

  _toolbar_cmds->add( _find );
  _toolbar_icons->append("DtMfi.s.pm");

  _toolbar_cmds->add( _open );
  _toolbar_icons->append("DtMsm.s.pm");

  _toolbar_cmds->add( _next );
  _toolbar_icons->append("DtMn.s.pm");

  _toolbar_cmds->add( _previous );
  _toolbar_icons->append("DtMpre.s.pm");

  _toolbar_cmds->add( _opt_al );
  _toolbar_icons->append("DtMal.s.pm");

  _toolbar_cmds->add( _comp_new );
  _toolbar_icons->append("DtMnm.s.pm");

  _toolbar_cmds->add( _forward );
  _toolbar_icons->append("DtMfo.s.pm");

  // No toolbar glyph for this command at this time.
  //     _toolbar_cmds->add( _forwardBody );
  //     _toolbar_icons->append("DtMfna.s.pm");

  _toolbar_cmds->add( _replySender );
  _toolbar_icons->append("DtMr.s.pm");

  _toolbar_cmds->add( _replyAll );
  _toolbar_icons->append("DtMra.s.pm");

  _toolbar_cmds->add( _replySinclude );
  _toolbar_icons->append("DtMri.s.pm");

  _toolbar_cmds->add( _replyAinclude );
  _toolbar_icons->append("DtMrai.s.pm");
}

void
RoamMenuWindow::resetCacheList(int new_size)
{
    // Remove the extra items in the list
    for (int i = _cached_list_size; i > new_size; i--) {
      _cached_move_containerlist->remove(i-1);
      _cached_copy_containerlist->remove(i-1);
      _cached_open_containerlist->remove(i-1);
      _menuBar->removeCommand(_moveMenu, _first_cached_move_item+i-1);
      _menuBar->removeCommand(_copyMenu, _first_cached_copy_item+i-1);
      _menuBar->removeCommand(_openMenu, _first_cached_open_item+i-1);
      _menuBar->removeCommand(_msgsPopupMoveMenu, _first_cached_copy_item+i-1);
    }
    _cached_list_size = new_size;
    
}

void
RoamMenuWindow::propsChanged(void)
{
    SdmMailRc *mailrc = get_mail_rc();
    char *value;
    SdmError error;
    int header_lines;

    // See if the header size has changed.
    //
    mailrc->GetValue(error, "headerlines", &value);
    if (error || value == NULL || *value == NULL) { 
	if (value) free(value);
    	header_lines = 15;
	error.Reset();
    }
    else {
     	header_lines = (int) strtol(value, NULL, 10);
    	free(value);
    }
    if (header_lines != _list->visibleItems())
	_list->visibleItems(header_lines);

    _list->checkDisplayProp();

    mailrc->GetValue(error, "dontdisplaycachedfiles", &value);
    if (error && value != NULL) {
        // dontdisplaycachedfiles is set, so dont display cached files
        if (_cached_list_size != 0)
		// They just turned off the Display Up To prop so
	  	//  reset the cache list 
		resetCacheList(0);
		_max_cached_list_size = 0;
    	free(value);
    }
    else {
        error.Reset();
        mailrc->GetValue(error, "cachedfilemenusize", &value);
        if (!error && value != NULL && *value != '\0') {
		int new_size;
		if ((new_size = (int)strtol(value, NULL, 10))
			!= _max_cached_list_size && new_size >=0) {
			// They just changed the display number so chop the 
			// list if it is bigger than the new size just set 
			if (new_size < _cached_list_size)
				resetCacheList(new_size);
			_max_cached_list_size = new_size;
		}
	}
    	if (value) free(value);
    }
    error.Reset(); 
    mailrc->GetValue(error, "movemenu", &value);
    if  ( (value == NULL && _filemenu2 != NULL) ||
	  (value != NULL && 
	  (_filemenu2 == NULL || strcmp(value, _filemenu2)) != 0)) {
    		// The resource changed...
		_cached_list_size = _user_list_size = 0;
		// Save this list as it gets lost in recreating the Move menu
		DtVirtArray<MoveCopyMenuCmd *> *save_cached_list = 
			_cached_copy_containerlist;

		// This is a really inefficient way to do this. Change before
		// FCS to just recreate the pullrights and reattach to the
		// existing menus vs. recreating the entire menu.

		// Recreate the Message menu
		this->construct_message_menu();	

		// Recreate the Move menu...
		this->construct_move_menu();	

		// Recreate the File menu
		this->construct_file_menu();	

		// Recreate the Message popup menu
		this->construct_message_popup();	

		// Add back the cached lists.
		for (int i = save_cached_list->length(); i > 0; i--) {
			char *name = (*save_cached_list)[i-1]->containerName();
			char *newname;
			SdmUtility::GetRelativePath(error, newname, 
                                name, *mailrc, SdmUtility::kFolderResourceName);
			addToRecentList(newname);
			free(newname);
		}
		delete save_cached_list;
    }
    error.Reset(); 
    if (value) free(value);
    mailrc->GetValue(error, "toolbarcommands", &value);
    //
    // If error.isSet() there is no need to rebuild the toolbar because
    // the user has not modified the toolbar commands.  We know this
    // because even if you
    // delete all the commands from the toolbar, it still writes out
    // an empty toolbarcommand string (like "set toolbarcommands").
    //
    if ( !error ) {
	if  ((value == NULL && _toolbar_mailrc_string != NULL) ||
	     (value != NULL && (_toolbar_mailrc_string == NULL ||
				strcmp(value, _toolbar_mailrc_string)))) {
	    buildToolbar();
	} else {
	    error.Reset();
	    if (value) free(value);
	    mailrc->GetValue(error, "toolbarusetext", &value);
	    if((( !error && (strcmp(value, "")==0))
		&& _toolbar_use_icons)) {
		buildToolbar();
	    } else if ((error || strcmp(value, "f") == 0 || !value) &&
		       !_toolbar_use_icons) {
		buildToolbar();
	    }
	    if (value) free(value);
	}
    } else {
	error.Reset();
	mailrc->GetValue(error, "toolbarusetext", &value);
	if(((!error && (strcmp(value, "")==0))
	    && _toolbar_use_icons)) {
	    buildToolbar();
	} else if ((error || strcmp(value, "f") == 0 || !value) &&
		   !_toolbar_use_icons) {
	    buildToolbar();
	}
    	if (value) free(value);
    }
    // Update the open log file cmd if user changes this in props.
    MailRcSource *mrc = (MailRcSource*)new MailRcSource (mailrc, "record");
    value = (char*)mrc->getValue();
    if (strcmp(_logfilename, value) != 0) {
	_logfile->setMailboxName(value);
	free(_logfilename);
	_logfilename = value;
    }
    else free(value);
}

void RoamMenuWindow::registerDialog( ViewMsgDialog *dialog )
{
    int i;
    ViewMsgDialog **newList = (ViewMsgDialog **)-1;
  
    // Allocate a new list large enough to hold the new
    // object, and copy the contents of the current list 
    // to the new list
  
    newList = new ViewMsgDialog*[_numDialogs + 1];
  
    for ( i = 0; i < _numDialogs; i++ )
	newList[i] = _dialogs[i];
  
    // Install the new list and add the window to the list
  
    if ( _numDialogs > 0 )
	delete []_dialogs;
    _dialogs =  newList;
    _dialogs[_numDialogs] = dialog;
    _numDialogs++;

}
  
void RoamMenuWindow::unregisterDialog ( ViewMsgDialog *dialog )
{
  int i, index;
  ViewMsgDialog **newList = (ViewMsgDialog **)-1;
  
  // Allocate a new, smaller list
  
  newList = new ViewMsgDialog *[_numDialogs - 1];
  
  // Copy all objects, except the one to be 
  // removed, to the new list
  
  index = 0;
  for ( i = 0; i < _numDialogs; i++ )
    if ( _dialogs[i] != dialog )
    newList[index++] = _dialogs[i];
  
  // Install the new list
  
  delete []_dialogs;
  _dialogs =  newList;
  
  _numDialogs--;

  // we must delete the message AFTER the dialog has been removed
  // from the list.
  if (!_expunging) {
    delete_message(dialog->msgno());
  }
}

void
RoamMenuWindow::title(const char *title )
{
  if ( title==NULL )
      title=_mailbox_name;
  MainWindow::title( title );
}

// If it is a configure notify, we are interested in it.
// We need to then capture its new position.
// And if the RMW has not been loaded, we need to load it.

void
RoamMenuWindow::structurenotify(
    Widget,
    XtPointer clientData,
    XEvent *event,
    char *)		// continue_to_dispatch
{

   if (event->type == ConfigureNotify) {

       RoamMenuWindow *window=(RoamMenuWindow *) clientData;

       // GL - The following if statement is a HACK dreamt up by my office
       // mate.  This seems to be the only way to get the toolbar to
       // resize itself correctly.  Note, setting the height of the
       // rowOfButtons widget to 10 is just a way to get the rowcolumn
       // widget to actually refigure its height.  The value 10 has no
       // significance.
       if (event->xconfigurerequest.width != window->_width) {
	   XtVaSetValues(window->rowOfButtons, XmNheight, 10, NULL);
       }

       window->configurenotify( event->xconfigurerequest.x,
				event->xconfigurerequest.y,
				event->xconfigurerequest.width,
				event->xconfigurerequest.height,
				event->xconfigurerequest.border_width );
   }
   else if ((event->type == MapNotify) || ( event->type == UnmapNotify)) {

       RoamMenuWindow *window=(RoamMenuWindow *) clientData;

       // If rmw was started iconic, we wait to map and build toolbar here.
       if (window->startedIconic()) {
		window->manage();
		window->buildToolbar(); 
		window->setIconic(0); 
       }
       window->mapnotify();
   }       
}

// Capture its position coordinates.

void 
RoamMenuWindow::configurenotify( 
    unsigned int win_x, unsigned int win_y,
    unsigned int win_wid, unsigned int win_ht,
    unsigned int win_bwid
)
{
    _x = win_x;
    _y = win_y;
    _width = win_wid;
    _height = win_ht;
    _border_width = win_bwid;
}

// If it is not already loaded, then load it (it might involve conversion,
// etc.; all handled by open_and_load()) If its been loaded already, then
// mapnotify gets called when the state changes from iconic to open (i.e.,
// the user double-clicks on an RMW icon).  If we want to load a folder at
// that time, this is the place to do it.

void
RoamMenuWindow::mapnotify()
{
    // If its not been loaded, then open and load it.
    
    if (!_is_loaded && !_passwd_dialog_up  && !_is_opening_and_loading) {
	
	SdmError mail_error;
	
	// Initialize the mail_error.
	mail_error.Reset();
	
	busyCursor();
	// We call XmUpdateDisplay so the cursor changes to the
	// hourglass right away.
	XmUpdateDisplay(baseWidget());

	this->open_and_load(mail_error);
	
	normalCursor();
	
	// If there's been an error then we quit the container.
 	if (mail_error) {
 	    
	    // Need to remove the base Widgets destroy callback since
	    // we end up destroying it twice otherwise...
 		
	    XtRemoveAllCallbacks (
			this->baseWidget(),
			XmNdestroyCallback);

            // Set the ToolTalk Status to Reject the request
            if (_tt_msg && tt_message_status(_tt_msg) == TT_OK ) {
              tt_message_status_set(_tt_msg, TT_MEDIA_ERR_FORMAT);
            } else if ( theRoamApp.num_windows()-1 == 0 ||
                      theRoamApp.num_windows()-1 == theCompose.numNotInUse() ) {
              theRoamApp.setExitCode(1);
            }
	    this->quit();
 	}
	else {
	    if (_list->get_num_messages()) {
		setToNormalIcon();
	    }
	    else {
		setToEmptyIcon();
	    }
	}
    }
    else {
	// If the mailbox has messages, set to normal icon
	if (_list->get_num_messages() > 0) {
	    setToNormalIcon();
	}
	else {
	    setToEmptyIcon();
	}
    }
    OptCmd *oc = (OptCmd*)theRoamApp.mailOptions();
    if (oc) {
    	DtbOptionsDialogInfo oHandle =
		(DtbOptionsDialogInfo)oc->optionsDialog();
    	if (oHandle && oc->windowIsAlive()) {
		int numopen;
		if ((numopen = theRoamApp.numOpenRMW()) == 0) {
			XtUnmanageChild(oHandle->dialog_shellform);
			XtPopdown(oHandle->dialog);
		}
		else if (numopen == 1) {
			XtManageChild(oHandle->dialog_shellform);
			XtPopup(oHandle->dialog, XtGrabNone);
		}
	}
    }
}

void
RoamMenuWindow::message( char *text )
{
    XmString labelStr;
    if (!text) {
        text = "";
    }

    char *buf = new char[strlen(text)+1];
    memset(buf, 0, strlen(text)+1);
    sprintf(buf, "%s", text);
    //    if (strlen(text) >= MAX_STATUS_LEN) {
    //  strncpy(buf, text, MAX_STATUS_LEN - 1);
    //  buf[MAX_STATUS_LEN - 1] = '\0';
    //} else {
    //  sprintf(buf, "%s", text);
    //}
    labelStr = XmStringCreateLocalized(buf);
    XtVaSetValues(_message, XmNlabelString, labelStr, NULL);

    delete []buf;
    XmStringFree(labelStr);
}

void
RoamMenuWindow::setStatus(const char * msg)
{
    message((char *)msg);
}

void
RoamMenuWindow::clearStatus(void)
{
    message("");
}

void
RoamMenuWindow::message_summary()
{
    this->message_summary(
		list()->selected_item_position(),
		list()->get_num_messages(), 
		list()->get_num_new_messages(),
		list()->get_num_deleted_messages());
}

void
RoamMenuWindow::message_summary(
    int sel_pos,
    int num_msgs,
    int num_new,
    int num_deleted
)
{
    char *str;
    XmString labelStr;
    int num_live_msgs = num_msgs - num_deleted;  // Undeleted msgs

    str = catgets(DT_catd, 3, 13, "Message %d of %d, %d new, %d deleted");

    // We have to allocate enough space for all locales
    char *buf = new char[strlen(str)+MAX_STATUS_LEN];
    sprintf(buf, str, sel_pos, num_live_msgs, num_new, num_deleted);
    labelStr = XmStringCreateLocalized(buf);

    XtVaSetValues(_message_summary, XmNlabelString, labelStr, NULL);
//    XmUpdateDisplay(this->baseWidget());

    delete []buf;
    XmStringFree(labelStr);
}    

// void
// RoamMenuWindow::message_selected(
//     int msg_num,
//     int num_msgs,
//     int num_new,
//     int num_deleted
// )
// {
//     char *buf, *str;
//     XmString labelStr;

// 	/* NL_COMMENT
// 	 * The user will see the following message display as:
// 	 * "Message 3 of 10, 2 new, 6 deleted"
// 	 * This means ??? -- Explain to translator.
// 	 */
//     str = catgets(DT_catd, 3, 14, "Message %d of %d, %d new, %d deleted"); 
//     buf = new char[strlen(str) + 20];
//     sprintf(buf, str, msg_num, num_msgs, num_new, num_deleted);
    
//     labelStr = XmStringCreateLocalized(buf);

//     XtVaSetValues(_message_summary, XmNlabelString, labelStr, NULL);
// //    XmUpdateDisplay(this->baseWidget());

//     XmStringFree(labelStr);
//     delete buf;
// }
    

FindDialog *
RoamMenuWindow::get_find_dialog()
{
  
  if (!_findDialog) {
    theRoamApp.busyAllWindows();
    // No find dialog.  Create it
    _findDialog = new FindDialog(this);
    _findDialog->initialize();
    theRoamApp.unbusyAllWindows();
  }

  // Show it
  _findDialog->manage();
  _findDialog->popup();
  return _findDialog;
}

Views *
RoamMenuWindow::get_views_dialog(SdmBoolean show,
				 SdmBoolean create)
{
    SdmBoolean regenerate = Sdm_False;
    SdmError error;

    if (!_viewsDialog && create) {
	_viewsDialog = new Views(this);
	_viewsDialog->initialize();
    } else if (!_viewsDialog) {
	return ((Views *) NULL);
    }

    if (show) {
        _viewsDialog->manage();
	_viewsDialog->popup();
    }

    return _viewsDialog;
}

void 
RoamMenuWindow::quit()
{
    int i = 0;
    SdmMessageNumberL messageList;
    SdmMailRc * mailrc = NULL;
    SdmError error;
    SdmBoolean keep_deleted, keep = Sdm_False;


    if (_mailbox) {
      mailrc = get_mail_rc();
    }

    // update message store only if the server is still connected.
    // and we are not restarting after updating imapserver on props
    // and the mailbox is writable.
    if (!_server_disconnected && !_restart && mailBoxWritable()) {
    
      theRoamApp.busyAllWindows(catgets(DT_catd, 3, 15, "Saving..."));

      if (_list->get_num_total_deleted_messages()) {
        // We need to deal with deleted messages, based on what the
        // user wants to do. There are two properties that control
        // this. They are:
        //
        // keepdeleted - Keep deleted messages on close.
        // quietdelete - Delete without asking.
        //
        // If the user wants to keep the deleted messages, then we
        // can just blow by the second. If not, then we have to
        // clear the deleted messages, asking first based on the
        // second option.
        //

        keep_deleted = mailrc->IsValueDefined("keepdeleted");
        if (!keep_deleted) {
            // The user wants to expunge on close. See if they want
            // to be asked first.
            //
            SdmBoolean quiet_delete;
            quiet_delete = mailrc->IsValueDefined("quietdelete");
            if (!quiet_delete && !theRoamApp.killedBySignal()) {
              if (isIconified()) {
                  this->manage();
              }

              /* NL_COMMENT
               * This dialog comes up when the user tries to quit the
               * mailbox.  The user is asked if they want to destroy
               * the deleted messages.
               */
              _genDialog->setToQuestionDialog(
                  catgets(DT_catd, 3, 87, "Mailer - Close"),
                  catgets(DT_catd, 3, 88, "Destroy the deleted messages and close this mailbox?"));

              char * helpId = DTMAILHELPDESTROYMARKMSG;
              int answer = _genDialog->post_and_return(
                  catgets(DT_catd, 3, 89, "Destroy and Close"),
                  catgets(DT_catd, 3, 73, "Cancel"),
                  catgets(DT_catd, 3, 90, "Retain and Close"),
                  helpId);
              if (answer == 1) {
                  error.Reset();
                  _mailbox->ExpungeDeletedMessages(error, messageList);
                  if ( error ) {
		    _genDialog->post_error(error, DTMAILHELPCANNOTDESTROYDELMSGS,
					   catgets(DT_catd, 3, 134, "Mailer cannot destroy the messages you deleted."));
                  }
              } else if (answer == 2) {
                  // This is a very bad way to code selection of the 
                  // cancel button.  If someone changes its position
                  // in the dialog, this code will break!
                  theRoamApp.unbusyAllWindows();
                  return;
              } else if (answer == 3) {
                  error.Reset();
                  // we save the message store state here.  without this call,
                  // we rely on the closing of the message store to save the
                  // state.  if an error occurs between now and when the 
                  // message store closes, we'll lose the changes.  therefore,
                  // we save changes to the message store here.
                  _mailbox->SaveMessageStoreState(error);
                  if ( error ) {
		    _genDialog->post_error(error, DTMAILHELPCANNOTSAVEMAILBOX,
					   catgets(DT_catd, 3, 134, "Mailer cannot save the state of your mailbox.  Your messages remain intact."));
                  }
              }
            } else {
              // If killed by a signal, don't post a dialog.
              error.Reset();
              _mailbox->ExpungeDeletedMessages(error, messageList);
              if ( error ) {
                  if ( !theRoamApp.killedBySignal() ) {
		    _genDialog->post_error(error, DTMAILHELPCANNOTDESTROYDELMSGS,
					   catgets(DT_catd, 3, 134, "Mailer cannot destroy the messages you deleted."));
                  }
              }
            }
        }
      }

      // If the mailbox is local, we want to check to see if we need to remove it.
      // We remove it ONLY if the mailbox is empty (eg. mailbox size is zero).
      if (_mailbox && isLocal()) {

        struct stat sbuf;
        if (SdmSystemUtility::SafeStat((const char *)_mailbox_fullpath, &sbuf) != -1) {

          // If the message store is indeed now empty we can remove it
          if (sbuf.st_size == 0) {

            keep = mailrc->IsValueDefined("keep");
            if (!keep) {
              // we need to call RemoveMessageStoreIfEmpty to handle the removal of
              // the mailbox.  DO NOT CALL UNLINK OR SAFEUNLINK.  There could be a
              // timing situation where the store was updated with new messages since
              // we did the stat above.  The RemoveMessageStoreIfEmpty call gets a lock
              // on the mailbox file, check the size, and only remove it if the
              // size is zero.  It guarantees that the file is REALLY empty before
              // it's unlinked!!
              //
              SdmMessageUtility::RemoveMessageStoreIfEmpty(error, _mailbox_fullpath);
              if (error) {
                // we couldn't remove the mailbox.  maybe it's not empty anymore
                // so we just continue here.  reset the error.
                error.Reset();
              }
            }
          }
        }
      }

      theRoamApp.unbusyAllWindows();
    }
	    
    for (i = 0; i < _numDialogs; i++) {
      _dialogs[i]->unmanage();
    }

    for (i = 0; i < _numDialogs; i++) {
      _dialogs[i]->quit();
    }

    this->unmanage();

    delete this;

    if ( (theRoamApp.num_windows() == 0 ||
       theRoamApp.num_windows() == theCompose.numNotInUse()))
        theRoamApp.shutdown();
}



// Callback to open a new mail container.

void
RoamMenuWindow::file_selection_callback(
    void *client_data,
    char *selection
)
{
    // If there is a selection, do something.

    struct OpenStruct *open_struct=(struct OpenStruct *) client_data;
    char fullpath2[2048];

    if (selection != NULL && *selection != '\0') {
	RoamMenuWindow *rmw = open_struct->rmw;
        SdmError error;
	CustomSelectFileCmd *customsfc =(CustomSelectFileCmd*)open_struct->command;
        char *newdestname=NULL;
        // Dont want to add the inbox or sent.mail path to recent used list
        char *path = theRoamApp.getInboxPath();
        SdmMailRc *mailrc;
        MailConnection *mailConnection = theRoamApp.connection();
        SdmSession * d_session = theRoamApp.isession()->isession();
        mailConnection->connection()->SdmMailRcFactory(error, mailrc);
        MailRcSource *mrc = (MailRcSource*)new MailRcSource (mailrc, "record");
        char *value = (char*)mrc->getValue(), *expanded_val=NULL;
        delete mrc;

	// Dont add the record or inbox to the most recently used list
	if (customsfc->isLocal()) {
        	if (!rmw->mail_file_selection(selection, NULL)) {
			if (path && strcmp(path, selection) != 0) {
		    		SdmUtility::ExpandPath(error, expanded_val, 
					value, *mailrc, SdmUtility::kFolderResourceName);
		    		if (*expanded_val != '/') {
					char *folder = theRoamApp.getFolderDir(local, Sdm_True);
					if (strcmp(folder, "/") == 0)
						sprintf(fullpath2, "/%s", expanded_val);
					else
						sprintf(fullpath2, "%s/%s", folder, expanded_val);
					free(folder);
		    		} else 
					strcpy(fullpath2, expanded_val);

		    		if (strcmp(fullpath2, selection) != 0) {
					SdmUtility::GetRelativePath(error, newdestname,
			    			selection, *rmw->get_mail_rc(),
			    			SdmUtility::kFolderResourceName);
					rmw->addToRecentList(newdestname);
		    		}
			}
	    	}
	} else {
	    char *server;
	    if (strcmp((server = customsfc->getServerValue()), "") != 0) {
		// mail_file_selection returns false if the mailbox is not 
		// already open.
		if (!rmw->mail_file_selection( selection, server )) {
		    if (path && strcmp(path, selection) != 0) {
			SdmUtility::ExpandPath(error, expanded_val, value, 
			       *mailrc, SdmUtility::kFolderResourceName);
			if (*expanded_val != '/') {
			    char *folder = theRoamApp.getFolderDir(local, Sdm_True);
			    if (strcmp(folder, "/") == 0)
			    	sprintf(fullpath2, "/%s", expanded_val);
			    else
			    	sprintf(fullpath2, "%s/%s", folder, expanded_val);
			    free(folder);
			} else 
			    strcpy(fullpath2, expanded_val);

			if (strcmp(fullpath2, selection) != 0) {
			    char name[2048];
			    SdmUtility::GetRelativePath(error, newdestname,
				selection, *rmw->get_mail_rc(),
				SdmUtility::kImapFolderResourceName);
			    sprintf(name, "%s(%s)", newdestname, server);
			    rmw->addToRecentList(name);
			}
		    }
		}
	    }
	}
	if (value)
	    free(value);
	if (expanded_val)
	    free(expanded_val);
	if (newdestname)
	    free(newdestname);
	if (rmw->GoAway()) {
	    delete rmw;
	}
    }
}

// Given the name of a container, create a new RoamMenuWindow
// and open the container into it.

SdmBoolean
RoamMenuWindow::mail_file_selection(
    const char *selection,
    const char *server
)
{
    theRoamApp.busyAllWindows(catgets(DT_catd, 3, 20, "Opening mailbox..."));
//     XmUpdateDisplay(baseWidget());

    // We should fill in and pass in the token here so that the RoamMenuWindow
    // has the IMAP server information if it needs to use IMAP.
    SdmToken *token = new SdmToken;
    token->SetValue("servicechannel", "cclient");
    token->SetValue("serviceclass", "messagestore");
    if (server != NULL) {
	token->SetValue("servicetype", "imap");
	token->SetValue("serviceoption", "hostname", server);
    } else if (selection &&
               (strcasecmp(selection, "inbox") == 0) &&
               theRoamApp.getInboxServer()) {
	token->SetValue("servicetype", "imap");
	server = theRoamApp.getInboxServer();
	token->SetValue("serviceoption", "hostname", server);
    }
    else {
	token->SetValue("servicetype", "local");
    }
    // Check to see if the requested mailbox is already open.  If so just
    // raise it and return.
    MailSession *ses = theRoamApp.isession();
    if (ses->isMboxOpen(selection, server)) {
	RoamMenuWindow *rmw = ses->getRMW(selection, server);
	Widget w = rmw->baseWidget();
	XMapRaised(XtDisplay(w), XtWindow(w));
	theRoamApp.unbusyAllWindows();
	return Sdm_True;
    }

    RoamMenuWindow *mailview=new RoamMenuWindow(selection, (Tt_message)NULL, token);

    mailview->initialize(0, NULL);

    theRoamApp.unbusyAllWindows();
    return Sdm_False;
}

void
RoamMenuWindow::move_callback(
    void *client_data,
    char *selection
)
{
    if (selection == NULL || *selection == '\0')
	return;

    SdmError mail_error;

    // Initialize the mail_error.
    mail_error.Reset();

    struct OpenStruct *obj = (struct OpenStruct *) client_data;
    RoamMenuWindow *rmw = (RoamMenuWindow *) obj->rmw;
    MoveCopyCmd *mcc = (MoveCopyCmd *) obj->command;

    // Passing Sdm_True to copySelected tells the routine to delete
    // the selected messages after copying them. Trailing Sdm_False
    // means don't be silent
    if (mcc->isLocal())
    	rmw->_list->copySelected(mail_error, selection, NULL, Sdm_True, Sdm_False);
    else {
      char *server = strdup(thePasswordDialogManager->imapserver());
    	rmw->_list->copySelected(mail_error, selection, server, Sdm_True, Sdm_False);
    	free (server);
    }
    if ( mail_error ) {
      rmw->postErrorDialog(mail_error);
    }

}

void
RoamMenuWindow::copy_callback(
    void *client_data,
    char *selection
)
{
    if (selection == NULL || *selection == '\0')
	return;

    SdmError mail_error;

    // Initialize the mail_error.
    mail_error.Reset();

    struct OpenStruct *obj = (struct OpenStruct *) client_data;
    RoamMenuWindow *rmw = (RoamMenuWindow *) obj->rmw;
    MoveCopyCmd *mcc = (MoveCopyCmd *) obj->command;

    if (mcc->isLocal())
    	rmw->_list->copySelected(mail_error, selection, NULL, Sdm_False, Sdm_False);
    else {
      char *server = strdup(thePasswordDialogManager->imapserver());
    	rmw->_list->copySelected(mail_error, selection, server, Sdm_False, Sdm_False);
    	free (server);
    }
    if ( mail_error ) {
      rmw->postErrorDialog(mail_error);	
    }
}


void
RoamMenuWindow::create_container_callback(
    void *client_data,
    char *selection
)
{
    struct OpenStruct *obj = (struct OpenStruct *) client_data;
    RoamMenuWindow *rmw=(RoamMenuWindow *) obj->rmw;
    CustomSelectFileCmd *newContainerCmd = rmw->_new_container;

    if (newContainerCmd->isLocal()) {
      rmw->create_new_container(NULL, selection );
    } else {
      char *server = strdup(thePasswordDialogManager->imapserver());
      rmw->create_new_container(server, selection );
      free(server);
    }
}

void
RoamMenuWindow::create_new_container(char* server, 
    char *filename
)
{
    int answer;
    char buf[2048];
    char * helpId;
    char *user=NULL, *password = NULL;
    SdmMailRc * mailRc;
    SdmError error;

    // create token for destination store.
    SdmToken token;
    token.SetValue("servicechannel", "cclient");
    token.SetValue("serviceclass", "messagestore");
    token.SetValue("serviceoption", "messagestorename", filename);
    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
    assert(!error);
    if (mailRc->IsValueDefined("dontsavemimemessageformat"))
	token.SetValue("serviceoption", "preservev3messages", "x");

    if (server == NULL  || *server == '\0') 
    {
      token.SetValue("servicetype", "local");

      // if this is a local mailbox and the mailbox exists, prompt the
      // user to see if it's ok to delete it.
      //
      if (SdmSystemUtility::SafeAccess(filename, F_OK) == 0) {

        sprintf(buf, catgets(DT_catd, 3, 21, "You are creating a new mailbox, but one already exists with the\n\
name you specified.\n\n\
Click Delete and Replace to delete the existing mailbox and replace it\n\
with a new, empty mailbox.  Otherwise, click Cancel."));
        _genDialog->setToQuestionDialog(
            catgets(DT_catd, 3, 22, "Mailer"),
            buf);
        helpId = DTMAILHELPNEWMBOXOVERWRITE;
        answer = _genDialog->post_and_return(catgets(DT_catd, 3, 147, "Delete and Replace"), 
						catgets(DT_catd, 1, 5, "Cancel"), 
						helpId);

        if (answer == 2) { // Pressed cancel
            return;
        }

        if (SdmSystemUtility::SafeUnlink(filename) < 0) {
            sprintf(buf, 
          catgets(DT_catd, 3, 23, "Unable to overwrite %s.\nCheck file permissions and retry."), 
          filename);

            _genDialog->setToErrorDialog(
            catgets(DT_catd, 3, 24, "Mailer"),
            buf);
            helpId = DTMAILHELPERROR;
            answer = _genDialog->post_and_return(helpId);
            return;
            
        } // end if SaveUnlink
      }  // end if SafeAccess


    // if this is not a local mailbox then we look for file on server.  
    // if exists,  ask the user if they want to delete it.  
    // If we can't open the file as a mailbox or delete it,
    // return error to user.  if file doesn't exist, just continue.
    //
    } else  {

      token.SetValue("servicetype", "imap");

      thePasswordDialogManager->setImapServer(strdup(server));

      if (!thePasswordDialogManager->loginDialog())
        return;

      token.SetValue("serviceoption", "hostname", thePasswordDialogManager->imapserver());
      token.SetValue("serviceoption", "username", thePasswordDialogManager->user());
      token.SetValue("serviceoption", "password", thePasswordDialogManager->password());

      SdmMessageStore *newmbox;
      SdmIntStrL scanResults;
      
      // attach to server but don't open the mailbox.
      if (theRoamApp.isession()->isession()->SdmMessageStoreFactory(error, newmbox)) {
        fprintf(stderr, "Can't create new mailbox.\n");
        return;
      }
      if (newmbox->StartUp(error)) {
        delete newmbox;
        fprintf(stderr, "Can't start up new mailbox.\n");
        return;
      }
      if (newmbox->Attach(error, token)) {
        delete newmbox;
        fprintf(stderr, "Can't attach to new mailbox.\n");
        return;      
      }
      
      // check to see if mailbox exists on server.  if it doesn't exist,
      // delete the temporary mailbox object and continue.
      if (newmbox->ScanNamespace(error, scanResults, filename, "") || 
	scanResults.ElementCount() == 0 || 
	(scanResults.ElementCount() == 1 && 
	*scanResults[0].GetString() == NULL) ) {
        delete newmbox;
        
      } else {
          // prompt the user to see if it's ok to delete it.
	  sprintf(buf, catgets(DT_catd, 3, 21, "You are creating a new mailbox, but one already exists with the\n\
name you specified.\n\n\
Click Delete and Replace to delete the existing mailbox and replace it\n\
with a new, empty mailbox.  Otherwise, click Cancel."));
          _genDialog->setToQuestionDialog(
              catgets(DT_catd, 3, 22, "Mailer"),
              buf);
          helpId = DTMAILHELPNEWMBOXOVERWRITE;
          answer = _genDialog->post_and_return(catgets(DT_catd, 3, 147, "Delete and Replace"), 
							catgets(DT_catd, 1, 5, "Cancel"),
							helpId);

          if (answer == 2) { // Pressed cancel
              delete newmbox;
              return;
          }
          
          SdmMessageNumber nmsgs = 0;
          // user says it's ok to delete the mailbox.  try to delete it here.
          if (newmbox->Delete(error, filename)) {
              sprintf(buf, 
		      catgets(DT_catd, 3, 23, "Unable to overwrite %s.\nCheck file permissions and retry."), 
		      filename);
	      _genDialog->post_error(error, DTMAILHELPERROR, buf);
              delete newmbox;
              return;
          } else {
            // delete of mailbox on server was successful.  clean up
            // temporary mailbox object.
            delete newmbox;
          }
      }
    }
   

    // create new store.
    SdmMessageStore *newmbox;
    if (theRoamApp.isession()->isession()->SdmMessageStoreFactory(error, newmbox))
      return;
    if (!error) {
      newmbox->StartUp(error);
    }
    if (!error) {
      newmbox->Attach(error, token);
    }
    if (!error) {
      newmbox->Create(error, filename);
    }
    if (!error) {
      // Path filename is ok -- now follow the same route as for Open
      delete newmbox;

      SdmString host;
      if (token.FetchValue(host, "serviceoption", "hostname")) {
	  // mail_file_selection should never return True because we should have
	  // never gotten this far trying to create a new mbox if that mbox is
	  // already open.
	  mail_file_selection(filename, (const char*)host);
      } else {
	  mail_file_selection(filename, NULL);
      }       
    } else if (error && newmbox) {
      delete newmbox;
      postErrorDialog(error);
    }
 
    return;
}

void
RoamMenuWindow::rename_mailbox_callback(
    void *client_data,
    char *selection
)
{
    struct OpenStruct *obj = (struct OpenStruct *) client_data;
    RoamMenuWindow *rmw=(RoamMenuWindow *) obj->rmw;
    CustomSelectFileCmd *renameContainerCmd = rmw->_rename_mailbox;

    if (renameContainerCmd->isLocal()) {
      rmw->rename_mailbox(NULL, selection );
    } else {
      char *server = strdup(thePasswordDialogManager->imapserver());
      rmw->rename_mailbox(server, selection );
    }
}

static Widget
getTopLevelShell(Widget w)
{
    Widget diashell, topshell;

    for (diashell=w;!XtIsShell(diashell);diashell=XtParent(diashell));

    for ( topshell=diashell;XtIsTopLevelShell( topshell );
	topshell = XtParent( topshell ));
    
    return(topshell);
}

// This routine cleans up the callbacks that were installed for
// the rename dialog.
void
RoamMenuWindow::rename_mailbox_cleanup(
    Widget w,
    struct RenameStruct *rename_struct
)
{
    // Remove all callbacks to avoid having duplicate
    // callback functions installed.

    if (XtHasCallbacks (w, XmNokCallback) == XtCallbackHasSome) {
	XtRemoveCallback(w,
			XmNokCallback,
			&RoamMenuWindow::rename_mailbox_ok,
			(XtPointer) rename_struct);
    }

    if (XtHasCallbacks(w, XmNcancelCallback) == XtCallbackHasSome) {
	XtRemoveCallback(w,
			XmNcancelCallback,
			&RoamMenuWindow::rename_mailbox_cancel,
			(XtPointer) rename_struct);
    }

    if (XtHasCallbacks(w, XmNhelpCallback) == XtCallbackHasSome) {
	XtRemoveAllCallbacks(w, XmNhelpCallback);
    }
    
    Widget topShell = getTopLevelShell(w);

    Atom WM_DELETE_WINDOW = XmInternAtom(XtDisplay(w),
					    "WM_DELETE_WINDOW",
					    False);
    XmRemoveWMProtocolCallback(topShell,
				WM_DELETE_WINDOW,
				(XtCallbackProc) &RoamMenuWindow::rename_mailbox_cancel,
				(XtPointer) rename_struct);
}


void 
RoamMenuWindow::rename_mailbox(
    char *server, 
    char *filename
)
{
    Widget rename_mailbox_dialog;
    XmString message, mailbox_name;
    char *basename;
    char path_sep = '/';
    struct RenameStruct *rename_struct;

    rename_struct = (struct RenameStruct *) malloc (sizeof(RenameStruct));
    memset(rename_struct, 0, sizeof(RenameStruct));

    rename_mailbox_dialog = XmCreatePromptDialog( this->baseWidget(),
						  "renameMailboxDialog",
						  NULL,
						  0);
    rename_struct->rmw = this;
    rename_struct->server = server;
    rename_struct->filename = filename;
    rename_struct->dialog = rename_mailbox_dialog;
    message = XmStringCreateLocalized(catgets(DT_catd, 1, 349, "New mailbox name:"));
    XtVaSetValues(rename_mailbox_dialog, XmNselectionLabelString, message, NULL);
    XmStringFree(message);

    // Strip off the directory part of the file name and get just the basename
    // (we don't want to overload the user with the full path name)
    //
    if ((basename = strrchr(filename, path_sep)) != NULL) {
	basename++;  // Skip over the last "/" character to point to just basename.
    } else {
	basename = filename;
    }
    mailbox_name = XmStringCreateLocalized(basename);
    XmString ok_str = XmStringCreateLocalized(catgets(DT_catd, 1, 350, "Rename"));
    XtVaSetValues(XtParent(rename_mailbox_dialog),
		    XmNtitle, catgets(DT_catd, 1, 345, "Rename Mailbox"),
		    NULL);
    XtVaSetValues(rename_mailbox_dialog,
		    XmNtextString, mailbox_name,
		    XmNokLabelString, ok_str,
		    XmNautoUnmanage, False,
		    XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL,
		    NULL);
    XmStringFree(mailbox_name);
    XmStringFree(ok_str);
    XtAddCallback(XmSelectionBoxGetChild(rename_mailbox_dialog, XmDIALOG_HELP_BUTTON),
		    XmNhelpCallback, HelpCB, DTMAILHELPMBOXRENAME);
    XtAddCallback(rename_mailbox_dialog, 
		    XmNcancelCallback, &RoamMenuWindow::rename_mailbox_cancel, 
		    (XtPointer) rename_struct);
    XtAddCallback(rename_mailbox_dialog, 
		    XmNokCallback, &RoamMenuWindow::rename_mailbox_ok,
		    (XtPointer) rename_struct);
    Widget topShell = getTopLevelShell(rename_mailbox_dialog);
    Atom WM_DELETE_WINDOW=XmInternAtom( XtDisplay( rename_mailbox_dialog ),
		    "WM_DELETE_WINDOW",
		    False );
    XmAddWMProtocolCallback( topShell,
                             WM_DELETE_WINDOW,
                             ( XtCallbackProc ) rename_mailbox_cancel,
                             (XtPointer) rename_struct);
    XtManageChild(rename_mailbox_dialog);
    // We want the file name to be selected so that it will be replaced as
    // soon as the user starts typing, pending delete.
    Widget t_field = XmSelectionBoxGetChild(rename_mailbox_dialog, XmDIALOG_TEXT);
    XmTextFieldSetSelection(t_field, 0, strlen(basename), CurrentTime);

}


// Here's where we actually do the renaming of the mailbox.  The rename
// dialog only gives us the new base name, so we still need to get the
// old path and base name to complete the rename.
//
void
RoamMenuWindow::rename_mailbox_ok(
    Widget w,
    XtPointer clientData,
    XtPointer callData
)
{
    char *new_basename;
    char new_path[MAXPATHLEN];
    struct RenameStruct *rename_struct = (RenameStruct *) clientData;
    RoamMenuWindow *rmw = rename_struct->rmw;
    char *server = rename_struct->server;
    char *old_path = rename_struct->filename;
    char *path_ptr;
    XmSelectionBoxCallbackStruct *cbs = (XmSelectionBoxCallbackStruct *) callData;
    char path_sep = '/';
    char dialog_buf[2048];
    char *helpId;
    
    XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &new_basename);

    // We take the new base name and the old path to create the new path.
    strcpy(new_path, old_path);
    path_ptr = strrchr(new_path, path_sep);
    path_ptr++;  // skip over last '/'
    strcpy(path_ptr, new_basename);

    // Now that we have both the new path and old path, do the copy
    // First, we check to see if the new path already exists.  If it
    // does, we pop up a dialog asking the user to confirm that he
    // or she wants to overwrite it.

    SdmIntStrL scanResults;
    SdmMessageStore *mstore;
    SdmToken token;
    SdmError mail_error;
    MailSession *session = theRoamApp.isession();
    session->isession()->SdmMessageStoreFactory(mail_error, mstore);

    token.SetValue("servicechannel", "cclient");
    token.SetValue("serviceclass", "messagestore");
    token.SetValue("messagestorename", old_path);


    if (server == NULL || *server == '\0') {
	token.SetValue("servicetype", "local");
    } else {
	token.SetValue("servicetype", "imap");
	token.SetValue("serviceoption", "hostname", server);
	token.SetValue("serviceoption", "username", thePasswordDialogManager->user());
	token.SetValue("serviceoption", "password", thePasswordDialogManager->password());
    }
    mstore->StartUp(mail_error);
    if (mail_error != Sdm_EC_Success) {
	DtMailGenDialog *genDialog = theRoamApp.genDialog();
	genDialog->post_error(mail_error, DTMAILHELPIMAPERROR,
				catgets(DT_catd, 16, 7, "An error occurred in accessing the IMAP server."));
	return;
    }
    mstore->Attach(mail_error, token);
    if (mail_error != Sdm_EC_Success) {
	DtMailGenDialog *genDialog = theRoamApp.genDialog();
	genDialog->post_error(mail_error, DTMAILHELPIMAPERROR,
				catgets(DT_catd, 16, 7, "An error occurred in accessing the IMAP server."));
	mstore->Close(mail_error);
	delete mstore;
	return;
    }

    int answer = 0;   // The response from the user pressing one of the buttons in a dialog.
    if (strcmp(old_path, theRoamApp.GetInbox(Sdm_True)) == 0) {
	sprintf(dialog_buf, catgets(DT_catd, 3, 151,
		    "You have specified your Inbox to be renamed.  Renaming your Inbox\nchanges it to a regular mailbox.  Mailer will create a new Inbox to receive\nmail."));
	helpId = DTMAILHELPRENAMEINBOX;
	DtMailGenDialog *genDialog = theRoamApp.genDialog();
	genDialog->setToQuestionDialog(catgets(DT_catd, 3, 22, "Mailer"), dialog_buf);
	answer = genDialog->post_and_return(catgets(DT_catd, 1, 350, "Rename"), 
						helpId);
	if (answer == 2) { // Cancel selected.
	    mstore->Close(mail_error);
	    delete mstore;
	    return;
	}
    }
    // If the mailbox exists, ask the user if it's okay to overwrite it.
    if (!(mstore->ScanNamespace(mail_error, scanResults, new_path, "") || scanResults.ElementCount() == 0)) {
	// Prompt the user if it's okay to overwrite it.
	sprintf(dialog_buf, catgets(DT_catd, 3, 150,
		    "A mailbox named %s already exists.  Either overwrite the mailbox,\nspecify another name, or cancel the operation."),
		    new_path);
	helpId = DTMAILHELPRENAMEEXISTS;
	DtMailGenDialog *genDialog = theRoamApp.genDialog();
	genDialog->setToQuestionDialog(catgets(DT_catd, 3, 22, "Mailer"), dialog_buf);
	answer = genDialog->post_and_return(catgets(DT_catd, 1, 351, "Overwrite"), 
						catgets(DT_catd, 1, 305, "Cancel"),
						catgets(DT_catd, 1, 352, "Choose New Name..."),
						helpId);
	if (answer == 2) { // Cancel clicked
	    mstore->Close(mail_error);
	    delete mstore;
	    return;
	} else if (answer == 3) {
	    // Go back to the rename dialog box.
	    delete mstore;
	    XtManageChild(w);
	    // We want the file name to be selected so that it will be replaced as
	    // soon as the user starts typing, pending delete.
	    Widget t_field = XmSelectionBoxGetChild(w, XmDIALOG_TEXT);
	    char *name;
	    XtVaGetValues(t_field, XmNvalue, &name, NULL);
	    XmTextFieldSetSelection(t_field, 0, strlen(name), CurrentTime);
	    return;
	} else { // Overwrite means delete and then rename, so we'll do the delete part here.
	    if (mstore->Delete(mail_error, new_path)) {
		sprintf(dialog_buf,
		    catgets(DT_catd, 3, 23, "Unable to overwrite %s.\nCheck file permissions and retry."),
		    old_path);
		genDialog->post_error(mail_error, DTMAILHELPERROR, dialog_buf);
		delete mstore;
		return;
	    }
	}
    }

    // We know the new name doesn't already exist, so we can do the rename now.

    mstore->Rename(mail_error, old_path, new_path);
    if (mail_error) {
	sprintf(dialog_buf, catgets(DT_catd, 3, 152, "Mailer cannot rename the mailbox %s."), old_path);
	DtMailGenDialog *genDialog = theRoamApp.genDialog();
	genDialog->post_error(mail_error, DTMAILHELPRENAMEERROR, dialog_buf);
    } else {
	rmw->message(catgets(DT_catd, 3, 157, "Renaming completed"));
    }
    free(old_path);
    free(rename_struct->server);
    free(rename_struct);
    delete mstore;
    XtDestroyWidget(w);
}

void
RoamMenuWindow::rename_mailbox_cancel(
    Widget w,
    XtPointer clientData,
    XtPointer callData
)
{
    XmAnyCallbackStruct *cbs = (XmAnyCallbackStruct *) callData;
    struct RenameStruct *rename_struct = (RenameStruct *) clientData;
    RoamMenuWindow *rmw = (RoamMenuWindow *) rename_struct->rmw;

    if (cbs->reason == XmCR_PROTOCOLS)
	rmw->rename_mailbox_cleanup((Widget) rename_struct->dialog, rename_struct);
    else
	rmw->rename_mailbox_cleanup(w, rename_struct);
	
    free(rename_struct->server);
    free(rename_struct->filename);
    free(rename_struct);
    XtDestroyWidget(w);
}
void
RoamMenuWindow::delete_mailbox_callback(
    void *client_data,
    char *selection
)
{
    struct OpenStruct *obj = (struct OpenStruct *) client_data;
    RoamMenuWindow *rmw=(RoamMenuWindow *) obj->rmw;
    CustomSelectFileCmd *newContainerCmd = rmw->_delete_mailbox;

    if (newContainerCmd->isLocal()) {
      rmw->delete_mailbox(NULL, selection );
    } else {
      char *server = strdup(thePasswordDialogManager->imapserver());
      rmw->delete_mailbox(server, selection );
    }
}

void
RoamMenuWindow::delete_mailbox(
    char *server,
    char *filename
)
{
    SdmError		mail_error;
    SdmSession *i_session = theRoamApp.isession()->isession();
    SdmMessageStore	*mstore = NULL;    // We use this to get a connection to the IMAP server.
    SdmToken token;
    char dialog_buf[2048];
    char *helpId;
    int answer;

    theRoamApp.osession()->osession()->SdmMessageStoreFactory(mail_error, mstore);
    if (mail_error) {
	DtMailGenDialog *genDialog = theRoamApp.genDialog();
	genDialog->post_error(mail_error, DTMAILHELPIMAPERROR,
				catgets(DT_catd, 16, 7, "An error occurred in accessing the IMAP server."));
	if (mstore)
	    delete mstore;
	free(server);
	free(filename);
	return;
    }

    // Set up the initial token info
    token.SetValue("servicechannel", "cclient");
    token.SetValue("serviceclass", "messagestore");

    // Set up token info based on whether we are using IMAP or local.
    if (server) {
	token.SetValue("servicetype", "imap");
	token.SetValue("serviceoption", "hostname", server);
	token.SetValue("serviceoption", "username", thePasswordDialogManager->user());
	token.SetValue("serviceoption", "password", thePasswordDialogManager->password());
    } else { // local
	passwd pw;
	SdmSystemUtility::GetPasswordEntry(pw);
	token.SetValue("servicetype", "local");
	token.SetValue("serviceoption", "username", pw.pw_name);
    }
    mail_error.Reset();

    // Get a connection to the IMAP server
    mstore->StartUp(mail_error);
    if (mail_error != Sdm_EC_Success) {
	DtMailGenDialog *genDialog = theRoamApp.genDialog();
	genDialog->post_error(mail_error, DTMAILHELPIMAPERROR,
				catgets(DT_catd, 16, 7, "An error occurred in accessing the IMAP server."));
	free(server);
	free(filename);
	delete mstore;
	return;
    }
    mstore->Attach(mail_error, token);
    if (mail_error) {
	DtMailGenDialog *genDialog = theRoamApp.genDialog();
	genDialog->post_error(mail_error, DTMAILHELPIMAPERROR,
				catgets(DT_catd, 16, 7, "An error occurred in accessing the IMAP server."));
	free(server);
	free(filename);
	delete mstore;
	return;
    }

    // We pop up a confirmation dialog here warning the user that this operation can't
    // be undone, so are they sure they want to do it?  We pop up a special warning
    // if the mailbox looks like it might be the inbox.

    if (strcmp(filename, theRoamApp.GetInbox(Sdm_True)) != 0) {
	sprintf(dialog_buf, 
		    catgets(DT_catd, 3, 153, "Mailer will delete the mailbox %s.\nYou cannot undelete it afterward."),
		    filename);
	_genDialog->setToQuestionDialog(catgets(DT_catd, 3, 22, "Mailer"), dialog_buf);
	helpId = DTMAILHELPMBOXDELETE;
	answer = _genDialog->post_and_return(catgets(DT_catd, 1, 353, "Delete"), catgets(DT_catd, 1, 5, "Cancel"), helpId);
    } else {   // It looks like the inbox.
	_genDialog->setToQuestionDialog(catgets(DT_catd, 3, 22, "Mailer"),
		    catgets(DT_catd, 3, 154, "You have specified your Inbox to be deleted.\nYou cannot undelete it afterward."));
	helpId = DTMAILHELPINBOXDELETE;
	answer = _genDialog->post_and_return(catgets(DT_catd, 1, 353, "Delete"), catgets(DT_catd, 1, 5, "Cancel"), helpId);
    }

    if (answer == 2) { // Pressed cancel
	free(server);
	free(filename);
	delete mstore;
	return;
    }

    // We could do a ScanNamespace here to see if the file we want to delete exists.
    // You would think that since we had to get here from the file selection box, that
    // the file should exists, but there is always the possibility that the user typed
    // in the name by hand and made a mistake.  The c-client will return an error if
    // the file you want to delete doesn't exist.

    mstore->Delete(mail_error, filename);
    if (mail_error) {
	sprintf(dialog_buf, catgets(DT_catd, 3, 155, "Mailer cannot delete the mailbox %s."), filename);
	DtMailGenDialog *genDialog = theRoamApp.genDialog();
	genDialog->post_error(mail_error, DTMAILHELPMBOXDELETEERROR, dialog_buf);
    } else {
	// Using a path separator of '/' might produce weird results on non-Unix IMAP servers.
	char path_sep = '/';
	char *path_ptr;
	char msg_buf[256];

	path_ptr = strrchr(filename, path_sep);
	if (path_ptr) {
	    path_ptr++;
	} else {
	    path_ptr = filename;
	}
	sprintf(msg_buf, catgets(DT_catd, 3, 156, "%s deleted"), path_ptr);
	message(msg_buf);
    }
    free(server);
    free(filename);
    delete mstore;
    return;
}
    

ViewMsgDialog*
RoamMenuWindow::ifViewExists(SdmMessageNumber msg_num)
{

    int i;
    FORCE_SEGV_DECL(ViewMsgDialog, a_view);

    for (i = 0; i < _numDialogs; i++) {
	a_view = _dialogs[i];
	if (a_view->msgno() == msg_num) {
	    return(a_view);
	}
    }
    return(NULL);
}

void
RoamMenuWindow::setToolbarIndices(DtVirtArray<long> *indices)
{
    delete _toolbar_indices;
    _toolbar_indices = indices;
}

void
RoamMenuWindow::buildToolbar()
{
    FORCE_SEGV_DECL(CmdInterface, ci);
    SdmError error;
    char *value=NULL,*toolbarCmds=NULL,*token;
    char cmd_name[64];
    int useicons=0, len, i, toolbar_cmd_count=0;
    Widget w;
    Pixmap glyph, inactive_glyph;
    Pixel fg, bg;

//     XtVaSetValues(rowOfButtons,
// 		  XmNwidth, _width,
// 		  NULL);
    error.Reset();

    // Retrieve the list of commands that the user wants in their toolbar.
    // There are 3 scenarios we must deal with:
    // 		1) This is a new mailbox and the toolbarcommands mailrc
    //		   variable has not been set. (create a default toolbar)
    //		2) User has deleted all the commands from the toolbar via
    //		   the options dialog.  This is characterized by the
    //		   toolbarcommands variable being set to an empty string.
    //		   (remove buttons in toolbar and return)
    //		3) Rebuilding the toolbar due to the user changing something
    //		   via the options dialog.
    SdmMailRc * mailRc;
    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
    assert(!error);
    mailRc->GetValue(error, "toolbarcommands", &toolbarCmds);
    if (error && _toolbar_mailrc_string == NULL) {
	toolbarCmds = strdup("delete next previous reply,_include_message forward new_message print_one");
	_toolbar_mailrc_string = strdup(toolbarCmds);
	error.Reset();
    } else if (!error && !strcmp(toolbarCmds, "")) {
	if (_toolbar_mailrc_string)
	    free(_toolbar_mailrc_string);
	_toolbar_mailrc_string = NULL;
	if (toolbarCmds) {
	    free(toolbarCmds);
	    toolbarCmds = NULL;
	}
    } else {
	if (_toolbar_mailrc_string)
	    free(_toolbar_mailrc_string);
	_toolbar_mailrc_string = strdup(toolbarCmds);
    }

    mailRc->GetValue(error, "toolbarusetext", &value);
    if (error || (strcmp(value, "f") == 0) || value == NULL) {
	error.Reset();
	_toolbar_use_icons = Sdm_True;
    } else if (strcmp(value, "") == 0) {
	_toolbar_use_icons = Sdm_False;
    }
    if (value) free(value);

    int num_children=0;
    WidgetList children;
    XtVaGetValues(rowOfButtons,
		  XmNforeground,  &fg,
		  XmNbackground,  &bg,
		  XmNchildren, &children,
		  XmNnumChildren, &num_children,
		  NULL);

    if (num_children > 0) {
	XtUnmanageChildren(children, num_children);

	for (i=0; i<num_children; i++) {
	    XtDestroyWidget(children[i]);
	}
    }

    if (_toolbar_indices)
	delete _toolbar_indices;
    _toolbar_indices = new DtVirtArray<long>(6);

    if(!toolbarCmds) {
	return;
    }

    token = strtok(toolbarCmds, " ");
    while (token) {
	len = strlen(token);
	for (i = 0; i < len+1; i++) {
// DEBUG
	    *(token+i) = tolower(*(token+i));
	    if (*(token+i) == '_') {
		cmd_name[i] = ' ';
	    } else {
		cmd_name[i] = *(token+i);
	    }
	}
	//
	// Add a button to the toolbar for the command we just
	// plucked out of the toolbarCmds string.
	//
	for (i = 0; i < _toolbar_cmds->size(); i++) {
	    if (strcasecmp(cmd_name, (*_toolbar_cmds)[i]->name()) == 0) {
		_toolbar_indices->append((long)i);
		ci = new ButtonInterface(rowOfButtons,(*_toolbar_cmds)[i]);
		w = ci->baseWidget();
		XtAddCallback(w, XmNhelpCallback, HelpCB, DTMAILDELBTNID);
		if (_toolbar_use_icons) {
		    glyph = XmGetPixmap(XtScreen(w), (*_toolbar_icons)[i],
					fg, bg);
		    if (i < _toolbar_inactive_icons->length()) {
			inactive_glyph = XmGetPixmap(XtScreen(w),
					(*_toolbar_inactive_icons)[i],
					fg, bg);
			XtVaSetValues(w,
				      XmNlabelInsensitivePixmap, inactive_glyph,
				      NULL);
		    }
		    XtVaSetValues(w,
				  XmNlabelType, XmPIXMAP,
				  XmNlabelPixmap, glyph,
				  XmNuserData, (XtPointer) i,
				  NULL);
		    XtAddEventHandler(w, EnterWindowMask | LeaveWindowMask,
				     Sdm_False, &RoamMenuWindow::toolbarEH,
				      (XtPointer) this);
		}
// 		ci->manage();
		continue;
	    }
	}
	// Get the next command from the toolbarCmds string.
	token = strtok((char *)0, " ");
    }

    // We want to manage all of the children at once for performance reasons.
    XtVaGetValues(rowOfButtons,
		  XmNchildren, &children,
		  XmNnumChildren, &num_children,
		  NULL);
    XtManageChildren(children, num_children);

    free(toolbarCmds);
}


void
RoamMenuWindow::toolbarEH( Widget w,
			     XtPointer client_data,
			     XEvent *event,
			     char *)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *)client_data;
    int index;

    XtVaGetValues(w, XmNuserData, &index, NULL);

    switch (event->type) {
    case EnterNotify:
	rmw->showToolbarHelp(index);
	break;
    case LeaveNotify:
	rmw->clearStatus();
	break;
    }
}


void
RoamMenuWindow::showToolbarHelp( int index )
{
    setStatus((*_toolbar_cmds)[index]->getLabel());
}

void
RoamMenuWindow::mailboxName(char *new_name)
{
     if (_mailbox_name) {
          free (_mailbox_name);
     }
     _mailbox_name=strdup( new_name );
}

void
RoamMenuWindow::addToRowOfLabels(MsgScrollingList *msglist, int msgnums)
{
    XmString labelStr;
    Widget labels[4];
    int i=0;

    labelStr = XmStringCreateLocalized(catgets(DT_catd, 1, 14, "Sender"));
    labels[i++] = XtVaCreateWidget("Label1", xmLabelGadgetClass, 
				   rowOfLabels,
				   XmNlabelString, labelStr,
				   NULL);
    XmStringFree(labelStr);

    labelStr = XmStringCreateLocalized(catgets(DT_catd, 1, 15, "Subject"));
    labels[i++] = XtVaCreateWidget("Label2", 
				   xmLabelGadgetClass, rowOfLabels,
				   XmNlabelString, labelStr,
				   NULL);
    XmStringFree(labelStr);

    labelStr = XmStringCreateLocalized(catgets(DT_catd, 1, 16,
					       "Date and Time"));
    labels[i++] = XtVaCreateWidget("Label3", 
				   xmLabelGadgetClass, rowOfLabels,
				   XmNlabelString, labelStr,
				   NULL);
    XmStringFree(labelStr);

    labelStr = XmStringCreateLocalized(catgets(DT_catd, 1, 17,
					       "Size"));
    labels[i++] = XtVaCreateWidget("Label4", 
				   xmLabelGadgetClass, rowOfLabels,
				   XmNlabelString, labelStr,
				   NULL);
    XmStringFree(labelStr);
    XtManageChildren(labels, i);

    // Adjust labels so the align on the columns
    msglist->layoutLabels(msgnums, labels[0], labels[1],
			  labels[2], labels[3]);
}

void
RoamMenuWindow::addToRowOfMessageStatus()
{
    XmString labelStr1, labelStr2;
    Widget w[2];

    // Size of first label
    
    labelStr1 = XmStringCreateLocalized(catgets(DT_catd, 3, 25,
	   "Loading container..."));

    labelStr2 = XmStringCreateLocalized(
			catgets(DT_catd, 3, 26, "Folder Summary Information"));

    _message_summary = XtVaCreateWidget("Message_Summary", 
					xmLabelWidgetClass,
					rowOfMessageStatus,
					XmNalignment, XmALIGNMENT_END,
					XmNlabelString, labelStr2,
					XmNrightAttachment, XmATTACH_FORM,
					NULL);

    _message = XtVaCreateWidget("Message_Status_Text",
				xmLabelWidgetClass,
				rowOfMessageStatus,
				XmNrightAttachment, XmATTACH_WIDGET,
				XmNrightWidget, _message_summary,
				XmNrightOffset, 10,
				XmNrecomputeSize, False,
				XmNalignment, XmALIGNMENT_BEGINNING,
				XmNleftAttachment, XmATTACH_FORM,
				XmNtopAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, labelStr1,
				// Prevent Recomputeing label size for performance improvment
				XmNrecomputeSize, False,     
				NULL);

//      XtVaSetValues(_message,
// 	XmNalignment, XmALIGNMENT_BEGINNING,
// 	XmNleftAttachment, XmATTACH_FORM,
// 	XmNtopAttachment, XmATTACH_FORM,
// 	XmNbottomAttachment, XmATTACH_FORM,
//  	XmNlabelString, labelStr1,
//         // Prevent Recomputeing label size for performance improvment
//         XmNrecomputeSize, False,     
//  	NULL );
    
//      XtVaSetValues(_message_summary,
// 	XmNalignment, XmALIGNMENT_END,
//  	XmNlabelString, labelStr2,
//  	XmNrightAttachment, XmATTACH_FORM,
//  	NULL );

//      XtVaSetValues(_message,
// 		   XmNrightAttachment, XmATTACH_WIDGET,
// 		   XmNrightWidget, _message_summary,
// 		   XmNrightOffset, 10,
// 		   XmNrecomputeSize, False,
// 		   NULL);

     XmStringFree(labelStr1);
     XmStringFree(labelStr2);

     w[0] = _message;
     w[1] = _message_summary;
     XtManageChildren(w, 2);
}

SdmBoolean 
RoamMenuWindow::notInList(char *str, CmdList *list) 
{
	int i, length;
	
	if (list == NULL || str == NULL)
		return Sdm_True;
	length = list->size();
	for (i=0; i < length; i++) 
		if (strcmp((char*)(*list)[i]->getLabel(), str) == 0)
			return Sdm_False;
	return Sdm_True;
}

void
RoamMenuWindow::construct_file_menu()
{
    SdmError error;
    char *bufptr=NULL, *expanded_dir=NULL;
    SdmBoolean add_separator = Sdm_False;
    char *start_ptr, *p, *ptmp, *serverTypePtr;
    char buf[2048];

    // Create the "Container" item in the menubar.  And fill 
    // with items below the "Container" item in the menubar.

    _mailbox_cmdList = new CmdList( "Mailbox", catgets(DT_catd, 1, 18, "Mailbox"));

    if (!_check_new_mail)
    	_check_new_mail	= new CheckForNewMailCmd(
				"Check for New Mail",
				catgets(DT_catd, 1, 19, "Check for New Mail"), 
				Sdm_True, 
				this);

    if (!_msg_select_all)
    	_msg_select_all = new SelectAllCmd (
                                "Select All Messages",
                                catgets(DT_catd, 1, 36, "Select All Messages"),
                                Sdm_True, this );

    if (!_new_container) {
      struct OpenStruct *os = (struct OpenStruct *) malloc(sizeof(struct OpenStruct));
      os->rmw = this;
    	_new_container     = new CustomSelectFileCmd( 
				"New Mailbox",
				catgets(DT_catd, 1, 22, "New Mailbox..."),
				catgets(DT_catd, 1, 23, "Mailer - New Mailbox"),
				catgets(DT_catd, 1, 24, "New Mailbox"),
				Sdm_True,
				RoamMenuWindow::create_container_callback,
				CustomSelectFileCmd::doDirSearch,
				os,
				this->baseWidget());
    }

    if (!_rename_mailbox) {
      struct OpenStruct *os = (struct OpenStruct *) malloc(sizeof(struct OpenStruct));
      os->rmw = this;
      _rename_mailbox       = new CustomSelectFileCmd(
				"Rename_Mailbox",
				catgets(DT_catd, 1, 343, "Rename Mailbox..."),
				catgets(DT_catd, 1, 344, "Mailer - Rename Mailbox"),
				catgets(DT_catd, 1, 345, "Rename Mailbox"),
				Sdm_True,
				RoamMenuWindow::rename_mailbox_callback,
				CustomSelectFileCmd::doDirSearch,
				os,
				this->baseWidget());
    }

    if (!_delete_mailbox) {
      struct OpenStruct *os = (struct OpenStruct *) malloc(sizeof(struct OpenStruct));
      os->rmw = this;
      _delete_mailbox       = new CustomSelectFileCmd(
				"Delete_Mailbox",
				catgets(DT_catd, 1, 346, "Delete Mailbox..."),
				catgets(DT_catd, 1, 347, "Mailer - Delete Mailbox"),
				catgets(DT_catd, 1, 348, "Delete Mailbox"),
				Sdm_True,
				RoamMenuWindow::delete_mailbox_callback,
				CustomSelectFileCmd::doDirSearch,
				os,
				this->baseWidget());
    }

    if (_openmailboxcmd_list == NULL) {
      _openmailboxcmd_list = new DtVirtArray<OpenMailboxCmd *> (3);
    }

    _openCmdlist = new CmdList("Open Mailbox",
			       catgets(DT_catd, 1, 27, "Open Mailbox"));

    bufptr=NULL;
    SdmMailRc * mailRc;
    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
    assert(!error);
    mailRc->GetValue(error, "movemenu", &bufptr);

    OpenMailboxCmd *cmd;
    start_ptr = bufptr;
    for (p = bufptr; p && *p; p++) {
	    if ( isspace(*p) && !isspace(*(p-1)) && *(p-1) != '\\' ) {
		strncpy(buf, start_ptr, p - start_ptr);
		buf[p - start_ptr] = '\0';
		for (ptmp = strchr(buf, '\\'); ptmp; ptmp = strchr(buf, '\\')) {
		    for (; ptmp && *ptmp; ptmp++)
			*ptmp = *(ptmp + 1);
		}

		//theRoamApp.fromTokenToLabel(buf);
		if ( (serverTypePtr = strstr(buf, ",imap")) != NULL ) {
		    *serverTypePtr = ')';
		    *(serverTypePtr + 1) = '\0';
		    if ( (serverTypePtr = strrchr(buf, ',')) != NULL ) {
			*serverTypePtr = '(';
		    }
		}

		// Pass the final path name into ExpandPath so all of the Unix-isms get expanded
		//
		char *expanded_name = NULL;
		error.Reset();
		SdmUtility::ExpandPath(error, expanded_name, buf, *mailRc, SdmUtility::kFolderResourceName);
		char *bufToUse = ((error == Sdm_EC_Success && expanded_name != NULL) ? expanded_name : buf);

		cmd = new OpenMailboxCmd(bufToUse,
                                         bufToUse,
                                         Sdm_True,
                                         this,
                                         bufToUse);
		_openCmdlist->add(cmd);
		_openmailboxcmd_list->append(cmd);

		// Free up any storage used by the result of ExpandPath being called
		//
		if (expanded_name) {
		  free(expanded_name);
		  expanded_name = NULL;
		}

		start_ptr = p + 1;
	    }
    }
    if (bufptr) {
      free(bufptr);
      bufptr = NULL;
    }

    // Backward Compatibility - add any items that are in old list to new one.
    error.Reset();
    mailRc->GetValue(error, "filemenu2", &bufptr);
    if (!error && bufptr != NULL && *bufptr != NULL) {
	start_ptr = bufptr;
	for (p = bufptr; p && *p; p++) {
	    if ( isspace(*p) && !isspace(*(p-1)) && *(p-1) != '\\' ) {
		strncpy(buf, start_ptr, p - start_ptr);
		buf[p - start_ptr] = '\0';
		for (ptmp = strchr(buf, '\\'); ptmp; ptmp = strchr(buf, '\\')) {
		    for (; ptmp && *ptmp; ptmp++)
			*ptmp = *(ptmp + 1);
		}

		//theRoamApp.fromTokenToLabel(buf);
		if ( (serverTypePtr = strstr(buf, ",imap")) != NULL ) {
		    *serverTypePtr = ')';
		    *(serverTypePtr + 1) = '\0';
		    if ( (serverTypePtr = strrchr(buf, ',')) != NULL ) {
			*serverTypePtr = '(';
		    }
		}

		// Pass the final path name into ExpandPath so all of the Unix-isms get expanded
		//
		char *expanded_name = NULL;
		error.Reset();
		SdmUtility::ExpandPath(error, expanded_name, buf, *mailRc, SdmUtility::kFolderResourceName);
		char *bufToUse = ((error == Sdm_EC_Success && expanded_name != NULL) ? expanded_name : buf);

		if ( notInList(bufToUse, _openCmdlist) ) {
		    cmd = new OpenMailboxCmd(bufToUse,
					     bufToUse,
					     Sdm_True,
					     this,
					     bufToUse);
		    _openCmdlist->add(cmd);
		    _openmailboxcmd_list->append(cmd);
		}

		// Free up any storage used by the result of ExpandPath being called
		//
		if (expanded_name) {
		  free(expanded_name);
		  expanded_name = NULL;
		}

		start_ptr = p + 1;
	    }
	}
	add_separator = Sdm_True;
    }
    if (bufptr) {
	free(bufptr);
	bufptr = NULL;
    }
   
    // Get a handle to the Inbox.

    bufptr = theRoamApp.GetInbox();
    if (!_open_inbox) 
    	if (this->inbox())  // Deactivate the Open Inbox item 
		_open_inbox	       = new OpenMailboxCmd(
					"Inbox",
					catgets(DT_catd, 1, 20, "Inbox"),
					Sdm_False,
					this,
					bufptr);

    	else  // Activate the Open Inbox item.
		_open_inbox	       = new OpenMailboxCmd(
					"Inbox",
					catgets(DT_catd, 1, 21, "Inbox"),
					Sdm_True,
					this,
					bufptr);


     if (!_logfile) {
       char *logfile = theRoamApp.getResource(SentMessageLogFile, Sdm_True);
       _logfile = new OpenMailboxCmd(
                        "Sent Mail",
                        catgets(DT_catd, 1, 255, "Sent Mail"), 
                        Sdm_True,
                        this,
                        logfile);
       free (logfile); 
    }

    _open_other_mboxes = new CmdList ( "Open Other Mailboxes",
				       catgets(DT_catd, 1, 321, "Open Other Mailboxes"));

    // This is for mapping the Move pullright menus dynamically...
    MenuClientData *mcd = (MenuClientData*) malloc (sizeof(MenuClientData));
    expanded_dir = theRoamApp.getFolderDir(local, Sdm_True);
    mcd->filename = expanded_dir;
    mcd->op = DTM_OPEN;
    mcd->checksum = -1;
    mcd->timestamp = 0;
    _open_other_mboxes->setClientData((void*)mcd);
    // Create a structure to hold the RoamMenuWindow and the IMAP server name.
    struct OpenStruct *open_struct = (struct OpenStruct *) malloc(sizeof(struct OpenStruct));
    open_struct->rmw = this;

    if (!_open) {
    	_open              = new CustomSelectFileCmd(
				"Select Mailbox",
				catgets(DT_catd, 1, 65, "Select Mailbox..."), 
				catgets(DT_catd, 1, 26, "Mailer - Open Mailbox"),
				catgets(DT_catd, 1, 340, "Open Mailbox"),
				Sdm_True,
				RoamMenuWindow::file_selection_callback,
				CustomSelectFileCmd::doDirSearch,
				open_struct,
				this->baseWidget());
    }
    // Set the handle to the CustomSelectFileCmd so that the
    // file_selection_callback will have a handle to it.
    // This is all so that we can pass around the IMAP server
    // value.
    open_struct->command = _open;  

    if (add_separator)
    	_openCmdlist->add(_separator);
    _openCmdlist->add(_open_inbox);
    _openCmdlist->add(_logfile);
    _openCmdlist->add(_open);
    _openCmdlist->add(_open_other_mboxes);
    _openCmdlist->add(_separator);

    if (!_show_views)
	_show_views = new ViewsCmd("Show Views...",
				   catgets(DT_catd, 1, 310, "Show Views..."), 
				   Sdm_True,
				   this);


    if (!_destroyDeletedMessages)
    	_destroyDeletedMessages  = new DestroyCmd(
					"Destroy Deleted Messages",
					catgets(DT_catd, 1, 28, 
						"Destroy Deleted Messages"), 
					      Sdm_True,
					      this);


    if (!_quit)
    	_quit              = new QuitCmd (
				"Close",
				catgets(DT_catd, 1, 29, "Close"), 
				Sdm_True, 
				this);
    

    _mailbox_cmdList->add(_check_new_mail);
#ifdef AUTOFILING
    _mailbox_cmdList->add (theRoamApp.autoFiling());
#endif // AUTOFILING
    _mailbox_cmdList->add(_show_views);
    _mailbox_cmdList->add(_msg_select_all);
    _mailbox_cmdList->add(_separator);
    _mailbox_cmdList->add(_new_container);
    _mailbox_cmdList->add(_rename_mailbox);
    _mailbox_cmdList->add(_delete_mailbox);
    _mailbox_cmdList->add(_openCmdlist);
    _mailbox_cmdList->add(_separator);

    _mailbox_cmdList->add (_destroyDeletedMessages);
    _mailbox_cmdList->add(_separator);
    _mailbox_cmdList->add(_quit);
    
    // We only delay the menu creation at RoamMenuWindow startup
    if (_isstartup == Sdm_False) {
      _menuBar->addCommands (&_file_cascade, _mailbox_cmdList, Sdm_False, XmMENU_BAR
);
      setup_open_menu(this);
      setup_open_other_menu(this); // mapping the dynamic pullright menus
    } else {
      // IMPORTANT!! The following callback must be registered so that we can
      // access the widgets associated with the _openCmdlist object.  We cannot access
      // them now because the work procs installed by MenuBar::createPulldown()
      // that will create the widgets have not run yet.
      _openCmdlist->addMenuCompleteCallback(&RoamMenuWindow::setup_open_menu, this);
      _open_other_mboxes->addMenuCompleteCallback(
						  &RoamMenuWindow::setup_open_other_menu,
						  this);

      // IMPORTANT!!  MenuBar::addCommands will return without actually having created
      // the various widgets that make up the menu we are attempting to build.  By passing TRUE
      // for the last argument we are asking the MenuBar object to delay creation of
      // items on the menu as well as submenus until after the GUI is up and running.
      // The MenuBar object achieves this by using Xt work procedures (XtAppAddWorkProc).
      _menuBar->addCommands (&_file_cascade,
			     _mailbox_cmdList,
			     Sdm_False,
			     XmMENU_BAR,
			     Sdm_True);  // use XtWorkProc
    }
    _cached_open_containerlist = new DtVirtArray<OpenMailboxCmd *>
      (_max_cached_list_size ? _max_cached_list_size : 1);
}

void
RoamMenuWindow::setup_open_menu(void *client_data)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *)client_data;

    Widget cascade = rmw->_openCmdlist->getCascadeWidget();
    rmw->_openMenu = rmw->_openCmdlist->getPaneWidget();

    XtVaSetValues(rmw->_openMenu,
                  XmNpacking, XmPACK_COLUMN,
                  XmNorientation, XmVERTICAL,
                  NULL);

    XtVaSetValues(cascade, XmNuserData, rmw, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_open_menu, 
			rmw->_openMenu);
}

// For mapping the dynamic pullright menus from the Other Mailboxes...
void
RoamMenuWindow::setup_open_other_menu(void *client_data)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *)client_data;

    Widget cascade = rmw->_open_other_mboxes->getCascadeWidget();
    XtVaSetValues(cascade, XmNuserData, rmw, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, 
		rmw->_open_other_mboxes);
}

void
RoamMenuWindow::createCopyList(CmdList * copy_to)
{
    SdmError error;
    char *val = NULL, *cached_containerlist_size = NULL, *display_cfs = NULL;
    SdmBoolean addedEntries = Sdm_False;
    MoveCopyMenuCmd	*copy_container;
    char *start_ptr, *p, *ptmp, *serverTypePtr;
    char buf[2048];

    // Get names for permanent containers from .mailrc and fill in
    // the menu.  We get the items from the "movemenu" variable, and
    // we no longer read "filemenu", which was used by V2 mailtool and
    // kept along in V3 mailtool for backward compatibility. Now we maintain
    // filemenu2 for backward compatibility

    if (_filemenu2 != NULL) {
      free (_filemenu2);
      _filemenu2 = NULL;
    }
      
    SdmMailRc * mailRc;
    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
    assert(!error);
    mailRc->GetValue(error, "movemenu", &val);
    if (!error && val != NULL && *val != '\0') {
        _filemenu2 = (char *) val;
    }
    else {
      _filemenu2 = NULL;
      error.Reset();
    }
    // We will user _copy_containerlist and cached_copy_containerlist to keep
    // track of the commands associated with the Copy menu items.

    if (_copy_containerlist != NULL) 
      delete (_copy_containerlist);

    _copy_containerlist = new DtVirtArray<MoveCopyMenuCmd *> (3);

    mailRc->GetValue(error, "dontdisplaycachedfiles", &display_cfs);
    if (!error && display_cfs != NULL)
        // dontdisplaycachedfiles is set, so dont display cached files
	_max_cached_list_size = 0;
    else {
	error.Reset();
        // Find out how large the recently used container list can be.
	mailRc->GetValue(error, 
		"cachedfilemenusize", &cached_containerlist_size);
	if (!error && cached_containerlist_size && 
		*cached_containerlist_size)
		_max_cached_list_size = 
			(int) strtol(cached_containerlist_size, NULL, 10);
	else {
		_max_cached_list_size = 10;
		error.Reset();
	}
    }

    _cached_copy_containerlist = new
		DtVirtArray<MoveCopyMenuCmd *>
		(_max_cached_list_size ? _max_cached_list_size : 1);

    if (val) {
	// Create arrays to hold the user defined container list and the
	// recently visited (cached) container list.

	addedEntries = Sdm_True;
	start_ptr = val;
	for (p = val; p && *p; p++) {
	    if ( isspace(*p) && !isspace(*(p-1)) && *(p-1) != '\\' ) {
		strncpy(buf, start_ptr, p - start_ptr);
		buf[p - start_ptr] = '\0';
		for (ptmp = strchr(buf, '\\'); ptmp; ptmp = strchr(buf, '\\')) {
		    for (; ptmp && *ptmp; ptmp++)
			*ptmp = *(ptmp + 1);
		}

		//theRoamApp.fromTokenToLabel(buf);
		if ( (serverTypePtr = strstr(buf, ",imap")) != NULL ) {
		    *serverTypePtr = ')';
		    *(serverTypePtr + 1) = '\0';
		    if ( (serverTypePtr = strrchr(buf, ',')) != NULL ) {
			*serverTypePtr = '(';
		    }
		}

		// Pass the final path name into ExpandPath so all of the Unix-isms get expanded
		//
		char *expanded_name = NULL;
		error.Reset();
		SdmUtility::ExpandPath(error, expanded_name, buf, *mailRc, SdmUtility::kFolderResourceName);
		char *bufToUse = ((error == Sdm_EC_Success && expanded_name != NULL) ? expanded_name : buf);

		copy_container = new MoveCopyMenuCmd(bufToUse,
						     bufToUse,
						     Sdm_True,
						     this,
						     DTM_COPY);
		copy_to->add(copy_container);
		_copy_containerlist->append(copy_container);
		_movecopymenucmd_list->append(copy_container);

		// Free up any storage used by the result of ExpandPath being called
		//
		if (expanded_name) {
		  free(expanded_name);
		  expanded_name = NULL;
		}

		start_ptr = p + 1;
	    }
	}
    }

    // Backward compatilibilty
    val = NULL;
    mailRc->GetValue(error, "filemenu2", &val);
    if ((val != NULL) && (*val != '\0') && (!error)) {
	addedEntries = Sdm_True;
	start_ptr = val;
	for (p = val; p && *p; p++) {
	    if ( isspace(*p) && !isspace(*(p-1)) && *(p-1) != '\\' ) {
		strncpy(buf, start_ptr, p - start_ptr);
		buf[p - start_ptr] = '\0';
		for (ptmp = strchr(buf, '\\'); ptmp; ptmp = strchr(buf, '\\')) {
		    for (; ptmp && *ptmp; ptmp++)
			*ptmp = *(ptmp + 1);
		}

		//theRoamApp.fromTokenToLabel(buf);
		if ( (serverTypePtr = strstr(buf, ",imap")) != NULL ) {
		    *serverTypePtr = ')';
		    *(serverTypePtr + 1) = '\0';
		    if ( (serverTypePtr = strrchr(buf, ',')) != NULL ) {
			*serverTypePtr = '(';
		    }
		}

		// Pass the final path name into ExpandPath so all of the Unix-isms get expanded
		//
		char *expanded_name = NULL;
		error.Reset();
		SdmUtility::ExpandPath(error, expanded_name, buf, *mailRc, SdmUtility::kFolderResourceName);
		char *bufToUse = ((error == Sdm_EC_Success && expanded_name != NULL) ? expanded_name : buf);

		if ( notInList(bufToUse, copy_to) ) {
			copy_container = new MoveCopyMenuCmd(bufToUse,
						     bufToUse,
						     Sdm_True,
						     this,
						     DTM_COPY);
			copy_to->add(copy_container);
			_copy_containerlist->append(copy_container);
			_movecopymenucmd_list->append(copy_container);
		}

		// Free up any storage used by the result of ExpandPath being called
		//
		if (expanded_name) {
		  free(expanded_name);
		  expanded_name = NULL;
		}

		start_ptr = p + 1;
	    }
	}
    }
    if (val) {
      free(val);
      val = NULL;
    }

    _copy_inbox = new CopyToInboxCmd(
			    "Inbox",
			    catgets(DT_catd, 1, 221, "Inbox"), 
			    Sdm_True, 
			    this);

    _copy_other_mboxes = new CmdList ( "Copy to Other Mailboxes",
		       catgets(DT_catd, 1, 252, "Copy to Other Mailboxes"));

     // This is for mapping the Copy pullright menus dynamically...
     MenuClientData *mcd = (MenuClientData*) malloc (sizeof(MenuClientData));
     mcd->filename = theRoamApp.getFolderDir(local, Sdm_True);
     mcd->op = DTM_COPY;
     mcd->checksum = -1;
     mcd->timestamp = 0;
     _copy_other_mboxes->setClientData((void*)mcd);

    if (addedEntries) {
	copy_to->add(_separator);
    }
    struct OpenStruct *os = (struct OpenStruct *) malloc(sizeof(struct OpenStruct));
    os->rmw = this;
    _copy_move_button = new MoveCopyCmd (
                                "Select Mailbox",
                                catgets(DT_catd, 1, 65, "Select Mailbox..."),
                                Sdm_True,
                                RoamMenuWindow::move_callback,
                                RoamMenuWindow::copy_callback,
                                CustomSelectFileCmd::doDirSearch,
                                os,
                                this->baseWidget(),
                                catgets(DT_catd, 1, 237, "Copy"),
				Sdm_True);

    os->command = _copy_move_button;

    copy_to->add(_copy_inbox);
    copy_to->add(_copy_move_button);
    copy_to->add(_copy_other_mboxes);
    copy_to->add(_separator);

}

void
RoamMenuWindow::construct_message_menu()
{
//     FORCE_SEGV_DECL(CmdList, cmdList);
    
    _open_msg		= new OpenMsgCmd(
				"Open",
				catgets(DT_catd, 1, 30, "Open"), 
				Sdm_True, this);
    
    _edit_copy = new EditCopyCmd(
			"Copy Selected Text",
			catgets(DT_catd, 1, 43, "Copy Selected Text"), 
			Sdm_True, 
			this
		     );

    _select_all = new EditSelectAllCmd(
			"Select All Text",
			catgets(DT_catd, 1, 44, "Select All Text"), 
			Sdm_True,
			this
		    );

    _save_msg_as	= new SaveAsTextCmd (
				"Save As Text...",
				catgets(DT_catd, 1, 31, "Save As Text..."),
				catgets(DT_catd, 1, 126, "Mailer - Save As Text"),
				Sdm_True,
				get_editor(),
				NULL,
				this->baseWidget());

    construct_attachment_menu();

    _copy_to = new CmdList("Copy To Mailbox",
			   catgets(DT_catd, 1, 33, "Copy To Mailbox"));

    createCopyList(_copy_to);

    _print_msg		= new PrintCmd(
				"Print...",
				catgets(DT_catd, 1, 34, "Print..."), 
				Sdm_True, Sdm_False, this);

    _print		= new PrintCmd(
				"Print One",
				catgets(DT_catd, 1, 13, "Print One"), 
				Sdm_True, Sdm_True, this);

    _find		= new FindCmd (
				"Search...",
 				catgets(DT_catd, 1, 35, "Search..."), 
 				Sdm_True, this );

    _delete_msg		= new DeleteCmd(
				"Delete",
				catgets(DT_catd, 1, 37, "Delete"),
				Sdm_True, this);

    _undeleteLast       = new UndeleteCmd ( 
				"Undelete Last",
				catgets(DT_catd, 1, 38, "Undelete Last"), 
				Sdm_True, this, Sdm_False );

    _undeleteFromList   = new UndeleteCmd(
				"Undelete From List...",
				catgets(DT_catd, 1, 39, 
					"Undelete From List..."), 
				Sdm_True, this, Sdm_True);

    // Message Menu
    
    _message_cmdList = new CmdList( "Message", catgets(DT_catd, 1, 40, "Message"));

    _message_cmdList->add(_open_msg);
    _message_cmdList->add(_edit_copy);
    _message_cmdList->add(_select_all);
    _message_cmdList->add(_save_msg_as);
    _message_cmdList->add(_separator);
    _message_cmdList->add(_attachmentMenuList);
    _message_cmdList->add(_separator);
    _message_cmdList->add(_copy_to);
    _message_cmdList->add(_print);
    _message_cmdList->add(_print_msg);
    _message_cmdList->add(_find);
    _message_cmdList->add(_separator);
    _message_cmdList->add(_delete_msg);
    _message_cmdList->add (_undeleteLast);
    _message_cmdList->add (_undeleteFromList);

    // We only delay the menu creation at RoamMenuWindow startup
    if (_isstartup == Sdm_False) {
      _menuBar->addCommands (&_message_cascade, _message_cmdList, Sdm_False, XmMENU_BAR);
      setup_copy_menu(this);
    } else {
      // IMPORTANT!! The following callback must be registered so that we can
      // access the widgets associated with the _copy_to object.  We cannot access
      // them now because the work procs installed by MenuBar::createPulldown()
      // that will create them have not run yet.
      _copy_to->addMenuCompleteCallback(&RoamMenuWindow::setup_copy_menu, this);
      // IMPORTANT!!  MenuBar::addCommands will return without actually having created
      // the various widgets that make up the menu we are attempting to build.  By passing TRUE
      // for the last argument we are asking the MenuBar object to delay creation of
      // items on the menu as well as submenus until after the GUI is up and running.
      // The MenuBar object achieves this by using Xt work procedures (XtAppAddWorkProc).
      _menuBar->addCommands (&_message_cascade,
			     _message_cmdList,
			     Sdm_False,
			     XmMENU_BAR,
			     Sdm_True);  // use XtWorkProc
    }
}

void
RoamMenuWindow::setup_copy_menu(void *client_data)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *)client_data;

    rmw->_attachmentMenuPR = new MenuBar("attachmentPR",
			 rmw->_attachmentMenuList->getPaneWidget());

    rmw->_copyMenu = rmw->_copy_to->getPaneWidget();
    Widget cascade = rmw->_copy_to->getCascadeWidget();

    XtVaSetValues(rmw->_copyMenu, 
 		  XmNpacking, XmPACK_COLUMN, 
 		  XmNorientation, XmVERTICAL,
 		  NULL);

    XtVaSetValues(cascade, XmNuserData, rmw->_copyMenu, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, NULL);

    // For the dynamic pullright menus from the Other Mailboxes...
    cascade = rmw->_copy_other_mboxes->getCascadeWidget();
    XtVaSetValues(cascade, XmNuserData, rmw, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, 
		rmw->_copy_other_mboxes);
}

void
RoamMenuWindow::construct_message_popup(void)
{
    SdmError error;

   _msgsPopupMenuList = new CmdList( "MsgsPopup", "MsgsPopup");

   _msgs_popup_title 	= new LabelCmd (
			"Mailer - Messages",
                        catgets(DT_catd, 1, 42, "Mailer - Messages"), Sdm_True);

   _msgsPopupMenuList->add(_msgs_popup_title);
   _msgsPopupMenuList->add(_separator);
   _msgsPopupMenuList->add(_delete_msg);
   _msgsPopupMenuList->add(_undeleteLast);
   _msgsPopupMenuList->add(_print_msg);
   _msgsPopupMenuList->add(_replySender);

   _move_popup_menu_cmdList = new CmdList( "Move", catgets(DT_catd, 1, 90, "Move"));
   _move_popup_other_mboxes = construct_move_menu_cmdlist(_move_popup_menu_cmdList, Sdm_False);

   _msgsPopupMenuList->add(_move_popup_menu_cmdList);
   _msgsPopupMenuList->add(_save_msg_as);
   _msgsPopupMenuList->add(_check_new_mail);

   Widget parent = XtParent(_list->get_scrolling_list());
   _menuPopupMsgs = new MenuBar(parent, "RoamMsgsPopup", XmMENU_POPUP);

   // We only delay the menu creation at RoamMenuWindow startup
   if (_isstartup == Sdm_False) {
     _msgsPopupMenu = _menuPopupMsgs->addCommands(_msgsPopupMenuList,
						  Sdm_False, XmMENU_POPUP);
     setup_move_popup_menu(this);
     setup_move_popup_other_menu(this);
   } else {
     // IMPORTANT!! The following callbacks must be registered so that we can
     // access the widgets associated with the CmdList objects.  We cannot access
     // them now because the work procs installed by MenuBar::createPulldown()
     // that will create the widgets have not run yet.
     _move_popup_menu_cmdList->addMenuCompleteCallback(
		&RoamMenuWindow::setup_move_popup_menu, this);
     _move_popup_other_mboxes->addMenuCompleteCallback(
		&RoamMenuWindow::setup_move_popup_other_menu, this);

     // IMPORTANT!!  MenuBar::addCommands will return without actually having created
     // the various widgets that make up the menu we are attempting to build.  By passing TRUE
     // for the last argument we are asking the MenuBar object to delay creation of
     // items on the menu as well as submenus until after the GUI is up and running.
     // The MenuBar object achieves this by using Xt work procedures (XtAppAddWorkProc).
     _msgsPopupMenu = _menuPopupMsgs->addCommands(_msgsPopupMenuList, 
						  Sdm_False,
						  XmMENU_POPUP,
						  Sdm_True);  // use XtWorkProc
   }
   XtAddEventHandler(parent, ButtonPressMask,
		     Sdm_False, MenuButtonHandler, (XtPointer) this);
}

void
RoamMenuWindow::setup_move_popup_other_menu(void *client_data)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *)client_data;

    Widget cascade = rmw->_move_popup_other_mboxes->getCascadeWidget();
    XtVaSetValues(cascade, XmNuserData, rmw, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, 
		  rmw->_move_popup_other_mboxes);
}

void
RoamMenuWindow::setup_move_popup_menu(void *client_data)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *)client_data;

    rmw->_msgsPopupMoveMenu = rmw->_move_popup_menu_cmdList->getPaneWidget();
    Widget cascade = rmw->_move_popup_menu_cmdList->getCascadeWidget();

    XtVaSetValues(rmw->_msgsPopupMoveMenu, 
		  XmNpacking, XmPACK_COLUMN, 
		  XmNorientation, XmVERTICAL,
		  NULL);

    XtVaSetValues(cascade, XmNuserData, rmw->_msgsPopupMoveMenu, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, NULL);
    if (!rmw->_is_writable) {
	XtSetSensitive(cascade, Sdm_False);
    }
    XtVaSetValues(rmw->_msgsPopupMoveMenu, XmNuserData, rmw, NULL);
}

void
RoamMenuWindow::construct_text_popup(void)
{
   if (theApplication->bMenuButton() != Button3)
       return;

   _textPopupMenuList = new CmdList( "TextPopup", "TextPopup");

   _text_popup_title     = new LabelCmd (
			"Mailer - Text",
			catgets(DT_catd, 1, 46, "Mailer - Text"), Sdm_True);

   _textPopupMenuList->add(_text_popup_title);
   _textPopupMenuList->add(_separator);
   _textPopupMenuList->add(_edit_copy);
   _textPopupMenuList->add(_select_all);

   Widget parent = my_editor->textEditor()->get_editor();
   _menuPopupText = new MenuBar(parent, "RoamTextPopup", XmMENU_POPUP);

    // IMPORTANT!! The following callback must be registered so that we can
    // access the widgets associated with the _textPopupMenuList object.  We cannot access
    // them now because the work procs installed by MenuBar::createPulldown()
    // that will create the widgets have not run yet.
   _textPopupMenuList->addMenuCompleteCallback(&RoamMenuWindow::setup_text_popup_menu,
					       this);
    // IMPORTANT!!  MenuBar::addCommands will return without actually having created
    // the various widgets that make up the menu we are attempting to build.  By passing TRUE
    // for the last argument we are asking the MenuBar object to delay creation of
    // items on the menu as well as submenus until after the GUI is up and running.
    // The MenuBar object achieves this by using Xt work procedures (XtAppAddWorkProc).
   _textPopupMenu = _menuPopupText->addCommands(_textPopupMenuList, 
						Sdm_False,
						XmMENU_POPUP,
						Sdm_True);  // use XtWorkProc
}

void
RoamMenuWindow::setup_text_popup_menu(void *client_data)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *)client_data;
    Widget parent = rmw->my_editor->textEditor()->get_editor();

    DtEditorCreateExtrasMenu(parent, rmw->_textPopupMenu);
}

void RoamMenuWindow::construct_view_menu()
{
//     FORCE_SEGV_DECL(CmdList, cmdList);

    _next		= new NextCmd ( 
				"Next",
				catgets(DT_catd, 1, 47, "Next"), Sdm_True, this );
    _previous		= new PrevCmd ( 
				"Previous",
				catgets(DT_catd, 1, 48, "Previous"), Sdm_True, this );
    
    _abbrevHeaders = new AbbrevHeadersCmd(
				"Abbreviated Headers",
				catgets(DT_catd, 1, 49, "Abbreviated Headers"), 
				this);

    // I18N - add Change Charset Command
    _charset = new GetPossibleCharsetsCmd(
				"Change Char Set",
				catgets(DT_catd, 1, 316, "Change Char Set"), 
				Sdm_True,
				this,
				Sdm_True );


    _sortTD = new SortCmd ("By Date/Time", 
				catgets(DT_catd, 1, 50, "By Date/Time"),
				Sdm_True,
				this,
				SortTimeDate);
    _sortSender = new SortCmd ("By Sender",
				catgets(DT_catd, 1, 51, "By Sender"),
				Sdm_True,
				this,
				SortSender);
    _sortSubject = new SortCmd ("By Subject",
				catgets(DT_catd, 1, 52, "By Subject"),
				Sdm_True,
				this,
				SortSubject);
    _sortSize = new SortCmd ("By Size",
				catgets(DT_catd, 1, 53, "By Size"),
				Sdm_True,
				this,
				SortSize);
    _sortStatus = new SortCmd ("By Status",
				catgets(DT_catd, 1, 54, "By Status"),
				Sdm_True,
				this,
				SortStatus);

    // View Menu
    
    _view_cmdList = new CmdList( "View", catgets(DT_catd, 1, 55, "View"));
    
    _view_cmdList->add ( _next );
    _view_cmdList->add ( _previous );
    _view_cmdList->add ( _separator );
    _view_cmdList->add( _abbrevHeaders);
    _view_cmdList->add( _charset);
    _view_cmdList->add ( _separator );
    _view_cmdList->add ( _sortTD );
    _view_cmdList->add ( _sortSender );
    _view_cmdList->add ( _sortSubject );
    _view_cmdList->add ( _sortSize );
    _view_cmdList->add ( _sortStatus );

    // IMPORTANT!! The following callback must be registered so that we can
    // access the widgets associated with the _textPopupMenuList object.  We cannot access
    // them now because the work procs installed by MenuBar::createPulldown()
    // that will create the widgets have not run yet.
//     _view_cmdList->addMenuCompleteCallback(&RoamMenuWindow::setup_view_menu, (void *)cmdList);

    // IMPORTANT!!  MenuBar::addCommands will return without actually having created
    // the various widgets that make up the menu we are attempting to build.  By passing TRUE
    // for the last argument we are asking the MenuBar object to delay creation of
    // items on the menu as well as submenus until after the GUI is up and running.
    // The MenuBar object achieves this by using Xt work procedures (XtAppAddWorkProc).
    _menuBar->addCommands (_view_cmdList,
			   Sdm_False,
			   XmMENU_BAR,
			   Sdm_True);  // use XtWorkProc
}

// void
// RoamMenuWindow::setup_view_menu(void *client_data)
// {
//     CmdList *cmdlist = (CmdList *)client_data;
//     cmdlist->deleteMe();
// }

void
RoamMenuWindow::construct_compose_menu()
{
//   FORCE_SEGV_DECL(CmdList, cmdList);
  char *value;
  SdmError error;

  if (!_comp_new)
    _comp_new	= new ComposeCmd ( 
                                  "New Message",
                                  catgets(DT_catd, 1, 245, "New Message"), 
                                  Sdm_True, 
                                  this );
                                  
  if (_templates != NULL) {
    _templates->deleteMe();
  }
  _templates = new CmdList ("New, Use Template", 
                            catgets(DT_catd, 1, 244, "New, Use Template"));


  SdmMailRc * mailRc;
  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
  assert(!error);
  mailRc->GetValue(error, "templates", &value);
  if (!error && *value != NULL) {
    DtVirtArray<PropStringPair *> templates(8);
    parsePropString(value, templates);
    int tcount = templates.length();
    for (int i = 0; i < tcount; i++) {
      PropStringPair * psp = templates[i];
      if (psp->label && psp->value) {
        Cmd * button = new LoadFileCmd(psp->label,
                                       psp->label, 1, (SendMsgDialog *) NULL, 
                                       psp->value, TYPE_TEMPLATE);
        _templates->insert(button);
      }
    }    
    for (i=0; i < tcount; i++) {
      PropStringPair *psp = templates[0];
      delete psp;
      templates.remove(0);
    }
  }
  else {
    Cmd * button = new LoadFileCmd("No Templates",
                                   catgets(DT_catd, 3, 114, "No Templates"), 
                                   1, (SendMsgDialog *) NULL, 
                                   NULL, TYPE_TEMPLATE);
    _templates->insert(button);
  }
  if (value != NULL)
    free(value);

  if (!_forward)
    _forward	= new ForwardCmd ( 
                                  "Forward",
                                  catgets(DT_catd, 1, 58, "Forward"), 
                                  Sdm_True, 
                                  this, 
                                  Sdm_True );

  if (!_forwardBody)
    _forwardBody = new ForwardCmd ("Forward, no Attachments",
			   catgets(DT_catd, 1, 271, "Forward, no Attachments"), 
			   Sdm_True, 
			   this, 
			   Sdm_False );

  if (!_replySender)
    _replySender = new ReplyCmd ( 
                                 "Reply",
                                 catgets(DT_catd, 1, 59, "Reply"), 
                                 Sdm_True, 
                                 this, 
                                 Sdm_False );

  if (!_replyAll)
    _replyAll	= new ReplyAllCmd ( 
                                   "Reply to All",
                                   catgets(DT_catd, 1, 60, "Reply to All"), 
                                   Sdm_True, 
                                   this, 
                                   Sdm_False );

  if (!_replySinclude)
    _replySinclude= new ReplyCmd ( 
                                  "Reply, Include Message",
                                  catgets(DT_catd, 1, 247, "Reply, Include Message"), 
                                  Sdm_True, 
                                  this, 
                                  Sdm_True );	

  if (!_replyAinclude)
    _replyAinclude= new ReplyAllCmd ( 
                                     "Reply to All, Include Message",
                                     catgets(DT_catd, 1, 248, "Reply to All, Include Message"), 
                                     Sdm_True, 
                                     this, 
                                     Sdm_True );	
  if (_drafts) {
    if (_drafts->getClientData() != NULL) {
      RAMenuClientData* mcd = (RAMenuClientData*) _drafts->getClientData();
      for (int i=0; i<2; i++) {
        if (mcd[i].filename != NULL) 
          free (mcd[i].filename);
      }
      free (mcd);
    }
    _drafts->deleteMe();
  }
  _drafts = new CmdList ("Continue Composing Draft", 
                         catgets(DT_catd, 1, 286, "Continue Composing Draft"));

  // Compose Menu
    
  _compose_cmdList = new CmdList( "Compose", catgets(DT_catd, 1, 63, "Compose"));
  _compose_cmdList->add ( _comp_new );
  _compose_cmdList->add (_templates);
  _compose_cmdList->add(_separator);
  _compose_cmdList->add (_forward);
  _compose_cmdList->add (_forwardBody);
  _compose_cmdList->add(_separator);
  _compose_cmdList->add ( _replySender );
  _compose_cmdList->add ( _replyAll );
  _compose_cmdList->add ( _replySinclude );
  _compose_cmdList->add ( _replyAinclude );
  _compose_cmdList->add(_separator);
  _compose_cmdList->add ( _drafts );

  // IMPORTANT!! The following callback must be registered so that we can
  // access the widgets associated with the _templates and _drafts objects.  We cannot
  // access them now because the work procs installed by MenuBar::createPulldown()
  // that will create the widgets have not run yet.
  _templates->addMenuCompleteCallback(&RoamMenuWindow::setup_template_menu, this);
  _drafts->addMenuCompleteCallback(&RoamMenuWindow::setup_draft_menu,
				   this);

    // IMPORTANT!!  MenuBar::addCommands will return without actually having created
    // the various widgets that make up the menu we are attempting to build.  By passing TRUE
    // for the last argument we are asking the MenuBar object to delay creation of
    // items on the menu as well as submenus until after the GUI is up and running.
    // The MenuBar object achieves this by using Xt work procedures (XtAppAddWorkProc).
  _menuBar->addCommands (&_compose_cascade,
			 _compose_cmdList,
			 Sdm_False,
			 XmMENU_BAR,
			 Sdm_True);  // use XtWorkProc

  RAMenuClientData *mcd =
      (RAMenuClientData*) malloc (2 * sizeof(RAMenuClientData));
  mcd->timestamp = 0;
  mcd->filename = theRoamApp.getResource(DraftDir, Sdm_True);
  mcd->smd = NULL;
  (mcd+1)->timestamp = 0;
  (mcd+1)->filename = theRoamApp.getResource(DeadletterDir, Sdm_True);
  (mcd+1)->smd = NULL;
  _drafts->setClientData((void*)mcd);
}

void
RoamMenuWindow::setup_template_menu(void *client_data)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *)client_data;

    Widget menu = rmw->_templates->getPaneWidget();
    XtVaSetValues(menu,
		  XmNpacking, XmPACK_COLUMN,
		  XmNorientation, XmVERTICAL,
		  NULL);

    Widget cascade = rmw->_templates->getCascadeWidget();
    XtVaSetValues(cascade, XmNuserData, menu, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, NULL);
}

void
RoamMenuWindow::setup_draft_menu(void *client_data)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *)client_data;

    Widget menu = rmw->_drafts->getPaneWidget();
    XtVaSetValues(menu,
		  XmNpacking, XmPACK_COLUMN,
		  XmNorientation, XmVERTICAL,
		  NULL);

    XtVaSetValues(menu, XmNuserData, rmw, NULL);
    XtAddCallback(menu, XmNmapCallback, &RoamApp::map_menu, rmw->_drafts);
}

void
RoamMenuWindow::addFoldersFiles(Widget menu, CmdList *currentCmdList)
{
  CmdList *folderCmdlist;
  Cmd *cmd;
  struct dirent *direntp;
  SdmError error;
  struct stat fstat;
  char filename[1024];
  MenuClientData *mcd = (MenuClientData *) currentCmdList->getClientData();

  if (mcd == NULL)
    return;

  stat(mcd->filename, &fstat);

  // Nothing has changed...
  if (fstat.st_mtime == mcd->timestamp) 
    return;

  DIR *dirp = opendir(mcd->filename);
  if (dirp == NULL) {
    Cardinal num_children=0;
    XtVaGetValues(menu,
        XmNnumChildren, &num_children,
        NULL);

    // if there is nothing currently in the menu,
    // add the "Empty Folder" button to the menu.
    if (num_children == 0) {
      cmd = new MoveCopyMenuCmd ("Empty Folder",
                               catgets(DT_catd, 1, 285, "Empty Folder"), 0, this, 
                               mcd->op);
      currentCmdList->add(cmd);
      MenuBar *mb = new MenuBar("menuPR", menu);
      mb->addCommands(currentCmdList, Sdm_False, XmMENU_PULLDOWN);
      XtVaSetValues(menu, XmNpacking, XmPACK_COLUMN,
                XmNorientation, XmVERTICAL,
                NULL);
    }

    // Can't do anything further without a directory ptr.
    return; 
  }

  int sum=0, i, j;
  while ((direntp = readdir(dirp)) != NULL) {
    // Don't include the files we don't show.
    if (direntp->d_ino == 0 || direntp->d_name[0] == '.')
      continue;
    // Count up the int values of the filenames...
    for (i = 0, j = strlen(direntp->d_name); i < j; i++) 
      sum += (int)direntp->d_name[i];
  }
  // Timestamp changed but the directory structure stayed the same.
  if (sum == mcd->checksum) {
    closedir(dirp);
    mcd->timestamp = fstat.st_mtime;
    return;
  }

  // Checksum's were different.
  mcd->checksum = sum;
  rewinddir(dirp);

  MenuBar *mb = new MenuBar("menuPR", menu);

  // Menu was mapped with commands already so remove them 
  if (mcd->timestamp != 0) {
    mb->removeCommands(menu, currentCmdList); 
    while (currentCmdList->size()) 
      currentCmdList->remove(0);
  }
  mcd->timestamp = fstat.st_mtime;

  filename [0] = '\0';
  if (dirp) {
    while ((direntp = readdir(dirp)) != NULL) {
      // Dont show dot files
      if (direntp->d_ino == 0 || direntp->d_name[0] == '.') 
        continue;
      sprintf(filename, "%s/%s", mcd->filename, direntp->d_name);
      stat(filename, &fstat);
      // It's a directory...Just create the cmdlist. We create the 
      // menu buttons dynamically in map_menu()
      if ((fstat.st_mode & S_IFMT) == S_IFDIR) {
        folderCmdlist = new CmdList (direntp->d_name, direntp->d_name);
        MenuClientData *mcd2 = (MenuClientData*) 
          malloc (sizeof(MenuClientData));
        mcd2->filename = strdup_n(filename);
        mcd2->op = mcd->op;
        mcd2->checksum = -1;
        mcd2->timestamp = 0;
        folderCmdlist->setClientData((void*)mcd2);
        currentCmdList->collInsert(folderCmdlist);
      }
      // It's a file...
      else {
        if (mcd->op == DTM_OPEN)
          cmd = new OpenMailboxCmd(direntp->d_name,
                                   direntp->d_name,
                                   Sdm_True,
                                   this,
                                   filename);
        else // Move or Copy...
          cmd = new MoveCopyMenuCmd (filename, direntp->d_name, 1, this, mcd->op);
        currentCmdList->collInsert(cmd);
      }
    }
    (void)closedir( dirp );
  }
  // Empty Directory...
  if (filename[0] == '\0') {
    cmd = new MoveCopyMenuCmd ("Empty Folder",
                               catgets(DT_catd, 1, 285, "Empty Folder"), 0, this, 
                               mcd->op);
    currentCmdList->add(cmd);
  }
  mb->addCommands(currentCmdList, Sdm_False, XmMENU_PULLDOWN);

  XtVaSetValues(menu, XmNpacking, XmPACK_COLUMN,
                XmNorientation, XmVERTICAL,
                NULL);
}

void
RoamMenuWindow::construct_options_menu()
{
//     FORCE_SEGV_DECL(CmdList, cmdList);

    _option_cmdList = new CmdList( "Options", catgets(DT_catd, 1, 260, "Options"));

    _opt_basic = new OptionsCmd(
			"Basic",
			catgets(DT_catd, 1, 261, "Basic..."),
                        1,
			this->baseWidget(),
			OPT_BASIC);
    _option_cmdList->add(_opt_basic);

    _option_cmdList->add(_separator);

    _opt_al = new OptionsCmd(
			"Aliases",
			catgets(DT_catd, 1, 269, "Aliases..."),
                        1,
			this->baseWidget(),
			OPT_AL);
    _option_cmdList->add(_opt_al);

    _opt_cw = new OptionsCmd(
			"Compose Window",
			catgets(DT_catd, 1, 265, "Compose Window..."),
                        1,
			this->baseWidget(),
			OPT_CW);
    _option_cmdList->add(_opt_cw);


    _opt_mv = new OptionsCmd(
			"Message View",
			catgets(DT_catd, 1, 263, "Message View..."),
                        1,
			this->baseWidget(),
			OPT_MV);
    _option_cmdList->add(_opt_mv);

    _opt_mf = new OptionsCmd(
			"Move Menu Setup",
			catgets(DT_catd, 1, 264, "Move Menu Setup..."),
                        1,
			this->baseWidget(),
			OPT_MF);
    _option_cmdList->add(_opt_mf);

    _option_cmdList->add(_separator);

// AUTOFILING change this back to Filing and Notification when we put back the changes
    _opt_no = new OptionsCmd(
			"Notification",
			catgets(DT_catd, 1, 327, "Notification..."),
                        1,
			this->baseWidget(),
			OPT_NO);
    _option_cmdList->add(_opt_no);

    _opt_sig = new OptionsCmd(
			"Signature",
			catgets(DT_catd, 1, 266, "Signature..."),
                        1,
			this->baseWidget(),
			OPT_SIG);
    _option_cmdList->add(_opt_sig);

    _opt_td = new OptionsCmd(
			"Templates",
			catgets(DT_catd, 1, 268, "Templates..."),
                        1,
			this->baseWidget(),
			OPT_TD);
    _option_cmdList->add(_opt_td);

    _opt_tb = new OptionsCmd(
			"Toolbar",
			catgets(DT_catd, 1, 262, "Toolbar..."),
                        1,
			this->baseWidget(),
			OPT_TB);
    _option_cmdList->add(_opt_tb);

    _opt_vac = new OptionsCmd(
			"Vacation Message",
			catgets(DT_catd, 1, 267, "Vacation Message..."),
                        1,
			this->baseWidget(),
			OPT_VAC);
    _option_cmdList->add(_opt_vac);

    _option_cmdList->add(_separator);

    _opt_ad = new OptionsCmd(
			"Advanced",
			catgets(DT_catd, 1, 270, "Advanced..."),
                        1,
			this->baseWidget(),
			OPT_AD);
    _option_cmdList->add(_opt_ad);

    // IMPORTANT!! The following callback must be registered so that we can
    // access the widgets associated with the cmdList object.  We cannot
    // access them now because the work procs installed by MenuBar::createPulldown()
    // that will create the widgets have not run yet.
//     cmdList->addMenuCompleteCallback(&RoamMenuWindow::setup_option_menu, cmdList);

    // IMPORTANT!!  MenuBar::addCommands will return without actually having created
    // the various widgets that make up the menu we are attempting to build.  By passing TRUE
    // for the last argument we are asking the MenuBar object to delay creation of
    // items on the menu as well as submenus until after the GUI is up and running.
    // The MenuBar object achieves this by using Xt work procedures (XtAppAddWorkProc).
    _menuBar->addCommands(_option_cmdList,
			  Sdm_False,
			  XmMENU_BAR,
			  Sdm_True);  // use XtWorkProc
}

// void
// RoamMenuWindow::setup_option_menu(void *client_data)
// {
//     CmdList *cmdlist = (CmdList *)client_data;
//     cmdlist->deleteMe();
// }

CmdList*
RoamMenuWindow::construct_move_menu_cmdlist(CmdList *list, SdmBoolean append)
{
    SdmError error;
    char *val=NULL;
    MoveCopyMenuCmd *move_container;
    char *start_ptr, *p, *ptmp, *serverTypePtr;
    char buf[2048];
    SdmBoolean addedEntries = Sdm_False;

    // Get names for permanent containers from .mailrc and fill in the menu.
    // We get them from the "movemenu" variable, and we no longer read
    // "filemenu", which was used by V2 mailtool and kept along in V3
    // mailtool for backward compatability.
    SdmMailRc * mailRc;
    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
    assert(!error);

    mailRc->GetValue(error, "movemenu", &val);
    error.Reset();

    if (val) {
	// Create arrays to hold the user defined container list and the
	// recently visited (cached) container list.

        addedEntries = Sdm_True;
	start_ptr = val;
	for (p = val; p && *p; p++) {
	    if ( isspace(*p) && !isspace(*(p-1)) && *(p-1) != '\\' ) {
		strncpy(buf, start_ptr, p - start_ptr);
		buf[p - start_ptr] = '\0';
		for (ptmp = strchr(buf, '\\'); ptmp; ptmp = strchr(buf, '\\')) {
		    for (; ptmp && *ptmp; ptmp++)
			*ptmp = *(ptmp + 1);
		}

		//theRoamApp.fromTokenToLabel(buf);
		if ( (serverTypePtr = strstr(buf, ",imap")) != NULL ) {
		    *serverTypePtr = ')';
		    *(serverTypePtr + 1) = '\0';
		    if ( (serverTypePtr = strrchr(buf, ',')) != NULL ) {
			*serverTypePtr = '(';
		    }
		}

		// Pass the final path name into ExpandPath so all of the Unix-isms get expanded
		//
		char *expanded_name = NULL;
		error.Reset();
		SdmUtility::ExpandPath(error, expanded_name, buf, *mailRc, SdmUtility::kFolderResourceName);
		char *bufToUse = ((error == Sdm_EC_Success && expanded_name != NULL) ? expanded_name : buf);

		move_container = new MoveCopyMenuCmd(bufToUse, bufToUse,
						Sdm_True, this, DTM_MOVE);
		list->add(move_container);
		if (append) {
			_move_containerlist->append(move_container);
			_user_list_size++;
		}
		_movecopymenucmd_list->append(move_container);

		// Free up any storage used by the result of ExpandPath being called
		//
		if (expanded_name) {
		  free(expanded_name);
		  expanded_name = NULL;
		}

		start_ptr = p + 1;
	    }
	}
    }
    if (val) {
      free(val);	
      val = NULL;
    }

    mailRc->GetValue(error, "filemenu2", &val);
    if (!error && val != NULL && *val != '\0') {

        addedEntries = Sdm_True;
	// Create arrays to hold the user defined container list and the
	// recently visited (cached) container list.
	start_ptr = val;
	for (p = val; p && *p; p++) {
	    if ( isspace(*p) && !isspace(*(p-1)) && *(p-1) != '\\' ) {
		strncpy(buf, start_ptr, p - start_ptr);
		buf[p - start_ptr] = '\0';
		for (ptmp = strchr(buf, '\\'); ptmp; ptmp = strchr(buf, '\\')) {
		    for (; ptmp && *ptmp; ptmp++)
			*ptmp = *(ptmp + 1);
		}

		//theRoamApp.fromTokenToLabel(buf);
		if ( (serverTypePtr = strstr(buf, ",imap")) != NULL ) {
		    *serverTypePtr = ')';
		    *(serverTypePtr + 1) = '\0';
		    if ( (serverTypePtr = strrchr(buf, ',')) != NULL ) {
			*serverTypePtr = '(';
		    }
		}

		// Pass the final path name into ExpandPath so all of the Unix-isms get expanded
		//
		char *expanded_name = NULL;
		error.Reset();
		SdmUtility::ExpandPath(error, expanded_name, buf, *mailRc, SdmUtility::kFolderResourceName);
		char *bufToUse = ((error == Sdm_EC_Success && expanded_name != NULL) ? expanded_name : buf);

		if (notInList(bufToUse, list)) {
			move_container = new MoveCopyMenuCmd(bufToUse, bufToUse,
						Sdm_True, this, DTM_MOVE);
			list->add(move_container);
			if (append) {
				_move_containerlist->append(move_container);
				_user_list_size++;
			}
			_movecopymenucmd_list->append(move_container);
		}

		// Free up any storage used by the result of ExpandPath being called
		//
		if (expanded_name) {
		  free(expanded_name);
		  expanded_name = NULL;
		}

		start_ptr = p + 1;
	    }
	}
    }
    if (val) {
      free(val);
      val = NULL;
    }

    // Move menu
    if (addedEntries) {
	list->add(_separator);
    }

    if (!_move_inbox)
    	_move_inbox = new MoveToInboxCmd(
                  "Inbox",
       		  catgets(DT_catd, 1, 221, "Inbox"),
 		  Sdm_True, 
  		  this);

    list->add(_move_inbox);

    CmdList *move_other_mboxes = new CmdList ( "Move to Other Mailboxes",
		       catgets(DT_catd, 1, 320, "Move to Other Mailboxes"));

    // This is for mapping the Move pullright menus dynamically...
    MenuClientData *mcd = (MenuClientData*) malloc (sizeof(MenuClientData));

    mcd->filename = theRoamApp.getFolderDir(local, Sdm_True);
    mcd->op = DTM_MOVE;
    mcd->checksum = -1;
    mcd->timestamp = 0;
    move_other_mboxes->setClientData((void*)mcd);

    struct OpenStruct *os = (struct OpenStruct *) malloc(sizeof(struct OpenStruct));
    os->rmw = this;

    _move_copy_button = new MoveCopyCmd (
                                "Select Mailbox",
                                catgets(DT_catd, 1, 65, "Select Mailbox..."),
                                Sdm_True,
                                RoamMenuWindow::move_callback,
                                RoamMenuWindow::copy_callback,
                                CustomSelectFileCmd::doDirSearch,
                                os,
                                this->baseWidget(),
				catgets(DT_catd, 1, 90, "Move"),
				Sdm_False);

    os->command = _move_copy_button;

    list->add(_move_copy_button);
    list->add(move_other_mboxes);
    list->add(_separator);

    return move_other_mboxes;
}

// construct_move_menu() builds the Move menu on the menu bar.  
// There is a user defined set of items at the top, the Inbox,
// the "Mail Filing..." item, and a dynamic list of most recently
// accessed containers at the bottom.
void
RoamMenuWindow::construct_move_menu()
{
    char *val = NULL;
    SdmError error;

    // We will use _containerlist and cached_containerlist to keep
    // track of the commands associated with the Move menu items.
    if (_move_containerlist != NULL) 
      delete (_move_containerlist);

    _move_containerlist = new DtVirtArray<MoveCopyMenuCmd *> (3);

    if (_cached_move_containerlist != NULL)
        delete _cached_move_containerlist;
    _cached_move_containerlist = new DtVirtArray<MoveCopyMenuCmd *>
		(_max_cached_list_size ? _max_cached_list_size : 1 );
    
    if (_move_menu_cmdList != NULL) {
      _move_menu_cmdList->deleteMe();
    }
    _move_menu_cmdList = new CmdList( "Move", catgets(DT_catd, 1, 64, "Move"));

    _move_other_mboxes = construct_move_menu_cmdlist(_move_menu_cmdList, Sdm_True);

    // We only delay the menu creation at RoamMenuWindow startup
    if (_isstartup == Sdm_False) {
      _moveMenu = _menuBar->addCommands(&_move_cascade, _move_menu_cmdList,
					Sdm_False, XmMENU_BAR);
      setup_move_menu(this);
    } else {
      // IMPORTANT!! The following callback must be registered so that we can
      // access the widgets associated with the _move_menu_cmdList object.  We cannot access
      // them now because the work procs installed by MenuBar::createPulldown()
      // that will create the widgets have not run yet.
      _move_other_mboxes->addMenuCompleteCallback(
			&RoamMenuWindow::setup_move_menu, this);

      // IMPORTANT!!  MenuBar::addCommands will return without actually having created
      // the various widgets that make up the menu we are attempting to build.  By passing TRUE
      // for the last argument we are asking the MenuBar object to delay creation of
      // items on the menu as well as submenus until after the GUI is up and running.
      // The MenuBar object achieves this by using Xt work procedures (XtAppAddWorkProc).
      _moveMenu = _menuBar->addCommands(&_move_cascade,
					_move_menu_cmdList,
					Sdm_False,
					XmMENU_BAR,
					Sdm_True);	// use XtWorkProc
    }
}

void
RoamMenuWindow::setup_move_menu(void *client_data)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *)client_data;

    Widget cascade = rmw->_move_other_mboxes->getCascadeWidget();
    XtVaSetValues(cascade, XmNuserData, rmw, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, 
		  rmw->_move_other_mboxes);

    XtVaSetValues(rmw->_moveMenu, 
 		  XmNpacking, XmPACK_COLUMN, 
 		  XmNorientation, XmVERTICAL,
 		  NULL);

    XtVaSetValues(rmw->_move_cascade, XmNuserData, rmw->_moveMenu, NULL);
    XtAddCallback(rmw->_move_cascade, XmNcascadingCallback,
		  &RoamMenuWindow::map_menu, NULL);
    if (!rmw->_is_writable) {
	XtSetSensitive(rmw->_move_cascade, Sdm_False);
    }
}

void
RoamMenuWindow::construct_attachment_menu()
{

    _save_attachment	= new SaveAttachCmd (
				"Save As...",
				catgets(DT_catd, 1, 66, "Save As..."),
				catgets(DT_catd, 1, 67, "Mailer - Attachments - Save As"),
				Sdm_False, 
				RoamMenuWindow::save_attachment_callback,
				this,
				this->baseWidget());

    _select_all_attachments = new SelectAllAttachsCmd(
					"Select All",
					catgets(DT_catd, 1, 68, "Select All"),
					this);

    _attachmentMenuList = new CmdList( "Attachments",
				       catgets(DT_catd, 1, 69, "Attachments"));
    _attachmentMenuList->add(_save_attachment);
    _attachmentMenuList->add(_select_all_attachments);

}

void
RoamMenuWindow::construct_attachment_popup(void)
{
   _attachmentPopupMenuList = new CmdList( "AttachmentsPopup", "AttachmentsPopup");

    _attachment_popup_title     = new LabelCmd (
			"Mailer - Attachments",
			catgets(DT_catd, 1, 70, "Mailer - Attachments"), Sdm_True);

    _attachmentPopupMenuList->add(_attachment_popup_title);
    _attachmentPopupMenuList->add(_separator);
    _attachmentPopupMenuList->add(_save_attachment);
    _attachmentPopupMenuList->add(_select_all_attachments);
    _menuPopupAtt = new MenuBar(my_editor->attachArea()->getClipWindow(), 
					"RoamAttachmentPopup", XmMENU_POPUP);
    _attachmentPopupMenu = _menuPopupAtt->addCommands(_attachmentPopupMenuList, 
						      Sdm_False,
						      XmMENU_POPUP,
						      FALSE);
}

void
RoamMenuWindow::construct_help_menu()
{
//     FORCE_SEGV_DECL(CmdList, cmdList);

    _overview = new OnAppCmd("Overview", catgets(DT_catd, 1, 71, "Overview"), 
				Sdm_True, this);
    _tasks = new TasksCmd("Tasks", catgets(DT_catd, 1, 72, "Tasks"), 
				Sdm_True, this);
    _reference = new ReferenceCmd("Reference", catgets(DT_catd, 1, 73, "Reference"), 
				Sdm_True, this);
    _messages = new MessagesCmd("Messages", catgets(DT_catd, 1, 301, "Messages"), 
				Sdm_True, this);
    _on_item = new OnItemCmd("On Item", catgets(DT_catd, 1, 74, "On Item"), 
				Sdm_True, this);
    _using_help = new UsingHelpCmd("Using Help", catgets(DT_catd, 1, 75, "Using Help"), 
					Sdm_True, this);
    _help_cmdList = new CmdList( "Help", catgets(DT_catd, 1, 76, "Help"));
    _help_cmdList->add ( _overview );
    _help_cmdList->add ( _separator );
    _help_cmdList->add ( _tasks );
    _help_cmdList->add ( _reference );
    _help_cmdList->add ( _messages );
    _help_cmdList->add ( _separator );
    _help_cmdList->add ( _on_item );
    _help_cmdList->add ( _separator );
    _help_cmdList->add ( _using_help );
    _help_cmdList->add ( _separator );

    _about_mailer = new RelNoteCmd("About Mailer...", 
				    catgets(DT_catd, 1, 77, "About Mailer..."),
    				    Sdm_True, this);
    _help_cmdList->add ( _about_mailer );

    // IMPORTANT!! The following callback must be registered so that we can
    // access the widgets associated with the cmdList object.  We cannot
    // access them now because the work procs installed by MenuBar::createPulldown()
    // that will create the widgets have not run yet.
//     cmdList->addMenuCompleteCallback(&RoamMenuWindow::setup_help_menu, cmdList);

    // IMPORTANT!!  MenuBar::addCommands will return without actually having created
    // the various widgets that make up the menu we are attempting to build.  By passing TRUE
    // for the last argument we are asking the MenuBar object to delay creation of
    // items on the menu as well as submenus until after the GUI is up and running.
    // The MenuBar object achieves this by using Xt work procedures (XtAppAddWorkProc).
    _menuBar->addCommands ( _help_cmdList,
			    Sdm_True,
			    XmMENU_BAR,
			    Sdm_True );  // use XtWorkProc
}

// All we need to do currently is delete the CmdList object we created the menu from.
// void RoamMenuWindow::setup_help_menu(void *client_data)
// {
//     CmdList *cmdlist = (CmdList *)client_data;
//     cmdlist->deleteMe();
// }
 
SdmMailRc *
RoamMenuWindow::get_mail_rc()
{
  SdmMailRc *mailRc;
  SdmError error;

  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
  assert(!error);

  return mailRc;
}

void
RoamMenuWindow::load_mailbox(SdmError & mail_error)
{
    int		count = 0;
    
    // If there is no mailbox, return.
    if (!_mailbox) {
	return;
    }

    // Now load the messages into the scrolling list.
    // This will get the DtMailMessageHandles into the _msgs array and
    // it will also get their XmStrings into the CharArray of the _list.
    theRoamApp.busyAllWindows(catgets(DT_catd, 3, 27, "Loading..."));
	
    // Call load_headers() to get the XmStrings into the XmList!
//     count = _list->load_all_headers(mail_error, _initial_nmsgs);
    count = _list->load_headers(mail_error, _initial_nmsgs);
    
    if ( mail_error ) {
      // Return whatever error mailbox->get_next_msg() returned.
      theRoamApp.unbusyAllWindows();
      return;
    } else {
      _is_loaded = Sdm_True;
    }

    // If no messages

    if (count == 0) {
      this->message(catgets(DT_catd, 3, 28, "Empty container"));
      setToEmptyIcon();
    }

    theRoamApp.unbusyAllWindows();
}

void
RoamMenuWindow::text_selected()
{
  // turn on sensitivity for Cut/Clear/Copy/Paste/Delete
}

void
RoamMenuWindow::text_unselected()
{
  // turn off sensitivity for those items
}


// Post the dialog displaying the error text.
// Also display the minor_code, if resource/environment variable
// wants it.
// User has no choice but to OK
// postErrorDialog() is the benign uncle of postFatalErrorDialog() --
// it does not quit the RMW.

void
RoamMenuWindow::postErrorDialog(SdmError &mail_error)
{
  _genDialog->post_error(mail_error);
}


void
RoamMenuWindow::manage()
{
  Dimension win_x, win_y, win_wid, win_ht, win_bwid;

  MainWindow::manage();

  XtVaGetValues(_w,
                XmNx, &win_x,
                XmNy, &win_y,
                XmNwidth, &win_wid,
                XmNheight, &win_ht,
                XmNborderWidth, &win_bwid,
                NULL);


  _x = win_x;
  _y = win_y;
  _width = win_wid;
  _height = win_ht;
  _border_width = win_bwid;

}

void
RoamMenuWindow::expunge(void)
{
    SdmError error;
    SdmMessageNumberL messageList;
    int answer = 1;

    // First order of business - busy out active windows
    //
    theRoamApp.busyAllWindows(catgets(DT_catd, 3, 32,
				      "Destroying deleted messages..."));

    if (_list->warnAboutDestroy()) {
      DtMailGenDialog *genDialog = theRoamApp.genDialog();
      char buf[256];

      sprintf(buf, "This mailbox contains messages marked for deletion that\nare not included in the deleted count for this view. Clicking\nOK destroys those messages as well as the ones in this view.");
      genDialog->setToWarningDialog(
          catgets(DT_catd, 3, 24, "Mailer"),
          catgets(DT_catd, 3, 141, buf));

      char * helpId = DTMAILHELPDESTROYDELMSGS;
      answer = genDialog->post_and_return(
          catgets(DT_catd, 3, 72, "OK"),
          catgets(DT_catd, 3, 73, "Cancel"),
          helpId);
    }

    // Cancel the expunge operation.
    if (answer == 2) {
      theRoamApp.unbusyAllWindows();
      return;
    }
    
    // Before changing the state of any deleted/undeleted lists,
    // perform the destroy deleted operation and make sure that
    // it was successful - if there was an error, notify the user
    // and discontinue processing
    //
    // We need to change this to call ExpungeDeletedMessages(), but
    // should there be a method that doesn't take a message list?
    //
    _mailbox->ExpungeDeletedMessages(error, messageList);
    if ( error ) {
      // An error happened - must inform the user
      //
      _genDialog->post_error(error, DTMAILHELPCANNOTDESTROYDELMSGS,
			     catgets(DT_catd, 3, 134, "Mailer cannot destroy the messages you deleted."));
    }
    else if (messageList.ElementCount() > 0) {
  
      // need to set _expunging flag here.  it is possible that a ViewMsgDialog
      // can get unregistered during the expunge.  This occurs if there
      // is a view dialog up for a deleted message.  The expunge will 
      // cause this dialog to quit and unregister itself.  In this case,
      // we don't want to call delete_message with the message number
      // of the expunged message.  To handle this, we set the _expunging
      // flag here.  If unregisterDialog gets called during an expunge,
      // it will not call delete_message for the expunged message.
      _expunging = Sdm_True;

      if (_undeleteFromList->dialog()) {
          _undeleteFromList->dialog()->expunge();
      }

      _list->expunge(messageList.ElementCount(), messageList);

      SdmMailRc * mailrc;

      theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailrc);
      if ( error ) {
          _genDialog->post_error(error);
      } else {
        if (mailrc->IsValueDefined("showmsgnum")) {
          // Need to update scrolling list to display
          // resequenced msg nums
          _list->updateListItems(error, -1);

          if (error) {
            _genDialog->post_error(error, DTMAILHELPCANNOTUPDATEMSGHDRLIST,
                catgets(DT_catd, 3, 133, "Mailer cannot update the Message Header List."));
          }
        }
      }

      // if there are view dialogs up, we need to renumber the message
      // number stored in the view dialog.
      if (_numDialogs > 0) {
        // renumber each message numbers stored in the views dialogs.
        for (int i = 0; i < _numDialogs; i++) {
          _dialogs[i]->msgno(get_msgno_after_expunge(_dialogs[i]->msgno(), messageList));
        }
      }

      // we need to update the _recently_used_messages list.  expunged
      // messages should be removed and all remaining message numbers
      // need to be renumbered.
      
      // removed expunged message numbers from recent msgno list.
      MsgNumberStruct *item = _recent_msgs_head;
      MsgNumberStruct *prev = NULL, *next = NULL;
      long newcount = 0;
      while (item != NULL) {
        if (messageList.FindElement(item->msgno) >= 0) {
           if (item == _recent_msgs_head) {
              _recent_msgs_head = _recent_msgs_head->next;
           } else {
              assert (prev != NULL);
              prev->next = item->next;
           }
           next = item->next;
           delete item;
           item = next;
        } else {
           prev = item;
           item = item->next;
           newcount++;
        }
      }
      if (newcount != _recent_msgs_cnt) {
         _recent_msgs_cnt = newcount;
         _recent_msgs_tail = prev;
         if (_recent_msgs_tail) {
            _recent_msgs_tail->next = NULL;
         }
      }

      // renumber each message in the _recently_used_messages list.
      item = _recent_msgs_head;
      while (item != NULL) {
         item->msgno = get_msgno_after_expunge(item->msgno, messageList);
         item = item->next;
      }

      SdmMessageNumber displayed_msgno = _list->current_msg_handle(); 
      SdmError localError;
      prev = next = NULL;
      newcount = 0;
      SdmMessage *msg;

      item = _recent_msgs_head;
      while (item != NULL) {
         if (ifViewExists(item->msgno) == NULL &&
                displayed_msgno != item->msgno)
         {
           _mailbox->SdmMessageFactory(localError, msg, item->msgno);
           if (!localError) {
              delete msg;
           } else {
              localError.Reset();
           }
           if (item == _recent_msgs_head) {
               _recent_msgs_head = _recent_msgs_head->next;
           } else {
               assert (prev != NULL);
               prev->next = item->next;
           }
           next = item->next;
           delete item;
           item = next;

         } else {
           prev = item;
           item = item->next;
           newcount++;
         }
      }

      if (newcount != _recent_msgs_cnt) {
         _recent_msgs_cnt = newcount;
         _recent_msgs_tail = prev;
         if (_recent_msgs_tail) {
            _recent_msgs_tail->next = NULL;
         }
      }

      // print_msgs_list(_recent_msgs_head, _recent_msgs_tail, _recent_msgs_cnt);
      _expunging = Sdm_False;
    }

    // Last order of business - unbusy out active windows
    theRoamApp.unbusyAllWindows();
}


SdmMessageNumber 
RoamMenuWindow::get_msgno_after_expunge(SdmMessageNumber msgnum,
            SdmMessageNumberL& expungedMessages)
{
    int offset = 0;
    for (int j=0; j< expungedMessages.ElementCount(); j++) {
      if (expungedMessages[j] >= msgnum) {
        break;
      } else {
        offset++;
      }
    }

    if (offset > 0) {
      return (msgnum-offset);
    } else {
      return msgnum;
    }
}

void
RoamMenuWindow::attachment_selected()
{
  _save_attachment->activate();
}


void
RoamMenuWindow::all_attachments_selected()
{
  _save_attachment->deactivate();

  if (_attachmentActionsList != NULL) {
    _attachmentMenuPR->removeCommands(NULL, _attachmentActionsList);
    _menuPopupAtt->removeCommands(_attachmentPopupMenu, 
                                  _attachmentActionsList);
    if (_attachmentActionsList != NULL) {
      while (_attachmentActionsList->size()) {
        delete (*_attachmentActionsList)[0];
        _attachmentActionsList->remove(0);
      }
      _attachmentActionsList->deleteMe();
    }
    _attachmentActionsList = NULL;
  }
}

void
RoamMenuWindow::all_attachments_deselected()
{
  _save_attachment->deactivate();

  if (_attachmentActionsList != NULL) {
    _attachmentMenuPR->removeCommands(NULL, _attachmentActionsList);
    _menuPopupAtt->removeCommands(_attachmentPopupMenu, 
                                  _attachmentActionsList);
                                  
    if (_attachmentActionsList != NULL) {
      while (_attachmentActionsList->size()) {
        delete (*_attachmentActionsList)[0];
        _attachmentActionsList->remove(0);
      }
      _attachmentActionsList->deleteMe();
    }
    _attachmentActionsList = NULL;
  }
}

void
RoamMenuWindow::activate_default_message_menu()
{
  Widget pw = _copy_to->getPaneWidget();

  _open_msg->activate();
  _save_msg_as->activate();

  if (pw)
    XtSetSensitive(pw, Sdm_True);

  _print_msg->activate();

  if (mailBoxWritable() == Sdm_True) {
    _delete_msg->activate();
  }
}

void
RoamMenuWindow::deactivate_default_message_menu()
{
  Widget pw = _copy_to->getPaneWidget();

  _open_msg->deactivate();
  _save_msg_as->deactivate();

  if (pw)
    XtSetSensitive(pw, Sdm_False);

  _print_msg->deactivate();

  _delete_msg->deactivate();
}  

void
RoamMenuWindow::activate_default_attach_menu()
{
  _select_all_attachments->activate();
}

void
RoamMenuWindow::deactivate_default_attach_menu()
{
  _select_all_attachments->deactivate();
}

void
RoamMenuWindow::activate_multi_select_menu()
{
  _save_msg_as->activate();
}

// When multiple selection in the scrolling list, we should grey out
// Save As Text menu 
//
void
RoamMenuWindow::deactivate_multi_select_menu()
{
  _save_msg_as->deactivate();
}  

void
RoamMenuWindow::newMailIndicators(void)
{
    // Set to new mail icon only if the window is iconified,
    // but we want to notify the front panel with the change
    // in property if the window is unmapped as well as when
    // it's iconified.
    if (this->isIconified())  {
        setIconName(NewMailIcon);
	// We want to let the front panel know when we detect
	// new mail.  We only do this if we have a lock on
	// the atom "_SUN_MAIL_STATUS".
	if (_lock_status) {
	    MailStatus *new_mail_data;

	    new_mail_data = (MailStatus *) malloc(sizeof(MailStatus));
	    new_mail_data->new_mail = True;
	    XChangeProperty(XtDisplay(baseWidget()),
			    XtWindow(baseWidget()),
			    _SUN_MAIL_STATUS, _SUN_MAIL_STATUS,
			    32, PropModeReplace,
			    (unsigned char *) new_mail_data,
			    1);   // The "1" means we are passing one element.
	    free(new_mail_data);
	}
    } else {
	if (_lock_status) {  // Don't need to change prop if we don't have the lock.
	    XWindowAttributes	xwa;
	    XGetWindowAttributes(XtDisplay(baseWidget()), XtWindow(baseWidget()), &xwa);

	    if (xwa.map_state != IsViewable) { // It's not mapped.
		MailStatus *new_mail_data;

		new_mail_data = (MailStatus *) malloc(sizeof(MailStatus));
		new_mail_data->new_mail = True;
		XChangeProperty(XtDisplay(baseWidget()),
				XtWindow(baseWidget()),
				_SUN_MAIL_STATUS, _SUN_MAIL_STATUS,
				32, PropModeReplace,
				(unsigned char *) new_mail_data,
				1);   // The "1" means we are passing one element.
		free(new_mail_data);
	    }

	}
    }

    SdmError error;
    SdmMailRc *mailRc;
    char *val;
    int notification;

    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
    assert(!error);
    // Beeps!
    mailRc->GetValue(error, "bell", &val);
    if ( !error ) {
	notification = (int) strtol(val, NULL, 10);
	while(notification > 0) {
		XBell(XtDisplay(baseWidget()), 0);
		notification -= 1;
    	}
	free (val);
    }
    else  // Default to 1 beep
    	XBell(XtDisplay(baseWidget()), 0);

    // Sound File!
    mailRc->GetValue(error, "realsound", &val);
    if ( !error ) {
    	struct stat stat;
    	if (SdmSystemUtility::SafeStat("/usr/bin/audioplay", &stat) == 0) {
		char play_str[1500];
		// Provide the -i switch to audioplay: If the audio device is unavailable
		// (that is, another process currently has write access), audioplay
		// ordinarily waits until it can obtain access to the device. When the -i
		// option is present, audioplay prints an error message and exits
		// immediately if the device is busy. While this means that if the user
		// is busy using the audio device at the time new mail is detected, no
		// new mail sound will be emitted, it prevents stacking up multiple
		// audioplay processes waiting for a hung audio device.
		sprintf(play_str, "/usr/bin/audioplay -i %s", val);
		// fork a new process
		// Changed to call fork1() so that only the thread making the call
		// is duplicated as opposed to all threads. See fork1(2) for details.
		// If fork1() is called and the child does more than just call exec(),
		// there is a possibility of deadlocking - buyer beware!
    		if (fork1() == 0) {
		  // exec the audio player
		  execl("/bin/sh", "sh", "-c", play_str, (char *)0);
		  // returned - exec failed - just exit this process.
		  _exit(0);
		}
     	}
	free(val);
    }
    // Flashes!
    mailRc->GetValue(error, "flash", &val);
    if ( !error ) {
    	notification = (int) strtol(val, NULL, 10);
    	flash(notification);
	free(val);
    }
}

void
RoamMenuWindow::save_attachment_callback(void *client_data,char *selection)
{
  RoamMenuWindow *obj = (RoamMenuWindow *) client_data;
  obj->save_selected_attachment(selection);
}

void
RoamMenuWindow::save_selected_attachment(char * selection)
{
    SdmError mail_error;

    // Initialize the mail_error.
    mail_error.Reset();

    DtMailEditor *editor = this->get_editor();
    AttachArea *attacharea = editor->attachArea();
    Attachment *attachment = attacharea->getSelectedAttachment();

   // Get selected attachment, if none selected, then return.
   if ( attachment == NULL ) {
	  // Let User know that no attachment has been selected???
	  int answer = 0;
	  char *helpId = NULL;


	  _genDialog->setToErrorDialog(
		    catgets(DT_catd, 1, 80, "Mailer"),		  
		    catgets(DT_catd, 2, 14, "An attachment needs to be selected before issuing the\n\"Save As\" command to save to a file.") );
	  helpId = DTMAILHELPSELECTATTACH;
	  answer = _genDialog->post_and_return(
			catgets(DT_catd, 3, 33, "OK"), helpId );
	  return;
      }

    attachment->saveToFile(mail_error, selection);

    if ( mail_error ) {
	// do something
    }
}

void
RoamMenuWindow::addAttachmentActions(char **actions,int indx)
{
    int i;
    char *anAction;
    AttachmentActionCmd *attachActionCmd;
    
    if (_attachmentActionsList == NULL) { 
      _attachmentActionsList = new CmdList("AttachmentActions", "AttachmentActions");
    }
    else {
      _attachmentMenuPR->removeCommands(NULL, _attachmentActionsList);
      _menuPopupAtt->removeCommands(_attachmentPopupMenu, 
           _attachmentActionsList);

      if (_attachmentActionsList != NULL) {
        while (_attachmentActionsList->size()) {
          delete (*_attachmentActionsList)[0];
          _attachmentActionsList->remove(0);
        }
        _attachmentActionsList->deleteMe();
      }
      _attachmentActionsList = new CmdList("AttachmentActions", "AttachmentActions");
    }

    char *actionLabel;
    for (i = 0; i < indx; i++) {
	anAction = actions[i];

	// Retrieve the localized action label
	actionLabel = DtActionLabel(anAction);
	attachActionCmd = new AttachmentActionCmd(
					anAction, 
					actionLabel,
					this,
					i);
	_attachmentActionsList->add(attachActionCmd);
	free (actionLabel);
	
    }
    // Add the dynamic commands
    if (_attachmentMenuPR)
      _attachmentMenuPR->addCommands( (Widget)NULL, _attachmentActionsList);

    _attachmentPopupMenu = _menuPopupAtt->addCommands(
				_attachmentPopupMenu, 
				_attachmentActionsList);
}

void
RoamMenuWindow::removeAttachmentActions()
{
    // Stubbed out for now
}

void
RoamMenuWindow::invokeAttachmentAction(
    int index
)
{
    DtMailEditor *editor = this->get_editor();
    AttachArea *attacharea = editor->attachArea();
    Attachment *attachment = attacharea->getSelectedAttachment();

    attachment->invokeAction(index);
}

void
RoamMenuWindow::selectAllAttachments()
{
    DtMailEditor *editor = this->get_editor();
    AttachArea *attachArea = editor->attachArea();
    
    attachArea->selectAllAttachments();
}

// Returns the index of the match, -1 if there is no match.
int
RoamMenuWindow::inList(char *filename, DtVirtArray<MoveCopyMenuCmd *> *list)
{
    if (list == NULL) return(-1);

    MoveCopyMenuCmd *cmd;
    char *tmpbuf=NULL, *server=NULL;
    int i, list_length = list->length(), ret = -1;

    if (list_length == 0) return ret;

    theRoamApp.fromLabelToServerMb(filename, &server, &tmpbuf);

    for (i=0; i < list_length; i++) {
	cmd = (*list)[i];

	if ( (server == NULL && cmd->server() == NULL) ||
		(server != NULL && cmd->server() != NULL &&
                        strcmp(cmd->server(), server) == 0) ) {

		char *name;
		SdmMailRc *mailrc;
		SdmError error;
		theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailrc);
		SdmUtility::GetRelativePath(error, name, cmd->containerName(),
		  	*mailrc, cmd->server()?SdmUtility::kImapFolderResourceName:
				SdmUtility::kFolderResourceName);

		if (strcmp(name, tmpbuf) == 0) {
			ret = i;
			free(name);
			break;
		}
		free(name);
	}
    }
    if (server) free(server);
    free(tmpbuf);
    return ret;
}

void
RoamMenuWindow::addToRecentList(char *filename)
{
  int i, index, bottom_index;
  MoveCopyMenuCmd	*move_container, *copy_container;
  OpenMailboxCmd	*open_container;
  char *name, *new_name, *server, *new_server;

  if (filename && *filename && _max_cached_list_size > 0) {
    // Is the file in the user defined list?
    if ((index = inList(filename, _move_containerlist)) != -1) 
      return;

    // Is the file in the recently used list?
    if ((index = inList(filename, _cached_move_containerlist)) != -1) {
      // Yes, move filename to top of list and move everything else down.
      if (index == 0) return;

      // Change the filenames that each of the Cmds points to in both
      // the move and copy cached lists.
      name = strdup((*_cached_move_containerlist)[index]->containerName());
      if ((*_cached_move_containerlist)[index]->server()) {
        server = strdup((*_cached_move_containerlist)[index]->server());
      } else {
        server = NULL;
      }

      for (i = index; i > 0; i--) {
        new_name = (*_cached_move_containerlist)[i-1]->containerName();
        new_server = (*_cached_move_containerlist)[i-1]->server();
        (*_cached_move_containerlist)[i]->changeContainer(new_name);
        (*_cached_move_containerlist)[i]->changeServer(new_server);
        (*_cached_copy_containerlist)[i]->changeContainer(new_name);
        (*_cached_copy_containerlist)[i]->changeServer(new_server);
        (*_cached_open_containerlist)[i]->changeContainer(new_name);
        (*_cached_open_containerlist)[i]->changeServer(new_server);
      }

      (*_cached_move_containerlist)[0]->changeContainer(name);
      (*_cached_move_containerlist)[0]->changeServer(server);
      (*_cached_copy_containerlist)[0]->changeContainer(name);
      (*_cached_copy_containerlist)[0]->changeServer(server);
      (*_cached_open_containerlist)[0]->changeContainer(name);
      (*_cached_open_containerlist)[0]->changeServer(server);

      free(name);
      if (server) free(server);
      
      // Rename the labels in the menu.
      if (_first_cached_move_item != _first_cached_move_item + index) {
        _menuBar->rotateLabels(_moveMenu,
                               _first_cached_move_item,
                               _first_cached_move_item + index);
        _menuBar->rotateLabels(_msgsPopupMoveMenu,
                               _first_cached_move_item,
                               _first_cached_move_item + index);
        _menuBar->rotateLabels(_copyMenu,
                               _first_cached_copy_item,
                               _first_cached_copy_item + index);
        _menuBar->rotateLabels(_openMenu,
                               _first_cached_open_item,
                               _first_cached_open_item + index);
      }
    }
    else {
      // Add filename to Recently Used List.
      bottom_index = _cached_list_size - 1;

      // Is there room for the menu to grow?
      if (_cached_list_size < _max_cached_list_size) {

        // Create the new command.
        move_container = new MoveCopyMenuCmd(filename,
                                             filename,
                                             Sdm_True,
                                             this,
                                             DTM_MOVE);
        copy_container = new MoveCopyMenuCmd(filename,
                                             filename,
                                             Sdm_True,
                                             this,
                                             DTM_COPY);
        open_container = new OpenMailboxCmd(filename,
                                            filename,
                                            Sdm_True,
                                            this,
                                            filename);
        // Add it to the bottom of the menu.
        _menuBar->addCommand(_moveMenu, move_container);
        _menuBar->addCommand(_msgsPopupMoveMenu, move_container);
        _menuBar->addCommand(_copyMenu, copy_container);
        _menuBar->addCommand(_openMenu, open_container);
        _cached_move_containerlist->append(move_container);
        _cached_copy_containerlist->append(copy_container);
        _cached_open_containerlist->append(open_container);
        
        // save cmds to list to be destroyed in destructor.
        _movecopymenucmd_list->append(move_container);
        _movecopymenucmd_list->append(copy_container);
        _openmailboxcmd_list->append(open_container);

        _cached_list_size++;
        bottom_index++;
        if (_cached_list_size > 1) {
          // Modify the Cmds in the DtVirtArray
          // and rotate the labels in the menu.
          name = strdup((*_cached_move_containerlist)[bottom_index]->containerName());
          if ((*_cached_move_containerlist)[bottom_index]->server()) {
            server = strdup((*_cached_move_containerlist)[bottom_index]->server());
          } else {
            server = NULL;
          }

          for (i = bottom_index; i > 0; i--) {
            new_name = (*_cached_move_containerlist)[i-1]->containerName();
            new_server = (*_cached_move_containerlist)[i-1]->server();
            (*_cached_move_containerlist)[i]->changeContainer(new_name);
            (*_cached_move_containerlist)[i]->changeServer(new_server);
            (*_cached_copy_containerlist)[i]->changeContainer(new_name);
            (*_cached_copy_containerlist)[i]->changeServer(new_server);
            (*_cached_open_containerlist)[i]->changeContainer(new_name);
            (*_cached_open_containerlist)[i]->changeServer(new_server);
          }
          (*_cached_move_containerlist)[0]->changeContainer(name);
          (*_cached_move_containerlist)[0]->changeServer(server);
          (*_cached_copy_containerlist)[0]->changeContainer(name);
          (*_cached_copy_containerlist)[0]->changeServer(server);
          (*_cached_open_containerlist)[0]->changeContainer(name);
          (*_cached_open_containerlist)[0]->changeServer(server);
  	  free(name);
	  if (server) free(server);
          
          if (_first_cached_move_item != 
              _first_cached_move_item + _cached_list_size-1) {
            _menuBar->rotateLabels(_moveMenu, 
                                   _first_cached_move_item,
                                   _first_cached_move_item + _cached_list_size-1);
            _menuBar->rotateLabels(_msgsPopupMoveMenu, 
                                   _first_cached_move_item,
                                   _first_cached_move_item + _cached_list_size-1);
            _menuBar->rotateLabels(_copyMenu, 
                                   _first_cached_copy_item,
                                   _first_cached_copy_item + _cached_list_size-1);
            _menuBar->rotateLabels(_openMenu, 
                                   _first_cached_open_item,
                                   _first_cached_open_item + _cached_list_size-1);
          }
        } else {
          // Initialize _first_cached_item.  The first cached
          // item is after the user defined list, after the
          // Inbox and Move To Mailboxes items, and after the
          // two separator items.
          if (_user_list_size > 0) {
            _first_cached_move_item = _user_list_size + 5;
            _first_cached_copy_item = _user_list_size + 5;
            _first_cached_open_item = _user_list_size + 6;
          } else {
            _first_cached_move_item = 4;
            _first_cached_copy_item = 4;
            _first_cached_open_item = 5;
          }
        }

      } else {
        // Add new entry to top of cache and drop off bottom item.
        for (i = bottom_index; i > 0; i--) {
          new_name = (*_cached_move_containerlist)[i-1]->containerName();
          new_server = (*_cached_move_containerlist)[i-1]->server();
          (*_cached_move_containerlist)[i]->changeContainer(new_name);
          (*_cached_move_containerlist)[i]->changeServer(new_server);
          (*_cached_copy_containerlist)[i]->changeContainer(new_name);
          (*_cached_copy_containerlist)[i]->changeServer(new_server);
          (*_cached_open_containerlist)[i]->changeContainer(new_name);
          (*_cached_open_containerlist)[i]->changeServer(new_server);
        }
	// Took out the fix for bug 4015384 because my recent changes are
	// such that all stored names are relative and the full path is
	// resolved when the item is selected on the menu.
        theRoamApp.fromLabelToServerMb(filename, &server, &name);
        (*_cached_move_containerlist)[0]->changeContainer(name);
        (*_cached_move_containerlist)[0]->changeServer(server);
        (*_cached_copy_containerlist)[0]->changeContainer(name);
        (*_cached_copy_containerlist)[0]->changeServer(server);
        (*_cached_open_containerlist)[0]->changeContainer(name);
        (*_cached_open_containerlist)[0]->changeServer(server);
        free(name);
        if (server) free(server);
        
        _menuBar->changeLabel(_moveMenu,
                              _first_cached_move_item + _cached_list_size-1,
                              filename);
        _menuBar->changeLabel(_msgsPopupMoveMenu,
                              _first_cached_move_item + _cached_list_size-1,
                              filename);
        _menuBar->changeLabel(_copyMenu,
                              _first_cached_copy_item + _cached_list_size-1,
                              filename);
        _menuBar->changeLabel(_openMenu,
                              _first_cached_open_item + _cached_list_size-1,
                              filename);
        if (_first_cached_move_item != 
            _first_cached_move_item + _cached_list_size-1) {
          _menuBar->rotateLabels(_moveMenu,
                                 _first_cached_move_item,
                                 _first_cached_move_item + _cached_list_size-1);
          _menuBar->rotateLabels(_msgsPopupMoveMenu,
                                 _first_cached_move_item,
                                 _first_cached_move_item + _cached_list_size-1);
          _menuBar->rotateLabels(_copyMenu,
                                 _first_cached_copy_item,
                                 _first_cached_copy_item + _cached_list_size-1);
          _menuBar->rotateLabels(_openMenu,
                                 _first_cached_open_item,
                                 _first_cached_open_item + _cached_list_size-1);
        }

      }
    }
  }
}

void
RoamMenuWindow::showAttachArea()
{
  DtMailEditor *editor = this->get_editor();
  editor->showAttachArea();
}

void
RoamMenuWindow::hideAttachArea()
{
  DtMailEditor *editor = this->get_editor();
  editor->hideAttachArea();
}

void
RoamMenuWindow::fullHeader(SdmBoolean state)
{
  SdmMessageNumber msgHandle;
  SdmError mail_error;

  // Initialize the mail_error.
  mail_error.Reset();

  _fullHeader = state;
    
  msgHandle = this->list()->current_msg_handle();
  if (msgHandle) {
    this->list()->display_message(mail_error, msgHandle);
  }
  if (mail_error) {
    _genDialog->post_error(mail_error, DTMAILHELPCANNOTDISPLAYMSG,
			   catgets(DT_catd, 3, 128, "Mailer cannot display the message you selected."));
    // don't return.  just continue.  reset error.
    mail_error.Reset();
  }
}

void 
RoamMenuWindow::postMsgsPopup(XEvent *event)
{
  XmMenuPosition(_msgsPopupMenu, (XButtonEvent *)event);
  XtManageChild(_msgsPopupMenu);
}

void
RoamMenuWindow::MenuButtonHandler(Widget , XtPointer cd, XEvent *event, char *)
{
  RoamMenuWindow *obj = (RoamMenuWindow *)cd;

  if(event->xany.type != ButtonPress)
    return;

  XButtonEvent *be = (XButtonEvent *)event;

  if(be->button == theApplication->bMenuButton())
    obj->postMsgsPopup(event);
}

void
RoamMenuWindow::attachmentFeedback(SdmBoolean bool)
{
  if (bool) {
    this->busyCursor();
  }
  else {
    this->normalCursor();
  }
}

void
RoamMenuWindow::map_open_menu(Widget cascade, XtPointer clientData, XtPointer)
{
  RoamMenuWindow *rmw; 
  Widget menu = (Widget)clientData;

  XtVaGetValues(cascade, XmNuserData, &rmw, NULL);
  char *logfile = theRoamApp.getResource(SentMessageLogFile, Sdm_True);
  if (SdmSystemUtility::SafeAccess(logfile, F_OK) == 0)
    rmw->_logfile->activate();
  else
    rmw->_logfile->deactivate();
  free(logfile);

  theRoamApp.configure_menu(menu);
}

// map_menu is used to figure out how many columns to split the menu into.
// It is a callback that is called when the menu is mapped.  If the menu is
// over half the height of the screen, it figures out how many columns to
// make the menu, and sets its XmNnumColumns attribute to that value.  It
// calculates the maximum number of columns that would fit and never goes
// beyond that number.
void
RoamMenuWindow::map_menu(Widget widget, XtPointer clientData, XtPointer)
{
  Widget  menu, cascade;

  if (clientData) {
    RoamMenuWindow *rmw;
    XtVaGetValues(widget, XmNuserData, &rmw, NULL);
    CmdList *cl = (CmdList*)clientData;
    menu = cl->getPaneWidget();
    rmw->addFoldersFiles(menu, cl);
    // Add the map_menu callbacks and User data to any new directories.
    for (int i = 0; i < cl->size(); i++) {
      if (strcmp((*cl)[i]->className(), "CmdList") == 0) {
        cascade = ((CmdList*)(*cl)[i])->getCascadeWidget();
        XtAddCallback(cascade, XmNcascadingCallback, 
                      &RoamMenuWindow::map_menu, (*cl)[i]);
        XtVaSetValues(cascade, XmNuserData, rmw, NULL);
      }
    }
  }
  else
    XtVaGetValues(widget, XmNuserData, &menu, NULL);

  theRoamApp.configure_menu(menu);
}

void
RoamMenuWindow::setVacationTitle()
{
  // Add "[Vacation]" to the title of the roam menu window 
  char *path = getTitle();
  if (path != NULL && *path != NULL) {
    char vacation_str[MAX_STATUS_LEN];
    memset(vacation_str, 0, MAX_STATUS_LEN);
    sprintf(vacation_str, "[%s]", catgets(DT_catd, 1, 3, "Vacation"));
    
    // If the Vacation string is displayed, do nothing
    char *p;
    if (p = strstr(path, vacation_str)) {
      return;
    }
    char *new_title = new char[strlen(path) + strlen(vacation_str) + 4];
    sprintf(new_title, "%s %s", path, vacation_str);
    this->title(new_title);
    delete [] new_title; 
  }
}

void
RoamMenuWindow::removeVacationTitle()
{
  // Reset the title on the roam menu window; take out "[Vacation]"
  char *path = this->getTitle();
  if (path != NULL && *path != NULL) {
    char vacation_str[MAX_STATUS_LEN];
    memset(vacation_str, 0, MAX_STATUS_LEN);
    sprintf(vacation_str, "[%s]", catgets(DT_catd, 1, 3, "Vacation"));
    
    char *new_title = strdup(path);
    char *p;
    if (p = strstr(new_title, vacation_str)) {
            *(p-1) = '\0';
	    this->title(new_title);
    }
    delete [] new_title;
  }
}



void RoamMenuWindow::delete_message(SdmMessageNumber msgno)
{
  static const int kMaxCachedMessages = 25;
  SdmMessageNumber displayed_msgno = _list->current_msg_handle(); 
  
  // add message number to cache only if it's not being viewed in the 
  // RoamMenuWindow or in a ViewMsgDialog.
  if (_mailbox && msgno > 0 && ifViewExists(msgno) == NULL &&
       displayed_msgno != msgno) 
  {
    // first, check to see if message is already there.  
    MsgNumberStruct *item = _recent_msgs_head;
    MsgNumberStruct *prev = NULL;
    while (item != NULL) {
       if (item->msgno == msgno) {
         break; 
       }
       prev = item;
       item = item->next;
    }


    // we didn't find the message, choose a message to delete.
    if (item == NULL) {

       // if we haven't exceeded the max number of cached messages,
       // we just add this one to the end of the list and return.
       if (_recent_msgs_cnt < kMaxCachedMessages) {
          add_to_recent_msgs(msgno);
          // print_msgs_list(_recent_msgs_head, _recent_msgs_tail, _recent_msgs_cnt);
          return;
       }

       // otherwise, we look for a message from the cache that
       // can be deleted.
       item = _recent_msgs_head;
       prev = NULL;
       while (item != NULL) {
         if (ifViewExists(item->msgno) == NULL && 
              displayed_msgno != item->msgno) 
         {
             break;
         }
         prev = item;
         item = item->next;
       }
    }

    // move the entry that we found to the end of the list.
    // note that we do not bump the _recent_msgs_cnt in this 
    // case because we are reusing an item that's already in
    // the list.
    if (item != NULL) {

       // if entry that we found does not match the 
       // deleted message, delete the SdmMessage object 
       // for the entry we found.
       if (item->msgno != msgno) {
           SdmMessage *msg;
           SdmError error;
           _mailbox->SdmMessageFactory(error, msg, item->msgno);
           if (!error) {
              delete msg;
           }
           item->msgno = msgno;
       }

       // if the item is not already at the end of the list,
       // move it to the end of the list.
       if (item != _recent_msgs_tail) {
           if (item == _recent_msgs_head) {
             _recent_msgs_head = _recent_msgs_head->next;
           } else {
             prev->next = item->next;
           }
           item->next = NULL;
           _recent_msgs_tail->next = item;
           _recent_msgs_tail = item;
       }

    // couldn't find an entry to replace.  just add msgno to 
    // of the end of list.
    } else {
       add_to_recent_msgs(msgno);
    }
  }

  // print_msgs_list(_recent_msgs_head, _recent_msgs_tail, _recent_msgs_cnt);
}

void RoamMenuWindow::add_to_recent_msgs(SdmMessageNumber msgno)
{
   MsgNumberStruct *mns = new MsgNumberStruct;
   mns->next = NULL;
   mns->msgno = msgno;
   if (_recent_msgs_head == NULL) {
      _recent_msgs_head = mns;
      _recent_msgs_tail = mns;
   } else {
      _recent_msgs_tail->next = mns;
      _recent_msgs_tail = mns;
   }
   _recent_msgs_cnt++;
}
