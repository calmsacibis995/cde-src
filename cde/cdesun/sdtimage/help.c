#ifndef lint
static char sccsid[] = "@(#)help.c 1.20 97/03/19";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/param.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Dt/HelpDialog.h>
#include <Dt/Help.h>
#include <Dt/IconFile.h>

#include "help.h"
#include "misc.h"
#include "imagetool.h"
#include "ui_imagetool.h"
#include "popup.h"


#define  IMAGETOOL_ABOUT_ICON    "SDtimageabout"

static Widget  help_dialog = (Widget) NULL;    /* Help Dialog widget */
static int     help_showing = False;           /* True if help showing */

AboutObjects  *about;

/*
 * Event handler to catch maps/unmaps of the
 * help dialog
 */
static void
HelpEventHandler (Widget     w,     
		  XtPointer  client_data,
		  XEvent    *event,
		  Boolean   *dispatch)
{
     if (event->type == MapNotify)
       help_showing = True;
     else if (event->type == UnmapNotify)
       help_showing = False;
}

static Widget
InitHelpDialog ()
{
     register int  ac;
     Arg           al[8];
     Widget        shell;
     extern void   HelpEventHandler();

     ac = 0;
     XtSetArg (al[ac], XmNtitle, catgets (prog->catd, 1, 59, "Image Viewer Help")); ac++;
     help_dialog = (Widget) DtCreateHelpDialog (base->top_level, 
						"MainHelpDialog", al, ac);
     /*
      * Add event handler to catch Maps/Unmaps.
      */
     shell = XtParent (help_dialog);
     XtAddEventHandler (shell, StructureNotifyMask, False,
			HelpEventHandler, NULL);
     
     return help_dialog;
}

/*
 * Callback for all items on help menu
 */
void
HelpMenuCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
     register int  n = 0;
     Arg           args[10];
     int           topic = (int) client_data;
     char         *location_id, *help_volume;

     setbusy();
     if (help_dialog == NULL)
       help_dialog = InitHelpDialog ();

/*
 * Need to check here if help volume was specified on 
 * command line or resource database.
 */
#ifdef LATER
	/* if (resources.helpVol != (char *) NULL)
	 *      helpVolume = resources.helpVol;
	 */ else
#endif
     help_volume = IMAGETOOL_HELP_VOLUME;
     switch (topic) {
     case HELP_OVERVIEW_EVENT:
       location_id = HELP_OVERVIEW_STR;
       break;
     case HELP_TASKS_EVENT:
       location_id = HELP_TASKS_STR;
       break;
     case HELP_REF_EVENT:
       location_id = HELP_REF_STR;
       break;
     case HELP_USING_HELP_EVENT:
       location_id = HELP_USING_HELP_STR;
       help_volume = "Help4Help";
       break;
     default:
       location_id = HELP_OVERVIEW_STR;
       break;
     }
     
/*
 * Set the help volume location id.
 */
     n = 0;
     XtSetArg (args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
     XtSetArg (args[n], DtNhelpVolume, help_volume); n++;
     XtSetArg (args[n], DtNlocationId, location_id); n++;
     XtSetValues (help_dialog, args, n);
/*
 * Display the help dialog.
 */
     ShowDialog (help_dialog, help_showing);
     setactive();

}

/*
 * Set up help dialog and display.
 */
void
HelpOnItemCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
     Widget   widget, top = base->top_level;

     switch (DtHelpReturnSelectedWidgetId (top, NULL, &widget)) {

     case DtHELP_SELECT_VALID:
       while (widget != NULL) {
	 if ((XtHasCallbacks (widget, XmNhelpCallback) == XtCallbackHasSome)) {
	   XtCallCallbacks (widget, XmNhelpCallback, NULL);
	   break;
	 }
	 else
	   widget = XtParent (widget);
       }
       break;

     case DtHELP_SELECT_INVALID:
       AlertPrompt (top, DIALOG_TYPE, XmDIALOG_INFORMATION,
		    DIALOG_TITLE, catgets (prog->catd, 1, 60, "Image Viewer Help"),
		    DIALOG_TEXT, catgets (prog->catd, 1, 61, "You must select an item within Image Viewer."),
		    BUTTON_IDENT, 0, catgets (prog->catd, 1, 62, "Continue"),
		    HELP_IDENT, HELP_USING_HELP_ITEM,
		    NULL);
       break;

     case DtHELP_SELECT_ERROR:
       fprintf (stderr, catgets (prog->catd, 1, 63, 
				 "Selection error, cannot continue.\n"));
       break;

     case DtHELP_SELECT_ABORT:
       fprintf (stderr, catgets (prog->catd, 1, 64,
				 "Selection aborted by user.\n"));
       break;
     }

}

/*
 * Generic callback when asking for Help on any widget.
 */
void
HelpItemCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
     int     ac;
     Arg     al[10];
     char   *location_id = (char *)client_data;

     setbusy();
     if (help_dialog == NULL)
       help_dialog = InitHelpDialog ();
     
     ac = 0;
     XtSetArg (al[ac], DtNhelpType, DtHELP_TYPE_TOPIC); ac++;
     XtSetArg (al[ac], DtNhelpVolume, IMAGETOOL_HELP_VOLUME); ac++;
     XtSetArg (al[ac], DtNlocationId, location_id); ac++;
     XtSetValues (help_dialog, al, ac);
     
     ShowDialog (help_dialog, help_showing);
     setactive();

}


AboutObjects *
AboutObjectsInitialize (parent)
     Widget parent;
{
     register int  ac = 0;
     Arg           al[16];
     Widget        children[5], form; 
     Widget        about_label1, about_label2, about_label3;
     XmString      xmstrings[5];
     char          text[512];
     char          actual_filename[MAXPATHLEN];
     AboutObjects *a;
     Pixel         fg, bg;
     Pixmap        icon = NULL, icon_mask, clipped_icon = NULL;
     Window        root;
     int           x, y;
     unsigned int  w = 0, h, border, depth;
     Dimension     w1;
     char         *icon_filename;
     extern void   PopupEventHandler ();
     extern void   AboutCancelCallback ();
     Display      *dpy = XtDisplay (base->top_level);
     Screen       *screen = XtScreen (base->top_level);
     XGCValues     gc_vals;
     GC            gc;
     unsigned long gc_mask;
     
     a = (AboutObjects *) calloc (1, sizeof (AboutObjects));
/*
 * Create Shell, Dialog, Forms.
 */	
     ac = 0;
     XtSetArg(al[ac], XmNdeleteResponse, XmDESTROY); ac++;
     XtSetArg(al[ac], XmNtitle, catgets (prog->catd, 1, 65, 
					 "About Image Viewer")); ac++;
     a->shell = XmCreateDialogShell ( parent, "AboutShell", al, ac );
/*
 * Create Message Dialog Box
 */
     ac = 0;
     XtSetArg(al[ac], XmNautoUnmanage, FALSE); ac++;
     XtSetArg(al[ac], XmNdialogType, XmDIALOG_TEMPLATE); ac++;	
     XtSetArg(al[ac], XmNnoResize, FALSE); ac++;
     a->dialog = XmCreateMessageBox (a->shell, "AboutDialog", al, ac );
/*
 * Add event handler to catch Maps/Unmaps.
 */
     XtAddEventHandler (a->shell, StructureNotifyMask, False,
			PopupEventHandler, a->dialog);
/*
 * Create Form
 */
     ac = 0;
     XtSetArg(al[ac], XmNautoUnmanage, FALSE); ac++;
     XtSetArg(al[ac], XmNfractionBase, 4); ac++;
     form = XmCreateForm (a->dialog, "AboutForm", al, ac );

/*
 * Getting CDE icon, else try local icon
 */
     icon_filename = XmGetIconFileName (screen, NULL, IMAGETOOL_ABOUT_ICON, NULL, 
					XmUNSPECIFIED_ICON_SIZE);
     actual_filename[0] = '\0';
     if (icon_filename) {
/*
 * Use bitmap if depth == 1
 */
       if (DefaultDepthOfScreen (XtScreen (base->top_level)) == 1) {
	 int len = strlen (icon_filename);
         if (len > 3 && 
	     icon_filename [len-1] == 'm' && icon_filename [len-2] == 'p' &&
	     icon_filename [len-3] == '.') {
	   icon_filename [len-2] = 'b';
	   if (access (icon_filename, R_OK) != 0)
	     icon_filename[len-2] = 'p';  /* put it back if not found */
	 }
       }
       sprintf (actual_filename, "%s", icon_filename);
       free (icon_filename);
     }
     
     if (actual_filename != NULL) {
       XtVaGetValues (form, XmNforeground, &fg,
		            XmNbackground, &bg, NULL);
       icon = XmGetPixmap (screen, actual_filename, fg, bg); 
       free (icon_filename);
       if (icon != XmUNSPECIFIED_PIXMAP)
	 XGetGeometry (dpy, icon, &root, &x, &y, &w, &h, &border, &depth);
     }
     a->about_icon = XtVaCreateWidget ("AboutIcon",
					xmLabelWidgetClass,
					form,
					XmNtopAttachment, XmATTACH_FORM,
					XmNtopOffset, 5,
					XmNleftAttachment, XmATTACH_FORM,
					XmNleftOffset, 10,
					NULL);

     if (icon == XmUNSPECIFIED_PIXMAP) {
       xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 66, 
							"Missing\nGraphics"));
       XtVaSetValues (a->about_icon,
		      XmNlabelType, XmSTRING,
		      XmNlabelString, xmstrings[0],
		      XmNborderWidth, 1,
		      NULL);
       XmStringFree(xmstrings[0]);
       XtVaGetValues (a->about_icon, XmNwidth, &w1, NULL);
       w = (int) w1;
     } 
     else {
       XtVaSetValues (a->about_icon,
 		      XmNlabelType, XmPIXMAP,
		      XmNlabelPixmap, icon,
		      NULL);
     }

/*
 * Create Text Label
 */
     ac = 0;
     sprintf (text, catgets (prog->catd, 1, 67, "Image Viewer Version %s"),
			     IMAGETOOL_VERSION);
     xmstrings[0] = XmStringCreateLocalized (text);
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNtopOffset, 10); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftOffset, w + 10 + 20); ac++;
     about_label1 = XmCreateLabel ( form, "AboutLabel1", al, ac );
     XmStringFree ( xmstrings [ 0 ] );

     ac = 0;
     sprintf (text, catgets (prog->catd, 1, 465, "Copyright (c) 1994 - 1997:"));
     xmstrings[0] = XmStringCreateLocalized (text);
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, about_label1); ac++;
     XtSetArg(al[ac], XmNtopOffset, 10); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, about_label1); ac++;
     about_label2 = XmCreateLabel ( form, "AboutLabel2", al, ac );
     XmStringFree ( xmstrings [ 0 ] );

     ac = 0;
     sprintf (text, catgets (prog->catd, 1, 69, "Sun Microsystems, Inc."));
     xmstrings[0] = XmStringCreateLocalized (text);
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, about_label2); ac++;
     XtSetArg(al[ac], XmNtopOffset, 3); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, about_label1); ac++;
     XtSetArg(al[ac], XmNleftOffset, 25); ac++;
     about_label3 = XmCreateLabel ( form, "AboutLabel3", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Create Close button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 70, 
						      "Close"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     a->close_button = XmCreatePushButton (a->dialog, "AboutCloseButton", 
					    al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Set Close to be default button.
 */
     XtVaSetValues (a->dialog, XmNdefaultButton, a->close_button, NULL);
/*
 * Set callbacks on buttons
 */
     XtAddCallback (a->close_button, XmNactivateCallback, 
		    AboutCancelCallback, a);
/*
 * Manage the children
 */
     ac = 0;
     children[ac++] = a->about_icon;
     children[ac++] = about_label1;
     children[ac++] = about_label2;
     children[ac++] = about_label3;
     XtManageChildren(children, ac);
     ac = 0;
     children[ac++] = form;
     children[ac++] = a->close_button;
     XtManageChildren(children, ac);

     return a;
}

/*
 * Callback for About Image Viewer...
 */
void
HelpAboutCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
     int     ac;
     Arg     al[10];
     char   *location_id = (char *)client_data;
     
     setbusy();

     if (!about) 
       about = AboutObjectsInitialize (base->top_level);

     ShowDialog (about->dialog, about->showing);

     setactive();

}

void
AboutCancelCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
     AboutObjects *a = (AboutObjects *)client_data;

     XtPopdown (XtParent (a->dialog));
}
