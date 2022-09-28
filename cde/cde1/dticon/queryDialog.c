/* $XConsortium: queryDialog.c /main/cde1_maint/2 1995/10/23 09:00:22 gtsang $ */
/*********************************************************************
*  (c) Copyright 1993, 1994 Hewlett-Packard Company
*  (c) Copyright 1993, 1994 International Business Machines Corp.
*  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
*  (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
*      Novell, Inc.
**********************************************************************/
/*******************************************************************************
        queryDialog.c

*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include <Xm/MwmUtil.h>
#include <Xm/MessageB.h>
#include "main.h"
#include "externals.h"

#define    RES_CONVERT( res_name, res_value) \
    XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1

/*******************************************************************************
        Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

Widget  queryDialog;

/*******************************************************************************
        Forward declarations of functions that are defined later in this file.
*******************************************************************************/

Widget  create_queryDialog();

/*******************************************************************************
        The following are callback functions.
*******************************************************************************/

static void
#ifdef _NO_PROTO
okCallback_queryDialog( w, clientData, callbackArg )
        Widget w ;
        XtPointer clientData ;
        XtPointer callbackArg ;
#else
okCallback_queryDialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackArg )
#endif /* _NO_PROTO */
{
        Process_Query_OK();
}

static void
#ifdef _NO_PROTO
cancelCB_queryDialog( w, clientData, callbackArg )
        Widget w ;
        XtPointer clientData ;
        XtPointer callbackArg ;
#else
cancelCB_queryDialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackArg )
#endif /* _NO_PROTO */
{
        Process_Query_Cancel();
}

/*******************************************************************************
        The 'build_' function creates all the widgets
        using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget
#ifdef _NO_PROTO
build_queryDialog()
#else
build_queryDialog( void )
#endif /* _NO_PROTO */
{
    Widget      queryDialog_shell;
    XmString tmpXmStr, tmpXmStr2, tmpXmStr3, tmpXmStr4, tmpXmStr5;
    Arg  args[10];
    int  n;


    queryDialog_shell = XtVaCreatePopupShell( "queryDialog_shell",
                        xmDialogShellWidgetClass, mainWindow,
                        XmNtitle, GETSTR(6,2, "Icon Editor - Warning"),
                        NULL );

    tmpXmStr = GETXMSTR(6,2, "Icon Editor - Warning");
    tmpXmStr2= GETXMSTR(6,6, "OK");
    tmpXmStr3= GETXMSTR(6,8, "Do ya really wanna?");
    tmpXmStr4= GETXMSTR(6,10, "Cancel");
    tmpXmStr5= GETXMSTR(4,10, "Help");
    queryDialog = XtVaCreateWidget( "queryDialog",
                        xmMessageBoxWidgetClass, queryDialog_shell,
                        XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON,
                        XmNdialogTitle, tmpXmStr,
                        XmNokLabelString, tmpXmStr2,
                        XmNmessageString, tmpXmStr3,
                        XmNcancelLabelString, tmpXmStr4,
                        XmNhelpLabelString, tmpXmStr5,
                        XmNdialogType, XmDIALOG_WARNING,
                        NULL );
    XmStringFree(tmpXmStr);
    XmStringFree(tmpXmStr2);
    XmStringFree(tmpXmStr3);
    XmStringFree(tmpXmStr4);
    XmStringFree(tmpXmStr5);

    XtAddCallback( queryDialog, XmNokCallback, okCallback_queryDialog, NULL);
    XtAddCallback( queryDialog, XmNcancelCallback, cancelCB_queryDialog, NULL);

    n = 0;
    XtSetArg (args[n], XmNuseAsyncGeometry, True);                          n++;
    XtSetArg (args[n], XmNmwmInputMode,MWM_INPUT_PRIMARY_APPLICATION_MODAL);n++;
    XtSetValues (queryDialog_shell, args, n);


    return ( queryDialog );
}

/*******************************************************************************
        The following is the 'Interface function' which is the
        external entry point for creating this interface.
        This function should be called from your application or from
        a callback function.
*******************************************************************************/

Widget
#ifdef _NO_PROTO
create_queryDialog()
#else
create_queryDialog( void )
#endif /* _NO_PROTO */
{
        Widget                  rtrn;

        rtrn = build_queryDialog();

        return(rtrn);
}

/*******************************************************************************
        END OF FILE
*******************************************************************************/

