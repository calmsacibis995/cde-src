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
#pragma ident "@(#)DtMailGenDialog.C	1.30 08/21/95"
#endif

//////////////////////////////////////////////////////////
// DtMailGenDialog.C: Generic dialog based on MessageBox
//////////////////////////////////////////////////////////
#include <Dt/Dt.h>
#include <Dt/Icon.h>
#include <Dt/IconP.h>
#include <Dt/IconFile.h>
#include "DtMailGenDialog.hh"
#include "Application.h"
#include "MailMsg.h"
#include "Help.hh"
#include "DtMailHelp.hh"
#include <Xm/MessageB.h>
#include <Xm/PushBG.h>
#include <Xm/MwmUtil.h>
#include <stdio.h>
#include <assert.h>

#ifdef undef
/* NL_COMMENT
 * This text is replaced with the text below.  It is just left in for
 * message catalog reasons and should eventually be removed.
 */
static const char * ABOUT_TITLE = catgets(DT_catd, 1, 178, "Mailer - About Dtmail");
static const char * DTMAIL_VERSION = catgets(DT_catd, 1, 179, 
						"Dtmail Version 1.0.1");
#endif
static const char * ABOUT_TITLE = catgets(DT_catd, 1, 235, "Mailer - About Mailer");
static const char * DTMAIL_VERSION = catgets(DT_catd, 1, 236, 
						"Mailer Version ");

// Leave credits blank for now
static const char * credits = "Dtmail was brought to you by: ";

static int doCredits = 0;

DtMailGenDialog::DtMailGenDialog ( 
    char   *name,
    Widget parent
) : UIComponent ( name )
{
    
    _w = XmCreateMessageDialog(parent, name, NULL, 0);
    XtVaSetValues (_w,
		   XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
		   NULL );

    // Disable the frame menu from all dialogs.  We don't want 
    // the user to be able to dismiss dialogs through the frame
    // menu.
    //
    XtVaSetValues (XtParent( _w ),
		   XmNmwmDecorations, MWM_DECOR_ALL | MWM_DECOR_MENU,
		   NULL );

    _info_dialog = 0;
    _otherWidget = (Widget) NULL;
}

 
Widget 
DtMailGenDialog::post(void          *clientData,
		      DialogCallback ok,
		      DialogCallback cancel,
		      DialogCallback other,
		      DialogCallback help,
		      char *helpId
)
{
    // _w is the MessageBox widget created in the constructor...

    Widget dialog = _w;
    
    // Make sure the dialog exists, and that it is an XmMessageBox
    // or subclass, since the callbacks assume this widget type
    
    assert (dialog);
//    assert ( XtIsSubclass ( dialog, xmMessageBoxWidgetClass ) );

    // Make sure the dialog buttons are managed
    Widget ok_button = XmMessageBoxGetChild ( dialog,
					  XmDIALOG_OK_BUTTON );
    Widget cancel_button = XmMessageBoxGetChild ( dialog,
					  XmDIALOG_CANCEL_BUTTON );
    Widget help_button = XmMessageBoxGetChild ( dialog,
					  XmDIALOG_HELP_BUTTON );
	
    // Create an object to carry the additional data needed
    // to cache the dialogs.
    
    DtMailDialogCallbackData *dcb = new DtMailDialogCallbackData( 
						(DtMailGenDialog *) this, 
						clientData,
						ok, 
						cancel, 
						other, 
						help, 
						_otherWidget);
    // Install callback function for each button 
    // support by Motif dialogs. If there is no help callback
    // unmanage the corresponding button instead, if possible.

    if ( ok ) {
      	XtAddCallback (dialog,
		      XmNokCallback, 
		      &DtMailGenDialog::okCallback,
		      (XtPointer) dcb );
	if (!XtIsManaged (ok_button))
		XtManageChild ( ok_button );
    } else {
        XtUnmanageChild ( ok_button );
    }


    if ( cancel ) {
	XtAddCallback ( dialog, 
		      XmNcancelCallback, 
		      &DtMailGenDialog::cancelCallback,
		      (XtPointer) dcb );
	if (!XtIsManaged (cancel_button))
		XtManageChild ( cancel_button );
    } else {
        XtUnmanageChild ( cancel_button );
    }


    if (other) {
	XtAddCallback ( _otherWidget,
			XmNactivateCallback,
		        &DtMailGenDialog::otherCallback,
                        (XtPointer) dcb);
    } else {
	if (_otherWidget) {
	    XtUnmanageChild ( _otherWidget );
	}
    }


    if ( help ) {
	XtAddCallback ( dialog,
                       XmNhelpCallback,
                       &HelpErrorCB,
                       (XtPointer) helpId);
	if (!XtIsManaged (help_button))
		XtManageChild ( help_button );
    } else {
	XtUnmanageChild ( help_button );
    }

    // Post the dialog.
    
    XtManageChild ( dialog );

    
    return dialog;
}

void 
DtMailGenDialog::okCallback(
    Widget w,
    XtPointer clientData,
    XtPointer cbs
)
{
    XmPushButtonCallbackStruct * pbcs = (XmPushButtonCallbackStruct *)cbs;

    DtMailDialogCallbackData *dcd = (DtMailDialogCallbackData *) clientData;
    DtMailGenDialog      *obj = (DtMailGenDialog *) dcd->dialog();
    DialogCallback      callback;
    
    // If caller specified an ok callback, call the function
    
    if ( ( callback = dcd->ok() ) != NULL )
	( *callback )( dcd->clientData() );

    // If the help widget was popped up, destroy it.
    Widget helpWidget = getErrorHelpWidget();
    if (helpWidget)
    {
        XtUnmanageChild (helpWidget);
	XtDestroyWidget (helpWidget);
	clearErrorHelpWidget();
    }

    // Reset for the next time
    
    Widget ow = dcd->other_w();
    if (ow != NULL)
    	XtRemoveCallback ( ow, 
		      XmNactivateCallback, 
		      &DtMailGenDialog::otherCallback,
		      (XtPointer) dcd );

    obj->cleanup ( w, dcd );

    if (obj->_info_dialog && (pbcs->event->xbutton.state & (ShiftMask | ControlMask))) {
#ifdef NEVER
	// Don't do credits for now
	doCredits = 1;
#endif
	doCredits = 0;
	obj->setToAboutDialog();
	// char * helpId = "About";
	char * helpId = NULL;
	int answer = obj->post_and_return(catgets(DT_catd, 1, 180, "OK"),
					  helpId);
    }
}

void DtMailGenDialog::cancelCallback ( Widget    w, 
				    XtPointer clientData,
				    XtPointer )
{
    DtMailDialogCallbackData *dcd = (DtMailDialogCallbackData *) clientData;
    DtMailGenDialog      *obj = (DtMailGenDialog *) dcd->dialog();
    DialogCallback      callback;
    
    if ( ( callback = dcd->cancel() ) != NULL )
	( *callback )( dcd->clientData() );
    
    // If the help widget was popped up, destroy it.
    Widget helpWidget = getErrorHelpWidget();
    if (helpWidget)
    {
        XtUnmanageChild (helpWidget);
	XtDestroyWidget (helpWidget);
	clearErrorHelpWidget();
    }


    Widget ow = dcd->other_w();
    if (ow != NULL)
    	XtRemoveCallback ( ow, 
		      XmNactivateCallback, 
		      &DtMailGenDialog::otherCallback,
		      (XtPointer) dcd );

    obj->cleanup ( w, dcd );
}

void DtMailGenDialog::otherCallback ( Widget w, 
				    XtPointer clientData,
				    XtPointer )
{
    DtMailDialogCallbackData *dcd = (DtMailDialogCallbackData *) clientData;
    DtMailGenDialog      *obj = (DtMailGenDialog *) dcd->dialog();
    DialogCallback      callback;
    
    if ( ( callback = dcd->other() ) != NULL )
	( *callback )( dcd->clientData() );

    XtRemoveCallback ( w, 
		      XmNactivateCallback, 
		      &DtMailGenDialog::otherCallback,
		      (XtPointer) dcd );
    
    Widget pw = XtParent(w);
    obj->cleanup ( pw, dcd );
}

void DtMailGenDialog::helpCallback ( Widget,
                                XtPointer clientData,
                                XtPointer )
{
    DtMailDialogCallbackData *dcd = (DtMailDialogCallbackData *) clientData;
    DtMailGenDialog      *obj = (DtMailGenDialog *) dcd->dialog();
    DialogCallback      callback;

    if ( ( callback = dcd->help() ) != NULL )
      ( *callback )( dcd->clientData() );
}

void DtMailGenDialog::cleanup ( Widget w, DtMailDialogCallbackData *dcd )
{
    // Remove all callbacks to avoid having duplicate 
    // callback functions installed.
    
    XtRemoveCallback ( w, 
		      XmNokCallback, 
		      &DtMailGenDialog::okCallback,
		      (XtPointer) dcd );
    
    XtRemoveCallback ( w, 
		      XmNcancelCallback, 
		      &DtMailGenDialog::cancelCallback,
		      (XtPointer) dcd );
   
    if (XtHasCallbacks (w, XmNhelpCallback) == XtCallbackHasSome)
    {
        XtRemoveAllCallbacks ( w, 
 		      XmNhelpCallback);
    }
    
    // Delete the DtMailDialogCallbackData instance for this posting
    
    delete dcd;
}

void
DtMailGenDialog::forceUpdate( Widget w )
{
  Widget diashell, topshell;
  Window diawindow, topwindow;

  Display		*dpy;
  XWindowAttributes	xwa;
  XEvent		event;
  if ( !w )
    return;
  XtAppContext cxt=XtWidgetToApplicationContext( w );
  for (diashell=w;!XtIsShell(diashell);diashell=XtParent(diashell));
  for ( topshell=diashell;XtIsTopLevelShell( topshell );
	topshell = XtParent( topshell ) );

//  if (XtIsRealized(diashell) && XtIsRealized(topshell)){
    dpy=XtDisplay(diashell);
    diawindow=XtWindow(diashell);
    topwindow=XtWindow(topshell);
    while ( XGetWindowAttributes(dpy,diawindow,&xwa) && 
	    xwa.map_state != IsViewable && XEventsQueued(dpy,QueuedAlready)){
//	if ( XGetWindowAttributes( dpy, topwindow, &xwa ) &&
//	   xwa.map_state != IsViewable )
//	  break;
        XtAppProcessEvent(cxt, XtIMXEvent );
    }
//  }
  XmUpdateDisplay(topshell);
}



// Added this extra functionality

void
genDialogOKCallback( int *data )
{
    *data=1;
}

void
genDialogCancelCallback( int *data )
{
    *data=2;
}

void
genDialogOtherCallback( int *data )
{
    *data=3;
}
// post_and_return takes a helpId, which is a string that is used to
// reference the related help in the Mailer help volume.  The helpId
// is passed to post(), which will attach help to the help button in
// the dialog.
int
DtMailGenDialog::post_and_return(char *helpId)
{
    int answer = 0;
    XmString okLabel, cancelLabel;

    // They may have been set via the overloaded post_and_return()
    // method before. Reset them to their default values...

    okLabel = XmStringCreateLocalized(catgets(DT_catd, 1, 181, "OK"));
    cancelLabel = XmStringCreateLocalized(catgets(DT_catd, 1, 182, "Cancel"));

    // Make sure the dialog exists, and that it is an XmMessageBox
    // or subclass, since the callbacks assume this widget type
    
    assert ( _w );

    XtVaSetValues(_w,
		  XmNokLabelString, okLabel,
		  XmNcancelLabelString, cancelLabel,
		  NULL);
    XmStringFree( okLabel);
    XmStringFree( cancelLabel);

    Widget dialog;
    if (helpId) {
	dialog =
	    this->post((void *) &answer,
		   ( DialogCallback ) &genDialogOKCallback,
		   ( DialogCallback ) &genDialogCancelCallback,
		   ( DialogCallback ) NULL,
		   ( DialogCallback ) &HelpErrorCB,
		   helpId
		   );
    } else {
	dialog =
	    this->post((void *) &answer,
		   ( DialogCallback ) &genDialogOKCallback,
		   ( DialogCallback ) &genDialogCancelCallback,
		   ( DialogCallback ) NULL,
		   ( DialogCallback ) NULL,
		   NULL
		   );
    }

    forceUpdate( dialog );
    while ( answer==0 ) 
    {
	XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMXEvent );
    }

    // Process just one more event to pop down dialog.
    XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMXEvent );

    return(answer);

}

// post_and_return(char *, char *) takes the string to be used for the
// OK button and the string that contains the helpId for the dialog being
// created, and passes them to post().
int
DtMailGenDialog::post_and_return(
	char *okLabelString,
	char *helpId
)
{
    int answer = 0;
    XmString okLabel;

    okLabel = XmStringCreateLocalized(okLabelString);

    // Make sure the dialog exists, and that it is an XmMessageBox
    // or subclass, since the callbacks assume this widget type
    
    assert ( _w );

    XtVaSetValues(_w,
		  XmNokLabelString, okLabel,
		  NULL);
    XmStringFree( okLabel);

    Widget dialog;
    if (helpId) {
	dialog = this->post((void *) &answer,
			       ( DialogCallback ) &genDialogOKCallback,
			       ( DialogCallback ) NULL,
			       ( DialogCallback ) NULL,
			       ( DialogCallback ) &HelpErrorCB,
			       helpId
			       );
    } else {
	dialog = this->post((void *) &answer,
			       ( DialogCallback ) &genDialogOKCallback,
			       ( DialogCallback ) NULL,
			       ( DialogCallback ) NULL,
			       ( DialogCallback ) NULL,
			       NULL
			       );
    }

    forceUpdate( dialog );
    while ( answer==0 ) 
    {
	XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMXEvent );
    }

    // Process just one more event to pop down dialog.
    XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMXEvent );

    return(answer);

}

// post_and_return(char*, char*, char*) takes the OK button label, Cancel
// button label, and the help id for the dialog and passes them to post().
int
DtMailGenDialog::post_and_return(
	char *okLabelString,
	char *cancelLabelString,
	char *helpId
)
{
    int answer = 0;
    XmString okLabel, cancelLabel;

    okLabel = XmStringCreateLocalized(okLabelString);
    cancelLabel = XmStringCreateLocalized(cancelLabelString);

    // Make sure the dialog exists, and that it is an XmMessageBox
    // or subclass, since the callbacks assume this widget type
    
    assert ( _w );

    XtVaSetValues(_w,
		  XmNokLabelString, okLabel,
		  XmNcancelLabelString, cancelLabel,
		  NULL);
    XmStringFree( okLabel);
    XmStringFree( cancelLabel);

    Widget dialog = NULL;
    if (helpId) {
	dialog = this->post((void *) &answer,
			       ( DialogCallback ) &genDialogOKCallback,
			       ( DialogCallback ) &genDialogCancelCallback,
			       ( DialogCallback ) NULL,
			       ( DialogCallback ) &HelpErrorCB,
			       helpId
			       );
    } else {
	dialog = this->post((void *) &answer,
			       ( DialogCallback ) &genDialogOKCallback,
			       ( DialogCallback ) &genDialogCancelCallback,
			       ( DialogCallback ) NULL,
			       ( DialogCallback ) NULL,
			       NULL
			       );
    }

    forceUpdate( dialog );
    while ( answer==0 ) 
    {
	XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMXEvent );
    }

    // Process just one more event to pop down dialog.
    XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMXEvent );

    return(answer);

}

int
DtMailGenDialog::post_and_return(
	char *okLabelString,
	char *cancelLabelString,
	char *otherLabelString,
	char *helpId
)
{
    int answer = 0;
    XmString okLabel, cancelLabel, otherLabel;

    okLabel = XmStringCreateLocalized(okLabelString);
    cancelLabel = XmStringCreateLocalized(cancelLabelString);
    otherLabel = XmStringCreateLocalized(otherLabelString);


    // Make sure the dialog exists, and that it is an XmMessageBox
    // or subclass, since the callbacks assume this widget type
    
    assert ( _w );

    Widget dialog = NULL;
    Widget cancel_w = XmMessageBoxGetChild ( _w, XmDIALOG_CANCEL_BUTTON );

    if (_otherWidget == NULL) {
	_otherWidget = XtVaCreateWidget(otherLabelString,
				xmPushButtonGadgetClass, _w,
				XmNleftAttachment, XmMessageBoxGetChild ( _w,
					  XmDIALOG_OK_BUTTON ),
				XmNrightAttachment, cancel_w,
				NULL);
	XtManageChild (_otherWidget);
    }

    if (!XtIsManaged(_otherWidget)) {
	XtManageChild (_otherWidget);
    }
    if (!XtIsManaged ( cancel_w ) ) {
	XtManageChild ( cancel_w );
    }

    XtVaSetValues(_w,
		  XmNokLabelString, okLabel,
		  XmNcancelLabelString, cancelLabel,
		  NULL);
    XtVaSetValues(_otherWidget,
		  XmNlabelString, otherLabel,
		  NULL);
    XmStringFree( okLabel);
    XmStringFree( cancelLabel);
    XmStringFree( otherLabel);

    if (helpId) {
        dialog = this->post((void *) &answer,
                   ( DialogCallback ) &genDialogOKCallback,
                   ( DialogCallback ) &genDialogCancelCallback,
                   ( DialogCallback ) &genDialogOtherCallback,
		   ( DialogCallback ) &HelpErrorCB,
                   helpId
                   );
    } else {
        dialog =
            this->post((void *) &answer,
                   ( DialogCallback ) &genDialogOKCallback,
                   ( DialogCallback ) &genDialogCancelCallback,
                   ( DialogCallback ) &genDialogOtherCallback,
                   ( DialogCallback ) NULL,
                   NULL
                   );
    }

    forceUpdate( dialog );
    while ( answer==0 ) 
    {
	XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMXEvent );
    }

    // Process just one more event to pop down dialog.
    XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMXEvent );

    return(answer);

}
void
DtMailGenDialog::setDialog(char * title, char * text, unsigned char type)
{
    XmString titleStr = XmStringCreateLocalized (title);
    XmString xmStr = XmStringCreateLocalized(text);
    XtVaSetValues ( _w,
		    XmNmessageString, xmStr,
		    XmNdialogTitle, titleStr,
		    XmNdialogType,  type,
		    NULL );
    XmStringFree(xmStr);
    XmStringFree ( titleStr );
    _info_dialog = 0;
}

void
DtMailGenDialog::setToQuestionDialog(
    char *title,
    char *text
)
{
    setDialog(title, text, XmDIALOG_QUESTION);
}

void
DtMailGenDialog::setToWarningDialog(
    char *title,
    char *text
)
{
    setDialog(title, text, XmDIALOG_WARNING);
}

void
DtMailGenDialog::setToErrorDialog(
    char *title,
    char *text
)
{
    setDialog(title, text, XmDIALOG_ERROR);
}

#ifdef DEAD_WOOD
void
DtMailGenDialog::setToInfoDialog(
    char *title,
    char *text
)
{
    setDialog(title, text, XmDIALOG_INFORMATION);
}
#endif /* DEAD_WOOD */

extern "C" Pixmap _DtGetMask(Screen * screen, char * image_name);

void
DtMailGenDialog::setToAboutDialog(void)
{
static char VERSIONBUF[120];

    (void) sprintf(VERSIONBUF, "%s%d.%d.%d", DTMAIL_VERSION, SDtVERSION, SDtREVISION, SDtUPDATE_LEVEL);
    if (doCredits) {
	setDialog((char *)"Credits",
		  (char *)credits,
		  XmDIALOG_INFORMATION);
	doCredits = 0;
    }
    else {
	setDialog((char *)ABOUT_TITLE,
		  (char *)VERSIONBUF,
		  XmDIALOG_INFORMATION);
    }

    _info_dialog = 1;

    char * icon_filename = XmGetIconFileName(XtScreen(_w),
					     NULL,
					     "DtMail",
					     NULL,
					     DtLARGE);

    if (icon_filename == NULL) {
	return;
    }

    Pixmap fg, bg;

    XtVaGetValues (_w,
		   XmNforeground, &fg,
		   XmNbackground, &bg,
		   NULL);

    Pixmap icon = XmGetPixmap(XtScreen(_w),
			      icon_filename,
			      fg, bg);

    Pixmap icon_mask = _DtGetMask(XtScreen(_w), icon_filename);

    Pixmap clipped_icon = icon;
    if (icon_mask) {
	Window root;
	int x, y;
	unsigned int width, height, border_width, depth;
	XGetGeometry(XtDisplay(_w),
		     icon,
		     &root,
		     &x, &y,
		     &width, &height,
		     &border_width, &depth);

	XtRealizeWidget(_w);

	clipped_icon = XCreatePixmap(XtDisplay(_w),
				     XtWindow(_w),
				     width,
				     height,
				     depth);

	XGCValues gc_vals;
	GC gc;
	memset(&gc_vals, 0, sizeof(gc_vals));
	gc_vals.background = bg;
	gc_vals.foreground = bg;
	gc_vals.fill_style = FillSolid;
	gc = XCreateGC(XtDisplay(_w),
		       XtWindow(_w),
		       GCForeground | GCBackground | GCFillStyle,
		       &gc_vals);

	XFillRectangle(XtDisplay(_w),
		       clipped_icon,
		       gc,
		       0, 0,
		       width, height);

	XFreeGC(XtDisplay(_w), gc);

	memset(&gc_vals, 0, sizeof(gc_vals));
	gc_vals.background = bg;
	gc = XCreateGC(XtDisplay(_w),
		       XtWindow(_w),
		       GCBackground,
		       &gc_vals);

	XSetClipMask(XtDisplay(_w), gc, icon_mask);

	XCopyArea(XtDisplay(_w),
		  icon,
		  clipped_icon,
		  gc,
		  0, 0,
		  width, height,
		  0, 0);
	XFreeGC(XtDisplay(_w), gc);
    }

    XtVaSetValues ( _w,
		    XmNsymbolPixmap, clipped_icon,
		    NULL );
}
