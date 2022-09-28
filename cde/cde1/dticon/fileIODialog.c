/* $XConsortium: fileIODialog.c /main/cde1_maint/2 1995/10/23 08:55:48 gtsang $ */
/*********************************************************************
*  (c) Copyright 1993, 1994 Hewlett-Packard Company
*  (c) Copyright 1993, 1994 International Business Machines Corp.
*  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
*  (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
*      Novell, Inc.
**********************************************************************/
/*******************************************************************************
        fileIODialog.c

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/FileSB.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include "main.h"
#include "externals.h"

#include <Xm/FileSB.h>

/*******************************************************************************
        Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

extern void Do_FileIO();

Widget  fileIODialog;

/*******************************************************************************
        Forward declarations of functions that are defined later in this file.
*******************************************************************************/

Widget  create_fileIODialog();

/*******************************************************************************
        The following are callback functions.
*******************************************************************************/

static void
#ifdef _NO_PROTO
cancelCB_fileIODialog( w, clientData, callbackArg )
        Widget w ;
        XtPointer clientData ;
        XtPointer callbackArg ;
#else
cancelCB_fileIODialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackArg )
#endif /* _NO_PROTO */
{
    XtUnmanageChild(fileIODialog);
}

/*******************************************************************************
        The following is the 'Interface function' which is the
        external entry point for creating this interface.
        This function should be called from your application or from
        a callback function.
*******************************************************************************/
Widget
#ifdef _NO_PROTO
create_fileIODialog()
#else
create_fileIODialog( void )
#endif /* _NO_PROTO */
{
    Widget    fileIODialog_shell, wid;
    XmString tmpXmStr, tmpXmStr1, tmpXmStr2, tmpXmStr3, tmpXmStr4,
             tmpXmStr6, tmpXmStr7;
    Arg  args[10];
    int  n;


    fileIODialog_shell = XtVaCreatePopupShell( "fileIODialog_shell",
                        xmDialogShellWidgetClass, mainWindow,
                        XmNtitle, GETSTR(2,2, "Icon Editor - File I/O"),
                        NULL );

    tmpXmStr  = GETXMSTR(2,6,  "Icon Editor - Save As");
    tmpXmStr1 = GETXMSTR(2,10, "Filter");
    tmpXmStr2 = GETXMSTR(2,12, "Folders");
    tmpXmStr3 = GETXMSTR(2,14, "Files");
    tmpXmStr4 = GETXMSTR(2,16, "Enter file name:");
    tmpXmStr6 = GETXMSTR(4,8,  "Cancel");
    tmpXmStr7 = GETXMSTR(4,10, "Help");

    fileIODialog = XtVaCreateWidget( "fileIODialog",
                        xmFileSelectionBoxWidgetClass, fileIODialog_shell,
                        XmNdialogTitle, tmpXmStr,
                        XmNfilterLabelString, tmpXmStr1,
                        XmNdirListLabelString, tmpXmStr2,
                        XmNfileListLabelString, tmpXmStr3,
                        XmNselectionLabelString, tmpXmStr4,
                        XmNapplyLabelString, tmpXmStr1,
                        XmNcancelLabelString, tmpXmStr6,
                        XmNhelpLabelString, tmpXmStr7,
                        NULL );

    XmStringFree(tmpXmStr);
    XmStringFree(tmpXmStr1);
    XmStringFree(tmpXmStr2);
    XmStringFree(tmpXmStr3);
    XmStringFree(tmpXmStr4);
    XmStringFree(tmpXmStr6);
    XmStringFree(tmpXmStr7);

    XtAddCallback(fileIODialog, XmNcancelCallback, cancelCB_fileIODialog, NULL);
    XtAddCallback(fileIODialog, XmNokCallback, Do_FileIO, NULL);

    n = 0;
    XtSetArg (args[n], XmNuseAsyncGeometry, True);                          n++;
    XtSetArg (args[n], XmNmwmInputMode,MWM_INPUT_PRIMARY_APPLICATION_MODAL);n++;
    XtSetValues (fileIODialog_shell, args, n);


    /* check resources to get rid of unwanted children :-) */
    if (!xrdb.useFileFilter)
    {
        /* remove file filter label and text, and filter button */
        wid = XmFileSelectionBoxGetChild(fileIODialog, XmDIALOG_FILTER_LABEL);
        XtUnmanageChild (wid);
        wid = XmFileSelectionBoxGetChild(fileIODialog, XmDIALOG_FILTER_TEXT);
        XtUnmanageChild (wid);

        /* if no File Lists, unmanage it... otherwise rename it */
        if (!xrdb.useFileLists)
        {
            XtUnmanageChild (XmFileSelectionBoxGetChild(fileIODialog,
                                                        XmDIALOG_APPLY_BUTTON));
        }
        else
        {
            n = 0;
            tmpXmStr = GETXMSTR(2,18, "Update");
            XtSetArg (args[n], XmNapplyLabelString, tmpXmStr);         n++;
            XtSetValues (fileIODialog, args, n);
            XmStringFree(tmpXmStr);
        }
    }
    if (!xrdb.useFileLists)
    {
        /* remove label and scrolled lists for directories and files */
        wid = XmFileSelectionBoxGetChild(fileIODialog, XmDIALOG_DIR_LIST_LABEL);
        XtUnmanageChild (wid);
        wid = XmFileSelectionBoxGetChild(fileIODialog, XmDIALOG_LIST_LABEL);
        XtUnmanageChild (wid);
        wid = XmFileSelectionBoxGetChild(fileIODialog, XmDIALOG_DIR_LIST);
        XtUnmanageChild (XtParent(wid));
        wid = XmFileSelectionBoxGetChild(fileIODialog, XmDIALOG_LIST);
        XtUnmanageChild (XtParent(wid));
    }

    return ( fileIODialog );
}

/*******************************************************************************
        END OF FILE
*******************************************************************************/

