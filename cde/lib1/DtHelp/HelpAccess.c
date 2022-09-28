/* $XConsortium: HelpAccess.c /main/cde1_maint/1 1995/07/17 17:35:07 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        HelpAccess.c
 **
 **   Project:     Cache Creek (Rivers) Project
 **
 **   Description: This module handles all the help requests within a given
 **                help dialog widget.
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#include <stdio.h>
#include <signal.h>

#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include <Xm/RowColumnP.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>

/* Help Library  Includes */
#include "Canvas.h"
#include "Access.h"
#include "bufioI.h"
#include "AccessI.h"

#include "HelpDialogI.h"
#include "HelpDialogP.h"

/* Display Area Includes */
#include <Dt/Help.h>
#include "HelpI.h"
#include "HelposI.h"
#include "HelpUtilI.h"
#include "DisplayAreaP.h"
#include "HelpQuickD.h"
#include "HelpQuickDP.h"
#include "HelpQuickDI.h"



/********    Static Function Declarations    ********/
#ifdef _NO_PROTO
static void CloseOnHelpCB();
static void SetupHelpDialog();

#else
static void  CloseOnHelpCB(
    Widget w,
    XtPointer client_data,
    XtPointer call_data);
static void SetupHelpDialog(
    Widget parent,
    char *helpOnHelpVolume,
    Widget *helpWidget,
    char *locationId);
#endif /* _NO_PROTO */

/********    End Static Function Declarations    ********/




/*****************************************************************************
 * Function:	   static void CloseOnHelpCB(
 *                            Widget w,   
 *                            XtPointer client_data,
 *                            XtPointer call_data);
 *       
 * Parameters:   
 *
 * Return Value:    
 *
 * Purpose: 	   This routine closes the onhelp dialog Widget that we 
 *                 created for our help on help stuff.
 *
 ****************************************************************************/
#ifdef _NO_PROTO
static void CloseOnHelpCB( w, client_data, call_data )
    Widget w ;
    XtPointer client_data ;
    XtPointer call_data ;
#else
static void  CloseOnHelpCB(
    Widget w,
    XtPointer client_data,
    XtPointer call_data )
#endif /* _NO_PROTO */
{

  XtUnmanageChild(w);


}



/****************************************************************************
 * Function:          void  _DtHelpCB ()
 * 
 * Parameters:           widget
 *                       clientData
 *                       callData
 *   
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Called when any "F1" key is pressed in a help dialog 
 *                  widget, or a user selects any of the help buttons in 
 *                  any of the help dialogs dialogs.
 *
 ***************************************************************************/
#ifdef _NO_PROTO
void  _DtHelpCB (widget, clientData, callData) 
    Widget	widget;
    XtPointer	clientData;
    XtPointer   callData;
#else
void  _DtHelpCB(
    Widget widget,
    XtPointer clientData,
    XtPointer callData)
#endif /* _NO_PROTO */

{
  char *errorStr;
  
  DtHelpListStruct * pHelpInfo = (DtHelpListStruct *) clientData;
  _DtHelpCommonHelpStuff * help = pHelpInfo->help;

  /* If helpOnHelp resource is set use it, else post an error dialog */
  if (help->helpOnHelpVolume != NULL)
    {
       SetupHelpDialog((Widget)pHelpInfo->widget, help->helpOnHelpVolume,
                     &(help->onHelpDialog), pHelpInfo->locationId);
    }
  else
    {
       /* We have no help available for the help stuff so generate an error */
       /* Do NOT EVER provide a help button */
       /* ??? ERROR */
       errorStr = XtNewString(((char *) _DTGETMESSAGE(6, 4,
                          "Help On Help not provided for this application!")));
       _DtHelpErrorDialog(XtParent(pHelpInfo->widget), errorStr);
       XtFree(errorStr);
    }

}




/*****************************************************************************
 * Function:	    void SetupHelpDialog (); 
 *
 *
 * Parameters:      
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Creates or re-uses a help on help quick help widget.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
static void SetupHelpDialog(parent, helpOnHelpVolume, helpDialog, locationId)
    Widget parent;
    char * helpOnHelpVolume;
    Widget *helpDialog;
    char *locationId;
#else
static void SetupHelpDialog(
    Widget parent,
    char *helpOnHelpVolume,
    Widget *helpDialog,
    char *locationId)
#endif 
{
  Arg  args[10];
  int    n;
  XmString printString;
  XmString closeString;
  XmString backString;
  XmString helpString;
  char *title;


      if (*helpDialog ==  NULL) 
        {
          /* Create the QuickHelpDialog widget for help on help */
          title = XtNewString(((char *)_DTGETMESSAGE(6, 1, "Help On Help")));
          printString = XmStringCreateLocalized((char *)_DTGETMESSAGE(6, 2,"Print ..."));
          closeString = XmStringCreateLocalized((char *)_DTGETMESSAGE(6, 3,"Close"));
          backString = XmStringCreateLocalized((char *)_DTGETMESSAGE(6, 5,"Backtrack"));
          helpString = XmStringCreateLocalized((char *)_DTGETMESSAGE (6, 6,"Help ..."));

          n =0;
          XtSetArg (args[n], DtNhelpType, DtHELP_TYPE_TOPIC);   n++; 
          XtSetArg (args[n], XmNtitle, title);                  n++;
          XtSetArg (args[n], DtNprintLabelString, printString); n++;
          XtSetArg (args[n], DtNcloseLabelString, closeString); n++;
          XtSetArg (args[n], DtNbackLabelString, backString);   n++;
          XtSetArg (args[n], DtNhelpLabelString, helpString);   n++;
          XtSetArg (args[n], DtNhelpVolume,helpOnHelpVolume);   n++; 
          XtSetArg (args[n], DtNlocationId, locationId);        n++;
          *helpDialog = DtCreateHelpQuickDialog((Widget)parent, 
                                                "onHelpDialog", args, n);
          XmStringFree(printString);
          XmStringFree(closeString);
          XmStringFree(backString);
          XmStringFree(helpString);
          XtFree((char*) title);

          /* Catch the close callback so we can destroy the widget */
          XtAddCallback(*helpDialog, DtNcloseCallback,
                      CloseOnHelpCB, (XtPointer) NULL);

          /* Add the popup position callback to our history dialog */
          XtAddCallback (XtParent(*helpDialog), XmNpopupCallback,
                        (XtCallbackProc)_DtHelpMapCB, 
                        (XtPointer)XtParent(parent));

 
        }
      else
        {
           n = 0;
           XtSetArg (args[n], DtNhelpType, DtHELP_TYPE_TOPIC);  n++; 
           XtSetArg (args[n], DtNhelpVolume,helpOnHelpVolume);   n++; 
           XtSetArg (args[n], DtNlocationId, locationId);        n++;
           XtSetValues(*helpDialog, args, n);
  
         }

      /* Display the dialog */
      XtManageChild(*helpDialog);     
      XtMapWidget(XtParent((Widget)*helpDialog));
}



/*****************************************************************************
 * Function:	    void _DtHelpListFree (DtHelpListStruct * * pHead);  
 *
 *
 * Parameters:      pHead   Specifies the head pointer to the help list.
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Frees all elements in a Help List.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
void _DtHelpListFree(pHead)
    DtHelpListStruct * * pHead;
#else
void _DtHelpListFree(
    DtHelpListStruct * * pHead)
#endif 
{
  DtHelpListStruct *pTemp = NULL;
  DtHelpListStruct *pTmpHead = *pHead;

   while (pTmpHead != NULL)
     {
       /* Free up each element in the current node */
       pTemp            = pTmpHead;
       pTmpHead         = pTmpHead->pNext;
       pTemp->pNext     = NULL;
      
       /* Free the id String */
       XtFree(pTemp->locationId);
      
       /* Now, free the whole node */
       XtFree((char *)pTemp);
    }
    *pHead = NULL;   /* reset pointer */
}










/*****************************************************************************
 * Function:	    DtHelpListStruct * _DtHelpListAdd(
 *                                       char *locationId,
 *                                       Widget new,
 *                                       DtHelpListStruct **pHead)
 *    
 *                            
 *
 * Parameters:  
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Adds an element to the top of the given topicList.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
DtHelpListStruct * _DtHelpListAdd(locationId, widget, help, pHead)
    char *locationId;
    Widget widget;
    _DtHelpCommonHelpStuff * help;
    DtHelpListStruct **pHead;
#else
DtHelpListStruct * _DtHelpListAdd(
    char *locationId,
    Widget widget,
    _DtHelpCommonHelpStuff * help,
    DtHelpListStruct **pHead)
#endif 
{
  DtHelpListStruct *pTemp=NULL;

  /* Add locationId as first element if pHead = NULL */
  if (*pHead == NULL)
    {
      *pHead = (DtHelpListStruct *) XtMalloc((sizeof(DtHelpListStruct)));
      
      /* Assign the passed in values to our first element */
      (*pHead)->locationId      = XtNewString(locationId);
      (*pHead)->widget          = widget;
      (*pHead)->help            = help;
      (*pHead)->pNext           = NULL;
      (*pHead)->pPrevious       = NULL;
       
    }
  else 
    {  /* We have a list so add the new Help item to the top */
     pTemp = (DtHelpListStruct *) XtMalloc((sizeof(DtHelpListStruct)));
    
     pTemp->pNext            = (*pHead);
     pTemp->pPrevious        = NULL;

     (*pHead)->pPrevious     = pTemp;

     pTemp->locationId       = XtNewString(locationId);
     pTemp->widget           = widget;
     pTemp->help             = help;

     /* Re-Assign our head pointer to point to the new head of the list */
     (*pHead) = pTemp;
    }

  return (*pHead);
}

