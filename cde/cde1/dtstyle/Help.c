/* $XConsortium: Help.c /main/cde1_maint/2 1995/10/23 11:15:20 gtsang $ */
/****************************************************************************
 ****************************************************************************
 **
 **   File:        HelpCB.h
 **
 **   Project:     DT 3.0
 **
 **   Description: Routines that create/cache the dialogs for Help
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992.  All rights are
 **  reserved.  Copying or other reproduction of this program
 **  except for archival purposes is prohibited without prior
 **  written consent of Hewlett-Packard Company.
 **
 **
 **
 ****************************************************************************/

#include <stdio.h>

#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/MwmUtil.h>
#include <Dt/DtP.h>

#include <Dt/Help.h>
#include <Dt/HelpDialog.h>

#include "Main.h"
#include "Help.h"

static Widget
#ifdef _NO_PROTO
GetHelpDialog( )
#else
GetHelpDialog( )
#endif /* _NO_PROTO */
{
    static HelpStruct       *pHelpCache;

    HelpStruct *pHelp;
    Arg args[5];
    int n;

    if(pHelpCache == (HelpStruct *)NULL)
    {
	pHelp = pHelpCache = (HelpStruct *)XtMalloc(sizeof(HelpStruct));
	pHelp->pNext = (HelpStruct *)NULL;
	pHelp->pPrev = (HelpStruct *)NULL;
	pHelp->inUse = True;

	n = 0;
        XtSetArg(args[n], XmNtitle, 
            ((char *)GETMESSAGE(2, 7, "Style Manager - Help"))); n++;
	pHelp->dialog = DtCreateHelpDialog(style.shell, "helpDlg", 
					    args, n);

        DtWsmRemoveWorkspaceFunctions(style.display, 
                XtWindow(XtParent(pHelp->dialog)));
        XtAddCallback(pHelp->dialog, DtNhyperLinkCallback, 
                      (XtCallbackProc)HelpHyperlinkCB, NULL);
        XtAddCallback(pHelp->dialog, DtNcloseCallback, 
                      (XtCallbackProc)HelpCloseCB, pHelpCache);
        return pHelp->dialog;
    }
    else
    {
	for(pHelp = pHelpCache; 
	    pHelp != (HelpStruct *)NULL;
	    pHelp = pHelp->pNext)
	{
	    if(pHelp->inUse == False)
	    {
		pHelp->inUse = True;
		return pHelp->dialog;
	    }
	}
	pHelp = (HelpStruct *) XtMalloc(sizeof(HelpStruct));
	pHelpCache->pPrev = pHelp;
	pHelp->pNext = pHelpCache;
	pHelpCache = pHelp;
	pHelp->pPrev = (HelpStruct *)NULL;
	pHelp->inUse = True;

        n=0;
        XtSetArg(args[n], XmNtitle, 
            ((char *)GETMESSAGE(2, 7, "Style Manager - Help"))); n++;
	pHelp->dialog = DtCreateHelpDialog(style.shell, "helpDlg", 
					    args, n);
        DtWsmRemoveWorkspaceFunctions(style.display, 
                      XtWindow(XtParent(pHelp->dialog)));
        XtAddCallback(pHelp->dialog, DtNhyperLinkCallback, 
                      (XtCallbackProc)HelpHyperlinkCB, NULL);
        XtAddCallback(pHelp->dialog, DtNcloseCallback, 
                      (XtCallbackProc)HelpCloseCB, pHelpCache);
	return pHelp->dialog;
    }
}

void
#ifdef _NO_PROTO
DisplayHelp( helpVolume, locationId)
        char *helpVolume;
	char *locationId;
#else
DisplayHelp(
        char *helpVolume,
	char *locationId)
#endif /* _NO_PROTO */
{
    Widget helpDialog;
    Arg args[10];
    int n;

    helpDialog = GetHelpDialog();

    n = 0;
    XtSetArg(args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
    XtSetArg(args[n], DtNhelpVolume, helpVolume);	n++;
    XtSetArg(args[n], DtNlocationId, locationId);	n++;
    XtSetValues(helpDialog, args, n);

    XtManageChild(helpDialog);
    XtMapWidget(XtParent(helpDialog));
}

void
#ifdef _NO_PROTO
Help( helpVolume, locationId)
        char *helpVolume;
	char *locationId;
#else
Help(
        char *helpVolume,
	char *locationId)
#endif /* _NO_PROTO */
{
    static  Widget helpDialog = NULL;
    Arg args[10];
    int n;

    if(helpDialog == NULL)
    {
        n = 0;
        XtSetArg(args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
        XtSetArg(args[n], DtNhelpVolume, helpVolume);	n++;
        XtSetArg(args[n], DtNlocationId, locationId);	n++;
        XtSetArg (args[n], XmNtitle, 
            ((char *)GETMESSAGE(2, 7, "Style Manager - Help"))); n++;
	helpDialog = DtCreateHelpDialog(style.shell, "helpDlg", args, n);

        DtWsmRemoveWorkspaceFunctions(style.display, 
                      XtWindow(XtParent(helpDialog)));
        XtAddCallback(helpDialog, DtNhyperLinkCallback, 
                      (XtCallbackProc)HelpHyperlinkCB, NULL);
        XtAddCallback(helpDialog, DtNcloseCallback, 
                      (XtCallbackProc)HelpCloseCB, NULL);

        XtManageChild(helpDialog);
    }
    else
    {
        n = 0;
        XtSetArg(args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
        XtSetArg(args[n], DtNhelpVolume, helpVolume);	n++;
        XtSetArg(args[n], DtNlocationId, locationId);	n++;
        XtSetValues(helpDialog, args, n);
    }

    XtMapWidget(XtParent(helpDialog));
}

