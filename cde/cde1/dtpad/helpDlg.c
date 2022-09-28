/**********************************<+>*************************************
***************************************************************************
**
**  File:        helpDlg.c
**
**  Project:     HP DT dtpad, a memo maker type editor based on the
**               Dt Editor widget.
**
**  Description:  Routines which manipulate the dialogs associated with
**                operations in the Help menu.
**  -----------
**
*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992.  All rights are
**  reserved.  Copying or other reproduction of this program
**  except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
**
*******************************************************************
**  (c) Copyright 1993, 1994 Hewlett-Packard Company
**  (c) Copyright 1993, 1994 International Business Machines Corp.
**  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
**  (c) Copyright 1993, 1994 Novell, Inc.
********************************************************************
**
**
**************************************************************************
**********************************<+>*************************************/
#include <stdio.h>

#include "dtpad.h"
#include "help.h"

#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/MwmUtil.h>
#include <Dt/HelpDialog.h>


/************************************************************************
 * GetHelpDialog - obtains an "unused" help dialog from the cached list
 *	for the pad or, if one doesn't exist, creates a new help dialog
 *	and puts it in the cache.
 ************************************************************************/
static Widget
GetHelpDialog(
        Editor *pPad)
{
    HelpStruct *pHelp;

    if (pPad->pHelpCache == (HelpStruct *)NULL) {
	pHelp = pPad->pHelpCache = (HelpStruct *)XtMalloc(sizeof(HelpStruct));
	pHelp->pNext = (HelpStruct *)NULL;
	pHelp->pPrev = (HelpStruct *)NULL;
	pHelp->inUse = True;
	pHelp->dialog = CreateHelpDialog(pPad);
        return pHelp->dialog;
    } else {
	for (pHelp = pPad->pHelpCache; pHelp != (HelpStruct *)NULL;
	  pHelp = pHelp->pNext) {
	    if (pHelp->inUse == False) {
		pHelp->inUse = True;
		return pHelp->dialog;
	    }
	}
	pHelp = (HelpStruct *) XtMalloc(sizeof(HelpStruct));
	pPad->pHelpCache->pPrev = pHelp;
	pHelp->pNext = pPad->pHelpCache;
	pPad->pHelpCache = pHelp;
	pHelp->pPrev = (HelpStruct *)NULL;
	pHelp->inUse = True;
	pHelp->dialog = CreateHelpDialog(pPad);
	return pHelp->dialog;
    }
}


/************************************************************************
 * SetHelpVolAndDisplayHelp - sets the help volume and displays the help
 *	text beginning at the specified location id within the volume
 ************************************************************************/
void
SetHelpVolAndDisplayHelp(
        Editor *pPad,
        char *locationId,
        char *helpVolume)
{

    if (helpVolume == (char *) NULL) {
	if (pPad->xrdb.helpVol != (char *) NULL) {
	    helpVolume = pPad->xrdb.helpVol;
	} else {
	    helpVolume = TEXTEDITOR_HELP_VOLUME;
	}
    }

    DisplayHelp(pPad, helpVolume, locationId);
}


/************************************************************************
 * DisplayHelp - is called to display all the "normal" help windows.  It
 *	uses the one "MainHelp" dialog associated with the relevant pad.
 ************************************************************************/
void
DisplayHelp(
        Editor *pPad,
        char *helpVolume,
	char *locationId)
{
    if (pPad->MainHelp == NULL) {
	pPad->MainHelp = CreateHelpDialog(pPad);
    }

    DisplayHelpDialog(pPad, pPad->MainHelp, helpVolume, locationId);
}



/************************************************************************
 * CreateHelpDialog - creates the "MainHelp" help for the pad and sets
 *	up the hyperLink and Close callbacks for it.
 ************************************************************************/
Widget
CreateHelpDialog(
	Editor *pPad)
{
    Arg args[10];
    int n = 0;
    Widget dialog;

    dialog = DtCreateHelpDialog(pPad->app_shell, "helpDlg", args, n);
    XtAddCallback(dialog, DtNhyperLinkCallback, 
                  (XtCallbackProc)HelpHyperlinkCB, pPad);
    XtAddCallback(dialog, DtNcloseCallback, 
                  (XtCallbackProc)HelpCloseCB, pPad);
    return dialog;
}


/************************************************************************
 * DisplayNewHelpWindow - is called by the hyperLink callback,
 *	helpCB.c:HelpHyperlinkCB to open a new, cached, help dialog for
 *	this pad.
 ************************************************************************/
void
DisplayNewHelpWindow(
        Editor *pPad,
        char *helpVolume,
	char *locationId)
{
    DisplayHelpDialog(pPad, GetHelpDialog(pPad), helpVolume, locationId);
}


/************************************************************************
 * DisplayHelpDialog - sets the help type topic, volume, location id and
 *	title for the specified help dialog.
 ************************************************************************/
void
DisplayHelpDialog(
	Editor *pPad,
	Widget helpDialog,
	char *helpVolume,
	char *locationId)
{
    Arg args[10];
    int n = 0;
    char *titleStr, *helpStr;

    n = 0;
    XtSetArg(args[n], DtNhelpType, DtHELP_TYPE_TOPIC);n++;
    XtSetArg(args[n], DtNhelpVolume, helpVolume);	n++;
    XtSetArg(args[n], DtNlocationId, locationId);	n++;
    XtSetValues(helpDialog, args, n);

    /*
     * It's a bit bogus to set the dialog title each time we display the
     * dialog, but it's an easy way to make sure the title is right if the
     * pad is reused and the previous user had a mainTitle specified.
     */
    helpStr = (char *)GETMESSAGE(2, 1, "Help");
    titleStr = (char *)XtMalloc(strlen(helpStr) + strlen(pPad->dialogTitle)+1);
    sprintf(titleStr, "%s%s", pPad->dialogTitle, helpStr);
    n = 0;
    XtSetArg (args[n], XmNtitle, titleStr);		n++;
    XtSetValues(XtParent(helpDialog), args, n);

    XtManageChild(helpDialog);

    XtFree(titleStr);
}
