 /*
 *+SNOTICE
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993, 1994, 1995, 1996 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)OptCmd.C	1.101G"
#endif

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//         This example code is from the book:
//
//           Object-Oriented Programming with C++ and OSF/Motif
//         by
//           Douglas Young
//           Prentice Hall, 1992
//           ISBN 0-13-630252-1	
//
//
//  Permission to use, copy, modify, and distribute this software for 
//  any purpose except publication and without fee is hereby granted, provided 
//  that the above copyright notice appear in all copies of the software.
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
// OptCmd.C : creates the mail options dialog
//////////////////////////////////////////////////////////
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include "RoamApp.h"
#include "RoamCmds.h"
#include "MainWindow.h"
#include "Application.h"
#include "Help.hh"
#include "SendMsgDialog.h"
#include <DtMail/PropUi.hh>
#include <DtMail/SpinBoxUiItem.hh>
#include <DtMail/TextFieldUiItem.hh>
#include <DtMail/FileBoxUiItem.hh>
#include <DtMail/CheckBoxUiItem.hh>
#include <DtMail/RadioBoxUiItem.hh>
#include <DtMail/AntiCheckBoxUiItem.hh>
#include <DtMail/ListUiItem.hh>
#include <DtMail/IgnoreListUiItem.hh>
#include <DtMail/MoveMenuListUiItem.hh>
#include <DtMail/AlternatesListUiItem.hh>
#include <DtMail/CustomListUiItem.hh>
#include <DtMail/ToolbarListUiItem.hh>
#include <DtMail/ToolbarConfig.hh>
#include <DtMail/TemplateListUiItem.hh>
#include <DtMail/AliasListUiItem.hh>
#include <DtMail/FilterListUiItem.hh>
#include <DtMail/ScaleUiItem.hh>
#include <DtMail/DateFieldUiItem.hh>
#include <DtMail/OptCmd.h>
#include "dtmailopts.h"
#ifdef AUTOFILING
#include "Rule_edit_ui.h"
#endif // AUTOFILING
#include <DtMail/DtVirtArray.hh>
#include <DtMail/options_util.h>
#include <DtMail/Common.h>
#include <nl_types.h>
#include "MailMsg.h"
#include "DtMailHelp.hh"
#include "ExistingButtonInterface.h"
#include "Xm/MessageB.h"
#include <Dt/SpinBox.h>

#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>

extern Boolean props_changed;

static Widget toolbarRCWidget;
static Dimension max_width = 0;
static Dimension max_height = 0;
extern void template_include_file_cb(void *, char *);
extern void msg_filing_browse_cb(void *, char *);
extern void basic_select_cb(void *, char *);
extern void vacationDateChangedCB(Widget, XtPointer, XtPointer);

OptCmd::OptCmd ( char *name, char *label, int active, Widget parent) : 
                 WarnNoUndoCmd ( name, label, active ) 
{
    _dialogParentWidget = parent;

    vacation_msg_changed = FALSE;
    vacation_date_changed = FALSE;
    vacation_onoff_changed = FALSE;

    _opt_dlog = NULL;
    catstr = NULL;
    generalDialog = new DtMailGenDialog("Dialog", _dialogParentWidget);
    CurrentPane = NULL;
    _startDateFormatError = NULL;
    _endDateFormatError = NULL;
#ifdef AUTOFILING
    _rule = NULL;
#endif // AUTOFILING
    _window_alive = FALSE;
}

void
OptCmd::help_button_CB(Widget current_pane)
{
    if (current_pane == _opt_dlog->msg_header_pane)
	dtb_show_help_volume_info("Mailer.sdl", "MessageHeaderListOptions");
    else if (current_pane == _opt_dlog->msg_view_pane)
	dtb_show_help_volume_info("Mailer.sdl", "MessageViewOptions");
    else if (current_pane == _opt_dlog->toolbar_pane)
	dtb_show_help_volume_info("Mailer.sdl", "ToolbarOptions");
    else if (current_pane == _opt_dlog->compose_win_pane)
	dtb_show_help_volume_info("Mailer.sdl", "ComposeWindowOptions");
    else if (current_pane == _opt_dlog->msg_filing_pane)
	dtb_show_help_volume_info("Mailer.sdl", "MessageFilingOptions");
    else if (current_pane == _opt_dlog->vacation_pane)
	dtb_show_help_volume_info("Mailer.sdl", "VacationMailOptions");
    else if (current_pane == _opt_dlog->templates_pane)
	dtb_show_help_volume_info("Mailer.sdl", "MailTemplateOptions");
    else if (current_pane == _opt_dlog->aliases_pane)
	dtb_show_help_volume_info("Mailer.sdl", "MailAliasOptions");
    else if (current_pane == _opt_dlog->advanced_pane)
	dtb_show_help_volume_info("Mailer.sdl", "AdvancedMailOptions");
    else if (current_pane == _opt_dlog->signature_pane)
	dtb_show_help_volume_info("Mailer.sdl", "SignatureOptions");
    else if (current_pane == _opt_dlog->notification_pane)
	dtb_show_help_volume_info("Mailer.sdl", "NotificationOptions");
}

void
opt_help_button_CB(Widget wid, XtPointer clientData, XtPointer calldata)
{
    OptCmd *cmd_ptr = (OptCmd*)clientData;

    // Suppress warnings
    wid = wid;
    calldata = calldata;

    cmd_ptr->help_button_CB(cmd_ptr->currentPane());
}

void OptCmd::create_dlog()
{
    theRoamApp.busyAllWindows(
	           catgets(DT_catd, 3, 78, "Initializing Mail Options..."));

    // make a new widget structure
    _opt_dlog = (DtbOptionsDialogInfo)malloc(sizeof(
	                       DtbOptionsDialogInfoRec));
	
    // make sure it's all cleaned up
    dtbOptionsDialogInfo_clear(_opt_dlog);

    /*
     * Open the standard message catalog for the project.
     */
    Dtb_project_catd = catopen(DTB_PROJECT_CATALOG, NL_CAT_LOCALE);

    // make us some a dem widgets!  :-)
    dtb_options_dialog_initialize(_opt_dlog,
				  _dialogParentWidget);

    // We need to do this for the help system
    dtb_save_toplevel_widget(_dialogParentWidget); 

    XtSetMappedWhenManaged(_opt_dlog->dialog, False);
    XtManageChild(_opt_dlog->dialog_shellform);

    // hook them up to the back end!
    this->init_panes();

    // now that we know the overall size, set it on the layer widget
    XtVaSetValues(_opt_dlog->layer, 
		  XmNwidth,  max_width, 
		  XmNheight, max_height, 
		  NULL);

    Atom WM_DELETE_WINDOW=XmInternAtom( XtDisplay( _opt_dlog->dialog),
                                        "WM_DELETE_WINDOW",
                                        False );

    XmAddWMProtocolCallback( _opt_dlog->dialog,
                             WM_DELETE_WINDOW,
                             ( XtCallbackProc ) &OptCmd::quitCB,
                             this );

    // Attach the object to the mail options panes so they can later be referred
    // to in options_set_category_pane().

    XtVaSetValues(_opt_dlog->msg_header_pane, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->msg_view_pane, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->toolbar_pane, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->compose_win_pane, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->signature_pane, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->msg_filing_pane, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->vacation_pane, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->templates_pane, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->aliases_pane, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->advanced_pane, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->notification_pane, XmNuserData, this, NULL);
    
    XtVaSetValues(_opt_dlog->cancel_button, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->ok_button, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->apply_button, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->reset_button, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->vacation_rb_items.On_item, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->vacation_rb_items.Off_item, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->subject_tf, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->vacation_msg_tp, XmNuserData, this, NULL);
#ifdef AUTOFILING
    XtVaSetValues(_opt_dlog->auto_edit_butt, XmNuserData, this, NULL);
    XtVaSetValues(_opt_dlog->auto_new_butt, XmNuserData, this, NULL);
#endif // AUTOFILING

    XtAddCallback(_opt_dlog->help_button, XmNactivateCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->ctrl_panel, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->msg_header_pane, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->msg_view_pane, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->toolbar_pane, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->compose_win_pane, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->signature_pane, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->msg_filing_pane, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->vacation_pane, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->templates_pane, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->aliases_pane, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->advanced_pane, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->notification_pane, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);
    XtAddCallback(_opt_dlog->dialog_button_panel, XmNhelpCallback, 
		opt_help_button_CB,	(XtPointer)this);

     theRoamApp.unbusyAllWindows();
     theRoamApp.setOptionsDialog((char*)_opt_dlog);
     props_changed = FALSE;
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
int
options_set_category_pane(
    Widget	pane
)
{
  OptCmd *cmd_ptr;
  Widget current_pane;
  int answer = 0;

  XtVaGetValues(pane,
	XmNuserData, &cmd_ptr,
	NULL);

  current_pane = cmd_ptr->currentPane();
  if (pane == current_pane) 
	return answer;

  if (current_pane != NULL && props_changed) {
  	char buf[1024];

        sprintf(buf, "%s", catgets(DT_catd, 15, 1, "You have made unsaved changes.\nYou can save your changes, discard them,\nor return to your previous place in the dialog."));
 
        cmd_ptr->genDialog()->setToQuestionDialog(catgets(DT_catd, 5, 2, "Mailer"), buf);
 
        answer = cmd_ptr->genDialog()->post_and_return(catgets(DT_catd, 15, 2, "Save"),
                        catgets(DT_catd, 1, 182, "Cancel"),
                        catgets(DT_catd, 15, 3, "Discard"),
                        DTMAILDISMISSOPTIONS);
 
	// With Save always update pane because the save may have failed
        if (answer == 1) {  //Save
		cmd_ptr->update_source();
	}

        else if (answer == 2) { // Cancel
                props_changed = FALSE;
                cmd_ptr->setOldCategoryStr();
                return answer;
     	}
#if DontDoThis
        else if (answer == 3)  // Discard
		cmd_ptr->update_pane(current_pane);
#endif

  }
  props_changed = FALSE;
  XtManageChild(pane);
 
  if (current_pane != NULL)
    XtUnmanageChild(current_pane);
 
  cmd_ptr->setCurrentPane(pane);
  cmd_ptr->setNewCategoryStr();
  cmd_ptr->update_pane(current_pane);

  return answer;
}

//////////////////////////////////////////////////////////////////////////////
OptCmd::~OptCmd()
{
  _opt_dlog = NULL;
}
//////////////////////////////////////////////////////////////////////////////
// hookup the front and back ends of the mail options
void OptCmd::init_panes()
{

  this->init_msg_hdr_pane();
  this->init_msg_view_pane();
  this->init_toolbar_pane();
  this->init_compose_pane();
  this->init_signature_pane();
  this->init_msg_filing_pane();
  this->init_vacation_pane();
  this->init_template_pane();
  this->init_aliases_pane();
  this->init_advanced_pane();
  this->init_notify_pane();
}

//////////////////////////////////////////////////////////////////////////////
void OptCmd::update_panes()
{

  this->updateUiFromBackEnd(mh_pane_list);    
  this->updateUiFromBackEnd(msg_view_pane_list);    
  this->updateUiFromBackEnd(toolbar_pane_list);    
  this->updateUiFromBackEnd(compose_pane_list);    
  this->updateUiFromBackEnd(msg_filing_pane_list);    
  this->updateUiFromBackEnd(notify_pane_list); 
  this->updateUiFromBackEnd(signature_pane_list);    
  this->updateUiFromBackEnd(template_pane_list);
  this->updateUiFromBackEnd(aliases_pane_list);
  this->updateUiFromBackEnd(vacation_pane_list);
  this->updateUiFromBackEnd(advanced_pane_list);

  this->update_vacation();
  this->init_logging();

  props_changed = FALSE;
  
}
void OptCmd::update_vacation()
{
  VacationCmd *vacation_cmd = theRoamApp.vacation();

  if (vacation_cmd != NULL) {
  	vacation_cmd->resetRadioButton(vacation_cmd->vacationIsOn());
  	vacation_cmd->resetVacationMessage();
  }

  vacation_msg_changed = FALSE;
  vacation_date_changed = FALSE;
  vacation_onoff_changed = FALSE;
}

void
OptCmd::init_logging()
{
  if (XmToggleButtonGetState(_opt_dlog->checkbox2_items.Save_a_copy_of_outgoing_mail_in_item) == False)
      XtSetSensitive(_opt_dlog->sent_mail_tf, False);
}


//////////////////////////////////////////////////////////////////////////////
void OptCmd::update_source()
{

  DtMailEnv error;
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMail::MailRc * mail_rc = d_session->mailRc(error);

  if (CurrentPane == _opt_dlog->msg_header_pane) {
	this->log_text_changed();
  	this->updateBackEndFromUi(mh_pane_list);    
  }
  else if (CurrentPane == _opt_dlog->msg_view_pane)
  	this->updateBackEndFromUi(msg_view_pane_list);    
  else if (CurrentPane == _opt_dlog->toolbar_pane)
      this->updateBackEndFromUi(toolbar_pane_list);    
  else if (CurrentPane == _opt_dlog->compose_win_pane)
  	this->updateBackEndFromUi(compose_pane_list);    
  else if (CurrentPane == _opt_dlog->msg_filing_pane) 
  	this->updateBackEndFromUi(msg_filing_pane_list);    
  else if (CurrentPane == _opt_dlog->signature_pane)
  	this->updateBackEndFromUi(signature_pane_list);    
  else if (CurrentPane == _opt_dlog->templates_pane)
  	this->updateBackEndFromUi(template_pane_list);
  else if (CurrentPane == _opt_dlog->aliases_pane)
  	this->updateBackEndFromUi(aliases_pane_list);
  else if (CurrentPane == _opt_dlog->vacation_pane) {
  	this->updateBackEndFromUi(vacation_pane_list);
        this->doVacationCheck();
    }
  else if (CurrentPane == _opt_dlog->advanced_pane)
  	this->updateBackEndFromUi(advanced_pane_list);
  else if (CurrentPane == _opt_dlog->notification_pane) {
  	this->updateBackEndFromUi(notify_pane_list);
	
#ifdef AUTOFILING
	// This error dialog needs to be taken out once the new backend is
	// available and the mailfiltering problem is working. It's here mainly
	// to remind user their rules will not work at the moment.
	DtMailGenDialog *mailrc_dialog = new DtMailGenDialog("MailRcDialog",
                                     theApplication->baseWidget());
    	mailrc_dialog->setToErrorDialog(catgets(DT_catd, 2, 1, "Mailer"),
	    "If you have set up some rules in the Message Filing list, these rules\nwill be saved in the ~/.mailrules file. However, the actual message filing\nis still not working for this build.");
    	mailrc_dialog->post_and_return(catgets(DT_catd, 3, 5, "OK"), NULL);
#endif // AUTOFILING
    }

  if (mail_rc->getParseError() != DTME_NoError) {
	char* helpId;
  	int answer = 0;
    	DtMailGenDialog *mailrc_dialog = new DtMailGenDialog("MailRcDialog",
                                     theApplication->baseWidget());
    	mailrc_dialog->setToErrorDialog(catgets(DT_catd, 2, 1, "Mailer"),
    	catgets(DT_catd, 2, 25, "There were unrecoverable syntax errors found in the ~/.mailrc file.\nYou can apply Mail Options settings ONLY in the current session (the\nsettings will take effect now for as long as Mailer is running) but will\nnot be saved to the .mailrc file."));
    	helpId = DTMAILHELPERROR;
	Widget hpBut = XmMessageBoxGetChild(mailrc_dialog->baseWidget(), 
			XmDIALOG_HELP_BUTTON);
	XtSetSensitive(hpBut, False);
    	mailrc_dialog->post_and_return(catgets(DT_catd, 3, 5, "OK"), helpId);
	return;
  }
  else
  	mail_rc->update(error);

  theRoamApp.globalPropChange();
  // This resets flag for updating alias list on File menu
  XtVaSetValues(_opt_dlog->alias_tf, XmNuserData, 0, NULL);
  props_changed = FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// Handle the vacation stuff.

void OptCmd::doVacationCheck()
{
  Boolean is_set;
  DtMailGenDialog *dialog = theRoamApp.genDialog();
  char buf[1024];
 
  VacationCmd *vacation_cmd = theRoamApp.vacation();

  if (!vacation_cmd) return;


  if (vacation_msg_changed) {
	char *subj, *body;
	XtVaGetValues(_opt_dlog->subject_tf, XmNvalue, &subj, NULL);
	XtVaGetValues(_opt_dlog->vacation_msg_tp, XmNvalue, &body, NULL);
	vacation_cmd->setSubject(subj);
	vacation_cmd->setBody(body);
	vacation_cmd->handleMessageFile();
	vacation_msg_changed = FALSE;
  }

  if (XtIsManaged(_opt_dlog->vacation_pane)) 
  	XtVaGetValues(_opt_dlog->vacation_rb_items.On_item,
		  XmNset, &is_set, NULL);

  // Check for date format errors.
  if (_startDateFormatError) {
  	if (is_set) 
		strcpy(buf, catgets(DT_catd, 15, 10, "The start date format is incorrect.\nUse the format shown next to the End Date.\nThe vacation notifier will not take effect."));
	else
		strcpy(buf, catgets(DT_catd, 15, 13, "The start date format is incorrect.\nUse the format shown next to the End Date."));
	dialog->setToErrorDialog(catgets(DT_catd, 3, 50, "Mailer"), buf);
	dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"),
		DTMAILHELPVACATIONINVALIDSTARTDATE);
	vacation_cmd->resetVacationItems();
        theRoamApp.resetTitles();
   }
   else if (_endDateFormatError) {
  	if (is_set) 
		strcpy(buf, catgets(DT_catd, 15, 11, "The end date format is incorrect.\nUse the format shown next to the End Date.\nThe vacation notifier will not take effect."));
	else
		strcpy(buf, catgets(DT_catd, 15, 14, "The end date format is incorrect.\nUse the format shown next to the End Date."));
	dialog->setToErrorDialog(catgets(DT_catd, 3, 50, "Mailer"), buf);
	dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"),
		DTMAILHELPVACATIONINVALIDENDDATE);
	vacation_cmd->resetVacationItems();
        theRoamApp.resetTitles();
  }
  else {
  	if (is_set) 
		this->startVacation();
  	else
		this->stopVacation();
  }
  // Reset these 
  _startDateFormatError = FALSE;
  _endDateFormatError = FALSE;

  // If the vacation button is off, they updated the date field and did not
  // make any changes to the radio buttons (On Off)
  if (vacation_date_changed && !vacation_onoff_changed && !is_set) {
	dialog->setToErrorDialog(catgets(DT_catd, 3, 71, "Mailer"),
		 catgets(DT_catd, 15, 9, "You did not turn on the vacation notifier.\nTo do so, check the On button under Vacation")); 
	dialog->post_and_return( catgets(DT_catd, 3, 5, "OK"),
                             DTMAILHELPVACATIONTURNONBUTTON);
   }
   vacation_date_changed = FALSE;
   vacation_onoff_changed = FALSE;
}
void OptCmd::setDateChanged()
{
  vacation_date_changed = TRUE;
  props_changed = TRUE;
}
void OptCmd::onOffButtonChanged()
{
  vacation_onoff_changed = TRUE;
  props_changed = TRUE;
}

void OptCmd::set_vaca_msg_Changed()
{
  vacation_msg_changed = TRUE;
  props_changed = TRUE;
}
void
OptCmd::startVacation()
{
  theRoamApp.startVacation();
}


void
OptCmd::stopVacation()
{
	theRoamApp.stopVacation();
}

//////////////////////////////////////////////////////////////////////////////
// Handle Log message
void
OptCmd::log_text_changed() 
{
        XmString str=NULL;
        XtVaGetValues(_opt_dlog->sent_mail_tf, XmNvalue, &str, NULL);

	// If "Mailbox for sent messages" textfield is empty, popup a warning
	// dialog and reset the text to the default value
        if (*str == NULL) {
	    if (XmToggleButtonGetState(_opt_dlog->checkbox2_items.Save_a_copy_of_outgoing_mail_in_item) == True) {
		DtMailGenDialog *dialog = genDialog();
		dialog->setToErrorDialog(catgets(DT_catd, 3, 71, "Mailer"),
                                 catgets(DT_catd, 3, 98, "You did not specify a file for saving outgoing mail."));
		int answer = dialog->post_and_return(
		             catgets(DT_catd, 3, 99, "Use Default File"),
		             catgets(DT_catd, 3, 100, "Turn Off Feature"),
		             DTMAILHELPLOGDEFAULT);

		if (answer == 2) {
		    XtVaSetValues(_opt_dlog->checkbox2_items.Save_a_copy_of_outgoing_mail_in_item,
                          XmNset, False, NULL);
		    XtSetSensitive(_opt_dlog->sent_mail_tf, False);
		}
	    }

	    XtVaSetValues(_opt_dlog->sent_mail_tf, XmNvalue, "$HOME/Mail/sent.mail", NULL);
        }
        else
		XmStringFree(str);
    
}

//////////////////////////////////////////////////////////////////////////////
void 
OptCmd::handleAutoEdit()
{

}


//////////////////////////////////////////////////////////////////////////////
void register_pane_size(Widget pane)
{
    Dimension   width, height;

    XtManageChild(pane);
    XtVaGetValues(pane,
                XmNwidth, &width, XmNheight, &height,
                NULL);

    if (width > max_width)
        max_width = width;

    if (height > max_height)
        max_height = height;

    XtUnmanageChild(pane);
}

//////////////////////////////////////////////////////////////////////////////
// init the message header list pane
void OptCmd::init_msg_hdr_pane()
{

  PropUiItem *propui_ptr = NULL;

  // register overall size of pane
  register_pane_size(_opt_dlog->msg_header_pane);

  // test for the message header list pane
  
  // make the list...
  mh_pane_list = new DtVirtArray<PropUiItem *>(3);

  // make the check for new mail SB
  propui_ptr = (PropUiItem *)new SpinBoxUiItem(_opt_dlog->check_new_mail_sb,
					       _FROM_MAILRC,
					       "retrieveinterval");

  mh_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new AntiCheckBoxUiItem(
		_opt_dlog->save_mb_every_cb_items.Save_mailbox_every_item,
                                                _FROM_MAILRC,
                                                "dontautosave");
  mh_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new SpinBoxUiItem(
		_opt_dlog->save_mb_sb,
                                                _FROM_MAILRC,
                                                "saveinterval");
  mh_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new AntiCheckBoxUiItem(
 	 _opt_dlog->checkbox2_items.Save_a_copy_of_outgoing_mail_in_item,
 						_FROM_MAILRC,
 						"dontlogmessages");
  mh_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new TextFieldUiItem(_opt_dlog->sent_mail_tf,
						 _FROM_MAILRC,
						 "record");
  mh_pane_list->append(propui_ptr);

  // make the disp headers SB
  propui_ptr = (PropUiItem *)new SpinBoxUiItem(_opt_dlog->display_sb,
					       _FROM_MAILRC,
					       "headerlines");
  mh_pane_list->append(propui_ptr);

  // make the sho "To: Recipient"
  propui_ptr = (PropUiItem *)new CheckBoxUiItem(
	 _opt_dlog->checkbox_items.Show_To_recipient_when_mail_is_from_me_item,
						_FROM_MAILRC,
						"showto");
  mh_pane_list->append(propui_ptr);

  // make the sho "Display message numbers"
  propui_ptr = (PropUiItem *)new CheckBoxUiItem(
	 _opt_dlog->checkbox_items.Display_message_numbers_item,
						_FROM_MAILRC,
						"showmsgnum");
  mh_pane_list->append(propui_ptr);

  // make the sho "destroy on close"
  propui_ptr = (PropUiItem *)new AntiCheckBoxUiItem(
	 _opt_dlog->deleted_msg_cb_items.When_I_close_the_mailbox_item,
						_FROM_MAILRC,
						"keepdeleted");
  mh_pane_list->append(propui_ptr);

  // make the sho "show confirm notice"
  propui_ptr = (PropUiItem *)new AntiCheckBoxUiItem(
	 _opt_dlog->deleted_msg_cb_items.Show_confirmation_notice_item,
						_FROM_MAILRC,
						"quietdelete");
  mh_pane_list->append(propui_ptr);
  
}

//////////////////////////////////////////////////////////////////////////////
void OptCmd::init_msg_view_pane()
{
  PropUiItem *propui_ptr = NULL;
  ListUiItem *list_ptr = NULL;

  // register overall size of pane 
  register_pane_size(_opt_dlog->msg_view_pane); 
  
  // make the list...
  msg_view_pane_list = new DtVirtArray<PropUiItem *>(2);

  //---
  propui_ptr = (PropUiItem *)new SpinBoxUiItem(_opt_dlog->display_row_sb,
					       _FROM_MAILRC,
					       "popuplines");
  msg_view_pane_list->append(propui_ptr);

  //---
  propui_ptr = (PropUiItem *)new SpinBoxUiItem(_opt_dlog->characters_wide_sb,
					       _FROM_MAILRC,
					       "toolcols");
  msg_view_pane_list->append(propui_ptr);

  list_ptr = (ListUiItem *)new IgnoreListUiItem(_opt_dlog->hidden_fields_list,
					    _FROM_MAILRC,
					    "ignore",
					    _opt_dlog->fields_hide);
  
  add_cbs_to_list(list_ptr,
		  _opt_dlog->h_field_add,
		  _opt_dlog->h_field_delete,
		  _opt_dlog->h_field_change);
    
  msg_view_pane_list->append(list_ptr);

}


//////////////////////////////////////////////////////////////////////////////
void OptCmd::init_toolbar_pane()
{
    ListUiItem *list_ptr = NULL;
    PropUiItem *prop_ptr = NULL;
    ToolbarConfig *toolbar_conf = NULL;

    register_pane_size(_opt_dlog->toolbar_pane); 
    toolbar_pane_list = new DtVirtArray<PropUiItem *>(3);

    toolbarRCWidget = XtVaCreateManagedWidget("Toolbar",
					      xmRowColumnWidgetClass,
					      _opt_dlog->toolbar_preview_pane,
					      XmNorientation, XmHORIZONTAL,
					      XmNpacking, XmPACK_TIGHT,
					      XmNspacing, 10,
					      XmNresizeHeight, True,
					      XmNresizeWidth, False,
					      XmNadjustLast, False,
					      XmNwidth, max_width,
					      XmNx, 0,
					      XmNy, 0,
					      NULL);

    toolbar_conf = new ToolbarConfig(toolbarRCWidget,
				     _opt_dlog->toolbar_my_cmds,
				     _opt_dlog->toolbar_status_help);

    list_ptr = (ListUiItem *)new
		ToolbarListUiItem(_opt_dlog->toolbar_my_cmds,
				  toolbar_conf,
				  _FROM_MAILRC,
				  "toolbarcommands",
				  _opt_dlog->toolbar_all_cmds);

    XtVaSetValues(_opt_dlog->toolbar_add_but, XmNuserData, list_ptr, NULL);
    XtVaSetValues(_opt_dlog->toolbar_delete_but, XmNuserData, list_ptr, NULL);
    XtVaSetValues(_opt_dlog->toolbar_move_up_but, XmNuserData,list_ptr, NULL);
    XtVaSetValues(_opt_dlog->toolbar_move_down_but,XmNuserData,list_ptr,NULL);

    toolbar_pane_list->append(list_ptr);

    prop_ptr = (PropUiItem *) new
		RadioBoxUiItem(_opt_dlog->toolbar_display_type_rb,
			       toolbar_conf,
			       _FROM_MAILRC,
			       "toolbarusetext",
			       _opt_dlog->toolbar_my_cmds);

    toolbar_pane_list->append(prop_ptr);
}

//////////////////////////////////////////////////////////////////////////////
void OptCmd::init_compose_pane()
{
  PropUiItem *propui_ptr = NULL;

  DtVirtArray<char *> fields_list(10);
  ListUiItem *list_ptr = NULL;

  // register overall size of pane 
  register_pane_size(_opt_dlog->compose_win_pane); 

  // make the list...
  compose_pane_list = new DtVirtArray<PropUiItem *>(1);

  propui_ptr = (PropUiItem *)new SpinBoxUiItem(
	_opt_dlog->compose_display_sb,
					       _FROM_MAILRC,
					       "composewincols");
  compose_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new AntiCheckBoxUiItem(
	_opt_dlog->wrap_checkbox_items.Show_Attachment_List_item,
					       _FROM_MAILRC,
					       "hideattachments");
  compose_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new TextFieldUiItem(_opt_dlog->indent_str_tf,
					       _FROM_MAILRC,
					       "indentprefix");
  compose_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new TextFieldUiItem(_opt_dlog->deadletter_field,
					       _FROM_MAILRC,
					       "deaddir");
  compose_pane_list->append(propui_ptr);

  list_ptr = (ListUiItem *)new CustomListUiItem(_opt_dlog->list,
						  _FROM_MAILRC,
						  "",
						  _opt_dlog->hdr_field,
						  _opt_dlog->def_value_field);
  add_cbs_to_list(list_ptr,
		  _opt_dlog->add_button,
		  _opt_dlog->del_button,
		  _opt_dlog->change_button);
  
  compose_pane_list->append(list_ptr);

}

//////////////////////////////////////////////////////////////////////////////
void OptCmd::init_msg_filing_pane()
{
  const char *list_str = NULL;
  DtVirtArray<char *> move_menu_list(10);
  PropUiItem *propui_ptr = NULL;
  SelectFileCmd *sfc;

  // register overall size of pane 
  register_pane_size(_opt_dlog->msg_filing_pane); 

  ListUiItem *list_ptr = NULL;

  // make the list...
  msg_filing_pane_list = new DtVirtArray<PropUiItem *>(5);

  propui_ptr = (PropUiItem *)new TextFieldUiItem(_opt_dlog->srt_looking_tf,
						 _FROM_MAILRC,
						 "folder");
  msg_filing_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new AntiCheckBoxUiItem(
	_opt_dlog->dpy_up_mbox_cb_items.Display_up_to_item,
					       _FROM_MAILRC,
					       "dontdisplaycachedfiles");
  msg_filing_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new SpinBoxUiItem(_opt_dlog->recent_sb,
					       _FROM_MAILRC,
					       "cachedfilemenusize");
  msg_filing_pane_list->append(propui_ptr);

  list_ptr = (ListUiItem *)new MoveMenuListUiItem(_opt_dlog->mv_menu_lst,
						  _FROM_MAILRC,
						  "",
						  _opt_dlog->path_file_name_tf);

  XtVaSetValues(_opt_dlog->auto_up_butt2, XmNuserData, list_ptr, NULL);
  XtVaSetValues(_opt_dlog->auto_down_butt2, XmNuserData, list_ptr, NULL);
  XtVaSetValues(_opt_dlog->alphalist_but, XmNuserData, list_ptr, NULL);

  add_cbs_to_list(list_ptr,
		  _opt_dlog->mm_add_butt,
		  _opt_dlog->mm_delete_butt,
		  _opt_dlog->mm_change_butt);

  msg_filing_pane_list->append(list_ptr);

  // Add connection to the browse button to bring up a FSB
  sfc = new SelectFileCmd ("Include...",
                 catgets(DT_catd, 1, 122, "Include..."),
                 catgets(DT_catd, 1, 123, "Mailer - Include"),
                 catgets(DT_catd, 1, 124, "Include"),
                 TRUE,
                 msg_filing_browse_cb,
                 list_ptr,
                 _dialogParentWidget);

  char *path = theRoamApp.getFolderDir(TRUE);
  sfc->setDirectory(path);
  free(path);

  new ExistingButtonInterface( _opt_dlog->mm_browse_butt, sfc);

}

void OptCmd::init_signature_pane()
{
  const char *list_str = NULL;
  DtVirtArray<char *> move_menu_list(10);
  PropUiItem *propui_ptr = NULL;

  // register overall size of pane 
  register_pane_size(_opt_dlog->signature_pane); 

  // make the list...
  signature_pane_list = new DtVirtArray<PropUiItem *>(2);

  propui_ptr = (PropUiItem *)new TextFieldUiItem(_opt_dlog->signature_file_tf,
						 _FROM_MAILRC,
						 "signature");
  signature_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new FileBoxUiItem(_opt_dlog->signature_tp,
						 _FROM_MAILRC,
						 "signature");
  signature_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new AntiCheckBoxUiItem(
                _opt_dlog->signature_cb_items.Include_signature_in_every_message_item,
                                                _FROM_MAILRC,
                                                "dontincludesignature");
  signature_pane_list->append(propui_ptr);

}
//////////////////////////////////////////////////////////////////////////////
void OptCmd::init_vacation_pane()
{
  VacationCmd *vacation_cmd = theRoamApp.vacation();
  PropUiItem *propui_ptr = NULL;

  // register overall size of pane 
  register_pane_size(_opt_dlog->vacation_pane); 

  // make the list...
  vacation_pane_list = new DtVirtArray<PropUiItem *>(5);

  // make the Vacation Reply Interval
  propui_ptr = (PropUiItem *)new SpinBoxUiItem(_opt_dlog->reply_vacation_every_sb,
                                               _FROM_MAILRC,
                                               "vacationreplyinterval");
  vacation_pane_list->append(propui_ptr);

  // make the Vacation start date
  propui_ptr = (DateFieldUiItem *)new DateFieldUiItem(_opt_dlog->startdate_tf,
                                               _FROM_MAILRC,
                                               "vacationstartdate");
  vacation_pane_list->append(propui_ptr);

  // make the Vacation end date
  propui_ptr = (PropUiItem *)new DateFieldUiItem(_opt_dlog->enddate_tf,
                                               _FROM_MAILRC,
                                               "vacationenddate");
  vacation_pane_list->append(propui_ptr);

  XtAddCallback(_opt_dlog->startdate_tf, XmNvalueChangedCallback,
		vacationDateChangedCB, this);
  XtAddCallback(_opt_dlog->enddate_tf, XmNvalueChangedCallback, 
		vacationDateChangedCB, this);

  vacation_cmd->parseVacationMessage();

  vacation_msg_changed = FALSE;
  vacation_date_changed = FALSE;
  vacation_onoff_changed = FALSE;
}

//////////////////////////////////////////////////////////////////////////////
void OptCmd::init_template_pane()
{
  ListUiItem *list_ptr = NULL;
  SelectFileCmd *sfc;
  DtMailEnv error;
  DtMail::Session * d_session = theRoamApp.session()->session();
  DtMail::MailRc * mail_rc = d_session->mailRc(error);

  // register overall size of pane 
  register_pane_size(_opt_dlog->templates_pane); 

  // make the list...
  template_pane_list = new DtVirtArray<PropUiItem *>(3);

  list_ptr = (ListUiItem *)new TemplateListUiItem(_opt_dlog->templates_list,
						  _FROM_MAILRC,
						  "templates");
  template_pane_list->append(list_ptr);

  XtVaSetValues(_opt_dlog->edit_template_button, XmNuserData, list_ptr, NULL);
  XtVaSetValues(_opt_dlog->delete_template_button, XmNuserData, list_ptr, NULL);
  XtVaSetValues(_opt_dlog->rename_template_button, XmNuserData, list_ptr, NULL);
  XtVaSetValues(_opt_dlog->templates_list, XmNuserData, list_ptr, NULL);

  sfc = new SelectFileCmd ("Import...",
                 catgets(DT_catd, 1, 293, "Import..."),
                 catgets(DT_catd, 1, 294, "Mailer - Import"),
                 catgets(DT_catd, 1, 295, "Import"),
                 TRUE,
                 template_include_file_cb,
                 list_ptr,
                 _dialogParentWidget);
   
  new ExistingButtonInterface( _opt_dlog->include_template_button, sfc);
}

//////////////////////////////////////////////////////////////////////////////
void OptCmd::init_aliases_pane()
{
  ListUiItem *list_ptr = NULL;

  // register overall size of pane 
  register_pane_size(_opt_dlog->aliases_pane); 

  // make the list...
   aliases_pane_list = new DtVirtArray<PropUiItem *>(3);

  list_ptr = (ListUiItem *)new AliasListUiItem(_opt_dlog->alias_list,
					       _FROM_MAILRC,
					       "alias",
					       _opt_dlog->alias_tf,
					       _opt_dlog->addresses_tf);

  add_cbs_to_list(list_ptr,
		  _opt_dlog->alias_add_but,
		  _opt_dlog->alias_delete_but,
		  _opt_dlog->al_change_but);

  aliases_pane_list->append(list_ptr);

}

//////////////////////////////////////////////////////////////////////////////
void OptCmd::init_advanced_pane()
{
  DtVirtArray<char *> alternates_list(8);
  PropUiItem *propui_ptr = NULL;
  uid_t euid;
  struct passwd *user_passwd = NULL;
  char *label_str = NULL;
  XmString label_xm_str;

  // register overall size of pane 
  register_pane_size(_opt_dlog->advanced_pane); 

  ListUiItem *list_ptr = NULL;

  label_str = (char *)malloc(256);

  // set the list to be a mono spaced font...


  // make the list...
  advanced_pane_list = new DtVirtArray<PropUiItem *>(6);

  propui_ptr = (PropUiItem *)new CheckBoxUiItem(
                _opt_dlog->warn_msg_cb_items.Warn_if_message_exceeds_item,
                                                _FROM_MAILRC,
                                                "displaymsgsizewarning");
  advanced_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new SpinBoxUiItem(
                _opt_dlog->warn_msg_sb,
                                                _FROM_MAILRC,
                                                "msgsizelimit");
  advanced_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new AntiCheckBoxUiItem(
	      _opt_dlog->adv_def_cb_items.Show_confirmation_notices_item,
						_FROM_MAILRC,	
						"expert");
  advanced_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new CheckBoxUiItem(
      _opt_dlog->adv_def_cb_items.Use_strict_MIME_character_encoding_item,
						_FROM_MAILRC,	
						"strictmime");
  advanced_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new AntiCheckBoxUiItem(
      _opt_dlog->adv_def_cb_items.Use_network_aware_mail_file_locking_item,
						_FROM_MAILRC,	
						"cdenotooltalklock");
  advanced_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new CheckBoxUiItem(
	      _opt_dlog->adv_def_cb_items.Keep_empty_mailboxes_item,
						_FROM_MAILRC,	
						"keep");
  advanced_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new CheckBoxUiItem(
      _opt_dlog->adv_reply_cb_items.Ignore_host_name_in_address_item,
						_FROM_MAILRC,	
						"allnet");
  advanced_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new CheckBoxUiItem(
      _opt_dlog->adv_reply_cb_items.Include_me_item,
						_FROM_MAILRC,	
						"metoo");
  advanced_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new CheckBoxUiItem(
	      _opt_dlog->adv_reply_cb_items.Treat_the_following_as_username_item,
						_FROM_MAILRC,	
						"usealternates");
  advanced_pane_list->append(propui_ptr);

  // set uid in string...
  euid = geteuid();  // get uid of user

  user_passwd = getpwuid(euid); // get passwd entry for user 

  sprintf(label_str,
		catgets(DT_catd, 15, 4, "Treat the following as: \"%s\" ="), 
		user_passwd->pw_name);

  label_xm_str = XmStringCreate(label_str, XmSTRING_DEFAULT_CHARSET);

  XtVaSetValues(_opt_dlog->adv_reply_cb_items.Treat_the_following_as_username_item,
		XmNlabelString, label_xm_str,
		NULL);


  list_ptr = (ListUiItem *)new AlternatesListUiItem(_opt_dlog->local_name_list,
						    _FROM_MAILRC,
						    "alternates",
						    _opt_dlog->local_name_tf);
  
  add_cbs_to_list(list_ptr,
		  _opt_dlog->local_name_add_but,
		  _opt_dlog->local_name_del_but,
		  _opt_dlog->local_name_chg_but);
    

  advanced_pane_list->append(list_ptr);

}

//////////////////////////////////////////////////////////////////////////////
void OptCmd::init_notify_pane()
{
  ListUiItem *list_ptr = NULL;
  PropUiItem *propui_ptr = NULL;

#ifdef AUTOFILING
  // Initialize the Rule Editor dialog here 
  _rule = new RuleCmd("Rules...",
      catgets(DT_catd, 4, 5,"Rules..."),
      TRUE,
      _opt_dlog->dialog);
  _rule->execute();
#endif // AUTOFILING


  // register overall size of pane 
  register_pane_size(_opt_dlog->notification_pane); 

  notify_pane_list = new DtVirtArray<PropUiItem *>(3);

// Unmanage until we hook back in later
  XtVaSetValues(_opt_dlog->separator13,
		XmNtopAttachment, XmATTACH_FORM,
		NULL);
  XtVaSetValues(_opt_dlog->signal_boxes_group,
		XmNbottomAttachment, XmATTACH_FORM,
		NULL);
  XtUnmanageChild(_opt_dlog->up_down_gp);
  XtUnmanageChild(_opt_dlog->group);
  XtUnmanageChild(_opt_dlog->auto_list);
  XtUnmanageChild(_opt_dlog->msg_filing_lbl2);
  XtUnmanageChild(_opt_dlog->msg_filing_lbl);
  XtUnmanageChild(_opt_dlog->separator13);

// Unmanage

#ifdef AUTOFILING 

  list_ptr = (ListUiItem *)new FilterListUiItem(_opt_dlog->auto_list,
                                                _FROM_MAILRC,
                                                "rule",
                                                _rule);

  notify_pane_list->append(list_ptr);

  XtVaSetValues(_opt_dlog->auto_up_butt, XmNuserData,list_ptr, NULL);
  XtVaSetValues(_opt_dlog->auto_down_butt,XmNuserData,list_ptr,NULL);
  XtVaSetValues(_opt_dlog->auto_delete_butt, XmNuserData, list_ptr, NULL);
#endif // AUTOFILING

  // make the signal with beep SB
  propui_ptr = (PropUiItem *)new SpinBoxUiItem(_opt_dlog->beep_sb2,
					       _FROM_MAILRC,
					       "bell");
  notify_pane_list->append(propui_ptr);

  propui_ptr = (PropUiItem *)new TextFieldUiItem(_opt_dlog->audio_tf2,
					       _FROM_MAILRC,
					       "realsound");
  notify_pane_list->append(propui_ptr);

  // Add connection to the Select button to bring up a FSB
  SelectFileCmd *sfc = new SelectFileCmd ("Select...",
                 catgets(DT_catd, 1, 296, "Select..."),
                 catgets(DT_catd, 1, 297, "Mailer - Select"),
                 catgets(DT_catd, 1, 298, "Select"),
                 TRUE,
                 basic_select_cb,
                 propui_ptr,
                 _dialogParentWidget);

  new ExistingButtonInterface( _opt_dlog->sound_browse_button2, sfc);

  // make the sig with flashes SB
  propui_ptr = (PropUiItem *)new SpinBoxUiItem(_opt_dlog->flash_sb2,
					       _FROM_MAILRC,
					       "flash");
  notify_pane_list->append(propui_ptr);

}

//////////////////////////////////////////////////////////////////////////////
void OptCmd::doit()
{
  // make sure all the sheets are uptodate
  this->update_panes();
  XtManageChild(_opt_dlog->dialog_shellform);
  XtPopup(_opt_dlog->dialog, XtGrabNone);
  props_changed = FALSE;
  _window_alive = TRUE;
}       
//////////////////////////////////////////////////////////////////
void OptCmd::updateUiFromBackEnd(DtVirtArray<PropUiItem *> *list)
{
  int len, i;

  if(list != NULL)
    {
      len = list->length();
      
      for(i = 0; i < len; i++)
	(*list)[i]->writeFromSourceToUi();
    }
  else
    fprintf(stderr,"NULL list passed to updateUiFromBackEnd\n");
  
}
//////////////////////////////////////////////////////////////////
void OptCmd::updateBackEndFromUi(DtVirtArray<PropUiItem *> *list)
{
  int len, i;

  if(list != NULL)
    {
      len = list->length();
      
      for(i = 0; i < len; i++)
	(*list)[i]->writeFromUiToSource();
    }
  else
    fprintf(stderr,"NULL list passed to updateBackEndFromUi\n");
  
}


//////////////////////////////////////////////////////////////////////////////
void OptCmd::unmanage()
{
    int answer = 0;

    if (props_changed) {
	char buf[1024];

        sprintf(buf, "%s", catgets(DT_catd, 15, 1, "You have made unsaved changes.\nYou can save your changes, discard them,\nor return to your previous place in the dialog."));

        this->genDialog()->setToQuestionDialog(catgets(DT_catd, 5, 2, "Mailer"), buf);

        answer = this->genDialog()->post_and_return(catgets(DT_catd, 15, 2, "Save"),
                        catgets(DT_catd, 1, 182, "Cancel"),
                        catgets(DT_catd, 15, 3, "Discard"),
                        DTMAILDISMISSOPTIONS);

        if (answer == 1)  //Save
                this->update_source();
        else if (answer == 2)  // Cancel
                this->setOldCategoryStr();
        else if (answer == 3)  // Discard
                this->update_pane(this->currentPane());
    }
    else // This may occur if user selects the cancel button from the main options pane,
	//  and the previous cancel came from message dialog above.  In this case, 
	// the popup may still contain old values (props_changed will be false, but the
        // pane does not reflect .mailrc values). To be safe we update pane.
	this->update_pane(this->currentPane());
    if (answer != 2) {
  	XtUnmanageChild(_opt_dlog->dialog_shellform);
  	XtPopdown(_opt_dlog->dialog);
	_window_alive = FALSE;
    	props_changed = FALSE;
    }
}

//////////////////////////////////////////////////////////////////////////////
void
OptCmd::execute()
{
    _opt_dlog = (DtbOptionsDialogInfo)theRoamApp.optionsDialog();	
    if (_opt_dlog == NULL) {
	this->create_dlog();
    	this->doit();
    }
    else {
	XtManageChild(_opt_dlog->dialog_shellform);
	XtPopup(_opt_dlog->dialog, XtGrabNone);
	this->update_pane(this->currentPane());
 	props_changed = FALSE;
	_window_alive = TRUE;
    }
}

//////////////////////////////////////////////////////////////////////////////
void OptCmd::setWithCategoryStr(char *str)
{
	if (catstr != NULL)
		XmStringFree(catstr);
 	catstr = XmStringCreateLocalized(str);
	setOldCategoryStr();
}
//////////////////////////////////////////////////////////////////////////////
void OptCmd::setNewCategoryStr()
{
	if (catstr != NULL)
		XmStringFree(catstr);
        XtVaGetValues(XmOptionButtonGadget(_opt_dlog->category_opmenu), XmNlabelString, &catstr, NULL);
}

//////////////////////////////////////////////////////////////////////////////
void OptCmd::setOldCategoryStr()
{
        XtVaSetValues(XmOptionButtonGadget(_opt_dlog->category_opmenu), XmNlabelString, catstr, NULL);
}

void
OptCmd::update_pane(Widget current_pane)
{
    if (current_pane == _opt_dlog->msg_header_pane)
        this->updateUiFromBackEnd(mh_pane_list);
    else if (current_pane == _opt_dlog->msg_view_pane)
	this->updateUiFromBackEnd(msg_view_pane_list);
    else if (current_pane == _opt_dlog->toolbar_pane)
	this->updateUiFromBackEnd(toolbar_pane_list);
    else if (current_pane == _opt_dlog->compose_win_pane)
    	this->updateUiFromBackEnd(compose_pane_list);
    else if (current_pane == _opt_dlog->signature_pane)
    	this->updateUiFromBackEnd(signature_pane_list);
    else if (current_pane == _opt_dlog->msg_filing_pane)
	this->updateUiFromBackEnd(msg_filing_pane_list);
    else if (current_pane == _opt_dlog->vacation_pane) {
        this->updateUiFromBackEnd(vacation_pane_list);
        this->update_vacation();
    }
    else if (current_pane == _opt_dlog->templates_pane)
	this->updateUiFromBackEnd(template_pane_list);
    else if (current_pane == _opt_dlog->aliases_pane)
        this->updateUiFromBackEnd(aliases_pane_list);
    else if (current_pane == _opt_dlog->advanced_pane)
        this->updateUiFromBackEnd(advanced_pane_list);
    else if (current_pane == _opt_dlog->notification_pane)
        this->updateUiFromBackEnd(notify_pane_list);

    props_changed = FALSE;

}

#ifdef AUTOFILING
void 
OptCmd::clear_rule_dialog()
{
    _rule->clear_dialog();
}

void
OptCmd::manage_rule_dialog()
{
    _rule->manage_dialog();
}

void
OptCmd::update_rule_dialog()
{
    _rule->update_fields();
}
#endif // AUTOFILING

void
OptCmd::quitCB( Widget, XtPointer clientData, XmAnyCallbackStruct *)
{
    DtbOptionsDialogInfo od = (DtbOptionsDialogInfo)theRoamApp.optionsDialog();
    OptCmd *oc=( OptCmd *) clientData;

    XtUnmanageChild(od->dialog_shellform);
    XtPopdown(od->dialog);
    oc->setWindowAlive(FALSE);
    props_changed = FALSE;
}

