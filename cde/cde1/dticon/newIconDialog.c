/* $XConsortium: newIconDialog.c /main/cde1_maint/3 1995/10/23 08:59:37 gtsang $ */
/*********************************************************************
*  (c) Copyright 1993, 1994 Hewlett-Packard Company
*  (c) Copyright 1993, 1994 International Business Machines Corp.
*  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
*  (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
*      Novell, Inc.
**********************************************************************/
/*******************************************************************************
    newIconDialog.c

*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include <Xm/MwmUtil.h>

#include <Xm/TextF.h>
#include <Xm/LabelG.h>
#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>

#include <Dt/DialogBox.h>

#include "main.h"
#include "externals.h"

/*******************************************************************************
    Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

static  Widget  newSeparator;
static  Widget  newWidthLabel;
static  Widget  newHeightLabel;

Widget  newIconDialog;
Widget  newWidthText;
Widget  newHeightText;

extern Widget  editMenu_deleteHS_pb;
/*******************************************************************************
    Forward declarations of functions that are defined later in this file.
*******************************************************************************/

Widget  create_newIconDialog();

/*******************************************************************************
    The following are callback functions.
*******************************************************************************/

/************************************************************************
 *   ButtonCB()
 *
 ************************************************************************/
static void
#ifdef _NO_PROTO
ButtonCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
ButtonCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    DtDialogBoxCallbackStruct *cb = (DtDialogBoxCallbackStruct *)call_data;

    switch (cb->button_position)
    {
      case 1:                    /* OK button */
             Process_Size_OK();
          break;

      case 2:                    /* CANCEL button */
          Process_Size_Cancel();
          break;

      case 3:                    /* HELP button */
          XtCallCallbacks(w, XmNhelpCallback, NULL);
          break;

      default:
          break;
    }
}


/*******************************************************************************
    The 'build_' function creates all the widgets
    using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget
#ifdef _NO_PROTO
build_newIconDialog()
#else
build_newIconDialog( void )
#endif /* _NO_PROTO */
{
    XmString    tmpXmStrings[3];
    Widget      workForm;
    Arg         args[10];
    int         n;


    /* create the dialog box with shell */

    tmpXmStrings[0] = GETXMSTR(4,6, "OK");
    tmpXmStrings[1] = GETXMSTR(4,8, "Cancel");
    tmpXmStrings[2] = GETXMSTR(4,10, "Help");

    n = 0;
    XtSetArg (args[n], XmNbuttonCount, 3);                                 n++;
    XtSetArg (args[n], XmNbuttonLabelStrings, tmpXmStrings);               n++;
    newIconDialog = __DtCreateDialogBoxDialog (mainWindow, "newIconDialog",
                                              args, n);

    /* free compound strings now */
    XmStringFree (tmpXmStrings[0]);
    XmStringFree (tmpXmStrings[1]);
    XmStringFree (tmpXmStrings[2]);

    XtAddCallback(newIconDialog, XmNcallback, ButtonCB, NULL);

    n = 0;
    workForm = _DtDialogBoxGetButton(newIconDialog, 2);
    XtSetArg(args[n], XmNcancelButton, workForm);                          n++;
    XtSetValues (newIconDialog, args, n);

    n = 0;
    XtSetArg (args[n], XmNtitle, GETSTR(4,4, "Icon Editor - Resize"));     n++;
    XtSetArg (args[n], XmNuseAsyncGeometry, True);                         n++;
    XtSetArg (args[n], XmNmwmInputMode,MWM_INPUT_PRIMARY_APPLICATION_MODAL);n++;
    XtSetValues (XtParent(newIconDialog), args, n);


    /* create form with width/height to go in dialog box as work area */

    workForm = XtVaCreateManagedWidget( "newIconForm",
                xmFormWidgetClass, newIconDialog,
                XmNchildType, XmWORK_AREA,
                XmNallowOverlap, False,
                NULL );

    newWidthText = XtVaCreateManagedWidget( "newWidthText",
                xmTextFieldWidgetClass, workForm,
                XmNtopOffset, 5,
                XmNtopAttachment, XmATTACH_FORM,
                XmNrightOffset, 5,
                XmNrightAttachment, XmATTACH_FORM,
                NULL );

    tmpXmStrings[0] = GETXMSTR(4,12, "New Width:");
    newWidthLabel = XtVaCreateManagedWidget( "newWidthLabel",
                xmLabelGadgetClass, workForm,
                XmNalignment, XmALIGNMENT_END,
                XmNlabelString, tmpXmStrings[0],
                XmNtopOffset, 5,
                XmNtopAttachment, XmATTACH_FORM,
                XmNrightWidget, newWidthText,
                XmNrightAttachment, XmATTACH_WIDGET,
                XmNleftOffset, 5,
                XmNleftAttachment, XmATTACH_FORM,
                NULL );
    XmStringFree(tmpXmStrings[0]);

    newHeightText = XtVaCreateManagedWidget( "newHeightText",
                xmTextFieldWidgetClass, workForm,
                XmNtopWidget, newWidthText,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNrightOffset, 5,
                XmNrightAttachment, XmATTACH_FORM,
                XmNbottomOffset, 5,
                XmNbottomAttachment, XmATTACH_FORM,
                NULL );

    tmpXmStrings[0] = GETXMSTR(4,14, "New Height:");
    newHeightLabel = XtVaCreateManagedWidget( "newHeightLabel",
                xmLabelGadgetClass, workForm,
                XmNalignment, XmALIGNMENT_END,
                XmNlabelString, tmpXmStrings[0],
                XmNtopWidget, newWidthLabel,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNrightWidget, newHeightText,
                XmNrightAttachment, XmATTACH_WIDGET,
                XmNbottomOffset, 5,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNleftOffset, 5,
                XmNleftAttachment, XmATTACH_FORM,
                NULL );
    XmStringFree(tmpXmStrings[0]);

    return ( newIconDialog );
}

/*******************************************************************************
    The following is the 'Interface function' which is the
    external entry point for creating this interface.
    This function should be called from your application or from
    a callback function.
*******************************************************************************/

Widget
#ifdef _NO_PROTO
create_newIconDialog()
#else
create_newIconDialog( void )
#endif /* _NO_PROTO */
{
    Widget                  rtrn;

    rtrn = build_newIconDialog();

    return(rtrn);
}


/*******************************************************************************
    END OF FILE
*******************************************************************************/

