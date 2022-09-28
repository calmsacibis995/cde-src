/*
 *+SNOTICE
 *
 *	$Revision: 1.48 $
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
#pragma ident "@(#)PasswordDialogManager.C	1.48 06/09/97"
#endif

#include <stdio.h>
#include "RoamApp.h"
#include "PasswordDialogManager.h"
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Form.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/Session.hh>
#include "MailMsg.h"
#include "RoamMenuWindow.h"
#include "Help.hh"

static Widget getTopLevelShell(Widget);

PasswordDialogManager *thePasswordDialogManager = 
    new PasswordDialogManager ( "PasswordDialog" );


PasswordDialogManager::PasswordDialogManager ( char   *name ) : 
                                 PromptDialogManager ( name )
{
    // stored values
    _pwd = NULL;
    _username = NULL;
    _servername = NULL;
    _error = NULL;

    _high_bandwidth = Sdm_True;

    // Widgets
    _imap_server = NULL;
    _user = NULL;
    _password = NULL;
    _dialog = NULL;
}

Widget PasswordDialogManager::createDialog ( Widget parent )
{
   
  Arg args[1];
  XtSetArg(args[0], XmNautoUnmanage, False);
  Widget dialog = XmCreatePromptDialog ( parent, _name, args, 1);
  static XtActionsRec myActions[] = { {"traverse", &PasswordDialogManager::traverse}};

  static char newTrans[] = "~s ~m ~a <Key>Return : traverse()";
  XtTranslations transTable;


  XtVaSetValues ( dialog,
		  XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
		  XmNnoResize, True,
		  NULL );

  XtUnmanageChild( XmSelectionBoxGetChild( dialog,
				      XmDIALOG_TEXT ) );

  XtUnmanageChild( XmSelectionBoxGetChild( dialog,
				      XmDIALOG_SELECTION_LABEL ) );

  Widget rc = XtVaCreateManagedWidget("PasswordArea",
					xmRowColumnWidgetClass, dialog,
					XmNnumColumns, 1,
					XmNorientation, XmVERTICAL,
					XmNisAligned, True,
					XmNentryAlignment, XmALIGNMENT_END,
					XmNalignment, XmALIGNMENT_END,
					NULL);

  // Server label and text field.
  Widget server_form = XtVaCreateManagedWidget("Server",
					xmFormWidgetClass, rc,
					NULL);

  _imap_server = XtVaCreateManagedWidget("Server",
					xmTextFieldWidgetClass, server_form, 
					XmNtopAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					NULL);
  Widget server_label = XtVaCreateManagedWidget (catgets(DT_catd, 16, 3, "IMAP Server:"),
					xmLabelGadgetClass, server_form,
					XmNtopAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_WIDGET,
					XmNrightWidget, _imap_server,
					NULL);


  // Login label and text field.
  Widget login_form = XtVaCreateManagedWidget("Login",
					xmFormWidgetClass, rc,
					NULL);

  _user = XtVaCreateManagedWidget("User", 
				    xmTextFieldWidgetClass, login_form, 
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    NULL);

  Widget user_label = XtVaCreateManagedWidget(catgets(DT_catd, 1, 334, "Login:"),
					xmLabelGadgetClass, login_form,
					XmNtopAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_WIDGET,
					XmNrightWidget, _user,
					NULL);

  // Password label and text field.
  Widget passwd_form = XtVaCreateManagedWidget("Login",
					xmFormWidgetClass, rc,
					NULL);

  _password = XtVaCreateManagedWidget("Password", 
					xmTextFieldWidgetClass, passwd_form,
					XmNtopAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					NULL, 0);

  Widget password_label = XtVaCreateManagedWidget(catgets(DT_catd, 1, 335, "Password:"), 
					xmLabelGadgetClass, passwd_form, 
					XmNtopAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_WIDGET,
					XmNrightWidget, _password,
					NULL, 0);

  _dialog = dialog;

  XtAppAddActions(theRoamApp.appContext(), myActions, 1);
  transTable = XtParseTranslationTable(newTrans);
  XtOverrideTranslations(_imap_server, transTable);
  XtOverrideTranslations(_user, transTable);
  XtRemoveAllCallbacks(_imap_server, XmNactivateCallback);

  // Radio buttons for selecting high/low bandwidth.
  Widget connection_form = XtVaCreateManagedWidget("Connection",
					xmFormWidgetClass, rc,
					NULL);

  XmString strFast = XmStringCreateLocalized(catgets(DT_catd, 1, 336, "Always"));
  XmString strSlow = XmStringCreateLocalized(catgets(DT_catd, 1, 337, "When Open"));
  XmString strConnection =  XmStringCreateLocalized(catgets(DT_catd, 1, 341, "Retrieve attachments:"));
  // What's the initial state for network connection
  SdmMailRc *mailrc;
  SdmError error;
  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailrc);
  int initial = 0;
  if(mailrc->IsValueDefined("slownet")) {
    initial = 1;
    setHigh(Sdm_False);
  }
  _choose_bandwidth = XmVaCreateSimpleRadioBox(connection_form, "Retrieve attachments",
					initial,      // Initial selection
					&PasswordDialogManager::bandwidthCallback,
					XmNorientation, XmHORIZONTAL,
					XmNalignment, XmALIGNMENT_END,
					XmNpacking, XmPACK_TIGHT,
					XmVaRADIOBUTTON, strFast, NULL, NULL, NULL,
					XmVaRADIOBUTTON, strSlow, NULL, NULL, NULL,
					XmNuserData, this,
					XmNsensitive, True,
					XmNtopAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNbottomAttachment, XmATTACH_FORM,
					NULL);

  Widget network_connection = XtVaCreateManagedWidget("Retrieve attachments:",
					xmLabelGadgetClass, connection_form,
					XmNlabelString, strConnection,
					XmNtopAttachment, XmATTACH_FORM,
					XmNbottomAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_WIDGET,
					XmNrightWidget, _choose_bandwidth,
					NULL);
  XmStringFree(strFast);
  XmStringFree(strSlow);
  XmStringFree(strConnection);
  XtManageChild(network_connection);
  XtManageChild(_choose_bandwidth);

  XtManageChild(rc);
  // Set up callbacks for the text fields.
  XtAddCallback (_imap_server,
		XmNactivateCallback, (XtCallbackProc) XmProcessTraversal,
		(XtPointer) XmTRAVERSE_NEXT_TAB_GROUP);

  XtAddCallback (_user,
		XmNactivateCallback, (XtCallbackProc) XmProcessTraversal,
		(XtPointer) XmTRAVERSE_NEXT_TAB_GROUP);

  XtAddCallback ( _password, XmNmodifyVerifyCallback,
	  	(XtCallbackProc )&PasswordDialogManager::modifyVerifyCallback,
	  	(XtPointer) this );

  Widget w;
  w = XmSelectionBoxGetChild(dialog, XmDIALOG_OK_BUTTON);
  XtVaSetValues(w, XmNlabelString, XmStringCreateLocalized(catgets(DT_catd, 1, 338, "Login")), NULL);
  w = XmSelectionBoxGetChild(dialog, XmDIALOG_APPLY_BUTTON);
#ifdef SPUTNIK_ADVANCED
  XtVaSetValues(w, XmNlabelString, XmStringCreateLocalized("Profile..."), NULL);
#else
  XtUnmanageChild(w);
#endif // SPUTNIK_ADVANCED

  return dialog;
}

char *
PasswordDialogManager::User()
{
    char *text=NULL;

    if (_user)
    	XtVaGetValues(_user, XmNvalue, &text, NULL);
    return(text);
}

char *
PasswordDialogManager::ImapServer()
{
    char *text=NULL;

    if (_imap_server)
    	XtVaGetValues(_imap_server, XmNvalue, &text, NULL);
    return(text);
}

void
PasswordDialogManager::modifyVerifyCallback(Widget,
		      XtPointer clientData,
		      XmTextVerifyCallbackStruct *cbs ) 
{
  PasswordDialogManager *pdm=( PasswordDialogManager * ) clientData;
  char *passwd = pdm->password();
  char *newpasswd;

  if (cbs->reason == XmCR_ACTIVATE) {
    printf("Password: %s\n", (pdm->password() != NULL) ? pdm->password() : "");
    XtPopdown(XtParent(pdm->baseWidget()));
    return;
  }

  // Handle backspace.
  if ( cbs->text->ptr == NULL ) {
    if (passwd) {
	cbs->endPos = strlen(passwd); // delete from here to end
	passwd[cbs->startPos] = 0;
    } else {
	cbs->endPos = 0;
    }
    return;
  }

  // Don't allow "paste" operations, make the user *type* the password!
  if (cbs->text->length > 1) {
    cbs->doit = False;
    return;
  }
  

  if (passwd) {
    newpasswd = XtMalloc(strlen(passwd) + 2); // new character + NULL terminator.
    memset(newpasswd, 0, strlen(passwd) + 2);
    
    if (cbs->startPos > 0) {
      strncpy(newpasswd, passwd, cbs->startPos);
    } 
    strncat(newpasswd, cbs->text->ptr, cbs->text->length);
    if (cbs->endPos < strlen(passwd)) {
      strcat(newpasswd, passwd+cbs->endPos);
    }
  } else {
    newpasswd = XtMalloc(2);                 // new character + NULL terminator.
    memset(newpasswd, 0, 2);
    strncat(newpasswd, cbs->text->ptr, cbs->text->length);
  }

  pdm->setPassword(newpasswd);

  if (cbs->text->length == 1)
    cbs->text->ptr[0] = '*';
}

void
PasswordDialogManager::OkCallback(Widget, XtPointer, XtPointer)
{

    thePasswordDialogManager->setImapServer(thePasswordDialogManager->ImapServer());
    thePasswordDialogManager->getDelimiter(thePasswordDialogManager->imapserver());
}
const char*
PasswordDialogManager::getDelimiter(const char *server)
{
    const char *delim=NULL;

    // Already in the cache?...
    if (delim = delimiter(server))
        return delim;
 
    SdmMessageStore *mstore;
    static SdmError err;
    SdmSession *i_session = theRoamApp.isession()->isession();
    SdmToken token;

    token.SetValue("servicechannel", "cclient");
    token.SetValue("servicetype", "imap");
    token.SetValue("serviceclass", "messagestore");
    token.SetValue("serviceoption", "hostname", server);
    token.SetValue("serviceoption", "username", User());
    token.SetValue("serviceoption", "password", password());
 
    i_session->SdmMessageStoreFactory(err, mstore);
    if (!err) {
    	mstore->StartUp(err);
    	if (err != Sdm_EC_Success) 
		_error = err;
	else if (mstore->Attach(err, token) == Sdm_EC_Success) {
        	err = setDelimiter(server, mstore);
   		if (!err)
			delim = delimiter(server);
        	mstore->Close(err);
	}
	if (err) 
		_error = err;
	delete mstore;
   }
   return delim;
}

void
PasswordDialogManager::anotherOkcb(Widget w, XtPointer clientData, XtPointer)
{
    DialogCallbackData *dcb = (DialogCallbackData *)clientData;

    Widget topShell = getTopLevelShell(w);

    Atom WM_DELETE_WINDOW=XmInternAtom( XtDisplay( w ),
					"WM_DELETE_WINDOW",
					False );

    XmRemoveWMProtocolCallback(topShell,
			WM_DELETE_WINDOW,
			( XtCallbackProc ) &PasswordDialogManager::cancelCallback,
			(XtPointer) dcb );

    thePasswordDialogManager->setUser(thePasswordDialogManager->User());

    // Set the "slow net" mailrc variable if it's toggled
    SdmMailRc *mailRc;
    SdmError error;
    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
    if (mailRc->IsValueDefined("slownet")) {
      if (thePasswordDialogManager->IsHighbandwidth() == Sdm_True) {
        mailRc->RemoveValue(error, "slownet");
        mailRc->Update(error);
      }  
    } else {
      if (thePasswordDialogManager->IsHighbandwidth() == Sdm_False) {
        mailRc->SetValue(error, "slownet", "");
        mailRc->Update(error);
      }  
    }  

    XtUnmanageChild(thePasswordDialogManager->baseWidget());
}
void
PasswordDialogManager::okcb(Widget dialog, XtPointer clientData, XtPointer)
{
  DialogCallbackData *dcd = (DialogCallbackData *) clientData;
  PasswordDialogManager *pdm = (PasswordDialogManager *) dcd->dialogManager();
  struct OpenStruct *open_struct = (struct OpenStruct *) dcd->clientData();
  RoamMenuWindow *rmw = (RoamMenuWindow *) open_struct->rmw;
  SdmToken *token = rmw->token();
  SdmError error;
  
  // Set the "slow net" mailrc variable if it's toggled
  SdmMailRc *mailRc;
  theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
  if (mailRc->IsValueDefined("slownet")) {
    if (pdm->IsHighbandwidth() == Sdm_True) {
      mailRc->RemoveValue(error, "slownet");
      mailRc->Update(error);
    }
  } else {
    if (pdm->IsHighbandwidth() == Sdm_False) {
      mailRc->SetValue(error, "slownet", "");
      mailRc->Update(error);
    }
  }

  // Set the hostname of the IMAP server.
  token->ClearValue("serviceoption", "hostname");
  token->SetValue("serviceoption", "hostname", pdm->ImapServer());
  token->ClearValue("serviceoption", "username");
  token->SetValue("serviceoption", "username", pdm->User());
  token->ClearValue("serviceoption", "password");
  token->SetValue("serviceoption", "password", pdm->password());


  rmw->busyCursor();
  rmw->open_internal(error);
  rmw->normalCursor();
  // If open failed, we should bring up the dialog again.
  if (error)
    XtManageChild(dialog);
  else {
    // If open succeeds, set user.
    pdm->setUser(pdm->User());
    pdm->setImapServer(pdm->ImapServer());
    theRoamApp.firstLogin(Sdm_False);
  }
  return;
}

void
PasswordDialogManager::bandwidthCallback(Widget w,
				XtPointer clientData,
				XtPointer)
{
    int which = (int) clientData;	// clientData contains button number.
    PasswordDialogManager *pwdm;

    // Client data is actually on the parent, not the toggle item.
    XtVaGetValues(XtParent(w), XmNuserData, &pwdm, NULL);

    if (which == 0) {
	pwdm->setHigh(Sdm_True);
    } else {
	pwdm->setHigh(Sdm_False);
    }
    return;
}

void
PasswordDialogManager::traverse(Widget w, XEvent *, String *, Cardinal *)
{
  XmProcessTraversal(w, XmTRAVERSE_NEXT_TAB_GROUP);
}

Widget
PasswordDialogManager::post(char *title,
                            char *text,
                            void *clientData,
                            DialogCallback ok,
                            DialogCallback apply,
                            DialogCallback cancel,
                            DialogCallback help)
{
  // Get a dialog widget from the cache
  Widget dialog = getDialog();
 
  // Make sure the dialog exists, and that it is an XmSelectionBox
  // or subclass, since the callbacks assume this widget type.
  // The fact that the dialog is an XmSelectionBox instead of an
  // XmMessageBox is the only reason we have to redefine this
  // virtual function.
 
  assert ( dialog );
  assert ( XtIsSubclass ( dialog, xmSelectionBoxWidgetClass ) );
 
  // Convert the text string to a compound string and
  // specify this to be the message displayed in the dialog.
 
  XmString titleStr = XmStringCreateLocalized (title);
  XmString xmstr = XmStringCreateLocalized ( text );
  XtVaSetValues ( dialog,
                  XmNmessageString, xmstr,
                  XmNdialogTitle, titleStr,
                  NULL );
  XmStringFree ( xmstr );
  XmStringFree ( titleStr );
 
  // Create an object to carry the additional data needed
  // to cache the dialogs.
 
  DialogCallbackData *dcb = new DialogCallbackData( this,
                                                    clientData,
                                                    ok, cancel,
                                                    help );
  // Install callback function for each button
  // support by Motif dialogs. If there is no help callback
  // unmanage the corresponding button instead, if possible.
  // Also call the default okCb for setting up passwd and login
 
  Atom WM_DELETE_WINDOW=XmInternAtom( XtDisplay( dialog ),
		"WM_DELETE_WINDOW",
		False );

  XmAddWMProtocolCallback( XtParent(dialog),
		WM_DELETE_WINDOW,
		( XtCallbackProc ) &PasswordDialogManager::cancelCallback,
		(XtPointer) dcb );

  if ( ok ) {
    XtRemoveAllCallbacks(dialog, XmNokCallback);
    XtAddCallback ( dialog,
                    XmNokCallback,
                    &PasswordDialogManager::anotherOkcb,
                    (XtPointer) dcb );
    XtAddCallback ( dialog,
                    XmNokCallback,
                    &PasswordDialogManager::OkCallback,
                    (XtPointer) dcb );
    XtAddCallback ( dialog,
                    XmNokCallback,
                    &DialogManager::okCallback,
                    (XtPointer) dcb );
  }
  else {
    Widget w = XmSelectionBoxGetChild(dialog, XmDIALOG_OK_BUTTON);
    XtUnmanageChild(w);
  }

  if (apply) {
    XtRemoveAllCallbacks(dialog, XmNapplyCallback);
    XtAddCallback(dialog,
                  XmNapplyCallback,
                  &DialogManager::applyCallback,
                  (XtPointer) dcb);
    Widget w = XmSelectionBoxGetChild(dialog, XmDIALOG_APPLY_BUTTON);
    XtManageChild(w);
  } else {
    Widget w = XmSelectionBoxGetChild(dialog, XmDIALOG_APPLY_BUTTON);
    XtUnmanageChild(w);
  }
 
  if ( cancel ) {
    XtRemoveAllCallbacks(dialog, XmNcancelCallback);
    XtAddCallback ( dialog,
                    XmNcancelCallback,
                    //&DialogManager::cancelCallback,
                    &PasswordDialogManager::cancelCallback,
                    (XtPointer) dcb );
  } else {
    Widget w = XmSelectionBoxGetChild (dialog, XmDIALOG_CANCEL_BUTTON);
    XtUnmanageChild (w);
  }
 
  if (help) {
    XtRemoveAllCallbacks(dialog, XmNhelpCallback);
    XtAddCallback ( dialog,
                    XmNhelpCallback,
                    &HelpErrorCB,
                    (XtPointer) DTMAILHELPLOGIN);
  } else {
    Widget w = XmSelectionBoxGetChild ( dialog,
                                        XmDIALOG_HELP_BUTTON );
    XtUnmanageChild ( w );
  }

  if (_servername)
    XmTextFieldSetString(_imap_server, _servername);
  if (_username) {
    XmTextFieldSetString(_user, _username);
  } else {
    // Set the default user name.
    passwd pw;
    SdmSystemUtility::GetPasswordEntry(pw);
    if (strcmp(XmTextFieldGetString(_user), "") == 0) {
      XmTextFieldInsert(_user, 0, pw.pw_name);
      setUser(strdup(pw.pw_name));
    }
  }

  // Clear the password.
  XmTextFieldSetString(_password, "");
 
  // Post the dialog.
  XtManageChild ( dialog );
 
  return dialog;
}

Widget
PasswordDialogManager::post(char *title,
			    char *text,
			    Widget wid,
			    void *clientData,
			    DialogCallback ok,
			    DialogCallback /*apply*/,
			    DialogCallback cancel,
			    DialogCallback help)
{
    OpenStruct *openstr = (OpenStruct *) clientData;
    RoamMenuWindow *rmw = (RoamMenuWindow *) openstr->rmw;
    rmw->passwd_dialog_up(Sdm_True);
    // Get a dialog widget from the cache
    Widget dialog = getDialog(wid);

    // Store the openstruct info in the dialog. We get it later if the
    // user cancels out of the login dialog via the window manager close
    // button. That way we have the rmw and we can bring down the window.
    _os = openstr;

    // Make sure the dialog exists, and that it is an XmMessageBox
    // or subclass, since the callbacks assume this widget type

    assert (dialog);
    assert (XtIsSubclass (dialog, xmSelectionBoxWidgetClass));

    // Convert the text string to a compound string and
    // specify this to be the message displayed in the dialog.
    XmString titleStr = XmStringCreateLocalized(title);
    XmString xmstr = XmStringCreateLocalized(text);
    XtVaSetValues (dialog,
		    XmNmessageString, xmstr,
		    XmNdialogTitle, titleStr,
		    NULL);
    XmStringFree(xmstr);
    XmStringFree(titleStr);

    // Create an object to carry the additional data needed
    // to cache the dialogs.

    DialogCallbackData *dcb = new DialogCallbackData(this,
						    clientData,
						    ok, cancel,
						    help);

    SdmError error;
    SdmMailRc *mailRc;
    char *server_name = NULL;
    theRoamApp.connection()->connection()->SdmMailRcFactory(error, mailRc);
    if ((server_name = imapserver()) == NULL) {
	if (rmw->inbox()) {
	    mailRc->GetValue(error, "imapinboxserver", &server_name);
	} else { // It's not the inbox.
	    mailRc->GetValue(error, "imapfolderserver", &server_name);
	}
	if (!error && server_name) {
	    if (strcmp(XmTextFieldGetString(_imap_server), "") == 0) {
		XmTextFieldInsert(_imap_server, 0, server_name);
	    }
	    free(server_name);
	}
    } else {
	// Clear the text field and insert the server name.
	// We don't want to simply do a set string of the
	// server name, because that will put the insertion
	// point at the beginning instead of at the end.

	XmTextFieldSetString(_imap_server, "");
	XmTextFieldInsert(_imap_server, 0, server_name);
    }


    // Set the default user name.
    passwd pw;
    SdmSystemUtility::GetPasswordEntry(pw);
    if (strcmp(XmTextFieldGetString(_user), "") == 0) {
	XmTextFieldInsert(_user, 0, pw.pw_name);
        setUser(strdup(pw.pw_name));
    }

    // Install callback function for each button support
    // by Motif dialogs.  If there is no help callback,
    // unmanage the corresponding button instead, if possible.
    // note that we remove any existing callbacks that could
    // exist for the widget.  the dialog could be reused and
    // we don't want to keep the old callback around because 
    // the old client data could be obsolete.

    if (ok) {
      XtRemoveAllCallbacks(dialog, XmNokCallback);
    	XtAddCallback ( dialog,
		XmNokCallback,
		&PasswordDialogManager::anotherOkcb,
		(XtPointer) dcb );
    	XtAddCallback(dialog,
        	XmNokCallback,
        	&PasswordDialogManager::OkCallback,
		(XtPointer) dcb);
    	XtAddCallback(dialog,
        	XmNokCallback,
        	&PasswordDialogManager::okcb,
		(XtPointer) dcb);
    } else {
    	Widget w = XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON);
      XtUnmanageChild(w);
    }

    if (cancel) {
      XtRemoveAllCallbacks(dialog, XmNcancelCallback);
      XtAddCallback(dialog,
          XmNcancelCallback,
          &DialogManager::cancelCallback,
          (XtPointer) dcb);
    }
    else {
      Widget w = XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON);
      XtUnmanageChild(w);
    }

    if (help) {
      XtRemoveAllCallbacks(dialog, XmNhelpCallback);
    	XtAddCallback(dialog,
          XmNhelpCallback,
	  &HelpErrorCB,
          (XtPointer) DTMAILHELPLOGIN);
    } else {
      Widget w = XmMessageBoxGetChild (dialog, XmDIALOG_HELP_BUTTON);
    	XtUnmanageChild(w);
    }
    // Post the dialog.
    XtManageChild(dialog);
    XmProcessTraversal(_password, XmTRAVERSE_CURRENT);

    return dialog;
}

Widget 
PasswordDialogManager::post (
   char *title,
   char *text, 
   Widget w,
   void *clientData,
   DialogCallback ok,
   DialogCallback cancel,
   DialogCallback help)
{
  return (this->post(title, text, w, clientData, ok, NULL, cancel, help));
}

Widget 
PasswordDialogManager::post (
   char *title,
   char *text, 
   void *clientData,
   DialogCallback ok,
   DialogCallback cancel,
   DialogCallback help)
{
  return (this->post(title, text, clientData, ok, NULL, cancel, help));
}

void
passwordDialogOKCallback( int *data )
{
    *data=1;
}

void
passwordDialogCancelCallback( int *data )
{
    *data=2;
}

int
PasswordDialogManager::post_and_return(
		char *title, 
		char *text, 
		DialogCallback ok,
		DialogCallback apply, 
		DialogCallback cancel, 
		DialogCallback help) 
{
    int answer = 0;

    Widget dialog = post(title, text, &answer, ok, apply, cancel, help);

    forceUpdate( dialog );
    while ( answer==0 )
        XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMXEvent );

    // Process just one more event to pop down dialog.
    XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMXEvent );

    return(answer);
}

int
PasswordDialogManager::post_and_return(
	char *title_str, 
	char *text_str,
	Widget wid
)
{
  // just call parent method.
  return PromptDialogManager::post_and_return(title_str, text_str, wid);
}

int
PasswordDialogManager::post_and_return(
	char *title_str, 
	char *text_str,
	char *okLabelString,
	char *cancelLabelString,
        Widget wid

)
{
  // just call parent.
  return PromptDialogManager::post_and_return(title_str, text_str,
          okLabelString, cancelLabelString, wid);
}

int
PasswordDialogManager::post_and_return(
	char *title_str, 
	char *text_str,
	char *okLabelString,
        Widget wid
)
{
  // just call parent.
  return PromptDialogManager::post_and_return(title_str, text_str,
          okLabelString, wid);
}
SdmBoolean
PasswordDialogManager::postDialog() 
{
    if (post_and_return(
		catgets(DT_catd, 16, 1, "Mailer - Login"),
		catgets(DT_catd, 16, 1, "Mailer - Login"),
		(DialogCallback)&passwordDialogOKCallback,
		(DialogCallback)NULL,
		(DialogCallback)&passwordDialogCancelCallback,
		(DialogCallback)&PasswordCmd::help) != 1) 

		// Canceled
		return Sdm_False;
    return Sdm_True;
}

SdmBoolean
PasswordDialogManager::loginDialog() 
{
    SdmBoolean posted = Sdm_False;
    while (_username == NULL || _pwd == NULL) {
	posted = Sdm_True;
	if (post_and_return(
		catgets(DT_catd, 16, 1, "Mailer - Login"),
		catgets(DT_catd, 16, 1, "Mailer - Login"),
		(DialogCallback)&passwordDialogOKCallback,
		(DialogCallback)NULL,
		(DialogCallback)&passwordDialogCancelCallback,
		(DialogCallback)&PasswordCmd::help) != 1) 

		// Canceled
		return Sdm_False;
    }    
    // This gets the delimiter for the server and stores it in a cache. 
    // This function normally gets called from the callback func on the
    // login button of the login dialog, but in case where the dialog was
    // up in previous session, the callback may never get called and the
    // delimiter may never get stored, so we do it here to be sure. If the
    // delimiter is already set, then this is basically a no-op, so low
    // performance hit.
    if (_servername != NULL && !posted)
    	getDelimiter(_servername);
    return Sdm_True;
}

// This returns the folder delimiter for a supplied imap server. 
// Used for determing the default imap folder for the FSB and move/copy 
// menus for imap mailboxes
const char *
PasswordDialogManager::delimiter(const char *server)
{
       	int i;
	static const char* delim;
	SdmString host;
	
	for (i = 0; i < _serverDelim.ElementCount(); i++) {
		host = _serverDelim[i].GetFirstString();
		if (strcmp((const char*)host, server) == 0) {
			delim = (const char*)_serverDelim[i].GetSecondString();
			return delim;
		}
	}
	return NULL;
}
SdmError
PasswordDialogManager::setDelimiter(const char *server, SdmMessageStore *mstore)
{
        SdmSortIntStrL dirInfo;
        SdmError err;  
        SdmString *reference, *pattern;
        const char *delimTable[3] = {"/", "\\", "."};
        int i;

        // Check to see if delimiter and host name are already stored in cache.
        if (delimiter(server))
                return (err=Sdm_EC_Success);

        // Not in cache, so put it there. Initially do a scan with null reference and
        // pattern. If single entry is returned than that is the delimiter.
        // If it fails than start guessing what the delimiter is...
        // We start with "/". "\", and ".".

        pattern = new SdmString((const char*)"");
        reference = new SdmString((const char*)"");
        mstore->ScanNamespace(err, dirInfo, *reference, *pattern);
        if (!err && (dirInfo.ElementCount() == 1 ||
                (dirInfo.ElementCount() == 2 &&
                dirInfo[0].GetString() == dirInfo[1].GetString())) ) {
                if (dirInfo[0].GetString() != NULL &&
                        *dirInfo[0].GetString() != NULL) {
                        _serverDelim(-1).SetBothStrings(server,
                                (const char*)dirInfo[0].GetString());
                        delete pattern; delete reference;
                        return err;
                }
        }
 
        for (i = 0; i < 3; i++) {
                err.Reset();
                dirInfo.ClearAllElements();
                delete reference;
                reference = new SdmString(delimTable[i]);
                mstore->ScanNamespace(err, dirInfo, *reference, *pattern);
                if (!err && (dirInfo.ElementCount() == 1 ||
                        (dirInfo.ElementCount() == 2 &&
                        dirInfo[0].GetString() == dirInfo[1].GetString())) &&
                        dirInfo[0].GetString() == delimTable[i]) {
                        if (dirInfo[0].GetString() != NULL &&
                                 *dirInfo[0].GetString() != NULL) {
                                _serverDelim(-1).SetBothStrings(server, delimTable[i]);
                                break;
                        }
                }
        }
        delete pattern; delete reference;
        return err;
}
static Widget
getTopLevelShell(Widget w)
{
  Widget diashell, topshell;

  for (diashell=w;!XtIsShell(diashell);diashell=XtParent(diashell));

  for ( topshell=diashell;XtIsTopLevelShell( topshell );
        topshell = XtParent( topshell ) );

  return(topshell);
}

static void
cleanupDialog(PasswordDialogManager *dialog)
{
    Widget w = dialog->dialog();

    if (XtHasCallbacks (w, XmNokCallback) == XtCallbackHasSome)
	XtRemoveAllCallbacks (w, XmNokCallback);

    if (XtHasCallbacks (w, XmNapplyCallback) == XtCallbackHasSome)
	XtRemoveAllCallbacks (w, XmNapplyCallback);

    if (XtHasCallbacks (w, XmNcancelCallback) == XtCallbackHasSome)
	XtRemoveAllCallbacks (w, XmNcancelCallback);

    if (XtHasCallbacks (w, XmNhelpCallback) == XtCallbackHasSome)
	XtRemoveAllCallbacks (w, XmNhelpCallback);
}
void
PasswordDialogManager::cancelCallback(Widget w,
                                      XtPointer clientData,
                                      XtPointer callData)
{
    XmAnyCallbackStruct *cbs = (XmAnyCallbackStruct *) callData;
    DialogCallbackData *dcb = (DialogCallbackData *)clientData;

    int *answer = (int *)dcb->clientData();
    *answer = 2;

    Widget topShell = getTopLevelShell(w);

    Atom WM_DELETE_WINDOW=XmInternAtom( XtDisplay( w ),
                                        "WM_DELETE_WINDOW",
                                        False );

    XmRemoveWMProtocolCallback(topShell,
                        WM_DELETE_WINDOW,
                        ( XtCallbackProc ) &PasswordDialogManager::cancelCallback,
                        (XtPointer) dcb );
 
    // If called from the window manager menu, pull out the dialog and call cleanup
    if ( cbs->reason == XmCR_PROTOCOLS) {
        cleanupDialog((PasswordDialogManager *)dcb->dialogManager());
    }
    else {
        if (XtHasCallbacks (w, XmNokCallback) == XtCallbackHasSome)
            XtRemoveAllCallbacks (w, XmNokCallback);
 
        if (XtHasCallbacks (w, XmNapplyCallback) == XtCallbackHasSome)
            XtRemoveAllCallbacks (w, XmNapplyCallback);
 
        if (XtHasCallbacks (w, XmNcancelCallback) == XtCallbackHasSome)
            XtRemoveAllCallbacks (w, XmNcancelCallback);
 
        if (XtHasCallbacks (w, XmNhelpCallback) == XtCallbackHasSome)
            XtRemoveAllCallbacks (w, XmNhelpCallback);
 
        XtUnmanageChild(w);
    }
 
    delete dcb;
}
