#ifdef VERBOSE_REV_INFO
static char rcsid[] = "$Header: WmPanel.c,v 1.45 94/05/05 16:24:49 arg_sun Exp $";
#endif /* VERBOSE_REV_INFO */
/******************************<+>*************************************
 **********************************************************************
 **
 **  File:        WmPanel.c
 **
 **  Project:     HP/Motif Workspace Manager (dtwm)
 **
 **  Description:
 **  -----------
 **  This file contains functions that control the creation and
 **  management of the Front Panel.
 **
 *********************************************************************
 **
 ** (c) Copyright 1992, 1993 HEWLETT-PACKARD COMPANY
 ** ALL RIGHTS RESERVED
 **
 **********************************************************************
 **********************************************************************
 **
 **
 **********************************************************************
 ******************************<+>*************************************/


/*-------------------------------------------------------------
**      Include Files
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "WmParse.h"
#include "WmParseP.h"
#include <Dt/GetDispRes.h>
#include <Dt/Wsm.h>
#include <Dt/WsmP.h>
#include <Dt/FileDrop.h>

#include <Xm/ColorObj.h>
#include <Xm/DialogS.h>
#include <Xm/Command.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/DrawingA.h>

#include "Button.h"
#include "Clock.h"
#include <Dt/Control.h>
#include <Dt/ControlP.h>
#include <Dt/Icon.h>
#include <Dt/IconFile.h>
#include "PanelS.h"
#include "PanelSP.h"
#include <Dt/View.h>
#include <Dt/ViewP.h>

#include "WmGlobal.h"
#include "WmPanelP.h"
#include "WmManage.h"
#include "WmWrkspace.h"
#include "WmFeedback.h"
#include "WmResNames.h"
#include "WmHelp.h"
#include "WmError.h"
#include "WmFunction.h"
#include "WmIBitmap.h"


/*-------------------------------------------------------------
**      Public Interface
**-------------------------------------------------------------
*/

WidgetClass     wmPanelistObjectClass;

Widget          WmPanelistCreate ();
void            WmPanelistPrint ();



/*-------------------------------------------------------------
**      Forward Declarations
*/

#define PANEL_HELP      "Help for this item is not currently available."

#define M_PopupList(w)          (w -> core.popup_list)
#define M_NumPopups(w)          (w -> core.num_popups)
#define M_Children(m)           (m -> composite.children)
#define M_NumChildren(m)        (m -> composite.num_children)
#define G_Height(r)             (r -> rectangle.height)
#define M_Background(w)         (w -> core.background_pixel)
#define G_Width(r)              (r -> rectangle.width)
#define G_X(r)                  (r -> rectangle.x)
#define G_Y(r)                  (r -> rectangle.y)
#define G_Expose(r,e,reg) \
 (((RectObjClassRec *)r -> object.widget_class) -> rect_class.expose)(r,e,reg)

/*      Private Functions
*/
#ifdef _NO_PROTO

static void     InvokeAction() ;

static void     DropCB ();
static void     ControlCB (); 
static void     ArrowCB (); 
static void     SwitchRenameLabel ();
static void     SwitchRenameCB ();
static void     SwitchRenameErrorHelpCB();
static XtActionProc SwitchRenameCancel ();
static void     SubpanelUnmapCB (); 
static void     SubpanelTornEventHandler ();
static void     ArrowSetBehavior ();
static Widget   HandleCreate ();
static void     HandlInputCB (); 
static void     SetDropZones ();

static String   GetImageName ();

static Widget   AllocatePanel ();
static Widget   AllocateSubpanel ();
static Widget   AllocateBox ();
static Widget   AllocateControl ();
static Widget   AllocateArrow ();
static Widget   AllocateSwitch ();

static void     Initialize ();

#else /* _NO_PROTO */

static void     InvokeAction (WmPanelistObject, Widget, String, int, String*,
                                String, String, String, Boolean);

static void     DropCB (Widget, XtPointer, XtPointer);
static void     ControlCB (Widget, XtPointer, XtPointer); 
static void     ArrowCB (Widget, XtPointer, XtPointer); 
static void     SwitchRenameCB (Widget, XtPointer, XtPointer); 
static void     SwitchRenameErrorHelpCB (Widget, XtPointer, XtPointer);
static void     SwitchRenameLabel (Widget, WmPanelistObject);
static XtActionProc SwitchRenameCancel (Widget, XEvent *, XtPointer, XtPointer);
static void     SubpanelUnmapCB (Widget, XtPointer, XtPointer); 
static void     SubpanelTornEventHandler (Widget, XtPointer, XEvent *, Boolean *);
static void     ArrowSetBehavior (WmPanelistObject, Widget, String, Boolean, Boolean);
static Widget   HandleCreate (Widget, WmPanelistObject);
static void     HandlInputCB (Widget, XtPointer, XtPointer); 

static void     SetDropZones (Widget);

static String   GetImageName (WmPanelistObject, String);

static Widget   AllocatePanel (WmPanelistObject, WmFpItem, Widget);
static Widget   AllocateSubpanel (WmPanelistObject, WmFpItem, Widget);
static Widget   AllocateBox (WmPanelistObject, WmFpItem, Widget);
static Widget   AllocateSwitch (WmPanelistObject, WmFpItem, Widget, Widget,
                                Pixmap, PixelSet*);
static Widget   AllocateControl (WmPanelistObject, WmFpItem, Widget, Widget,
                                WmFpToken, PixelSet*);
static Widget   AllocateArrow (WmPanelistObject, WmFpItem, Widget, Widget,
                                Widget, WmFpToken, PixelSet*);
static void     Initialize (Widget, Widget);


#endif /* _NO_PROTO */


#ifdef _NO_PROTO
extern void ChangeWorkspaceTitle ();
#else
extern void ChangeWorkspaceTitle (WmWorkspaceData *, XmString);
#endif /* _NO_PROTO */


/*  Translations and action definitions	                        */
/*	These are used specifically for the text field overlay  */
/*	on the switch button for renaming the workspace.  They  */
/*	are necessary for handling escape key processing.       */

static char translations_escape[] = "<Key>osfCancel:Escape()";

static XtActionsRec actionTable[] = {
   {"Escape", (XtActionProc)SwitchRenameCancel},
};



/*      Warning and error messages
 */
static  char    sWarnBuf[256];
static  char    sWarnMissingItem[] =
        "Missing definition in front panel: %s";
static  char    sWarnDuplicateItem[] =
        "Invalid duplicate reference in front panel: %s";
static  char    sWarnNoRoom[] =
        "Not enough space for component in front panel: %s";
static  char    sWarnDisallowed[] =
        "Component %1$s not allowed in front panel container: %2$s";


/*-------------------------------------------------------------
**      Resource List
*/
#define P_Offset(field) \
        XtOffset (WmPanelistObject, panelist.field)
static XtResource resources[] = 
{
        {
                WmNuseDefaultGeometry,
                XmCBoolean, XmRBoolean, sizeof (Boolean),
                P_Offset (bDefaultGeo), XmRImmediate, (XtPointer) FALSE
        },
        {
                WmNuseDefaultColors,
                XmCBoolean, XmRBoolean, sizeof (Boolean),
                P_Offset (bDefaultColors), XmRImmediate, (XtPointer) FALSE
        },
        {
                WmNname,
                WmCName, XmRString, sizeof (String),
                P_Offset (pchPanelName), XmRImmediate, (caddr_t) NULL
        },
        {
                WmNglobalData,
                WmCGlobalData, XmRPointer, sizeof (XtPointer),
                P_Offset (pGlobalData), XmRImmediate, (caddr_t) NULL
        },
        {
                WmNscreenData,
                WmCScreenData, XmRPointer, sizeof (XtPointer),
                P_Offset (pScreenData), XmRImmediate, (caddr_t) NULL
        }
};
#undef  P_Offset


/*-------------------------------------------------------------
**      Class Record
*/
WmPanelistClassRec wmPanelistClassRec =
{
/*      Core Part
*/
        {       
                (WidgetClass) &objectClassRec,  /* superclass           */
                "WmPanelist",                   /* class_name           */
                sizeof (WmPanelistRec),         /* widget_size          */
                NULL,                           /* class_initialize     */
                NULL,                           /* class_part_initialize*/
                False,                          /* class_inited         */
                (XtInitProc) Initialize,        /* initialize           */
                NULL,                           /* initialize_hook      */
                NULL,                           /* unused               */
                NULL,                           /* unused               */
                0,                              /* unused               */
                resources,                      /* resources            */
                XtNumber (resources),           /* num_resources        */
                NULLQUARK,                      /* xrm_class            */
                False,                          /* unused               */
                False,                          /* unused               */
                False,                          /* unused               */
                False,                          /* unused               */      
                NULL,                           /* destroy              */      
                NULL,                           /* unused               */
                NULL,                           /* unused               */      
                NULL,                           /* set_values           */      
                NULL,                           /* set_values_hook      */
                NULL,                           /* unused               */
                NULL,                           /* get_values_hook      */
                NULL,                           /* unused               */      
                XtVersion,                      /* version              */
                NULL,                           /* callback private     */
                NULL,                           /* unused               */
                NULL,                           /* unused               */
                NULL,                           /* unused               */
                NULL,                           /* extension            */
        },

/*      WmPanelist Part
*/
        {
                NULL,                           /* extension            */
        }
};


WidgetClass wmPanelistObjectClass = (WidgetClass) &wmPanelistClassRec;



/************************************************************************
 *
 *      Private Functions
 *
 ************************************************************************/


/************************************************************************
 *
 *  InvokeAction
 *      Check for exit actions and exit, otherwise invoke the requested
 *      action.
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
InvokeAction (pPanelist, w, actionName, numObjectLists, objectListArray,
              execParms, cwdHost, cwdDir, bTrace)
WmPanelistObject pPanelist;
Widget w;
String actionName;
int numObjectLists;
String * objectListArray;
String execParms;
String cwdHost;
String cwdDir;
Boolean bTrace;

#else /* _NO_PROTO */
static void
InvokeAction (WmPanelistObject pPanelist,
              Widget           w,
              String           actionName,
              int              numObjectLists,
              String           *objectListArray,
              String           execParms,
              String           cwdHost,
              String           cwdDir,
              Boolean          bTrace)
#endif /* _NO_PROTO */

{
   if ((! strcmp (actionName, "Exit")) ||
       (! strcmp (actionName, "ExitSession")) ||
       (! strcmp (actionName, "EXIT_SESSION")))
      exit (0);
   else
      DtInvokeAction (w, actionName, numObjectLists, objectListArray,
                       execParms, cwdHost, cwdDir);
}




/************************************************************************
 *
 *  DropCB
 *      Invoke the drop action associated with the control the drop
 *      occurred on.
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
DropCB (w, client_data, call_data)
Widget w ;
XtPointer client_data ;
XtPointer call_data ;

#else  /* _NO_PROTO */
static void
DropCB (Widget       w,
        XtPointer    client_data,
	XtPointer    call_data)
#endif /* _NO_PROTO */

{
   DtControlGadget cg = (DtControlGadget) w;
   DtFileDropCallbackStruct *fileList =
			 (DtFileDropCallbackStruct *)call_data;

   if (fileList->reason == DtCR_DND_DROP_ANIMATE) {
      _DtControlDoDropAnimation ((Widget) cg);

      DtInvokeDropAction(w, G_DropAction (cg), fileList->count,
			      fileList->file_list, NULL, NULL, NULL, True);
   }

   fileList->status = DtFILE_DROP_SUCCESS;
}




/************************************************************************
 *
 *  ControlCB
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
ControlCB (w, clientData, callData)
Widget    w;
XtPointer clientData;
XtPointer callData;

#else /* _NO_PROTO */
static void
ControlCB (Widget    w,
           XtPointer clientData,
           XtPointer callData) 
#endif /* _NO_PROTO */

{
   WmPanelistObject         pPanelist = (WmPanelistObject) clientData;
   Widget                   wShell = O_Shell (pPanelist);
   DtControlCallbackStruct *cb = (DtControlCallbackStruct*) callData;
   WmWorkspaceData          *pWS;
   WmFunction               wmFunc;
   Widget                   *pWid;
   int                      iWid;
   DtControlGadget         cg = (DtControlGadget) w;
   int                      i;
   Widget                   subpanel;
   ClientData               *pCD = NULL;


   /*  See if the control action occurred out of a subpanel.  If so,     */
   /*  get the subpanel, check to see if it is set to unpost on select,  */
   /*  call the unposting function and reset the arrow behavior.         */

   subpanel = w;
   while (!XtIsShell (XtParent (subpanel)))
      subpanel = XtParent (subpanel);

   if (XtIsSubclass (XtParent(subpanel), xmDialogShellWidgetClass))
   {
      Arg           al[2];
      int           ac;
      Boolean       unpost;
      Boolean       torn;
      Widget        arrow;
      XtPointer     push_argument;
      XtPointer     push_function;


      /*  Get the subpanel unposting behavior setting  */

      ac = 0;
      XtSetArg (al[ac], XmNsubpanelUnpostOnSelect, &unpost);  ac++;
      XtSetArg (al[ac], XmNsubpanelTorn, &torn);              ac++;
      XtGetValues (subpanel, al, ac);


      if (unpost == TRUE && torn == FALSE)
      {

         /*  Get the arrow widget out of the subpanel.  */

         ac = 0;
         XtSetArg (al[ac], XmNcancelButton, &arrow);  ac++;
         XtGetValues (subpanel, al, ac);
      

         /*  Get the unposting push argument from the arrow  */

         ac = 0;
         XtSetArg (al[ac], XmNpushFunction, &push_function);  ac++;
         XtSetArg (al[ac], XmNpushArgument, &push_argument);  ac++;
         XtGetValues (arrow, al, ac);


         XFindContext (O_GlobalData (pPanelist)->display,
                       XtWindow (XtParent (subpanel)),
                       O_GlobalData (pPanelist)->windowContextType,
                       (XtPointer) &(pCD));

         wmFunc = (WmFunction) push_function;
         wmFunc (push_argument, pCD, cb->event);

         ArrowSetBehavior (pPanelist, arrow, NULL, FALSE, True);
      }
   }


   XFindContext (O_GlobalData (pPanelist)->display, XtWindow (wShell),
                 O_GlobalData (pPanelist)->windowContextType,
                 (XtPointer) &(O_ClientData (pPanelist)));


   
   switch (cb->reason)
   {
      case XmCR_BUSY_START:
         XDefineCursor (XtDisplay (w), XtWindow (wShell),
                        O_CursorBusy (pPanelist));

         for (iWid = 0, pWid = M_PopupList (wShell);
              iWid < M_NumPopups (wShell); iWid++)
            XDefineCursor (XtDisplay (w), XtWindow (pWid[iWid]),
                           O_CursorBusy (pPanelist));
      break;

      case XmCR_BUSY_STOP:
         XUndefineCursor (XtDisplay (w), XtWindow (wShell));

         for (iWid = 0, pWid = M_PopupList (wShell);
              iWid < M_NumPopups (wShell); iWid++)
            XUndefineCursor (XtDisplay (w), XtWindow (pWid[iWid]));
      break;

      case XmCR_ACTIVATE:
         if (cb->control_type == XmCONTROL_MAIL)
         {
            _DtControlSetFileChanged (w, False);

            if (O_NumRecallClients (pPanelist) > 0)
            {
               String sClient = NULL;
               WmFpPushRecallClientData *pRecallData;

               for (i = 0;
                    ((sClient == NULL) && (i < O_NumRecallClients(pPanelist)));
                    i++)
               {
                  pRecallData = &(O_RecallClients (pPanelist)[i]);

                  if (pRecallData->wControl == w)
                     sClient = pRecallData->pchResName;
               }

               if (sClient != NULL)
               {
                  DtControlGadget cg;

                  for (i = 0; i < O_NumRecallClients (pPanelist); i++)
                  {
                     pRecallData = &(O_RecallClients (pPanelist)[i]);
                     cg = (DtControlGadget) pRecallData->wControl;

                     if ((! strcmp (sClient, pRecallData->pchResName)) &&
                          (w != pRecallData->wControl) &&
                          (G_ControlType (cg) == XmCONTROL_MAIL) )
                     {
                        _DtControlSetFileChanged(pRecallData->wControl, False);
                     }
                  }
               }
            }
         }


         if (G_PushAction (cg) != NULL)
         {
            if (DtIsControl (w))
               _DtControlDoPushAnimation (w);

            InvokeAction (pPanelist, (Widget) cg, G_PushAction (cg), 0,
                          NULL, NULL, NULL, NULL, True);
         }

         if (cb->push_function != NULL)
         {
            if (DtIsControl (w))
               _DtControlDoPushAnimation (w);

            wmFunc = (WmFunction) cb->push_function;
            wmFunc (cb->push_argument, O_ClientData (pPanelist), cb->event);
         }
      break;

      case XmCR_VALUE_CHANGED:
         switch (cb->control_type)
         {
            case XmCONTROL_SWITCH:
               if (w != O_SwitchLast (pPanelist))
               {
                  _DtIconSetState (O_SwitchLast (pPanelist), False, False);
                  DtWmRequest (XtDisplay(w), XRootWindowOfScreen (XtScreen(w)),
                                G_PushAction (cg));
                  O_SwitchLast (pPanelist) = w;
               }
               else
               {
                  SwitchRenameLabel (w, pPanelist);
                  _DtIconSetState (w, True, False);
               }
            break;
         }       
      break;
   }
}




/************************************************************************
 *
 *  ArrowCB
 *	Process the callback on the subpanel posting/unposting arrow
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
ArrowCB (w, clientData, callData)
Widget    w;
XtPointer clientData;
XtPointer callData;

#else /* _NO_PROTO */
static void
ArrowCB (Widget    w,
         XtPointer clientData,
         XtPointer callData) 
#endif /* _NO_PROTO */

{
   WmPanelistObject         pPanelist = (WmPanelistObject) clientData;
   DtControlCallbackStruct *cb = (DtControlCallbackStruct*) callData;
   WmFunction               wmFunc = (WmFunction) cb->push_function;
   ClientData               *pCD = NULL;
   Boolean                  set_for_posting = TRUE;


   /*  needed for f.kill wrong for open control  */
   /*  optimize to save until unmapped           */

   XFindContext (O_GlobalData (pPanelist)->display,
                 XtWindow (XtParent (cb->subpanel)),
                 O_GlobalData (pPanelist)->windowContextType,
                 (XtPointer) &(pCD));

   if (!XtIsManaged (cb->subpanel))
   {
      Position iX, iY;
      Arg al[2];
      int ac = 0;

      set_for_posting = FALSE;

      XtTranslateCoords (w, 0, 0, &iX, &iY);

      switch (O_SubpanelDirection (pPanelist))
      {
         case WM_FP_TOK_north:
            iX -= (XtWidth (cb->subpanel) - XtWidth (w))/2;
         break;


         /*  Unimplemented cases for future vertical subpanel  */

         case WM_FP_TOK_south:
         case WM_FP_TOK_east:
         case WM_FP_TOK_west:
         break;
      }

      ac = 0;
      XtSetArg (al[ac], XmNx, iX);  ac++;
      XtSetValues (cb->subpanel, al, ac);
   }


   /*  Excute the window manager function to either post or unpost  */

   wmFunc (cb->push_argument, pCD, cb->event);


   /*  Set the arrows behavior, (the image, cursor, and callback functions  */

   ArrowSetBehavior (pPanelist, w, NULL, FALSE, set_for_posting);
}




/************************************************************************
 *
 *  SubpanelUnmapCB
 *	Process the callback on the subpanel unposting initiated through
 *	the window manager close menu item.
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
SubpanelUnmapCB (w, clientData, callData)
Widget    w;
XtPointer clientData;
XtPointer callData;

#else /* _NO_PROTO */
static void
SubpanelUnmapCB (Widget    w,
                 XtPointer clientData,
                 XtPointer callData) 
#endif /* _NO_PROTO */

{
   Widget        arrow = (Widget) clientData;
   Arg           al[1];
   register int  ac;


   /*  Set the arrows behavior, (the image, cursor, and callback functions  */

   ArrowSetBehavior (NULL, arrow, NULL, FALSE, TRUE);



   /*  Remove the event handler from the subpanel  */

   XtRemoveEventHandler (XtParent (w), StructureNotifyMask, False,
                         (XtEventHandler) SubpanelTornEventHandler,
                         (XtPointer) w);

   ac = 0;
   XtSetArg (al[ac], XmNsubpanelTorn, False);  ac++;
   XtSetValues (w, al, ac);

}




/************************************************************************
 *
 *  SubpanelTornEventHandler
 *	Set a subpanel flag to change its posting behavior when it is
 *	torn off.
 *
 ************************************************************************/


#ifdef _NO_PROTO
static void
SubpanelTornEventHandler (subpanel_shell, client_data, event, continue_dispatch)
Widget      subpanel_shell;
XtPointer   client_data;
XEvent    * event;
Boolean   * continue_dispatch;

#else /* _NO_PROTO */
static void
SubpanelTornEventHandler (Widget      subpanel_shell,
                          XtPointer   client_data,
                          XEvent    * event,
                          Boolean   * continue_dispatch)
#endif /* _NO_PROTO */

{
   Arg           al[1];
   register int  ac;

   ac = 0;
   XtSetArg (al[ac], XmNsubpanelTorn, TRUE);  ac++;
   XtSetValues ((Widget) client_data, al, ac);

   *continue_dispatch = TRUE;
}




/************************************************************************
 *
 *  SwitchRenameErrorHelpCB
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
SwitchRenameErrorHelpCB (w, clientData, callData)
Widget    w;
XtPointer clientData;
XtPointer callData;

#else /* _NO_PROTO */
static void
SwitchRenameErrorHelpCB (Widget    w,
                         XtPointer clientData,
                         XtPointer callData) 
#endif /* _NO_PROTO */

{
}




/************************************************************************
 *
 * SwitchRenameCancel
 *        This is the callback which gets called when a user types
 *        escape in a text widget which has this translation tied to it.
 *
 ***********************************************************************/

#ifdef _NO_PROTO
static XtActionProc
SwitchRenameCancel (w, event, params, num_params)
Widget    w;
XEvent    *event;
XtPointer params;
XtPointer num_params;

#else /* _NO_PROTO */
static XtActionProc
SwitchRenameCancel (Widget    w,
                    XEvent    *event,
                    XtPointer params,
                    XtPointer num_params)
#endif /* _NO_PROTO */

{
   Widget switch_button;
   Arg al[1];
 

   /*  Extract the toggle button from the text field's user data  */

   XtSetArg (al[0], XmNuserData, &switch_button);
   XtGetValues (w, al, 1);
   

   /*  Unmanage the text widget, set the traversal to the button and exit  */

   XtUnmanageChild (w);
   XmProcessTraversal (switch_button, XmTRAVERSE_CURRENT);
}




/************************************************************************
 *
 *  SwitchRenameCB
 *	Process callbacks on the text field when a switch name is
 *	being edited.
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
SwitchRenameCB (w, clientData, callData)
Widget    w;
XtPointer clientData;
XtPointer callData;

#else /* _NO_PROTO */
static void
SwitchRenameCB (Widget    w,
                XtPointer clientData,
                XtPointer callData) 
#endif /* _NO_PROTO */

{
   Widget   switch_button = (Widget) clientData;
   char     *edit_string;
   XmString toggle_string;
   String   str;
   String   sAction;

   XmAnyCallbackStruct * callback = (XmAnyCallbackStruct *) callData;

   WmPanelistObject panelist;

   Boolean             valid;
   int                 i;
   WmScreenData        *pSD;
   WmWorkspaceData     *pWS;
	 
   int ac;
   Arg al[2];


   /*  Process the two reasons this callback is invoked.  */
   /*  1.  The text fields value has changed, so change   */
   /*      the workspace name.                            */
   /*  2.  The text field has lost the focus, so remove   */
   /*      the text field.                                */

   if (callback->reason == XmCR_ACTIVATE)
   {   
      ac = 0;
      XtSetArg (al[ac], XmNuserData, &panelist);   ac++;
      XtGetValues (switch_button, al, ac);

      
      pSD = O_ScreenData (panelist);
      pWS = pSD->pActiveWS;


      /*  Get the new name from the text field  */

      edit_string = (char *) XmTextFieldGetString (w);


      /*  Verify that title is valid and unique and if so, validate     */
      /*  uniqueness and then change the toggle and workspace internal  */
      /*  information.                                                  */

      valid = True;

      _DtStripSpaces (edit_string);

      if (strlen (edit_string) == 0)
         valid = False;
      else
      {
         toggle_string = XmStringCreateLtoR(edit_string, XmFONTLIST_DEFAULT_TAG);

         for (i = 0; i < pSD->numWorkspaces; i++)
         {
            if ((pWS != &pSD->pWS[i]) && 
                (XmStringCompare (toggle_string, pSD->pWS[i].title)))
            {
               valid = False;
               break;
            }
         }
      }


      /*  If the title is not valid, post and error dialog and  */
      /*  then leave the text field active to allow reediting.  */
      /*  Otherwise, change the title                           */

      if (!valid)
      {
         _DtMessage (XtParent (w), "Rename Error",
                      "Invalid or duplicate workspace name", NULL,
                      SwitchRenameErrorHelpCB);
      }
      else
      {
	 
         /*  Turn on the busy indicators  */

         ShowWaitState (TRUE);


         /*  Change title within this workspace data  */
         /*  Save changes to resource database        */
         /*  Replace old workspace in info property   */

         if (pWS->title)
            XmStringFree (pWS->title);

         pWS->title = toggle_string;


         /*  Set the wm action for workspace switching  */

         ac = 0;
         XmStringGetLtoR (toggle_string, XmFONTLIST_DEFAULT_TAG, &str);
         sAction = XtMalloc (strlen (str) + 20);
         sprintf (sAction, "f.goto_workspace \"%s\"", str);
         XtSetArg (al[ac], "pushAction", sAction);  ac++;
         XtSetArg (al[ac], XmNstring, toggle_string);   ac++;
         XtSetValues (switch_button, al, ac);


         /*   Set the window manager property and workspace info  */

         SaveWorkspaceResources (pWS, (WM_RES_WORKSPACE_TITLE), False);
         SetWorkspaceInfoProperty (pWS);


         /*  Unmanage and set the traversal to the button and exit  */

         XtUnmanageChild (w);
         XmProcessTraversal (switch_button, XmTRAVERSE_CURRENT);


         /*  Turn off the wait cursor  */

         ShowWaitState (FALSE);
      }


      /*  Free the text field allocated edit string.  The toggle_string  */
      /*  is not freed because it is gobbled by ChangeWorkspaceTitle()   */

      XtFree (edit_string);
   }
   else
   {
      /*  Unmanage the text widget and exit  */

      XtUnmanageChild (w);
   }
}




/************************************************************************
 *
 *  SwitchRenameLabel
 *	Set up the text field and callbacks needed for renaming a 
 *	workspace.
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
SwitchRenameLabel (switch_button, panelist)
Widget           switch_button;
WmPanelistObject panelist;

#else /* _NO_PROTO */
static void
SwitchRenameLabel (Widget switch_button,
                   WmPanelistObject panelist)
#endif /* _NO_PROTO */

{
   static Widget switch_edit = NULL;

   XmString   toggle_string;
   Position   toggle_x;
   Position   toggle_y;
   Dimension  toggle_width;
   Dimension  toggle_height;
   XmFontList toggle_font_list;
   Pixel      toggle_background;

   Position  switch_rc_x;
   Position  switch_rc_y;

   char * edit_string;

   XtTranslations trans_table;

   int    ac;
   Arg    al[15];


   /*  Extract the label and toggle position  */

   ac = 0;
   XtSetArg (al[ac], XmNx, &toggle_x);   ac++;
   XtSetArg (al[ac], XmNy, &toggle_y);   ac++;
   XtSetArg (al[ac], XmNwidth, &toggle_width);   ac++;
   XtSetArg (al[ac], XmNheight, &toggle_height);   ac++;
   XtSetArg (al[ac], XmNstring, &toggle_string);   ac++;
   XtSetArg (al[ac], XmNfontList, &toggle_font_list);   ac++;
   XtGetValues (switch_button, al, ac);
   

   /*  Extract the switch_rc's position  */

   ac = 0;
   XtSetArg (al[ac], XmNx, &switch_rc_x);   ac++;
   XtSetArg (al[ac], XmNy, &switch_rc_y);   ac++;
   XtGetValues (XtParent (switch_button), al, ac);


   /*  Convert the xm string into a char string for editing  and  */
   /*  then create the text widget to perform the name editing.   */

   if (XmStringGetLtoR (toggle_string, XmFONTLIST_DEFAULT_TAG, &edit_string))
   {
      ac = 0;
      XtSetArg (al[ac], XmNleftOffset, toggle_x + switch_rc_x);   ac++;
      XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);   ac++;
      XtSetArg (al[ac], XmNtopOffset, toggle_y + switch_rc_y);   ac++;
      XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);   ac++;
      XtSetArg (al[ac], XmNwidth, toggle_width);   ac++;
      XtSetArg (al[ac], XmNheight, toggle_height);   ac++;
      XtSetArg (al[ac], XmNmarginWidth, 4);   ac++;
      XtSetArg (al[ac], XmNmarginHeight, 0);   ac++;
      XtSetArg (al[ac], XmNfontList, toggle_font_list);   ac++;
      XtSetArg (al[ac], XmNcursorPosition, strlen (edit_string));   ac++;
      XtSetArg (al[ac], XmNhighlightThickness, 1);   ac++;
      XtSetArg (al[ac], XmNvalue, edit_string);   ac++;
      XtSetArg (al[ac], XmNuserData, switch_button);   ac++;

      if (switch_edit == NULL)
      {
         Arg al2[2];
         Pixel background;
         Pixel foreground;
         Pixel top_shadow;
         Pixel bottom_shadow;
         Pixel select;
         Colormap colormap;


         /*  Get the select color  */

         XtSetArg (al2[0], XmNbackground, &background);
         XtSetArg (al2[1], XmNcolormap,  &colormap);
         XtGetValues (XtParent (switch_button), al2, 2);

         XmGetColors (XtScreen (XtParent (switch_button)), colormap, background,
                      &foreground, &top_shadow, &bottom_shadow, &select);

         switch_edit = (Widget) XmCreateTextField (O_Switch(panelist), 
                                                   "switch_edit", al, ac);

         XtSetArg (al2[0], XmNbackground, select);
         XtSetValues (switch_edit, al2, 1);
      }
      else
      {
         Arg al2[1];
         Widget old_client_data;

         XtSetArg (al2[0], XmNuserData, &old_client_data);
         XtGetValues (switch_edit, al2, 1);


         /*  Remove the existing callbacks so that proper  */
         /*  client data will show up.                     */


         XtRemoveCallback (switch_edit, XmNlosingFocusCallback, 
                           (XtCallbackProc) SwitchRenameCB, 
                           (XtPointer) old_client_data);

         XtRemoveCallback (switch_edit, XmNactivateCallback, 
                           (XtCallbackProc) SwitchRenameCB, 
                           (XtPointer) old_client_data);


         /*  Set the switch editing field to the new resource set  */

         XtSetValues (switch_edit, al, ac);
      }


      /*  Add the callbacks for the text input processing  */

      XtAddCallback (switch_edit, XmNlosingFocusCallback, 
                     (XtCallbackProc) SwitchRenameCB, 
                     (XtPointer) switch_button);

      XtAddCallback (switch_edit, XmNactivateCallback, 
                     (XtCallbackProc) SwitchRenameCB, 
                     (XtPointer) switch_button);

      XtManageChild (switch_edit);
      XmRemoveTabGroup (switch_edit);
      XmProcessTraversal (switch_edit, XmTRAVERSE_CURRENT);


      /* set up translations in main edit widget */

      trans_table = XtParseTranslationTable (translations_escape);
      XtOverrideTranslations (switch_edit, trans_table);
   }

   XmStringFree (toggle_string);
}



 
/************************************************************************
 *
 *  GetImageName
 *      Get the file name for an icon by extracting the panel resolution
 *      and then looking up the image name.
 *
 ************************************************************************/

#ifdef _NO_PROTO
static String
GetImageName (pPanelist, sImage)
WmPanelistObject pPanelist;
String           sImage;

#else /* _NO_PROTO */
static String
GetImageName (WmPanelistObject pPanelist,
              String           sImage)
#endif /* _NO_PROTO */

{
   unsigned int iSize;
   String       sReturn;
   Screen       *scr = XtScreen ((Widget) pPanelist);


   /*  Get icon size.  */

   switch (O_Resolution (pPanelist))
   {
      case HIGH_RES_DISPLAY:    iSize = DtLARGE;       break;
      case MED_RES_DISPLAY:     iSize = DtMEDIUM;      break;
      case LOW_RES_DISPLAY:     iSize = DtSMALL;       break;
   }


   /*  Get name.  */

   sReturn = _DtGetIconFileName (scr, sImage, NULL, NULL, iSize);
   if (sReturn == NULL)
       sReturn = _DtGetIconFileName (scr, sImage, NULL, NULL, DtUNSPECIFIED);
   if (sReturn == NULL)
       sReturn = XtNewString (sImage);


   /*  Return value to be freed by caller.  */

   return sReturn;
}




/************************************************************************
 *
 *  SetDropZones
 *      Build up the drop registration strings for each drop accepting
 *      control and then register the information with the d&d controller
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
SetDropZones (wBox)
Widget            wBox;

#else /* _NO_PROTO */
static void
SetDropZones (Widget wBox)
#endif /* _NO_PROTO */

{
   CompositeWidget  cw = (CompositeWidget) wBox;
   DtControlGadget cg;
   Widget           *pWid;
   int              iWid;
   int              i;
   Arg		    args[1];

   iWid = M_NumChildren (cw);
   pWid = M_Children (cw);

   for (i = 0; i < iWid; i++)
   {
      cg = (DtControlGadget) pWid[i];

      if (DtIsControl (pWid[i]) && (G_DropAction (cg) != NULL)) {

         XtSetArg (args[0], XmNdropSiteOperations,
		   XmDROP_COPY | XmDROP_MOVE | XmDROP_LINK);
         XtSetValues((Widget)pWid[i], args, 1);

         XtAddCallback (pWid[i], XmNdropCallback, DropCB, (XtPointer) NULL);
      }
   }
}




/************************************************************************
 *
 *  HandleInputCB
 *	Process button events on the frontpanel handles.
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
HandleInputCB (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;

#else /* _NO_PROTO */
static void
HandleInputCB (Widget    w,
               XtPointer client_data,
               XtPointer call_data) 
#endif /* _NO_PROTO */

{
   XmAnyCallbackStruct * callback;
   WmPanelistObject      pPanelist;
   ClientData          * pCD;

   XButtonEvent * event;
   XEvent       * new_event;

   static first = TRUE;

   static WmFunction menu_wm_func;
   static WmFunction move_wm_func;
   static int        menu_function_index;
   static int        move_function_index;
   static String     menu_func_arg = NULL;


   callback = (XmAnyCallbackStruct *) call_data;
   new_event = (XEvent *) callback->event;


   /*  One time parse for the window manager functions for  */
   /*  window move and system menu posting.                 */

   if (first)
   {
      String s1;

      first = FALSE;

      s1 = "f.menu";
      menu_function_index = 
         ParseWmFunction ((unsigned char **)&s1, CRS_ANY, &menu_wm_func);

      s1 = "DtPanelMenu";
      ParseWmFunctionArg ((unsigned char **)&s1, menu_function_index,
                          menu_wm_func, 
                          (void **)&menu_func_arg, NULL, NULL);

      s1 = "f.move";
      move_function_index =
          ParseWmFunction ((unsigned char **)&s1, CRS_ANY, &move_wm_func);
   }


   /*  If the callback was called because of a button press then,  */
   /*  if it is button1, call the window manager function to       */
   /*  initiate a window move.  It it is button3, call the window  */
   /*  manager function to post the system menu.                   */


   pPanelist = (WmPanelistObject) client_data;

   XFindContext (O_GlobalData (pPanelist)->display,
                 XtWindow (XtParent (XtParent (w))),
                 O_GlobalData (pPanelist)->windowContextType,
                 (XtPointer) &(pCD));

   event = (XButtonEvent *) new_event;

   if (new_event->type == ButtonPress && event->button == Button1)
   {
      XUngrabPointer (XtDisplay (w), event->time);
      XSync (XtDisplay (w), FALSE);
      move_wm_func (NULL, pCD, new_event);
   }
   else if (new_event->type == ButtonPress && event->button == Button3)
   {
      XUngrabPointer (XtDisplay (w), event->time);
      XSync (XtDisplay (w), FALSE);
      menu_wm_func (menu_func_arg, pCD, new_event);
   }
}




/************************************************************************
 *
 *  HandleCreate
 *	Create the two handles used for system menu and panel moving
 *	at the ends of the main front panel.
 *
 ************************************************************************/

#ifdef _NO_PROTO
static Widget
HandleCreate (parent, pPanelist)
Widget           parent;
WmPanelistObject pPanelist;

#else /* _NO_PROTO */
static Widget
HandleCreate (Widget           parent,
              WmPanelistObject pPanelist)
#endif /* _NO_PROTO */

{
   Arg al[10];
   int ac = 0;
   Widget handle;
   String image_name;
   Pixel top_shadow;
   Pixel bottom_shadow;
   Pixmap pixmap;


   image_name = _DtGetIconFileName (XtScreen (parent),
                                    "handle", NULL, NULL, DtLARGE);

   ac = 0;
   XtSetArg (al[ac], XmNtopShadowColor, &top_shadow); ac++;
   XtSetArg (al[ac], XmNbottomShadowColor, &bottom_shadow); ac++;
   XtGetValues (parent, al, ac);

   pixmap = XmGetPixmap (XtScreen (parent), image_name,
                         bottom_shadow, top_shadow);

   ac = 0;
   XtSetArg (al[ac], XmNwidth, 10); ac++;
   XtSetArg (al[ac], XmNshadowThickness, 0); ac++;
   XtSetArg (al[ac], XmNborderWidth, 0); ac++;
   XtSetArg (al[ac], XmNbackgroundPixmap, pixmap); ac++;

   handle = XmCreateDrawingArea (parent, "handle", al, ac);
   XtManageChild (handle);

   XtAddCallback (handle, XmNinputCallback,
                  (XtCallbackProc) HandleInputCB, (XtPointer) pPanelist);

   return (handle);
}




/************************************************************************
 *
 *  Allocate Panel
 *
 ************************************************************************/

#ifdef _NO_PROTO
static Widget
AllocatePanel (pPanelist, pItem, wParent)
WmPanelistObject pPanelist;
WmFpItem         pItem;
Widget           wParent;

#else /* _NO_PROTO */
static Widget
AllocatePanel (WmPanelistObject pPanelist,
               WmFpItem         pItem,
               Widget           wParent)
#endif /* _NO_PROTO */

{
   Widget              wPanel;
   Widget              wBox;
   Arg                 al[40];
   register int        ac;
   int                 i;
   int                 iPos;
   WmFpItem            pBox;
   WmFpToken           eToken;
   WmFpToken           eTokOrientation = WM_FP_TOK_none;
   WmScreenData        *pSD = O_ScreenData (pPanelist);
   String              sName = pItem->pchName;
   String              sHelp = NULL;
   String              sTopic = NULL;
   Boolean             bVertical = False;
   PixelSet            *pPS;
   DtPanelShellWidget psw;


   /*  Create panel shell.  */

   ac = 0;
   XtSetArg (al[ac], XmNallowShellResize, True); ac++;
   O_Shell (pPanelist) =
      XtCreatePopupShell (sName, dtPanelShellWidgetClass, wParent, al, ac);
   psw = (DtPanelShellWidget) O_Shell (pPanelist);


   /*  Add the additional actions needed by the panel  */

   XtAppAddActions (XtWidgetToApplicationContext (O_Shell (pPanelist)),
                     actionTable, 1);


   /*  Set to invalid value for now.  */
   O_SubpanelDirection (pPanelist) = WM_FP_TOK_none;

   /*  Process panel tokens.  */

   for (i = 0; i < pItem->iTokens; i++)
   {
      if (pItem->ppchTokens[i] == NULL)
         continue;

      eToken = WmFpStringToPanelToken (pItem->ppchTokens[i]);

      switch (eToken)
      {
         case WM_FP_TOK_subpanelDirection:
            O_SubpanelDirection (pPanelist) =
               WmFpStringToSubpanelDirectionToken (pItem->ppchTokens[++i]);
         break;

         case WM_FP_TOK_orientation:
            eTokOrientation =
               WmFpStringToOrientationToken (pItem->ppchTokens[++i]);
         break;
      }
   }


   /*  Validate subpanel direction and panel orientation.  */

   if ((O_SubpanelDirection (pPanelist) != WM_FP_TOK_north) &&
       (O_SubpanelDirection (pPanelist) != WM_FP_TOK_south) &&
       (O_SubpanelDirection (pPanelist) != WM_FP_TOK_east)  &&
       (O_SubpanelDirection (pPanelist) != WM_FP_TOK_west))
      O_SubpanelDirection (pPanelist) = WM_FP_TOK_north;

   switch (O_SubpanelDirection (pPanelist))
   {
      case WM_FP_TOK_north:
      case WM_FP_TOK_south:
        bVertical = (eTokOrientation != WM_FP_TOK_horizontal);
        break;

      case WM_FP_TOK_east:
      case WM_FP_TOK_west:
        bVertical = (eTokOrientation == WM_FP_TOK_vertical);
      break;
    }


   /*  Validate resolution.  */

   if (S_Resolution (psw) > 0)
   {
      Display *dpy = XtDisplay (psw);
       O_Resolution (pPanelist) = 
          _DtGetDisplayResolution (XtDisplay ((Widget) psw), 0);
   }

   if ((O_Resolution (pPanelist) != HIGH_RES_DISPLAY) &&
       (O_Resolution (pPanelist) != MED_RES_DISPLAY) &&
       (O_Resolution (pPanelist) != LOW_RES_DISPLAY))
   {
      O_Resolution (pPanelist) =
         _DtGetDisplayResolution (XtDisplay ((Widget) psw),
            XScreenNumberOfScreen (XtScreen ((Widget) psw)));
   }


   /*  Get font list.  */

   if (O_FontList (pPanelist) == NULL)
   {
      switch (O_Resolution (pPanelist))
      {
         case HIGH_RES_DISPLAY:
            O_FontList (pPanelist) = S_HighResFontList (psw);
         break;

         case MED_RES_DISPLAY:
            O_FontList (pPanelist) = S_MediumResFontList (psw);
         break;

         case LOW_RES_DISPLAY:
            O_FontList (pPanelist) = S_LowResFontList (psw);
         break;
      }
   }

   /*  Get pixel set.  */

   if (O_DefaultColors (pPanelist))
   {
       Widget   w = O_Shell (pPanelist);
       Screen   *pScr = XtScreen (w);
       XrmValue pixelData;

       pPS = O_PixelSetDefault (pPanelist);


       /*  Invert black and white.  */

       if ((pSD->colorUse == B_W) && (pPS->bg != M_Background (w)))
       {
          Screen *pScr = XtScreen (w);
          Pixel  pixelB = BlackPixelOfScreen (pScr);
          Pixel  pixelW = WhitePixelOfScreen (pScr);

          pPS->fg = pixelB;
          pPS->bg = pixelW;
          pPS->ts = pixelB;
          pPS->bs = pixelB;
          pPS->sc = pixelW;
       }

       /*  Use Motif defaults instead of black and white.  */

      else
      {
         pPS->bg = M_Background (w);

         XmGetColors (pScr, DefaultColormapOfScreen (pScr), pPS->bg,
                      &(pPS->fg), &(pPS->ts), &(pPS->bs), &(pPS->sc));
      }
   }
   else
      if (pSD->pPrimaryPixelSet != NULL)
         pPS = pSD->pPrimaryPixelSet;
      else
         pPS = O_PixelSetDefault (pPanelist);


   /*  Set pixel resources.  */

   ac = 0;
   XtSetArg (al[ac], XmNforeground, pPS->fg);  ac++;
   XtSetArg (al[ac], XmNbackground, pPS->bg);  ac++;

   if ((pSD->colorUse == B_W) || (pSD->pPixelData == NULL))
   {
      XtSetArg (al[ac], XmNtopShadowColor,
                WhitePixelOfScreen (XtScreen (wParent)));  ac++;
      XtSetArg (al[ac], XmNbottomShadowColor,
                BlackPixelOfScreen (XtScreen (wParent)));  ac++;
      XtSetArg (al[ac], XmNtopShadowPixmap, XmUNSPECIFIED_PIXMAP);  ac++;
      XtSetArg (al[ac], XmNbottomShadowPixmap, XmUNSPECIFIED_PIXMAP);  ac++;
   }
   else
   {
      XtSetArg (al[ac], XmNtopShadowColor, pPS->ts);  ac++;
      XtSetArg (al[ac], XmNbottomShadowColor, pPS->bs);  ac++;
      XtSetArg (al[ac], XmNtopShadowPixmap, XmUNSPECIFIED_PIXMAP);  ac++;
      XtSetArg (al[ac], XmNbottomShadowPixmap, XmUNSPECIFIED_PIXMAP);  ac++;
   }


   /*  Set RowColumn resources.  */

   XtSetArg (al[ac], XmNspacing, 0);  ac++;
   XtSetArg (al[ac], XmNmarginWidth, 0);  ac++;
   XtSetArg (al[ac], XmNmarginHeight, 0);  ac++;
   XtSetArg (al[ac], XmNpacking, XmPACK_TIGHT);  ac++;
   XtSetArg (al[ac], XmNnumColumns, 1);  ac++;

/* FORCE ORIENTATION --- for now  */
   XtSetArg (al[ac], XmNorientation, XmHORIZONTAL);  ac++;
/*
   if (bVertical)
   {
      XtSetArg (al[ac], XmNorientation, XmVERTICAL);  ac++;
   }
   else
   {
      XtSetArg (al[ac], XmNorientation, XmHORIZONTAL);  ac++;
   }
*/

   /*  Create panel view.  */

   wPanel = XmCreateRowColumn (O_Shell (pPanelist), sName, al, ac);
   XtManageChild (wPanel);
   O_Panel (pPanelist) = wPanel;


   /*  Create boxes  */

   for (i = 0; i < pItem->iTokens; i++)
   {
      if (pItem->ppchTokens[i] == NULL)
         continue;

      eToken = WmFpStringToPanelToken(pItem->ppchTokens[i]);

      switch (eToken)
      {
         case WM_FP_TOK_box:
            iPos = WmFpListGetItemPos (O_Boxes (pPanelist),
                                       pItem->ppchTokens[++i]);
            if (iPos > 0)
            {
               Widget button1, button2;


               /*  Create three widgets along the single main panel  */
               /*  row.  The first and last are drawn buttons.  In   */
               /*  in the middle is the main panel.                  */

               button1 = HandleCreate (wPanel, pPanelist);

               pBox = WmFpListGetItem (O_Boxes (pPanelist), iPos);
               wBox = AllocateBox (pPanelist, pBox, wPanel);

               button2 = HandleCreate (wPanel, pPanelist);
            }
            else
            {
               sprintf (sWarnBuf, 
                        ((char*) GETMESSAGE (78, 1, sWarnMissingItem)),
                        pItem->ppchTokens[i]);
               Warning (sWarnBuf);
            }
         break;

         case WM_FP_TOK_helpString:
            sHelp = pItem->ppchTokens[++i];

            if (sHelp != NULL)
               XtAddCallback (wPanel, XmNhelpCallback,
                              (XtCallbackProc) WmXvhStringHelpCB,
                              (XtPointer) sHelp);
         break;

         case WM_FP_TOK_helpTopic:
            sTopic = pItem->ppchTokens[++i];

            if (sTopic != NULL)
               XtAddCallback (wPanel, XmNhelpCallback,
                              (XtCallbackProc) WmXvhTopicHelpCB,
                              (XtPointer) sTopic);
         break;

         default:
            i++;
         break;
      }
   }


   /*  Register default help string.  */

   if ((sHelp == NULL) && (sTopic == NULL))
      XtAddCallback (wPanel, XmNhelpCallback,
                     (XtCallbackProc) WmXvhStringHelpCB,
                     (XtPointer) PANEL_HELP);

   return wPanel;
}




/************************************************************************
 *
 *  Allocate Subpanel
 *
 ************************************************************************/

#ifdef _NO_PROTO
static Widget
AllocateSubpanel (pPanelist, pItem, wParent)
WmPanelistObject pPanelist;
WmFpItem         pItem;
Widget           wParent;

#else /* _NO_PROTO */
static Widget
AllocateSubpanel (WmPanelistObject pPanelist,
                  WmFpItem         pItem,
                  Widget           wParent)
#endif /* _NO_PROTO */

{
   Widget       wShell;
   Widget       wBox;
   Arg          al[20];
   register int ac;
   WmFpToken    eToken;
   String       sName = pItem->pchName;
   Boolean      bEmpty = True;
   int          i;


   /*  Check for empty subpanel  */

   for (i = 0; (bEmpty && (i < pItem->iTokens)); i++)
   {
       if (pItem->ppchTokens[i] == NULL)
          continue;

       eToken = WmFpStringToBoxToken (pItem->ppchTokens[i]);

       if ((eToken == WM_FP_TOK_control) || (eToken == WM_FP_TOK_switch))
          bEmpty = False;
   }

   if (bEmpty)
      return NULL;

   ac = 0;
   XtSetArg (al[ac], XmNallowShellResize, True); ac++;

#ifdef _NO_WM
   switch (O_SubpanelDirection (pPanelist))
   {
      case WM_FP_TOK_north:
      case WM_FP_TOK_south:
         XtSetArg (al[ac], XmNwinGravity, SouthGravity); ac++;
      break;

      case WM_FP_TOK_east:
      case WM_FP_TOK_west:
         XtSetArg (al[ac], XmNwinGravity, WestGravity); ac++;
      break;
   }
#endif  /* _NO_WM */

   wShell = 
      XtCreatePopupShell (sName, xmDialogShellWidgetClass, wParent, al, ac);

#ifdef _NO_WM
   wBox = AllocateBox (pPanelist, pItem, O_Panel (pPanelist));
#else
   wBox = AllocateBox (pPanelist, pItem, wShell);
#endif  /* _NO_WM */


   return (wBox);
}




/************************************************************************
 *
 *  AllocateBox
 *
 ************************************************************************/

#ifdef _NO_PROTO
static Widget
AllocateBox (pPanelist, pItem, wParent)
WmPanelistObject pPanelist;
WmFpItem         pItem;
Widget           wParent;

#else /* _NO_PROTO */
static Widget
AllocateBox (WmPanelistObject pPanelist,
             WmFpItem         pItem,
             Widget           wParent)
#endif /* _NO_PROTO */

{
   Widget        wPanel;
   Widget        wBox;
   Widget        wControl;
   Widget        wPrev = NULL;
   Arg           al[40];
   register int  ac;
   int           i;
   int           iColorSet = 0;
   int           iPos;
   int           iNumRows = 0;
   int           iNumColumns = 0;
   WmFpItem      pControl;
   WmFpItem      pSwitch;
   WmFpToken     eToken;
   WmFpToken     eTokLayout = WM_FP_TOK_none;
   WmFpToken     eTokOrientation = WM_FP_TOK_none;
   WmFpToken     eTokType = WM_FP_TOK_primary;
   WmScreenData *pSD = O_ScreenData (pPanelist);
   PixelSet     *pPS = NULL;
   Pixmap        pix = XmUNSPECIFIED_PIXMAP;
   String        sName = pItem->pchName;
   String        sPix = NULL;
   String        sHelp = NULL;
   String        sTopic = NULL;
   String        sTitle = NULL;
   String        sTile;
   String        sRows = NULL;
   String        sColumns = NULL;
   Boolean       bAsNeeded = False;
   Boolean       bVertical = True;


   /*  Process box tokens.  */

   for (i = 0; i < pItem->iTokens; i++)
   {
      if (pItem->ppchTokens[i] == NULL)
         continue;

      eToken = WmFpStringToBoxToken (pItem->ppchTokens[i]);

      switch (eToken)
      {
         case WM_FP_TOK_type:
            eTokType = WmFpStringToBoxTypeToken (pItem->ppchTokens[++i]);
         break;

         case WM_FP_TOK_backgroundTile:
            sPix = pItem->ppchTokens[++i];
         break;

         case WM_FP_TOK_colorSet:
            sTile = pItem->ppchTokens[++i];
            if (sTile != NULL)
                iColorSet = atoi (sTile);
         break;

         case WM_FP_TOK_title:
            sTitle = pItem->ppchTokens[++i];
            if (sTitle == NULL)
               break;

            ac = 0;
            XtSetArg (al[ac], XmNtitle, sTitle);  ac++;
            XtSetValues (wParent, al, ac);
         break;

         case WM_FP_TOK_layoutPolicy:
            eTokLayout =
               WmFpStringToLayoutPolicyToken (pItem->ppchTokens[++i]);
            bAsNeeded = (eTokLayout == WM_FP_TOK_asNeeded);
         break;

         case WM_FP_TOK_orientation:
            eTokOrientation =
               WmFpStringToOrientationToken (pItem->ppchTokens[++i]);
            bVertical = (eTokOrientation != WM_FP_TOK_horizontal);
         break;

         case WM_FP_TOK_numberOfRows:
            sRows = pItem->ppchTokens[++i];
            if (sRows != NULL)
               iNumRows = atoi (sRows);
         break;

         case WM_FP_TOK_numberOfColumns:
            sColumns = pItem->ppchTokens[++i];
            if (sColumns != NULL)
               iNumColumns = atoi (sColumns);
         break;
      }
   }


   /*  Set pixels.   */


   if (O_DefaultColors (pPanelist) || (pSD->pPixelData == NULL))
      pPS = O_PixelSetDefault (pPanelist);
   else if ((iColorSet > 0) && (iColorSet <= 8))
      pPS = &(pSD->pPixelData)[iColorSet-1];
   else
      pPS = pSD->pPrimaryPixelSet;


   ac = 0;
   XtSetArg (al[ac], XmNforeground, pPS->fg);  ac++;
   XtSetArg (al[ac], XmNbackground, pPS->bg);  ac++;

   if ((pSD->colorUse == B_W) || (pSD->pPixelData == NULL))
   {
      XtSetArg (al[ac], XmNtopShadowColor,
                        WhitePixelOfScreen (XtScreen (wParent)));  ac++;
      XtSetArg (al[ac], XmNbottomShadowColor,
                        BlackPixelOfScreen (XtScreen (wParent)));  ac++;
      XtSetArg (al[ac], XmNhighlightColor, pPS->fg);  ac++;
      XtSetArg (al[ac], XmNtopShadowPixmap, XmUNSPECIFIED_PIXMAP);  ac++;
      XtSetArg (al[ac], XmNbottomShadowPixmap, XmUNSPECIFIED_PIXMAP);  ac++;
   }
   else
   {
      XtSetArg (al[ac], XmNtopShadowColor, pPS->ts);  ac++;
      XtSetArg (al[ac], XmNbottomShadowColor, pPS->bs);  ac++;
      XtSetArg (al[ac], XmNtopShadowPixmap, XmUNSPECIFIED_PIXMAP);  ac++;
      XtSetArg (al[ac], XmNbottomShadowPixmap, XmUNSPECIFIED_PIXMAP);  ac++;
   }


   /*  Use default dither if color use is black and white  */

   if ((sPix == NULL) && ((pSD->colorUse == B_W) ||
      (pSD->pPixelData == NULL) && (! O_DefaultColors (pPanelist))))
      sPix = WM_FP_DITHER;

   if (sPix != NULL)
   {
      String sImage = GetImageName (pPanelist, sPix);

      if ((pSD->colorUse == B_W) || (pSD->pPixelData == NULL))
         pix = XmGetPixmap (XtScreen (wParent), sImage,
                            BlackPixelOfScreen (XtScreen (wParent)),
                            WhitePixelOfScreen (XtScreen (wParent)));
      else
         pix = XmGetPixmap (XtScreen (wParent), sImage, pPS->bs, pPS->ts);
    
      if (pix != XmUNSPECIFIED_PIXMAP)
         XtSetArg (al[ac], XmNbackgroundPixmap, pix);  ac++;
   }


   /*  Set dimensions.   */

   XtSetArg (al[ac], XmNmarginWidth, 0);  ac++;
   XtSetArg (al[ac], XmNmarginHeight, 0);  ac++;

   switch (eTokType)
   {
      case WM_FP_TOK_primary:
         XtSetArg (al[ac], XmNboxType, XmBOX_PRIMARY);  ac++;
         XtSetArg (al[ac], XmNshadowThickness, 1);  ac++;
         XtSetArg (al[ac], XmNshadowType, XmSHADOW_OUT);  ac++;

         switch (O_Resolution (pPanelist))
         {
            case HIGH_RES_DISPLAY:
               if (!bAsNeeded)
               {
                  switch (O_SubpanelDirection (pPanelist))
                  {
                     case WM_FP_TOK_north:
                     case WM_FP_TOK_south:
                        XtSetArg (al[ac], XmNwidthIncrement, 
                                  WM_FP_HI_PRI_WIDTH/2);  ac++;
                        XtSetArg (al[ac], XmNheight, 
                                  WM_FP_HI_PRI_HEIGHT+4);  ac++;
                     break;

                     case WM_FP_TOK_east:
                     case WM_FP_TOK_west:
                        XtSetArg (al[ac], XmNheightIncrement, 
                                  WM_FP_HI_PRI_HEIGHT);  ac++;
                     break;
                  }
               }

               XtSetArg (al[ac], XmNleftInset, WM_FP_HI_PRI_INSET_LEFT);  ac++;
               XtSetArg (al[ac], XmNrightInset, 
                         WM_FP_HI_PRI_INSET_RIGHT);  ac++;
               XtSetArg (al[ac], XmNtopInset, WM_FP_HI_PRI_INSET_TOP);  ac++;
               XtSetArg (al[ac], XmNbottomInset, 
                         WM_FP_HI_PRI_INSET_BOTTOM);  ac++;
            break;

            case MED_RES_DISPLAY:
               if (!bAsNeeded)
               {
                  switch (O_SubpanelDirection (pPanelist))
                  {
                     case WM_FP_TOK_north:
                     case WM_FP_TOK_south:
                        XtSetArg (al[ac], XmNwidthIncrement,
                                  WM_FP_MED_PRI_WIDTH/2);  ac++;
                        XtSetArg (al[ac], XmNheight,
                                   WM_FP_MED_PRI_HEIGHT+4);  ac++;
                     break;

                     case WM_FP_TOK_east:
                     case WM_FP_TOK_west:
                        XtSetArg (al[ac], XmNheightIncrement,
                                   WM_FP_MED_PRI_HEIGHT);  ac++;
                     break;
                  }
               }

               XtSetArg (al[ac], XmNleftInset, 
                         WM_FP_MED_PRI_INSET_LEFT);  ac++;
               XtSetArg (al[ac], XmNrightInset,
                          WM_FP_MED_PRI_INSET_RIGHT);  ac++;
               XtSetArg (al[ac], XmNtopInset, WM_FP_MED_PRI_INSET_TOP);  ac++;
               XtSetArg (al[ac], XmNbottomInset,
                         WM_FP_MED_PRI_INSET_BOTTOM);  ac++;
            break;

            case LOW_RES_DISPLAY:
               if (!bAsNeeded)
               {
                  switch (O_SubpanelDirection (pPanelist))
                  {
                     case WM_FP_TOK_north:
                     case WM_FP_TOK_south:
                        XtSetArg (al[ac], XmNwidthIncrement,
                                  WM_FP_LOW_PRI_WIDTH/2);  ac++;
                        XtSetArg (al[ac], XmNheight,
                                  WM_FP_LOW_PRI_HEIGHT+4);  ac++;
                     break;

                     case WM_FP_TOK_east:
                     case WM_FP_TOK_west:
                        XtSetArg (al[ac], XmNheightIncrement,
                                  WM_FP_LOW_PRI_HEIGHT);  ac++;
                     break;
                  }
               }

               XtSetArg (al[ac], XmNleftInset, 
                         WM_FP_LOW_PRI_INSET_LEFT);  ac++;
               XtSetArg (al[ac], XmNrightInset,
                         WM_FP_LOW_PRI_INSET_RIGHT);  ac++;
               XtSetArg (al[ac], XmNtopInset, WM_FP_LOW_PRI_INSET_TOP);  ac++;
               XtSetArg (al[ac], XmNbottomInset,
                         WM_FP_LOW_PRI_INSET_BOTTOM);  ac++;
            break;
         }
      break;

      case WM_FP_TOK_subpanel:
         XtSetArg (al[ac], XmNboxType, XmBOX_SUBPANEL);  ac++;
         XtSetArg (al[ac], XmNshadowThickness, 1);  ac++;
         XtSetArg (al[ac], XmNshadowType, XmSHADOW_OUT);  ac++;
         XtSetArg (al[ac], XmNheightIncrement, 2);  ac++;
         XtSetArg (al[ac], XmNwidthIncrement, 4);  ac++;

#ifndef _NO_WM
         XtSetArg (al[ac], XmNdefaultPosition, False);  ac++;
         XtSetArg (al[ac], XmNautoUnmanage, False);  ac++;
#endif /* _NO_WM */

      break;

      case WM_FP_TOK_rowColumn:
         XtSetArg (al[ac], XmNspacing, 0);  ac++;
         XtSetArg (al[ac], XmNmarginWidth, 0);  ac++;
         XtSetArg (al[ac], XmNmarginHeight, 0);  ac++;

         if (!bAsNeeded)
         {
            XtSetArg (al[ac], XmNpacking, XmPACK_COLUMN);  ac++;
         }
         else
         {
            XtSetArg (al[ac], XmNpacking, XmPACK_TIGHT);  ac++;
         }

         if (bVertical)
         {
            XtSetArg (al[ac], XmNorientation, XmVERTICAL);  ac++;

            if (iNumColumns < 1)
               iNumColumns = (iNumRows > 1) ? iNumRows : 1;

            XtSetArg (al[ac], XmNnumColumns, iNumColumns);  ac++;
         }
         else
         {
            XtSetArg (al[ac], XmNorientation, XmHORIZONTAL);  ac++;

            if (iNumRows < 1)
               iNumRows = (iNumColumns > 1) ? iNumColumns : 1;

            XtSetArg (al[ac], XmNnumColumns, iNumRows);  ac++;
         }
      break;
   }


   switch (eTokType)
   {
      case WM_FP_TOK_primary:
         wBox = XtCreateWidget (sName, dtViewWidgetClass, wParent, al, ac);
         XtManageChild (wBox);
      break;

      case WM_FP_TOK_subpanel:
         wPanel = XtCreateWidget (sName, dtViewWidgetClass, wParent, al, ac);
         XtSetArg (al[ac], XmNpacking, XmPACK_COLUMN);  ac++;

         if (bVertical)
         {
            XtSetArg (al[ac], XmNorientation, XmVERTICAL);  ac++;

            if (iNumColumns < 1)
               iNumColumns = (iNumRows > 1) ? iNumRows : 1;

            XtSetArg (al[ac], XmNnumColumns, iNumColumns);  ac++;
         }
         else
         {
            XtSetArg (al[ac], XmNorientation, XmHORIZONTAL);  ac++;

            if (iNumRows < 1)
               iNumRows = (iNumColumns > 1) ? iNumColumns : 1;

            XtSetArg (al[ac], XmNnumColumns, iNumRows);  ac++;
         }

         XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;
         XtSetArg (al[ac], XmNtopOffset, 2);  ac++;
         XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);  ac++;
         XtSetArg (al[ac], XmNbottomOffset, 2);  ac++;
         XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
         XtSetArg (al[ac], XmNleftOffset, 2);  ac++;
         XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  ac++;
         XtSetArg (al[ac], XmNrightOffset, 2);  ac++;

         wBox = XmCreateRowColumn (wPanel, sName, al, ac);
         XtManageChild (wBox);
      break;

      case WM_FP_TOK_rowColumn:
         wBox = XmCreateRowColumn (wParent, sName, al, ac);
         XtManageChild (wBox);
      break;
   }


   for (i = 0; i < pItem->iTokens; i++)
   {
      WmFpToken  eControlType = WM_FP_TOK_none;
      WmFpItem   pStdControl;
      Boolean    bAlloc = False;
      String     sName = NULL;

      if (pItem->ppchTokens[i] == NULL)
         continue;

      eToken = WmFpStringToBoxToken (pItem->ppchTokens[i]);

      switch (eToken)
      {
         case WM_FP_TOK_helpString:
            sHelp = pItem->ppchTokens[++i];

            if (sHelp != NULL)
               XtAddCallback (wBox, XmNhelpCallback,
                              (XtCallbackProc) WmXvhStringHelpCB,
                              (XtPointer) sHelp);
         break;

         case WM_FP_TOK_helpTopic:
            sTopic = pItem->ppchTokens[++i];

            if (sTopic != NULL)
               XtAddCallback (wBox, XmNhelpCallback,
                              (XtCallbackProc) WmXvhTopicHelpCB,
                              (XtPointer) sTopic);
         break;

#if 1
         case WM_FP_TOK_box:
            iPos = WmFpListGetItemPos (O_Boxes (pPanelist),
                                       pItem->ppchTokens[++i]);

            if (iPos > 0)
            {
               WmFpItem pBoxKid = WmFpListGetItem (O_Boxes (pPanelist), iPos);
               Widget   wBoxKid = AllocateBox (pPanelist, pBoxKid, wBox);
            }
            else
            {
               sprintf (sWarnBuf, 
                        ((char*) GETMESSAGE (78, 1, sWarnMissingItem)),
                        pItem->ppchTokens[i]);
               Warning (sWarnBuf);
            }
         break;
#endif

         case WM_FP_TOK_control:
            iPos = WmFpListGetItemPos (O_Controls (pPanelist),
                                       pItem->ppchTokens[++i]);

            if (iPos > 0)
            {
               Widget w;

               pControl = WmFpListGetItem (O_Controls (pPanelist), iPos);
               w = AllocateControl (pPanelist, pControl, wBox, wPrev,
                                    eTokType, pPS);

               if (w != NULL) wPrev = w;

               bAlloc = True;
            }
            else
            {
               /*  Try to use name for control type.  */

               String sTemp = XtMalloc (strlen (pItem->ppchTokens[i]) + 1);

               strcpy (sTemp, pItem->ppchTokens[i]);
               _DtWmParseToLower ((unsigned char *) sTemp);
               eControlType = WmFpStringToControlTypeToken (sTemp);
               XtFree (sTemp);

               switch (eControlType)
               {
                  case WM_FP_TOK_blank:
                  case WM_FP_TOK_date:
                  case WM_FP_TOK_clock:
                     sName = (WmFpString) XtMalloc (strlen ((char *)
                                                    pItem->ppchTokens[i]) + 1);
                     strcpy ((char *) sName, (char *) pItem->ppchTokens[i]);
                     pStdControl = WmFpItemCreate (WM_FP_TOK_control,
                                                   sName, NULL, 0);
                     wPrev = AllocateControl (pPanelist, pStdControl,
                                              wBox, wPrev, eTokType, pPS);
                     WmFpItemDestroy (pStdControl);
                     bAlloc = True;
                  break;
               }
            }

            if (!bAlloc)
            {
               sprintf (sWarnBuf,
                        ((char*) GETMESSAGE (78, 1, sWarnMissingItem)),
                        pItem->ppchTokens[i]);
               Warning (sWarnBuf);
            }
         break;

         case WM_FP_TOK_switch:

            /*  Disallow multiple switches.  */

            if (O_Switch (pPanelist) != NULL)
            {
               sprintf (sWarnBuf,
                        ((char*) GETMESSAGE (78, 2, sWarnDuplicateItem)),
                        pItem->ppchTokens[++i]);
               Warning (sWarnBuf);
               break;
            }


            /*  Disallow switch in subpanel.  */

            else if (eTokType == WM_FP_TOK_subpanel)
            {
               sprintf (sWarnBuf,
                        ((char*) GETMESSAGE (78, 4, sWarnDisallowed)),
                        pItem->ppchTokens[++i], pItem->pchName);
               Warning (sWarnBuf);
               break;
            }

            iPos = WmFpListGetItemPos (O_Boxes (pPanelist),
                                       pItem->ppchTokens[++i]);
            if (iPos > 0)
            {
               pSwitch = WmFpListGetItem (O_Boxes (pPanelist), iPos);
               wPrev = AllocateSwitch (pPanelist, pSwitch, wBox,
                                       wPrev, pix, pPS);
            }
            else
               wPrev = AllocateSwitch (pPanelist, NULL, wBox, wPrev, pix, pPS);
         break;
      }
   }


   /*  Attach right side of last control.   */

   if (wPrev != NULL)
   {
      ac = 0;
      switch (eTokType)
      {       
         case WM_FP_TOK_primary:
            switch (O_SubpanelDirection (pPanelist))
            {
               case WM_FP_TOK_north:
               case WM_FP_TOK_south:
                  XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  ac++;
                  XtSetArg (al[ac], XmNrightOffset, 2);  ac++;
               break;

               case WM_FP_TOK_east:
               case WM_FP_TOK_west:
                  XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);  ac++;
                  XtSetArg (al[ac], XmNbottomOffset, 2);  ac++;
               break;
            }
         break;
      }

      XtSetValues (wPrev, al, ac);
   }


   /* Register default help string for subpanel.  */

   if ((eTokType == WM_FP_TOK_subpanel) && (sHelp == NULL) && (sTopic == NULL))
      XtAddCallback (wBox, XmNhelpCallback,
                     (XtCallbackProc) WmXvhStringHelpCB,
                     (XtPointer) PANEL_HELP);

   return (wBox);
}




/************************************************************************
 *
 *  AllocateSwitch
 *
 ************************************************************************/

#ifdef _NO_PROTO
static Widget
AllocateSwitch (pPanelist, pItem, wParent, wPrev, pix, pPS)
WmPanelistObject pPanelist;
WmFpItem         pItem;
Widget           wParent;
Widget           wPrev;
Pixmap           pix;
PixelSet         *pPS;

#else /* _NO_PROTO */
static Widget
AllocateSwitch (WmPanelistObject pPanelist,
                WmFpItem         pItem,
                Widget           wParent,
                Widget           wPrev,
                Pixmap           pix,
                PixelSet         *pPS)
#endif /* _NO_PROTO */

{
   Widget              switch_form;
   Widget              switch_rc;
   Widget              wControl;
   Widget              subpanel = NULL;
   Widget              control1 = NULL;
   Widget              control2 = NULL;
   Widget              control3 = NULL;
   Widget              control4 = NULL;
   Widget              arrow = NULL;
   Arg                 al[40];
   register int        ac;
   int                 ac_save;
   int                 i;
   int                 iNumRows = WM_FP_SWI_NUM_ROWS;
   int                 iNumWS = 0;
   XmString            xmStr;
   PixelSet            *pTogglePS = NULL;
   WmFpToken           eToken;
   WmScreenData        *pSD = O_ScreenData (pPanelist);
   String              sName;
   String              sRows;
   String              sColumns;
   String              sHelp = NULL;
   String              sTopic = NULL;
   String              sSubpanel = NULL;
   WmFpToken           eTokLayout = WM_FP_TOK_none;
   WmFpToken           eTokOrientation = WM_FP_TOK_none;
   Boolean             bAsNeeded = False;
   Boolean             bVertical = False;
   DtPanelShellWidget psw = (DtPanelShellWidget) O_Shell (pPanelist);
   Window              root = XRootWindowOfScreen (XtScreen (wParent));
   Display             *dpy = XtDisplay (wParent);
   int                 iNumWs;
   Atom                *paWsList;
   Status              statusWsList;


   /*  Get workspace list.  */

   statusWsList = DtWsmGetWorkspaceList (dpy, root, &iNumWs, &paWsList);


   /*  Process Switch tokens.  */

   for (i = 0; (pItem != NULL) && (i < pItem->iTokens); i++)
   {
      if (pItem->ppchTokens[i] == NULL)
         continue;

      eToken = WmFpStringToSwitchToken (pItem->ppchTokens[i]);

      switch (eToken)
      {
         case WM_FP_TOK_numberOfRows:
            sRows = pItem->ppchTokens[++i];

            if (sRows != NULL)
               iNumRows = atoi (sRows);
         break;

         case WM_FP_TOK_layoutPolicy:
            eTokLayout =
               WmFpStringToLayoutPolicyToken (pItem->ppchTokens[++i]);
            bAsNeeded = (eTokLayout == WM_FP_TOK_asNeeded);
         break;

         case WM_FP_TOK_orientation:
            eTokOrientation =
               WmFpStringToOrientationToken (pItem->ppchTokens[++i]);
               bVertical = (eTokOrientation != WM_FP_TOK_horizontal);
            break;

         case WM_FP_TOK_helpString:
            sHelp = pItem->ppchTokens[++i];
         break;

         case WM_FP_TOK_helpTopic:
            sTopic = pItem->ppchTokens[++i];
         break;
      }
   }


   /*  Validate name, number of workspaces, and switch rows.  */

   sName = (pItem != NULL) ? pItem->pchName : "switch"; 

   if (statusWsList == Success)
      iNumWS = iNumWs;
   else
      iNumWS = pSD->numWorkspaces;

   if ((iNumRows < 1) || (iNumRows > iNumWS))
      iNumRows = WM_FP_SWI_NUM_ROWS;


   /*  Set pixel and tile resources.  */

   ac = 0;
   XtSetArg (al[ac], XmNforeground, pPS->fg);  ac++;

   if (pix != XmUNSPECIFIED_PIXMAP)
   {
      XtSetArg (al[ac], XmNbackgroundPixmap, pix);  ac++;
   }
   else
   {
      XtSetArg (al[ac], XmNbackground, pPS->bg);  ac++;
   }

   if ((pSD->colorUse == B_W) || (pSD->pPixelData == NULL))
   {
      XtSetArg (al[ac], XmNtopShadowColor,
                WhitePixelOfScreen (XtScreen (wParent)));  ac++;
      XtSetArg (al[ac], XmNbottomShadowColor,
                BlackPixelOfScreen (XtScreen (wParent)));  ac++;
      XtSetArg (al[ac], XmNhighlightColor, pPS->fg);  ac++;
   }
   else
   {
      XtSetArg (al[ac], XmNtopShadowColor, pPS->ts);  ac++;
      XtSetArg (al[ac], XmNbottomShadowColor, pPS->bs);  ac++;
   }


   /*  Save the position within the resource list that is to be  */
   /*  by both the form and its contained row column             */

   ac_save = ac;

   if (!bAsNeeded)
   {
      switch (O_Resolution (pPanelist))
      {
         case HIGH_RES_DISPLAY:
            XtSetArg (al[ac], XmNheight, WM_FP_HI_PRI_HEIGHT);  ac++;
            XtSetArg (al[ac], XmNmarginWidth, WM_FP_HI_SWI_MARGIN_W);  ac++;
            XtSetArg (al[ac], XmNmarginHeight, WM_FP_HI_SWI_MARGIN_H);  ac++;
            XtSetArg (al[ac], XmNspacing, WM_FP_HI_SWI_SPACING);  ac++;
         break;

         case MED_RES_DISPLAY:
            XtSetArg (al[ac], XmNheight, WM_FP_MED_PRI_HEIGHT);  ac++;
            XtSetArg (al[ac], XmNmarginWidth, WM_FP_MED_SWI_MARGIN_W);  ac++;
            XtSetArg (al[ac], XmNmarginHeight, WM_FP_MED_SWI_MARGIN_H);  ac++;
            XtSetArg (al[ac], XmNspacing, WM_FP_MED_SWI_SPACING);  ac++;
         break;

         case LOW_RES_DISPLAY:
            XtSetArg (al[ac], XmNheight, WM_FP_LOW_PRI_HEIGHT);  ac++;
            XtSetArg (al[ac], XmNmarginWidth, WM_FP_LOW_SWI_MARGIN_W);  ac++;
            XtSetArg (al[ac], XmNmarginHeight, WM_FP_LOW_SWI_MARGIN_H);  ac++;
            XtSetArg (al[ac], XmNspacing, WM_FP_LOW_SWI_SPACING);  ac++;
         break;
      }
   }

   if (wPrev != NULL)
   {
      XtSetArg (al[ac], XmNleftAttachment, XmATTACH_WIDGET);  ac++;
      XtSetArg (al[ac], XmNleftWidget, wPrev);  ac++;

      switch (O_Resolution (pPanelist))
      {
         case HIGH_RES_DISPLAY:
            XtSetArg (al[ac], XmNleftOffset, WM_FP_HI_PRI_SPACING);  ac++;
         break;

         case MED_RES_DISPLAY:
            XtSetArg (al[ac], XmNleftOffset, WM_FP_MED_PRI_SPACING);  ac++;
         break;

         case LOW_RES_DISPLAY:
            XtSetArg (al[ac], XmNleftOffset, WM_FP_LOW_PRI_SPACING);  ac++;
         break;
      }
   }
   else
   {
      XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
      XtSetArg (al[ac], XmNleftOffset, 2);  ac++;
   }

   XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;
   XtSetArg (al[ac], XmNtopOffset, 1);  ac++;
   XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);  ac++;
   XtSetArg (al[ac], XmNbottomOffset, 1);  ac++;
   XtSetArg (al[ac], XmNshadowThickness, 1);  ac++;


   /*  Create switch form  */

   switch_form = XmCreateForm (wParent, sName, al, ac);
   XmRemoveTabGroup (switch_form);


   /*  Reuse the top part of the arg list and reset the bottom part   */
   /*  to set the values for the rowcolumn contained within the form  */

   ac = ac_save;

   XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;
   XtSetArg (al[ac], XmNtopOffset, WM_FP_HI_PRI_PIX_OFFSET);  ac++;
   XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
   XtSetArg (al[ac], XmNleftOffset, 1);  ac++;
   XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);  ac++;
   XtSetArg (al[ac], XmNbottomOffset, 1);  ac++;

   XtSetArg (al[ac], XmNspacing, 1);  ac++;
   XtSetArg (al[ac], XmNnumColumns, iNumRows);  ac++;
   XtSetArg (al[ac], XmNpacking, XmPACK_COLUMN);  ac++;
   XtSetArg (al[ac], XmNorientation, XmHORIZONTAL);  ac++;
   XtSetArg (al[ac], XmNadjustLast, False);  ac++;

   switch_rc = XmCreateRowColumn (switch_form, "switch_rc", al, ac);
   XtManageChild (switch_rc);
   XmRemoveTabGroup (switch_rc);


   /*  Create Switch Toggles  */

   for (i = 0; i < iNumWS; i++)
   {
      String        str;
      String        sAction;
      DtWsmWorkspaceInfo *pWsInfo;
      Status        statusWsInfo;
      int           iPS;


      /*  Try to get title and color set from window manager.  */

      if (statusWsList == Success)
      {
         statusWsInfo = DtWsmGetWorkspaceInfo (dpy, root, paWsList[i], 
                                                &pWsInfo);
         if (statusWsInfo == Success)
         {
            str = pWsInfo->pchTitle;
            iPS = pWsInfo->colorSetId - 1;
         }
      }


      /*  Use default title and color set.  */

      if ((statusWsList != Success) || (statusWsInfo != Success))
      {
         if (i == 5) iPS = 4;
         else if (i == 4) iPS = 5;
         else iPS = i;

         iPS = 7-(iPS%8);
      }

      pTogglePS = (O_DefaultColors (pPanelist) || (pSD->pPixelData == NULL) )
                   ? O_PixelSetDefault (pPanelist)
                   : &((pSD->pPixelData)[iPS]);


      /*  Set pixel resources.  */

      ac = 0;
      XtSetArg (al[ac], XmNforeground, pTogglePS->fg);  ac++;
      XtSetArg (al[ac], XmNbackground, pTogglePS->bg);  ac++;
      XtSetArg (al[ac], XmNpixmapBackground, pTogglePS->ts);  ac++;
      XtSetArg (al[ac], XmNpixmapForeground, pTogglePS->bs);  ac++;
      XtSetArg (al[ac], XmNarmColor, pTogglePS->bg);  ac++;
      XtSetArg (al[ac], XmNfillOnArm, False);  ac++;

      if ((pSD->colorUse == B_W) || (pSD->pPixelData == NULL))
      {
         XtSetArg (al[ac], XmNuseEmbossedText, False);  ac++;
         XtSetArg (al[ac], XmNpixmapBackground,
                   WhitePixelOfScreen (XtScreen (wParent)));  ac++;
         XtSetArg (al[ac], XmNpixmapForeground,
                   BlackPixelOfScreen (XtScreen (wParent)));  ac++;
      }
      else
      {
         XtSetArg (al[ac], XmNpixmapBackground, pTogglePS->ts);  ac++;
         XtSetArg (al[ac], XmNpixmapForeground, pTogglePS->bs);  ac++;
      }


      /*  Set label.  */

      if (pSD->pWS[i].title)
         xmStr = XmStringCopy (pSD->pWS[i].title);
      else
         xmStr = XmStringCreate (pSD->pWS[i].name, XmFONTLIST_DEFAULT_TAG);

      XtSetArg (al[ac], XmNstring, xmStr);  ac++;


      /*  Set the wm action for workspace switching  */

      XmStringGetLtoR (xmStr, XmFONTLIST_DEFAULT_TAG, &str);
      sAction = XtMalloc (strlen (str) + 20);
      sprintf (sAction, "f.goto_workspace \"%s\"", str);
      XtSetArg (al[ac], "pushAction", sAction);  ac++;

      XtSetArg (al[ac], XmNbehavior, XmICON_TOGGLE);  ac++;
      XtSetArg (al[ac], XmNfillMode, XmFILL_SELF);  ac++;
      XtSetArg (al[ac], XmNalignment, XmALIGNMENT_BEGINNING);  ac++;
      XtSetArg (al[ac], XmNpixmapPosition, XmPIXMAP_BOTTOM);  ac++;
      XtSetArg (al[ac], XmNhighlightThickness, 1);  ac++;
      XtSetArg (al[ac], XmNshadowThickness, 2);  ac++;
      XtSetArg (al[ac], XmNmarginWidth, 4);  ac++;
      XtSetArg (al[ac], XmNmarginHeight, 0);  ac++;
      XtSetArg (al[ac], XmNspacing, 0);  ac++;
      XtSetArg (al[ac], XmNcontrolType, XmCONTROL_SWITCH);  ac++;
      XtSetArg (al[ac], XmNfontList, O_FontList (pPanelist));  ac++;
      XtSetArg (al[ac], XmNuserData, pPanelist);  ac++;

      XtSetArg (al[ac], XmNrecomputeSize, False);  ac++;

      if (!bAsNeeded)
      {
         switch (O_Resolution (pPanelist))
         {
            case HIGH_RES_DISPLAY:
               XtSetArg (al[ac], XmNwidth, WM_FP_HI_SWI_WIDTH);  ac++;
               XtSetArg (al[ac], XmNheight, WM_FP_HI_SWI_HEIGHT);  ac++;
            break;

            case MED_RES_DISPLAY:
               XtSetArg (al[ac], XmNwidth, WM_FP_MED_SWI_WIDTH);  ac++;
               XtSetArg (al[ac], XmNheight, WM_FP_MED_SWI_HEIGHT);  ac++;
            break;

            case LOW_RES_DISPLAY:
               XtSetArg (al[ac], XmNwidth, WM_FP_LOW_SWI_WIDTH);  ac++;
               XtSetArg (al[ac], XmNheight, WM_FP_LOW_SWI_HEIGHT);  ac++;
            break;
         }
      }

      if (i == 0)
      {
         XtSetArg (al[ac], XmNset, True);  ac++;
      }

      wControl = _DtCreateControl (switch_rc, "toggle", al, ac);

      XtAddCallback (wControl, XmNcallback, (XtCallbackProc) ControlCB,
                     (XtPointer) pPanelist);
      XtManageChild (wControl);

      XmStringFree (xmStr);
      XtFree (sAction);


      /*  Check screen data  */

      if (i == 0)
      {
         O_SwitchLast (pPanelist) = wControl;
      }

      if ((statusWsList == Success) && (statusWsInfo == Success))
      {
         DtWsmFreeWorkspaceInfo (pWsInfo);
      }
   }

   if (statusWsList == Success)
      XFree ((char*) paWsList);

   if (sHelp != NULL)
      XtAddCallback (switch_rc, XmNhelpCallback,
                     (XtCallbackProc) WmXvhStringHelpCB,
                     (XtPointer) sHelp);

   if (sTopic != NULL)
      XtAddCallback (switch_rc, XmNhelpCallback,
                     (XtCallbackProc) WmXvhTopicHelpCB,
                     (XtPointer) sTopic);

   O_Switch (pPanelist) = switch_form;
   O_SwitchRC (pPanelist) = switch_rc;



   /*  Second pass through the switch tokens to process potential  */
   /*  subpanel and 4 controls.                                    */

   for (i = 0; (pItem != NULL) && (i < pItem->iTokens); i++)
   {
      if (pItem->ppchTokens[i] == NULL)
         continue;

      eToken = WmFpStringToSwitchToken (pItem->ppchTokens[i]);

      switch (eToken)
      {
         case WM_FP_TOK_control:
         {
            static int control_num = 0;
            Widget    w;
            Boolean   bAlloc = False;
            String    sName = NULL;
            WmFpToken eControlType = WM_FP_TOK_none;
            WmFpItem  pStdControl;
            int       iPos;

            iPos = WmFpListGetItemPos (O_Controls (pPanelist),
                                       pItem->ppchTokens[++i]);

            if (iPos > 0)
            {
               WmFpItem pControl;

               pControl = WmFpListGetItem (O_Controls (pPanelist), iPos);
               w = AllocateControl (pPanelist, pControl, switch_form, NULL,
                                    WM_FP_TOK_primary, pPS);
               bAlloc = True;
            }
            else
            {
               /*  Try to use name for control type.  */

               String sTemp = XtMalloc (strlen (pItem->ppchTokens[i]) + 1);

               strcpy (sTemp, pItem->ppchTokens[i]);
               _DtWmParseToLower ((unsigned char *) sTemp);
               eControlType = WmFpStringToControlTypeToken (sTemp);
               XtFree (sTemp);

               switch (eControlType)
               {
                  case WM_FP_TOK_blank:
                  case WM_FP_TOK_date:
                  case WM_FP_TOK_clock:
                     sName = (WmFpString) XtMalloc (strlen ((char *)
                                                    pItem->ppchTokens[i]) + 1);
                     strcpy ((char *) sName, (char *) pItem->ppchTokens[i]);
                     pStdControl = WmFpItemCreate (WM_FP_TOK_control,
                                                   sName, NULL, 0);
                     w = AllocateControl (pPanelist, pStdControl, switch_form,
                                          NULL, WM_FP_TOK_primary, pPS);
                     WmFpItemDestroy (pStdControl);
                     bAlloc = True;
                  break;
               }
            }

            if (!bAlloc)
            {
               sprintf (sWarnBuf,
                        ((char*) GETMESSAGE (78, 1, sWarnMissingItem)),
                        pItem->ppchTokens[i]);
               Warning (sWarnBuf);
            }
            else
            {
               control_num++;
               if (control_num == 1) control1 = w;
               else if (control_num == 2) control2 = w;
               else if (control_num == 3) control3 = w;
               else control4 = w;
            }
         }
         break;

         case WM_FP_TOK_subpanel:
         {
            String s1;
            int  iPos;
            WmFpItem pArrow;
            WmFpItem pBox;
            WmFpToken save_subpanel_direction;

            s1 = pItem->ppchTokens[++i];

            if (s1 == NULL) break;

            sSubpanel = XtMalloc (strlen ((char *) s1) + 1);
            strcpy ((char *) sSubpanel, (char *) s1);
            iPos = WmFpListGetItemPos (O_Boxes (pPanelist), s1);

            if (iPos > 0)
            {
               pBox = WmFpListGetItem (O_Boxes (pPanelist), iPos);

               subpanel = AllocateSubpanel (pPanelist, pBox, O_Shell (pPanelist));


               if (subpanel)
               {

                  /*  Force the subpanel's orientation and packing  */

                  ac = 0;
                  XtSetArg (al[ac], XmNorientation, XmHORIZONTAL);  ac++;
                  XtSetArg (al[ac], XmNpacking, XmPACK_TIGHT);  ac++;
                  XtSetValues (subpanel, al, ac);


                  /*  Create the arrow for posting the subpanel  */

                  pArrow = WmFpItemCreate (WM_FP_TOK_arrow,
                                           (WmFpString) sSubpanel, NULL, 0);

                  arrow = AllocateArrow (pPanelist, pArrow, switch_form, 
                                         switch_rc, subpanel, WM_FP_TOK_primary,
                                         pPS);

                  WmFpItemDestroy (pArrow);


                  /*  Set panelists switch subpanel field -- to the  */
                  /*  form widget, the immediate child of the shell  */

                  subpanel = (XtParent (subpanel));
                  O_SwitchSubpanel(pPanelist) = subpanel;


                  /*  Make subpanel adjustments to handle special    */
                  /*  case behaviors.                                */
                  /*  Force the unposting behavior for the switches  */
                  /*  subpanel to remain sticky.                     */
                  /*  Shrink the shadow thickness                    */

                  ac = 0;
                  XtSetArg (al[ac], XmNsubpanelUnpostOnSelect, FALSE);  ac++;
                  XtSetValues (subpanel, al, ac);
               }
            }
            else
            {
               sprintf (sWarnBuf,
                        ((char*) GETMESSAGE (78, 1, sWarnMissingItem)), s1);
               Warning (sWarnBuf);
               break;
            }
         }
         break;
      }
   }


   /*  If no subpanel was created, reset the top offset of the row column  */
   /*  and space out the switch button.                                    */

   if (subpanel == NULL)
   {
      ac = 0;
      XtSetArg (al[ac], XmNtopOffset, 1);  ac++;
      XtSetArg (al[ac], XmNspacing, 5);  ac++;
      XtSetValues (switch_rc, al, ac);
   }


   /*  Set up the contsraints for each of the controls  */

   if (control1 != NULL)
   {
      ac = 0;
      XtSetArg (al[ac], XmNtopOffset, 1);  ac++;
      XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;
      XtSetArg (al[ac], XmNleftOffset, 1);  ac++;
      XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
      XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_NONE);  ac++;
      XtSetValues (control1, al, ac);
   }

   if (control2 != NULL)
   {
      ac = 0;

      if (control1 != NULL)
      {
         XtSetArg (al[ac], XmNtopOffset, 2);  ac++;
         XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  ac++;
         XtSetArg (al[ac], XmNtopWidget, control1);  ac++;
      }
      else
      {
         XtSetArg (al[ac], XmNtopOffset, WM_FP_HI_PRI_PIX_OFFSET + 5);  ac++;
         XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;
      }

      XtSetArg (al[ac], XmNleftOffset, 1);  ac++;
      XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
      XtSetArg (al[ac], XmNbottomOffset, 1);  ac++;
      XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);  ac++;
      XtSetValues (control2, al, ac);
   }

   if (control3 != NULL)
   {
      ac = 0;
      XtSetArg (al[ac], XmNtopOffset, 1);  ac++;
      XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;
      XtSetArg (al[ac], XmNleftOffset, 1);  ac++;
      XtSetArg (al[ac], XmNleftAttachment, XmATTACH_WIDGET);  ac++;
      XtSetArg (al[ac], XmNleftWidget, switch_rc);  ac++;
      XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_NONE);  ac++;
      XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  ac++;
      XtSetArg (al[ac], XmNrightOffset, 1);  ac++;
      XtSetValues (control3, al, ac);
   }

   if (control4 != NULL)
   {
      ac = 0;

      if (control3 != NULL)
      {
         XtSetArg (al[ac], XmNtopOffset, 2);  ac++;
         XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  ac++;
         XtSetArg (al[ac], XmNtopWidget, control3);  ac++;
      }
      else
      {
         XtSetArg (al[ac], XmNtopOffset, WM_FP_HI_PRI_PIX_OFFSET + 5);  ac++;
         XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;
      }

      XtSetArg (al[ac], XmNleftAttachment, XmATTACH_WIDGET);  ac++;
      XtSetArg (al[ac], XmNleftWidget, switch_rc);  ac++;
      XtSetArg (al[ac], XmNleftOffset, 1);  ac++;
      XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  ac++;
      XtSetArg (al[ac], XmNrightOffset, 1);  ac++;
      XtSetArg (al[ac], XmNbottomOffset, 1);  ac++;
      XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);  ac++;
      XtSetValues (control4, al, ac);
   }


   /*  Readjusts the constraints for the switch row column and the  */
   /*  arrow based on whether other controls have been created.     */

   if (switch_rc != NULL)
   {
      Widget w;

      ac = 0;

      w = NULL;
      if (control1 != NULL)
         w = control1;
      else if (control2 != NULL)
         w = control2;
    
      if (w)
      {
         XtSetArg (al[ac], XmNleftAttachment, XmATTACH_WIDGET);  ac++;
         XtSetArg (al[ac], XmNleftWidget, w);  ac++;
      }

      w = NULL;
      if (control3 != NULL)
         w = control3;
      else if (control4 != NULL)
         w = control4;

      if (w == NULL)
      {
         XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  ac++;
         XtSetArg (al[ac], XmNrightOffset, 1);  ac++;
      }

      XtSetValues (switch_rc, al, ac);

      if (subpanel)
         XtSetValues (arrow, al, ac);
   }


   /*  Manage and return the switch  */

   XtManageChild (switch_form);
   XmRemoveTabGroup (switch_form);

   return (switch_form);
}




/************************************************************************
 *
 *  Allocate Arrow
 *      Create Button to open or close subpanel.
 *
 ************************************************************************/

#ifdef _NO_PROTO
static Widget
AllocateArrow (pPanelist, pItem, wParent, wPrev, wSubpanel, eTokBoxType, pPS)
WmPanelistObject pPanelist;
WmFpItem         pItem;
Widget           wParent;
Widget           wPrev;
Widget           wSubpanel;
WmFpToken        eTokBoxType;
PixelSet         *pPS;

#else /* _NO_PROTO */
static Widget
AllocateArrow (WmPanelistObject pPanelist,
               WmFpItem  pItem,
               Widget    wParent,
               Widget    wPrev,
               Widget    wSubpanel,
               WmFpToken eTokBoxType,
               PixelSet  *pPS)
#endif /* _NO_PROTO */

{
   Arg              al[30];
   register int     ac = 0;
   Widget           w;
   WmScreenData     *pSD = O_ScreenData (pPanelist);
   XmManagerWidget  mw = (XmManagerWidget) wSubpanel;
   DtControlGadget cg;
   static Pixel top_shadow_color;
   static Pixel bottom_shadow_color;
   static Pixel arm_color;
   static Boolean first = TRUE;


   /*  Must attach to previous widget.  */

   if (wPrev == NULL)
      return NULL;


   /*  Set the argument for the subpanel of the arrow  */

   XtSetArg (al[ac], XmNsubpanel, XtParent (wSubpanel));  ac++;


   /*  Set the pixel set for the arrow  */

   XtSetArg (al[ac], XmNforeground, pPS->fg);  ac++;
   XtSetArg (al[ac], XmNbackground, pPS->bg);  ac++;


   if ((pSD->colorUse == B_W) || (pSD->pPixelData == NULL))
   {
      XtSetArg (al[ac], XmNtopShadowColor,
                WhitePixelOfScreen (XtScreen (wParent)));  ac++;
      XtSetArg (al[ac], XmNbottomShadowColor,
                BlackPixelOfScreen (XtScreen (wParent)));  ac++;
      XtSetArg (al[ac], XmNhighlightColor, pPS->fg);  ac++;
      XtSetArg (al[ac], XmNarmColor, pPS->bg);  ac++;
   }
   else
   {
      XtSetArg (al[ac], XmNtopShadowColor, pPS->ts);  ac++;
      XtSetArg (al[ac], XmNbottomShadowColor, pPS->bs);  ac++;
      XtSetArg (al[ac], XmNarmColor, pPS->sc);  ac++;
   }


   switch (O_SubpanelDirection (pPanelist))
   {
      case WM_FP_TOK_north:
         XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;
         XtSetArg (al[ac], XmNtopOffset, 2);  ac++;
         XtSetArg (al[ac], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET);  ac++;
         XtSetArg (al[ac], XmNleftWidget, wPrev);  ac++;
         XtSetArg (al[ac], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET);  ac++;
         XtSetArg (al[ac], XmNrightWidget, wPrev);  ac++;
         XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_WIDGET);  ac++;
         XtSetArg (al[ac], XmNbottomWidget, wPrev);  ac++;
         XtSetArg (al[ac], XmNbottomOffset, 0);  ac++;
      break;


      /*  Structure for future front panel orientations.  */

      case WM_FP_TOK_south:
      case WM_FP_TOK_east:
      case WM_FP_TOK_west:
      break;
   }


   /*  Create Button.  */

   w = DtCreateButton (wParent, "arrow", al, ac);
   XtManageChild (w);


   /*  Set the arrows behavior, (the image, cursor, and callback functions  */

   ArrowSetBehavior (pPanelist, w, pItem->pchName, TRUE, TRUE);


   /*  Add the callback to handle posting and unposting of the subpanel  */

   XtAddCallback (w, XmNcallback, (XtCallbackProc) ArrowCB,
                  (XtPointer) pPanelist);

   XtAddCallback (XtParent (wSubpanel), XmNunmapCallback, 
                  (XtCallbackProc) SubpanelUnmapCB, (XtPointer) w);


   /*  Set the resource which closed te subpanel on the escape key.  */

   ac = 0;
   XtSetArg (al[ac], XmNcancelButton, w); ac++;
   XtSetValues (XtParent (wSubpanel), al, ac);

   return (w);
}




/************************************************************************
 *
 *  ArrowSetBehavior
 *      Set / Toggle the subpanel posting / unposting arrow's behaviour
 *	and visual.
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
ArrowSetBehavior (pPanelist, w, sSubpanel, new_subpanel, set_for_posting)
WmPanelistObject pPanelist;
Widget           w;
String           sSubpanel;
Boolean          new_subpanel;
Boolean          set_for_posting;

#else /* _NO_PROTO */
static void
ArrowSetBehavior (WmPanelistObject pPanelist,
                  Widget w,
                  String sSubpanel,
                  Boolean new_subpanel,
                  Boolean set_for_posting)
#endif /* _NO_PROTO */

{
   static Boolean    first = TRUE;
   static WmFunction post_wm_func;
   static WmFunction unpost_wm_func;
   static int        post_function_index;
   static int        post_cursor;
   static int        unpost_cursor;
   static String     post_image = NULL;
   static String     unpost_image = NULL;

   int        unpost_function_index;
   String     wm_func_parsed_arg = NULL;

   Arg al[4];
   int ac;

   String s1, s2;


   /*  If this is the first time within this function, allocate the   */
   /*  pixmaps for the arrows and initialize the data containing the  */
   /*  posting and unposting behavior.                                */

   if (first)
   {
      first = FALSE;

      switch (O_SubpanelDirection (pPanelist))
      {
         case WM_FP_TOK_north:
            post_image = GetImageName (pPanelist, WM_FP_UP_PIX);
            unpost_image = GetImageName (pPanelist, WM_FP_DOWN_PIX);
            post_cursor = XC_sb_up_arrow;
            unpost_cursor = XC_sb_down_arrow;
         break;

         case WM_FP_TOK_south:
         case WM_FP_TOK_east:
         case WM_FP_TOK_west:
         break;
      }


      /*  One time parse for the window manager subpanel post and  */
      /*  unpost function pointers.                                */

      s1 = "f.subpanel";
      post_function_index = 
         ParseWmFunction ((unsigned char **)&s1, CRS_ANY, &post_wm_func);

      s1 = "f.kill";
      unpost_function_index =
          ParseWmFunction ((unsigned char **)&s1, CRS_ANY, &unpost_wm_func);
   }


   /*  Create a new function argument once for each new subpanel.  */

   if (new_subpanel)
   {
      s2 = sSubpanel;
      ParseWmFunctionArg ((unsigned char **)&s2, post_function_index,
                          post_wm_func, 
                          (void **)&wm_func_parsed_arg, NULL, NULL);
   }


   /*  Build and arg list for the arrow image, cursor and callback  */
   /*  functions and then set the arrow.                            */

   ac = 0;
   if (set_for_posting)
   {
      XtSetArg (al[ac], XmNimageName, post_image);          ac++;
      XtSetArg (al[ac], XmNcursorFont, post_cursor);        ac++;
      XtSetArg (al[ac], XmNpushFunction, post_wm_func);     ac++;
   }
   else
   {
      XtSetArg (al[ac], XmNimageName, unpost_image);          ac++;
      XtSetArg (al[ac], XmNcursorFont, unpost_cursor);        ac++;
      XtSetArg (al[ac], XmNpushFunction, unpost_wm_func);     ac++;
   }

   if (new_subpanel)
   {
      XtSetArg (al[ac], XmNpushArgument, wm_func_parsed_arg);  ac++;
   }

   XtSetValues (w, al, ac);


}




/************************************************************************
 *
 *  Allocate Control
 *
 ************************************************************************/

#ifdef _NO_PROTO
static Widget
AllocateControl (pPanelist, pItem, wParent, wPrev, eTokBoxType, pPS)
WmPanelistObject pPanelist;
WmFpItem         pItem;
Widget           wParent;
Widget           wPrev;
WmFpToken        eTokBoxType;
PixelSet         *pPS;

#else /* _NO_PROTO */
static Widget
AllocateControl (WmPanelistObject pPanelist,
                 WmFpItem  pItem,
                 Widget    wParent,
                 Widget    wPrev,
                 WmFpToken eTokBoxType,
                 PixelSet  *pPS)
#endif /* _NO_PROTO */

{
   Widget                   wBox;
   Widget                   wControl;
   Arg                      al[40];
   register int             ac;
   int                      i;
   WmFpToken                eToken;
   WmFpToken                eTokBool;
   WmFpToken                eTokType = WM_FP_TOK_none;
   Boolean                  bSubpanel = False;
   Boolean                  bStringSet = False;
   Boolean                  bImageSet = False;
   Boolean                  bPushSet = False;
   Boolean                  bPushRecall = False;
   Boolean                  bAlloc = False;
   Boolean                  bBusy = False;
   String                   sName = pItem->pchName;
   String                   string = NULL;
   String                   sHelp = NULL;
   String                   sTopic = NULL;
   String                   sPushAnimation = NULL;
   String                   sDropAnimation = NULL;
   String                   sParsedArg = NULL;
   String                   sClient = NULL;
   String                   sTitle = NULL;
   String                   sPushAction = NULL;
   String                   sPushArg = NULL;
   String                   sImage = NULL;
   String                   sAltImage = NULL;
   String                   sSubpanel = NULL;
   String                   sGeometry = NULL;
   String                   sFile = NULL;
   String                   s1, s2;
   XmString                 xmStr = NULL;
   WmFunction               wmFunc;
   int                      ix;
   Widget                   wShell;
   int                      iPos;
   int                      iDelay;
   WmFpItem                 pArrow;
   WmFpItem                 pBox;
   WmFpItem                 pAnimation;
   WmFpEmbeddedClientData   *pClient = NULL;
   WmFpPushRecallClientData *pRecallClient = NULL;
   unsigned int             iSize;
   Dimension                iWidth, iHeight;
   WmScreenData             *pSD = O_ScreenData (pPanelist);
   CompositeWidget          cwPanel = (CompositeWidget) O_Panel (pPanelist);


   ac = 0;
   XtSetArg (al[ac], XmNuserData, (XtPointer) pPanelist);  ac++;

   for (i = 0; i < pItem->iTokens; i++)
   {
      if (pItem->ppchTokens[i] == NULL)
         continue;

      eToken = WmFpStringToControlToken (pItem->ppchTokens[i]);

      switch (eToken)
      {
         case WM_FP_TOK_type:
            s1 = pItem->ppchTokens[++i];

            if (s1 == NULL)
               break;

            eTokType = WmFpStringToControlTypeToken (s1);
         break;

         case WM_FP_TOK_clientGeometry:
            sGeometry = pItem->ppchTokens[++i];
         break;

         case WM_FP_TOK_clientName:
            sClient = pItem->ppchTokens[++i];
         break;

         case WM_FP_TOK_monitorFile:
            s1 = pItem->ppchTokens[++i];

            if (s1 == NULL)
               break;

            sFile = (String) _DtWmParseFilenameExpand ((unsigned char*) s1);
            XtSetArg (al[ac], XmNfileName, sFile);  ac++;
         break;

         case WM_FP_TOK_alternateImage:
            s1 = pItem->ppchTokens[++i];

            if (s1 == NULL)
               break;

            sAltImage = GetImageName (pPanelist, s1);

            if (sAltImage != NULL)
            {
               XtSetArg (al[ac], XmNalternateImage, sAltImage);  ac++;
            }
         break;

         case WM_FP_TOK_image:
            s1 = pItem->ppchTokens[++i];

            if (s1 == NULL)
               break;

            sImage = GetImageName (pPanelist, s1);

            if (sImage != NULL)
            {
               XtSetArg (al[ac], XmNimageName, sImage);  ac++;
               bImageSet = True;
            }
         break;

         case WM_FP_TOK_label:
            string = pItem->ppchTokens[++i];
            bStringSet = True;
         break;

         case WM_FP_TOK_pushAnimation:
            sPushAnimation = pItem->ppchTokens[++i];
         break;

         case WM_FP_TOK_dropAnimation:
            sDropAnimation = pItem->ppchTokens[++i];
         break;

         case WM_FP_TOK_dropAction:
            s1 = pItem->ppchTokens[++i];

            if (s1 == NULL)
               break;

            s2 = pItem->ppchTokens[++i];

            if (s2 == NULL)
               break;

            if (!strcmp (s1, "f.action"))
            {
               XtSetArg (al[ac], XmNdropAction, s2);  ac++;
            }
         break;

         case WM_FP_TOK_subpanel:
            s1 = pItem->ppchTokens[++i];

            if ((s1 == NULL) || (eTokBoxType != WM_FP_TOK_primary))
               break;

            sSubpanel = XtMalloc (strlen ((char *) s1) + 1);
            strcpy ((char *) sSubpanel, (char *) s1);
            iPos = WmFpListGetItemPos (O_Boxes (pPanelist), s1);

            if (iPos > 0)
            {
               /*  Allocate subpanel.  */

               pBox = WmFpListGetItem (O_Boxes (pPanelist), iPos);
               wBox = AllocateSubpanel (pPanelist, pBox, O_Shell (pPanelist));


               /*  If a subpanel was created, set an indicator for    */
               /*  later in the procedure to create the arrow within  */
               /*  the primary panel.                                 */

               if (wBox != NULL)
               {
                  bSubpanel = True;
                  pArrow = WmFpItemCreate (WM_FP_TOK_arrow,
                                           (WmFpString) sSubpanel, NULL, 0);
               }
            }
            else
            {
               sprintf (sWarnBuf,
                        ((char*) GETMESSAGE (78, 1, sWarnMissingItem)), s1);
               Warning (sWarnBuf);
               break;
            }
         break;

         case WM_FP_TOK_pushRecall:
            s1 = pItem->ppchTokens[++i];

            if (s1 == NULL)
               break;

            eTokBool = WmFpStringToBooleanToken (s1); 

            if (eTokBool == WM_FP_TOK_true)
               bPushRecall = True;
         break;

         case WM_FP_TOK_pushAction:
            s1 = pItem->ppchTokens[++i];

            if (s1 == NULL)
               break;

            s2 = pItem->ppchTokens[++i];

            if (! strcmp (s1, "f.subpanel"))
               break;

            sPushAction = s1;
            sPushArg = s2;
         break;

         case WM_FP_TOK_helpString:
            sHelp = pItem->ppchTokens[++i];
         break;

         case WM_FP_TOK_helpTopic:
            sTopic = pItem->ppchTokens[++i];
         break;

         case WM_FP_TOK_title:
            sTitle = pItem->ppchTokens[++i];
         break;
        
         default:
            ++i;
         break;
      }
   }

   XtSetArg (al[ac], XmNmarginWidth, 0);  ac++;
   XtSetArg (al[ac], XmNmarginHeight, 0);  ac++;
   XtSetArg (al[ac], XmNspacing, 0);  ac++;
   XtSetArg (al[ac], XmNbehavior, XmICON_BUTTON);  ac++;
   XtSetArg (al[ac], XmNshadowType, XmSHADOW_OUT);  ac++;
   XtSetArg (al[ac], XmNfillOnArm, True);  ac++;
   XtSetArg (al[ac], XmNfontList, O_FontList (pPanelist));  ac++;


   /*  Set pixel resources.  */

   XtSetArg (al[ac], XmNforeground, pPS->fg);  ac++;
   XtSetArg (al[ac], XmNbackground, pPS->bg);  ac++;

   if ((pSD->colorUse == B_W) || (pSD->pPixelData == NULL))
   {
      XtSetArg (al[ac], XmNuseEmbossedText, False);  ac++;
      XtSetArg (al[ac], XmNarmColor, pPS->bg);  ac++;
   }
   else
   {
      XtSetArg (al[ac], XmNarmColor, pPS->sc);  ac++;
   }


   /*  Use background for color set matching.  */
   /*  Use top shadow for clock hand color.    */

   if (O_UseMultiColorIcons (pPanelist))
   {
      XtSetArg (al[ac], XmNpixmapBackground, pPS->bg);  ac++;
      XtSetArg (al[ac], XmNpixmapForeground, pPS->ts);  ac++;
   }
   else
   {
      /*  Use black and white for bitonal icons.  */

      if (eTokType == WM_FP_TOK_clock)
      {
         if (pSD->colorUse == B_W)
         {
            XtSetArg (al[ac], XmNpixmapBackground, pPS->fg);  ac++;
            XtSetArg (al[ac], XmNpixmapForeground, pPS->bg);  ac++;
         }
         else
         {
            XtSetArg (al[ac], XmNpixmapBackground,
                      BlackPixelOfScreen (XtScreen (wParent)));  ac++;
            XtSetArg (al[ac], XmNpixmapForeground,
                      WhitePixelOfScreen (XtScreen (wParent)));  ac++;
         }
      }
      else
      {
         XtSetArg (al[ac], XmNpixmapBackground,
                   WhitePixelOfScreen (XtScreen (wParent)));  ac++;
         XtSetArg (al[ac], XmNpixmapForeground,
                   BlackPixelOfScreen (XtScreen (wParent)));  ac++;
      }
   }

   switch (eTokBoxType)
   {
      case WM_FP_TOK_primary:
         XtSetArg (al[ac], XmNfillMode, XmFILL_SELF);  ac++;
         XtSetArg (al[ac], XmNalignment, XmALIGNMENT_CENTER);  ac++;
         XtSetArg (al[ac], XmNpixmapPosition, XmPIXMAP_TOP);  ac++;

         switch (O_SubpanelDirection (pPanelist))
         {
            case WM_FP_TOK_north:
            case WM_FP_TOK_south:
               if (wPrev != NULL)
               {
                  XtSetArg (al[ac], XmNleftAttachment, XmATTACH_WIDGET);  ac++;
                  XtSetArg (al[ac], XmNleftWidget, wPrev);  ac++;

                  switch (O_Resolution (pPanelist))
                  {
                     case HIGH_RES_DISPLAY:
                        XtSetArg (al[ac], XmNleftOffset, WM_FP_HI_PRI_SPACING);  ac++;
                     break;

                     case MED_RES_DISPLAY:
                        XtSetArg (al[ac], XmNleftOffset, WM_FP_MED_PRI_SPACING);  ac++;
                     break;

                     case LOW_RES_DISPLAY:
                        XtSetArg (al[ac], XmNleftOffset, WM_FP_LOW_PRI_SPACING);  ac++;
                     break;
                  }
               }
               else
               {
                  XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
                  XtSetArg (al[ac], XmNleftOffset, 2);  ac++;
               }

               XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;

               switch (O_Resolution (pPanelist))
               {
                  case HIGH_RES_DISPLAY:
                     XtSetArg (al[ac], XmNtopOffset, WM_FP_HI_PRI_PIX_OFFSET + 2);  ac++;
                  break;

                  case MED_RES_DISPLAY:
                     XtSetArg (al[ac], XmNtopOffset, WM_FP_MED_PRI_PIX_OFFSET + 2);  ac++;
                  break;

                  case LOW_RES_DISPLAY:
                     XtSetArg (al[ac], XmNtopOffset, WM_FP_LOW_PRI_PIX_OFFSET + 2);  ac++;
                  break;
               }

               XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);  ac++;
               XtSetArg (al[ac], XmNbottomOffset, 2);  ac++;
            break;

            case WM_FP_TOK_east:
            case WM_FP_TOK_west:
               if (wPrev != NULL)
               {
                  XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  ac++;
                  XtSetArg (al[ac], XmNtopWidget, wPrev);  ac++;

                  switch (O_Resolution (pPanelist))
                  {
                     case HIGH_RES_DISPLAY:
                        XtSetArg (al[ac], XmNtopOffset, WM_FP_HI_PRI_SPACING);  ac++;
                     break;

                     case MED_RES_DISPLAY:
                        XtSetArg (al[ac], XmNtopOffset, WM_FP_MED_PRI_SPACING);  ac++;
                     break;

                     case LOW_RES_DISPLAY:
                        XtSetArg (al[ac], XmNtopOffset, WM_FP_LOW_PRI_SPACING);  ac++;
                     break;
                  }
               }
               else
               {
                  XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;
                  XtSetArg (al[ac], XmNtopOffset, 2);  ac++;
               }

               XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  ac++;

               switch (O_Resolution (pPanelist))
               {
                  case HIGH_RES_DISPLAY:
                     XtSetArg (al[ac], XmNrightOffset, WM_FP_HI_PRI_PIX_OFFSET + 2);  ac++;
                  break;

                  case MED_RES_DISPLAY:
                     XtSetArg (al[ac], XmNrightOffset, WM_FP_MED_PRI_PIX_OFFSET + 2);  ac++;
                  break;

                  case LOW_RES_DISPLAY:
                     XtSetArg (al[ac], XmNrightOffset, WM_FP_LOW_PRI_PIX_OFFSET + 2);  ac++;
                  break;
               }

               XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
               XtSetArg (al[ac], XmNleftOffset, 2);  ac++;
            break;
         }
      break;

      case WM_FP_TOK_subpanel:
         XtSetArg (al[ac], XmNfillMode, XmFILL_SELF);  ac++;
         XtSetArg (al[ac], XmNpixmapPosition, XmPIXMAP_LEFT);  ac++;
         XtSetArg (al[ac], XmNalignment, XmALIGNMENT_CENTER);  ac++;
      break;
   }


   /*  Try to use name for control type.  */

   if (eTokType == WM_FP_TOK_none)
   {
      String sTemp = XtMalloc (strlen (pItem->pchName) + 1);

      strcpy (sTemp, pItem->pchName);
      eTokType = WmFpStringToControlTypeToken (sTemp);
      XtFree (sTemp);
   }


   /*  Guess monitor if file specified.  */

   if ((eTokType == WM_FP_TOK_none) && (sFile != NULL))
      eTokType = WM_FP_TOK_monitorFile;


   /*  Set resources based on control type.  */

   switch (eTokType)
   {
      case WM_FP_TOK_blank:
         XtSetArg (al[ac], XmNcontrolType, XmCONTROL_BLANK);  ac++;
      break;

      case WM_FP_TOK_date:
         XtSetArg (al[ac], XmNcontrolType, XmCONTROL_DATE);  ac++;
         XtSetArg (al[ac], XmNshadowThickness, 0);  ac++;
      break;

      case WM_FP_TOK_client:
         XtSetArg (al[ac], XmNcontrolType, XmCONTROL_CLIENT);  ac++;
         XtSetArg (al[ac], XmNshadowThickness, 2);  ac++;
      break;

      case WM_FP_TOK_busy:
         XtSetArg (al[ac], XmNcontrolType, XmCONTROL_BUSY);  ac++;
         XtSetArg (al[ac], XmNshadowThickness, 1);  ac++;
         bBusy = True;
      break;

      case WM_FP_TOK_mail:
         XtSetArg (al[ac], XmNcontrolType, XmCONTROL_MAIL);  ac++;
         XtSetArg (al[ac], XmNshadowThickness, 1);  ac++;
      break;

      case WM_FP_TOK_monitorFile:
         XtSetArg (al[ac], XmNcontrolType, XmCONTROL_MONITOR);  ac++;
         XtSetArg (al[ac], XmNshadowThickness, 1);  ac++;
      break;

      case WM_FP_TOK_clock:
         XtSetArg (al[ac], XmNshadowThickness, 0);  ac++;
      break;

      default:
      case WM_FP_TOK_button:
      case WM_FP_TOK_iconBox:
         XtSetArg (al[ac], XmNcontrolType, XmCONTROL_BUTTON);  ac++;
         XtSetArg (al[ac], XmNshadowThickness, 1);  ac++;
      break;
   }


   switch (eTokType)
   {
      case WM_FP_TOK_clock:
         XtSetArg (al[ac], XmNhighlightThickness, 1);  ac++;

         switch (O_Resolution (pPanelist))
         {
            case HIGH_RES_DISPLAY:
               XtSetArg (al[ac], XmNwidth, WM_FP_HI_PRI_WIDTH);  ac++;
               XtSetArg (al[ac], XmNheight,
                  WM_FP_HI_PRI_HEIGHT - WM_FP_HI_PRI_PIX_OFFSET);  ac++;
               XtSetArg (al[ac], XmNleftInset, WM_FP_HI_PRI_CLOCK_LEFT);  ac++;
               XtSetArg (al[ac], XmNrightInset, WM_FP_HI_PRI_CLOCK_RIGHT);  ac++;
               XtSetArg (al[ac], XmNtopInset, WM_FP_HI_PRI_CLOCK_TOP);  ac++;
               XtSetArg (al[ac], XmNbottomInset, WM_FP_HI_PRI_CLOCK_BOTTOM);  ac++;
            break;

            case MED_RES_DISPLAY:
               XtSetArg (al[ac], XmNwidth, WM_FP_MED_PRI_WIDTH);  ac++;
               XtSetArg (al[ac], XmNheight,
                  WM_FP_MED_PRI_HEIGHT - WM_FP_MED_PRI_PIX_OFFSET);  ac++;
               XtSetArg (al[ac], XmNleftInset, WM_FP_MED_PRI_CLOCK_LEFT);  ac++;
               XtSetArg (al[ac], XmNrightInset, WM_FP_MED_PRI_CLOCK_RIGHT);  ac++;
               XtSetArg (al[ac], XmNtopInset, WM_FP_MED_PRI_CLOCK_TOP);  ac++;
               XtSetArg (al[ac], XmNbottomInset, WM_FP_MED_PRI_CLOCK_BOTTOM);  ac++;
            break;

            case LOW_RES_DISPLAY:
               XtSetArg (al[ac], XmNwidth, WM_FP_LOW_PRI_WIDTH);  ac++;
               XtSetArg (al[ac], XmNheight,
                  WM_FP_LOW_PRI_HEIGHT - WM_FP_LOW_PRI_PIX_OFFSET);  ac++;
               XtSetArg (al[ac], XmNleftInset, WM_FP_LOW_PRI_CLOCK_LEFT);  ac++;
               XtSetArg (al[ac], XmNrightInset, WM_FP_LOW_PRI_CLOCK_RIGHT);  ac++;
               XtSetArg (al[ac], XmNtopInset, WM_FP_LOW_PRI_CLOCK_TOP);  ac++;
               XtSetArg (al[ac], XmNbottomInset, WM_FP_LOW_PRI_CLOCK_BOTTOM);  ac++;
            break;
         }

         sImage = GetImageName (pPanelist, WM_FP_CLOCK_PIX);

         if (sImage != NULL)
         {
            XtSetArg (al[ac], XmNimageName, sImage);  ac++;
         }

         XtSetArg (al[ac], XmNstring, NULL);  ac++;
         XtSetArg (al[ac], XmNbehavior, XmICON_BUTTON);  ac++;
      break;

      case WM_FP_TOK_blank:
         XtSetArg (al[ac], XmNbehavior, XmICON_LABEL);  ac++;
         XtSetArg (al[ac], XmNhighlightThickness, 0);  ac++;
         XtSetArg (al[ac], XmNstring, NULL);  ac++;
      break;

      case WM_FP_TOK_date:

         /* Set standard height.  */

         switch (O_Resolution (pPanelist))
         {
            case HIGH_RES_DISPLAY:
               XtSetArg (al[ac], XmNheight,
                         WM_FP_HI_PRI_HEIGHT - WM_FP_HI_PRI_PIX_OFFSET); ac++;
            break;

            case MED_RES_DISPLAY:
               XtSetArg (al[ac], XmNheight,
                         WM_FP_MED_PRI_HEIGHT - WM_FP_MED_PRI_PIX_OFFSET); ac++;
            break;

            case LOW_RES_DISPLAY:
               XtSetArg (al[ac], XmNheight,
                         WM_FP_LOW_PRI_HEIGHT - WM_FP_LOW_PRI_PIX_OFFSET); ac++;
            break;
         }

         XtSetArg (al[ac], XmNbehavior, XmICON_BUTTON);  ac++;
         XtSetArg (al[ac], XmNhighlightThickness, 1);  ac++;
      break;

      case WM_FP_TOK_client:
         XtSetArg (al[ac], XmNbehavior, XmICON_BUTTON);  ac++;
         XtSetArg (al[ac], XmNhighlightThickness, 1);  ac++;
         XtSetArg (al[ac], XmNstring, NULL);  ac++;

         switch (eTokBoxType)
         {
            case WM_FP_TOK_primary:
               switch (O_Resolution (pPanelist))
               {
                  case HIGH_RES_DISPLAY:
                     iWidth = WM_FP_HI_PRI_WIDTH;
                     iHeight = WM_FP_HI_PRI_HEIGHT - WM_FP_HI_PRI_PIX_OFFSET;
                  break;

                  case MED_RES_DISPLAY:
                     iWidth = WM_FP_MED_PRI_WIDTH;
                     iHeight = WM_FP_MED_PRI_HEIGHT - WM_FP_MED_PRI_PIX_OFFSET;
                  break;

                  case LOW_RES_DISPLAY:
                     iWidth = WM_FP_LOW_PRI_WIDTH;
                     iHeight = WM_FP_LOW_PRI_HEIGHT - WM_FP_LOW_PRI_PIX_OFFSET;
                  break;
               }
            break;

            case WM_FP_TOK_subpanel:
               switch (O_Resolution (pPanelist))
               {
                  case HIGH_RES_DISPLAY:
                     iWidth = WM_FP_HI_SUB_WIDTH;
                     iHeight = WM_FP_HI_SUB_HEIGHT;
                  break;

                  case MED_RES_DISPLAY:
                     iWidth = WM_FP_MED_SUB_WIDTH;
                     iHeight = WM_FP_MED_SUB_HEIGHT;
                  break;

                  case LOW_RES_DISPLAY:
                     iWidth = WM_FP_LOW_SUB_WIDTH;
                     iHeight = WM_FP_LOW_SUB_HEIGHT;
                  break;
               }
            break;
         }

         if ((eTokType == WM_FP_TOK_client) && (sGeometry != NULL))
         {
            int           iMask;
            int           iLeft, iTop, iBottom, iRight;
            int           iClientX, iClientY;
            unsigned int  iClientWidth, iClientHeight;

            switch (eTokBoxType)
            {
               case WM_FP_TOK_primary:
                  switch (O_Resolution (pPanelist))
                  {
                     case HIGH_RES_DISPLAY:
                        iLeft = WM_FP_HI_PRI_INSET_LEFT;
                        iTop = WM_FP_HI_PRI_INSET_TOP;
                        iBottom = WM_FP_HI_PRI_INSET_BOTTOM;
                        iRight = WM_FP_HI_PRI_INSET_RIGHT;
                     break;

                     case MED_RES_DISPLAY:
                        iLeft = WM_FP_MED_PRI_INSET_LEFT;
                        iTop = WM_FP_MED_PRI_INSET_TOP;
                        iBottom = WM_FP_MED_PRI_INSET_BOTTOM;
                        iRight = WM_FP_MED_PRI_INSET_RIGHT;
                     break;

                     case LOW_RES_DISPLAY:
                        iLeft = WM_FP_LOW_PRI_INSET_LEFT;
                        iTop = WM_FP_LOW_PRI_INSET_TOP;
                        iBottom = WM_FP_LOW_PRI_INSET_BOTTOM;
                        iRight = WM_FP_LOW_PRI_INSET_RIGHT;
                     break;
                  }
               break;

               case WM_FP_TOK_subpanel:
                  iLeft = iTop = iBottom = iRight = 3;
               break;
            }

            iMask = XParseGeometry (sGeometry, &iClientX, &iClientY,
                                    &iClientWidth, &iClientHeight);

            if ((iMask & WidthValue) && (iClientWidth > 0))
               iWidth = iClientWidth + iLeft + iRight;

            if ((iMask & HeightValue) && (iClientHeight > 0))
               iHeight = iClientHeight + iTop + iBottom;
         }

         XtSetArg (al[ac], XmNwidth, iWidth);  ac++;
         XtSetArg (al[ac], XmNheight, iHeight);  ac++;
      break;

      default:
      case WM_FP_TOK_button:
      case WM_FP_TOK_busy:
      case WM_FP_TOK_mail:
      case WM_FP_TOK_monitorFile:
         XtSetArg (al[ac], XmNhighlightThickness, 1);  ac++;

         if (string != NULL)
         {
            xmStr = XmStringCreate (string, XmFONTLIST_DEFAULT_TAG);
            XtSetArg (al[ac], XmNstring, xmStr);  ac++;
         }
         else if (bImageSet)
         {
            XtSetArg (al[ac], XmNstring, NULL);  ac++;
         }
      break;
   }


   if (eTokType != WM_FP_TOK_clock)
   {
      /*  Disallow multiple busy controls  */

      if ((eTokType == WM_FP_TOK_busy) && (O_Busy (pPanelist) != NULL))
      {
         sprintf (sWarnBuf, ((char*) GETMESSAGE (78, 2, sWarnDuplicateItem)),
                  sName);
         Warning (sWarnBuf);
         return NULL;
      }

      /*  Disallow busy control if subpanel box.  */

      else if ((eTokType == WM_FP_TOK_busy) &&
               (eTokBoxType == WM_FP_TOK_subpanel))
      {
         sprintf (sWarnBuf, ((char*) GETMESSAGE (78, 4, sWarnDisallowed)),
                  sName, XtName (wParent));
         Warning (sWarnBuf);
         return NULL;
      }

      wControl = _DtCreateControl (wParent, sName, al, ac);
   }
   else
      wControl = DtCreateClock (wParent, sName, al, ac);

   XtAddCallback (wControl, XmNcallback, (XtCallbackProc) ControlCB,
                  (XtPointer) pPanelist);
   XtManageChild (wControl);

   XmStringFree (xmStr);
   XtFree (sFile);
   XtFree (sImage);
   XtFree (sAltImage);

   if (XtWidth (wControl) % 2 == 1)
   {
      ac = 0;
      XtSetArg (al[ac], XmNwidth, XtWidth (wControl) + 1);  ac++;
      XtSetValues (wControl, al, ac);
   }

   ac = 0;
   if (eTokType == WM_FP_TOK_iconBox) 
   {
      RegisterIconBoxControl ((Widget) pPanelist);

      if (sPushAction == NULL)
      {
         sPushAction = "f.normalize_and_raise";
         sPushArg = "-iconbox";
      }
   }


   if (sPushAction != NULL)
   {
      s1 = sPushAction;
      s2 = sPushArg;

      if (sClient == NULL)
        sClient = pItem->pchName;

      ix = ParseWmFunction ((unsigned char **)&s1, CRS_ANY, &wmFunc);

      if (ParseWmFunctionArg ((unsigned char **)&s2, ix, wmFunc,
                              (void **)&sParsedArg, sClient, sTitle))
      {
         if (bPushRecall)
         {
            WmPushRecallArg *pRArg;

            if (O_NumRecallClients (pPanelist) ==
                O_MaxRecallClients (pPanelist))
            {
               O_MaxRecallClients (pPanelist) += WM_FP_MAX_ITEMS;
               O_RecallClients (pPanelist) = (WmFpPushRecallClientList)
                  XtRealloc ((char*) O_RecallClients (pPanelist),
                             sizeof (WmFpPushRecallClientData) *
                             O_MaxRecallClients (pPanelist));
            }

            pRecallClient = &(O_RecallClients (pPanelist)[
                              O_NumRecallClients (pPanelist)++]);
            pRArg = XtNew (WmPushRecallArg);
            pRArg->ixReg = O_NumRecallClients (pPanelist) - 1;
            pRArg->wmFunc = wmFunc;
            pRArg->pArgs = sParsedArg;
            pRecallClient->pRArg = (XtPointer) pRArg;
            pRecallClient->pchResName = XtMalloc (strlen (sClient) + 1);
            strcpy (pRecallClient->pchResName, sClient);
            pRecallClient->wControl = wControl;
            pRecallClient->pCD = NULL;

            XtSetArg (al[ac], XmNpushFunction, F_Push_Recall);  ac++;
            XtSetArg (al[ac], XmNpushArgument, pRArg);  ac++;
         }
         else
         {
            XtSetArg (al[ac], XmNpushFunction, wmFunc);  ac++;
            XtSetArg (al[ac], XmNpushArgument, sParsedArg);  ac++;
         }
      }

      bPushSet = True;
   }


   if (!bPushSet)
      XtSetSensitive (wControl, False);

   if ((sPushAnimation != NULL) && (eTokType != WM_FP_TOK_clock) &&
       (eTokType != WM_FP_TOK_date) && (eTokType != WM_FP_TOK_client))
   {
      int    iDelaySet = WM_FP_DEFAULT_ANIMATION_DELAY;
      String sDelay;

      iPos = WmFpListGetItemPos (O_Animations (pPanelist), sPushAnimation);

      if (iPos > 0)
      {
         pAnimation = WmFpListGetItem (O_Animations (pPanelist), iPos);

         for (i = 0; i < pAnimation->iTokens; i++)
         {
            if (pAnimation->ppchTokens[i] == NULL) 
               continue;

            sImage = GetImageName (pPanelist, pAnimation->ppchTokens[i]);

            if (sImage != NULL)
            {
               sDelay = (++i < pAnimation->iTokens)
                        ? pAnimation->ppchTokens[i] : NULL;
               iDelay = (sDelay != NULL) ? atoi (sDelay) : 0;

               if (iDelay > 0) iDelaySet = iDelay;

               if (iDelay == 0) iDelay = iDelaySet;

               _DtControlAddPushAnimationImage (wControl, sImage, iDelay);
            }
         }
      }
      else
      {
         sprintf (sWarnBuf,
                 ((char*) GETMESSAGE (78, 1, sWarnMissingItem)),
                 sPushAnimation);
         Warning (sWarnBuf);
      }
   }

   if ((sDropAnimation != NULL) && (eTokType != WM_FP_TOK_clock) &&
       (eTokType != WM_FP_TOK_date) && (eTokType != WM_FP_TOK_client))
   {
      int    iDelaySet = WM_FP_DEFAULT_ANIMATION_DELAY;
      String sDelay;

      iPos = WmFpListGetItemPos (O_Animations (pPanelist), sDropAnimation);

      if (iPos > 0)
      {
         pAnimation = WmFpListGetItem (O_Animations (pPanelist), iPos);

         for (i = 0; i < pAnimation->iTokens; i++)
         {
            if (pAnimation->ppchTokens[i] == NULL) 
               continue;

            sImage = GetImageName (pPanelist, pAnimation->ppchTokens[i]);

            if (sImage != NULL)
            {
               sDelay = (++i < pAnimation->iTokens)
                        ? pAnimation->ppchTokens[i] : NULL;
               iDelay = (sDelay != NULL) ? atoi (sDelay) : 0;

               if (iDelay > 0) iDelaySet = iDelay;

               if (iDelay == 0) iDelay = iDelaySet;

               _DtControlAddDropAnimationImage (wControl, sImage, iDelay);
            }
         }
      }
      else
      {
         sprintf (sWarnBuf,
                 ((char*) GETMESSAGE (78, 1, sWarnMissingItem)),
                 sDropAnimation);
         Warning (sWarnBuf);
      }
   }

   if (ac > 0)
      XtSetValues (wControl, al, ac);


   /*  Save embedded client information.  */

   if (eTokType == WM_FP_TOK_client)
   {
      if (O_NumEmbeddedClients (pPanelist) == O_MaxEmbeddedClients (pPanelist))
      {
         O_MaxEmbeddedClients (pPanelist) += WM_FP_MAX_ITEMS;
         O_EmbeddedClients (pPanelist) = (WmFpEmbeddedClientList)
            XtRealloc ((char*) O_EmbeddedClients (pPanelist),
                       sizeof (WmFpEmbeddedClientData) *
                       O_MaxEmbeddedClients (pPanelist));
      }

      pClient = &(O_EmbeddedClients 
                  (pPanelist)[O_NumEmbeddedClients (pPanelist)++]);
                        
      if (sClient == NULL)
         sClient = pItem->pchName;

      pClient->pchResName = XtMalloc (strlen (sClient) + 1);
      strcpy (pClient->pchResName, sClient);
      pClient->wControl = wControl;
      pClient->winParent = NULL;
      pClient->x = 0;
      pClient->y = 0;
      pClient->width = 0;
      pClient->height = 0;
      pClient->pCD = NULL;
   }


   /*  Allocate open control for subpanel.  */

   if (bSubpanel)
   {
      AllocateArrow (pPanelist, pArrow, wParent, 
                     wControl, wBox, WM_FP_TOK_primary, pPS);
      WmFpItemDestroy (pArrow);
   }


   /*  Add Help callback.  */

   if (sHelp != NULL)
      XtAddCallback (wControl, XmNhelpCallback,
                     (XtCallbackProc) WmXvhStringHelpCB, (XtPointer) sHelp);

   if (sTopic != NULL)
      XtAddCallback (wControl, XmNhelpCallback,
                     (XtCallbackProc) WmXvhTopicHelpCB, (XtPointer) sTopic);

   if (bBusy)
      O_Busy (pPanelist) = wControl;

   return wControl;
}



/*-------------------------------------------------------------
**      Core Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**      ClassInitialize
**              Initialize gadget class.
*/

#if 0
static void ClassInitialize ()
{
}
#endif




/*-------------------------------------------------------------
**      ClassPartInitialize
**              Initialize class part.
*/

#if 0
static void ClassPartInitialize (wc)
WidgetClass wc;
{
        WmPanelistClass oc =    (WmPanelistClass) wc;
        WmPanelistClass super = (WmPanelistClass) C_Superclass (wc);
}
#endif




/*-------------------------------------------------------------
**      Initialize
**              Initialize a new gadget instance.
*/

/* ARGSUSED */
static void Initialize (request_w, new_w)
Widget request_w, new_w;
{
   WmPanelistObject  request = (WmPanelistObject) request_w,
                     new = (WmPanelistObject) new_w;
   String            name_string = O_PanelName (request);
   Boolean           useMaskRtn, useMultiColorIconsRtn, useIconFileCacheRtn;
   PixelSet          *pPS = O_PixelSetDefault (new);
   WmScreenData      *pSD = O_ScreenData (new);
   Screen            *pScr = XtScreen (new_w);
   Display           *pDpy = XtDisplay (new_w);
   Pixel             pixelB = BlackPixelOfScreen (pScr);
   Pixel             pixelW = WhitePixelOfScreen (pScr);
   Pixmap            pixmapBusy;
   Pixmap            pixmapBusyMask;
   XColor            xcolors[2];


   /*  Create busy cursor.  */

   xcolors[0].pixel = pixelB;
   xcolors[1].pixel = pixelW;
   XQueryColors (pDpy, DefaultColormapOfScreen (pScr), xcolors, 2);
   pixmapBusy = XCreateBitmapFromData (pDpy, RootWindowOfScreen (pScr),
                                time16_bits, time16_width, time16_height);
   pixmapBusyMask = XCreateBitmapFromData (pDpy, RootWindowOfScreen (pScr),
                                time16m_bits, time16_width, time16_height);
   O_CursorBusy (new) = XCreatePixmapCursor (pDpy, pixmapBusy, pixmapBusyMask,
                                             &(xcolors[0]), &(xcolors[1]),
                                time16_x_hot, time16_y_hot);
   XFreePixmap (pDpy, pixmapBusy);
   XFreePixmap (pDpy, pixmapBusyMask);

                                
   /*  Set default tiles and pixels.  */

   pPS->fg = pixelW;
   pPS->bg = pixelB;
   pPS->ts = pixelW;
   pPS->bs = pixelW;
   pPS->sc = pixelB;


   /*  See if using bitonal or multicolor icons.  */

   if ((! O_DefaultColors (new)) &&
       ((pSD->colorUse == B_W) || (pSD->pPixelData == NULL)) )
      O_UseMultiColorIcons (new) = False;
   else
   {
      _XmGetIconControlInfo (pScr,
                             &useMaskRtn, &useMultiColorIconsRtn,
                             &useIconFileCacheRtn);
      O_UseMultiColorIcons (new) = useMultiColorIconsRtn;
   }

   O_AppShell (new) = XtParent (new);

   O_Resolution (new) = NO_RES_DISPLAY;
   O_FontList (new) = NULL;

   O_Busy (new) = NULL;

   O_Switch (new) = NULL;
   O_SwitchLast (new) = NULL;
   O_SwitchRC (new) = NULL;
   O_Shell (new) = NULL;
   O_Panel (new) = NULL;
   O_ClientData (new) = NULL;
   O_SwitchSubpanel(new) = NULL;

   O_Panels (new) = WmFpListCreate ();
   O_Boxes (new) = WmFpListCreate ();
   O_Controls (new) = WmFpListCreate ();
   O_Animations (new) = WmFpListCreate ();

   O_NumEmbeddedClients (new) = 0;
   O_MaxEmbeddedClients (new) = WM_FP_MAX_ITEMS;
   O_EmbeddedClients (new) = (WmFpEmbeddedClientList)
      XtMalloc (WM_FP_MAX_ITEMS * sizeof (WmFpEmbeddedClientData));
   O_NumRecallClients (new) = 0;
   O_MaxRecallClients (new) = WM_FP_MAX_ITEMS;
   O_RecallClients (new) = (WmFpPushRecallClientList)
     XtMalloc (WM_FP_MAX_ITEMS * sizeof (WmFpPushRecallClientData));


   /*  Copy panel name.  */

   if (name_string == NULL)
      name_string = WM_FP_DEFAULT_PANEL_NAME;

   O_PanelName (new) = XtMalloc (strlen (name_string) + 1);
   strcpy (O_PanelName (new), name_string);
}




/*-------------------------------------------------------------
**      Destroy
**              Release resources allocated for gadget.
*/

#if 0
static void Destroy (w)
Widget w;
{
   WmPanelistObject pPanelist = (WmPanelistObject) w;
   int              i;

   if (O_Panels (pPanelist) != NULL)
      WmFpListDestroy (O_Panels (pPanelist));
   if (O_Boxes (pPanelist) != NULL)
      WmFpListDestroy (O_Boxes (pPanelist));
   if (O_Controls (pPanelist) != NULL)
      WmFpListDestroy (O_Controls (pPanelist));
   if (O_Animations (pPanelist) != NULL)
      WmFpListDestroy (O_Animations (pPanelist));

   if (O_NumEmbeddedClients (pPanelist) > 0)
   {
      WmFpEmbeddedClientData *pEmbeddedClientData;

      for (i = 0; i < O_NumEmbeddedClients (pPanelist); i++)
      {
         pEmbeddedClientData = &(O_EmbeddedClients (pPanelist)[i]);
         XtFree ((XtPointer) pEmbeddedClientData->pchResName);
      }

      XtFree ((XtPointer) O_EmbeddedClients (pPanelist));
   }

   if (O_NumRecallClients (pPanelist) > 0)
   {
      WmFpPushRecallClientData *pRecallData;

      for (i = 0; i < O_NumRecallClients (pPanelist); i++)
      {
         pRecallData = &(O_RecallClients (pPanelist)[i]);
         XtFree ((XtPointer) pRecallData->pRArg);
         XtFree ((XtPointer) pRecallData->pchResName);
      }

      XtFree ((XtPointer) O_RecallClients (pPanelist));
   }

   if (O_Shell (pPanelist) != NULL)
      XtDestroyWidget (O_Shell (pPanelist));
}
#endif




/*-------------------------------------------------------------
**      SetValues
**              
*/

#if 0
/* ARGSUSED */
static Boolean SetValues (current_w, request_w, new_w)
Widget current_w, request_w, new_w;
{
   WmPanelistObject request = (WmPanelistObject) request_w,
                    current = (WmPanelistObject) current_w,
                    new = (WmPanelistObject) new_w;
   String           name_string = O_PanelName (request);


   /*  Update panel name.  */

   if (O_PanelName (new) != O_PanelName (current))
   {
      O_PanelName (new) = XtMalloc (strlen (name_string) + 1);
      strcpy (O_PanelName (new), name_string);
      XtFree (name_string);
   }

   return (False);
}
#endif




/*-------------------------------------------------------------
**      Public Entry Points
**-------------------------------------------------------------
*/


/*-------------------------------------------------------------
**      WmPanelistCreate
**              Create a new panelist instance.
**-------------------------------------------------------------
*/

Widget WmPanelistCreate (parent, name, arglist, argcount)
Widget   parent;
String   name;
ArgList  arglist;
Cardinal argcount;

{
   return (XtCreateWidget (name, wmPanelistObjectClass, 
                           parent, arglist, argcount));
}




/************************************************************************
 *
 *  WmPanelistAllocate
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
WmPanelistAllocate (w)
Widget w;

#else /* _NO_PROTO */
void
WmPanelistAllocate (Widget w)
#endif /* _NO_PROTO */

{
   WmPanelistObject pPanelist = (WmPanelistObject) w;
   int              iPos;
   WmFpItem         pItem;


   iPos = WmFpListGetItemPos (O_Panels (pPanelist), O_PanelName (pPanelist));

   if (iPos > 0)
   {
      pItem = WmFpListGetItem (O_Panels (pPanelist), iPos);
      AllocatePanel (pPanelist, pItem, XtParent (w));
   }
   else
   {
      sprintf (sWarnBuf,
               ((char*) GETMESSAGE (78, 1, sWarnMissingItem)),
               O_PanelName (pPanelist));
      Warning (sWarnBuf);
   }

   if (O_Panels (pPanelist) != NULL)
   {
      WmFpListDestroy (O_Panels (pPanelist));
      O_Panels (pPanelist) = NULL;
   }

   if (O_Boxes (pPanelist) != NULL)
   {
      WmFpListDestroy (O_Boxes (pPanelist));
      O_Boxes (pPanelist) = NULL;
   }

   if (O_Controls (pPanelist) != NULL)
   {
      WmFpListDestroy (O_Controls (pPanelist));
      O_Controls (pPanelist) = NULL;
   }

   if (O_Animations (pPanelist) != NULL)
   {
      WmFpListDestroy (O_Animations (pPanelist));
      O_Animations (pPanelist) = NULL;
   }
}




/************************************************************************
 *
 *  WmPanelistSetWorkspace
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
WmPanelistSetWorkspace (w, i)
Widget w;
int    i;

#else /* _NO_PROTO */
void
WmPanelistSetWorkspace (Widget w,
                        int    i)
#endif /* _NO_PROTO */

{
   WmPanelistObject pPanelist = (WmPanelistObject) w;
   Widget           wSwitchRC = O_SwitchRC (pPanelist);
   Widget           wToggle = NULL;
   Widget          *pWid;
   int              iWid;
   Arg              al[20];
   register int     ac;
   Dimension        iWidth, iMaxWidth;


   if (wSwitchRC == NULL)
      return;

   ac = 0;
   XtSetArg (al[ac], XmNchildren, &pWid);  ac++;
   XtSetArg (al[ac], XmNnumChildren, &iWid);  ac++;
   XtGetValues (wSwitchRC, al, ac);

   if ((i > 0) && (i <= iWid))
      wToggle = pWid[i-1];

   if (wToggle != NULL)
      _DtIconSetState (wToggle, True, True);
}




/************************************************************************
 *
 *  WmPanelistSetWorkspaceTitle
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
WmPanelistSetWorkspaceTitle (w, i, xmStr)
Widget   w;
int      i;
XmString xmStr;

#else /* _NO_PROTO */
void
WmPanelistSetWorkspaceTitle (Widget   w,
                             int      i,
                             XmString xmStr)
#endif /* _NO_PROTO */

{
   WmPanelistObject pPanelist = (WmPanelistObject) w;
   Widget           wSwitchRC = O_SwitchRC (pPanelist);
   Widget           wToggle = NULL;
   Widget          *pWid;
   int              iWid;
   Arg              al[20];
   register int     ac;

   if (wSwitchRC == NULL)
      return;

   ac = 0;
   XtSetArg (al[ac], XmNchildren, &pWid);  ac++;
   XtSetArg (al[ac], XmNnumChildren, &iWid);  ac++;
   XtGetValues (wSwitchRC, al, ac);

   /*  Set the name string on the button  */

   if ((i > 0) && (i <= iWid))
      wToggle = pWid[i-1];

   if (wToggle != NULL)
   {
      ac = 0;
      XtSetArg (al[ac], XmNstring, xmStr);  ac++;
      XtSetValues (wToggle, al, ac);
   }
}




/************************************************************************
 *
 *  WmPanelistNameToSubpanel
 *
 ************************************************************************/

#ifdef _NO_PROTO
Widget
WmPanelistNameToSubpanel (w, str)
Widget w;
String str;

#else /* _NO_PROTO */
Widget
WmPanelistNameToSubpanel (Widget w,
                          String str)
#endif /* _NO_PROTO */

{
   WmPanelistObject pPanelist = (WmPanelistObject) w;
   Widget           wShell = O_Shell (pPanelist);
   CompositeWidget  cw;
   Widget           *pWid;
   int              iWid;
   int              i;

   for (i = 0, pWid = M_PopupList (wShell); i < M_NumPopups (wShell); i++)
   {
      if (!strcmp (XtName (pWid[i]), str))
      {
         cw = (CompositeWidget) pWid[i];

         if (M_NumChildren (cw) > 0)
            return (M_Children (cw))[0];
         else
            return NULL;
      }
   }
}




/************************************************************************
 *
 *  WmPanelistGetSubpanelShells
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
WmPanelistGetSubpanelShells (w, ppWid, pInt)
Widget w;
Widget **ppWid;
int    *pInt;

#else /* _NO_PROTO */
void
WmPanelistGetSubpanelShells (Widget w,
                             Widget **ppWid,
                             int    *pInt)
#endif /* _NO_PROTO */

{
   WmPanelistObject pPanelist = (WmPanelistObject) w;
   Widget           wShell = O_Shell (pPanelist);


   if (wShell != NULL)
   {
      *ppWid = M_PopupList (wShell);
      *pInt = M_NumPopups (wShell);
   }
   else
   {
      *ppWid = NULL;
      *pInt = 0;
   }
}




/************************************************************************
 *
 *  WmPanelistGetSwitchToggleWidth
 *
 ************************************************************************/

#ifdef _NO_PROTO
int
WmPanelistGetSwitchToggleWidth (w)
Widget w;

#else /* _NO_PROTO */
int
WmPanelistGetSwitchToggleWidth (Widget w)
#endif /* _NO_PROTO */

{
   WmPanelistObject pPanelist = (WmPanelistObject) w;


   switch (O_Resolution (pPanelist))
   {
      case HIGH_RES_DISPLAY:
         return (WM_FP_HI_SWI_WIDTH - 8);

      case MED_RES_DISPLAY:
         return (WM_FP_MED_SWI_WIDTH - 8);

      case LOW_RES_DISPLAY:
         return (WM_FP_LOW_SWI_WIDTH - 8);
   }
}




/************************************************************************
 *
 *  WmPanelistWindowToSubpanel
 *	Get a panel widget given its shell window.
 *
 ************************************************************************/

#ifdef _NO_PROTO
Widget
WmPanelistWindowToSubpanel (dpy, win)
Display *dpy;
Window  win;

#else /* _NO_PROTO */
Widget
WmPanelistWindowToSubpanel (Display *dpy, Window win)
#endif /* _NO_PROTO */

{
   Widget wShell;
   Widget *pWid;
   Arg    al[5];
   int    ac;
   int    iWid;


   /*  Get the widget for the subpanel (Should be only child of the shell!)  */

   wShell = XtWindowToWidget (dpy, win);

   ac = 0; 
   XtSetArg (al[ac], XmNchildren, &pWid);  ac++;
   XtSetArg (al[ac], XmNnumChildren, &iWid);  ac++;
   XtGetValues (wShell, al, ac);

   return ((iWid > 0) ? pWid[0] : (Widget)NULL);
}




/************************************************************************
 *
 *  WmPanelistSetBusy
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
WmPanelistSetBusy (w, b)
Widget  w;
Boolean b;

#else /* _NO_PROTO */
void
WmPanelistSetBusy (Widget  w,
                   Boolean b)
#endif /* _NO_PROTO */

{
   WmPanelistObject pPanelist = (WmPanelistObject) w;
   DtControlGadget cg = (DtControlGadget) O_Busy (pPanelist);

   if (O_Busy (pPanelist) != NULL)
      _DtControlSetBusy (O_Busy (pPanelist), b);
}




/************************************************************************
 *
 *  WmPanelistSetClientGeometry
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
WmPanelistSetClientGeometry (w)
Widget w;

#else /* _NO_PROTO */
void
WmPanelistSetClientGeometry (Widget w)
#endif /* _NO_PROTO */

{
   WmPanelistObject       pPanelist = (WmPanelistObject) w;
   DtControlGadget       cg;
   DtViewWidget          view;
   WmFpEmbeddedClientList pClient;
   int                    i;


   for (i = 0; i < O_NumEmbeddedClients (pPanelist); i++)
   {
      pClient = &(O_EmbeddedClients (pPanelist)[i]);
      cg = (DtControlGadget) pClient->wControl;
      view = (DtViewWidget) XtParent (pClient->wControl);

      pClient->winParent = XtWindow ((Widget) view);

      switch (M_BoxType (view))
      {
         case XmBOX_PRIMARY:
            switch (O_Resolution (pPanelist))
            {
              case HIGH_RES_DISPLAY:
                 pClient->x = G_X (cg) + WM_FP_HI_PRI_INSET_LEFT;
                 pClient->y = G_Y (cg) + WM_FP_HI_PRI_INSET_TOP;
                 pClient->width = G_Width (cg) - WM_FP_HI_PRI_INSET_LEFT
                                  - WM_FP_HI_PRI_INSET_RIGHT;
                 pClient->height = G_Height (cg) - WM_FP_HI_PRI_INSET_TOP
                                   - WM_FP_HI_PRI_INSET_BOTTOM;
               break;

               case MED_RES_DISPLAY:
                  pClient->x = G_X (cg) + WM_FP_MED_PRI_INSET_LEFT;
                  pClient->y = G_Y (cg) + WM_FP_MED_PRI_INSET_TOP;
                  pClient->width = G_Width (cg) - WM_FP_MED_PRI_INSET_LEFT
                                   - WM_FP_MED_PRI_INSET_RIGHT;
                  pClient->height = G_Height (cg) - WM_FP_MED_PRI_INSET_TOP
                                    - WM_FP_MED_PRI_INSET_BOTTOM;
               break;

               case LOW_RES_DISPLAY:
                  pClient->x = G_X (cg) + WM_FP_LOW_PRI_INSET_LEFT;
                  pClient->y = G_Y (cg) + WM_FP_LOW_PRI_INSET_TOP;
                  pClient->width = G_Width (cg) - WM_FP_LOW_PRI_INSET_LEFT
                                   - WM_FP_LOW_PRI_INSET_RIGHT;
                  pClient->height = G_Height (cg) - WM_FP_LOW_PRI_INSET_TOP
                                    - WM_FP_LOW_PRI_INSET_BOTTOM;
               break;
            }
         break;

         case XmBOX_SUBPANEL:
            pClient->x = G_X (cg) + 3;
            pClient->y = G_Y (cg) + 3;
            pClient->width = G_Width (cg) - 6;
            pClient->height = G_Height (cg) - 6;
         break;
      }
   }
}




/************************************************************************
 *
 *  WmPanelistSetDropZones
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
WmPanelistSetDropZones (w)
Widget w;

#else /* _NO_PROTO */
void
WmPanelistSetDropZones (Widget w)
#endif /* _NO_PROTO */

{
   WmPanelistObject pPanelist = (WmPanelistObject) w;
   Widget           wShell = O_Shell (pPanelist);
   CompositeWidget  cw;
   Widget           *pWid;
   int              i;



   for (i = 0, pWid = M_PopupList (wShell); i < M_NumPopups (wShell); i++)
   {
      cw = (CompositeWidget) pWid[i];

      XmDropSiteStartUpdate(pWid[i]);

      if (M_NumChildren (cw) > 0);
      {
         CompositeWidget cwBox = (CompositeWidget) (M_Children (cw))[0];
         Widget          *pWidBox = M_Children (cwBox);

         SetDropZones (pWidBox[0]);
      }


      XmDropSiteEndUpdate(pWid[i]);
   }

   cw = (CompositeWidget) O_Panel (pPanelist);


   /*  Set the drop zones for the main panel.  Skip the first and last  */
   /*  children.  These are the system menu and more handles.           */

   XmDropSiteStartUpdate(wShell);

   for (i = 1, pWid = M_Children (cw); i < M_NumChildren (cw) - 1; i++)
      SetDropZones (pWid[i]);

   XmDropSiteEndUpdate(wShell);
}




/************************************************************************
 *
 *  WmPanelistSubpanelPosted
 *	Called by window manager subpanel slideup code.  Used to add an
 *	event handler to catch when the subpanel is torn off.  The event
 *      handler will then change the subpanel's behavior to remain displayed
 *      after a control is selected.
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
WmPanelistSubpanelPosted (display, shell_window)
Display * display;
Window    shell_window;

#else /* _NO_PROTO */
void
WmPanelistSubpanelPosted (Display * display,
                          Window  shell_window)
#endif /* _NO_PROTO */

{
   Widget subpanel;


   subpanel = WmPanelistWindowToSubpanel (display, shell_window);


   /*  Add the event handler  */

   XtAddEventHandler (XtParent (subpanel), StructureNotifyMask, False,
                      (XtEventHandler) SubpanelTornEventHandler,
                      (XtPointer) subpanel);
}





/************************************************************************
 *
 *   WmPanelistShow
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
WmPanelistShow (w)
Widget w;

#else /* _NO_PROTO */
void
WmPanelistShow (Widget w)
#endif /* _NO_PROTO */

{
   WmPanelistObject pPanelist = (WmPanelistObject) w;
   Widget           wShell = O_Shell (pPanelist);
   CompositeWidget  cw;
   Widget           wPanel = O_Panel (pPanelist);
   Widget           *pWid;
   int              iWid;
   Arg              al[20];
   register int     ac;
   Position         iX;
   Position         iY;
   Dimension        iWidth;
   Dimension        iHeight;
   Dimension        iScrWidth;
   Dimension        iScrHeight;
   Dimension        iMax;
   Display          *dpy;
   String           sGeo = NULL;
   Boolean          bGeo = False;
   char             pchGeoBuf[32];
   CompositeWidget  cwPanel = (CompositeWidget) O_Panel (pPanelist);
   unsigned char    eBoxType = XmBOX_NONE;


   if (wShell == NULL)
      return;


   /*  See if default geometry is needed.  */

   ac = 0;
   XtSetArg (al[ac], XmNgeometry, &sGeo);  ac++;
   XtGetValues (wShell, al, ac);

   if (sGeo == NULL)
      bGeo = True;

#ifdef _NO_WM
   if ((sGeo == NULL) ||
      (O_SubpanelDirection (pPanelist) == WM_FP_TOK_west) ||
      (O_SubpanelDirection (pPanelist) == WM_FP_TOK_east))
      bGeo = True;
#endif /* _NO_WM */


   dpy = XtDisplay (wShell);

   if (!XtIsRealized (wShell))
   {
      DtWmHints vHints;

      XtRealizeWidget (wShell);

      if (bGeo || O_DefaultGeo (pPanelist))
      {
         /*  Construct default geometry string, set geometry resource.  */

         switch (O_SubpanelDirection (pPanelist))
         {
            case WM_FP_TOK_north:
            case WM_FP_TOK_south:
               iWidth = XtWidth (wShell);
               iScrWidth = WidthOfScreen (XtScreen (w));
               iX = (iScrWidth > iWidth) ? (iScrWidth - iWidth) / 2 : 0;
               sprintf (pchGeoBuf, "+%d-0", iX);
            break;

            case WM_FP_TOK_east:
            case WM_FP_TOK_west:
#if 0
               iHeight = XtHeight (wShell);
               iScrHeight = HeightOfScreen (XtScreen (w));
               iY = (iScrHeight > iHeight) ? (iScrHeight - iHeight) / 2 : 0;
               sprintf (pchGeoBuf, "+0+%d", iY);
#endif
               sprintf (pchGeoBuf, "+0-0", iY);
             break;
         }

         ac = 0;
         XtSetArg (al[ac], XmNgeometry, pchGeoBuf);  ac++;
         XtSetValues (wShell, al, ac);

         /*  Reallocate windows to apply geometry.  */

         XtUnrealizeWidget (wShell);
         XtRealizeWidget (wShell);
      }


      /*  Set panels and subpanel hints.  */

      vHints.flags = DtWM_HINTS_BEHAVIORS;
#ifdef _NO_WM
      vHints.behaviors = 0;
      DtWsmSetDtWmHints (dpy, XtWindow (wShell), &vHints);
#else
      vHints.behaviors = DtWM_BEHAVIOR_PANEL;
      DtWsmSetDtWmHints (dpy, XtWindow (wShell), &vHints);
      vHints.behaviors |= DtWM_BEHAVIOR_SUBPANEL;
      vHints.flags |= DtWM_HINTS_ATTACH_WINDOW;
      vHints.attachWindow = XtWindow (wShell);

      for (iWid = 0, pWid = M_PopupList (wShell); 
           iWid < M_NumPopups (wShell); iWid++)
      {
         cw = (CompositeWidget) pWid[iWid];

         if (bGeo || O_DefaultGeo (pPanelist))
         {
            /*  Force TransientShell to update transientFor.  */

            ac = 0;
            XtSetArg (al[ac], XmNtransientFor, NULL);  ac++;
            XtSetValues ((Widget) cw, al, ac);

            ac = 0;
            XtSetArg (al[ac], XmNtransientFor, wShell);  ac++;
            XtSetValues ((Widget) cw, al, ac);
         }

         if (M_NumChildren (cw) > 0)
         {
            XtRealizeWidget ((M_Children (cw))[0]);
            DtWsmSetDtWmHints (dpy, XtWindow (pWid[iWid]), &vHints);
         }
      }
#endif /* _NO_WM */


      /*  Validate switch layout.  */

      cw = (CompositeWidget) O_Switch (pPanelist);


      /*  Force the width of the switch subpanel to equal the  */
      /*  width of the switch and turn off all decorations.    */

      if ((cw != NULL) && (O_SwitchSubpanel (pPanelist) != NULL))
      {
         ac = 0;
         XtSetArg (al[ac], XmNwidth, XtWidth (cw));  ac++;
         XtSetValues (O_SwitchSubpanel (pPanelist), al, ac);

         ac = 0;
         XtSetArg (al[ac], XmNmwmDecorations, 0);  ac++;
         XtSetValues (XtParent (O_SwitchSubpanel (pPanelist)), al, ac);
      }


#if 0

      if ((cw != NULL) && (M_NumChildren (cw) > 0))
      {
         Widget    w1 = M_Children (cw)[0];
         Widget    w2 = M_Children (cw)[M_NumChildren (cw) - 1];
         Dimension h1 = XtY (w1);
         Dimension h2 = (XtHeight (cw) > XtY (w2) + XtHeight (w2))
                         ? XtHeight (cw) - XtY (w2) - XtHeight (w2) : h1;


         /*  Equalize top margin and bottom margin.  */

         ac = 0;

         if (h2 > h1)
         {
            XtSetArg (al[ac], XmNmarginHeight, (h1 + h2) / 2);  ac++;
         }


         /*  Force even size.  */

         if (XtWidth (cw) % 2 == 1)
         {
            XtSetArg (al[ac], XmNwidth, XtWidth (cw) + 1);  ac++;
         }

         XtSetArg (al[ac], XmNresizeWidth, False);  ac++;
         XtSetValues ((Widget) cw, al, ac);
      }
#endif


      /*  Freeze layout.  */

#if 0
      else
      {
         ac = 0;
         XtSetArg (al[ac], XmNresizePolicy, XmRESIZE_NONE);  ac++;
         XtSetValues (O_Panel (pPanelist), al, ac);
      }
#endif


      /*  Set drop zones and client geometry.  */

      WmPanelistSetDropZones (w);
      WmPanelistSetClientGeometry (w);


      /*  Register embedded and push recall clients.  */

      RegisterEmbeddedClients (w, O_EmbeddedClients (pPanelist),
                               O_NumEmbeddedClients (pPanelist));
      RegisterPushRecallClients (w, O_RecallClients (pPanelist),
                                 O_NumRecallClients (pPanelist));
   }


   /*  Display panel.  */

   XtPopup (O_Shell (pPanelist), XtGrabNone);
}




/************************************************************************
 *
 *  WmPanelistParseItem
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
WmPanelistParseItem (w, pWmPB)
Widget         w;
DtWmpParseBuf *pWmPB;

#else /* _NO_PROTO */
void
WmPanelistParseItem (Widget         w,
                     DtWmpParseBuf *pWmPB  )
#endif /* _NO_PROTO */

{
   WmPanelistObject pPanelist = (WmPanelistObject) w;
   unsigned char    *pchLine, *pch;
   WmFpItem         pItem;
   WmFpToken        eTokItem;
   WmFpString       pchName;
   WmFpStringList   ppchTokens;
   int              i;
   int              iTokens = 0;
   int              iMaxTokens = WM_FP_MAX_ITEMS;
   Boolean          bDone;


   if (pch = _DtWmParseNextTokenExpand (pWmPB))
   {

      /*  Get item type.  */

      _DtWmParseToLower ((unsigned char *) pch);
      eTokItem = WmFpStringToItemToken ((WmFpString) pch);
      XtFree ((char *)pch);

      if (eTokItem != WM_FP_TOK_none)
      {

         /*  Get name.  */

         pchName = (WmFpString) _DtWmParseNextTokenExpand (pWmPB);

         /*  Get open bracket.  */

         if (pchLine = _DtWmParseNextLine (pWmPB))
         {
            pch = _DtWmParseNextTokenExpand (pWmPB);
            ppchTokens = (WmFpStringList)
               XtMalloc (sizeof (WmFpString) * iMaxTokens);

            if (! strcmp ((char *) pch, "{"))
               bDone = False;
            else
               bDone = True;

	    XtFree ((char *) pch);
         }
         else
            bDone = True;


         /*  Get token list.  */

         while (!bDone)
         {
            Boolean bFirst;

            if (pchLine = _DtWmParseNextLine (pWmPB))
            {
               bFirst = True;

               while (pch = _DtWmParseNextTokenExpand (pWmPB))
               {

                  /*  Get close bracket.  */

                  if (!strcmp ((char *) pch, "}"))
		  {
                     bDone = True;
		  }


                  /*  Allocate more space.  */

                  if (iTokens + 1 >= iMaxTokens)
                  {
                     iMaxTokens += WM_FP_MAX_ITEMS;
                     ppchTokens = (WmFpStringList)
                                   XtRealloc ((char *) ppchTokens,
                                              sizeof (WmFpString) * iMaxTokens);
                  }


                  /*  Mark line boundary.  */

                  if (bFirst || bDone)
                  {
                     bFirst = False;
                     ppchTokens[iTokens++] = NULL;
                     _DtWmParseToLower ((unsigned char *) pch);
                  }


                  /*  Save copy of token.  */

                  if (! bDone)
                  {
                     ppchTokens[iTokens++] = (WmFpString) pch;
                  }
		  else
		  {
		      XtFree ((char *) pch);
		  }
               }
            }
            else
               bDone = True;
         }


         /*  Create Item.  */

         pItem = WmFpItemCreate (eTokItem,
                                 (WmFpString) pchName, ppchTokens, iTokens);

         switch (eTokItem)
         {
            case WM_FP_TOK_animation:
               WmFpListAddItem (O_Animations (pPanelist), pItem);
            break;

            case WM_FP_TOK_panel:
               WmFpListAddItem (O_Panels (pPanelist), pItem);
            break;

            case WM_FP_TOK_box:
            case WM_FP_TOK_switch:
               WmFpListAddItem (O_Boxes (pPanelist), pItem);
            break;

            case WM_FP_TOK_control:
               WmFpListAddItem (O_Controls (pPanelist), pItem);
            break;
         }
      }
   }
}


















