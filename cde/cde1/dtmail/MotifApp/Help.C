/*
 *+SNOTICE
 *
 *      $Revision: 1.2 $
 *
 *      RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *      The information in this document is subject to special
 *      restrictions in a confidential disclosure agreement between
 *      HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *      document outside HP, IBM, Sun, USL, SCO, or Univel without
 *      Sun's specific written approval.  This document and all copies
 *      and derivative works thereof must be returned or destroyed at
 *      Sun's request.
 *
 *      Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */
 
#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Help.C	1.17 10/10/95"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <Xm/Xm.h>
#include <Dt/HelpDialog.h>
#include <Dt/HelpQuickD.h>
#include <Dt/Help.h>
#include "Help.hh"

#include <nl_types.h>
extern nl_catd catd;

void DisplayMain( Widget, char *, char *);
static Widget helpMain = NULL;
static Widget helpError = NULL;
static Widget versionMain = NULL;

Widget
getErrorHelpWidget(void)
{
    return (helpError);
}

void
clearErrorHelpWidget(void)
{
    helpError = NULL;
}

char *
getHelpId (Widget w)
{
    char *helpId;
    char *buf;
    char *index;
    int i = 0, j = 0;
 
    if (XtParent(w) == NULL) {
        helpId = (char *) malloc(1000);
	index = helpId;
	buf = XtName(w);
	while(*buf) {
	    if (isalnum(*buf)) {
		*index++ = toupper(*buf);
	    } else if ('_' == *buf) {
		*index++ = '-';
	    }
	    buf++;
	}
	*index++ = '\0';
        return (helpId);
    } else {
        helpId = getHelpId (XtParent(w));
	i = strlen(helpId);
	buf = XtName(w);
	while(*buf) {
	    if (isalnum(*buf)) {
		helpId[i++] = toupper(*buf);
	    } else if ('_' == *buf) {
		helpId[i++] = '-';
	    }
	    buf++;
	}
	helpId[i++] = '\0';
        return (helpId);
    }
 
}


void
DisplayVersion (
	Widget parent,
	char *helpVolume,
	char *locationId )
{
    Arg		args[10];
    int		n;
    Widget	printWidget;
    Widget	helpWidget;

    if (versionMain != NULL) {
	n = 0;
	XtSetArg (args[n], XmNtitle, catgets(catd, 1, 4,
	  "DtMail Version Dialog")); n++;
	if (helpVolume != NULL) {
	    XtSetArg (args[n], DtNhelpVolume, helpVolume); n++;
	}
	XtSetArg (args[n], DtNlocationId, locationId); n++;
	XtSetArg (args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
	XtSetValues(versionMain, args, n);
	XtManageChild(versionMain);
    } else {
	while (!XtIsSubclass(parent, applicationShellWidgetClass))
	    parent = XtParent(parent);

	// Build a new one in our cached list
	n = 0;
	XtSetArg (args[n], XmNtitle, catgets(catd, 1, 5,
	  "DtMail Version Dialog")); n++;
	if (helpVolume != NULL) {
	    XtSetArg (args[n], DtNhelpVolume, helpVolume); n++;
	}
	XtSetArg (args[n], DtNlocationId, locationId); n++;
	XtSetArg (args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
	versionMain = DtCreateQuickHelpDialog(parent, "versionWidget", args, n);
	XtAddCallback(versionMain, XmNokCallback,
		    CloseMainCB, (XtPointer) versionMain);
	
	// We do not want a print button for now so we unmap it
	printWidget = DtHelpQuickDialogGetChild (versionMain,
					    DtHELP_QUICK_PRINT_BUTTON);
	XtUnmanageChild (printWidget);

	// We do not want a help button for now so we unmap it 

	helpWidget = DtHelpQuickDialogGetChild (versionMain,
					    DtHELP_QUICK_PRINT_BUTTON);
	XtUnmanageChild (helpWidget);
	XtManageChild(versionMain);
    }
}


#ifdef DEAD_WOOD
// The callback for the Help Menu in the combo window.

void
HelpMenuCB (
	Widget widget,
	XtPointer	clientdata,
	XtPointer)
{
    Widget selWidget = NULL;
    int	status = DtHELP_SELECT_ERROR;

    // Determine which help button was activated and display the
    // appropriate help information.

    switch ((int) clientdata) {
	case HELP_ON_ITEM:
	    while (!XtIsSubclass(widget, applicationShellWidgetClass))
		widget = XtParent(widget);
	    status = DtHelpReturnSelectedWidgetId(widget, NULL, &selWidget);

	    switch ((int) status) {
		case DtHELP_SELECT_ERROR:
		    printf(catgets(catd, 2, 1, "Selection Error, cannot continue\n"));
		    break;
		case DtHELP_SELECT_VALID:
		    while (selWidget != NULL) {
			if ((XtHasCallbacks(selWidget, XmNhelpCallback)
					== XtCallbackHasSome)) {
			    XtCallCallbacks((Widget)selWidget,
					XmNhelpCallback, NULL);
			    break;
			} else {
			    selWidget = XtParent(selWidget);
			}
		    }
		    break;
		case DtHELP_SELECT_ABORT:
		    printf(catgets(catd, 2, 2, "Selection aborted by user.\n"));
		    break;
		case DtHELP_SELECT_INVALID:
		    printf(catgets(catd, 1, 6, 
			   "You must select a component withing your app.\n"));
		    break;
	    }
	    break;
	case HELP_ON_TOPIC:
	    DisplayMain(widget, NULL, APP_MENU_ID);
	    break;
	case HELP_ON_VERSION:
	    DisplayVersion(widget, NULL, VER_MENU_ID);
	    break;
	default:
	    break;
    }
}
#endif /* DEAD_WOOD */

	
// Callback to process JUMP-NEW and APP-LINK hypertext requests in a
// given Help Dialog Window.
//
// This is the callback used for the DtNhyperLinkCallback
// on each of the help dialog widgets created.

static void
ProcessLinkCB (
	Widget,
	XtPointer,
	XtPointer callData)
{
//    Arg 	args[20];
//    Position 	xPos, yPos;
    int 	appLinkNum = 0;

    DtHelpDialogCallbackStruct * hyperData =
		(DtHelpDialogCallbackStruct *) callData;
    
}


void
DisplayMain(
	Widget parent,
	char *helpVolume,
	char *locationId)
{
    Arg	args[10];
    int n;

    if (helpMain != NULL) {
	n = 0;
#ifdef undef
	XtSetArg (args[n], XmNtitle, catgets(catd, 1, 7, "DtMail Help")); n++;
#endif
	XtSetArg (args[n], XmNtitle, catgets(catd, 1, 12, "Mailer : Help")); n++;
	if (helpVolume != NULL) {
	    XtSetArg (args[n], DtNhelpVolume, helpVolume); n++;
	}
	XtSetArg (args[n], DtNlocationId, locationId); n++;
	XtSetArg (args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
	//XtSetArg (args[n], DtNlocationId, "DTMAILVIEWMAINWINDOWWORK-AREA"); n++;
	XtSetValues (helpMain, args, n);
	XtManageChild(helpMain);
	XtPopup(XtParent(helpMain), XtGrabNone);
    } else {
	while (!XtIsSubclass(parent, applicationShellWidgetClass))
	    parent = XtParent(parent);
	
	// Build a new one in our cached list
	n = 0;
#ifdef undef
	XtSetArg (args[n], XmNtitle, catgets(catd, 1, 8, "DtMail Help")); n++;
#endif
	XtSetArg (args[n], XmNtitle, catgets(catd, 1, 12, "Mailer : Help")); n++;
	if (helpVolume != NULL) {
	    XtSetArg (args[n], DtNhelpVolume, helpVolume); n++;
	}
	//XtSetArg (args[n], DtNlocationId, "DTMAILVIEWMAINWINDOWWORK-AREAPANEDWFORM2ROWCOLUMNPREVIOUS"); n++;
	XtSetArg (args[n], DtNlocationId, locationId); n++;
	XtSetArg (args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
	helpMain = DtCreateHelpDialog(parent, "Mailer", args, n);

	XtAddCallback(helpMain, DtNhyperLinkCallback, ProcessLinkCB, NULL);

	XtAddCallback(helpMain, DtNcloseCallback, 
			CloseMainCB, (XtPointer) helpMain);
	
	XtManageChild(helpMain);
    }
}

void
DisplayErrorHelp(
	Widget parent,
	char *helpVolume,
	char *locationId)
{
    Arg	args[10];
    int n;

    if (helpError) {
	n = 0;
#ifdef undef
	XtSetArg (args[n], XmNtitle, catgets(catd, 1, 7, "DtMail Help")); n++;
#endif
	XtSetArg (args[n], XmNtitle, catgets(catd, 1, 12, "Mailer : Help")); n++;
	if (helpVolume != NULL) {
	    XtSetArg (args[n], DtNhelpVolume, helpVolume); n++;
	}
	XtSetArg (args[n], DtNlocationId, locationId); n++;
	XtSetArg (args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
	//XtSetArg (args[n], DtNlocationId, "DTMAILVIEWMAINWINDOWWORK-AREA"); n++;
	XtSetValues (helpError, args, n);
	XtManageChild(helpError);
    } else {
    
	// Create a new help on the error dialogs each time, destroy it
	// when done.
	n = 0;
#ifdef undef
	XtSetArg (args[n], XmNtitle, catgets(catd, 1, 8, "DtMail Help")); n++;
#endif
	XtSetArg (args[n], XmNtitle, catgets(catd, 1, 12, "Mailer : Help")); n++;
	if (helpVolume != NULL) {
	XtSetArg (args[n], DtNhelpVolume, helpVolume); n++;
	}
	//XtSetArg (args[n], DtNlocationId, "DTMAILVIEWMAINWINDOWWORK-AREAPANEDWFORM2ROWCOLUMNPREVIOUS"); n++;
	XtSetArg (args[n], DtNlocationId, locationId); n++;
	XtSetArg (args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
	helpError = DtCreateHelpDialog(parent, "Mailer", args, n);

	XtAddCallback(helpError, DtNhyperLinkCallback, ProcessLinkCB, NULL);

	XtAddCallback(helpError, DtNcloseCallback, 
		CloseMainCB, (XtPointer) helpError);

	XtManageChild(helpError);
    }
}


// The Help Callback for when the F1 key is pressed or when On Item
// Help is selected from the Help menu.

void
HelpCB (
	Widget w,
	XtPointer clientData,
	XtPointer)
{
    char *locationId = (char *) clientData;

    // printf("locationId = %s\n", locationId);
    // Just display the proper help based on the id string passed in.

    // We pass in a NULL for our helpVolume and let the value defined
    // in the app-defaults file be used.

    DisplayMain (w, "Mailer", locationId);
}

void
HelpErrorCB (
	Widget w,
	XtPointer clientData,
	XtPointer)
{
    char *locationId = (char *) clientData;

    // printf("locationId = %s\n", locationId);
    // Just display the proper help based on the id string passed in.

    // We pass in a NULL for our helpVolume and let the value defined
    // in the app-defaults file be used.

    DisplayErrorHelp (w, "Mailer", locationId);
}


// Callback to process close requests on our main help dialog.

static void
CloseMainCB (
	Widget,
	XtPointer	clientData,
	XtPointer)
{
    Widget currentDialog = (Widget) clientData;

    // Unmap and clean up help widget

    XtUnmanageChild(currentDialog);
}

#if defined(PRINT_HELPIDS)

void
printHelpId (char *w_name, Widget w)
{
    char *helpId;
 
    helpId = getHelpId (w);
    printf("%s = %s\n", w_name, helpId);
    free(helpId);
 
}

#else

void
printHelpId (char *, Widget)
{
}

#endif
