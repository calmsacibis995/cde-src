/* $XConsortium: copydialog.c /main/cde1_maint/2 1995/08/29 19:42:39 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           copydialog.c
 *
 *
 *   DESCRIPTION:    create dtcopy main dialog window
 *
 *   FUNCTIONS: cancel_callback
 *		create_copydir_dialog
 *		create_error_text_window
 *		desensitize_copy_action_area
 *		display_cancel_warning
 *		get_icons
 *		ok_callback
 *		pause_callback
 *		sensitize_copy_action_area
 *		warning_no_callback
 *		warning_yes_callback
 *
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/BulletinB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ScrolledW.h>
#include <Xm/Separator.h>
#include <Xm/XmStrDefs.h>
#include <Xm/ToggleB.h>

#include "dtcopy.h"
#include "sharedFuncs.h"


#define VERTICAL_SPACING  5
#define HORIZONTAL_SPACING  8
#define TOP_SPACING  (VERTICAL_SPACING + 5)
#define INDENTED_SPACING (HORIZONTAL_SPACING + 5)


/*--------------------------------------------------------------------
 * Global variables
 *------------------------------------------------------------------*/

static Pixmap pxWorking;
static Pixmap pxStopped;
static int pxWorkingMargin;
static int pxStoppedMargin;
static Widget icon;




static void
#ifdef _NO_PROTO
get_icons(w)
	Widget w;
#else
get_icons(
	Widget w)
#endif
{
  Arg args[2];
  Pixel background, foreground, top_shadow, bottom_shadow, select;
  Colormap colormap;

  unsigned int width1, width2, height, dummy;

  /* get foreground and background colors */
  XtSetArg (args[0], XmNbackground, &background);
  XtSetArg (args[1], XmNcolormap,  &colormap);
  XtGetValues (w, args, 2);
  XmGetColors ( XtScreen(w) , colormap, background,
	       &foreground, &top_shadow, &bottom_shadow, &select);

  /* get "working" pixmap */
  pxWorking = XmGetPixmapByDepth(XtScreen(w), "xm_working",
                                 foreground, background, w->core.depth);
  if (pxWorking == XmUNSPECIFIED_PIXMAP)
    pxWorking = XmGetPixmapByDepth(XtScreen(w), "default_xm_working",
                                   foreground, background, w->core.depth);

  /* get "stopped/done" pixmap */
  pxStopped = XmGetPixmapByDepth(XtScreen(w), "xm_information",
                                 foreground, background, w->core.depth);
  if (pxStopped == XmUNSPECIFIED_PIXMAP)
    pxStopped = XmGetPixmapByDepth(XtScreen(w), "default_xm_information",
                                   foreground, background, w->core.depth);

  /* get the width of the two pixmaps */
  XGetGeometry(XtDisplay(w), pxWorking,
               (Window *) &dummy,               /* returned root window */
               (int *) &dummy, (int *) &dummy,  /* x, y of pixmap */
               &width1, &height,                /* pixmap width, height */
               &dummy, &dummy);                 /* border width, depth */
  XGetGeometry(XtDisplay(w), pxStopped,
               (Window *) &dummy,               /* returned root window */
               (int *) &dummy, (int *) &dummy,  /* x, y of pixmap */
               &width2, &height,                /* pixmap width, height */
               &dummy, &dummy);                 /* border width, depth */

  /* Compute margins necessary to make the labels the same width */
  if (width1 > width2)
  {
    pxWorkingMargin = 0;
    pxStoppedMargin = width1 - width2;
  }
  else
  {
    pxWorkingMargin = width2 - width1;
    pxStoppedMargin = 0;
  }
}


/*--------------------------------------------------------------------
 * sensitize & desensitize buttons in copy action area
 *------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
sensitize_copy_action_area(pushbutton_array)
	Widget *pushbutton_array;
#else
sensitize_copy_action_area(
	Widget *pushbutton_array)
#endif
{
  /* Change icon to "working" */
  XtVaSetValues (icon,
                 XmNlabelPixmap, pxWorking,
                 XmNmarginLeft, pxWorkingMargin/2,
                 XmNmarginRight, (pxWorkingMargin + 1)/2,
                 NULL);

  /* Desensitize the OK button */
  XtVaSetValues (pushbutton_array[0],
                 XmNsensitive, FALSE,
                 NULL);

  /* Sensitize the Pause button */
  XtVaSetValues (pushbutton_array[1],
                 XmNsensitive, TRUE,
                 NULL);

  /* Sensitize the Cancel button */
  XtVaSetValues (pushbutton_array[2],
                 XmNsensitive, TRUE,
                 NULL);
}


void
#ifdef _NO_PROTO
desensitize_copy_action_area(pushbutton_array)
	Widget *pushbutton_array;
#else
desensitize_copy_action_area(
	Widget *pushbutton_array)
#endif
{
  /* Change icon to "stopped" */
  XtVaSetValues (icon,
                 XmNlabelPixmap, pxStopped,
                 XmNmarginLeft, pxStoppedMargin/2,
                 XmNmarginRight, (pxStoppedMargin + 1)/2,
                 NULL);

  /* Sensitize the OK button */
  XtVaSetValues (pushbutton_array[0],
                 XmNsensitive, TRUE,
                 NULL);

  /* Desensitize the Pause button */
  XtVaSetValues (pushbutton_array[1],
                 XmNsensitive, FALSE,
                 NULL);

  /* Desensitize the Cancel button */
  XtVaSetValues (pushbutton_array[2],
                 XmNsensitive, FALSE,
                 NULL);
}



/*--------------------------------------------------------------------
 * callbacks
 *------------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
ok_callback(w, client_data, call_data)
	Widget w;
	XtPointer client_data;
	XtPointer call_data;
#else
ok_callback(
	Widget w,
	XtPointer client_data,
	XtPointer call_data)
#endif
{
  G_dialog_closed = TRUE;
  G_do_copy       = FALSE;
  return;
}


void
#ifdef _NO_PROTO
cancel_callback(w, client_data, call_data)
	Widget w;
	XtPointer client_data;
	XtPointer call_data;
#else
cancel_callback(
	Widget w,
	XtPointer client_data,
	XtPointer call_data)
#endif
{

   display_cancel_warning(G_copy_dialog);

   G_pause_copy = TRUE;
}



static void
#ifdef _NO_PROTO
pause_callback(w, client_data, call_data)
	Widget w;
	XtPointer client_data;
	XtPointer call_data;
#else
pause_callback(w, client_data, call_data)
	Widget w;
	XtPointer client_data;
	XtPointer call_data;
#endif
{
  XmString xm_string;
  Pixmap px;
  int margin;

  G_pause_copy      = !G_pause_copy;
  G_user_pause_copy = !G_user_pause_copy;

  if (G_user_pause_copy)
  {
    xm_string = XmStringCreateLtoR(
                           GETMESSAGE(3, 1, "Resume"), XmFONTLIST_DEFAULT_TAG);
    px = pxStopped;
    margin = pxStoppedMargin;
  }
  else
  {
    xm_string = XmStringCreateLtoR(
                            GETMESSAGE(3, 2, "Pause"), XmFONTLIST_DEFAULT_TAG);
    px = pxWorking;
    margin = pxWorkingMargin;
  }

  XtVaSetValues(w,
                XmNlabelString, xm_string,
                NULL);
  XtVaSetValues(icon,
                XmNlabelPixmap, px,
                XmNmarginLeft, margin/2,
                XmNmarginRight, (margin + 1)/2,
                NULL);

  return;
}


/*---------------------------------------------------------------------
 *
 *  warning_yes_callback and warning_no_callback
 *
 *  callback for the warning dialog displayed when user selects cancel
 *
 *---------------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
warning_yes_callback(w, client_data, call_data)
	Widget w;
	XtPointer client_data;
	XtPointer call_data;
#else
warning_yes_callback(
	Widget w,
	XtPointer client_data,
	XtPointer call_data)
#endif
{
   G_do_copy=FALSE;
   G_wait_on_input = FALSE;
   if (G_over_dialog != NULL) {
     G_overwrite_selection = G_SKIP;
     XtDestroyWidget(G_over_dialog);
     G_over_dialog = NULL;
   }
   if (G_error_dialog != NULL) {
     XtDestroyWidget(G_error_dialog);
     G_error_dialog = NULL;
   }
   G_dialog_closed = TRUE;
   G_pause_copy = FALSE;
}


static void
#ifdef _NO_PROTO
warning_no_callback(w, client_data, call_data)
	Widget w;
	XtPointer client_data;
	XtPointer call_data;
#else
warning_no_callback(
	Widget w,
	XtPointer client_data,
	XtPointer call_data)
#endif
{
   G_user_pause_copy = TRUE;
   G_pause_copy = TRUE;
   pause_callback(G_copy_action_area_pbuttons[1], 0, 0);
}




/*-------------------------
 *
 *  display_cancel_warning
 *
 *------------------------*/
void
#ifdef _NO_PROTO
display_cancel_warning(parent)
   Widget parent;
#else
display_cancel_warning(Widget parent)
#endif

{
   XmString  msg, dialogTitle, cancelMsg, resumeMsg;
   int       n;
   Arg       args[20];
   Widget    dialog;

   String   copyWarnMessage = "This will stop the copy process. Any files that have\n\
already been copied will remain in the destination folder.";
   String   moveWarnMessage = "This will stop the move process. Any files that have\n\
already been moved will remain in the destination folder.";



   if (G_move)
   {
      dialogTitle = XmStringCreateLtoR (GETMESSAGE(3, 12, "Folder Move - Warning"),
                                        XmFONTLIST_DEFAULT_TAG);
      msg         = XmStringCreateLtoR (GETMESSAGE(3, 10, moveWarnMessage),
                                        XmFONTLIST_DEFAULT_TAG);
      cancelMsg   = XmStringCreateLtoR (GETMESSAGE(3, 15, "Cancel Move"),
                                        XmFONTLIST_DEFAULT_TAG);
      resumeMsg   = XmStringCreateLtoR (GETMESSAGE(3, 16, "Continue Move"),
                                        XmFONTLIST_DEFAULT_TAG);
   }
   else
   {
      dialogTitle = XmStringCreateLtoR (GETMESSAGE(3, 11, "Folder Copy - Warning"),
                                        XmFONTLIST_DEFAULT_TAG);
      msg         = XmStringCreateLtoR (GETMESSAGE(3,  7, copyWarnMessage),
                                        XmFONTLIST_DEFAULT_TAG);
      cancelMsg   = XmStringCreateLtoR (GETMESSAGE(3, 13, "Cancel Copy"),
                                        XmFONTLIST_DEFAULT_TAG);
      resumeMsg   = XmStringCreateLtoR (GETMESSAGE(3, 14, "Continue Copy"),
                                        XmFONTLIST_DEFAULT_TAG);
   }


   n = 0;
   XtSetArg(args[n], XmNmessageString, msg); n++;
   XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL); n++;
   XtSetArg(args[n], XmNdialogTitle, dialogTitle); n++;
   dialog = (Widget) XmCreateWarningDialog(parent, "warnDialog", args, n);

   XmStringFree(msg);
   XmStringFree(dialogTitle);

   n = 0;
   XtSetArg(args[n], XmNlabelString, cancelMsg); n++;
   XtSetValues((Widget) XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON), args, n);
   XmStringFree(cancelMsg);

   n = 0;
   XtSetArg(args[n], XmNlabelString, resumeMsg); n++;
   XtSetValues((Widget) XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON), args, n);
   XmStringFree(resumeMsg);

   XtAddCallback((Widget) XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON),
                 XmNactivateCallback,
   		 warning_yes_callback,
   		 dialog);
   XtAddCallback((Widget) XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON),
                 XmNactivateCallback,
   		 warning_no_callback,
   		 dialog);
   XtAddCallback((Widget) XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON),
                 XmNactivateCallback,
   		 help_callback,
   		 (XtPointer) STOPW_HELP_DIALOG);
   		
   XtManageChild(dialog);

}  /* end display_cancel_warning */



/*--------------------------------------------------------------------
 * create dtcopy dialog
 * these widgets are not managed until there is an error
 *------------------------------------------------------------------*/

static Widget
#ifdef _NO_PROTO
create_error_text_window (parent_widget, top_widget, bottom_widget)
	Widget parent_widget;
	Widget top_widget;
	Widget bottom_widget;
#else
create_error_text_window (
	Widget parent_widget,
	Widget top_widget,
	Widget bottom_widget)
#endif
{
  Widget scrolled_text;
  Arg args[15];
  int n;

  n = 0;

  /* set text to non-editable */
  XtSetArg(args[n], XmNeditable, FALSE); n ++;
  XtSetArg(args[n], XmNcursorPositionVisible, FALSE); n ++;
  XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
  XtSetArg(args[n], XmNrows, 5); n++;
  XtSetArg(args[n], XmNscrollVertical, True); n++;
  XtSetArg(args[n], XmNscrollHorizontal, True); n++;

  /* create the scrolled_text_window */
  scrolled_text = XmCreateScrolledText(parent_widget,"error_text", args, n);

  /* perform top attachments */
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNtopWidget,top_widget); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftOffset, INDENTED_SPACING); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNbottomWidget,bottom_widget); n++;
  XtSetValues(XtParent(scrolled_text), args, n);

  XtUnmanageChild(XtParent(scrolled_text));

  return (scrolled_text);
}


void
#ifdef _NO_PROTO
create_copydir_dialog(parent_widget, source, target)
	Widget parent_widget;
	char *source;
	char *target;
#else
create_copydir_dialog(
	Widget parent_widget,
	char *source,
	char *target)
#endif
{
  Arg args[10];
  int n;
  XmString xm_string;
  Widget form, form1, action_area, summary_label, separator2;
  Widget source_label, source_text_field, target_label, target_text_field;
  char   target_folder[MAX_PATH], target_object[MAX_PATH];

  Pixel background;
  Pixel foreground;

  Dimension width_target, width_source;

  static ActionAreaItem action_items[] = {
    {"Ok",          6, 1, ok_callback,     NULL},
    {"Pause",       6, 2, pause_callback,  NULL},
    {"Cancel Copy", 6, 7, cancel_callback, NULL},  /* possibly changed below to Cancel Move*/
    {"Help",        6, 4, help_callback,   (XtPointer) HELP_DIR_COPY},
  };

  ActionAreaDefn copydirActions = {
     XtNumber(action_items),
     0,                      /* Ok is default action */
     action_items
  };


  /* Save the name of the source directory in order to truncate the pathname */
  /* displayed as the copy proceeds ... see function UpdateStatus.           */
  strcpy(G_source_dir,source);


  if (G_move)
  {
     action_items[2].label   = "Cancel Move";
     action_items[2].msg_num = 8;
  }

  /* create a frame around the dialog for better
     separation from window border */
  G_copy_dialog =
    XtVaCreateWidget("outerFrame",
		     xmFrameWidgetClass,
		     parent_widget,
		     XmNshadowThickness, 3,
		     XmNshadowType,      XmSHADOW_ETCHED_IN,
		     NULL);

  /* Create the Manager Widget, form, for the copy dialog */
  form = XtVaCreateManagedWidget ("dialog_form",
				  xmFormWidgetClass,
				  G_copy_dialog,
				  XmNverticalSpacing,   VERTICAL_SPACING,
				  XmNwidth,             500,
				  XmNhorizontalSpacing, HORIZONTAL_SPACING,
				  NULL);

   /* create the toggle button */
   xm_string = XmStringCreateLtoR(GETMESSAGE(4, 2, "Pause for errors"),
                                     XmFONTLIST_DEFAULT_TAG);
   G_toggle_main = XtVaCreateManagedWidget("toggle",
   			      xmToggleButtonWidgetClass,
                              form,
                              XmNlabelString,      xm_string,
                              XmNset,              !G_ignore_errors,
                              XmNtopAttachment,    XmATTACH_FORM,
			      XmNtopOffset,        TOP_SPACING,
                              XmNrightAttachment,  XmATTACH_FORM,
			      XmNrightOffset,      INDENTED_SPACING,
                              XmNalignment,        XmALIGNMENT_BEGINNING,
                              NULL);
   XtAddCallback(G_toggle_main, XmNvalueChangedCallback, toggle_callback, NULL);
   XmStringFree(xm_string);

  /* Create a form for the working/stopped icon, from/to path names, and summary lines */
  form1 = XtVaCreateWidget ("msg_form",
				   xmFormWidgetClass,
				   form,
				   XmNtopAttachment,   XmATTACH_WIDGET,
				   XmNtopWidget,       G_toggle_main,
				   XmNtopOffset,       TOP_SPACING,
				   XmNleftAttachment,  XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   NULL);

  /* Create the working/stopped icon */
  get_icons(form1);

  icon = XtVaCreateManagedWidget("working_icon",
				 xmLabelWidgetClass,
				 form1,
				 XmNlabelType,        XmPIXMAP,
				 XmNlabelPixmap,      pxWorking,
				 XmNmarginLeft,       pxWorkingMargin/2,
				 XmNmarginRight,      (pxWorkingMargin + 1)/2,
				 XmNtopAttachment,    XmATTACH_FORM,
				 XmNbottomAttachment, XmATTACH_FORM,
				 XmNleftAttachment,   XmATTACH_FORM,
				 NULL);

  /* MJT Get color map from this particular spot in the
   widget hierarchy.  This is being done because of a strange
   behavior found in the widget hierarchy.  The behavior is
   identified by a change of the background and forground color
   making children widgets look somewhat odd.  It is possible
   that this is a bug that will be modified in the future.
   */
  XtSetArg (args[0], XmNbackground, &background);
  XtSetArg (args[1], XmNforeground, &foreground);
  XtGetValues (form1, args, 2);

  /* Create the Source Directory Widgets */
  if (G_move)
    xm_string = XmStringCreateLtoR(GETMESSAGE(3, 3, "Moving:"),
				   XmFONTLIST_DEFAULT_TAG);
  else
    xm_string = XmStringCreateLtoR(GETMESSAGE(3, 4, "Copying:"),
				   XmFONTLIST_DEFAULT_TAG);
  source_label =
    XtVaCreateManagedWidget("source_label",
			    xmLabelWidgetClass,
			    form1,
			    XmNlabelString,    xm_string,
			    XmNalignment,      XmALIGNMENT_BEGINNING,
			    XmNtopAttachment,  XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_WIDGET,
			    XmNleftWidget,     icon,
			    XmNleftOffset,     HORIZONTAL_SPACING,
			    NULL);
  XmStringFree(xm_string);

  xm_string = XmStringCreateLtoR(source, XmFONTLIST_DEFAULT_TAG);
  source_text_field =
    XtVaCreateManagedWidget("source_text_field",
			    xmLabelWidgetClass,
			    form1,
			    XmNlabelString,     xm_string,
			    XmNalignment,       XmALIGNMENT_BEGINNING,
			    XmNtopAttachment,   XmATTACH_OPPOSITE_WIDGET,
			    XmNtopWidget,       source_label,
			    XmNtopOffset,       0,
			    XmNleftAttachment,  XmATTACH_WIDGET,
			    XmNleftWidget,      source_label,
			    XmNrightAttachment, XmATTACH_FORM,
			    NULL);
  XmStringFree(xm_string);

  /* Create the Target Directory Widgets */
  xm_string = XmStringCreateLtoR(GETMESSAGE(3, 5, "Into folder:"), XmFONTLIST_DEFAULT_TAG);
  target_label =
    XtVaCreateManagedWidget("target_label",
			    xmLabelWidgetClass,
			    form1,
			    XmNlabelString,     xm_string,
			    XmNalignment,       XmALIGNMENT_BEGINNING,
			    XmNtopAttachment,   XmATTACH_WIDGET,
			    XmNtopWidget,       source_label,
			    XmNleftAttachment,  XmATTACH_WIDGET,
			    XmNleftWidget,      icon,
			    XmNleftOffset,      HORIZONTAL_SPACING,
			    NULL);
  XmStringFree(xm_string);

  /* modify width of shorter label so the labels are right-aligned */
  XtVaGetValues(target_label,
                XmNwidth, &width_target,
                NULL);
  XtVaGetValues(source_label,
                XmNwidth, &width_source,
                NULL);

  if (width_target < width_source)
  {
     XtVaSetValues(target_label,
                   XmNwidth,     width_source,
                   XmNalignment, XmALIGNMENT_END,
                   NULL);
  }
  else if (width_target > width_source)  /* do nothing if widths are the same */
  {
     XtVaSetValues(source_label,
                   XmNwidth,     width_target,
                   XmNalignment, XmALIGNMENT_END,
                   NULL);
  }

  split_path(target,target_folder,target_object);
  xm_string = XmStringCreateLtoR(target_folder, XmFONTLIST_DEFAULT_TAG);
  target_text_field =
    XtVaCreateManagedWidget("target_text_field",
			    xmLabelWidgetClass,
			    form1,
			    XmNlabelString,     xm_string,
			    XmNalignment,       XmALIGNMENT_BEGINNING,
			    XmNtopAttachment,   XmATTACH_OPPOSITE_WIDGET,
			    XmNtopWidget,       target_label,
			    XmNtopOffset,       0,
			    XmNleftAttachment,  XmATTACH_WIDGET,
			    XmNleftWidget,      target_label,
			    XmNrightAttachment, XmATTACH_FORM,
			    NULL);


  /* create copy-summary text field */
  if (G_move)
     xm_string = XmStringCreateLtoR(GETMESSAGE(3, 17, "Moved"), XmFONTLIST_DEFAULT_TAG);
  else
     xm_string = XmStringCreateLtoR(GETMESSAGE(3, 18, "Copied"), XmFONTLIST_DEFAULT_TAG);

  summary_label =
    XtVaCreateManagedWidget("target_label",
			    xmLabelWidgetClass,
			    form1,
			    XmNlabelString,     xm_string,
			    XmNalignment,       XmALIGNMENT_END,
			    XmNtopAttachment,   XmATTACH_WIDGET,
			    XmNtopWidget,       target_label,
			    XmNtopOffset,       TOP_SPACING,
			    XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
			    XmNrightWidget,     target_label,
			    XmNrightOffset,     0,
			    XmNleftAttachment,  XmATTACH_WIDGET,
			    XmNleftWidget,      icon,
			    NULL);
  XmStringFree(xm_string);

  G_summary_text =
    XtVaCreateManagedWidget("copy_summary",
			    xmLabelWidgetClass,
			    form1,
			    XmNtopAttachment,    XmATTACH_WIDGET,
			    XmNtopWidget,        target_label,
			    XmNtopOffset,        TOP_SPACING,
			    XmNleftAttachment,   XmATTACH_WIDGET,
			    XmNleftWidget,       summary_label,
			    XmNrightAttachment,  XmATTACH_FORM,
			    XmNalignment,        XmALIGNMENT_BEGINNING,
			    NULL);
			
			
   /* that is all that goes in form1 */
   XtManageChild(form1);

  /* create the action area of the Copy Directory Dialog */
  action_area =
    Create_Action_Area(form, copydirActions, G_copy_action_area_pbuttons);

  /* create the second separator widget */
  separator2 =
    XtVaCreateManagedWidget("separator",
			    xmSeparatorWidgetClass,
			    form,
			    XmNbottomAttachment,  XmATTACH_WIDGET,
			    XmNbottomWidget,      action_area,
			    XmNleftAttachment,    XmATTACH_FORM,
			    XmNrightAttachment,   XmATTACH_FORM,
			    NULL);

   /* create copy-status text field */
  G_status_text =
    XtVaCreateManagedWidget("copy_status",
			    xmTextFieldWidgetClass,
			    form,
			    XmNbackground,            background,
			    XmNforeground,            foreground,
/*
			    XmNcolumns,               1,
*/
			    XmNresizeWidth,           True,
			    XmNshadowThickness,       0,
			    XmNhighlightThickness,    0,
			    XmNtraversalOn,           False,
			    XmNeditable,              False,
			    XmNcursorPositionVisible, False,
			    XmNtopAttachment,         XmATTACH_WIDGET,
			    XmNtopWidget,             G_summary_text,
			    XmNtopOffset,             TOP_SPACING,
			    XmNrightAttachment,       XmATTACH_FORM,
			    XmNrightOffset,           INDENTED_SPACING,
			    XmNleftAttachment,        XmATTACH_FORM,
			    XmNleftOffset,            INDENTED_SPACING,
			    XmNbottomAttachment,      XmATTACH_WIDGET,
			    XmNbottomWidget,          separator2,
			    NULL);

  /* create copy-status text field */
/*
  G_status_text2 =
    XtVaCreateManagedWidget("copy_status",
			    xmTextFieldWidgetClass,
			    form,
			    XmNbackground,            background,
			    XmNforeground,            foreground,
			    XmNshadowThickness,       0,
			    XmNhighlightThickness,    0,
			    XmNtraversalOn,           False,
			    XmNeditable,              False,
			    XmNcursorPositionVisible, False,
			    XmNtopAttachment,         XmATTACH_WIDGET,
			    XmNtopWidget,             G_summary_text,
			    XmNtopOffset,             TOP_SPACING,
			    XmNleftAttachment,        XmATTACH_WIDGET,
			    XmNleftWidget,            G_status_text,
			    XmNrightAttachment,       XmATTACH_FORM,
			    XmNbottomAttachment,      XmATTACH_WIDGET,
			    XmNbottomWidget,          separator2,
			    NULL);
*/			

   /* create error-count text field ... not managed until there is an error */
  G_error_count =
    XtVaCreateWidget("error_count",
			    xmTextFieldWidgetClass,
			    form,
			    XmNbackground,            background,
			    XmNforeground,            foreground,
			    XmNshadowThickness,       0,
			    XmNhighlightThickness,    0,
			    XmNtraversalOn,           False,
			    XmNeditable,              False,
			    XmNcursorPositionVisible, False,
			    XmNtopAttachment,         XmATTACH_WIDGET,
			    XmNtopWidget,             G_status_text,
			    XmNtopOffset,             TOP_SPACING,
			    XmNleftAttachment,        XmATTACH_FORM,
			    XmNrightAttachment,       XmATTACH_FORM,
			    NULL);

  /* create error-message scroll window ... not managed until there is an error */
  G_error_msgs = create_error_text_window (form,
					   G_error_count,
					   separator2);

  /* set initial keyboard focus to the action button area */
  XtSetArg (args[0], XmNinitialFocus, action_area);
  XtSetValues(form, args, 1);

  /* Manage the Copy Directory Dialog */
  XtManageChild(G_copy_dialog);

  return;
}

