/* $XConsortium: SharedProcs.c /main/cde1_maint/2 1995/09/06 02:17:28 lehors $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **   File:        SharedProcs.c
 **
 **   Project:	   DT
 **
 **   Description: Contains the set of functions which are of general
 **                use to all DT clients.
 **		  
 **
 ****************************************************************************
 ************************************<+>*************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/VendorSEP.h>
#include <Xm/MessageB.h>
#include <Xm/RowColumn.h>
#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>
#include <X11/ShellP.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>


#include <Dt/DtP.h>
#include <Dt/Connect.h>
#include <Dt/DtNlUtils.h>

#include "SharedProcs.h"


/* Defines */
#define RW_ALL S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH

/* Global controlling whether error dialogs are displayed */
Boolean messageDisplayEnabled = True;

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO


#else


#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


/************************************************************************
 *
 *  _DtStripSpaces
 *
 *     Strip all leading and trailing spaces.
 *
 ************************************************************************/

String
#ifdef _NO_PROTO
_DtStripSpaces( string )
        String string ;
#else
_DtStripSpaces(
        String string )
#endif /* _NO_PROTO */

{
   int i;
   String space;

   if (string == NULL)
      return (string);


   /* Strip off leading spaces first */
   i = 0;
   while (
#ifdef NLS16
          (!is_multibyte || (mblen(string + i, MB_CUR_MAX) == 1)) &&
#endif
          isspace((u_char)string[i]))
   {
      i++;
   }

   /* Copy over the leading spaces */
   strcpy(string, string + i);

   /* Drop out, if the string is now empty */
   if ((i = strlen(string) - 1) < 0)
      return(string);

   /* Strip off trailing spaces */
#ifdef NLS16
   if (!is_multibyte)
   {
#endif
      /* No multibyte; simply work back through the string */
      while ((i >= 0) && (isspace((u_char)string[i])))
         i--;
      string[i + 1] = '\0';
#ifdef NLS16
   }
   else
   {
      /* Work forward, looking for a trailing space of spaces */
      int len;

      i = 0;
      space = NULL;

      while (string[i])
      {
         if (((len = mblen(string + i, MB_CUR_MAX)) == 1) && isspace((u_char)string[i]))
         {
            /* Found a space */
            if (space == NULL)
               space = string + i;
         }
         else if (space)
            space = NULL;

         /* if there is an invalid character, treat as a valid one-byte */
         if (len == -1)
           len = 1;
         
         i += len;
  
      }

      if (space)
         *space = '\0';
   }
#endif

   return (string);
}

/************************************************************************
 *
 *  _DtMessage
 *	Create and display an error message.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
_DtMessage( w, title, message_text, helpIdStr, helpCallback )
        Widget w ;
        char *title ;
        char *message_text ;
        XtPointer helpIdStr ;
        void (*helpCallback)() ;
#else
_DtMessage(
        Widget w,
        char *title,
        char *message_text,
        XtPointer helpIdStr,
        void (*helpCallback)() )
#endif /* _NO_PROTO */

{
   _DtMessageDialog(w, title, message_text, helpIdStr, False,
                 NULL, _DtMessageOK, _DtMessageClose, helpCallback, True, 
                 ERROR_DIALOG);
}


/************************************************************************
 *
 * Generic warning/error dialog creation function.
 *
 ************************************************************************/

Widget
#ifdef _NO_PROTO
_DtMessageDialog( w, title, message_text, helpIdStr, cancel_btn, cancel_callback, 
               ok_callback, close_callback, help_callback, deleteOnClose, 
               dialogType )
        Widget w ;
        char *title ;
        char *message_text ;
        XtPointer helpIdStr ;
        Boolean cancel_btn ;
        void (*cancel_callback)() ;
        void (*ok_callback)() ;
        void (*close_callback)() ;
        void (*help_callback)() ;
        Boolean deleteOnClose ;
        int dialogType ;
#else
_DtMessageDialog(
        Widget w,
        char *title,
        char *message_text,
        XtPointer helpIdStr,
        Boolean cancel_btn,
        void (*cancel_callback)(),
        void (*ok_callback)(),
        void (*close_callback)(),
        void (*help_callback)(),
        Boolean deleteOnClose,
        int dialogType )
#endif /* _NO_PROTO */

{
   Widget message;
   Widget widget;
   XmString message_string;
   XWindowAttributes attributes;
   Arg args[10];
   XmString okString, cancelString, helpString;

   if (messageDisplayEnabled == False)
      return(NULL);

   okString = XmStringCreateLtoR((Dt11GETMESSAGE(28,1, "OK")), 
                                 XmFONTLIST_DEFAULT_TAG);
   cancelString = XmStringCreateLtoR((Dt11GETMESSAGE(28,2, "Cancel")), 
                                 XmFONTLIST_DEFAULT_TAG);
   helpString = XmStringCreateLtoR((Dt11GETMESSAGE(28,3, "Help")), 
                                 XmFONTLIST_DEFAULT_TAG);

   XtSetArg (args[0], XmNautoUnmanage, False);
   XtSetArg (args[1], XmNcancelLabelString, cancelString);
   XtSetArg (args[2], XmNokLabelString, okString);
   XtSetArg (args[3], XmNhelpLabelString, helpString);
   XtSetArg (args[4], XmNuseAsyncGeometry, True);

   /*  Search up to get the topmost shell  */

   while (XtParent (w) != NULL && !(XtIsSubclass (w, shellWidgetClass)))
      w = XtParent (w);

   switch (dialogType)
   {  
      case ERROR_DIALOG:
        message = XmCreateErrorDialog(w, title, args, 5);
      break;

      case WARNING_DIALOG:
        message = XmCreateWarningDialog(w, title, args, 5);
      break;

      case QUESTION_DIALOG:
        message = XmCreateQuestionDialog(w, title, args, 5);
      break;
    }
   if (XtWindow (w) != 0)
      XGetWindowAttributes(XtDisplay (w), XtWindow (w), &attributes);
   else
      attributes.map_state = IsUnmapped;

   if (attributes.map_state == IsUnmapped)
   {
      XtSetArg(args[0], XmNx, (WidthOfScreen(XtScreen (w)) - 350) / 2);
      XtSetArg(args[1], XmNy, (HeightOfScreen(XtScreen (w)) - 200) / 2);
      XtSetArg(args[2], XmNdefaultPosition, False);
      XtSetValues(message, args, 3);
   }


   /*  Adjust the decorations and title for the dialog shell of the dialog  */

   XtSetArg(args[0], XmNtitle, title);
   XtSetArg(args[1], XmNmwmFunctions, MWM_FUNC_MOVE);
   XtSetArg(args[2], XmNmwmDecorations, MWM_DECOR_BORDER | MWM_DECOR_TITLE);
   XtSetValues(XtParent (message), args, 3);

   widget = XmMessageBoxGetChild(message, XmDIALOG_CANCEL_BUTTON);
   if (!cancel_btn)
      XtUnmanageChild(widget);
   else if (cancel_callback)
   {
      XtAddCallback(widget, XmNactivateCallback, cancel_callback, 
                     (caddr_t) message);
      XtSetArg(args[0], XmNcancelButton, widget);
      XtSetValues(message, args, 1);
   }

   widget = XmMessageBoxGetChild(message, XmDIALOG_OK_BUTTON);
   XtSetArg(args[0], XmNmarginWidth, 10);
   XtSetArg(args[1], XmNmarginHeight, 2);
   XtSetValues(widget, args, 2);
   XtAddCallback(widget, XmNactivateCallback, ok_callback, (caddr_t) message);
  
   widget = XmMessageBoxGetChild(message, XmDIALOG_HELP_BUTTON);
   if (helpIdStr != NULL)
   {
      if (help_callback)
      {
         XtAddCallback(widget, XmNactivateCallback, help_callback, 
                       (caddr_t) helpIdStr);
      }
      XtSetValues(widget, args, 2);
   }
   else
      XtUnmanageChild(widget);

   widget = XmMessageBoxGetChild(message, XmDIALOG_MESSAGE_LABEL);
   message_string = XmStringLtoRCreate(message_text, XmFONTLIST_DEFAULT_TAG);
   XtSetArg(args[0], XmNlabelString, message_string);
   XtSetValues(widget, args, 1);
   XmStringFree(message_string);

   XtManageChild(message);

   if (deleteOnClose)
   {
      if (close_callback == NULL)
         close_callback = _DtMessageClose;

      XtAddEventHandler(XtParent (message), StructureNotifyMask, True,
                         (XtEventHandler)close_callback, message);
   }

   XmStringFree(okString);
   XmStringFree(cancelString);
   XmStringFree(helpString);

   return(message);
}




/************************************************************************
 *
 *  _DtMessageOK
 *	Close the error message box.
 *
 ************************************************************************/ 

/* ARGSUSED */
void
#ifdef _NO_PROTO
_DtMessageOK( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
_DtMessageOK(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */

{
   XtUnmanageChild(client_data);
}




/************************************************************************
 *
 *  _DtMessageClose
 *	Close the error message box.
 *
 ************************************************************************/ 

void
#ifdef _NO_PROTO
_DtMessageClose( w, client_data, event )
        Widget w ;
        XtPointer client_data ;
        XEvent *event ;
#else
_DtMessageClose(
        Widget w,
        XtPointer client_data,
        XEvent *event )
#endif /* _NO_PROTO */

{
   if (event->type == UnmapNotify)
   {
      XtRemoveEventHandler(XtParent(client_data), StructureNotifyMask, 
                            True, (XtEventHandler)_DtMessageClose, client_data);

      XtUnmanageChild(client_data);
      XtDestroyWidget(w);
   }
}


