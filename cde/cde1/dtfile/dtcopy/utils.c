/* $XConsortium: utils.c /main/cde1_maint/2 1995/08/29 19:46:47 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           utils.c
 *
 *
 *   DESCRIPTION:    Utility routines
 *
 *   FUNCTIONS: Create_Action_Area
 *		Display_Help
 *		Get_Help_Dialog
 *		Help
 *		_DtChildPosition
 *		help_callback
 *		help_close_callback
 *		help_hyperyperlink_callback
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <time.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/XmP.h>
#include <Xm/Xm.h>
#include <Xm/VendorSEP.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/BulletinB.h>
#include <Xm/TextF.h>

#include <Dt/HelpDialog.h>

#include "dtcopy.h"
#include "sharedFuncs.h"



/*............... definitions for Help .............................*/
void
#ifdef _NO_PROTO
Help();
#else
Help(
	char *helpVolume,
	char *locationId);
#endif
void
#ifdef _NO_PROTO
Display_Help();
#else
Display_Help(
	char *helpVolume,
	char *locationId);
#endif

/* Help volume tags used to attach a certian help volume
 * to a particular call back.  Look to function help_callback
 * for more code.  MJT
 */
#define HELP_USING_HELP_VOLUME		"Help4Help"
#define HELP_USING_STR			"_HomeTopic"
#define HELP_VOLUME			"Filemgr"
#define HELP_HELP_MENU_STR		"Reference"
#define HELP_INTRODUCTION_STR		"_HomeTopic"
#define HELP_DIR_COPY_STR		"FMCopyDirDialogStatusDE"
#define HELP_DIR_ERROR_STR		"FMCopyDIrDialogErrorDE"
#define HELP_OVERWRITE_STR		"FMCopyDirOverDialogDE"
#define HELP_VERSION_STR		"FMHelpMenuDE"
#define STOPW_HELP_STR			"FMCopyFoldWarnDialogDE"

/*--------------------------------------------------------------------
 * Callback routines
 *------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
help_close_callback(widget, client_data, call_data)
	Widget widget;
	XtPointer client_data;
	XtPointer call_data;
#else
help_close_callback(
	Widget widget,
	XtPointer client_data,
	XtPointer call_data)
#endif/* _NO_PROTO */
{
  XtUnmapWidget(XtParent(widget));
}

void
#ifdef _NO_PROTO
help_hyperyperlink_callback(widget, client_data, call_data)
	Widget widget;
	XtPointer client_data;
	XtPointer call_data;
#else
help_hyperyperlink_callback(
	Widget widget,
	XtPointer client_data,
	XtPointer call_data)
#endif/* _NO_PROTO */
{
  DtHelpDialogCallbackStruct *pHyper = (DtHelpDialogCallbackStruct *) call_data;

  switch(pHyper->hyperType)
  {
     case DtHELP_LINK_JUMP_NEW:
          Display_Help(pHyper->helpVolume, pHyper->locationId);
          break;
     default:
          ;
   }
}

 void
#ifdef _NO_PROTO
help_callback(widget, client_data, call_data)
	Widget widget;
	XtPointer client_data;
	XtPointer call_data;
#else
help_callback(
	Widget widget,
	XtPointer client_data,
	XtPointer call_data)
#endif/* _NO_PROTO */
{
  char  *helpVolume, *locationId;
  int   topic;

  topic = (int) client_data;
  helpVolume = HELP_VOLUME;

  switch (topic)
  {
	case HELP_HELP_MENU:
	    locationId = HELP_HELP_MENU_STR;
            break;
        case HELP_INTRODUCTION:
            locationId = HELP_INTRODUCTION_STR;
            break;
        case HELP_DIR_COPY:
            locationId = HELP_DIR_COPY_STR;
            break;
        case HELP_DIR_ERROR:
            locationId = HELP_DIR_ERROR_STR;
            break;
        case HELP_OVERWRITE:
            locationId = HELP_OVERWRITE_STR;
            break;
        case HELP_USING:
            locationId = HELP_USING_STR;
            helpVolume = HELP_USING_HELP_VOLUME;
            break;
        case HELP_VERSION:
            locationId = HELP_VERSION_STR;
            break;
	case STOPW_HELP_DIALOG:
	    locationId = STOPW_HELP_STR;
	    break;
	default:
            locationId = HELP_USING_STR;
            helpVolume = HELP_USING_HELP_VOLUME;
            break;
  }
  Help(helpVolume, locationId);
}


/*--------------------------------------------------------------------
 * _DtChildPosition
 *------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
_DtChildPosition(w, parent, newX, newY)
         Widget w;
         Widget parent;
         Position *newX, *newY;
#else
_DtChildPosition(
         Widget w,
         Widget parent,
         Position *newX,
         Position *newY)
#endif /* _NO_PROTO */
{
   Position pY, pX;
   XmVendorShellExtObject vendorExt;
   XmWidgetExtData        extData;
   int xOffset, yOffset;
   int pHeight, myHeight, sHeight;
   int pWidth, myWidth, sWidth;
   enum { posRight, posBelow, posLeft, posAbove } pos;
   int space;

   /* get x, y offsets for the parent's window frame */
   extData = _XmGetWidgetExtData(parent, XmSHELL_EXTENSION);
   if (extData)
   {
     vendorExt = (XmVendorShellExtObject)extData->widget;
     xOffset = vendorExt->vendor.xOffset;
     yOffset = vendorExt->vendor.yOffset;
   }
   else
     xOffset = yOffset = 0;

   /* get size/position of screen, parent, and widget */
   sHeight = HeightOfScreen(XtScreen(parent));;
   sWidth = WidthOfScreen(XtScreen(parent));
   pX = XtX(parent) - xOffset;
   pY = XtY(parent) - yOffset;
   pHeight = XtHeight(parent) + yOffset + xOffset;
   pWidth = XtWidth(parent) + 2*xOffset;
   myHeight = XtHeight(w) + yOffset + xOffset;
   myWidth = XtWidth(w) + 2*xOffset;

   {
     XWindowAttributes attr;

     XGetWindowAttributes(XtDisplay(parent), XtWindow(parent), &attr);
   }

   /*
    * Determine how much space would be left if the child was positioned
    * to the right, below, left, or above the parent.  Choose the child
    * positioning so that the maximum space is left.
    */
   pos = posRight;
   space = sWidth - (pX + pWidth + myWidth);

   if (sHeight - (pY + pHeight + myHeight) > space)
   {
      pos = posBelow;
      space = sHeight - (pY + pHeight + myHeight);
   }

   if (pX - myWidth > space)
   {
      pos = posLeft;
      space = pX - myWidth;
   }

   if (pY - myHeight > space)
   {
      pos = posAbove;
      space = pY - myHeight;
   }

   /* Given relative positioning, determine x, y coordinates for the child */

   switch (pos)
   {
     case posRight:
       *newX = pX + pWidth + 5;
       *newY = pY + (pHeight - myHeight)/2;
       break;

     case posBelow:
       *newX = pX + (pWidth - myWidth)/2;
       *newY = pY + pHeight + 5;
       break;

     case posLeft:
       *newX = pX - myWidth - 5;
       *newY = pY + (pHeight - myHeight)/2;
       break;

     case posAbove:
       *newX = pX + (pWidth - myWidth)/2;
       *newY = pY - myHeight - 5;
       break;
   }
}



static Widget
#ifdef _NO_PROTO
Get_Help_Dialog()
#else
Get_Help_Dialog()
#endif/* _NO_PROTO */
{
    Widget dialog;
    Arg args[5];
    int n;

        n = 0;
        XtSetArg(args[n], XmNtitle, GETMESSAGE(6, 5, "Directory Copy Help")); n++;
        dialog = DtCreateHelpDialog(G_toplevel, "helpDlg",  args, n);

        XtAddCallback(dialog, DtNhyperLinkCallback,
                      (XtCallbackProc)help_hyperyperlink_callback, NULL);
        XtAddCallback(dialog, DtNcloseCallback,
                      (XtCallbackProc)help_close_callback, NULL);
        return dialog;
}

void
#ifdef _NO_PROTO
Display_Help( helpVolume, locationId)
	char *helpVolume;
	char *locationId;
#else
Display_Help(
	char *helpVolume,
	char *locationId)
#endif/* _NO_PROTO */
{
    Arg args[10];
    int n;

    G_help_dialog = Get_Help_Dialog();

    n = 0;
    XtSetArg(args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
    XtSetArg(args[n], DtNhelpVolume, helpVolume);       n++;
    XtSetArg(args[n], DtNlocationId, locationId);       n++;
    XtSetValues(G_help_dialog, args, n);

    XtManageChild(G_help_dialog);
    XtMapWidget(XtParent(G_help_dialog));
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
#endif/* _NO_PROTO */
{
    Arg args[10];
    int n;

    if(G_help_dialog == NULL)
    {
        n = 0;
        XtSetArg(args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
        XtSetArg(args[n], DtNhelpVolume, helpVolume);   n++;
        XtSetArg(args[n], DtNlocationId, locationId);   n++;
        XtSetArg (args[n], XmNtitle,  GETMESSAGE(6, 5, "Directory Copy Help"));	n++;

        G_help_dialog = DtCreateHelpDialog(G_toplevel, "helpDlg", args, n);

        XtAddCallback(G_help_dialog, DtNhyperLinkCallback,
                      (XtCallbackProc)help_hyperyperlink_callback, NULL);
        XtAddCallback(G_help_dialog, DtNcloseCallback,
                      (XtCallbackProc)help_close_callback, NULL);

        XtManageChild(G_help_dialog);
    }
    else
    {
        n = 0;
        XtSetArg(args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
        XtSetArg(args[n], DtNhelpVolume, helpVolume);   n++;
        XtSetArg(args[n], DtNlocationId, locationId);   n++;
        XtSetValues(G_help_dialog, args, n);
    }

    XtMapWidget(XtParent(G_help_dialog));
}



/*--------------------------------------------------------------------
 *
 * Create Action Area
 *
 * Use of this function requires #include "sharedFuncs.h"
 * The source for Create_Action_Area is in dtfile/OverWrite.c and
 * dtfile/dtcopy/utils.c. Because it uses GETMESSAGE, it could not
 * be placed in dtcopy/sharedFunc.c.
 *
 *------------------------------------------------------------------*/

Widget
#ifdef _NO_PROTO
Create_Action_Area(parent_widget, actions, pushbutton_array)
	Widget parent_widget;
	ActionAreaDefn actions;
	Widget *pushbutton_array; /* if set to NULL, then ignored */
#else
Create_Action_Area(
	Widget parent_widget,
	ActionAreaDefn actions,
	Widget *pushbutton_array)
#endif
{
  Widget action_area, widget;
  int i;
  int fractbase_value;
  XmString xm_string;


  if (actions.defaultAction < 0  ||  actions.defaultAction > actions.numActions-1)
     actions.defaultAction = 0;

  fractbase_value = (TIGHTNESS * actions.numActions) - 1;
  action_area = XtVaCreateManagedWidget("action_area", xmFormWidgetClass ,parent_widget,
                                XmNfractionBase, fractbase_value,
                                XmNleftAttachment, XmATTACH_FORM,
                                XmNrightAttachment, XmATTACH_FORM,
                                XmNbottomAttachment, XmATTACH_FORM,
                                NULL);

  for (i=0; i < actions.numActions ;i++ )
    {
      xm_string = XmStringCreateLtoR(GETMESSAGE(actions.actionList[i].msg_set,
                                                actions.actionList[i].msg_num,
                                                actions.actionList[i].label),
                                     XmFONTLIST_DEFAULT_TAG);
      widget = XtVaCreateManagedWidget(actions.actionList[i].label,
               xmPushButtonWidgetClass, action_area,
               XmNleftAttachment,     (i ? XmATTACH_POSITION: XmATTACH_FORM),
               XmNleftPosition,       (TIGHTNESS * i),
               XmNtopAttachment,      XmATTACH_FORM,
               XmNbottomAttachment,   XmATTACH_FORM,
               XmNrightAttachment,
                        ((i != (actions.numActions - 1)) ? XmATTACH_POSITION: XmATTACH_FORM),
               XmNrightPosition,      ((TIGHTNESS*i) + (TIGHTNESS - 1)),
               XmNshowAsDefault,      (i == actions.defaultAction),
               XmNdefaultButtonShadowThickness, 1,
               XmNlabelString,         xm_string,
               NULL);
      XmStringFree(xm_string);



       if (actions.actionList[i].callback)
         {
           XtAddCallback(widget, XmNactivateCallback,
                         actions.actionList[i].callback, actions.actionList[i].data);
         }

       if (i == actions.defaultAction)
         {
            Dimension height, h;
            XtVaGetValues (action_area, XmNmarginHeight, &h, NULL);
            XtVaGetValues (widget, XmNheight, &height, NULL);

            height +=2 * h;
            XtVaSetValues (action_area,
                           XmNdefaultButton, widget,
                           XmNpaneMaximum,   height,
                           XmNpaneMinimum,   height,
                           NULL);

         }

       if (pushbutton_array != NULL)
           pushbutton_array[i] = widget;

    } /* endfor */

   XtManageChild(action_area);
   return action_area;

}  /* end Create_Action_Area */

