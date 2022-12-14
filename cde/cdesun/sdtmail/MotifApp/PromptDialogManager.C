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
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)PromptDialogManager.C	1.11 08/07/96"
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
//         Copyright 1991 by Prentice Hall
//         All Rights Reserved
//
//  Permission to use, copy, modify, and distribute this software for 
//  any purpose except publication and without fee is hereby granted, provided 
//  that the above copyright notice appear in all copies of the software.
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
// PromptDialogManager.C: 
//////////////////////////////////////////////////////////
#include "PromptDialogManager.h"
#include <Xm/Xm.h>
#include <Xm/SelectioB.h>
#include <assert.h>
// Define an instance to be available throughout the framework.

PromptDialogManager *thePromptDialogManager = 
    new PromptDialogManager ( "PromptDialog" );

PromptDialogManager::PromptDialogManager ( char   *name ) : 
                                 DialogManager ( name )
{
    // Empty
}

Widget PromptDialogManager::createDialog ( Widget parent )
{
  Widget dialog = XmCreatePromptDialog ( parent, _name, NULL, 0);
    
  XtVaSetValues ( dialog,
		  XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
		  NULL );
  
  return dialog;
}


Widget
PromptDialogManager::post(  char *title,
			    char		*text,
			    void		*clientData,
			    DialogCallback ok,
			    DialogCallback apply,
			    DialogCallback cancel,
			    DialogCallback help )
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

    if ( ok ) {
      XtAddCallback ( dialog, 
		      XmNokCallback, 
		      &DialogManager::okCallback,
		      (XtPointer) dcb );
    } else
      {
	Widget w = XmSelectionBoxGetChild ( dialog,
					  XmDIALOG_OK_BUTTON );
        XtUnmanageChild ( w );
      }


    if (apply) {
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

    if ( cancel )
      XtAddCallback ( dialog, 
		      XmNcancelCallback, 
		      &DialogManager::cancelCallback,
		      (XtPointer) dcb );
    else
      {
	Widget w = XmSelectionBoxGetChild ( dialog,
					  XmDIALOG_CANCEL_BUTTON );
        XtUnmanageChild ( w );
      }
    
    
    if ( help )	    
	XtAddCallback ( dialog, 
		       XmNhelpCallback, 
		       &DialogManager::helpCallback,
		       (XtPointer) dcb );
    else
    {
	Widget w = XmSelectionBoxGetChild ( dialog,
					 XmDIALOG_HELP_BUTTON );
        XtUnmanageChild ( w );
    }
    
    // Post the dialog.
    
    XtManageChild ( dialog );

    
    {
      Widget w = XmSelectionBoxGetChild(dialog, XmDIALOG_OK_BUTTON);
      XtRemoveAllCallbacks(w, XmNactivateCallback);
      XtRemoveAllCallbacks(w, XmNarmCallback);
    }
    return dialog;
}

Widget 
PromptDialogManager::post(    char	  *title,
			char          *text,
			Widget	   wid,
			void          *clientData,
			DialogCallback ok,
			DialogCallback cancel,
			DialogCallback help)
{
  // just call parent.
  return DialogManager::post(title, text, wid, clientData, ok, cancel, help);
}

Widget 
PromptDialogManager::post (	char	  *title,
			char          *text,
			void          *clientData,
			DialogCallback ok,
			DialogCallback cancel,
			DialogCallback help)
{
  // just call parent.
  return DialogManager::post(title, text, clientData, ok, cancel, help);
}


Widget
PromptDialogManager::post(  char *title,
			    char		*text,
			    Widget wid,
			    void		*clientData,
			    DialogCallback ok,
			    DialogCallback apply,
			    DialogCallback cancel,
			    DialogCallback help )
{

    // Get a dialog widget from the cache
    
    Widget dialog = getDialog(wid);
    
    // Make sure the dialog exists, and that it is an XmMessageBox
    // or subclass, since the callbacks assume this widget type
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

    if ( ok ) {
      XtAddCallback ( dialog, 
		      XmNokCallback, 
		      &DialogManager::okCallback,
		      (XtPointer) dcb );
	Widget w = XmSelectionBoxGetChild(dialog, XmDIALOG_OK_BUTTON );
    } else {
	Widget w = XmSelectionBoxGetChild ( dialog,
					  XmDIALOG_OK_BUTTON );
        XtUnmanageChild ( w );
      }


    if (apply) {
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

    if ( cancel )
      XtAddCallback ( dialog, 
		      XmNcancelCallback, 
		      &DialogManager::cancelCallback,
		      (XtPointer) dcb );
    else
      {
	Widget w = XmSelectionBoxGetChild ( dialog,
					  XmDIALOG_CANCEL_BUTTON );
        XtUnmanageChild ( w );
      }
    
    
    if ( help )	    
	XtAddCallback ( dialog, 
		       XmNhelpCallback, 
		       &DialogManager::helpCallback,
		       (XtPointer) dcb );
    else
    {
	Widget w = XmSelectionBoxGetChild ( dialog,
					 XmDIALOG_HELP_BUTTON );
        XtUnmanageChild ( w );
    }
    
    // Post the dialog.
    
    XtManageChild ( dialog );

    
    return dialog;
}
