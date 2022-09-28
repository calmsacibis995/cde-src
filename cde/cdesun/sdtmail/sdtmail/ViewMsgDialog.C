/*
 *+SNOTICE
 *
 *	$Revision: 1.7 $
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
#pragma ident "@(#)ViewMsgDialog.C	1.93 05/30/97"
#else
static char *sccs__FILE__ = "@(#)ViewMsgDialog.C	1.93 05/30/97";
#endif

#include <assert.h>
#include <Xm/Label.h>
#include <Xm/ScrolledW.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>
#include <Xm/PushB.h>
#include <Xm/MainW.h>
#include <Dt/Editor.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Error.hh>

#include "MainWindow.h"
#include "ViewMsgDialog.h"
#include "RoamMenuWindow.h"
#include "MenuBar.h"
#include "RoamApp.h"
#include "MailMsg.h"
#include "Attachment.h"
#include "Help.hh"
#include "Help.hh"
#include "DtMailHelp.hh"

#include "DtMailEditor.hh"

extern nl_catd	DtMailMsgCat;

//#include "CDEM_CoeEd.hh"


// The WM_CLASS_NAME of a VMD is DtMail_SecWin.
// SQE's  test suites depend on the WM_CLASS_NAME being a 
// constant.  If you are going to change the name, talk to SQE
// and get their consent for whatever change you are making.

ViewMsgDialog::ViewMsgDialog ( 
    RoamMenuWindow *parent 
    )
    : DialogShell ( 
	"DtMail_SecWin",		// Setting WM_CLASS_NAME
	parent
    )
{
    _parent = parent;
    _workArea = NULL;
    _menuBar = NULL;
    my_editor = NULL;
    _genDialog  = NULL;

    _edit_copy = NULL;
    _edit_select_all = NULL;
    
    _attach_save_as = NULL;
    _attach_select_all = NULL;

    _attachmentActionsList = NULL;
    _attachmentMenu = NULL;
    _attachmentMenuList = NULL;
    _attachmentPopupMenuList = NULL;
    _attachment_popup_title = NULL;
    _textPopupMenuList = NULL;
    _text_popup_title = NULL;

    _about_mailer = NULL;
    _reference = NULL;
    _overview = NULL;
    _messages = NULL;
    _using_help = NULL;
    _on_item = NULL;
    _tasks = NULL;
    _msgno = 0;

    _separator = new SeparatorCmd( "Separator", TRUE );

    assert( theApplication );

}


ViewMsgDialog::~ViewMsgDialog()
{

    delete _menuBar;
    delete _genDialog;

    delete _edit_copy;
    delete _edit_select_all;

    delete _attach_save_as;
    delete _attach_select_all;

    if (_attachmentActionsList != NULL) {
      _attachmentActionsList->deleteMe();
    }
    _attachmentMenuList->deleteMe();
    _attachmentPopupMenuList->deleteMe();
    if (_textPopupMenuList != NULL) {
      _textPopupMenuList->deleteMe();
    }
    delete _text_popup_title;
    delete _attachment_popup_title;

    delete _about_mailer;
    delete _reference;
    delete _messages;
    delete _overview;
    delete _using_help;
    delete _on_item;
    delete _tasks;

    delete my_editor;
    delete _separator;

}


Widget 
ViewMsgDialog::createWorkArea ( Widget shell )
{

    Widget form = XtCreateWidget(
			"Work_Area", xmFormWidgetClass,
			shell, NULL, 0);

    _main = XtVaCreateManagedWidget ("separateViewWindow", 
		xmMainWindowWidgetClass,
		form, 
		XmNleftAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		NULL);

    Widget form1 = XtCreateWidget("Work_Area", xmFormWidgetClass,
 				  _main, NULL, 0);
    my_editor = new DtMailEditor(form1, this);
    my_editor->initialize();

    my_editor->attachArea()->setOwnerShell(this);

    Widget wid = my_editor->container();

    XtVaSetValues( wid,
  	XmNrightAttachment,XmATTACH_FORM, 
  	XmNleftAttachment,XmATTACH_FORM, 
  	XmNtopAttachment,XmATTACH_FORM, 
  	XmNbottomAttachment, XmATTACH_FORM,
  	NULL );

    XtManageChild(form1);

    return form;
}				 

void
ViewMsgDialog::propsChanged(void)
{
}

void
ViewMsgDialog::text(const char *text, const unsigned long len)
{

    my_editor->textEditor()->set_contents(text, len);

}

void
ViewMsgDialog::append( const char *text, const unsigned long len )
{
    my_editor->textEditor()->insert_at_cursor(text, len);
}

void 
ViewMsgDialog::popped_down()
{
    delete this;
}


void
ViewMsgDialog::popped_up()
{
    Widget text;
    Dimension width, height;

    text = _parent->get_editor()->container();
    
    XtVaGetValues(text,
	XmNwidth, &width,
	XmNheight, &height,
	NULL);

    XtVaSetValues(_w,
	XmNx, _parent->x() + (_parent->width() / 2),
	XmNy, _parent->y() + (_parent->height() /2),
	NULL );

}


void
ViewMsgDialog::initialize()
{

    DialogShell::initialize();

    _menuBar = new MenuBar ( _main, "menubar", XmMENU_BAR );

    XtVaSetValues ( _main, 
		   XmNmenuBar, _menuBar->baseWidget(),
		   NULL);

    this->construct_edit_menu();
    this->construct_text_popup();
    this->construct_attachment_menu();
    this->construct_attachment_popup();
    this->construct_help_menu();

    _menuBar->manage();

    _genDialog = new DtMailGenDialog("Dialog", _main,
                        XmDIALOG_PRIMARY_APPLICATION_MODAL);

    XtManageChild(_main);
}


void 
ViewMsgDialog::quit()
{
    _parent->unregisterDialog(this);
    delete this;
}

void 
ViewMsgDialog::raise()
{
    XRaiseWindow(XtDisplay(_w), XtWindow(_w));
}

DtMailEditor* 
ViewMsgDialog::get_editor()
{

    return(my_editor);

}

void
ViewMsgDialog::auto_show_cursor_off()
{
    
    my_editor->textEditor()->auto_show_cursor_off();

}

void
ViewMsgDialog::auto_show_cursor_restore()
{

    my_editor->textEditor()->auto_show_cursor_restore();

}

void
ViewMsgDialog::set_to_top()
{

    my_editor->textEditor()->set_to_top();

}

void
ViewMsgDialog::text_selected()
{

    // turn on sensitivity for Cut/Clear/Copy/Paste/Delete

}

void
ViewMsgDialog::text_unselected()
{

    // turn off sensitivity for those items

}


void
ViewMsgDialog::attachment_selected()
{
    _attach_save_as->activate();
}


void
ViewMsgDialog::all_attachments_selected()
{
    _attach_save_as->deactivate();
    if (_attachmentActionsList != NULL) {
	_menuBar->removeCommands(_attachmentMenu, _attachmentActionsList);
	_menuPopupAtt->removeCommands(_attachmentPopupMenu, 
				_attachmentActionsList);
	_attachmentActionsList->deleteMe();
	_attachmentActionsList = NULL;
    }
}

void
ViewMsgDialog::all_attachments_deselected()
{
    _attach_save_as->deactivate();
    if (_attachmentActionsList != NULL) {
	_menuBar->removeCommands(_attachmentMenu, _attachmentActionsList);
	_menuPopupAtt->removeCommands(_attachmentPopupMenu, 
			_attachmentActionsList);
	_attachmentActionsList->deleteMe();
	_attachmentActionsList = NULL;
    }
}

void
ViewMsgDialog::construct_edit_menu()
{

    CmdList *cmdList;
    
    _edit_copy = new EditCopyCmd(
			"Copy",
			catgets(DT_catd, 1, 161, "Copy"), 
			TRUE, 
			this
		     );

    _edit_select_all = new EditSelectAllCmd(
			"Select All",
			catgets(DT_catd, 1, 162, "Select All"), 
			TRUE,
			this
		    );

    cmdList = new CmdList("Edit", catgets(DT_catd, 1, 163, "Edit"));
    cmdList->add(_edit_copy);
    cmdList->add(_edit_select_all);
    
    _menuBar->addCommands(cmdList);

    cmdList->deleteMe();
}

void
ViewMsgDialog::construct_attachment_menu()
{
    _attach_save_as	= new SaveAttachCmd (
				"Save As...",
				catgets(DT_catd, 1, 164, "Save As..."),
				catgets(DT_catd, 1, 165, "Mailer - Attachments - Save As"),
				FALSE, 
				ViewMsgDialog::save_attachment_callback,
				this,
				this->baseWidget());

    _attach_select_all = new SelectAllAttachsCmd(
				"Select All",
				catgets(DT_catd, 1, 166, "Select All"), 
				this);

    _attachmentMenuList = new CmdList( 
				"Attachments",
				catgets(DT_catd, 1, 167, "Attachments") );

    _attachmentMenuList->add(_attach_save_as);
    _attachmentMenuList->add(_attach_select_all);

    _attachmentMenu = _menuBar->addCommands(_attachmentMenuList);

}

void
ViewMsgDialog::construct_attachment_popup(void)
{
   _attachmentPopupMenuList = new CmdList( "AttachmentsPopup", "AttachmentsPopup");

    _attachment_popup_title = new LabelCmd (
			"Mailer - Attachments",
			catgets(DT_catd, 1, 168, "Mailer - Attachments"), TRUE);

    _attachmentPopupMenuList->add(_attachment_popup_title);
    _attachmentPopupMenuList->add(_separator);
    _attachmentPopupMenuList->add(_attach_save_as);
    _attachmentPopupMenuList->add(_attach_select_all);
    _menuPopupAtt = new MenuBar(my_editor->attachArea()->getClipWindow(), 
					"ViewMsgAttachmentPopup", XmMENU_POPUP);
    _attachmentPopupMenu = _menuPopupAtt->addCommands(_attachmentPopupMenuList, 
				FALSE, XmMENU_POPUP);
}

void
ViewMsgDialog::construct_text_popup(void)
{
   if (theApplication->bMenuButton() != Button3)
     return;

   _textPopupMenuList = new CmdList("TextPopup", "TextPopup");

    _text_popup_title     = new LabelCmd (
			"Mailer - Text",
			catgets(DT_catd, 1, 169, "Mailer - Text"), TRUE);

    _textPopupMenuList->add(_text_popup_title);
    _textPopupMenuList->add(_separator);
    _textPopupMenuList->add(_edit_copy);
    _textPopupMenuList->add(_edit_select_all);

    Widget parent = my_editor->textEditor()->get_editor();
    _menuPopupText = new MenuBar(parent, "ViewMsgTextPopup", XmMENU_POPUP);
    _textPopupMenu = _menuPopupText->addCommands(_textPopupMenuList, 
				FALSE, XmMENU_POPUP);

    DtEditorCreateExtrasMenu(parent, _textPopupMenu);
}



void
ViewMsgDialog::construct_help_menu()
{
    CmdList * cmdList;
    
    _overview = new OnAppCmd("Overview", catgets(DT_catd, 1, 170, "Overview"), 
				TRUE, this);
    _tasks = new TasksCmd("Tasks", catgets(DT_catd, 1, 171, "Tasks"), 
				TRUE, this);
    _reference = new ReferenceCmd("Reference", catgets(DT_catd, 1, 172, "Reference"), 
				TRUE, this);
    _messages = new MessagesCmd("Messages", catgets(DT_catd, 1, 301, "Messages"), 
				TRUE, this);
    _on_item = new OnItemCmd("On Item", catgets(DT_catd, 1, 173, "On Item"), 
				TRUE, this);
    _using_help = new UsingHelpCmd("Using Help", catgets(DT_catd, 1, 174, "Using Help"), 
				TRUE, this);
    cmdList = new CmdList( "Help", catgets(DT_catd, 1, 175, "Help") );
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

    _about_mailer = new RelNoteCmd("About Mailer", catgets(DT_catd, 1, 176, "About Mailer..."),
    				    TRUE, this);
    cmdList->add ( _about_mailer );

    // Make help menu show up on right side of menubar.
    _menuBar->addCommands ( cmdList, TRUE );

    cmdList->deleteMe();
}

void
ViewMsgDialog::save_attachment_callback(
    void *client_data,
    char *selection
)
{
    ViewMsgDialog *obj = (ViewMsgDialog *) client_data;

    obj->save_selected_attachment(selection);

}

void
ViewMsgDialog::save_selected_attachment(
    char *selection
)
{
   
    SdmError mail_error;
    
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
		    catgets(DT_catd, 1, 177, "Mailer"),		  
		    catgets(DT_catd, 2, 20, "An attachment needs to be selected before issuing the\n\"Save As\" command to save to a file.") );
	  helpId = DTMAILHELPSELECTATTACH;
	  answer = _genDialog->post_and_return(
			catgets(DT_catd, 3, 75, "OK"), helpId );
	  return;
      }

    attachment->saveToFile(mail_error, selection);

    if (mail_error) {
      _genDialog->post_error(mail_error);
    }

}

void
ViewMsgDialog::manage()
{

    Widget text;
    Dimension width, height;

    text = _parent->get_editor()->container();
    
    XtVaGetValues(text,
	XmNwidth, &width,
	XmNheight, &height,
	NULL);

    XtVaSetValues(_w,
	XmNx, _parent->x() + (_parent->width() / 2),
	XmNy, _parent->y() + (_parent->height() /2),
	NULL );

    DialogShell::manage();

}

void
ViewMsgDialog::addAttachmentActions(
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
	_menuBar->removeCommands(_attachmentMenu, _attachmentActionsList);
	_menuPopupAtt->removeCommands(_attachmentPopupMenu, 
					_attachmentActionsList);
	_attachmentActionsList->deleteMe();
	_attachmentActionsList = new CmdList("AttachmentActions", "AttachmentActions");
    }

    char *actionLabel;
    for (i = 0; i < indx; i++) {
	anAction = actions[i];
	actionLabel = DtActionLabel(anAction);  // get the localized action label
	attachActionCmd = new AttachmentActionCmd(
					anAction,
  				        actionLabel,
					this,
					i);
	free (actionLabel);
	_attachmentActionsList->add(attachActionCmd);
	
    }
    _attachmentMenu = _menuBar->addCommands(
				_attachmentMenu, 
				_attachmentActionsList
			);

    _attachmentPopupMenu = _menuPopupAtt->addCommands(
				_attachmentPopupMenu, 
				_attachmentActionsList);

}


void
ViewMsgDialog::removeAttachmentActions()
{

    // Stubbed out for now
}

void
ViewMsgDialog::invokeAttachmentAction(
    int index
)
{
    DtMailEditor *editor = this->get_editor();
    AttachArea *attacharea = editor->attachArea();
    Attachment *attachment = attacharea->getSelectedAttachment();

    attachment->invokeAction(index);

}

void
ViewMsgDialog::selectAllAttachments()
{
    
    DtMailEditor *editor = this->get_editor();
    AttachArea *attachArea = editor->attachArea();
    
    attachArea->selectAllAttachments();

}

void
ViewMsgDialog::activate_default_attach_menu()
{
    _attach_select_all->activate();
}

void
ViewMsgDialog::deactivate_default_attach_menu()
{
    _attach_select_all->deactivate();
}


void
ViewMsgDialog::showAttachArea()
{
    DtMailEditor *editor = this->get_editor();
    editor->showAttachArea();
}

void
ViewMsgDialog::hideAttachArea()
{
    DtMailEditor *editor = this->get_editor();
    editor->hideAttachArea();
}


void
ViewMsgDialog::attachmentFeedback( SdmBoolean bool )
{
    if (bool) {
	this->busyCursor();
    }
    else {
	this->normalCursor();
    }
}
