/* $XConsortium: ErrorDialog.c /main/cde1_maint/1 1995/07/17 20:25:25 drk $ */

/*******************************************************************************
        ErrorDialog.c

       Associated Header file: ErrorDialog.h
*******************************************************************************/

#include <stdio.h>

#include <Xm/Xm.h>
#include "UxXt.h"
#include "dtcreate.h"

#define CONTEXT_MACRO_ACCESS 1
#include "ErrorDialog.h"
#undef CONTEXT_MACRO_ACCESS

/******************************************************************************/
/* activateCB_ErrorDialog_OkButton                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void    activateCB_ErrorDialog_OkButton( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
void    activateCB_ErrorDialog_OkButton( Widget  UxWidget,
                                        XtPointer UxClientData,
                                        XtPointer UxCallbackArg)
#endif /* _NO_PROTO */

{
  XtDestroyWidget((Widget)UxClientData);
}

/******************************************************************************/
/* display_error_message - display desired message in error dialog            */
/* INPUT:  char *message - message to display in dialog box                   */
/* OUTPIT: none                                                               */
/******************************************************************************/
#ifdef _NO_PROTO
void display_error_message (parent, message)
     Widget parent;
     char   *message;
#else /* _NO_PROTO */
void display_error_message (Widget parent, char *message)
#endif /* _NO_PROTO */
{

  Widget ErrorDialog = XmCreateErrorDialog(parent, "errorDialog", NULL, 0);
  XtUnmanageChild (XmMessageBoxGetChild (ErrorDialog, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (ErrorDialog, XmDIALOG_HELP_BUTTON));

  XtAddCallback( ErrorDialog, XmNokCallback,
          (XtCallbackProc) activateCB_ErrorDialog_OkButton,
          ErrorDialog );

  XtVaSetValues (ErrorDialog,
                 RES_CONVERT(XmNdialogTitle, GETMESSAGE(6, 31, "Create Action - Error")),
                 RES_CONVERT(XmNmessageString, message),
                 NULL);

  XtManageChild (ErrorDialog);
  return;
}
/*******************************************************************************
       END OF FILE
*******************************************************************************/

