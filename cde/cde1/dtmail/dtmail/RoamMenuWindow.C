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
#pragma ident "@(#)RoamMenuWindow.C	1.351 07/18/96"
#endif

#include <stdio.h>
#include <unistd.h>
#if !defined(USL)
#include <strings.h>
#endif
#include <sys/utsname.h>
#ifdef SunOS
#include <sys/systeminfo.h>
#endif
#include <ctype.h>
#include <X11/cursorfont.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/RepType.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Dt/Dnd.h>
#include <DtMail/DtMailTypes.h>
#include <DtMail/IO.hh>
#include <DtMail/PropSource.hh>
#include <DtMail/OptCmd.h>
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
#include <dirent.h>

#include "ComposeCmds.hh"
#include "EUSDebug.hh"

#include "SortCmd.hh"

extern force( Widget );

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
#define MAX_STATUS_LEN	50

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
    sizeof( Boolean ),
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

RoamMenuWindow::RoamMenuWindow ( char *name, Tt_message tt_msg ) :
    MenuWindow ( "dtmail", tt_msg )
{
    DtMailEnv mail_error;
    
    // Initialize mail_error.
    mail_error.clear();
    
    // Get a handle to the Inbox.
    char * mail_file = NULL;
    DtMailObjectSpace space;
    DtMail::Session * d_session = theRoamApp.session()->session();

    // Initialize private variables

    _mbox_image = NULL;
    _mbox_mask  = NULL;
    _numDialogs = 0;
    _mailbox    = NULL;
    _list	= NULL;
    _genDialog  = NULL;
    _findDialog = NULL;
    _fullHeader = FALSE;
    _dialogs    = NULL;
    _mailbox_fullpath = NULL;

    // Set the _mailbox_name here.  
    // In the absence of the RMW's title being set via title(),
    // the _mailbox_name is what will be used in initialize().

    _mailbox_name = strdup(name);
//  _timer_id = 0;
    clear_message_p = FALSE;
    _is_loaded = FALSE;
    _requiredConversion = FALSE;
    _we_called_newmail = FALSE;

    _hide_show = NULL;
    _message = NULL;
    _message_summary = NULL;
    rowOfLabels = NULL;
    rowOfButtons = NULL;
    rowOfMessageStatus = NULL;
  
        _separator= new SeparatorCmd( "Separator", TRUE );
    
//    _list = NULL;
    
    
    // Mailbox Menu
    
    _check_new_mail = NULL;
    _msg_select_all = NULL;
    _open_inbox	= NULL;
    _new_container = NULL;
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

    // Cascade Widgets
    _message_cascade = NULL;
    _file_cascade = NULL;
    _compose_cascade = NULL;
    _move_cascade = NULL;
    
    // View Menu
    
    _abbrevHeaders = NULL;
    _previous = NULL;
    _next = NULL;
    _find = NULL;
    _sortTD = NULL;
    _sortSender = NULL;	
    _sortSubject = NULL;	
    _sortSize = NULL;	
    _sortStatus = NULL;
    
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
    _folder = NULL;
    _move_popup_menu_cmdList = NULL;

    // Help Menu
    _overview = NULL;
    _tasks = NULL;
    _reference = NULL;
    _messages = NULL;
    _on_item = NULL;
    _using_help = NULL;
    _about_mailer = NULL;
    
    _move_copy_button = NULL;
    _copy_move_button = NULL;
    _open_other_mboxes = NULL;
    _move_other_mboxes = NULL;
    _move_popup_other_mboxes = NULL;
    _copy_other_mboxes = NULL;
#ifdef AUTOFILING
    _mailbox_list = NULL;
#endif // AUTOFILING

    _toolbar_cmds = NULL;
    _toolbar_icons = NULL;
    _toolbar_inactive_icons = NULL;
    _toolbar_mailrc_string = NULL;
    _toolbar_use_icons = False;

    _mailbox_index = 0;

#ifdef DEAD_WOOD
    // ConvertContainerCmd

    _convertContainerCmd = NULL;
#endif // DEAD_WOOD
    _openContainerCmd = NULL; 
   
    // See if this RMW is for the inbox 
    
    d_session->queryImpl(mail_error,
			 d_session->getDefaultImpl(mail_error),
			 DtMailCapabilityInboxName,
			 &space,
			 &mail_file);

    if (strcmp(mail_file, _mailbox_name) == 0) {
	_inbox = TRUE;
    }
    else {
	_inbox = FALSE;
    }
}

RoamMenuWindow::~RoamMenuWindow() 
{
    delete	_list;

    DtMailEnv	error;

    // Initialize the mail_error.
    error.clear();

    theRoamApp.session()->close(error, _mailbox);
    if (error.isSet()) {
	// Cannot return error to caller since there's
	// no caller.  What do we do?
	// exit(0)??
    }
    
    // Mailbox Menu
    
    delete	_check_new_mail;
    delete	_open_inbox;
    delete	_new_container;
    delete	_logfile;
    delete	_open;
    delete	_open_other_mboxes;
    delete	_move_other_mboxes;
    delete	_move_popup_other_mboxes;
    delete	_copy_other_mboxes;
#ifdef AUTOFILING
    delete	_mailbox_list;
#endif // AUTOFILING
    delete	_destroyDeletedMessages;
    delete	_quit;

    // View Menu
    
    delete	_next;
    delete	_previous;
    delete	_abbrevHeaders;
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
    delete	_copy_to;
    delete	_copy_inbox;
    delete	_print_msg;
    delete	_print;
    delete	_find;
    delete	_delete_msg;
    delete	_undeleteLast;
    delete	_undeleteFromList;

    // Move menu

    delete	_move_menu_cmdList;
    delete	_move_popup_menu_cmdList;
    delete	_move_inbox;
    delete	_move_containerlist;
    delete	_cached_move_containerlist;
  
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

    delete	_attachmentActionsList;
    delete 	_attachmentMenuList;
    delete 	_attachmentPopupMenuList;
    delete      _attachment_popup_title;
    delete 	_textPopupMenuList;
    delete      _text_popup_title;
    delete      _msgsPopupMenuList;
    delete      _msgs_popup_title;
    delete 	_menuPopupMsgs;
    
    // Compose Menu
    
    delete	_comp_new;
    delete	_forward;
    delete	_forwardBody;
    delete      _templates;
    delete      _drafts;
    
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
    
#ifdef DEAD_WOOD
    // ConvertContainerCmd
    
    delete	_convertContainerCmd;
#endif // DEAD_WOOD
    delete	_openContainerCmd;
    
    // Generic
    
    delete	_genDialog;
    delete [] _dialogs;
    
    // Mailbox Image
    if (_mbox_image)
	XmDestroyPixmap(XtScreen(baseWidget()), _mbox_image);
    if (_mbox_mask)
	XmDestroyPixmap(XtScreen(baseWidget()), _mbox_mask);
    if (_mailbox_fullpath)
        free(_mailbox_name);

    theRoamApp.unregisterMailView(_mailbox_index);
    delete _toolbar_cmds;
    delete _toolbar_icons;
    delete _toolbar_inactive_icons;
    free (_toolbar_mailrc_string);
    delete _separator;
}

void
RoamMenuWindow::initialize()
{
    XmRepTypeInstallTearOffModelConverter();
    MenuWindow::initialize();

    if (!this->baseWidget()) {
	fprintf(stderr, 
			catgets(DT_catd, 2, 3,
			"Unable to initialize windows. Exiting.\n"));
	exit(1);
    }

    busyCursor();
    setStatus(catgets(DT_catd, 3, 2, "Initializing..."));

    XInternAtom( XtDisplay(this->baseWidget()) ,"STRING", False);
    XInternAtom( XtDisplay(this->baseWidget()) ,"MESSAGES", False);

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
    
    // Add an event handler for structureNotify.
    // This EH will handle exposure, configure notifies ...

    XtAddEventHandler( this->baseWidget(), StructureNotifyMask,
		       False,
		       ( XtEventHandler ) &RoamMenuWindow::structurenotify,
		       (XtPointer) this );

#ifdef DEAD_WOOD
    _convertContainerCmd = new ConvertContainerCmd(
					"convertContainer", 
					"convertContainer", 
					TRUE, this);
#endif // DEAD_WOOD
    _openContainerCmd = new OpenContainerCmd("openContainer", 
					     "openContainer",
					     TRUE, this);

    _genDialog = new DtMailGenDialog("Dialog", _main);

    setIconName(NormalIcon);
    // register ourselves with the RoamApp object and then record what number
    // mailbox we are.
    setMailboxIndex(theRoamApp.registerMailView(this));
    manage();
    buildToolbar();
}

Boolean
RoamMenuWindow::inbox()
{
    return(_inbox);
}

Widget
RoamMenuWindow::createWorkArea ( Widget parent )
{
    Widget form1, panedW;
    Widget form2, form3;
    Dimension x, y, width, height, bwid;
    XtWidgetGeometry size;
    VacationCmd *vacation_cmd = theRoamApp.vacation();

    form1 = XmCreateForm(parent, "Work_Area",  NULL, 0);
    printHelpId("form1", form1);
    /* add help callback */
    // XtAddCallback(form1, XmNhelpCallback, HelpCB, helpId);
    XtAddCallback(form1, XmNhelpCallback, HelpCB, DTMAILWINDOWID);

    panedW = XtCreateManagedWidget("panedW", xmPanedWindowWidgetClass,
				   form1, NULL, 0);
    printHelpId ("panedW", panedW);
    /* add help callback */
    // XtAddCallback(panedW, XmNhelpCallback, HelpCB, helpId);

    XtVaSetValues(panedW,
		  XmNrightAttachment,XmATTACH_FORM, 
		  XmNleftAttachment,XmATTACH_FORM, 
		  XmNtopAttachment,XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNsashIndent, -30,
		  XmNsashWidth, 18,
		  XmNsashHeight, 12,
		  NULL );


    form2 = XtCreateManagedWidget("form2", xmFormWidgetClass,
				  panedW, NULL, 0);
    printHelpId ("form2", form2);
    /* add help callback */
    // XtAddCallback(form2, XmNhelpCallback, HelpCB, helpId);

    rowOfLabels = XtCreateManagedWidget("RowOfLabels", 
 					  xmFormWidgetClass,
 					  form2,
 					  NULL, 0);

    printHelpId("rowOfLabels", rowOfLabels);
    /* add help callback */
    // XtAddCallback(rowOfLabels, XmNhelpCallback, HelpCB, helpId);
    XtAddCallback(
	rowOfLabels, XmNhelpCallback, HelpCB, DTMAILWINDOWROWOFLABELSID);


    size.request_mode = CWHeight;
    XtQueryGeometry(rowOfLabels, NULL, &size);

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

    DtMailEnv error;
    DtMail::Session * d_session = theRoamApp.session()->session();
    DtMail::MailRc * mailrc = d_session->mailRc(error);
    char *value=NULL;
    int	msgnums = False;

    error.clear();
    mailrc->getValue(error, "showmsgnum", (const char **)&value);
    if (error.isNotSet()) {
	msgnums = True;
    } else {
	error.clear();
    }

    this->addToRowOfLabels(_list, msgnums);

    // See if the header size has changed.
    //
    if (value) free(value);
    value = NULL;
    mailrc->getValue(error, "headerlines", (const char **)&value);
    if (error.isSet()) {
	value = strdup("15");
	error.clear();
    }

    int header_lines = (int) strtol(value, NULL, 10);
    _list->visibleItems(header_lines);

    XtAddCallback( _list->get_scrolling_list(),
		   XmNhelpCallback, HelpCB, DTMAILMSGLISTID);

    XtVaSetValues(_list->get_scrolling_list(), XmNuserData, this, NULL);

    rowOfButtons = XtVaCreateManagedWidget("Toolbar", 
					   xmRowColumnWidgetClass,
					   form2,
					   NULL);

    printHelpId ("rowOfButtons", rowOfButtons);
    /* add help callback */
    // XtAddCallback(rowOfButtons, XmNhelpCallback, HelpCB, helpId);


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

    XtVaSetValues(rowOfButtons,
		  XmNorientation, XmHORIZONTAL,
		  XmNpacking, XmPACK_TIGHT,
		  XmNspacing, 10,
		  XmNrightAttachment, XmATTACH_FORM, 
		  XmNleftAttachment, XmATTACH_FORM, 
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNresizeHeight, True,
		  XmNresizeWidth, False,
		  XmNadjustLast, False,
		  NULL );

    form3 = XtCreateManagedWidget("form3", xmFormWidgetClass,
				    panedW, NULL, 0);
    printHelpId ("form3", form3);
    /* add help callback */
    // XtAddCallback(form3, XmNhelpCallback, HelpCB, helpId);

    XtVaSetValues(form3,
	XmNrightAttachment,XmATTACH_FORM, 
	XmNleftAttachment,XmATTACH_FORM, 
	XmNtopAttachment,XmATTACH_FORM, 
	NULL );

    rowOfMessageStatus = XtCreateManagedWidget("Message_Status",
				xmFormWidgetClass,
				form3, NULL, 0);

     XtVaSetValues(rowOfMessageStatus,
  	XmNrightAttachment,XmATTACH_FORM, 
  	XmNleftAttachment,XmATTACH_FORM, 
  	XmNtopAttachment,XmATTACH_FORM,
 	XmNtopOffset, 5,
 	NULL );

    this->addToRowOfMessageStatus();

//    my_editor = new CDEM_DtWidgetEditor(form3, this);

    my_editor = new DtMailEditor(form3, this);
    
    my_editor->initialize();

    my_editor->attachArea()->setOwnerShell(this);

    if (value) free (value);
    value = NULL;
    mailrc->getValue(error, "toolcols", (const char **)&value);
    if (error.isNotSet() && value && *value){
       int cols = (int) strtol(value, NULL, 10);
       Widget twid = my_editor->textEditor()->get_editor();
       XtVaSetValues(twid, DtNcolumns, cols, NULL);
    }
    if (value) free(value);

    // DtMailEditor contains a widget that contains the textEditor and 
    // attachArea.  Get that widget...

    Widget wid = my_editor->container();

    printHelpId ("wid", wid);
    /* add help callback */
    // XtAddCallback(wid, XmNhelpCallback, HelpCB, helpId);

    XtVaSetValues(wid,
	XmNrightAttachment,XmATTACH_FORM, 
	XmNleftAttachment,XmATTACH_FORM, 
	XmNtopAttachment,XmATTACH_WIDGET, 
	XmNtopWidget, rowOfMessageStatus,
	XmNbottomAttachment, XmATTACH_FORM,
	NULL );

    _mailbox_fullpath = strdup(_mailbox_name);

    // Set the appShell's title to be _mailbox_fullpath

    if (vacation_cmd != NULL && vacation_cmd->vacationIsOn()) {
  	char * new_title = new char[strlen(_mailbox_fullpath) +
                 strlen(catgets(DT_catd, 1, 3, "Vacation")) + 10];
    	sprintf(new_title, "%s [%s]", _mailbox_fullpath,
                 catgets(DT_catd, 1, 3, "Vacation"));
	this->title(new_title);
	delete new_title;
    }
    else
	 this->title(_mailbox_fullpath);

    // Set the icon name to be the folder name.  Extract foldername
    // from full path.

    char *fname = strrchr(_mailbox_fullpath, '/');

    if (fname == NULL) {
	// path has no slash in it.
	setIconTitle(_mailbox_fullpath);
    }
    else {
	// Extract the filename from it.
	fname++;
	setIconTitle(fname);
    }

    XtManageChild(_list->baseWidget());
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

// This routine is just a place holder callback to pass when creating
// a mailbox when we doing want to do anything when the mailbox gets
// updated.
//
static DtMailBoolean
openCallback(
	DtMailCallbackOp,
	const char *,
	const char *,
	void *,
	...
)
{
    return(DTM_FALSE);
}

// msgListTransferCallback
//
// Handles the transfer of data that is dragged and dropped to the
// MsgScrollingList.  Files and buffers are transferred by inserting 
// them as messages into the scrolling list.
//
void
RoamMenuWindow::msgListTransferCallback(
    Widget	,
    XtPointer	clientData,
    XtPointer	callData)
{
    RoamMenuWindow *rmw = (RoamMenuWindow *) clientData;
    DtDndTransferCallbackStruct *transferInfo =
			(DtDndTransferCallbackStruct *) callData;
    DtDndContext        *dropData = transferInfo->dropData;
    char		*filepath;
    int			ii, numItems;
    DtMail::MailBox	*mbox, *tmpmbox;
    DtMailEnv mail_error;
    MailSession *session = theRoamApp.session();

    DebugPrintf(3, "In RoamMenuWindow::msgListTransferCallback\n");

    // Initialize the mail_error.
    mail_error.clear();

    numItems = dropData->numItems;

    switch (dropData->protocol) {
	case DtDND_FILENAME_TRANSFER:

    	    // Loop through the files that were dropped onto the msg list

    	    for (ii = 0; ii < numItems; ii++) {

		// Try to open the file as a mail container.
		filepath = dropData->data.files[ii];
		tmpmbox = session->open(mail_error, filepath, 
					&RoamMenuWindow::syncViewAndStoreCallback,
					rmw, DTM_FALSE, DTM_TRUE);

		// Reject the drop if we weren't able to open the file.
		if (!tmpmbox || mail_error.isSet()) {
		    transferInfo->status = DtDND_FAILURE;
		    return;
		} else {
		    // We were able to open the container, so now we get the 
		    // current mailbox and copy all the messages into it.
		    mbox = rmw->mailbox();
		    mbox->copyMailBox(mail_error, tmpmbox);
		    rmw->checkForMail(mail_error);
		    delete tmpmbox;
		}
	    }
	    break;

	case DtDND_BUFFER_TRANSFER:

    	    // Loop through the buffers that were dropped onto the msg list

    	    for (ii = 0; ii < numItems; ii++) {

		// Turn the dropped data into a mail buffer to pass to
		// the mailbox constructor.
		DtMailBuffer buf;
		buf.buffer = (char *) transferInfo->dropData->data.buffers->bp;
		buf.size = transferInfo->dropData->data.buffers->size;

		// Convert the buffer into a mailbox object.
		tmpmbox = session->session()->mailBoxConstruct(mail_error,
						DtMailBufferObject, &buf,
						openCallback, NULL);

		// Reject the drop if we weren't able to convert it to a mailbox
		if (!tmpmbox || mail_error.isSet()) {
		    transferInfo->status = DtDND_FAILURE;
		    return;
		} else {
		    // Parse the dropped data into the tmpmbox.
		    tmpmbox->open(mail_error);
		    if (mail_error.isSet()) {
			transferInfo->status = DtDND_FAILURE;
			delete tmpmbox;
			return;
		    }
		    mbox = rmw->mailbox();
		    mbox->copyMailBox(mail_error, tmpmbox);
		    rmw->checkForMail(mail_error);
		    delete tmpmbox;
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
	    DtNtextIsBuffer, True,
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
    DtMail::MailBox	*mbox;
    DtMail::Message	*msg;
    DtMailMessageHandle msgHandle;
    DtMailBuffer 	*mbufList;
    int			*pos_list, pos_count, ii;
    unsigned long	bufSize = 0;
    char 		*bufPtr;
    DtMailEnv 		error;

    DebugPrintf(3, "In RoamMenuWindow:msgListConvertCallback\n");

    error.clear();

    switch (convertInfo->reason) {
	case DtCR_DND_CONVERT_DATA:

	    msgList = rmw->list();
	    listW = msgList->get_scrolling_list();
	    mbox = rmw->mailbox();

	    if (!XmListGetSelectedPos(listW, &pos_list, &pos_count)) {
		convertInfo->status = DtDND_FAILURE;
		return;
	    }

	    mbufList = (DtMailBuffer*)XtMalloc(pos_count*sizeof(DtMailBuffer));
	
	    for (ii = 0; ii < pos_count; ii++) {
		    msgHandle = msgList->msgno(pos_list[ii]);
		    msg = mbox->getMessage(error, msgHandle);
		    if (error.isSet()) {
			convertInfo->status = DtDND_FAILURE;
			return;
		    }
		    msg->toBuffer(error, mbufList[ii]);
		    if (error.isSet()) {
			convertInfo->status = DtDND_FAILURE;
			return;
		    }
		    bufSize += mbufList[ii].size + 1;
	    }

	    bufPtr = XtMalloc((unsigned int)bufSize);

	    buffer->bp = bufPtr;
	    buffer->size = (int)bufSize;
	    buffer->name = "Mailbox";

	    for (ii = 0; ii < pos_count; ii++) {
		strncpy(bufPtr, (char *)mbufList[ii].buffer, 
				(unsigned int)mbufList[ii].size);
		bufPtr += mbufList[ii].size;
		bufPtr[0] = '\n';
		bufPtr++;
		delete mbufList[ii].buffer;
	    }

	    XtFree((char *)mbufList);
	    XtFree((char *)pos_list);
	    break;

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
    DtMailEnv 		mail_error;
    int			ii;

    DebugPrintf(3, "In RoamMenuWindow::msgListDragFinishCallback\n");

    mail_error.clear();

    // Re-enable drops on this message list
    if (rmw->mailbox()->mailBoxWritable(mail_error) == DTM_TRUE) {
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
    DtMailEnv 		mail_error;

    DebugPrintf(3, "In RoamMenuWindow::msgListProcessDrag\n");

    mail_error.clear();

    XtVaGetValues(widget, XmNuserData, &rmw, NULL);

    if (rmw == NULL)
	return NULL;

    drag_icon = rmw->getDragIcon(widget);
    //drag_icon = NULL;

    rmw->msgListDropDisable();

    // Choose the drag operations based on the writeability of the mailbox

    if (rmw->mailbox()->mailBoxWritable(mail_error) == DTM_TRUE) {
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
	DebugPrintf(3, "DragStart returned NULL.\n");
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

    DebugPrintf(3, "In RoamMenuWindow::msgListDragSetup\n");

    XtAppAddActions(theRoamApp.appContext(), actionTable, 1);
    new_translations = XtParseTranslationTable(translations);
    XtOverrideTranslations(msgList, new_translations);

    XtVaGetValues(
	(Widget)XmGetXmDisplay(XtDisplayOfObject(msgList)),
	"enableBtn1Transfer", &btn1_transfer,
	NULL);
    
    if (btn1_transfer != True) {
	new_translations = XtParseTranslationTable(btn2_translations);
	XtOverrideTranslations(msgList, new_translations);
    }
}

void
RoamMenuWindow::open_and_load(
    DtMailEnv &error,
    DtMailBoolean create,
    DtMailBoolean lock
)
{

    _openContainerCmd->set_create_lock_flags(create, lock);
    _openContainerCmd->execute();

    // if it required conversion, let conversion handle the error
    // (its a callback routine and we have "lost control" of what
    // it returns anyway.)
    // 
    // if it did not require conversion, then open() should have succeeded.
    // If it did succeed, then its _mailbox should be set by now.
    // If its not set, then either open() failed somewhere or it was
    // cancelled by the user.  We now have an error condition.

    if ((!_requiredConversion) &&
	(_mailbox == NULL)) {
	error.setError(DTME_NoMailBox);
	return;
    }

    // If it required conversion, then let the conversion process handle
    // errors if any occur.
    // Reset the DtMailEnv for this method.

    if (_requiredConversion) {
	error.clear();
    }

    // If it required conversion, and conversion succeeded, then the
    // mailbox was also loaded and _is_loaded gets set to TRUE.
    // If it required no conversion, then it implies that we already
    // have a container in hand and we just load it (implicitly
    // setting _is_loaded to TRUE)

    if ((!_requiredConversion) &&
	(_is_loaded == FALSE)) {

	this->load_mailbox(error);
	_is_loaded = TRUE;

    }
}

// Ideally, open() should set the error if the user cancels the open.  
// And we should error after open() returns at the caller's end...

void
RoamMenuWindow::open(
    DtMailEnv & error, 
    DtMailBoolean create_flag, 
    DtMailBoolean lock_flag
)
{
    FORCE_SEGV_DECL(char, tmp);
    Dimension win_x, win_y, win_wid, win_ht, win_bwid;
    MailSession *ses = theRoamApp.session();
    char	buf[512];
    int answer = 0;
    char *helpId;
    int already_open = 0;

// why are we calling XmUpdateDisplay here?  The RMW will be displayed
// very shortly.
//   XmUpdateDisplay(this->baseWidget());
    _openContainerCmd->set_create_lock_flags(create_flag, lock_flag);

    // Obtain the current dimensions of the RMW

    XtVaGetValues(_main,
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

    // Check to see if the mbox is already open.  If it is, we will
    // simply map-raise the original RoamMenuWindow that opened it.
    if (ses->isMboxOpen(_mailbox_fullpath)) {
	already_open = 1;
    }

    // Try to open this folder, but don\'t take the lock and
    // and don\'t create it.
    _mailbox = ses->open(error, _mailbox_fullpath, 
			 &RoamMenuWindow::syncViewAndStoreCallback,
			 this, create_flag, lock_flag);

    if (already_open && _mailbox) {
	DtMailEnv mail_error;
	RoamMenuWindow *rmw = ses->getRMW(_mailbox);
	Widget w = rmw->baseWidget();

	XMapRaised(XtDisplay(w), XtWindow(w));
	mail_error.clear();
	ses->close(mail_error, _mailbox);
	_mailbox = NULL;
    }

    if (error.isSet()) {
	
	// Did we ask for a non-existent file?
	if ((DTMailError_t)error == DTME_NoSuchFile) {
	    if (create_flag == DTM_TRUE) {
		sprintf(buf,
			catgets(DT_catd, 
				3, 3, 
				"The mailbox %s does not exist.\n\
Create a mailbox with this name?"),
			_mailbox_fullpath);
		_genDialog->setToQuestionDialog(catgets(DT_catd, 
							3, 4, "Mailer"),
					     buf);
		helpId = DTMAILHELPERROR;
		answer = _genDialog->post_and_return(catgets(DT_catd, 3, 5, "OK"), 
				     catgets(DT_catd, 1, 5, "Cancel"), helpId);
		if (answer == 1) {
		    open(error, DTM_TRUE, lock_flag);
		}
	    }
	    else {
		// This looks like a bug!  We are doing the same 
		// thing for both cases in the if statement.
		open(error, DTM_TRUE, lock_flag);
	    }
	}
	else if (((DTMailError_t)error == DTME_OtherOwnsWrite) ||
	         ((DTMailError_t)error == DTME_AlreadyLocked)) {
	    // See if they want to take the lock.
// 	    sprintf(buf, 
// 		    catgets(DT_catd, 3, 6, "The mailbox %s is locked.\n\
// You can manually unlock the mailbox and try again\n\
// or contact your System Administrator."),
// 		   _mailbox_fullpath);

// 	    _genDialog->setToErrorDialog(
// 				catgets(DT_catd, 3, 7, "Mailer"),
// 				buf);
// 	    helpId = DTMAILHELPTAKELOCK;
// 	    _genDialog->post_and_return(
// 				catgets(DT_catd, 3, 8, "OK"),
// 				helpId);

// 	    error.setError(DTME_GetLockRefused);
	    // The above else-if code doesn't make sense.  If ses->open() failed
	    // because the folder was already locked or not writable then 
	    // posting the specified error dialog and setting the error to GetLockRefused
	    // is meaningless.  Especially since the calling function 
	    // OpenContainerCmd::doit() doesn't even check the error we 
	    // return.  Lets post a meaningful error dialog and return.
	    postErrorDialog(error);
	    return;
	}
	else if ((DTMailError_t)error == DTME_BadRunGroup) {

	    sprintf(buf, catgets(DT_catd, 2, 4,
		   "Mailer has not been properly installed,\n\
and cannot run because the execution group\n\
is incorrectly set."));

	    _genDialog->setToQuestionDialog(
				catgets(DT_catd, 1, 6, "Mailer"),
				buf);
	    
	    // No choice at this state other than to OK.

	    helpId = DTMAILHELPBADGROUPID;
	    answer = _genDialog->post_and_return(
				catgets(DT_catd, 3, 9, "OK"),
				helpId);
	    return;

	}
	else if ((DTMailError_t)error == DTME_NoPermission) {
	    /* NL_COMMENT
	     * The %s is the name of the mailbox the user doesn't have
	     * permission to view.
	     */
	    sprintf(buf, catgets(DT_catd, 2, 5,
		   "You do not have permission to view %s"), _mailbox_fullpath);

	    _genDialog->setToQuestionDialog(
				catgets(DT_catd, 1, 7, "Mailer"),
				buf);
	    
	    // No choice at this state other than to OK.

	    helpId = DTMAILHELPNOVIEW;
	    answer = _genDialog->post_and_return(
				catgets(DT_catd, 3, 10, "OK"),
				helpId);
	    return;
	}
	else if ((DTMailError_t)error == DTME_IsDirectory) {
	    sprintf(buf, catgets(DT_catd, 2, 6,
		    "The mailbox %s is a directory and can not be opened."),
		    _mailbox_fullpath);

	    _genDialog->setToQuestionDialog(catgets(DT_catd, 1, 8, "Mailer"),
					    buf);

	    helpId = DTMAILHELPDIRECTORYONLY;
	    answer = _genDialog->post_and_return(catgets(DT_catd, 3, 11, "OK"),
						 helpId);
	    return;
	}
	else if ((DTMailError_t)error == DTME_NotMailBox) {
	        this->postErrorDialog(error);
		error.setError(DTME_NotMailBox);
	}
	else {

	    // This is a real error. Punt!
	    this->postErrorDialog(error);
	}
    }

    if (_mailbox && _mailbox->mailBoxWritable(error) == DTM_TRUE) {
	msgListDropRegister();
    }

    return;
}

void RoamMenuWindow::createMenuPanes()
{
    this->construct_file_menu();

    this->construct_message_menu();

    this->construct_compose_menu();

    this->construct_view_menu();

    this->construct_options_menu();

    this->construct_move_menu();

    this->construct_help_menu();

    this->construct_message_popup();

    this->construct_text_popup();

    this->construct_attachment_popup();

    this->create_toolbar_cmdlist();
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
    _toolbar_inactive_icons = new DtVirtArray<char *>(4);

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

#ifdef AUTOFILING
    _toolbar_cmds->add( _mailbox_list );
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
	_menuBar->removeCommand(_moveMenu, _first_cached_move_item+i-1);
	_menuBar->removeCommand(_copyMenu, _first_cached_copy_item+i-1);
	_menuBar->removeCommand(_msgsPopupMoveMenu, _first_cached_copy_item+i-1);
    }
    _cached_list_size = new_size;
    
}

void
RoamMenuWindow::propsChanged(void)
{
    FORCE_SEGV_DECL(DtMail::Session, m_session);
    DtMail::MailRc * mailrc = get_mail_rc();
    char *value = NULL;
    DtMailEnv error;

    // See if the header size has changed.
    //
    mailrc->getValue(error, "headerlines", (const char **)&value);
    if (error.isSet() || value == NULL || *value == NULL) { 
	if (value) free(value);
	value = strdup("15");
	error.clear();
    }

    int header_lines = (int) strtol(value, NULL, 10);
    if (header_lines != _list->visibleItems())
	_list->visibleItems(header_lines);

    _list->checkDisplayProp();

    // Set the default directories for _open and _new_container.
    free(value);

    m_session = theRoamApp.session()->session();

    char * full_dirname = theRoamApp.getFolderDir(TRUE);
    _new_container->setDirectory(full_dirname);
    _open->setDirectory(full_dirname);
    free(full_dirname);
    value = NULL;

    mailrc->getValue(error, "dontdisplaycachedfiles", (const char **)&value);
    if (error.isNotSet() && value != NULL) {
        // dontdisplaycachedfiles is set, so dont display cached files
        if (_cached_list_size != 0)
		// They just turned off the Display Up To prop so
	  	//  reset the cache list 
		resetCacheList(0);
	_max_cached_list_size = 0;
    }
    else {
        error.clear();
        if (value) free(value);
	value = NULL;
        mailrc->getValue(error, "cachedfilemenusize", (const char **)&value);
        if (error.isNotSet() && value != NULL && *value != '\0') {
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
    }
    error.clear(); 
    if (value) free(value);
    value = NULL;
    mailrc->getValue(error, "filemenu2", (const char **)&value);
    char *folder = theRoamApp.getFolderDir(TRUE);
    if  ( ((value == NULL && _filemenu2 != NULL) ||
	  (value != NULL && 
	  (_filemenu2 == NULL || strcmp(value, _filemenu2)) != 0))
		|| strcmp(folder, _folder) != 0 ) {
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
			addToRecentList(name);
		}
		delete save_cached_list;
    }

    error.clear(); 
    if (value) free(value);
    value = NULL;
    mailrc->getValue(error, "toolbarcommands", (const char **)&value);
    //
    // If error.isSet() there is no need to rebuild the toolbar because
    // the user has not modified the toolbar commands.  We know this
    // because even if you
    // delete all the commands from the toolbar, it still writes out
    // an empty toolbarcommand string (like "set toolbarcommands").
    //
    if (error.isNotSet()) {
	if  ((value == NULL && _toolbar_mailrc_string != NULL) ||
	     (value != NULL && (_toolbar_mailrc_string == NULL ||
				strcmp(value, _toolbar_mailrc_string)))) {
	    buildToolbar();
	} else {
	    error.clear();
	    if (value) free(value);
	    value = NULL;
	    mailrc->getValue(error, "toolbarusetext", (const char **)&value);
	    if(((error.isNotSet() && (strcmp(value, "")==0))
		&& _toolbar_use_icons)) {
		buildToolbar();
	    } else if ((error.isSet() || strcmp(value, "f") == 0 || !value) &&
		       !_toolbar_use_icons) {
		buildToolbar();
	    }
	}
    } else {
	error.clear();
	if (value) free(value);
	value = NULL;
	mailrc->getValue(error, "toolbarusetext", (const char **)&value);
	if(((error.isNotSet() && (strcmp(value, "")==0))
	    && _toolbar_use_icons)) {
	    buildToolbar();
	} else if ((error.isSet() || strcmp(value, "f") == 0 || !value) &&
		   !_toolbar_use_icons) {
	    buildToolbar();
	}
    }

    if (value) free(value);
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
    Widget,		// w
    XtPointer clientData,
    XEvent *event,
    Boolean * )		// continue_to_dispatch
{

   if (event->type == ConfigureNotify) {

       RoamMenuWindow *window=(RoamMenuWindow *) clientData;

       window->configurenotify( event->xconfigurerequest.x,
				event->xconfigurerequest.y,
				event->xconfigurerequest.width,
				event->xconfigurerequest.height,
				event->xconfigurerequest.border_width );
   }
   else if ((event->type == MapNotify) || ( event->type == UnmapNotify)) {

       RoamMenuWindow *window=(RoamMenuWindow *) clientData;

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

// If it is not already loaded, then load it (it might involve
// conversion, etc.; all handled by open_and_load())
// If its been loaded already, then mapnotify gets called when
// the state changes from iconic to open (i.e., the user double-clicks
// on an RMW icon).  If we want to load a folder at that time, this
// is the place to do it.


void
RoamMenuWindow::mapnotify()
{
    // If its not been loaded, then open and load it.
    
    if (!_is_loaded) {
	
	DtMailEnv mail_error;
	
	// Initialize the mail_error.
	mail_error.clear();
	
	theRoamApp.busyAllWindows();

	this->open_and_load(mail_error, DTM_FALSE, DTM_TRUE);
	
	theRoamApp.unbusyAllWindows();
	
	// If there's been an error then we quit the container.
 	
 	if (mail_error.isSet()) {
 	    
	    // Need to remove the base Widgets destroy callback since
	    // we end up destroying it twice otherwise...
 		
	    XtRemoveAllCallbacks (
			this->baseWidget(),
			XmNdestroyCallback);
	    this->quit();
 	}
	else {
	    // We need to disable the editable menu options if the mail
	    // box is readonly.
	    //
	    if (_mailbox->mailBoxWritable(mail_error) == DTM_FALSE) {
		_delete_msg->deactivate();
		_undeleteLast->deactivate();
		_undeleteFromList->deactivate();
		_destroyDeletedMessages->deactivate();
		
		char * new_title = new char[strlen(_mailbox_fullpath) + 30];
		strcpy(new_title, _mailbox_fullpath);
		strcat(new_title, " [Read Only]");
		this->title(new_title);
		delete new_title;
	    }

	    if (_list->get_num_messages()) {
		setIconName(NormalIcon);
	    }
	    else {
		setIconName(EmptyIcon);
	    }
	}
    }
    else {
	// If the mailbox has messages, set to normal icon
	if (_list->get_num_messages() > 0) {
	    setIconName(NormalIcon);
	}
	else {
	    setIconName(EmptyIcon);
	}
    }
    DtbOptionsDialogInfo oHandle =
	(DtbOptionsDialogInfo)theRoamApp.optionsDialog();
    OptCmd *oc = (OptCmd*)theRoamApp.mailOptions();
    if (oHandle && oc && oc->windowIsAlive()) {
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

void
RoamMenuWindow::message( char *text )
{
    char buf[MAX_STATUS_LEN];
    XmString labelStr;

    memset(buf, 0, MAX_STATUS_LEN);
    if (!text) {
        text = "";
    }
    if (strlen(text) >= MAX_STATUS_LEN) {
        strncpy(buf, text, MAX_STATUS_LEN - 1);
        buf[MAX_STATUS_LEN - 1] = '\0';
    } else {
        sprintf(buf, "%s", text);
    }
    labelStr = XmStringCreateLocalized(buf);

    XtVaSetValues(_message, XmNlabelString, labelStr, NULL);
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
//    setStatus("");
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
    char buf[MAX_STATUS_LEN], *str;
//    char *buf, *str; 
    XmString labelStr;
    int num_live_msgs = num_msgs - num_deleted;  // Undeleted msgs
//     DtMail::MailRc * mailrc = get_mail_rc();
//     DtMailEnv error;
//     char * value = NULL;

    str = catgets(DT_catd, 3, 13, "Message %d of %d, %d new, %d deleted"); 

//    buf = new char[strlen(str) + 100];
    sprintf(buf, str, sel_pos, num_live_msgs, num_new, num_deleted);

    labelStr = XmStringCreateLocalized(buf);
    
    XtVaSetValues(_message_summary, XmNlabelString, labelStr, NULL);
//    XmUpdateDisplay(this->baseWidget());

    XmStringFree(labelStr);
//    delete buf;
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

void 
RoamMenuWindow::quit()
{
    int i = 0;
    DtMailBoolean keep = DTM_FALSE;
    DtMail::MailRc *mailrc = NULL;
    char * value=NULL;
    DtMailEnv error;

    theRoamApp.busyAllWindows(catgets(DT_catd, 3, 15, "Saving..."));

    if (_mailbox) {
	mailrc = get_mail_rc();
    }

    if (_list->get_num_deleted_messages()) {
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
	error.clear();
	mailrc->getValue(error, "keepdeleted", (const char **)&value);
        if (value) free (value);
        value = NULL;
	if (error.isSet()) {
	    // The user wants to expunge on close. See if they want
	    // to be asked first.
	    //
	    error.clear();
	    mailrc->getValue(error, "quietdelete", (const char **)&value);
            if (value) free (value);
	    if (error.isSet() && !theRoamApp.killedBySignal()) {
		if (isIconified()) {
		    this->manage();
		}
		/* NL_COMMENT
		 * This dialog comes up when the user tries to quit the
		 * mailbox and the user is asked if he wants to destroy
		 * the messages marked for deletion.
		 */
		/* NL_COMMENT
		 * Messages 16 and 17 are no longer being used.  They are
		 * being replaced by message 86 and 87.
		 */
		_genDialog->setToQuestionDialog(
#ifdef undef
		    catgets(DT_catd, 3, 16, "Mailer"),
		    catgets(DT_catd, 3, 17, "Destroy the messages you have marked\nfor deletion in this mailbox?"));
#endif // undef
		/* NL_COMMENT
		 * This dialog comes up when the user tries to quit the
		 * mailbox.  The user is asked if they want to destroy
		 * the deleted messages.
		 */
		    catgets(DT_catd, 3, 87, "Mailer - Close"),
		    catgets(DT_catd, 3, 88, "Destroy the deleted messages and close this mailbox?"));
		char * helpId = DTMAILHELPDESTROYMARKMSG;
		int answer = _genDialog->post_and_return(
#ifdef undef
				catgets(DT_catd, 3, 79, "Destroy and Quit"),
				catgets(DT_catd, 3, 80, "Don't destroy, Quit"),
#endif // undef
				catgets(DT_catd, 3, 89, "Destroy and Close"),
				catgets(DT_catd, 3, 73, "Cancel"),
				catgets(DT_catd, 3, 90, "Retain and Close"),
				helpId);
		if (answer == 1) {
		    error.clear();
		    _mailbox->expunge(error);
		    if (error.isSet()) {
			this->postErrorDialog(error);
		    }
		} else if (answer == 2) {
		    // This is a very bad way to code selection of the 
		    // cancel button.  If someone changes its position
		    // in the dialog, this code will break!
		    theRoamApp.unbusyAllWindows();
		    return;
		}
	    } else {
		// If killed by a signal, don't post a dialog.
		error.clear();
		_mailbox->expunge(error);
		if (error.isSet()) {
		    if ( !theRoamApp.killedBySignal() ) {
			this->postErrorDialog(error);
		    }
		}
	    }
	}
    }

    // We need to make sure that not only are there no more messages
    // in this mailbox, but that there are no more deleted messages
    // in this mailbox.  This is because even if the messages are
    // deleted, if the user didn't do a destroy deleted, we still
    // want them to be able to do an undeleted the next time they
    // bring up the mailbox.

    if (_mailbox) {
	if ((_list->get_num_messages() == 0) && (_list->get_num_deleted_messages() == 0)) {
	    mailrc->getValue(error, "keep", &value);
	    if (error.isNotSet()) {
		keep = DTM_TRUE;
		if (value != NULL) {
		    free (value);
		    value = NULL;
		}
	    }
	    if (!keep) {
		SafeUnlink(_mailbox_fullpath);
	    }
	}
    }
	    


    for (i = 0; i < _numDialogs; i++) {
	_dialogs[i]->unmanage();
    }

//    XmUpdateDisplay(this->baseWidget());

    for (i = 0; i < _numDialogs; i++) {
	_dialogs[i]->quit();
    }

    _list->shutdown();

    // Quitting the last container window and 
    // all Compose windows are unmapped.
    if ( (theApplication->num_windows() - 1) == 
	      theCompose.numNotInUse() ) {
		theCompose.~Compose();
    }

    this->unmanage();

    delete this;

    theRoamApp.unbusyAllWindows();
}


void 
RoamMenuWindow::quit_silently()
{
    int i = 0;
    DtMail::MailRc * mailrc = NULL;
    char * value=NULL;
    DtMailEnv error;
    DtMailBoolean keep = DTM_FALSE;

    theRoamApp.busyAllWindows(catgets(DT_catd, 3, 15, "Saving..."));

    if (_mailbox) {
	mailrc = get_mail_rc();
    }

    if (_list->get_num_deleted_messages()) {
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

	error.clear();
	mailrc->getValue(error, "keepdeleted", (const char **)&value);
	if (value) free (value);
	if (error.isSet()) {
	    error.clear();
	    _mailbox->expunge(error);
	    if (error.isSet()) {
		this->postErrorDialog(error);
	    }
 	}
    }

    if ((_list->get_num_messages() == 0) && (_list->get_num_deleted_messages() == 0)) {
	mailrc->getValue(error, "keep", &value);
	if (error.isNotSet()) {
	    keep = DTM_TRUE;
	    if (value != NULL) {
		free (value);
		value = NULL;
	    }
	}
	if (!keep) {
	    SafeUnlink(_mailbox_fullpath);
	}
    }

    for (i = 0; i < _numDialogs; i++) {
	_dialogs[i]->unmanage();
    }

//    XmUpdateDisplay(this->baseWidget());

    for (i = 0; i < _numDialogs; i++) {
	_dialogs[i]->quit();
    }

    _list->shutdown();

    // Quitting the last container window and 
    // all Compose windows are unmapped.
    if ((theApplication->num_windows() - 1) == theCompose.numNotInUse()) {
		XtRemoveAllCallbacks(theApplication->baseWidget(), XmNdestroyCallback);
      theCompose.~Compose();
    }

    this->unmanage();

    delete this;

    theRoamApp.unbusyAllWindows();
}

// Callback to open a new mail container.

void
RoamMenuWindow::file_selection_callback(
    void *client_data,
    char *selection
)
{
    // If there is a selection, do something.

    if (strlen(selection) > 0) {
	RoamMenuWindow *obj=(RoamMenuWindow *) client_data;
	obj->mail_file_selection( selection );
    }
}

// Given the name of a container, create a new RoamMenuWindow
// and open the container into it.

void
RoamMenuWindow::mail_file_selection(
    char *selection
)
{
    theRoamApp.busyAllWindows(catgets(DT_catd, 3, 20, "Opening mailbox..."));

    RoamMenuWindow *mailview=new RoamMenuWindow(selection);

    mailview->initialize();

    theRoamApp.unbusyAllWindows();
}

void
RoamMenuWindow::move_callback(
    void *client_data,
    char *selection
)
{
    DtMailEnv mail_error;

    // Initialize the mail_error.
    mail_error.clear();

    RoamMenuWindow *obj = (RoamMenuWindow *) client_data;

    // Passing TRUE to copySelected tells the routine to delete
    // the selected messages after copying them. Trailing FALSE
    // means don't be silent
    obj->_list->copySelected(mail_error, selection, TRUE, FALSE);
    if (mail_error.isSet()) {
	obj->postErrorDialog(mail_error);
    }

}

void
RoamMenuWindow::copy_callback(
    void *client_data,
    char *selection
)
{
    DtMailEnv mail_error;

    // Initialize the mail_error.
    mail_error.clear();

    RoamMenuWindow *obj = (RoamMenuWindow *) client_data;
    obj->_list->copySelected(mail_error, selection, FALSE, FALSE);
    if (mail_error.isSet()) {
	obj->postErrorDialog(mail_error);	
    }
}


void
RoamMenuWindow::create_container_callback(
    void *client_data,
    char *selection
)
{
    RoamMenuWindow *obj=(RoamMenuWindow *) client_data;
    obj->create_new_container( selection );
}

void
RoamMenuWindow::create_new_container(
    char *filename
)
{
    int answer;
    char buf[2048];
    char * helpId;

    if (SafeAccess(filename, F_OK) == 0) {

	sprintf(buf, catgets(DT_catd, 3, 21, "%s already exists.\n\
Overwrite?"),
		filename);
	_genDialog->setToQuestionDialog(
			catgets(DT_catd, 3, 22, "Mailer"),
			buf);
	helpId = DTMAILHELPERROR;
	answer = _genDialog->post_and_return(helpId);

	if (answer == 2) { // Pressed cancel
	    return;
	}

	if (unlink(filename) < 0) {
	    sprintf(buf, 
		catgets(DT_catd, 3, 23, "Unable to overwrite %s.\nCheck file permissions and retry."), 
		filename);

	    _genDialog->setToErrorDialog(
			catgets(DT_catd, 3, 24, "Mailer"),
			buf);
	    helpId = DTMAILHELPERROR;
	    answer = _genDialog->post_and_return(helpId);
	    return;
	}
    }
    
    // Path filename is ok -- now follow the same route as for Open

    this->mail_file_selection(filename);

}

// SR - added methods below

ViewMsgDialog*
RoamMenuWindow::ifViewExists(DtMailMessageHandle msg_num)
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
    FORCE_SEGV_DECL(DtMail::Session, m_session);
    FORCE_SEGV_DECL(CmdInterface, ci);
    DtMailEnv error;
    char *value=NULL,*toolbarCmds=NULL,*token;
    char cmd_name[64];
    int useicons=0, len, i, toolbar_cmd_count=0;
    Widget w;
    Pixmap glyph, inactive_glyph;
    Pixel fg, bg;

    XtVaSetValues(rowOfButtons,
		  XmNwidth, _width,
		  NULL);
    error.clear();
    m_session = theRoamApp.session()->session();

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
    m_session->mailRc(error)->getValue(error, "toolbarcommands", (const char **)&toolbarCmds);
    if (error.isSet() && _toolbar_mailrc_string == NULL) {
	toolbarCmds = strdup("delete next previous reply,_include_message forward new_message print...");
	_toolbar_mailrc_string = strdup(toolbarCmds);
	error.clear();
    } else if (error.isNotSet() && !strcmp(toolbarCmds, "")) {
	if (_toolbar_mailrc_string)
	    free(_toolbar_mailrc_string);
	_toolbar_mailrc_string = NULL;
	toolbarCmds = NULL;
    } else {
	if (_toolbar_mailrc_string)
	    free(_toolbar_mailrc_string);
	_toolbar_mailrc_string = strdup(toolbarCmds);
    }

    m_session->mailRc(error)->getValue(error, "toolbarusetext", (const char **)&value);
    if (error.isSet() || (strcmp(value, "f") == 0) || value == NULL) {
	error.clear();
	_toolbar_use_icons = True;
    } else if (strcmp(value, "") == 0) {
	_toolbar_use_icons = False;
    }
    if (value) free(value);
    value = NULL;

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
				      False, &RoamMenuWindow::toolbarEH,
				      (XtPointer) this);
		}
		ci->manage();
		continue;
	    }
	}
	// Get the next command from the toolbarCmds string.
	token = strtok((char *)0, " ");
    }
    free(toolbarCmds);
}


void
RoamMenuWindow::toolbarEH( Widget w,
			     XtPointer client_data,
			     XEvent *event,
			     Boolean *)
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
    Widget l1, l2, l3, l4;

    labelStr = XmStringCreateLocalized(catgets(DT_catd, 1, 14,
	   "Sender"));

    l1 = XtVaCreateManagedWidget("Label1", xmLabelGadgetClass, 
		rowOfLabels,
		XmNlabelString, labelStr,
		NULL);
	
    XmStringFree(labelStr);
    printHelpId("l1", l1);
    /* add help callback */
    // XtAddCallback(l1, XmNhelpCallback, HelpCB, helpId);


    labelStr = XmStringCreateLocalized(catgets(DT_catd, 1, 15,
	   "Subject"));
    l2 = XtVaCreateManagedWidget("Label2", 
				xmLabelGadgetClass, rowOfLabels,
				XmNlabelString, labelStr,
				NULL);


    XmStringFree(labelStr);
    printHelpId("l2", l2);
    /* add help callback */
    // XtAddCallback(l2, XmNhelpCallback, HelpCB, helpId);

    labelStr = XmStringCreateLocalized(catgets(DT_catd, 1, 16,
	   "Date and Time"));
    l3 = XtVaCreateManagedWidget("Label3", 
			xmLabelGadgetClass, rowOfLabels,
			XmNlabelString, labelStr,
			NULL);

    XmStringFree(labelStr);
    printHelpId("l3", l3);
    /* add help callback */
    // XtAddCallback(l3, XmNhelpCallback, HelpCB, helpId);

    labelStr = XmStringCreateLocalized(catgets(DT_catd, 1, 17,
	   "Size"));
    l4 = XtVaCreateManagedWidget("Label4", 
		xmLabelGadgetClass, rowOfLabels,
		XmNlabelString, labelStr,
		NULL);

    XmStringFree(labelStr);
    printHelpId("l4", l4);
    /* add help callback */

    // Adjust labels so the align on the columns
    msglist->layoutLabels(msgnums, l1, l2, l3, l4);

    return;
}

void
RoamMenuWindow::addToRowOfMessageStatus()
{
    XmString labelStr1, labelStr2;

    // Size of first label
    
    labelStr1 = XmStringCreateLocalized(catgets(DT_catd, 3, 25,
	   "Loading container..."));

    labelStr2 = XmStringCreateLocalized(
			catgets(DT_catd, 3, 26, "Folder Summary Information"));

    _message = XtCreateManagedWidget(
			"Message_Status_Text", xmLabelWidgetClass,
			rowOfMessageStatus, NULL, 0);
    printHelpId("_message", _message);
    /* add help callback */
    // XtAddCallback(_message, XmNhelpCallback, HelpCB, helpId);


     XtVaSetValues(_message,
	XmNalignment, XmALIGNMENT_BEGINNING,
	XmNleftAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
 	XmNlabelString, labelStr1,
 	NULL );
    
    _message_summary = XtCreateManagedWidget("Message_Summary", 
					xmLabelWidgetClass,
				        rowOfMessageStatus, NULL, 0);

     XtVaSetValues(_message_summary,
	XmNalignment, XmALIGNMENT_END,
 	XmNlabelString, labelStr2,
 	XmNrightAttachment, XmATTACH_FORM,
 	NULL );



     XtVaSetValues(_message,
	 XmNrightAttachment, XmATTACH_WIDGET,
	 XmNrightWidget, _message_summary,
	 XmNrightOffset, 10,
	 NULL);

     XmStringFree(labelStr1);
     XmStringFree(labelStr2);
 
}

void
RoamMenuWindow::construct_file_menu()
{
    FORCE_SEGV_DECL(CmdList, cmdList);
    FORCE_SEGV_DECL(CmdList, openCmdlist);
    FORCE_SEGV_DECL(DtMail::Session, m_session);
    DtMailEnv error;
    char *bufptr=NULL, *expanded_dir;

    m_session = theRoamApp.session()->session();

    // Create the "Container" item in the menubar.  And fill 
    // with items below the "Container" item in the menubar.
    
    cmdList = new CmdList( "Mailbox", catgets(DT_catd, 1, 18, "Mailbox") );

    if (!_check_new_mail)
    	_check_new_mail	= new CheckForNewMailCmd(
				"Check for New Mail",
				catgets(DT_catd, 1, 19, "Check for New Mail"), 
				TRUE, 
				this);

    if (!_msg_select_all)
    	_msg_select_all = new SelectAllCmd (
                                "Select All Messages",
                                catgets(DT_catd, 1, 36, "Select All Messages"),
                                TRUE, this );

    if (!_new_container)
    	_new_container     = new SelectFileCmd( 
				"New Mailbox",
				catgets(DT_catd, 1, 22, "New Mailbox..."),
				catgets(DT_catd, 1, 23, "Mailer - New Mailbox"),
				catgets(DT_catd, 1, 24, "New Mailbox"),
				TRUE,
				RoamMenuWindow::create_container_callback,
				this,
				this->baseWidget());

    openCmdlist = new CmdList( "Open Mailbox", catgets(DT_catd, 1, 27, "Open Mailbox") );

    bufptr=NULL;
    m_session->mailRc(error)->getValue(error, "filemenu2", (const char **)&bufptr);
    expanded_dir = NULL;
    if (error.isNotSet() && bufptr != NULL && *bufptr != NULL) 
        expanded_dir = m_session->expandPath(error, bufptr);

    if (expanded_dir) {
	char *token;
	OpenMailboxCmd *cmd;
        if ((token = (char *) strtok(expanded_dir, " "))) {
		cmd = new OpenMailboxCmd(strdup(token),
                                                token,
                                                TRUE,
                                                this,
                                                strdup(token));
            	openCmdlist->add(cmd);
            	while (token = (char *) strtok(NULL, " ")) {
		 	cmd = new OpenMailboxCmd(strdup(token),
                                                token,
                                                TRUE,
                                                this,
                                                strdup(token));
                	openCmdlist->add(cmd);
            	}
        }      
        free(expanded_dir);
    }
   
    // Get a handle to the Inbox.
    DtMailObjectSpace space;

    bufptr=NULL;
    m_session->queryImpl(error,
		 m_session->getDefaultImpl(error),
		 DtMailCapabilityInboxName,
		 &space,
		 &bufptr);
    if (!_open_inbox) 
    	if (this->inbox())  // Deactivate the Open Inbox item 
		_open_inbox	       = new OpenMailboxCmd(
					"Inbox",
					catgets(DT_catd, 1, 20, "Inbox"),
					FALSE,
					this,
					strdup(bufptr));

    	else  // Activate the Open Inbox item.
		_open_inbox	       = new OpenMailboxCmd(
					"Inbox",
					catgets(DT_catd, 1, 21, "Inbox"),
					TRUE,
					this,
					strdup(bufptr));


   if (!_logfile)
	_logfile	       = new OpenMailboxCmd(
				"Sent Mail",
				catgets(DT_catd, 1, 255, "Sent Mail"), 
				TRUE,
				this,
				theRoamApp.getResource(SentMessageLogFile, TRUE));

    _open_other_mboxes = new CmdList ( "Other Mailboxes",
			 catgets(DT_catd, 1, 252, "Other Mailboxes") );

    // This is for mapping the Move pullright menus dynamically...
    MenuClientData *mcd = (MenuClientData*) malloc (sizeof(MenuClientData));
    expanded_dir = theRoamApp.getFolderDir(TRUE);
    mcd->filename = expanded_dir;
    mcd->op = DTM_OPEN;
    mcd->checksum = -1;
    mcd->timestamp = 0;
    _open_other_mboxes->setClientData((void*)mcd);

    if (!_open)
    	_open              = new SelectFileCmd(
				"Select Mailbox",
				catgets(DT_catd, 1, 65, "Select Mailbox..."), 
				catgets(DT_catd, 1, 259, "Mailer - Select Mailbox"),
				catgets(DT_catd, 1, 30, "Open"),
				TRUE,
				RoamMenuWindow::file_selection_callback,
				this,
				this->baseWidget());

    openCmdlist->add(_separator);
    openCmdlist->add(_open_inbox);
    openCmdlist->add(_logfile);
    openCmdlist->add(_open);
    openCmdlist->add(_open_other_mboxes);

#ifdef AUTOFILING
    if (!_mailbox_list)
    	_mailbox_list              = new SelectFileCmd(
				"Show Mailboxes with New Mail",
				catgets(DT_catd, 1, 256, "Show Mailboxes with New Mail..."), 
				catgets(DT_catd, 1, 258, "Mailer - Show Mailboxes with New Mail List"),
				catgets(DT_catd, 1, 257, "Show Mailboxes with New Mail"),
				TRUE,
				RoamMenuWindow::file_selection_callback,
				this,
				this->baseWidget());

    // Set the default directories for _open and _new_container.
    _new_container->setDirectory(expanded_dir);
    _open->setDirectory(expanded_dir);

#endif // AUTOFILING

    if (!_destroyDeletedMessages)
    	_destroyDeletedMessages  = new DestroyCmd(
					"Destroy Deleted Messages",
					catgets(DT_catd, 1, 28, 
						"Destroy Deleted Messages"), 
					      TRUE,
					      this);


    if (!_quit)
    	_quit              = new QuitCmd (
				"Close",
				catgets(DT_catd, 1, 29, "Close"), 
				TRUE, 
				this);
    

    cmdList->add(_check_new_mail);
#ifdef AUTOFILING
    cmdList->add (_mailbox_list);
#endif // AUTOFILING
    cmdList->add(_msg_select_all);
    cmdList->add(_separator);
    cmdList->add(_new_container);
    cmdList->add(openCmdlist);
    cmdList->add(_separator);

    cmdList->add (_destroyDeletedMessages);
    cmdList->add(_separator);
    cmdList->add(_quit);

    _menuBar->addCommands (&_file_cascade, cmdList, FALSE, XmMENU_BAR);

    Widget cascade = openCmdlist->getCascadeWidget();
    Widget menu = openCmdlist->getPaneWidget();

    XtVaSetValues(menu,
                  XmNpacking, XmPACK_COLUMN,
                  XmNorientation, XmVERTICAL,
                  NULL);

    XtVaSetValues(cascade, XmNuserData, this, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_open_menu, 
			menu);

    // For mapping the dynamic pullright menus from the Other Mailboxes...
    cascade = _open_other_mboxes->getCascadeWidget();
    XtVaSetValues(cascade, XmNuserData, this, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, 
		_open_other_mboxes);

    delete cmdList;
}

void
RoamMenuWindow::createCopyList(CmdList * copy_to)
{
    FORCE_SEGV_DECL(DtMail::Session, m_session);
    DtMailEnv error;
    const char *val = NULL, *cached_containerlist_size = NULL, *display_cfs = NULL;
    char *token, *expanded_val=NULL;
    DtMailBoolean	user_list = DTM_FALSE;
    MoveCopyMenuCmd	*copy_container;

    // Get names for permanent containers from .mailrc and fill in
    // the menu.  We get the items from the "filemenu2" variable, and
    // we no longer read "filemenu", which was used by V2 mailtool and
    // kept along in V3 mailtool for backward compatibility.
    if (_filemenu2 != NULL)
	free (_filemenu2);
    m_session = theRoamApp.session()->session();
    m_session->mailRc(error)->getValue(error, "filemenu2", &val);
    if (error.isNotSet() && val != NULL && *val != '\0') {
	user_list = DTM_TRUE;
    	expanded_val = m_session->expandPath(error, val);
        _filemenu2 = (char *) val;
    }
    else _filemenu2 = NULL;
    if (_folder)
	free(_folder);
    _folder = theRoamApp.getFolderDir(TRUE);

    // We will user _copy_containerlist and cached_copy_containerlist to keep
    // track of the commands associated with the Copy menu items.

    if (_copy_containerlist != NULL)
	delete (_copy_containerlist);
    _copy_containerlist = new DtVirtArray<MoveCopyMenuCmd *> (3);

    m_session->mailRc(error)->getValue(error, "dontdisplaycachedfiles",
						&display_cfs);
    if (error.isNotSet() && display_cfs != NULL)
        // dontdisplaycachedfiles is set, so dont display cached files
	_max_cached_list_size = 0;
    else {
	error.clear();
        // Find out how large the recently used container list can be.
	m_session->mailRc(error)->getValue(error, 
		"cachedfilemenusize", &cached_containerlist_size);
	if (error.isNotSet() && cached_containerlist_size && 
		*cached_containerlist_size)
		_max_cached_list_size = 
			(int) strtol(cached_containerlist_size, NULL, 10);
	else 
		_max_cached_list_size = 10;
    }

    _cached_copy_containerlist = new
		DtVirtArray<MoveCopyMenuCmd *>
		(_max_cached_list_size ? _max_cached_list_size : 1);

    if (user_list == DTM_TRUE) {
	// Create arrays to hold the user defined container list and the
	// recently visited (cached) container list.

	if ((token = (char *) strtok(expanded_val, " "))) {
	    // add MoveCopyMenuCmd to the array, passing in the token.
	    copy_container = new MoveCopyMenuCmd(strdup(token),
						token,
						TRUE,
						this,
						DTM_COPY);
	    copy_to->add(copy_container);
	    _copy_containerlist->append(copy_container);
	    while (token = (char *) strtok(NULL, " ")) {
		// add MoveCopyMenuCmd to the array, passing in the token.
		copy_container = new MoveCopyMenuCmd (strdup(token),
							token,
							TRUE,
							this,
							DTM_COPY);
		copy_to->add(copy_container);
		_copy_containerlist->append(copy_container);
	    }
	}
        free(expanded_val);
    }

    _copy_inbox = new CopyToInboxCmd(
			    "Inbox",
			    catgets(DT_catd, 1, 221, "Inbox"), 
			    TRUE, 
			    this);

    _copy_other_mboxes = new CmdList ( "Other Mailboxes", catgets(DT_catd, 1, 252, "Other Mailboxes") );
     // This is for mapping the Copy pullright menus dynamically...
     MenuClientData *mcd = (MenuClientData*) malloc (sizeof(MenuClientData));
     mcd->filename = theRoamApp.getFolderDir(TRUE);
     mcd->op = DTM_COPY;
     mcd->checksum = -1;
     mcd->timestamp = 0;
     _copy_other_mboxes->setClientData((void*)mcd);

    // We now create two seperate MoveCopyCmd objects.  This is so we can
    // set the default button correctly in the FSB that is displayed.

    _copy_move_button = new MoveCopyCmd (
				"Select Mailbox",
                                catgets(DT_catd, 1, 65, "Select Mailbox..."),
				TRUE,
                                TRUE,
                                RoamMenuWindow::move_callback,
                                RoamMenuWindow::copy_callback,
                                this,
                                this->baseWidget());

    if (user_list == DTM_TRUE) copy_to->add(_separator);

    copy_to->add(_copy_inbox);
    copy_to->add(_copy_move_button);
    copy_to->add(_copy_other_mboxes);
    copy_to->add(_separator);

}

void
RoamMenuWindow::construct_message_menu()
{
    FORCE_SEGV_DECL(CmdList, cmdList);
    
    _open_msg		= new OpenMsgCmd(
				"Open",
				catgets(DT_catd, 1, 30, "Open"), 
				TRUE, this);
    
    _edit_copy = new EditCopyCmd(
			"Copy Selected Text",
			catgets(DT_catd, 1, 43, "Copy Selected Text"), 
			TRUE, 
			this
		     );

    _select_all = new EditSelectAllCmd(
			"Select All Text",
			catgets(DT_catd, 1, 44, "Select All Text"), 
			TRUE,
			this
		    );

    _save_msg_as	= new SaveAsTextCmd (
				"Save As Text...",
				catgets(DT_catd, 1, 31, "Save As Text..."),
				catgets(DT_catd, 1, 126, "Mailer - Save As Text"),
				TRUE,
				get_editor()->textEditor(),
				NULL,
				this->baseWidget());
    construct_attachment_menu();

    _copy_to = new CmdList("Copy To Mailbox", catgets(DT_catd, 1, 33, "Copy To Mailbox"));
    createCopyList(_copy_to);

    _print_msg		= new PrintCmd(
				"Print...",
				catgets(DT_catd, 1, 34, "Print..."), 
				TRUE, FALSE, this);

    _print		= new PrintCmd(
				"Print One",
				catgets(DT_catd, 1, 13, "Print One"), 
				TRUE, TRUE, this);

    _find		= new FindCmd (
				"Search...",
 				catgets(DT_catd, 1, 35, "Search..."), 
 				TRUE, this );

    _delete_msg		= new DeleteCmd(
				"Delete",
				catgets(DT_catd, 1, 37, "Delete"),
				TRUE, this);

    _undeleteLast       = new UndeleteCmd ( 
				"Undelete Last",
				catgets(DT_catd, 1, 38, "Undelete Last"), 
				TRUE, this, FALSE );

    _undeleteFromList   = new UndeleteCmd(
				"Undelete From List...",
				catgets(DT_catd, 1, 39, 
					"Undelete From List..."), 
				TRUE, this, TRUE);

    // Message Menu
    
    cmdList = new CmdList( "Message", catgets(DT_catd, 1, 40, "Message") );

    cmdList->add(_open_msg);
    cmdList->add(_edit_copy);
    cmdList->add(_select_all);
    cmdList->add(_save_msg_as);
    cmdList->add(_separator);
    cmdList->add(_attachmentMenuList);
    cmdList->add(_separator);
    cmdList->add(_copy_to);
    cmdList->add(_print);
    cmdList->add(_print_msg);
    cmdList->add(_find);
    cmdList->add(_separator);
    cmdList->add(_delete_msg);
    cmdList->add (_undeleteLast);
    cmdList->add (_undeleteFromList);

    _menuBar->addCommands (&_message_cascade, cmdList, FALSE, XmMENU_BAR);
    _attachmentMenuPR = new MenuBar("attachmentPR",
			 _attachmentMenuList->getPaneWidget());

    delete cmdList;

    _copyMenu = _copy_to->getPaneWidget();
    Widget cascade = _copy_to->getCascadeWidget();

    XtVaSetValues(_copyMenu, 
 		  XmNpacking, XmPACK_COLUMN, 
 		  XmNorientation, XmVERTICAL,
 		  NULL);

    XtVaSetValues(cascade, XmNuserData, _copyMenu, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, NULL);

    XtSetSensitive(_copyMenu, TRUE);

    // For the dynamic pullright menus from the Other Mailboxes...
    cascade = _copy_other_mboxes->getCascadeWidget();
    XtVaSetValues(cascade, XmNuserData, this, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, 
		_copy_other_mboxes);

}

void
RoamMenuWindow::construct_message_popup(void)
{
    FORCE_SEGV_DECL(DtMail::Session, m_session);
    DtMailEnv error;

   _msgsPopupMenuList = new CmdList( "MsgsPopup", "MsgsPopup");

   _msgs_popup_title 	= new LabelCmd (
			"Mailer - Messages",
                        catgets(DT_catd, 1, 42, "Mailer - Messages"), TRUE);

   _msgsPopupMenuList->add(_msgs_popup_title);
   _msgsPopupMenuList->add(_separator);
   _msgsPopupMenuList->add(_delete_msg);
   _msgsPopupMenuList->add(_undeleteLast);
   _msgsPopupMenuList->add(_print_msg);
   _msgsPopupMenuList->add(_replySender);

   _move_popup_menu_cmdList = new CmdList( "Move", catgets(DT_catd, 1, 64, "Move") );
   _move_popup_other_mboxes = construct_move_menu_cmdlist(_move_popup_menu_cmdList, FALSE);

   _msgsPopupMenuList->add(_move_popup_menu_cmdList);
   _msgsPopupMenuList->add(_save_msg_as);
   _msgsPopupMenuList->add(_check_new_mail);

   Widget parent = XtParent(_list->get_scrolling_list());
   _menuPopupMsgs = new MenuBar(parent, "RoamMsgsPopup", XmMENU_POPUP);
   _msgsPopupMenu = _menuPopupMsgs->addCommands(_msgsPopupMenuList, 
			FALSE, XmMENU_POPUP);
   XtAddEventHandler(parent, ButtonPressMask,
                        FALSE, MenuButtonHandler, (XtPointer) this);
   
   _msgsPopupMoveMenu = _move_popup_menu_cmdList->getPaneWidget();
   Widget cascade = _move_popup_menu_cmdList->getCascadeWidget();

   XtVaSetValues(_msgsPopupMoveMenu, 
 		 XmNpacking, XmPACK_COLUMN, 
 		 XmNorientation, XmVERTICAL,
 		 NULL);

   XtVaSetValues(cascade, XmNuserData, _msgsPopupMoveMenu, NULL);
   XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, NULL);

   XtSetSensitive(_msgsPopupMoveMenu, TRUE);
 
   cascade = _move_popup_other_mboxes->getCascadeWidget();
   XtVaSetValues(cascade, XmNuserData, this, NULL);
   XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, 
		_move_popup_other_mboxes);
}

void
RoamMenuWindow::construct_text_popup(void)
{
   if (theApplication->bMenuButton() != Button3)
       return;

   _textPopupMenuList = new CmdList( "TextPopup", "TextPopup");

    _text_popup_title     = new LabelCmd (
			"Mailer - Text",
			catgets(DT_catd, 1, 46, "Mailer - Text"), TRUE);

    _textPopupMenuList->add(_text_popup_title);
    _textPopupMenuList->add(_separator);
    _textPopupMenuList->add(_edit_copy);
    _textPopupMenuList->add(_select_all);

    Widget parent = my_editor->textEditor()->get_editor();
    _menuPopupText = new MenuBar(parent, "RoamTextPopup", XmMENU_POPUP);
    _textPopupMenu = _menuPopupText->addCommands(_textPopupMenuList, 
				FALSE, XmMENU_POPUP);
}


void RoamMenuWindow::construct_view_menu()
{
    FORCE_SEGV_DECL(CmdList, cmdList);

    _next		= new NextCmd ( 
				"Next",
				catgets(DT_catd, 1, 47, "Next"), TRUE, this );
    _previous		= new PrevCmd ( 
				"Previous",
				catgets(DT_catd, 1, 48, "Previous"), TRUE, this );
    
    _abbrevHeaders = new AbbrevHeadersCmd(
				"Abbreviated Headers",
				catgets(DT_catd, 1, 49, "Abbreviated Headers"), 
				this);

/*
    _sortTD	= new NoOpCmd ( "By Date/Time", catgets(DT_catd, 1, 47, "By Date/Time"), TRUE);
    _sortSender	= new NoOpCmd ( "By Sender", catgets(DT_catd, 1, 48, "By Sender"), TRUE);
    _sortSubject	= new NoOpCmd (
				"By Subject",
				catgets(DT_catd, 1, 49, "By Subject"), TRUE);
    _sortSize	= new NoOpCmd ( "By Size", catgets(DT_catd, 1, 50, "By Size"), TRUE);
    _sortStatus	= new NoOpCmd ( "By Status", catgets(DT_catd, 1, 51, "By Status"), TRUE);
*/

    _sortTD = new SortCmd ("By Date/Time", 
				catgets(DT_catd, 1, 50, "By Date/Time"),
				TRUE,
				this,
				SortTimeDate);
    _sortSender = new SortCmd ("By Sender",
				catgets(DT_catd, 1, 51, "By Sender"),
				TRUE,
				this,
				SortSender);
    _sortSubject = new SortCmd ("By Subject",
				catgets(DT_catd, 1, 52, "By Subject"),
				TRUE,
				this,
				SortSubject);
    _sortSize = new SortCmd ("By Size",
				catgets(DT_catd, 1, 53, "By Size"),
				TRUE,
				this,
				SortSize);
    _sortStatus = new SortCmd ("By Status",
				catgets(DT_catd, 1, 54, "By Status"),
				TRUE,
				this,
				SortStatus);

    // View Menu
    
    cmdList = new CmdList( "View", catgets(DT_catd, 1, 55, "View") );
    
    cmdList->add ( _next );
    cmdList->add ( _previous );
    cmdList->add ( _separator );
    cmdList->add( _abbrevHeaders);
    cmdList->add ( _separator );
    cmdList->add ( _sortTD );
    cmdList->add ( _sortSender );
    cmdList->add ( _sortSubject );
    cmdList->add ( _sortSize );
    cmdList->add ( _sortStatus );

    _menuBar->addCommands ( cmdList );

    delete cmdList;
}
void
RoamMenuWindow::construct_compose_menu()
{
    FORCE_SEGV_DECL(CmdList, cmdList);
    FORCE_SEGV_DECL(DtMail::Session, m_session);
    char *value = NULL;
    DtMailEnv error;

    if (!_comp_new)
    	_comp_new	= new ComposeCmd ( 
			"New Message",
			catgets(DT_catd, 1, 245, "New Message"), 
			TRUE, 
			this );
    if (_templates)
	delete _templates;
    _templates = new CmdList ( 
			"New, Use Template", 
			catgets(DT_catd, 1, 244, "New, Use Template") );

    m_session = theRoamApp.session()->session();
    m_session->mailRc(error)->getValue(error, "templates", (const char **)&value);
    if (value != NULL && *value != NULL) {
    	DtVirtArray<PropStringPair *> templates(8);
    	parsePropString(value, templates);
    	int tcount = templates.length();
    	for (int i = 0; i < tcount; i++) {
        	PropStringPair * psp = templates[i];
        	if (psp->label && psp->value) {
            		Cmd * button = new LoadFileCmd(strdup(psp->label),
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
			TRUE, 
			this, 
			TRUE );

    if (!_forwardBody)
    		_forwardBody	= new ForwardCmd ( 
			"Forward, no Attachments",
			catgets(DT_catd, 1, 271, "Forward, no Attachments"), 
			TRUE, 
			this, 
			FALSE );

    if (!_replySender)
    		_replySender = new ReplyCmd ( 
			      "Reply",
			      catgets(DT_catd, 1, 59, "Reply"), 
			      TRUE, 
			      this, 
			      FALSE );

    if (!_replyAll)
    		_replyAll	= new ReplyAllCmd ( 
			"Reply to All",
			catgets(DT_catd, 1, 60, "Reply to All"), 
			TRUE, 
			this, 
			FALSE );

    if (!_replySinclude)
    		_replySinclude= new ReplyCmd ( 
			"Reply, Include Message",
			catgets(DT_catd, 1, 247, "Reply, Include Message"), 
			TRUE, 
			this, 
			TRUE );	

    if (!_replyAinclude)
    		_replyAinclude= new ReplyAllCmd ( 
			"Reply to All, Include Message",
			catgets(DT_catd, 1, 248, "Reply to All, Include Message"), 
			TRUE, 
			this, 
			TRUE );	
    if (_drafts)
	delete _drafts;
    _drafts = new CmdList ( 
			"Continue Composing Draft", 
			catgets(DT_catd, 1, 286, "Continue Composing Draft") );

    // Compose Menu
    
    cmdList = new CmdList( "Compose", catgets(DT_catd, 1, 63, "Compose") );
    cmdList->add ( _comp_new );
    cmdList->add (_templates);
    cmdList->add(_separator);
    cmdList->add (_forward);
    cmdList->add (_forwardBody);
    cmdList->add(_separator);
    cmdList->add ( _replySender );
    cmdList->add ( _replyAll );
    cmdList->add ( _replySinclude );
    cmdList->add ( _replyAinclude );
    cmdList->add(_separator);
    cmdList->add ( _drafts );

    _menuBar->addCommands ( &_compose_cascade,  cmdList, FALSE, XmMENU_BAR  );

    Widget menu = _templates->getPaneWidget();
    XtVaSetValues(menu,
                  XmNpacking, XmPACK_COLUMN,
                  XmNorientation, XmVERTICAL,
                  NULL);

    Widget cascade = _templates->getCascadeWidget();
    XtVaSetValues(cascade, XmNuserData, menu, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, NULL);

    menu = _drafts->getPaneWidget();
    XtVaSetValues(menu,
                  XmNpacking, XmPACK_COLUMN,
                  XmNorientation, XmVERTICAL,
                  NULL);

    RAMenuClientData *mcd =
                (RAMenuClientData*) malloc (sizeof(RAMenuClientData));
    mcd->timestamp = 0;
    mcd->filename = theRoamApp.getResource(DraftDir, TRUE);
    mcd->smd = NULL;
    _drafts->setClientData((void*)mcd);

    XtVaSetValues(menu, XmNuserData, this, NULL);
    XtAddCallback(menu, XmNmapCallback, &RoamApp::map_menu, _drafts);

    delete cmdList;
}

void
RoamMenuWindow::addFoldersFiles(Widget menu, CmdList *currentCmdList)
{
	CmdList *folderCmdlist;
	Cmd *cmd;
	struct dirent *direntp;
	FORCE_SEGV_DECL(DtMail::Session, m_session);
	DtMailEnv error;
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
	if (dirp == NULL)
		// Cant do anything without a directory ptr.
		return; 

	int sum=0, i, j;
	while ((direntp = readdir(dirp)) != NULL) {
		// Dont include the files we dont show.
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
			// Its a directory...Just create the cmdlist. We create the 
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
			// Its a file...
			else {
				if (mcd->op == DTM_OPEN)
					cmd = new OpenMailboxCmd(
                                        	strdup(direntp->d_name),
                                        	direntp->d_name,
                                        	TRUE,
                                        	this,
                                        	strdup(filename));
				else // Move or Copy...
					cmd = new MoveCopyMenuCmd (strdup(filename),
						direntp->d_name, 1, this, mcd->op);
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
	mb->addCommands(currentCmdList, FALSE, XmMENU_PULLDOWN);

	XtVaSetValues(menu, XmNpacking, XmPACK_COLUMN,
                        XmNorientation, XmVERTICAL,
                        NULL);
}
void
RoamMenuWindow::construct_options_menu()
{
    FORCE_SEGV_DECL(CmdList, cmdList);

    cmdList = new CmdList( "Options", catgets(DT_catd, 1, 260, "Options") );

    _opt_basic = new OptionsCmd(
			"Basic",
			catgets(DT_catd, 1, 261, "Basic..."),
                        1,
			OPT_BASIC);
    cmdList->add(_opt_basic);

    cmdList->add(_separator);

    _opt_al = new OptionsCmd(
			"Aliases",
			catgets(DT_catd, 1, 269, "Aliases..."),
                        1,
			OPT_AL);
    cmdList->add(_opt_al);

    _opt_cw = new OptionsCmd(
			"Compose Window",
			catgets(DT_catd, 1, 265, "Compose Window..."),
                        1,
			OPT_CW);
    cmdList->add(_opt_cw);


    _opt_mv = new OptionsCmd(
			"Message View",
			catgets(DT_catd, 1, 263, "Message View..."),
                        1,
			OPT_MV);
    cmdList->add(_opt_mv);

    _opt_mf = new OptionsCmd(
			"Move Menu Setup",
			catgets(DT_catd, 1, 264, "Move Menu Setup..."),
                        1,
			OPT_MF);
    cmdList->add(_opt_mf);

    cmdList->add(_separator);

// AUTOFILING change this back to Filing and Notification when we put back the changes
    _opt_no = new OptionsCmd(
			"Notification",
			catgets(DT_catd, 1, 282, "Notification..."),
                        1,
			OPT_NO);
    cmdList->add(_opt_no);

    _opt_sig = new OptionsCmd(
			"Signature",
			catgets(DT_catd, 1, 266, "Signature..."),
                        1,
			OPT_SIG);
    cmdList->add(_opt_sig);

    _opt_td = new OptionsCmd(
			"Templates",
			catgets(DT_catd, 1, 268, "Templates..."),
                        1,
			OPT_TD);
    cmdList->add(_opt_td);

    _opt_tb = new OptionsCmd(
			"Toolbar",
			catgets(DT_catd, 1, 262, "Toolbar..."),
                        1,
			OPT_TB);
    cmdList->add(_opt_tb);

    _opt_vac = new OptionsCmd(
			"Vacation Message",
			catgets(DT_catd, 1, 267, "Vacation Message..."),
                        1,
			OPT_VAC);
    cmdList->add(_opt_vac);

    cmdList->add(_separator);

    _opt_ad = new OptionsCmd(
			"Advanced",
			catgets(DT_catd, 1, 270, "Advanced..."),
                        1,
			OPT_AD);
    cmdList->add(_opt_ad);

    _menuBar->addCommands(cmdList);

    delete cmdList;
}
CmdList*
RoamMenuWindow::construct_move_menu_cmdlist(CmdList *list, Boolean append)
{
    FORCE_SEGV_DECL(DtMail::Session, m_session);
    DtMailEnv error;
    char *val = NULL;
    char *expanded_list = NULL, *token;
    DtMailBoolean	user_list = DTM_FALSE;
    MoveCopyMenuCmd	*move_container;

    // Get names for permanent containers from .mailrc and fill in
    // the menu.  We get them from the "filemenu2" variable, and we
    // no longer read "filemenu", which was used by V2 mailtool and
    // kept along in V3 mailtool for backward compatability.

    m_session = theRoamApp.session()->session();
    m_session->mailRc(error)->getValue(error, "filemenu2", (const char **)&val);
    if (error.isNotSet() && val != NULL && *val != NULL) {
        expanded_list = m_session->expandPath(error, val);
	free(val);
	user_list = DTM_TRUE;
    }

    if (user_list == DTM_TRUE) {
	// Create arrays to hold the user defined container list and the
	// recently visited (cached) container list.

	if ((token = (char *) strtok(expanded_list, " "))) {
	    // add MoveMenuCmd to the array, passing in the token.
	    move_container = new MoveCopyMenuCmd(strdup(token), 
						token,
						TRUE, 
						this, 
						DTM_MOVE);
	    list->add(move_container);
	    if (append) {
	    	_move_containerlist->append(move_container);
	    	_user_list_size++;
	    }
	    while (token = (char *) strtok(NULL, " ")) {
		// add MoveMenuCmd to the array, passing in the token.
		move_container = new MoveCopyMenuCmd(strdup(token), 
							token,
							TRUE, 
							this,
							DTM_MOVE);
		list->add(move_container);
	    	if (append) {
			_move_containerlist->append(move_container);
			_user_list_size++;
		}
	    }
	}
	free(expanded_list);
    }
    // Move menu
    if (user_list == DTM_TRUE)
	list->add(_separator);

    if (!_move_inbox)
    	_move_inbox = new MoveToInboxCmd(
                  "Inbox",
       		  catgets(DT_catd, 1, 221, "Inbox"),
 		  TRUE, 
  		  this);

    list->add(_move_inbox);

    // We expect _move_copy_button to have been initialized already when
    // we constructed the copy menu.

    if (!_move_copy_button)
    	_move_copy_button = new MoveCopyCmd (
				"Select Mailbox",
				catgets(DT_catd, 1, 65, "Select Mailbox..."),
				FALSE,
				TRUE,
				RoamMenuWindow::move_callback,
				RoamMenuWindow::copy_callback,
				this,
				this->baseWidget());


    CmdList *move_other_mboxes = new CmdList ( "Other Mailboxes", catgets(DT_catd, 1, 252, "Other Mailboxes") );

    // This is for mapping the Move pullright menus dynamically...
    MenuClientData *mcd = (MenuClientData*) malloc (sizeof(MenuClientData));
    mcd->filename = theRoamApp.getFolderDir(TRUE);
    mcd->op = DTM_MOVE;
    mcd->checksum = -1;
    mcd->timestamp = 0;
    move_other_mboxes->setClientData((void*)mcd);

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
    FORCE_SEGV_DECL(DtMail::Session, m_session);
    char *val = NULL;
    DtMailEnv error;

    m_session = theRoamApp.session()->session();
    // We will use _containerlist and cached_containerlist to keep
    // track of the commands associated with the Move menu items.

    if (_move_containerlist != NULL)
	delete _move_containerlist;
    _move_containerlist = new DtVirtArray<MoveCopyMenuCmd *> (3);

    if (_cached_move_containerlist != NULL)
	delete _cached_move_containerlist;
    _cached_move_containerlist = new DtVirtArray<MoveCopyMenuCmd *>
		(_max_cached_list_size ? _max_cached_list_size : 1 );
    
    if (_move_menu_cmdList != NULL)
	delete _move_menu_cmdList;
    _move_menu_cmdList = new CmdList( "Move", catgets(DT_catd, 1, 64, "Move") );

    _move_other_mboxes = construct_move_menu_cmdlist(_move_menu_cmdList, TRUE);

    _moveMenu = _menuBar->addCommands(&_move_cascade, _move_menu_cmdList, 
			FALSE, XmMENU_BAR);

    // For dynamic Move pullright menus.
    Widget cascade = _move_other_mboxes->getCascadeWidget();
    XtVaSetValues(cascade, XmNuserData, this, NULL);
    XtAddCallback(cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, 
		_move_other_mboxes);

    XtVaSetValues(_moveMenu, 
 		  XmNpacking, XmPACK_COLUMN, 
 		  XmNorientation, XmVERTICAL,
 		  NULL);

    XtVaSetValues(_move_cascade, XmNuserData, _moveMenu, NULL);
    XtAddCallback(_move_cascade, XmNcascadingCallback, &RoamMenuWindow::map_menu, NULL);

}

// Attachment menu

void
RoamMenuWindow::construct_attachment_menu()
{

    _save_attachment	= new SaveAttachCmd (
				"Save As...",
				catgets(DT_catd, 1, 66, "Save As..."),
				catgets(DT_catd, 1, 67, "Mailer - Attachments - Save As"),
				FALSE, 
				RoamMenuWindow::save_attachment_callback,
				this,
				this->baseWidget());

    _select_all_attachments = new SelectAllAttachsCmd(
					"Select All",
					catgets(
					    DT_catd, 1, 68, "Select All"
					),
					this);

    _attachmentMenuList = new CmdList( 
					"Attachments",
					catgets(
						DT_catd, 1, 69, 
						"Attachments"
					) 
				);
    _attachmentMenuList->add(_save_attachment);
    _attachmentMenuList->add(_select_all_attachments);

}

void
RoamMenuWindow::construct_attachment_popup(void)
{
   _attachmentPopupMenuList = new CmdList( "AttachmentsPopup", "AttachmentsPopup");

    _attachment_popup_title     = new LabelCmd (
			"Mailer - Attachments",
			catgets(DT_catd, 1, 70, "Mailer - Attachments"), TRUE);

    _attachmentPopupMenuList->add(_attachment_popup_title);
    _attachmentPopupMenuList->add(_separator);
    _attachmentPopupMenuList->add(_save_attachment);
    _attachmentPopupMenuList->add(_select_all_attachments);
    _menuPopupAtt = new MenuBar(my_editor->attachArea()->getClipWindow(), 
					"RoamAttachmentPopup", XmMENU_POPUP);
    _attachmentPopupMenu = _menuPopupAtt->addCommands(_attachmentPopupMenuList, 
				FALSE, XmMENU_POPUP);
}

void
RoamMenuWindow::construct_help_menu()
{
    FORCE_SEGV_DECL(CmdList, cmdList);

    _overview = new OnAppCmd("Overview", catgets(DT_catd, 1, 71, "Overview"), 
				TRUE, this);
    _tasks = new TasksCmd("Tasks", catgets(DT_catd, 1, 72, "Tasks"), 
				TRUE, this);
    _reference = new ReferenceCmd("Reference", catgets(DT_catd, 1, 73, "Reference"), 
				TRUE, this);
    _messages = new MessagesCmd("Messages", catgets(DT_catd, 1, 301, "Messages"), 
				TRUE, this);
    _on_item = new OnItemCmd("On Item", catgets(DT_catd, 1, 74, "On Item"), 
				TRUE, this);
    _using_help = new UsingHelpCmd("Using Help", catgets(DT_catd, 1, 75, "Using Help"), 
					TRUE, this);
    cmdList = new CmdList( "Help", catgets(DT_catd, 1, 76, "Help") );
    cmdList->add ( _overview );
    cmdList->add ( _separator );
    cmdList->add ( _tasks );
    cmdList->add ( _reference );
    cmdList->add ( _messages );
    cmdList->add ( _separator );
    cmdList->add ( _on_item );
    cmdList->add ( _separator );
    cmdList->add ( _using_help );
    cmdList->add ( _separator );

    _about_mailer = new RelNoteCmd("About Mailer...", 
				    catgets(DT_catd, 1, 77, "About Mailer..."),
    				    TRUE, this);
    cmdList->add ( _about_mailer );

    // Make help menu show up on right side of menubar.
    _menuBar->addCommands ( cmdList, TRUE );
    
    delete cmdList;
}
 
DtMail::MailRc *
RoamMenuWindow::get_mail_rc()
{
    DtMailEnv error;

    return (_mailbox->session()->mailRc(error));
}


void
RoamMenuWindow::load_mailbox(
    DtMailEnv & mail_error
)
{
    int		count = 0;
    
    // If there is no mailbox, return.
    if (!_mailbox) {
	mail_error.clear();
	return;
    }

    // Now load the messages into the scrolling list.
    // This will get the DtMailMessageHandles into the _msgs array and
    // it will also get their XmStrings into the CharArray of the _list.
    theRoamApp.busyAllWindows(catgets(DT_catd, 3, 27, "Loading..."));
	
    // Call load_headers() to get the XmStrings into the XmList!

    count = _list->load_headers(mail_error);
    
    if (mail_error.isSet()) {
	// Return whatever error mailbox->get_next_msg() returned.
	theRoamApp.unbusyAllWindows();
	return;
    }

    // If no messages

    if (count == 0) {
	this->message(catgets(DT_catd, 3, 28, "Empty container"));
	setIconName(EmptyIcon);
    }

    theRoamApp.unbusyAllWindows();
}

void
RoamMenuWindow::clear_message()
{
    this->message("");
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

    // syncViewAndStore() does the sync-ing of the view of a mail
    // container and the storage of that container.
    // This method gets invoked every time a message gets expunged
    // by the back end based on "timed delete".
    // The method needs to then remove the expunged message from the
    // deleted messages list, thereby syncing the view to be always
    // current with the storage.
    // Similarly, the method also gets invoked when the container store
    // has received new mail.  The view then needs to be updated....


DtMailBoolean
RoamMenuWindow::syncViewAndStoreCallback(
    DtMailCallbackOp op,
    const char *,	// path
    const char *,	// prompt_hint
    void *client_data,	
    ...
)
{

    DtMailBoolean bool;
    va_list args;

    va_start(args, client_data);
    RoamMenuWindow * rmw = (RoamMenuWindow *)client_data;

    bool = rmw->syncViewAndStore(op, args);
    return(bool);
}

DtMailBoolean
RoamMenuWindow::syncViewAndStore(
    DtMailCallbackOp op,
    va_list args
)
{

    DtMailEnv error;
    DtMailMessageHandle	tmpMH;
    char buf[100];
    int answer;
    char *helpId;


    this->busyCursor();

    switch(op) {
      case DTMC_NEWMAIL:

	// New mail has come in.  Load it in and update the
	// view's list of headers to also display it.

	tmpMH = va_arg(args, DtMailMessageHandle);
	this->_list->load_headers(error, tmpMH);

	newMailIndicators();

	break;

      case DTMC_DELETEMSG:

	// A message has been expunged from the store by the back end.  
	// The expunge has been done based on "timed delete".
	// Remove the expunged message from the displayed list of 
	// message headers.

	break;

      case DTMC_BADSTATE:

	// The backend has lost track of where it is in the folder.
	// We need to close this folder and go away, right now.
	sprintf(buf, 
		catgets(DT_catd, 1, 78, "Mailer is confused about the state of this mailbox.\nClose the mailbox by choosing OK and then restart Mailer."));
	
	this->_genDialog->setToErrorDialog(
				catgets(DT_catd, 2, 10, "Mailer"),
				buf);
	helpId = DTMAILHELPUNKNOWNSTATE;
	answer = this->_genDialog->post_and_return(
				catgets(DT_catd, 3, 29, "OK"),
				helpId);
	_exit(0);
	break;

      case DTMC_UNLOCK:
	
	// We are asked to save changes and close the file.
	// The backend will take care of unlocking the file.
	this->quit_silently();
	return(DTM_TRUE);

      case DTMC_QUERYLOCK:

	// The file is lock by another mailer.
	// Should we ask for the lock?
	_genDialog->setToQuestionDialog(catgets(DT_catd, 3, 16, "Mailer"),
	   catgets(DT_catd, 3, 82, "Someone else is using this mailbox.\nWould you like to demand exclusive access?"));
	answer = _genDialog->post_and_return(catgets(DT_catd, 3, 18, "OK"),
	   catgets(DT_catd, 3, 19, "Cancel"), DTMAILHELPTAKELOCK);

	this->normalCursor();

	if (answer == 1) {
	  return(DTM_TRUE);
	} else {
	  return(DTM_FALSE);
	}

    case DTMC_READONLY:

      // We are not able to obtain a lock on this folder,
      // ask user if he wants to open it as read only.
      _genDialog->setToQuestionDialog(catgets(DT_catd, 3, 16, "Mailer"),
				      catgets(DT_catd, 3, 83, "Mailer is unable to obtain exclusive access to this mailbox.\nWould you like to open this mailbox read-only?"));
	answer = _genDialog->post_and_return(catgets(DT_catd, 3, 18, "OK"),
					     catgets(DT_catd, 3, 19, "Cancel"),
					     DTMAILHELPOPENREADONLY);
	this->normalCursor();
	if (answer == 1) {
	  return(DTM_TRUE);
	} else {
	  return(DTM_FALSE);
	}

    case DTMC_READWRITEOVERRIDE:
      // We are not able to obtain a lock on this folder,
      // ask user if he wants to open it as read only.
      _genDialog->setToQuestionDialog(catgets(DT_catd, 3, 16, "Mailer"),
				      catgets(DT_catd, 3, 94, "Mailer is unable to obtain exclusive access to this\nmailbox because the system is not responding.\n(The file $HOME/.dt/errorlog may contain additional information).\n\nFor this time only, you can choose to open this mailbox\nread-only, or to open it read-write without exclusive\naccess (use only if no one else is using this mailbox)."));
	answer = _genDialog->post_and_return(catgets(DT_catd, 3, 95, "Read-only"),
					     catgets(DT_catd, 3, 19, "Cancel"),
					     catgets(DT_catd, 3, 96, "Read-Write"), 
					     DTMAILHELPOPENREADWRITEOVERRIDE);
	this->normalCursor();
	// Now the tricky part - since this method can only return 
	// DTM_TRUE or DTM_FALSE, we must have a way to indicate "readOnly",
	// "readWrite" or "cancel" - horrid hack: readOnly is DTM_FALSE, 
	// "readWrite" is DTM_TRUE, and "cancel" is (DTM_TRUE+DTM_FALSE)*2
	//
	switch (answer)
	  {
	  case 1:	// Read-only
	    return(DTM_FALSE);
	  case 3:	// Read-Write
	    return(DTM_TRUE);
	  default:	// cancel
	    return((DtMailBoolean)((DTM_FALSE+DTM_TRUE)*2));
	  }


      default:
	this->normalCursor();
	return(DTM_FALSE);
    }

    this->normalCursor();
    return(DTM_TRUE);
}

#ifdef DEAD_WOOD
void
RoamMenuWindow::MailboxUpdateCB(
    DtMailCallbackOp,		   // op
    DtMailMessageHandle,	   // msg_num
    void *			   // client_data
)
{
    // Stubbed out
}

// Convert the rfc file.
// Commented out for PAR 5.  Needs to be uncommented out for PAR 6
// and above.

void
RoamMenuWindow::convert(
    char *,		// src
    char *		// dest
)
{
    // Obsoleted when Bento went away
}

// The back end uses the return value to either continue conversion or
// to stop converting.

int
RoamMenuWindow::ConvertStatusCB(
    int current, 
    int total, 
    void * client_data
)
{

    RoamMenuWindow *rmw = (RoamMenuWindow *)client_data;

    // Return 0 if the conversion is still to proceed.
    // Return 1 if the conversion is to stop (e.g., if the user
    // has interrupted it...)

    return(rmw->showConversionStatus(current, total));
}

int
RoamMenuWindow::showConversionStatus(
    int current, 
    int total
)
{
    char *buf, *str;
    int num_already_converted;  // num msgs already converted as set prev.
    int previously_complete;  // percent of converted msgs as set prev.
    int now_complete;  // percent of converted messages based on parameters

    // Remember,  the number set may differ from the previous call to
    // this method.  We are not doing set_convert_data() for every call
    // to this method and so, what's set will differ from the previous
    // call to this method...

    num_already_converted = _convertContainerCmd->get_num_converted();
    previously_complete = num_already_converted * 100 / total;

    now_complete = current * 100 / total;

    if ((now_complete > (previously_complete + 5)) ||
	(now_complete == 100)) {
	str = catgets(DT_catd, 1, 79, "Converting... %d percent complete"); 
	// Estimate 4 characters for numbers and 1 null terminator.
	buf = new char[strlen(str) + 5];   
	sprintf(buf, str, now_complete);
	
	_convertContainerCmd->updateDialog(buf);
	delete buf;

	_convertContainerCmd->set_convert_data(current, total);
    }
    else {
	_convertContainerCmd->updateAnimation();
    }

    // Check and see if the user had interrupted the conversion
    // If the user had interrupted, we need to stop the back-end
    // convert...

    if (_convertContainerCmd->interrupted()) {
	return(1);
    }
    else {
	return(0);
    }
}

    
void
RoamMenuWindow::conversionFinishedCallback( 
    RoamInterruptibleCmd *,
    Boolean interrupted,
    void	     *clientData
)
{

    // Do something only it the conversion really finished.
    // If it was interrupted, just return...

    if (!interrupted) {
	RoamMenuWindow *rmw = (RoamMenuWindow *) clientData;
	rmw->conversionFinished();
    }
}

// If the conversion finished successfully, then we try to open() again
// and if no problems, load the headers into the scrolling list...

void
RoamMenuWindow::conversionFinished()
{
    DtMailEnv mail_error;

    // Initialize the mail_error.
    mail_error.clear();

    _mailbox_fullpath = strdup(_convertContainerCmd->get_destination_name());

    this->open(mail_error, 
	       _openContainerCmd->_open_create_flag,
	       _openContainerCmd->_open_lock_flag);
    
    // if the user had cancelled the open, then the RMW will not have
    // its mailbox set. Do not proceed to load the mailbox.  Return.

    // Ideally, open() should set the mail_error if the user cancels
    // the open.  And we should check the mail_error here...

    if (mail_error.isSet()) {
	return;
    }

    this->load_mailbox(mail_error);

    if (mail_error.isSet()) {

	// Post a dialog indicating problems in loading...
    }

}

// Post the dialog displaying the error text.
// Also display the minor_code, if resource/environment variable
// wants it.
// User has no choice but to OK
// Quit the RMW on user response.  If its the last window, it also
// quits the application.

void
RoamMenuWindow::postFatalErrorDialog(
    DtMailEnv &mail_error
)
{
    char *buf, *str, *helpId;

    const char *text_str = (const char *)mail_error;

    str = catgets(DT_catd, 2, 11, "%s");

	// 9 for minor code + 1 for null terminator.
    buf = new char[strlen(str) + strlen(text_str) + 10];  
    sprintf(buf, str, text_str);

    // Assume that the char *text that comes in has already 
    // been processed for i18n.

    _genDialog->setToErrorDialog(catgets(DT_catd, 2, 12,
	   "Mailer"), buf);
    
    helpId = DTMAILHELPFATALERROR;
    int i = _genDialog->post_and_return(catgets(DT_catd, 3, 30, "OK"), helpId);
    delete buf;
    this->quit();
}
#endif /* DEAD_WOOD */

// Post the dialog displaying the error text.
// Also display the minor_code, if resource/environment variable
// wants it.
// User has no choice but to OK
// postErrorDialog() is the benign uncle of postFatalErrorDialog() --
// it does not quit the RMW.

void
RoamMenuWindow::postErrorDialog(
    DtMailEnv &mail_error
)
{
    char *helpId;

    const char *text_str = (const char *)mail_error;

    // Assume that the char *text that comes in has already 
    // been processed for i18n.

    _genDialog->setToErrorDialog(catgets(DT_catd, 2, 13, "Mailer"), 
				 (char *)text_str);
    
    helpId = DTMAILHELPERROR;
    int i = _genDialog->post_and_return(catgets(DT_catd, 3, 31, "OK"), helpId);

    // Clear the Error
    mail_error.clear();

}


void
RoamMenuWindow::manage()
{
    Dimension win_x, win_y, win_wid, win_ht, win_bwid;

    MainWindow::manage();
//    XmUpdateDisplay(this->baseWidget());

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
    DtMailEnv error;
    error.clear();
    
    // First order of business - busy out active windows
    //
    theRoamApp.busyAllWindows(catgets(DT_catd, 3, 32, "Destroying deleted messages..."));

    // Before changing the state of any deleted/undeleted lists,
    // perform the destroy deleted operation and make sure that
    // it was successful - if there was an error, notify the user
    // and discontinue processing
    //
    _mailbox->expunge(error);
    if (error.isSet()) {
	// An error happened - must inform the user
	//
	postErrorDialog(error);
	error.clear();
    }
    else {
	
	if (_undeleteFromList->dialog()) {
	    _undeleteFromList->dialog()->expunge();
	}
	
	_list->expunge();
	
	DtMail::MailRc * mailrc;
	const char * val;
	
	mailrc = _mailbox->session()->mailRc(error);
	if (error.isSet()) {
	    // what to do??
	    error.clear();
	}
	else {
	    mailrc->getValue(error, "showmsgnum", &val);
	    if (error.isNotSet()) {
		// Need to update scrolling list to display resequenced msg nums
		_list->updateListItems(-1);
	    }
	}
    }

    // Last order of business - unbusy out active windows
    //
    theRoamApp.unbusyAllWindows();
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
	delete _attachmentActionsList;
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
	delete _attachmentActionsList;
	_attachmentActionsList = NULL;
    }
}

void
RoamMenuWindow::activate_default_message_menu()
{
    
    _open_msg->activate();

    _save_msg_as->activate();

    XtSetSensitive(_copy_to->getPaneWidget(), TRUE);

    _print_msg->activate();

    _delete_msg->activate();

}

void
RoamMenuWindow::deactivate_default_message_menu()
{
    
    _open_msg->deactivate();

    _save_msg_as->deactivate();

    XtSetSensitive(_copy_to->getPaneWidget(), FALSE);

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
RoamMenuWindow::newMailIndicators(void)
{
    if (_we_called_newmail == FALSE) {
	// Set to new mail icon only if the window is iconified
	if (this->isIconified()) {
	    setIconName(NewMailIcon);
	}

	DtMailEnv error;
	char *val = NULL;
	DtMail::Session * m_session = theRoamApp.session()->session(); 

	m_session->mailRc(error)->getValue(error, "bell", (const char **)&val);
	if (error.isNotSet()) {
	    int beeps = (int) strtol(val, NULL, 10);
	    while(beeps > 0) {
		XBell(XtDisplay(baseWidget()), 0);
		beeps -= 1;
	    }
	    if (val != NULL) {
		free (val);
		val = NULL;
 	    }
	}
	else {// Default to 1 beep
	    XBell(XtDisplay(baseWidget()), 0);
	    error.clear();
	    val = NULL;
	}

	m_session->mailRc(error)->getValue(error, "realsound", (const char **)&val);
	if (error.isNotSet()) {
	    struct stat stat;
	    if (SafeStat("/usr/bin/audioplay", &stat) == 0) {
		char play_str[1500];
		sprintf(play_str, "/usr/bin/audioplay %s", val);
		// fork a new process
    		if (fork() == 0)
			execl("/bin/sh", "sh", "-c", play_str, (char *)0);
	    }
	    if (val != NULL) {
		free (val);
		val = NULL;
 	    }
	}
        else {
		error.clear();
		val = NULL;
	}
	// See if we are supposed to ring the bell.
	//
	m_session->mailRc(error)->getValue(error, "flash", (const char **)&val);
	if (error.isNotSet()) {
	    int flashes = (int) strtol(val, NULL, 10);
	    flash(flashes);
	    if (val != NULL) {
		free (val);
		val = NULL;
 	    }
	}
    }

    _we_called_newmail = FALSE;
}

void
RoamMenuWindow::save_attachment_callback(
    void *client_data,
    char *selection
)
{

    RoamMenuWindow *obj = (RoamMenuWindow *) client_data;
    obj->save_selected_attachment(selection);
}

void
RoamMenuWindow::save_selected_attachment(char * selection)
{
    DtMailEnv mail_error;

    // Initialize the mail_error.
    mail_error.clear();

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


    if (mail_error.isSet()) {
	// do something
    }

}

void
RoamMenuWindow::addAttachmentActions(
    char **actions,
    int indx
)
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
	delete _attachmentActionsList;
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
	
    }
    // Add the dynamic commands
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
    MoveCopyMenuCmd *cmd;

    if (list == NULL) return(-1);

    if (filename && *filename != '/' && *filename != '.' && 
		*filename != '$' && *filename != '~') { // relative path
	
    	char *value, *newname;

	value = theRoamApp.getFolderDir(TRUE);
	newname = (char*)malloc(strlen(filename) + strlen(value) + 2);
	memset(newname, 0, strlen(filename) + strlen(value) + 2);
	sprintf(newname, "%s/%s", value, filename);
	free (value);
	for (int i=0; i < list->length(); i++) {
		cmd = (*list)[i];
		if (strcmp(newname, cmd->containerName()) == 0) {
			free(newname);
			return(i);
		}
	}
	free(newname);
    }

    for (int i=0; i < list->length(); i++) {
	cmd = (*list)[i];
	if (strcmp(filename, cmd->containerName()) == 0) {
	    return(i);
	}
    }
    return(-1);
}
void
RoamMenuWindow::addToRecentList(char *filename)
{
    int i, index, bottom_index;
    MoveCopyMenuCmd	*move_container, *copy_container;
    char *name, *new_name;

    if (filename != NULL && *filename != '\0' 
		&& (_max_cached_list_size > 0)) {
	// Is the file in the user defined list?
	if ((index = inList(filename, _move_containerlist)) != -1) {
	    return;
	}

	// Is the file in the recently used list?
	if ((index = inList(filename, _cached_move_containerlist)) != -1) {
	    // Yes, move filename to top of list and move everything else down.

	    if (index == 0) return;

	    // Change the filenames that each of the Cmds points to in both
	    // the move and copy cached lists.
	    name = (*_cached_move_containerlist)[index]->containerName();

	    for (i = index; i > 0; i--) {
		new_name = (*_cached_move_containerlist)[i-1]->containerName();
		(*_cached_move_containerlist)[i]->changeContainer(new_name);
		(*_cached_copy_containerlist)[i]->changeContainer(new_name);
	    }

	    (*_cached_move_containerlist)[0]->changeContainer(name);
	    (*_cached_copy_containerlist)[0]->changeContainer(name);
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
	     }
	 }
	 else {

	    // Add filename to Recently Used List.

	    bottom_index = _cached_list_size - 1;

	    // Is there room for the menu to grow?
	    if (_cached_list_size < _max_cached_list_size) {

		// Create the new command.
		move_container = new MoveCopyMenuCmd(strdup(filename),
							filename,
							TRUE,
							this,
							DTM_MOVE);
		copy_container = new MoveCopyMenuCmd(strdup(filename),
							filename,
							TRUE,
							this,
							DTM_COPY);
		// Add it to the bottom of the menu.
		_menuBar->addCommand(_moveMenu, move_container);
		_menuBar->addCommand(_msgsPopupMoveMenu, move_container);
		_menuBar->addCommand(_copyMenu, copy_container);
		_cached_move_containerlist->append(move_container);
		_cached_copy_containerlist->append(copy_container);
		_cached_list_size++;
		bottom_index++;
		if (_cached_list_size > 1) {
		    // Modify the Cmds in the DtVirtArray
		    // and rotate the labels in the menu.
		    name = 
			(*_cached_move_containerlist)[bottom_index]->containerName();
		    for (i = bottom_index; i > 0; i--) {
			new_name = (*_cached_move_containerlist)[i-1]->containerName();
			(*_cached_move_containerlist)[i]->changeContainer(new_name);
			(*_cached_copy_containerlist)[i]->changeContainer(new_name);
		    }
		    (*_cached_move_containerlist)[0]->changeContainer(name);
		    (*_cached_copy_containerlist)[0]->changeContainer(name);
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
	            }
		} else {
		    // Initialize _first_cached_item.  The first cached
		    // item is after the user defined list, after the
		    // Inbox and Move To Mailboxes items, and after the
		    // two separator items.
		    if (_user_list_size > 0) {
			_first_cached_move_item = _user_list_size + 5;
			_first_cached_copy_item = _user_list_size + 5;
		    } else {
			_first_cached_move_item = 4;
			_first_cached_copy_item = 4;
		    }
		}


	    } else {
		// Add new entry to top of cache and drop off bottom item.
		for (i = bottom_index; i > 0; i--) {
		    new_name = (*_cached_move_containerlist)[i-1]->containerName();
		    (*_cached_move_containerlist)[i]->changeContainer(new_name);
		    (*_cached_copy_containerlist)[i]->changeContainer(new_name);
		}
		(*_cached_move_containerlist)[0]->changeContainer(strdup(filename));
		(*_cached_copy_containerlist)[0]->changeContainer(strdup(filename));
		_menuBar->changeLabel(_moveMenu,
				_first_cached_move_item + _cached_list_size-1,
				filename);
		_menuBar->changeLabel(_msgsPopupMoveMenu,
				_first_cached_move_item + _cached_list_size-1,
				filename);
		_menuBar->changeLabel(_copyMenu,
				_first_cached_copy_item + _cached_list_size-1,
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
		}

	    }
	}


    } else {
	// No filename!
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
RoamMenuWindow::fullHeader(
    Boolean state
)
{
    DtMailMessageHandle msgHandle;
    DtMailEnv mail_error;

    // Initialize the mail_error.
    mail_error.clear();

    _fullHeader = state;
    
    msgHandle = this->list()->current_msg_handle();
    if (msgHandle) {
	this->list()->display_message(mail_error, msgHandle);
    }

}

void 
RoamMenuWindow::postMsgsPopup(XEvent *event)
{
	XmMenuPosition(_msgsPopupMenu, (XButtonEvent *)event);
	XtManageChild(_msgsPopupMenu);
}

void
RoamMenuWindow::MenuButtonHandler(
    Widget ,
    XtPointer cd,
    XEvent *event,
    Boolean *)
{
	RoamMenuWindow *obj = (RoamMenuWindow *)cd;

	if(event->xany.type != ButtonPress)
		return;

	XButtonEvent *be = (XButtonEvent *)event;

	if(be->button == theApplication->bMenuButton())
		obj->postMsgsPopup(event);
}

void
RoamMenuWindow::attachmentFeedback(
    Boolean bool
)
{
    if (bool) {
	this->busyCursor();
    }
    else {
	this->normalCursor();
    }
}
void
RoamMenuWindow::map_open_menu(
		    Widget cascade, 
		    XtPointer clientData,
		    XtPointer)
{
	RoamMenuWindow *rmw; 
   	Widget menu = (Widget)clientData;

	XtVaGetValues(cascade, XmNuserData, &rmw, NULL);
	char *logfile = theRoamApp.getResource(SentMessageLogFile, TRUE);
	if (SafeAccess(logfile, F_OK) == 0)
		rmw->_logfile->activate();
	else
		rmw->_logfile->deactivate();
	free(logfile);

	theRoamApp.configure_menu(menu);
}

// map_menu is used to figure out how many columns to split the menu
// into.  It is a callback that is called when the menu is mapped.
// If the menu is over half the height of the screen, it figures out
// how many columns to make the menu, and sets its XmNnumColumns
// attribute to that value.  It calculates the maximum number of columns
// that would fit and never goes beyond that number.

void
RoamMenuWindow::map_menu(
		    Widget widget, 
		    XtPointer clientData,
		    XtPointer)
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
    char *path = this->mailbox_fullpath();
    if (path != NULL && *path != NULL) {
    	char *new_title = new char[strlen(path) + 
                 strlen(catgets(DT_catd, 1, 3, "Vacation")) + 10];
    	sprintf(new_title, "%s [%s]", path, catgets(DT_catd, 1, 3, 
				"Vacation"));
    	this->title(new_title);
    	delete new_title; 
     }
}

void
RoamMenuWindow::removeVacationTitle()
{
    // Reset the title on the roam menu window; take out "[Vacation]"
    char *path = this->mailbox_fullpath();
    if (path != NULL && *path != NULL)
        this->title(path);
}
