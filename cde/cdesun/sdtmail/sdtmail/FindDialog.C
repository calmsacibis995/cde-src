/*
 *+SNOTICE
 *
 *	$Revision: 1.2 $
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
#pragma ident "@(#)FindDialog.C	1.79 05/30/97"
#endif

#include <assert.h>
#include <X11/Intrinsic.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/TextF.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/PanedW.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/SeparatoG.h>
#include <Xm/CascadeBG.h>
#include <Dt/MenuButton.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Vector.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Search.hh>
#include <SDtMail/String.hh>
#include "FindDialog.h"
#include "RoamApp.h"
#include "RoamMenuWindow.h"
#include "RoamCmds.h"
#include "Help.hh"
#include "MailMsg.h"


//
// Clear out the data. After this function is complete the
// data should look as if the constructor was just called and
// before initialize().
//
void
FindDialog::clear()
{
  register unsigned int		offset;

  if (_text_labels != NULL) {
    for (offset = 0; offset < _num_text_fields; offset++) {
      if (_text_labels[offset] != NULL) {
	free(_text_labels[offset]);
	_text_labels[offset] = NULL;
      }
    }
    delete [] _text_labels;
  }

  if (_text_values != NULL) {
    for (offset = 0; offset < _num_text_fields; offset++) {
      if (_text_values[offset] != NULL) {
	free(_text_values[offset]);
	_text_values[offset] = NULL;
      }
    }
    delete [] _text_values;
  }

  if (_buttonData != NULL) {
    for (offset = 0; offset < _num_buttons; offset++) {
      if (_buttonData[offset].label != NULL) {
	free(_buttonData[offset].label);
	_buttonData[offset].label = NULL;
      }
    }
    delete [] _buttonData;
  }

  if (_text_names != NULL) {
    for (offset = 0; offset < _num_text_fields; offset++) {
      if (_text_names[offset] != NULL) {
	free(_text_names[offset]);
	_text_names[offset] = NULL;
      }
    }
    delete [] _text_names;
  }
}

//
// The only constructor.
//
FindDialog::FindDialog(RoamMenuWindow *parent)
    : Dialog("find", parent), _num_text_fields(5)
{
  _srch_crit = new SdmSearch;
  _roamWindow = parent;
  _viewsDialog = NULL;
  _view_name = NULL;
  _num_buttons = 4;
  _script_is_hidden = Sdm_True;
  _save_as_view_dialog = NULL;
  _basic_str = XmStringCreateLocalized(catgets(DT_catd, 1, 303, "Basic"));
  _advanced_str = XmStringCreateLocalized(catgets(DT_catd, 1, 304, "Advanced"));

  //
  // Allocate storage for labels, widgets, and data.
  //
  _text_labels = new char *[_num_text_fields];
  _text_names = new char *[_num_text_fields];
  _text_values = new char *[_num_text_fields];
  _text_fields = new Widget[_num_text_fields];
  _buttonData = new ActionAreaItem[_num_buttons];
  _searchForward = TRUE;

  //
  // private data members (widgets)
  //
  _search_type = NULL;
  _save_as_view_dialog = NULL;
  _simple_form = NULL;
  _script_form = NULL;
  _script_area = NULL;
  _status_text = NULL;

  // set the _text_values array to all NULL pointers
  clearValues();

  //
  // Initialize the buttons.
  //
   /*
    * NL_COMMENT
    * This message replaces message 184 in set 1
    */
  _buttonData[0].label = strdup(catgets(DT_catd, 1, 220, "Select All"));
  _buttonData[0].callback = findSelectAllCallback;
  _buttonData[0].data = (caddr_t) this;

  _buttonData[1].label = strdup(catgets(DT_catd, 1, 311, "Clear"));
  _buttonData[1].callback = clearCallback;
  _buttonData[1].data = (caddr_t) this;

   /*
    * NL_COMMENT
    * This message replaces message 186 in set 1
    */
  _buttonData[2].label = strdup(catgets(DT_catd, 1, 305, "Cancel"));
  _buttonData[2].callback = closeCallback;
  _buttonData[2].data = (caddr_t) this;

  _buttonData[3].label = strdup(catgets(DT_catd, 1, 187, "Help"));
  _buttonData[3].callback = HelpCB;
  _buttonData[3].data = (caddr_t) DTMAILFINDDIALOG;

  _headers = new SdmStringL(6);
  _dates = new SdmStringL(9);
  _states = new SdmStringL(2);
  _operators = new SdmStringL(5);


  //
  // Do NOT change the order of the items in _headers!  They must be in
  // sync with the actual fields in the FindDialog simple pane.
  //
  _headers->AddElementToList((char *)SdmSearch::Sdm_STK_To);
  _headers->AddElementToList((char *)SdmSearch::Sdm_STK_From);
  _headers->AddElementToList((char *)SdmSearch::Sdm_STK_Subject);
  _headers->AddElementToList((char *)SdmSearch::Sdm_STK_CC);
  _headers->AddElementToList((char *)SdmSearch::Sdm_STK_Text);
  _headers->AddElementToList((char *)SdmSearch::Sdm_STK_BCC);

  _dates->AddElementToList((char *)SdmSearch::Sdm_STK_Before);
  _dates->AddElementToList((char *)SdmSearch::Sdm_STK_Since);
  _dates->AddElementToList((char *)SdmSearch::Sdm_STK_On);
  _dates->AddElementToList((char *)SdmSearch::Sdm_STK_During);
  _dates->AddElementToList((char *)SdmSearch::Sdm_DDF_Today);
  _dates->AddElementToList((char *)SdmSearch::Sdm_DDF_Yesterday);
  _dates->AddElementToList((char *)SdmSearch::Sdm_DDF_LastWeek);
  _dates->AddElementToList((char *)SdmSearch::Sdm_DDF_ThisMonth);
  _dates->AddElementToList((char *)SdmSearch::Sdm_DDF_LastMonth);

  _states->AddElementToList((char *)SdmSearch::Sdm_STK_Read);
  _states->AddElementToList((char *)SdmSearch::Sdm_STK_Unread);

  _operators->AddElementToList("AND");
  _operators->AddElementToList("OR");
  _operators->AddElementToList("~");
  _operators->AddElementToList("(");
  _operators->AddElementToList(")");

  _text_labels[0] = strdup(catgets(DT_catd, 1, 188, "To:"));
  _text_labels[1] = strdup(catgets(DT_catd, 1, 189, "From:"));
  _text_labels[2] = strdup(catgets(DT_catd, 1, 190, "Subject:"));
  _text_labels[3] = strdup(catgets(DT_catd, 1, 191, "Cc:"));
  _text_labels[4] = strdup(catgets(DT_catd, 1, 306, "Entire Message:"));

  // These strings should not be translated.  They are
  // the Motif names for the widgets that will be created (they are
  // not the labels).
  _text_names[0] = strdup("To");
  _text_names[1] = strdup("From");
  _text_names[2] = strdup("Subject");
  _text_names[3] = strdup("Cc");
  _text_names[4] = strdup("Message_Contains");
}

//
//////////////////////////////////////////////////////////////////////
FindDialog::~FindDialog()
{
    clear();
    XmStringFree(_basic_str);
    XmStringFree(_advanced_str);
    if (_view_name)
	free(_view_name);
    delete _srch_crit;
    delete _headers;
    delete _dates;
    delete _states;
    delete _operators;
}

void
FindDialog::widgetDestroyed()
{
  _w = NULL;
  _search_type = NULL;
  _save_as_view_dialog = NULL;
  _simple_form = NULL;
  _script_form = NULL;
  _script_area = NULL;
  _status_text = NULL;
  for (int i=0; i<_num_text_fields; i++)
    _text_fields[i] = NULL;
}


//
// Print a string in the status line of the find dialog.
//
//////////////////////////////////////////////////////////////////////
void
FindDialog::setStatus(const char * str)
{
    char *tmpstr = strdup(str);
    XmString label = XmStringCreateLocalized(tmpstr);
 
    XtVaSetValues(_status_text,
                  XmNlabelString, label,
                  NULL);

    XmUpdateDisplay(baseWidget());
    XmStringFree(label);
    free(tmpstr);
}

//
// Clear the status line of the find dialog.
//
//////////////////////////////////////////////////////////////////////
void
FindDialog::clearStatus(void)
{
    setStatus(" ");
}

//
// Create the guts of the dialog
//
//////////////////////////////////////////////////////////////////////
Widget
FindDialog::createWorkArea(Widget dialog)
{
    register unsigned int offset;
    XmString str;
    char buffer[128], *dialog_name, *mbox_name;

    dialog_name = catgets(DT_catd, 1, 192, "Mailer - Search");
    mbox_name = _roamWindow->mailboxName();
    sprintf(buffer, "%s [%s]", dialog_name, mbox_name);
    _name = strdup(buffer);

    title(_name);

    Widget fd_pane = XtVaCreateWidget ("fd_pane",
				       xmPanedWindowWidgetClass,
				       dialog,
				       XmNsashWidth,	1,
				       XmNsashHeight,	1,
				       NULL);

    Widget top_form = XtVaCreateWidget ("top_form",
					xmFormWidgetClass,
					fd_pane,
					XmNfractionBase, 100,
					XmNallowResize, True,
					NULL);

    XmString strng = XmStringCreateLocalized(catgets(DT_catd, 1, 307,
						   "Search Type:"));
    Widget menu, option_menu, w;
    Arg args[10];
    int n=0;
    menu = XmCreatePulldownMenu(top_form, "option_menu", NULL, 0);
    XtSetArg(args[n], XmNsubMenuId, menu); n++;
    XtSetArg(args[n], XmNlabelString, strng); n++;
    XtSetArg(args[n], XmNmarginWidth, 25); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    option_menu = XmCreateOptionMenu(top_form, "option_menu", args, n);
    _search_type = XmOptionButtonGadget(option_menu);
    w = XtVaCreateManagedWidget("Basic", xmPushButtonGadgetClass,
				menu,
				XmNlabelString, _basic_str,
				NULL);
    XtAddCallback(w, XmNactivateCallback, FindDialog::optionMenuCallback,
		  (XtPointer) this);
    XtVaSetValues(w, XmNuserData, 0, NULL);
    w = XtVaCreateManagedWidget("Advanced", xmPushButtonGadgetClass,
				menu,
				XmNlabelString, _advanced_str,
				NULL);
    XtAddCallback(w, XmNactivateCallback, FindDialog::optionMenuCallback,
		  (XtPointer) this);
    XtVaSetValues(w, XmNuserData, 1, NULL);
    XmStringFree(strng);

    Widget fd_form = XtVaCreateWidget ("fd_form",
				       xmFormWidgetClass,
				       fd_pane,
				       XmNfractionBase,	100,
				       XmNallowResize, True,
				       NULL);

    _simple_form = XtVaCreateManagedWidget ("_simple_form",
				xmRowColumnWidgetClass,
				fd_form,
				XmNtopAttachment,	XmATTACH_FORM,
				XmNleftAttachment,	XmATTACH_POSITION,
				XmNrightAttachment,	XmATTACH_POSITION,
				XmNleftPosition,	5,
				XmNrightPosition,	95,
				XmNpacking,		XmPACK_COLUMN,
				XmNnumColumns,		2,
				XmNorientation,		XmVERTICAL,
				XmNisAligned,		True,
				XmNentryAlignment,	XmALIGNMENT_END,
				XmNentryVerticalAlignment, XmALIGNMENT_CENTER,
				NULL); 


    Widget fd_labels[7];
    int i = 0;
    for (i = 0; i < _num_text_fields; i++) {
	fd_labels [i] = XtVaCreateManagedWidget(_text_labels [i],
						xmLabelWidgetClass,
						_simple_form,
						NULL);
    }

    for (i = 0; i < _num_text_fields; i++) {
	_text_fields [i] = XtVaCreateManagedWidget (_text_names [i],
						    xmTextFieldWidgetClass,
						    _simple_form,
						    XmNuserData, i,
						    NULL);

	XtAddCallback(_text_fields [i], XmNactivateCallback,
		      (XtCallbackProc)textFieldCallback, (XtPointer)this);
    }


    _script_form = XtVaCreateWidget("script_form",
				    xmFormWidgetClass,
				    fd_form,
				    XmNfractionBase, 100,
				    XmNmappedWhenManaged, True,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
			            XmNallowResize, True,
				    NULL);


    XmString label, item1;
    Widget header_submenu, date_submenu, state_submenu, operator_submenu;
    Widget header_button, date_button, state_button, operator_button;
    Widget button;
    n = 0;

    // Create the Headers menu button for the advanced pane
    label = XmStringCreateLocalized(catgets(DT_catd, 1, 323, "Headers"));
    XtSetArg(args[n], XmNlabelString, label); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    header_button = DtCreateMenuButton(_script_form, "header_button", args, n);
    XtManageChild(header_button);
    XmStringFree(label);

    XtVaGetValues(header_button, XmNsubMenuId, &header_submenu, NULL);

    n=0;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_STK_To);
    XtSetArg(args[n], XmNuserData, this); n++;
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(header_submenu, "hb1", args, n);
    XtAddCallback(button, XmNactivateCallback, headerTokenCallback, (XtPointer)1);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_STK_From);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(header_submenu, "hb2", args, n);
    XtAddCallback(button, XmNactivateCallback, headerTokenCallback, (XtPointer)2);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_STK_Subject);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(header_submenu, "hb3", args, n);
    XtAddCallback(button, XmNactivateCallback, headerTokenCallback, (XtPointer)3);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_STK_CC);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(header_submenu, "hb4", args, n);
    XtAddCallback(button, XmNactivateCallback, headerTokenCallback, (XtPointer)4);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_STK_Text);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(header_submenu, "hb5", args, n);
    XtAddCallback(button, XmNactivateCallback, headerTokenCallback, (XtPointer)5);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_STK_BCC);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(header_submenu, "hb6", args, n);
    XtAddCallback(button, XmNactivateCallback, headerTokenCallback, (XtPointer)6);
    XtManageChild(button);
    XmStringFree(item1);

    // Setup the Date token menu button
    label = XmStringCreateLocalized(catgets(DT_catd, 1, 324, "Dates"));
    n = 0;
    XtSetArg(args[n], XmNlabelString, label); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, header_button); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    date_button = DtCreateMenuButton(_script_form, "date_button", args, n);
    XtManageChild(date_button);
    XmStringFree(label);

    XtVaGetValues(date_button, XmNsubMenuId, &date_submenu, NULL);

    n=0;
    XtSetArg(args[n], XmNuserData, this); n++;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_STK_Before);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(date_submenu, "db1", args, n);
    XtAddCallback(button, XmNactivateCallback, dateTokenCallback, (XtPointer)1);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_STK_Since);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(date_submenu, "db2", args, n);
    XtAddCallback(button, XmNactivateCallback, dateTokenCallback, (XtPointer)2);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_STK_On);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(date_submenu, "db3", args, n);
    XtAddCallback(button, XmNactivateCallback, dateTokenCallback, (XtPointer)3);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_STK_During);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(date_submenu, "db4", args, n);
    XtAddCallback(button, XmNactivateCallback, dateTokenCallback, (XtPointer)4);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_DDF_Today);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(date_submenu, "db5", args, n);
    XtAddCallback(button, XmNactivateCallback, dateTokenCallback, (XtPointer)5);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_DDF_Yesterday);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(date_submenu, "db6", args, n);
    XtAddCallback(button, XmNactivateCallback, dateTokenCallback, (XtPointer)6);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_DDF_LastWeek);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(date_submenu, "db7", args, n);
    XtAddCallback(button, XmNactivateCallback, dateTokenCallback, (XtPointer)7);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_DDF_ThisMonth);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(date_submenu, "db8", args, n);
    XtAddCallback(button, XmNactivateCallback, dateTokenCallback, (XtPointer)8);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_DDF_LastMonth);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(date_submenu, "db9", args, n);
    XtAddCallback(button, XmNactivateCallback, dateTokenCallback, (XtPointer)9);
    XtManageChild(button);
    XmStringFree(item1);

    // Setup the States token menu button
    label = XmStringCreateLocalized(catgets(DT_catd, 1, 325, "States"));
    n = 0;
    XtSetArg(args[n], XmNlabelString, label); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, date_button); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    state_button = DtCreateMenuButton(_script_form, "state_button", args, n);
    XtManageChild(state_button);
    XmStringFree(label);

    XtVaGetValues(state_button, DtNsubMenuId, &state_submenu, NULL);

    // Add the buttons to the menu button's submenu
    n=0;
    XtSetArg(args[n], XmNuserData, this); n++;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_STK_Read);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(state_submenu, "sb1", args, n);
    XtAddCallback(button, XmNactivateCallback, stateTokenCallback, (XtPointer)1);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized((char *)SdmSearch::Sdm_STK_Unread);
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(state_submenu, "sb2", args, n);
    XtAddCallback(button, XmNactivateCallback, stateTokenCallback, (XtPointer)2);
    XtManageChild(button);
    XmStringFree(item1);

    // Setup the Operators token menu button
    label = XmStringCreateLocalized(catgets(DT_catd, 1, 326, "Operators"));
    n = 0;
    XtSetArg(args[n], XmNlabelString, label); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, state_button); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    operator_button = DtCreateMenuButton(_script_form,"operator_button",
					 args, n);
    XtManageChild(operator_button);
    XmStringFree(label);

    XtVaGetValues(operator_button, DtNsubMenuId, &operator_submenu, NULL);

    n=0;
    XtSetArg(args[n], XmNuserData, this); n++;
    item1 = XmStringCreateLocalized("AND");
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(operator_submenu, "ob1", args, n);
    XtAddCallback(button, XmNactivateCallback, operatorTokenCallback, (XtPointer)1);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized("OR");
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(operator_submenu, "ob2", args, n);
    XtAddCallback(button, XmNactivateCallback, operatorTokenCallback, (XtPointer)2);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized("~  (not)");
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(operator_submenu, "ob3", args, n);
    XtAddCallback(button, XmNactivateCallback, operatorTokenCallback, (XtPointer)3);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized("(");
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(operator_submenu, "ob4", args, n);
    XtAddCallback(button, XmNactivateCallback, operatorTokenCallback, (XtPointer)4);
    XtManageChild(button);
    XmStringFree(item1);
    n=1;
    item1 = XmStringCreateLocalized(")");
    XtSetArg(args[n], XmNlabelString, item1); n++;
    button = XmCreatePushButtonGadget(operator_submenu, "ob5", args, n);
    XtAddCallback(button, XmNactivateCallback, operatorTokenCallback, (XtPointer)5);
    XtManageChild(button);
    XmStringFree(item1);


    _script_area = XtVaCreateManagedWidget("script_text",
					   xmTextWidgetClass,
					   _script_form,
					   XmNeditMode, XmMULTI_LINE_EDIT,
					   XmNrows, 6,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_FORM,
					   XmNleftOffset, 5,
					   XmNrightOffset, 5,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget, header_button,
					   XmNtopOffset, 5,
					   XmNwordWrap, True,
					   NULL);

    // Need to break this string up so that some of the strings can be
    // translated to other locales (like "Example" and "good morning").
    // The other items in this string ("subject", "AND", "before", etc..)
    // will not be translated...they are part of the c-client.
    //
    item1 = XmStringCreateLocalized(catgets(DT_catd, 1, 308,
					    "Example:"));
    Widget example = XtVaCreateManagedWidget("example", xmLabelGadgetClass,
			     _script_form,
			     XmNtopAttachment, XmATTACH_WIDGET,
			     XmNtopWidget, _script_area,
			     XmNtopOffset, 10,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNlabelString, item1,
			     NULL);
    XmStringFree(item1);
    // Only translate the good morning! (for now)
    sprintf(buffer, "subject %s and before yesterday", catgets(DT_catd, 1, 339, "\"good morning\""));
    item1 = XmStringCreateLocalized(buffer);
    Widget example2 = XtVaCreateManagedWidget("example", xmLabelGadgetClass,
			     _script_form,
			     XmNtopAttachment, XmATTACH_WIDGET,
			     XmNtopWidget, example,
			     XmNtopOffset, 5,
			     XmNleftAttachment, XmATTACH_POSITION,
			     XmNleftPosition, 10,
			     XmNlabelString, item1,
			     NULL);
    XmStringFree(item1);

    //
    // Now create the Action Area.
    //

    Widget fd_action = XtVaCreateWidget("actionArea",
				xmFormWidgetClass,
				fd_pane,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNfractionBase, 100,
				NULL);

    str = XmStringCreateLocalized(catgets(DT_catd, 1, 331, "Search Down"));
    Widget find_next = XtVaCreateManagedWidget("find_next",
				       xmPushButtonWidgetClass,
				       fd_action,
				       XmNtopAttachment, XmATTACH_FORM,
				       XmNlabelString, str,
				       XmNleftAttachment, XmATTACH_POSITION,
				       XmNleftPosition, 2,
				       XmNrightAttachment, XmATTACH_POSITION,
				       XmNrightPosition, 23,
				       NULL);
    XmStringFree(str);
    str = XmStringCreateLocalized(catgets(DT_catd, 1, 330, "Search Up"));
    Widget find_prev = XtVaCreateManagedWidget("find_prev",
				       xmPushButtonWidgetClass,
				       fd_action,
				       XmNtopAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_POSITION,
				       XmNrightPosition, 48,
				       XmNlabelString, str,
				       XmNleftAttachment, XmATTACH_POSITION,
				       XmNleftPosition, 27,
				       NULL);
    XmStringFree(str);
    str = XmStringCreateLocalized(catgets(DT_catd,1,309,"Save As View..."));
    Widget save_view = XtVaCreateManagedWidget("save_view",
			       xmPushButtonWidgetClass,
				   fd_action,
				   XmNtopAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_POSITION,
				   XmNrightPosition, 73,
				   XmNlabelString, str,
				 XmNleftAttachment, XmATTACH_POSITION,
				 XmNleftPosition, 52,
				   NULL);
    XmStringFree(str);
    str = XmStringCreateLocalized(catgets(DT_catd,1,302,"Show Views..."));
    Widget show_views = XtVaCreateManagedWidget("show_views",
				   xmPushButtonWidgetClass,
				   fd_action,
				   XmNtopAttachment, XmATTACH_FORM,
				   XmNleftAttachment, XmATTACH_POSITION,
				   XmNleftPosition, 77,
				   XmNrightAttachment, XmATTACH_POSITION,
				   XmNrightPosition, 98,
				   XmNlabelString, str,
				   NULL);
    XmStringFree(str);

    XtAddCallback(find_next, XmNactivateCallback,
		  (XtCallbackProc)findCallback, (XtPointer) this);
    XtAddCallback(find_prev, XmNactivateCallback,
		  (XtCallbackProc)findPrevCallback, (XtPointer) this);
    XtAddCallback(save_view, XmNactivateCallback,
		  (XtCallbackProc)saveAsViewCallback, (XtPointer) this);
    XtAddCallback(show_views, XmNactivateCallback,
		  (XtCallbackProc)showViewsCallback, (XtPointer) this);

    Widget widget;
    for (offset = 0; offset < _num_buttons; offset++) {
	widget = XtVaCreateManagedWidget(_buttonData[offset].label,
				 xmPushButtonWidgetClass,
				 fd_action,
				 XmNtopAttachment, XmATTACH_WIDGET,
				 XmNtopWidget, save_view,
				 XmNtopOffset, 10,
 				 XmNbottomOffset, 10,
				 XmNleftAttachment, XmATTACH_POSITION,
				 XmNleftPosition, (25 * offset) + 2,
				 XmNrightAttachment, XmATTACH_POSITION,
				 XmNrightPosition, (25 * (offset + 1)) - 2,
				 NULL);

	if (_buttonData[offset].callback != NULL) {
	    XtAddCallback(widget, XmNactivateCallback,
			  _buttonData[offset].callback,
			  _buttonData[offset].data);
	}

    }

    _status_text = XtVaCreateManagedWidget("StatusLabel",
					   xmLabelWidgetClass,
					   fd_pane,
                                           XmNrightAttachment, XmATTACH_FORM,
                                           XmNleftAttachment, XmATTACH_FORM,
                                           XmNalignment, XmALIGNMENT_BEGINNING,
                                           NULL);
	    
    XtWidgetGeometry size;

    size.request_mode = CWHeight;
    XtQueryGeometry(_status_text, NULL, &size);
    XtVaSetValues(_status_text,
		  XmNpaneMaximum, size.height,
		  XmNpaneMinimum, size.height,
		  NULL);
 
    clearStatus();

    XtManageChild (_simple_form);
    XtManageChild (option_menu);
    XtManageChild (top_form);
    XtManageChild (fd_form);
    XtManageChild (fd_action);
    XtManageChild (fd_pane);

    Dimension height;

    XtVaGetValues(fd_action, XmNheight, &height, NULL);
    XtVaSetValues(fd_action,
		  XmNpaneMaximum,	height,
		  XmNpaneMinimum,	height,
		  NULL);
    //
    // The dialog shell must allow resize so that the hide/show button
    // will work.
    XtVaSetValues(dialog, XmNallowShellResize, True, NULL);

    // Make sure get the height of the dialog after it has been
    // managed.
    XtVaGetValues(dialog, XmNheight, &height, NULL);
    XtVaSetValues(dialog, 
		  XmNmappedWhenManaged, True,
		  XmNminHeight, height,
		  NULL);

    return (fd_pane);
}


//
//////////////////////////////////////////////////////////////////////
void
FindDialog::optionMenuCallback(Widget w,
		      XtPointer client_data,
		      XtPointer)
{
    int item;
    FindDialog *fd = (FindDialog *)client_data;

    XtVaGetValues(w, XmNuserData, &item, NULL);
    if (item == 0) {		// basic
	if ( !fd->_script_is_hidden ) {
	    // Switching from the advanced pane to the basic pane.
	    XtUnmanageChild(fd->_script_form);
	    XtMapWidget(fd->_simple_form);
	    fd->_script_is_hidden = Sdm_True;
	}
    } else if (item == 1) {
	if ( fd->_script_is_hidden ) {
	    XtUnmapWidget(fd->_simple_form);
	    XtManageChild(fd->_script_form);
	    // Switching from basic to advanced.  If the advanced
	    // pane is empty, then preload the search defined on the
	    // basic pane.
// 	    char *buf=NULL;
// 	    buf = XmTextGetString(_script_area);
	    fd->_script_is_hidden = Sdm_False;
	}
    }
    fd->clearStatus();
}

//
//////////////////////////////////////////////////////////////////////
Boolean
FindDialog::findMatching (Boolean findAll)
{
    SdmError error;
    SdmMessageNumberL *matching_msgs;
    unsigned int matchCount = 0;
    int numberMessages = 0, current_index = 0;
    SdmMessageNumber currentMsg;

    /* NL_COMMENT
     * This string is displayed on the find dialog status line
     * when searching for a matching message.
     */
    setStatus(catgets(DT_catd, 1, 231, "Searching..."));
    theRoamApp.busyAllWindows(NULL);

    MsgScrollingList *msgList = _roamWindow->list();
    SdmMessageStore *mbox = _roamWindow->mailbox();
    SdmMessageNumber initialMsg = msgList->current_msg_handle();
    SdmMessageNumberL *msgHandles = msgList->get_message_numbers();
    current_index = msgList->get_selected_item() - 1;
    numberMessages = msgList->get_num_messages();
    SdmBoolean found = Sdm_False;


    if (numberMessages > 0) {
	matching_msgs = new SdmMessageNumberL(numberMessages);

	//
	// Search for messages matching the search criteria
	//
	if (mbox->PerformSearch(error, *matching_msgs, *_srch_crit)) {
      DtMailGenDialog *dialog = _roamWindow->genDialog();
      dialog->post_error(error);
	    matchCount = 0;
	} else {
	    matchCount = matching_msgs->ElementCount();
	    //
	    // Only report those messages found that are being displayed in
	    // the current view.  Walk through the matching message list
	    // backwards so that we can remove elements as we need to without
	    // screwing up the indices of elements we have yet to verify.
	    for (int i=matchCount-1; i>=0; i--) {
		for (int j=0; j<numberMessages; j++) {
		    if ((*matching_msgs)[i] == (*msgHandles)[j]) {
			found = Sdm_True;
			break;
		    }
		}
		if (!found) {
		    matching_msgs->RemoveElement(i);
		} else {
		    found = Sdm_False;
		}
	    }
	    matchCount = matching_msgs->ElementCount();
	}

	if (matchCount == 0) {
	    /* NL_COMMENT
	     * This string is displayed on the find dialog status line when
	     * no matching messages were found.
	     */
	    setStatus(catgets(DT_catd, 1, 234, "No matches were found"));
	    theRoamApp.unbusyAllWindows();
	    return(False);
	}

	//
	// If the user has selected "find all", then highlight all the
	// corresponding headers.
	if (findAll) {
	    msgList->select_all_and_display_last(error, matching_msgs,
						 matchCount);
	    if (error) {
		fprintf(stderr, "Error in select_all_and_display_last\n");
	    }
	    char line[80];
	    /* NL_COMMENT
	     * These strings are displayed on the find dialog status line
	     * when one or more matching messages are found.  The first
	     * string is displayed when there is one matching message,
	     * and the second string is displayed when there is more than
	     * one.  The %d is the number of messages that matched.
	     */
	    if (matchCount == 1) {
		strcpy(line, catgets(DT_catd, 1, 232, "1 message selected"));
	    } else {
		sprintf(line, catgets(DT_catd, 1, 233,"%d messages selected"), 
			matchCount);
	    }
	    setStatus(line);
	} else {
	    //
	    // Finds the next message in the scrolling list that matches
	    // the search criteria provided.
	    //
	    found = Sdm_False;
	    while (!found) {
		if (_searchForward) {
		    current_index++;
		    if (current_index >= numberMessages) {
			current_index = 0;
		    }
		} else {
		    current_index--;
		    if (current_index < 0) {
			current_index = numberMessages - 1;
		    }	
		}
//		currentMsg = msgHandles->at(current_index)->message_handle;
		currentMsg = (*msgHandles)[current_index];
		//
		// We have looped back to the initially selected message
		// without findind the matching message...looks like the
		// initial message was the message we were looking for.
		if (currentMsg == initialMsg) {
		    break;
		}
		for (int i=0; i<matchCount; i++) {
		    if ((*matching_msgs)[i] == currentMsg) {
			found = Sdm_True;
			break;
		    }
		}
	    }
// Should be calling a MsgScrollingList method to do this.
//	    XmListDeselectAllItems(msgList->baseWidget());
	    msgList->display_and_select_message(error, currentMsg);
	    if (error) {
		setStatus(catgets(DT_catd, 1, 312,
				  "Error displaying search results"));
	    }
	}
	delete matching_msgs;
    } else
	setStatus(catgets(DT_catd, 1, 234, "No matches were found"));
	

    theRoamApp.unbusyAllWindows();
    if (!error) {
	if (!findAll)
	    clearStatus();
	return(TRUE);
    } else
	return(False);
}

/////////////////////////////////////////////////////////////////////////
void
FindDialog::headerTokenCallback(Widget w,
				XtPointer client_data,
				XtPointer)
{
    FindDialog *fd;
    XmTextPosition pos;
    int i = (int) client_data;

    XtVaGetValues(w, XmNuserData, &fd, NULL);
    pos = XmTextGetInsertionPosition(fd->_script_area);
    XmTextInsert(fd->_script_area, pos,
		 (char*)((const char *)((*fd->_headers)[i-1] + " ")));
}

/////////////////////////////////////////////////////////////////////////
void
FindDialog::dateTokenCallback(Widget w,
			      XtPointer client_data,
			      XtPointer)
{
    FindDialog *fd;
    XmTextPosition pos;
    int i = (int) client_data;

    XtVaGetValues(w, XmNuserData, &fd, NULL);
    pos = XmTextGetInsertionPosition(fd->_script_area);
    XmTextInsert(fd->_script_area, pos,
		 (char *)((const char *)((*fd->_dates)[i-1] + " ")));
}

/////////////////////////////////////////////////////////////////////////
void
FindDialog::stateTokenCallback(Widget w,
			      XtPointer client_data,
			      XtPointer)
{
    FindDialog *fd;
    XmTextPosition pos;
    int i = (int) client_data;

    XtVaGetValues(w, XmNuserData, &fd, NULL);
    pos = XmTextGetInsertionPosition(fd->_script_area);
    XmTextInsert(fd->_script_area, pos,
		 (char *)((const char *)((*fd->_states)[i-1] + " ")));
}

/////////////////////////////////////////////////////////////////////////
void
FindDialog::operatorTokenCallback(Widget w, 
			      XtPointer client_data,
			      XtPointer)
{
    FindDialog *fd;
    XmTextPosition pos;
    int i = (int) client_data;


    XtVaGetValues(w, XmNuserData, &fd, NULL);
    pos = XmTextGetInsertionPosition(fd->_script_area);
    XmTextInsert(fd->_script_area, pos,
		 (char *)((const char *)((*fd->_operators)[i-1] + " ")));
}


//
// Pull all fields out of the dialog and store in the class.
//
/////////////////////////////////////////////////////////////////////////
SdmBoolean
FindDialog::getAllFields()
{
    SdmError err;
    register unsigned int i;
    int len;
    SdmString srch_str;
    SdmSearchLeaf *leaf;

    char temp[128];
    SdmBoolean first_field = Sdm_True;

    //
    // If the user is in the advanced scripting pane just get the contents
    // of the script area.
    // 
    if (!_script_is_hidden) {
	char *tmp_str, *str2;
	tmp_str = XmTextGetString(_script_area);
	if (tmp_str == NULL)
	    return Sdm_False;
	len = strlen(tmp_str);
	str2 = (char *)malloc(sizeof(char)*len + 1);

	// Need to make sure to copy the null terminator too
	for (i=0; i<=len; i++) {
	    //
	    // Need to strip out the newline characters so that we can
	    // correctly save the search string out to the user's .mailrc
	    // file.  Having newlines in the search string screws up parsing
	    // of the .mailrc file.
	    //
	    if (tmp_str[i] != '\n') {
		str2[i] = tmp_str[i];
	    } else {
		str2[i] = ' ';
	    }
	}
	srch_str = (const char *)str2;
	XtFree(tmp_str);
	free(str2);
    } else {
	for (i = 0; i < _num_text_fields; i++) {
	    if (_text_fields[i] != NULL) {
		// We are responsible for freeing strings returned by
		// XmTextFieldGetString().
		if (_text_values[i])
		    XtFree(_text_values[i]);
		_text_values[i] = XmTextFieldGetString(_text_fields[i]);
		if (_text_values[i] != NULL) {
		    len = strlen(_text_values[i]);
		    if (len == 0) {
			_text_values[i] = NULL;
			continue;
		    }
		    //
		    // We need to double quote strings that contain spaces
		    // because they will be considered invalid search strings
		    // otherwise.
		    if (strchr(_text_values[i], 32)) {
			char *tmp_str = (char *)malloc((sizeof(char)*(len+3)));
			sprintf(tmp_str, "\"%s\"", _text_values[i]);
			XtFree(_text_values[i]);
			_text_values[i] = tmp_str;
		    }
		    if (first_field == Sdm_True)
			sprintf(temp, "%s %s",
				(const char *)((*_headers)[i]),
				_text_values[i]);
		    else
			sprintf(temp, " AND %s %s",
				(const char *)((*_headers)[i]),
				_text_values[i]);
		    srch_str += (const char *) temp;

		    first_field = Sdm_False;
		}
	    }
	}
    }

    if (_srch_crit->SetSearchString(err, (const SdmString)srch_str)) {
	return Sdm_False;
    }
    if (_srch_crit->ParseSearch(err, leaf, Sdm_False)) {
// 	setStatus(catgets(DT_catd, 1, 313, "Invalid search string"));
	return Sdm_False;
    }
    return Sdm_True;
}

//
///////////////////////////////////////////////////////////////////////
void
FindDialog::textFieldCallback(Widget field,
			      XtPointer client_data,
			      XtPointer)
{
    char *s;
    FindDialog *fd = (FindDialog *) client_data;

    if (*(s = XmTextFieldGetString(field)) == '\0') {
	// Empty field.  Traverse
	(void) XmProcessTraversal(field, XmTRAVERSE_NEXT_TAB_GROUP);
    } else {
	// Field not empty. Do search
	if (fd->getAllFields()) {
	    if (!fd->findMatching(False)) {
		XBell(XtDisplay(field), 0);
	    }
	} else {
	    fd->setStatus(catgets(DT_catd, 1, 313, "Invalid search string"));
	}
	XtFree(s);
    }
}	

//
///////////////////////////////////////////////////////////////////////
void
FindDialog::loadSearchCriteria(char *name, SdmString *ss)
{
    SdmError err;
    SdmSearchLeaf *leaf;
    int i, len;
    SdmBoolean simple_search = Sdm_True;
    char *token, *tmp_str, header_val[64];
    unsigned char mask=0;

    clearFields();
    clearValues();

    _view_name = strdup(name);

    _srch_crit->SetSearchString(err, (const SdmString) *ss);
    if (err) {
      DtMailGenDialog *dialog = _roamWindow->genDialog();
      dialog->post_error(err);
      return;
    }
    _srch_crit->ParseSearch(err, leaf, Sdm_True);
    if (err) {
      DtMailGenDialog *dialog = _roamWindow->genDialog();
      dialog->post_error(err);
      return;
    }

    mask = 0;
    while(leaf) {
	if (leaf->sl_predicate == NULL) {
	    leaf = leaf->sl_next;
	} else {
	    // In order to edit a search string in the basic search pane,
	    // it must only use the "AND" logic operator.  Anything else
	    // must be edited in the advanced script pane.
	    if (((leaf->sl_logic_op != Sdm_BOP_And) &&
		(leaf->sl_logic_op != Sdm_BOP_None)) ||
		((leaf->sl_atomic_not != Sdm_NOB_NoBinding) ||
		 (leaf->sl_expression_not != Sdm_NOB_NoBinding))) {
			simple_search = Sdm_False;
			break;
	    }

	    token = strtok(leaf->sl_predicate, " ");
	    for (i=0; i<_num_text_fields; i++) {
		if (strcasecmp(token, (const char *)((*_headers)[i])) == 0) {
		    //
		    // If we already have a value for this header then we
		    // can't use the basic pane to edit it.  We keep track
		    // of which headers we have values for by setting a
		    // corresponding bit in mask. For example; a value (binary)
		    // of 1011 for mask means that the To, From, and CC fields
		    // all have values associated with them.  The right most
		    // bit represents the first header field in the _headers
		    // array.  We keep track of
		    // this because we can't display a search string in the
		    // basic pane that has multiple values for one header
		    // field.
		    if ((mask >> i) & 1) {
			simple_search = Sdm_False;
			break;
		    } else {
			tmp_str = strtok(NULL, " ");
			//
			// Remove the double quotes.  If the first char is
			// a double quote, remove the other double quote at
			// the end of the string.  Be careful of spaces.
			if (*tmp_str == '"') {
			    sprintf(header_val, "%s", tmp_str+1);
			    len = strlen(tmp_str);
			    // while the last character of this token is not
			    // a double quote add the token to the end of
			    // header_val.
			    while (*(tmp_str + len - 1) != '"') {
				tmp_str = strtok(NULL, " ");
				len = strlen(tmp_str);
				sprintf(header_val, "%s %s", header_val,
					tmp_str);
			    }
			    len = strlen(header_val);
			    header_val[len - 1] = '\0';
			    _text_values[i] = strdup(header_val);
			}
			//
			// The token matched one of the headers in the basic
			// pane.  Set the corresponding bit in the mask and
			// get the next token from the search string.
			mask = mask | (1 << i);
			break;
		    }
		}
	    }
	    if (!simple_search)
		break;

	    // If the token doesn't match any of the basic headers then
	    // we can't display it in the basic pane.
	    if (i >= _num_text_fields) {
		simple_search = Sdm_False;
		break;
	    }
		
	    leaf = leaf->sl_next;
	}
    }

    if (simple_search) {
	loadFields();
	// load the search criteria into the advanced search pane as well.
	XmTextSetString(_script_area, (char *)((const char *)*ss));
	if (!_script_is_hidden) {
	    XtUnmanageChild(_script_form);
	    XtMapWidget(_simple_form);
	    XtVaSetValues(_search_type, XmNlabelString, _basic_str, NULL);
	    _script_is_hidden = Sdm_True;
	}
    } else {
	XmTextSetString(_script_area, (char *)((const char *)*ss));
	if (_script_is_hidden) {
	    XtUnmapWidget(_simple_form);
	    XtManageChild(_script_form);
	    XtVaSetValues(_search_type, XmNlabelString, _advanced_str, NULL);
	    _script_is_hidden = Sdm_False;
	}
    }
}

//
///////////////////////////////////////////////////////////////////////
void
FindDialog::loadFields()
{
    int i;

    for (i=0; i<_num_text_fields; i++) {
	if (_text_values[i] != NULL) {
	    XmTextFieldSetString(_text_fields[i], _text_values[i]);
	}
    }
}

//
///////////////////////////////////////////////////////////////////////
void
FindDialog::clearValues()
{
    int i;

    for (i=0; i<_num_text_fields; i++) {
	// should the pointer be freed before we reset it to NULL?
	_text_values[i] = NULL;
    }
}

//
///////////////////////////////////////////////////////////////////////
void
FindDialog::saveAsViewCallback(Widget w,
			       XtPointer client_data,
			       XtPointer)
{
    FindDialog *fd = (FindDialog *)client_data;

    if (!fd->getAllFields()) {
	fd->setStatus(catgets(DT_catd, 1, 319,
		      "Unable to save as view, invalid search string"));
	return;
    }

    fd->clearStatus();

    if (!fd->_save_as_view_dialog) {
	Arg args[2];
	XmString str;
	char *title;
	int n=0;

	str = XmStringCreateLocalized(
	    catgets(DT_catd, 1, 314, "Enter View Name:"));
	title = catgets(DT_catd, 1, 315, "Mailer - Save As View");

	XtSetArg(args[n], XmNselectionLabelString, str); n++;
	XtSetArg(args[n], XmNtitle, title); n++;

	fd->_save_as_view_dialog = XmCreatePromptDialog(w, "saved_search",
							args, n);
	XtVaSetValues(fd->_save_as_view_dialog,
		      XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL,
		      NULL);

	XmStringFree(str);

	XtAddCallback(fd->_save_as_view_dialog, XmNokCallback,
		      &FindDialog::storeViewNameCallback, fd);
	XtAddCallback(fd->_save_as_view_dialog, XmNcancelCallback,
		      &FindDialog::storeViewNameCallback, fd);
	XtAddCallback(fd->_save_as_view_dialog, XmNhelpCallback,
		      HelpCB, DTMAILHELPSAVEASVIEW);
    }
    if (fd->_view_name) {
	XmString view_str = XmStringCreateLocalized(fd->_view_name);
	XtVaSetValues(fd->_save_as_view_dialog,
		     XmNtextString, view_str,
		     NULL);
	XmStringFree(view_str);
    }
    XtManageChild(fd->_save_as_view_dialog);
    XtPopup(XtParent(fd->_save_as_view_dialog), XtGrabNone);
}

//
///////////////////////////////////////////////////////////////////////
void
FindDialog::storeViewNameCallback(Widget w,
				  XtPointer client_data,
				  XtPointer call_data)
{
    FindDialog *fd = (FindDialog *)client_data;
    XmSelectionBoxCallbackStruct *cbs=(XmSelectionBoxCallbackStruct *)call_data;
    char *view_name;

    switch(cbs->reason) {
    case XmCR_OK:
	XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &view_name);
	//
	// Save the "search string" out to the mailrc file
	//
	fd->addView(view_name);
	break;
    case XmCR_CANCEL:
	XtPopdown(XtParent(w));
	break;
    default:
	break;
    }
}

///////////////////////////////////////////////////////////////////////
void
FindDialog::addView(char *name)
{
    SdmString str;
    SdmError error;


    _srch_crit->GetSearchString(error, str);
    if (error) {
      DtMailGenDialog *dialog = _roamWindow->genDialog();
      dialog->post_error(error);
      error.Reset();
    }

    if (!_viewsDialog) {
	_viewsDialog = _roamWindow->get_views_dialog(Sdm_False);
    }
    _viewsDialog->addView(name, (const SdmString) str);
}

//
//////////////////////////////////////////////////////////////////////
void
FindDialog::showViewsCallback(Widget,
			      XtPointer client_data,
			      XtPointer)
{
    FindDialog *fd = (FindDialog *)client_data;

    fd->busyCursor();
    fd->_viewsDialog = fd->parent()->get_views_dialog(Sdm_True);
    fd->normalCursor();
}

//
//////////////////////////////////////////////////////////////////////
void
FindDialog::findCallback(Widget,
			 XtPointer client_data,
			 XtPointer)
{
    FindDialog	*findData = (FindDialog *) client_data;

    if (findData->getAllFields()) {
	findData->_searchForward = TRUE;
	findData->findMatching(False);
    } else {
	findData->setStatus(catgets(DT_catd, 1, 313, "Invalid search string"));
    }
}

//
//////////////////////////////////////////////////////////////////////
void
FindDialog::findPrevCallback(Widget,
			     XtPointer client_data,
			     XtPointer)
{
    FindDialog	*findData = (FindDialog *) client_data;

    if (findData->getAllFields()) {
	findData->_searchForward = FALSE;
	findData->findMatching(False);
    } else {
	findData->setStatus(catgets(DT_catd, 1, 313, "Invalid search string"));
    }
}

//
//////////////////////////////////////////////////////////////////////
void
FindDialog::findSelectAllCallback(Widget,
				  XtPointer client_data,
				  XtPointer)
{
    FindDialog	*findData = (FindDialog *) client_data;
    
    if (findData->getAllFields()) {
	findData->findMatching(TRUE);
    } else {
	findData->setStatus(catgets(DT_catd, 1, 313, "Invalid search string"));
    }
    return;
}

//
//////////////////////////////////////////////////////////////////////
void
FindDialog::clearCallback(Widget,
			  XtPointer client_data,
			  XtPointer)
{
    FindDialog	*fd = (FindDialog *) client_data;

    fd->clearFields();
    fd->clearStatus();
    
    if (fd->_save_as_view_dialog) {
	XmString str = XmStringCreateLocalized("");
	XtVaSetValues(fd->_save_as_view_dialog,
		      XmNtextString, str,
		      NULL);
	XmStringFree(str);
    }
}

//
//////////////////////////////////////////////////////////////////////
void FindDialog::clearFields()
{
    int i;

    for (i=0; i<_num_text_fields; i++) {
	_text_values[i] = NULL;
	if (_text_fields[i] != NULL) {
	    XmTextFieldSetString(_text_fields[i], "");
	}
    }
    XmTextSetString(_script_area, "");
}

//
//////////////////////////////////////////////////////////////////////
void
FindDialog::closeCallback(Widget,
			  XtPointer client_data,
			  XtPointer)
{
    FindDialog	*fd = (FindDialog *) client_data;
  
    fd->popdown();
    fd->clearStatus();
    return;
}
