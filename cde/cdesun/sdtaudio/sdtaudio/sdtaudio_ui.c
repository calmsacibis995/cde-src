/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)sdtaudio_ui.c	1.28 97/06/04 SMI"

#include <stdlib.h>
#include <thread.h>
#include <nl_types.h>
#include <unistd.h>
#include <sys/param.h>

#include <X11/Intrinsic.h>
#include <X11/Core.h>
#include <Xm/XmAll.h>
#include <Dt/Dnd.h>
#include <Dt/Dts.h>
#include <Dt/HelpDialog.h>
#include <Dt/IconFile.h>
#include <Tt/tttk.h>

#include "sdtaudio.h"
#include "help.h"
#include "extern.h"
#include "utils.h"
#include "callbacks.h"
#include "cb_utils.h"

/* Globals */
extern XFontStruct *ww_font;
extern int          ww_font_height;
extern WindowData  *wd;

/* XXX Change this later when you get a real about size icon */
#define AUDIO_ABOUT_ICON	"Dtaudio.m"
#define AUDIO_VERSION		"1.0"

/*------------------- Functions internal to this file ------------------------*/

/* Function	: SDtAuCreateBasePanel()
 * Objective	: Create the main panel of the sdtaudio window.
 * Arguments	: WindowData *	- the sdtaudio data structure
 * Return value : Boolean true if the base form was created, 
 *		  false otherwise.
 */
Boolean 
SDtAuCreateBasePanel(WindowData *wd)
{
	/* Create a form */
	wd->base_form = XmCreateForm(wd->mainWindow, "BaseForm", NULL, 0);
	if (wd->base_form == NULL)
		return(False);

	/* Set the base form to be the work window. */
	XmMainWindowSetAreas(wd->mainWindow, NULL, NULL, NULL, NULL,
			     wd->base_form);
	return(True);
}


/* Function	: SDtAuCreateFileMenu()
 * Objective	: Create the File menu found in the menu bar, and its
 *		  associated menu items.
 * Arguments	: Widget	- the menu bar that the File menu will be
 *				  associated with
 * 		  WindowData *	- the sdtaudio data structure
 * Return value : Boolean true if successful, false otherwise.
 */
Boolean 
SDtAuCreateFileMenu(Widget menubar, WindowData *wd)
{
	Widget          pd;	       /* pulldown menu widget */
	Widget          cb;	       /* cascade button */
	Widget          pb;	       /* push button */
	Widget          sep;	       /* separator */
	XmString        labelString;
	Arg             args[20];
	int             n;
	static SDtAudioHelpR help_rec;

	/* Create the File pulldown menu. */
	pd = XmCreatePulldownMenu(menubar, "FilePulldown", NULL, 0);
	if (pd == NULL)
		return(False);

	/* Create the cascade menu. */
	labelString = 
		XmStringCreateLocalized(catgets(msgCatalog, 2, 1, "File"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	XtSetArg(args[n], XmNsubMenuId, pd); ++n;
	cb = XmCreateCascadeButton(menubar, "FileCascade", args, n);
	if (cb == NULL)
		return(False);
	XtManageChild(cb);
	XmStringFree(labelString);
	help_rec.help_parent_win = wd->mainWindow;
	help_rec.help_event = HELP_AUDIO_FILE_MENU_EVENT;
	help_rec.wd = wd;
	XtAddCallback(cb, XmNhelpCallback, HelpCb, (XtPointer) &help_rec);


	/* New */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 9, 
						      "New"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "NewItem", args, n);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	XtAddCallback(pb, XmNactivateCallback, NewCb, (XtPointer) wd);
	XmStringFree(labelString);

	/* Open */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 2, 
						      "Open..."));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "OpenItem", args, n);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	XtAddCallback(pb, XmNactivateCallback, OpenCb, (XtPointer) wd);
	XmStringFree(labelString);

	/* Save As */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 3, 
						      "Save As..."));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	XtSetArg(args[n], XmNsensitive, False); ++n;
	wd->menu_items->SaveAsItem = 
		XmCreatePushButton(pd, "SaveAsItem", args, n);
	if (wd->menu_items->SaveAsItem == NULL)
		return(False);
	XtManageChild(wd->menu_items->SaveAsItem);
	XtAddCallback(wd->menu_items->SaveAsItem, XmNactivateCallback, 
		      SaveAsCb, (XtPointer) wd);
	XmStringFree(labelString);

	/* Separator */
	n = 0;
	XtSetArg(args[n], XmNsensitive, False); ++n;
	sep = XmCreateSeparator(pd, "Separator", args, n);
	if (sep == NULL)
		return(False);
	XtManageChild(sep);

	/* Get Info */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 17,
						      "Get Info..."));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	XtSetArg(args[n], XmNsensitive, False); ++n;
	wd->menu_items->InfoItem = XmCreatePushButton(pd, "InfoItem", 
						      args, n);
	if (wd->menu_items->InfoItem == NULL)
		return(False);
	XtManageChild(wd->menu_items->InfoItem);
	XtAddCallback(wd->menu_items->InfoItem, XmNactivateCallback, 
		      InfoCb, (XtPointer) wd);
	XmStringFree(labelString);

	/* Separator */
	n = 0;
	XtSetArg(args[n], XmNsensitive, False); ++n;
	sep = XmCreateSeparator(pd, "Separator", args, n);
	if (sep == NULL)
		return(False);
	XtManageChild(sep);

	/* Exit */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 6, 
						      "Exit"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "ExitItem", args, n);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	XtAddCallback(pb, XmNactivateCallback, ExitCb, (XtPointer) wd);
	XmStringFree(labelString);
	return(True);
}

/* Function	: SDtAuCreateOptionsMenu()
 * Objective	: Create the Options menu found in the menu bar, and its
 *		  associated menu items.
 * Arguments	: Widget	- the menu bar that the Options menu will be
 *				  associated with
 * 		  WindowData *	- the sdtaudio data structure
 * Return value : Boolean true if successful, false otherwise.
 */
Boolean 
SDtAuCreateOptionsMenu(Widget menuBar, WindowData * wd)
{
	Widget          pd;	/* pulldown widget */
	Widget          cb;	/* cascade button */
	Widget          pb;	/* pushbutton */
	Widget          sep;    /* separator */
	XmString        labelString;
	Arg             args[20];
	int             n;
	static SDtAudioHelpR help_rec;

	/* Create the Options pulldown menu. */
	pd = XmCreatePulldownMenu(menuBar, "OptionsPulldown", NULL, 0);
	if (pd == NULL)
		return(False);

	/* Create the cascade menu. */
	labelString = 
		XmStringCreateLocalized(catgets(msgCatalog, 2, 4, 
						"Options"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	XtSetArg(args[n], XmNsubMenuId, pd); ++n;
	cb = XmCreateCascadeButton(menuBar, "OptionsCascade", args, n);
	XmStringFree(labelString);
	if (cb == NULL)
		return(False);
	XtManageChild(cb);
	help_rec.help_parent_win = wd->mainWindow;
	help_rec.help_event = HELP_AUDIO_OPTIONS_MENU_EVENT;
	help_rec.wd = wd;
	XtAddCallback(cb, XmNhelpCallback, HelpCb, (XtPointer) &help_rec);

	/* Startup */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 20, 
						      "Startup..."));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "StartupItem", args, n);
	XmStringFree(labelString);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	XtAddCallback(pb, XmNactivateCallback, OptionsCb, (XtPointer) wd);

	/* Play */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 1, 17, 
						      "Play..."));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "PlayItem", args, n);
	XmStringFree(labelString);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	XtAddCallback(pb, XmNactivateCallback, OptionsCb, (XtPointer) wd);

	/* Record */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 1, 18, 
						      "Record..."));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "RecordItem", args, n);
	XmStringFree(labelString);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	XtAddCallback(pb, XmNactivateCallback, OptionsCb, (XtPointer) wd);

	/* Devices */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 73, 
						      "Devices..."));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "DevicesItem", args, n);
	XmStringFree(labelString);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	XtAddCallback(pb, XmNactivateCallback, OptionsCb, (XtPointer) wd);
	return(True);
}

/* Function	: SDtAuSDtAuCreateHelpMenu()
 * Objective	: Create the Help menu found in the menu bar, and its
 *		  associated menu items.
 * Arguments	: Widget	- the menu bar that the Help menu will be
 *				  associated with
 * 		  WindowData *	- the sdtaudio data structure
 * Return value : Boolean true if successful, false otherwise.
 */
Boolean 
SDtAuCreateHelpMenu(Widget menuBar, WindowData * wd)
{
	Widget          pd;	/* pulldown widget */
	Widget          cb;	/* cascade button */
	Widget          pb;	/* pushbutton */
	Widget          sep;    /* separator */
	XmString        labelString;
	Arg             args[20];
	int             n;
	static SDtAudioHelpR	help_rec, overview_help, tasks_help,
				reference_help, using_help;

	/* Create the help pulldown menu. */
	pd = XmCreatePulldownMenu(menuBar, "HelpPulldown", NULL, 0);
	if (pd == NULL)
		return(False);

	/* Create the cascade menu. */
	labelString = 
		XmStringCreateLocalized(catgets(msgCatalog, 2, 7, "Help"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	XtSetArg(args[n], XmNsubMenuId, pd); ++n;
	cb = XmCreateCascadeButton(menuBar, "HelpCascade", args, n);
	if (cb == NULL)
		return(False);
	XtManageChild(cb);
	XmStringFree(labelString);
	XtVaSetValues(menuBar, XmNmenuHelpWidget, cb, NULL);
	help_rec.help_parent_win = wd->mainWindow;
	help_rec.help_event = HELP_AUDIO_HELP_MENU_EVENT;
	help_rec.wd = wd;
	XtAddCallback(cb, XmNhelpCallback, HelpCb, (XtPointer) &help_rec);

	/* Overview */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 8, 
						      "Overview..."));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "OverviewItem", args, n);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	overview_help.help_parent_win = wd->mainWindow;
	overview_help.help_event = HELP_OVERVIEW_EVENT;
	overview_help.wd = wd;
	XtAddCallback(pb, XmNactivateCallback, HelpCb, 
		      (XtPointer) &overview_help);
	XmStringFree(labelString);

	/* separator. */
	n = 0;
	XtSetArg(args[n], XmNsensitive, False); ++n;
	sep = XmCreateSeparator(pd, "Separator", args, n);
	if (sep == NULL)
		return(False);
	XtManageChild(sep);

	/* Tasks */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 57, 
						      "Tasks..."));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "TasksItem", args, n);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	tasks_help.help_parent_win = wd->mainWindow;
	tasks_help.help_event = HELP_TASKS_EVENT;
	tasks_help.wd = wd;
	XtAddCallback(pb, XmNactivateCallback, HelpCb, 
		      (XtPointer) &tasks_help);
	XmStringFree(labelString);

	/* Reference */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 58, 
						      "Reference..."));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "ReferenceItem", args, n);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	reference_help.help_parent_win = wd->mainWindow;
	reference_help.help_event = HELP_REF_EVENT;
	reference_help.wd = wd;
	XtAddCallback(pb, XmNactivateCallback, HelpCb, 
		      (XtPointer) &reference_help);
	XmStringFree(labelString);

	/* On Item */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 59, 
						      "On Item"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "OnItemItem", args, n);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	XtAddCallback(pb, XmNactivateCallback, OnItemCb, 
		      (XtPointer) wd->toplevel);
	XmStringFree(labelString);

	/* separator. */
	n = 0;
	XtSetArg(args[n], XmNsensitive, False); ++n;
	sep = XmCreateSeparator(pd, "Separator", args, n);
	if (sep == NULL)
		return(False);
	XtManageChild(sep);

	/* Using Help */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 60, 
						      "Using Help..."));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "UsingHelpItem", args, n);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	using_help.help_parent_win = wd->mainWindow;
	using_help.help_event = HELP_USING_HELP_EVENT;
	using_help.wd = wd;
	XtAddCallback(pb, XmNactivateCallback, HelpCb, 
		      (XtPointer) &using_help);
	XmStringFree(labelString);

	/* separator. */
	n = 0;
	XtSetArg(args[n], XmNsensitive, False); ++n;
	sep = XmCreateSeparator(pd, "Separator", args, n);
	if (sep == NULL)
		return(False);
	XtManageChild(sep);

	/* About Audio */
	labelString = XmStringCreateLocalized(catgets(msgCatalog, 2, 61, 
						      "About Audio..."));
	n = 0;
	XtSetArg(args[n], XmNlabelString, labelString); ++n;
	pb = XmCreatePushButton(pd, "AboutAudioItem", args, n);
	if (pb == NULL)
		return(False);
	XtManageChild(pb);
	XtAddCallback(pb, XmNactivateCallback, AboutCb, wd);
	XmStringFree(labelString);
	return(True);
}


/* Function	: SDtAuCreateMenus()
 * Objective	: Populate the menu bar associated with the sdtaudio
 *		  window.
 * Arguments	: WindowData *		- the sdtaudio data structure
 * Return Value : Boolean True if the menu bar and its children were
 *		  successfully created, False otherwise.
 */
Boolean 
SDtAuCreateMenus(WindowData * wd)
{
	Widget    menu_bar;

	menu_bar = XmCreateMenuBar(wd->mainWindow, "MenuBar", NULL, 0);
	if (menu_bar == NULL)
		return(False);

	if (!SDtAuCreateFileMenu(menu_bar, wd))
		return(False);

	if (!SDtAuCreateOptionsMenu(menu_bar, wd))
		return(False);

	if (!SDtAuCreateHelpMenu(menu_bar, wd))
		return(False);

	/* Set the menu bar area of the main window, and return. */
	XtManageChild(menu_bar);
	XmMainWindowSetAreas(wd->mainWindow, menu_bar, NULL, NULL, NULL, NULL);
	return(True);
}


/* Function	: SDtAuCreateFooter()
 * Objective	: Create the footer areas of the sdtaudio window.
 * Arguments	: WindowData *		- the sdtaudio data structure
 * Return Value : Boolean True if the footer areas were create.
 */
Boolean 
SDtAuCreateFooter(WindowData *wd)
{
	Widget		footer_pane;
	XmString        label_xmstring = NULL;
	Arg             args[20];
	int             n;

	/* Create the footer pane. */
	n = 0;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); ++n;
	footer_pane = XtCreateWidget("FooterPane", xmFormWidgetClass, 
				     wd->mainWindow, args, n);

	/* Create the general message area. */
	n = 0;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNlabelString, label_xmstring); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); ++n;
	wd->footer_msg = XmCreateLabel(footer_pane, "FooterMsg", args, n);
	if (wd->footer_msg == NULL)
		return(False);
	XtManageChild(wd->footer_msg);

	/* Create the status message area. */
	n = 0;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNlabelString, label_xmstring); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); ++n;
	wd->status_msg = XmCreateLabel(footer_pane, "StatusMsg", args, n);
	if (wd->status_msg == NULL)
		return(False);
	XtManageChild(wd->status_msg);

	/* Set the footer pane as the message area of the main
	 * window and return.
	 */
	XtManageChild(footer_pane);
	XtVaSetValues(wd->mainWindow, 
		      XmNmessageWindow, footer_pane,
		      NULL);
	return(True);
}

/* Function	: SDtAuCreateMainButtonGroup()
 * Objective	: Create the main sdtaudio operation buttons - play,
 *		  rewind, forward, record, as well as the record light.
 * Argument	: Widget		- parent of the form that will hold
 *					  the above-mentioned GUI items
 *		  WindowData *		- the sdtaudio structure
 * Return Value : Widget		- the form that contains all these
 *					  buttons
 */
Widget
SDtAuCreateMainButtonGroup(Widget parent, WindowData *wd)
{
	Widget          main_group = (Widget) NULL;
	Arg             args[20];
        register int    n;

	/* Create the main buttons group - play, rewind, forward, record
	 * and the record light.
	 */
	n = 0;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); ++n;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	main_group = XmCreateForm(parent, "MainButtonsGroup", args, n);
	if (main_group == (Widget) NULL)
		return((Widget) NULL);

	/* Play button */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrecomputeSize, False); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	XtSetArg(args[n], XmNwidth, CONTROL_BUTTON_WIDTH); ++n;
	XtSetArg(args[n], XmNheight, CONTROL_BUTTON_HEIGHT); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	wd->play_button =  XmCreatePushButton(main_group, "PlayButton",
					      args, n);
	if (wd->play_button == NULL)
		return((Widget) NULL);

	/* Assign the Play pixmap to the play button. */
	dtb_set_label_from_image_file(wd->play_button, "SDtAudply.s");
	XtVaGetValues(wd->play_button,
		      XmNlabelPixmap, &(wd->play_pixmap),
		      XmNlabelInsensitivePixmap, &(wd->ins_play_pixmap),
		      NULL);

	/* Create the stop pixmap - this will be superimposed on the
	 * play button when the tool is playing a file.  We're making
	 * the play button act as stop button as well when playing a
	 * file, rather than having a dedicated stop button.  So, we
	 * need a stop pixmap for the play button as well.  In addition,
	 * the same holds true for the record button.
	 */
	dtb_cvt_image_file_to_pixmap(wd->play_button, "SDtAudstp.s",
				     &(wd->stop_pixmap));
	wd->ins_stop_pixmap = dtb_create_greyed_pixmap(wd->play_button,
						       wd->stop_pixmap);

	/* Assign callbacks for play button, and manage it. */
	XtAddCallback(wd->play_button, XmNactivateCallback, PlayCb, wd);
	XtAddEventHandler(wd->play_button, EnterWindowMask, False,
			  EnterButtonCb, wd);
	XtAddEventHandler(wd->play_button, LeaveWindowMask, False,
			  LeaveButtonCb, wd);
	XtManageChild(wd->play_button);

	/* Create the rewind button */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->play_button); ++n;
	XtSetArg(args[n], XmNwidth, CONTROL_BUTTON_WIDTH); ++n;
	XtSetArg(args[n], XmNheight, CONTROL_BUTTON_HEIGHT); ++n;
	XtSetArg(args[n], XmNrecomputeSize, False); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	wd->bwd_button = XmCreatePushButton(main_group, "BackwardButton",
					    args, n);
	if (wd->bwd_button == NULL)
		return((Widget) NULL);

	/* Assign the pixmap and callbacks to the rewind button,
	 * and then manage it.
	 */
	dtb_set_label_from_image_file(wd->bwd_button, "SDtAudrev.s");
	XtAddEventHandler(wd->bwd_button, EnterWindowMask, False,
			  EnterButtonCb, wd);
	XtAddEventHandler(wd->bwd_button, LeaveWindowMask, False,
			  LeaveButtonCb, wd);
	XtAddCallback(wd->bwd_button, XmNactivateCallback, SetPositionCb, wd);
	XtManageChild(wd->bwd_button);

	/* Create the forward button. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->bwd_button); ++n;
	XtSetArg(args[n], XmNwidth, CONTROL_BUTTON_WIDTH); ++n;
	XtSetArg(args[n], XmNheight, CONTROL_BUTTON_HEIGHT); ++n;
	XtSetArg(args[n], XmNrecomputeSize, False); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	wd->fwd_button = XmCreatePushButton(main_group, "ForwardButton",
					    args, n);
	if (wd->fwd_button == NULL)
		return((Widget) NULL);

	/* Assign the pixmap and callbacks to the forward button,
	 * then manage it.
	 */
	dtb_set_label_from_image_file(wd->fwd_button, "SDtAudfor.s");
	XtAddEventHandler(wd->fwd_button, EnterWindowMask, False,
			  EnterButtonCb, wd);
	XtAddEventHandler(wd->fwd_button, LeaveWindowMask, False,
			  LeaveButtonCb, wd);
	XtAddCallback(wd->fwd_button, XmNactivateCallback, SetPositionCb, wd);
	XtManageChild(wd->fwd_button);

	/* Record button. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->fwd_button); ++n;
	XtSetArg(args[n], XmNwidth, CONTROL_BUTTON_WIDTH); ++n;
	XtSetArg(args[n], XmNheight, CONTROL_BUTTON_HEIGHT); ++n;
	XtSetArg(args[n], XmNrecomputeSize, False); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	wd->record_button = XmCreatePushButton(main_group, "RecordButton", 
					       args, n);
	if (wd->record_button == NULL)
		return((Widget) NULL);

	/* Assign the pixmap and callbacks to the record button,
	 * and manage it.
	 */
	dtb_set_label_from_image_file(wd->record_button, "SDtAudrec.s");
	XtVaGetValues(wd->record_button,
		      XmNlabelPixmap, &(wd->record_pixmap),
		      XmNlabelInsensitivePixmap, &(wd->ins_record_pixmap),
		      NULL);
	XtAddCallback(wd->record_button, XmNactivateCallback, RecordCb, wd);
	XtAddEventHandler(wd->record_button, EnterWindowMask, False,
			  EnterButtonCb, wd);
	XtAddEventHandler(wd->record_button, LeaveWindowMask, False,
			  LeaveButtonCb, wd);
	XtManageChild(wd->record_button);

	/* Create the record light. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->record_button); ++n;
	XtSetArg(args[n], XmNleftOffset, RECORD_BLINKER_OFFSET); ++n;
	XtSetArg(args[n], XmNwidth, RECORD_BLINKER_WIDTH); ++n;
	XtSetArg(args[n], XmNheight, CONTROL_BUTTON_HEIGHT); ++n;
	XtSetArg(args[n], XmNrecomputeSize, False); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	wd->record_light = XmCreateLabel(main_group, "RecordLightLabel",
					 args, n);
	if (wd->record_light == NULL)
		return((Widget) NULL);

	/* Assign the pixmap to the record light - one pixmap when it
	 * is on, the other when it is off.
	 */
	dtb_set_label_from_image_file(wd->record_light, "SDtAudrbf.s");
	XtVaGetValues(wd->record_light,
		      XmNlabelPixmap, &(wd->rec_blink_off_pixmap),
		      XmNlabelInsensitivePixmap, &(wd->ins_blink_off_pixmap),
		      NULL);
	dtb_cvt_image_file_to_pixmap(wd->record_light, "SDtAudrcb.s",
				     &(wd->rec_blink_pixmap));
	wd->ins_blink_pixmap = dtb_create_greyed_pixmap(wd->record_light, 
					                wd->rec_blink_pixmap);
	XtManageChild(wd->record_light);

	/* Manage the main button group, and return. */
	XtManageChild(main_group);
	return(main_group);
}


/* Function	: SDtAuCreateVolumeButtonsGroup()
 * Objective	: Create the sdtaudio buttons that deal with volume - the,
 *		  mute button, volume gauge, up and down volume buttons
 * Argument	: Widget		- parent of the form that will hold
 *					  the above-mentioned GUI items
 *		  WindowData *		- the sdtaudio structure
 * Return Value : Widget		- the form that contains all these
 *					  buttons
 */
Widget
SDtAuCreateVolumeButtonsGroup(Widget parent, WindowData *wd)
{
	Widget          vol_group = (Widget) NULL;
	Arg             args[20];
        register int    n;
	SAError		error;

	/* Now create the form for the other incendiary buttons,
         * like mute, volume control.
	 */
	n = 0;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); ++n;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	vol_group = XmCreateForm(parent, "VolumeGroup", args, n);
	if (vol_group == NULL)
		return((Widget) NULL);

	/* Create the mute button. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNwidth, MUTE_BUTTON_WIDTH); ++n;
	XtSetArg(args[n], XmNheight, CONTROL_BUTTON_HEIGHT); ++n;
	XtSetArg(args[n], XmNrecomputeSize, False); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	wd->mute_button = XmCreateDrawnButton(vol_group, "MuteButton",
					     args, n);
	if (wd->mute_button == NULL)
		return((Widget) NULL);

	/* Assign the pixmap and callbacks to the mute button, and
	 * manage it.
	 */
	dtb_set_label_from_image_file(wd->mute_button, "SDtAudmut.s");
	XtAddCallback(wd->mute_button, XmNactivateCallback, MuteButtonCb, wd);
	XtAddEventHandler(wd->mute_button, EnterWindowMask, False,
			  EnterButtonCb, wd);
	XtAddEventHandler(wd->mute_button, LeaveWindowMask, False,
			  LeaveButtonCb, wd);
	XtManageChild(wd->mute_button);
	if (wd->is_muted)
		XtVaSetValues(wd->mute_button,
			      XmNshadowType, XmSHADOW_ETCHED_IN,
			      NULL);
	else
		XtVaSetValues(wd->mute_button,
			      XmNshadowType, XmSHADOW_ETCHED_OUT,
			      NULL);

	/* Create the wave button */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->mute_button); ++n;
	XtSetArg(args[n], XmNtopOffset, PROGRESS_BAR_TOP_GAP); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->mute_button); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, wd->mute_button); ++n;
	XtSetArg(args[n], XmNheight, GAUGE_WIDTH); ++n;
	XtSetArg(args[n], XmNrecomputeSize, False); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); ++n;
	wd->wave_button = XmCreateDrawnButton(vol_group, "WaveButton",
					      args, n);
	if (wd->wave_button == NULL)
		return((Widget) NULL);

	/* Assign the pixmap and callbacks to the wave button, and
	 * then manage the widget.
	 */
	dtb_set_label_from_image_file(wd->wave_button, "SDtAudwav.s");
	XtAddCallback(wd->wave_button, XmNactivateCallback,
		      ShowWaveWindowCb, wd);
	XtAddEventHandler(wd->wave_button, EnterWindowMask, False,
			  EnterButtonCb, wd);
	XtAddEventHandler(wd->wave_button, LeaveWindowMask, False,
			  LeaveButtonCb, wd);
	XtManageChild(wd->wave_button);

	/* Create the volume gauge. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNbottomWidget, wd->wave_button); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->mute_button); ++n;
	XtSetArg(args[n], XmNleftOffset, VOL_WIDGET_OFFSET); ++n;
	XtSetArg(args[n], XmNshowValue, False); ++n;
	XtSetArg(args[n], XmNvalue, wd->options->playVolume); ++n;
	XtSetArg(args[n], XmNscaleMultiple, 1); ++n;
	XtSetArg(args[n], XmNminimum, 0); ++n;
	XtSetArg(args[n], XmNmaximum, 100); ++n;
	XtSetArg(args[n], XmNwidth, GAUGE_WIDTH); ++n;
	XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_TOP); ++n;
	XtSetArg(args[n], XmNorientation, XmVERTICAL); ++n;
	XtSetArg(args[n], "slidingMode", True); ++n;
	XtSetArg(args[n], XmNorientation, XmVERTICAL); ++n;
	wd->vol_gauge = XmCreateScale(vol_group, "VolumeScale",
				      args, n);
	if (wd->vol_gauge == NULL)
		return((Widget) NULL);

	XtManageChild(wd->vol_gauge);

	/* Set the device volume given the vol_levl.  Currently, if
	 * setting fails, we ignore it, since we're just creating the
	 * GUI right now.  However, do we need to do something if
	 * the volume can't be set to the user's specification?
	 */
	(void) SDtAudioSetPlayVol(wd->sa_stream, wd->options->playVolume, 
				  &error);

	/* Create the up volume button. */
	n = 0;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); ++n;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->vol_gauge); ++n;
	XtSetArg(args[n], XmNleftOffset, VOL_WIDGET_OFFSET); ++n;
	XtSetArg(args[n], XmNrecomputeSize, True); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	wd->up_vol = XmCreatePushButton(vol_group, "UpVolumeButton",
					args, n);
	if (wd->up_vol == NULL)
		return((Widget) NULL);

	/* Assign the pixmap and callbacks associated with the up
	 * volume button, and manage the widget.
	 */
	dtb_set_label_from_image_file(wd->up_vol, "SDtAudmrs.s");
	XtAddCallback(wd->up_vol, XmNactivateCallback, ChangeVolCb, wd);
	XtAddEventHandler(wd->up_vol, EnterWindowMask, False,
			  EnterButtonCb, wd);
	XtAddEventHandler(wd->up_vol, LeaveWindowMask, False,
			  LeaveButtonCb, wd);
	XtManageChild(wd->up_vol);

	/* Create the down volume button. */
	n = 0;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); ++n;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNbottomWidget, wd->wave_button); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->up_vol); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, wd->up_vol); ++n;
	XtSetArg(args[n], XmNrecomputeSize, True); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	wd->down_vol = XmCreatePushButton(vol_group, "DownVolumeButton",
					  args, n);
	if (wd->down_vol == NULL)
		return((Widget) NULL);

	/* Assign pixmap and callbacks to down button, and manage
	 * the widget.
	 */
	dtb_set_label_from_image_file(wd->down_vol, "SDtAudlss.s");
	XtAddCallback(wd->down_vol, XmNactivateCallback, ChangeVolCb, wd);
	XtAddEventHandler(wd->down_vol, EnterWindowMask, False,
			  EnterButtonCb, wd);
	XtAddEventHandler(wd->down_vol, LeaveWindowMask, False,
			  LeaveButtonCb, wd);
	XtManageChild(wd->down_vol);

	/* Manage the volume buttons group, and return. */
	XtManageChild(vol_group);
	return(vol_group);
}


/* Function	: SDtAuCreateMetersGroup()
 * Objective	: Create the form that will hold the progress and level
 *		  meters.
 * Arguments	: Widget		- parent of the meter form
 * 		  WindowData * 		- the sdtaudio structure
 * Return Value	: Widget		- the meter form that is created
 */
Widget
SDtAuCreateMetersGroup(Widget parent, WindowData *wd)
{
	Widget          meter_group = (Widget) NULL;
	Widget		level_frame = (Widget) NULL;
	XmString        label_str;
	Arg             args[20];
        register int    n;
	static SDtAudioHelpR help_rec;

	/* Create the meter form. */
	n = 0;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); ++n;
	meter_group = XmCreateForm(parent, "MeterGroup", args, n);
	if (meter_group == NULL)
		return((Widget) NULL);

	/* Create the progress indicator. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNshowValue, False); ++n;
	XtSetArg(args[n], XmNvalue, 0); ++n;
	XtSetArg(args[n], XmNscaleMultiple, 1); ++n;
	XtSetArg(args[n], XmNminimum, 0); ++n;
	XtSetArg(args[n], XmNmaximum, 100); ++n;
	XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); ++n;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
	XtSetArg(args[n], "slidingMode", False); ++n;
	XtSetArg(args[n], XmNscaleHeight, GAUGE_WIDTH); ++n;
	wd->prog_scale = XmCreateScale(meter_group, "ProgressScale", args, n);
	if (wd->prog_scale == NULL)
		return((Widget) NULL);
	XtAddCallback(wd->prog_scale, XmNvalueChangedCallback,
		      ProgScaleChangedCb, (XtPointer) wd);
	XtAddCallback(wd->prog_scale, XmNdragCallback,
		      ProgScaleDragCb, (XtPointer) wd);
	XtManageChild(wd->prog_scale);

	/* Create the frame around the level meter, but do not manage it. */
	n = 0;
	XtSetArg(args[n], XmNshadowType, XmSHADOW_IN); ++n;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); ++n;
	level_frame = XmCreateFrame(meter_group, "LevelMeterFrame", args, n);
	if (level_frame == NULL)
		return(False);
	help_rec.help_parent_win = wd->mainWindow;
	help_rec.help_event = HELP_AUDIO_WAVE_WIN_EVENT;
	help_rec.wd = wd;
	XtAddCallback(level_frame, XmNhelpCallback, HelpCb, 
		      (XtPointer) &help_rec);

	/* Create the level meter. 4 pixels is subtracted due to the frame
	 * that surrounds the level meter.  The level meter is supposed
	 * to be the same size as the wave window button.
	 */
	n = 0;
	XtSetArg(args[n], XmNheight, GAUGE_WIDTH - 4); ++n;
	XtSetArg(args[n], XmNbackground, 
		 BlackPixel(XtDisplay(wd->mainWindow), 
		            DefaultScreen(XtDisplay(wd->mainWindow))));
	++n;
	wd->level_meter = XmCreateDrawingArea(level_frame, "LevelMeter",
					      args, n);
	if (wd->level_meter == NULL)
		return((Widget) NULL);
	XtAddCallback(wd->level_meter, XmNexposeCallback,
		      LevelMeterRefreshCb, wd);
	XtManageChild(wd->level_meter);

	/* Create the progress start label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 51, "0:00"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->prog_scale); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrecomputeSize, True); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->prog_start = XmCreateLabel(meter_group, "ProgressStartLabel",
				       args, n);
	if (wd->prog_start == NULL)
		return(False);
	XmStringFree(label_str);
	XtManageChild(wd->prog_start);

	/* Create the progress end label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 52, "-:--"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->prog_scale); ++n;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrecomputeSize, True); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->prog_end = XmCreateLabel(meter_group, "ProgressEndLabel",
				     args, n);
	if (wd->prog_end == NULL)
		return(False);
	XmStringFree(label_str);
	XtManageChild(wd->prog_end);
	XtManageChild(meter_group);
	return(meter_group);
}


/* Function	: SDtAuCreateControls()
 * Objective	: Create the main sdtaudio buttons that control 
 *		  play and recording, volume, etc.
 * Arguments	: WindowData *		- the sdtaudio data structure
 * Return Value : Boolean True if the controls are created, False otherwise.
 */
Boolean
SDtAuCreateControls(WindowData * wd)
{
	Widget		main_group, vol_group, meter_group;
	Arg             args[20];
	int             n;
	Widget          separator;
	XmString        label_str;

	/* Create the control pane for the various sdtaudio buttons. */
	n = 0;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); ++n;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNtopOffset, WIDGET_OFFSET); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, WIDGET_OFFSET); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightOffset, WIDGET_OFFSET); ++n;
	wd->control_pane = XmCreateForm(wd->base_form, "ControlPane", args, n);
	if (wd->control_pane == NULL)
		return(False);

	main_group = SDtAuCreateMainButtonGroup(wd->control_pane, wd);
	if (main_group == NULL)
		return(False);

	vol_group = SDtAuCreateVolumeButtonsGroup(wd->control_pane, wd);
	if (vol_group == NULL)
		return(False);

	meter_group = SDtAuCreateMetersGroup(wd->control_pane, wd);
	if (meter_group == NULL)
		return(False);

	/* Now assign the volume group's location within the control
	 * pane form.
	 */
	n = 0;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, main_group); ++n;
	XtSetArg(args[n], XmNleftOffset, VOL_WIDGET_OFFSET); ++n;
	XtSetValues(vol_group, args, n);

	/* Now assign the meter group's location within the control
	 * pane form.
	 */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, main_group); ++n;
	XtSetArg(args[n], XmNtopOffset, PROGRESS_BAR_TOP_GAP); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, main_group); ++n;
	XtSetValues(meter_group, args, n);

	/* Manage the remaining unamanged forms, and return. */
	XtManageChild(main_group);
	XtManageChild(vol_group);
	XtManageChild(meter_group);
	XtManageChild(wd->control_pane);
	return(True);
}


/* Function	: SDtAuCreateWavePane()
 * Objective	: Create the panel that will contain the wave window
 *		  and its accompanying widgets.
 * Arguments	: WindowData *		- the sdtaudio data structure
 * Return Value : Boolean True if the controls are created, False otherwise.
 */
Boolean
SDtAuCreateWavePane(WindowData *wd)
{
	Arg                    args[20];
	int                    n;
	Widget                 wave_frame;
	XmString               label_str;
	XSetWindowAttributes   attrs;
	static SDtAudioHelpR   help_rec;

	/* Create the pane. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->control_pane); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->control_pane); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, wd->control_pane); ++n;

	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); ++n;
	wd->wave_pane = XmCreateForm(wd->base_form, "WavePane", args, n);
	if (wd->wave_pane == NULL)
		return(False);
	help_rec.help_parent_win = wd->mainWindow;
	help_rec.help_event = HELP_AUDIO_WAVE_WIN_EVENT;
	help_rec.wd = wd;
	XtAddCallback(wd->wave_pane, XmNhelpCallback, HelpCb, 
		      (XtPointer) &help_rec);

	/* Create the frame around the wave window. */
	n = 0;
	XtSetArg(args[n], XmNshadowType, XmSHADOW_IN); ++n;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); ++n;
	wave_frame = XmCreateFrame(wd->wave_pane, "WaveFrame", args, n);
	if (wave_frame == NULL)
		return(False);
	XtManageChild(wave_frame);

	/* Create the wave window. */
	n = 0;
	XtSetArg(args[n], XmNheight, WAVE_WINDOW_HEIGHT); ++n;
	wd->wave_window = XmCreateDrawingArea(wave_frame, "WaveWindow",
					      args, n);
	if (wd->wave_window == NULL)
		return(False);
	XtManageChild(wd->wave_window);
	XtAddCallback(wd->wave_window, XmNexposeCallback,
		      WaveWindowRefreshCb, wd);
	XtAddEventHandler(wd->wave_window, StructureNotifyMask, False,
			  ConfigNotifyWWCb, wd);
	XtAddEventHandler(wd->wave_window, PointerMotionMask, False,
			  DrawLocationCb, wd);
	XtAddEventHandler(wd->wave_window, EnterWindowMask, False,
			  EnterWaveWindowCb, wd);
	XtAddEventHandler(wd->wave_window, LeaveWindowMask, False,
			  LeaveWaveWindowCb, wd);
	XtAddEventHandler(wd->wave_window, ButtonPressMask, False,
			  ButtonDownWWCb, wd);
	XtAddEventHandler(wd->wave_window, ButtonReleaseMask, False,
			  ButtonUpWWCb, wd);

	/* Create the label for the wave start time. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 51, "0:00"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wave_frame); ++n;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->wave_start = XmCreateLabel(wd->wave_pane, "WaveStartLabel", 
				       args, n);
	if (wd->wave_start == NULL)
		return(False);
	XmStringFree(label_str);
	XtManageChild(wd->wave_start);

	/* Create the label for the wave's end time. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 52, "-:--"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wave_frame); ++n;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->wave_end = XmCreateLabel(wd->wave_pane, "WaveEndLabel", args, n);
	if(wd->wave_end == NULL)
		return(False);
	XmStringFree(label_str);
	XtManageChild(wd->wave_end);
	return(True);
}


/* Function	: SDtAuInitWWFont
 * Objective	: Initialize the fonts used by the wave window.
 * Arguments	: Widget		- a label widget from the sdtaudio
 *					  GUI; use the font used by this
 *					  label widget as the font for
 *					  the wave window.
 * Return Value : None.
 */
void 
SDtAuInitWWFont(Widget wid)
{
	XmFontList        font_list;
	XmFontContext     context;
	XmStringCharSet   charset;

	/* Get the font list for any label, e.g., footer_msg,
	 * because it already is in use and has been created.
	 */
	XtVaGetValues(wid, XmNfontList, &font_list, NULL);
	if (font_list) {

		XmFontListInitFontContext(&context, font_list);
		if (XmFontListGetNextFont(context, &charset, &ww_font) == True) 
			ww_font_height = ww_font->ascent + ww_font->descent;
		XmFontListFreeFontContext(context);
		XmFontListFree(font_list);
	}
}


/* Function	: SDtAuCreateStartupOptions()
 * Objective	: Create the widgets that make up the start-up
 *		  pane of the options dialog.
 * Arguments	: WindowData *		- the sdtaudio structure
 *		  Widget		- the top widget where the start-up
 *					  pane will attach itself
 * Return Value : Boolean True if creation was successful, False otherwise.
 */
Boolean
SDtAuCreateStartupOptions(WindowData *wd, Widget top_attach)
{
	Widget        start_label, start_radio_box;
	Arg           args[20];
	register int  n;
	XmString      label_str;
	Dimension     border_width = 1;

	/* Create the start-up pane. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, top_attach); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, border_width); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightOffset, border_width); ++n;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); ++n;
	wd->opt_startup_pane = XtCreateWidget("StartupPane", xmFormWidgetClass,
					      wd->options_form, args, n);
	if (wd->opt_startup_pane == NULL)
		return(False);

	/* Create the audio start-up label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 1,
						    "Start up Audio with:"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, 15); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	start_label =  XtCreateWidget("StartupAudioLabel", xmLabelWidgetClass,
				      wd->opt_startup_pane, args, n);
	XmStringFree(label_str);
	if (start_label == NULL)
		return(False);
	XtManageChild(start_label);

	/* Create the Startup radio box options. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, start_label); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, start_label); ++n;
	XtSetArg(args[n], XmNleftOffset, 25); ++n;
	XtSetArg(args[n], XmNorientation, XmVERTICAL); ++n;
	start_radio_box = XmCreateRadioBox(wd->opt_startup_pane, 
					   "StartupRadioBox", args, n);
	if (start_radio_box == NULL)
		return(False);
	XtManageChild(start_radio_box);

	/* Now create the Standard View radio button item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 23,
						    "Standard View"));
	n = 0;
	XtSetArg(args[n], XmNset, !(wd->options->showWave)); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->standard_view_rb = 
		XtCreateWidget("StandardViewRBItem", xmToggleButtonWidgetClass, 
			       start_radio_box, args, n);
	XmStringFree(label_str);
	if (wd->standard_view_rb == NULL)
		return(False);
	XtManageChild(wd->standard_view_rb);
	XtAddCallback(wd->standard_view_rb, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));

	/* Now create the Wave View radio button item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 24,
					            "Waveform View"));
	n = 0;
	XtSetArg(args[n], XmNset, wd->options->showWave); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->wave_view_rb = XtCreateWidget("WaveViewRBItem",
					  xmToggleButtonWidgetClass,
					  start_radio_box, args, n);
	XmStringFree(label_str);
	if (wd->wave_view_rb == NULL)
		return(False);
	XtManageChild(wd->wave_view_rb);
	XtManageChild(start_radio_box);
	XtAddCallback(wd->wave_view_rb, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	XtManageChild(wd->opt_startup_pane);
	return(True);
}


/* Function	: SDtAuCreatePlayOptions()
 * Objective	: Create the pane that will contain the widgets for
 *		  basic playing options.
 * Arguments	: WindowData *	- the sdtaudio structure
 * 		  Widget	- the top widget that this pane will
 *				  attach to
 * Return Value : Boolean True if pane was successfully created, false
 *		  otherwise.
 */
Boolean
SDtAuCreatePlayOptions(WindowData *wd, Widget top_attach)
{
	Widget        default_checkbox;
	Widget	      sep, vol_label, bal_label, left_label, right_label;
	Widget        left_tick, right_tick, mid_tick;
	XmString      label_str;
	Arg           args[20];
	register int  n;
	Dimension     border_width = 1;

	/* Create the start-up pane. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, top_attach); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, border_width); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightOffset, border_width); ++n;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); ++n;
	wd->opt_play_pane = XtCreateWidget("PlayPane", xmFormWidgetClass,
					   wd->options_form, args, n);
	if (wd->opt_play_pane == NULL)
		return(False);

	/* Create the check box area. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->wave_view_rb); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, 15); ++n;
	XtSetArg(args[n], XmNradioBehavior, False); ++n;
	XtSetArg(args[n], XmNorientation, XmVERTICAL); ++n;
	default_checkbox = XmCreateRadioBox(wd->opt_play_pane, 
					    "DefaultCheckbox", args, n);
	if (default_checkbox == NULL)
		return(False);

	/* Create the play on open check box item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 25,
				    "Automatically play audio file on Open."));
	n = 0;
	XtSetArg(args[n], XmNset, wd->options->autoPlayOnOpen); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->auto_play_open = XtCreateWidget("AutoPlayOpen", 
				            xmToggleButtonWidgetClass, 
				            default_checkbox, args, n);
	XmStringFree(label_str);
	if (wd->auto_play_open == NULL)
		return(False);
	XtManageChild(wd->auto_play_open);
	XtAddCallback(wd->auto_play_open, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));

#ifdef SELECTION
	/* Create the auto play selection check box item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 26,
				"Automatically play selection."));
	n = 0;
	XtSetArg(args[n], XmNset, wd->options->autoPlayOnSelect); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->auto_play_select = XtCreateWidget("AutoPlaySelection", 
					      xmToggleButtonWidgetClass, 
					      default_checkbox, args, n);
	XmStringFree(label_str);
	if (wd->auto_play_select == NULL)
		return(False);
	XtManageChild(wd->auto_play_select);
	XtAddCallback(wd->auto_play_select, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
#endif
	XtManageChild(default_checkbox);

	/* Create separator. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, default_checkbox); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNheight, 5); ++n;
	sep = XtCreateWidget("default_separator", xmSeparatorWidgetClass,
			     wd->opt_play_pane, args, n);
	if (sep == NULL)
		return(False);
	XtManageChild(sep);

	/* Create the play volume label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 27,
					         "Set Default Play Volume:"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, sep); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, default_checkbox); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	vol_label =  XtCreateWidget("PlayVolumeLabel", xmLabelWidgetClass,
				    wd->opt_play_pane, args, n);
	if (vol_label == NULL)
		return(False);
	XmStringFree(label_str);
	XtManageChild(vol_label);

	/* Create the play volume scale. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, vol_label); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, 50); ++n;
	XtSetArg(args[n], XmNshowValue, True); ++n;
	XtSetArg(args[n], XmNvalue, wd->options->playVolume); ++n;
	XtSetArg(args[n], XmNscaleMultiple, 1); ++n;
	XtSetArg(args[n], XmNscaleWidth, OPTIONS_SCALE_WIDTH); ++n;
	XtSetArg(args[n], XmNminimum, 0); ++n;
	XtSetArg(args[n], XmNmaximum, 100); ++n;
	XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); ++n;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
	XtSetArg(args[n], "slidingMode", False); ++n;
	wd->play_vol_scale = XtCreateWidget("PlayVolumeScale",
					    xmScaleWidgetClass,
					    wd->opt_play_pane, args, n);
	if (wd->play_vol_scale == NULL)
		return(False);
	XtManageChild(wd->play_vol_scale);
	XtAddCallback(wd->play_vol_scale, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));

	/* Create the play balance label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 28,
						  "Balance Audio Output:"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->play_vol_scale); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, default_checkbox); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	bal_label = XtCreateWidget("PlayBalanceLabel", xmLabelWidgetClass, 	
                                   wd->opt_play_pane, args, n);
	if (bal_label == NULL)
		return(False);
	XmStringFree(label_str);
	XtManageChild(bal_label);

	/* Create the play balance scale */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, bal_label); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->play_vol_scale); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, wd->play_vol_scale); ++n;
	XtSetArg(args[n], XmNshowValue, False); ++n;
	XtSetArg(args[n], XmNvalue, wd->options->playBalance); ++n;
	XtSetArg(args[n], XmNscaleMultiple, 1); ++n;
	XtSetArg(args[n], XmNminimum, -50); ++n;
	XtSetArg(args[n], XmNmaximum, 50); ++n;
	XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); ++n;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
	XtSetArg(args[n], "slidingMode", False); ++n;
	wd->play_bal_scale = XtCreateWidget("PlayBalanceScale",
					    xmScaleWidgetClass,
					    wd->opt_play_pane, args, n);
	if (wd->play_bal_scale == NULL)
		return(False);
	XtManageChild(wd->play_bal_scale);
	XtAddCallback(wd->play_bal_scale, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));

	/* Create the Left label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 29, "Left"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->play_bal_scale); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, default_checkbox); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, wd->play_bal_scale); ++n;
	XtSetArg(args[n], XmNrightOffset, 3); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	left_label = XtCreateWidget("LeftPlayBalLabel", xmLabelWidgetClass, 
				    wd->opt_play_pane, args, n);
	XmStringFree(label_str);
	if (left_label == NULL)
		return(False);
	XtManageChild(left_label);

	/* Create the Right label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 30, 
						    "Right"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->play_bal_scale); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->play_bal_scale); ++n;
	XtSetArg(args[n], XmNleftOffset, 3); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	right_label = XtCreateWidget("RightPlayBalLabel",
				     xmLabelWidgetClass, 
				     wd->opt_play_pane, args, n);
	XmStringFree(label_str);
	if (right_label == NULL)
		return(False);
	XtManageChild(right_label);

	/* Create the left tick mark for the balance. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->play_bal_scale); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->play_bal_scale); ++n;
	XtSetArg(args[n], XmNorientation, XmVERTICAL); ++n;
	XtSetArg(args[n], XmNheight, 8); ++n;
	XtSetArg(args[n], XmNwidth, 5); ++n;
	left_tick = XtCreateWidget("LeftTick", xmSeparatorWidgetClass,
			           wd->opt_play_pane, args, n);
	if (left_tick == NULL)
		return(False);
	XtManageChild(left_tick);

	/* Create the right tick mark for the balance. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->play_bal_scale); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, wd->play_bal_scale); ++n;
	XtSetArg(args[n], XmNorientation, XmVERTICAL); ++n;
	XtSetArg(args[n], XmNheight, 8); ++n;
	XtSetArg(args[n], XmNwidth, 5); ++n;
	right_tick = XtCreateWidget("RightTick", xmSeparatorWidgetClass, 
			            wd->opt_play_pane, args, n);
	if (right_tick == NULL)
		return(False);
	XtManageChild(right_tick);

	/* Create the middle tick mark for the balance. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->play_bal_scale); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, left_tick); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, right_tick); ++n;
	XtSetArg(args[n], XmNorientation, XmVERTICAL); ++n;
	XtSetArg(args[n], XmNheight, 8); ++n;
	XtSetArg(args[n], XmNwidth, 5); ++n;
	mid_tick = XtCreateWidget("MiddleTick", xmSeparatorWidgetClass,
			          wd->opt_play_pane, args, n);
	if (mid_tick == NULL)
		return(False);
	XtManageChild(mid_tick);
	XtManageChild(wd->opt_play_pane);
	return(True);
}


/* Function	: SDtAuCreateRecordOptions()
 * Objective	: Create the pane that will contain the widgets for
 *		  recording options.
 * Arguments	: WindowData *	- the sdtaudio structure
 * 		  Widget	- the top widget that this pane will
 *				  attach to
 * Return Value : Boolean True if pane was successfully created, false
 *		  otherwise.
 */
Boolean
SDtAuCreateRecordOptions(WindowData *wd, Widget top_attach)
{
	Widget          recoptions_label, recqual_pulldown;
	Widget          audformat_pulldown, dfltrec_radiobox;
	Widget          vol_label, bal_label, left_label, right_label;
	Widget          left_tick, right_tick, mid_tick, sep;
	Widget		wids[8];
	XmString        label_str;
	Arg             args[20];
	register int    n;
	Dimension       border_width = 1;

	/* Now create the recording pane. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, top_attach); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, border_width); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightOffset, border_width); ++n;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); ++n;
	wd->opt_record_pane = XtCreateWidget("RecordingOptionsPane",
					     xmFormWidgetClass,
					     wd->options_form, args, n);
	if (wd->opt_record_pane == NULL)
		return(False);

	/* Create the recording options label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 67, 
						    "Recording Options:"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNtopOffset, 15); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, 15); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	recoptions_label = XmCreateLabel(wd->opt_record_pane, 
					 "RecordingOptionsLabel",
					  args, n);
	XmStringFree(label_str);
	if (recoptions_label == NULL)
		return(False);
	XtManageChild(recoptions_label);

	/* Create the recording quality pulldown menu. */
	recqual_pulldown = XmCreatePulldownMenu(wd->opt_record_pane, 
					        "RecQualityPulldown", 
					        NULL, 0);
	if (recqual_pulldown == NULL)
		return(False);

	/* Create the voice menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 32, 
						    "Voice"));
	wd->voice_menu_item = XtVaCreateWidget("VoiceMenuItem",
					       xmPushButtonGadgetClass,
					       recqual_pulldown,
					       XmNlabelString, label_str,
					       NULL);
	XmStringFree(label_str);
	XtAddCallback(wd->voice_menu_item, XmNactivateCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	if (wd->voice_menu_item == NULL)
		return(False);
	XtManageChild(wd->voice_menu_item);


	/* Create the CD menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 33, "CD"));
	wd->cd_menu_item = XtVaCreateWidget("CDMenuItem",
					    xmPushButtonGadgetClass,
					    recqual_pulldown,
					    XmNlabelString, label_str,
					    NULL);
	XmStringFree(label_str);
	XtAddCallback(wd->cd_menu_item, XmNactivateCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	if (wd->cd_menu_item == NULL)
		return(False);
	XtManageChild(wd->cd_menu_item);

	/* Create the DAT menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 68, "DAT"));
	wd->dat_menu_item = XtVaCreateWidget("DATMenuItem",
					     xmPushButtonGadgetClass,
					     recqual_pulldown,
					     XmNlabelString, label_str,
					     NULL);
	XmStringFree(label_str);
	XtAddCallback(wd->dat_menu_item, XmNactivateCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	if (wd->dat_menu_item == NULL)
		return(False);
	XtManageChild(wd->dat_menu_item);

	/* Create the recording quality options menu. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 31,
						    "Audio Quality:"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, recoptions_label); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, recoptions_label); ++n;
	XtSetArg(args[n], XmNleftOffset, 25); ++n;
	XtSetArg(args[n], XmNsubMenuId, recqual_pulldown); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->recqual_option = XmCreateOptionMenu(wd->opt_record_pane,
					        "RecQualityOptionMenu",
					        args, n);
	XmStringFree(label_str);
	if (wd->recqual_option == NULL)
		return(False);
	XtManageChild(wd->recqual_option);

	/* Set the recording quality menu item to be selected. */
	n = 0;
	switch (wd->options->recordingQuality) {

		case SDTA_CD_QUALITY:
			XtSetArg(args[n], XmNmenuHistory, wd->cd_menu_item); 
			++n;
			break;

		case SDTA_DAT_QUALITY:
			XtSetArg(args[n], XmNmenuHistory, wd->dat_menu_item); 
			++n;
			break;

		default:
			XtSetArg(args[n], XmNmenuHistory, wd->voice_menu_item); 
			++n;
			break;
	}
	XtSetValues(wd->recqual_option, args, n);

	/* Create the audio format pulldown menu. */
	audformat_pulldown = XmCreatePulldownMenu(wd->opt_record_pane, 
					          "AudioFormatPulldown",
						  NULL, 0);
	if (audformat_pulldown == NULL)
		return(False);

	/* Create the AIFF menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 66, "AIFF"));
	wd->aiff_menu_item = XtVaCreateWidget("AIFFMenuItem",
					      xmPushButtonGadgetClass,
					      audformat_pulldown,
					      XmNlabelString, label_str,
					      NULL);
	XmStringFree(label_str);
	if (wd->aiff_menu_item == NULL)
		return(False);
	XtAddCallback(wd->aiff_menu_item, XmNactivateCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	XtManageChild(wd->aiff_menu_item);

	/* Create the AU menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 64, "AU"));
	wd->au_menu_item = XtVaCreateWidget("AUMenuItem",
					    xmPushButtonGadgetClass,
					    audformat_pulldown,
					    XmNlabelString, label_str,
					    NULL);
	XmStringFree(label_str);
	if (wd->au_menu_item == NULL)
		return(False);
	XtAddCallback(wd->au_menu_item, XmNactivateCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	XtManageChild(wd->au_menu_item);

	/* Create the WAV menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 65, "WAV"));
	wd->wav_menu_item = XtVaCreateWidget("WAVMenuItem",
					     xmPushButtonGadgetClass,
					     audformat_pulldown,
					     XmNlabelString, label_str,
					     NULL);
	XmStringFree(label_str);
	if (wd->wav_menu_item == NULL)
		return(False);
	XtAddCallback(wd->wav_menu_item, XmNactivateCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	XtManageChild(wd->wav_menu_item);

	/* Create the audio format options menu. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 63,
						    "Audio Format:"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->recqual_option); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->recqual_option); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, wd->recqual_option); ++n;
	XtSetArg(args[n], XmNsubMenuId, audformat_pulldown); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->audformat_option = XmCreateOptionMenu(wd->opt_record_pane,
					          "AudioFormatOptionMenu",
					          args, n);
	XmStringFree(label_str);
	if (wd->audformat_option == NULL)
		return(False);
	XtManageChild(wd->audformat_option);
	XtAddCallback(wd->audformat_option, XmNentryCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));

	/* Set the format menu item to be selected. */
	n = 0;
	switch (wd->options->recordingFormat) {

		case SAWavType:
			XtSetArg(args[n], XmNmenuHistory, wd->wav_menu_item); 
			++n;
			break;

		case SAAiffType:
			XtSetArg(args[n], XmNmenuHistory, wd->aiff_menu_item); 
			++n;
			break;

		default:
			XtSetArg(args[n], XmNmenuHistory, wd->au_menu_item); 
			++n;
			break;
	}
	XtSetValues(wd->audformat_option, args, n);

	/* Invoke callback when the audio format options menu
	 * is changed.
	 */
	XtAddCallback(wd->audformat_option, XmNentryCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));

	/* Make the options menu the same width. */
	n = 0;
	wids[n] = wd->cd_menu_item; ++n;
	wids[n] = wd->dat_menu_item; ++n;
	wids[n] = wd->voice_menu_item; ++n;
	wids[n] = wd->wav_menu_item; ++n;
	wids[n] = wd->aiff_menu_item; ++n;
	wids[n] = wd->au_menu_item; ++n;
	wids[n] = wd->recqual_option; ++n;
	wids[n] = wd->audformat_option; ++n;
	SDtAuSetSameDimension(wids, n, XmNwidth);

	/* Now create the radio boxes for default recording behavior. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->audformat_option);++n;
	XtSetArg(args[n], XmNtopOffset, 15);++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, recoptions_label); ++n;

	XtSetArg(args[n], XmNorientation, XmVERTICAL); ++n;
	dfltrec_radiobox = XmCreateRadioBox(wd->opt_record_pane,
					    "dfltrec_radiobox", args, n);
	if (dfltrec_radiobox == NULL)
		return(False);

	/* Now create the "Use selected as default" radio item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 34, 
		  "Use selected recording options as\ndefault for new files"));
	n = 0;
	XtSetArg(args[n], XmNset, wd->options->useDefaultRecValues); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->selected_rb =  XtCreateWidget("selected_rb", 
					  xmToggleButtonWidgetClass,
					  dfltrec_radiobox, args, n);
	XmStringFree(label_str);
	if (wd->selected_rb == NULL)
		return(False);
	XtManageChild(wd->selected_rb);
	XtAddCallback(wd->selected_rb, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));

	/* Create the "Ask me when I start recording" radio item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 35,
					      "Ask me when I start recording"));
	n = 0;
	XtSetArg(args[n], XmNset, !(wd->options->useDefaultRecValues)); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->ask_rb = XtCreateWidget("selected_rb",
				    xmToggleButtonWidgetClass,
				    dfltrec_radiobox, args, n);
	XmStringFree(label_str);
	if (wd->ask_rb == NULL)
		return(False);
	XtManageChild(wd->ask_rb);
	XtAddCallback(wd->ask_rb, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	XtManageChild(dfltrec_radiobox);

	/* Create a separator. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, dfltrec_radiobox); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, border_width); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightOffset, border_width); ++n;
	XtSetArg(args[n], XmNheight, 5); ++n;
	sep = XmCreateSeparator(wd->opt_record_pane, "Separator", args, n);
	if (sep == NULL)
		return(False);
	XtManageChild(sep);

	/* Create the record volume label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 54,
					        "Set Default Record Volume:"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, sep); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, recoptions_label); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	vol_label =  XtCreateWidget("RecVolLabel", xmLabelWidgetClass,
				    wd->opt_record_pane, args, n);
	XmStringFree(label_str);
	if (vol_label == NULL)
		return(False);
	XtManageChild(vol_label);

	/* Create the record volume scale. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, vol_label); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, 50); ++n;
	XtSetArg(args[n], XmNshowValue, True); ++n;
	XtSetArg(args[n], XmNvalue, wd->options->recordVolume); ++n;
	XtSetArg(args[n], XmNscaleMultiple, 1); ++n;
	XtSetArg(args[n], XmNmaximum, 100); ++n;
	XtSetArg(args[n], XmNminimum, 0); ++n;
	XtSetArg(args[n], XmNscaleWidth, OPTIONS_SCALE_WIDTH); ++n;
	XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); ++n;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
	XtSetArg(args[n], "slidingMode", False); ++n;
	wd->record_vol_scale = XtCreateWidget("RecordingVolumeScale", 
					      xmScaleWidgetClass,
					      wd->opt_record_pane,
					      args, n);
	if (wd->record_vol_scale == NULL)
		return(False);
	XtManageChild(wd->record_vol_scale);
	XtAddCallback(wd->record_vol_scale, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));

	/* Create the recording balance label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 55,
						    "Balance Audio Input:"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->record_vol_scale); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, recoptions_label); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	bal_label = XtCreateWidget("RecordingBalanceLabel",
				   xmLabelWidgetClass, 	
                                   wd->opt_record_pane, args, n);
	XmStringFree(label_str);
	if (bal_label == NULL)
		return(False);
	XtManageChild(bal_label);

	/* Create the recording balance scale */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, bal_label); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->record_vol_scale); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, wd->record_vol_scale); ++n;
	XtSetArg(args[n], XmNshowValue, False); ++n;
	XtSetArg(args[n], XmNvalue, wd->options->recordBalance); ++n;
	XtSetArg(args[n], XmNscaleMultiple, 1); ++n;
	XtSetArg(args[n], XmNmaximum, 50); ++n;
	XtSetArg(args[n], XmNminimum, -50); ++n;
	XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); ++n;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
	XtSetArg(args[n], "slidingMode", False); ++n;
	wd->record_bal_scale = XtCreateWidget("RecordingBalanceScale", 
					      xmScaleWidgetClass,
					      wd->opt_record_pane,
					      args, n);
	if (wd->record_bal_scale == NULL)
		return(False);
	XtManageChild(wd->record_bal_scale);
	XtAddCallback(wd->record_bal_scale, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));

	/* Create the Left label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 29, "Left"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->record_bal_scale); ++n;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNbottomOffset, 15); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, recoptions_label); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, wd->record_bal_scale); ++n;
	XtSetArg(args[n], XmNrightOffset, 3); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	left_label = XtCreateWidget("LeftRecBalLabel", xmLabelWidgetClass, 
				    wd->opt_record_pane, args, n);
	XmStringFree(label_str);
	if (left_label == NULL)
		return(False);
	XtManageChild(left_label);

	/* Create the Right label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 30, 
						    "Right"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->record_bal_scale); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->record_bal_scale); ++n;
	XtSetArg(args[n], XmNleftOffset, 3); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	right_label = XtCreateWidget("RightRecBalLabel",
				     xmLabelWidgetClass, 
				     wd->opt_record_pane, args, n);
	XmStringFree(label_str);
	if (right_label == NULL)
		return(False);
	XtManageChild(right_label);

	/* Create the left tick mark for the balance. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->record_bal_scale); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, wd->record_bal_scale); ++n;
	XtSetArg(args[n], XmNorientation, XmVERTICAL); ++n;
	XtSetArg(args[n], XmNheight, 8); ++n;
	XtSetArg(args[n], XmNwidth, 5); ++n;
	left_tick = XtCreateWidget("left_tick", xmSeparatorWidgetClass,
			           wd->opt_record_pane, args, n);
	if (left_tick == NULL)
		return(False);
	XtManageChild(left_tick);

	/* Create the right tick mark for the balance. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->record_bal_scale); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, wd->record_bal_scale); ++n;
	XtSetArg(args[n], XmNorientation, XmVERTICAL); ++n;
	XtSetArg(args[n], XmNheight, 8); ++n;
	XtSetArg(args[n], XmNwidth, 5); ++n;
	right_tick = XtCreateWidget("right_tick", xmSeparatorWidgetClass, 
			            wd->opt_record_pane, args, n);
	if (right_tick == NULL)
		return(False);
	XtManageChild(right_tick);

	/* Create the middle tick mark for the balance. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->record_bal_scale); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, left_tick); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, right_tick); ++n;
	XtSetArg(args[n], XmNorientation, XmVERTICAL); ++n;
	XtSetArg(args[n], XmNheight, 8); ++n;
	XtSetArg(args[n], XmNwidth, 5); ++n;
	mid_tick = XtCreateWidget("mid_tick", xmSeparatorWidgetClass,
			          wd->opt_record_pane, args, n);
	if (mid_tick == NULL)
		return(False);
	XtManageChild(mid_tick);
	XtManageChild(wd->opt_record_pane);
	return(True);
}


/* Function	: SDtAuCreateDevicesOptions()
 * Objective	: Create the widgets that are required for the devices
 *		  pane of the options window; the pane allows the user
 *		  to select the audio input and output devices to use.
 * Arguments	: WindowData *		- sdtaudio data structure
 *		  Widget		- the top widget that the
 *					  devices pane will attach to
 * Return Value : Boolean True if the dialog is successfully created, False
 *		  otherwise.
 */
Boolean
SDtAuCreateDevicesOptions(WindowData *wd, Widget top_attach)
{
	XmString        label_str;
	Widget		output_frame, input_frame;
	Widget		output_label, input_label;
	Widget		output_chkbox, input_radio;
	Arg             args[20];
        register int    n;
	Dimension       border_width = 1;

	/* Create the devices pane. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, top_attach); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, border_width); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightOffset, border_width); ++n;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY); ++n;
	wd->opt_devices_pane = XtCreateWidget("DevicesOptionsPane",
					      xmFormWidgetClass,
					      wd->options_form, args, n);
	if (wd->opt_devices_pane == NULL)
		return(False);

	/* Create the Audio Output Devices frame. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, 15); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightOffset, 15); ++n;
	XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_IN); ++n;
	output_frame = XmCreateFrame(wd->opt_devices_pane, "AudioOutputFrame",
				     args, n);
	if (output_frame == NULL)
		return(False);
	XtManageChild(output_frame);

	/* Create the Audio Output Devices label. */
	n = 0;
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 75,
					            "Output Source (Play)"));
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); ++n;
	XtSetArg(args[n], XmNchildHorizontalAlignment, XmALIGNMENT_BEGINNING);
	++n;
	XtSetArg(args[n], XmNchildVerticalAlignment, XmALIGNMENT_BASELINE_TOP); 
	++n;
	output_label = XmCreateLabel(output_frame, "AudioOutputLabel", 
				     args, n);
	XmStringFree(label_str);
	if (output_label == NULL)
		return(False);
	XtManageChild(output_label);

	/* Create the Audio output check box. */
	n = 0;
	XtSetArg(args[n], XmNchildType, XmFRAME_WORKAREA_CHILD); ++n;
	output_chkbox = XmCreateSimpleCheckBox(output_frame,
					       "AudioOutputCheckBox",
					       args, n);
	if (output_chkbox == NULL)
		return(False);
	XtManageChild(output_chkbox);

	/* Create the button for the speaker. */
	n = 0;
	label_str =  XmStringCreateLocalized(catgets(msgCatalog, 2, 77,
					             "Speaker"));
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->devices_speaker = 
		XmCreateToggleButtonGadget(output_chkbox, "SpeakerButton",
					   args, n);
	XmStringFree(label_str);
	if (wd->devices_speaker == NULL)
		return(False);
	XtManageChild(wd->devices_speaker);

	/* Create the button for the headphone. */
	label_str =  XmStringCreateLocalized(catgets(msgCatalog, 2, 78,
					             "Headphone"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->devices_headphone = 
		XmCreateToggleButtonGadget(output_chkbox, "headphoneButton",
					   args, n);
	XmStringFree(label_str);
	if (wd->devices_headphone == NULL)
		return(False);
	XtManageChild(wd->devices_headphone);

	/* Create the button for line out. */
	label_str =  XmStringCreateLocalized(catgets(msgCatalog, 2, 79,
					             "Line Out"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->devices_line_out = 
		XmCreateToggleButtonGadget(output_chkbox, "LineOutButton",
					   args, n);
	XmStringFree(label_str);
	if (wd->devices_line_out == NULL)
		return(False);
	XtManageChild(wd->devices_line_out);

	/* Create the Audio Input Devices frame. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, output_frame); ++n;
	XtSetArg(args[n], XmNtopOffset, 15); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNleftWidget, output_frame); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); ++n;
	XtSetArg(args[n], XmNrightWidget, output_frame); ++n;
	XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_IN); ++n;
	input_frame = XmCreateFrame(wd->opt_devices_pane, "AudioInputFrame", 
				    args, n);
	if (input_frame == NULL)
		return(False);
	XtManageChild(input_frame);

	/* Create the Audio Input Devices label. */
	n = 0;
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 76,
					            "Input Source (Record)"));
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); ++n;
	XtSetArg(args[n], XmNchildHorizontalAlignment, XmALIGNMENT_BEGINNING);
	++n;
	XtSetArg(args[n], XmNchildVerticalAlignment, XmALIGNMENT_BASELINE_TOP); 
	++n;
	input_label = XmCreateLabel(input_frame, "AudioInputLabel", 
				     args, n);
	XmStringFree(label_str);
	if (input_label == NULL)
		return(False);
	XtManageChild(input_label);

	/* Create the Audio Input Radio Box. */
	n = 0;
	XtSetArg(args[n], XmNchildType, XmFRAME_WORKAREA_CHILD); ++n;
	input_radio = XmCreateSimpleRadioBox(input_frame,
					     "AudioInputRadioBox",
					     args, n);
	if (input_radio == NULL)
		return(False);
	XtManageChild(input_radio);

	/* Create the button for the microphone. */
	n = 0;
	label_str =  XmStringCreateLocalized(catgets(msgCatalog, 2, 80,
					             "Microphone"));
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->devices_mic = 
		XmCreateToggleButtonGadget(input_radio, "MicButton", args, n);
	XmStringFree(label_str);
	if (wd->devices_mic == NULL)
		return(False);
	XtManageChild(wd->devices_mic);

	/* Create the button for the line in input. */
	label_str =  XmStringCreateLocalized(catgets(msgCatalog, 2, 81,
					             "Line In"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->devices_line_in = 
		XmCreateToggleButtonGadget(input_radio, "LineInButton",
					   args, n);
	XmStringFree(label_str);
	if (wd->devices_line_in == NULL)
		return(False);
	XtManageChild(wd->devices_line_in);

	/* Create the button for CD. */
	label_str =  XmStringCreateLocalized(catgets(msgCatalog, 2, 33, "CD"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->devices_cd = 
		XmCreateToggleButtonGadget(input_radio, "CDButton", args, n);
	XmStringFree(label_str);
	if (wd->devices_cd == NULL)
		return(False);
	XtManageChild(wd->devices_cd);
		XtVaSetValues(wd->devices_speaker,
			      XmNset, True,
			      NULL);

	/* Update the view with devices present on the system. */
	SDtAuSetDeviceState(wd);
	SDtAuUpdateDevices(wd);

	/* Now add callbacks when user changes values for devices. */
	XtAddCallback(wd->devices_speaker, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	XtAddCallback(wd->devices_headphone, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	XtAddCallback(wd->devices_line_out, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	XtAddCallback(wd->devices_mic, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	XtAddCallback(wd->devices_line_in, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	XtAddCallback(wd->devices_cd, XmNvalueChangedCallback,
		      OptionsChangedCb, (XtPointer) &(wd->options_changed));
	XtManageChild(wd->opt_devices_pane);
	return(True);
}

/* Function	: SDtAuCreateCmdButtonsPane()
 * Objective	: Create the pane in the options dialog that will contain
 *		  the OK, Apply, Cancel, Reset, and Help buttons.
 * Arguments	: WindowData *		- the sdtaudio structure
 * Return Value : Boolean True if the pane and all its widgets were created,
 *		  False otherwise.
 */
Boolean
SDtAuCreateCmdButtonsPane(WindowData *wd)
{
	Arg           args[20];
        register int  n;
	Widget        sep, command_rowcol;
	Widget	      apply_but, reset_but, help_but;
	XmString      label_str;
	Dimension     border_width = 1;
	static SDtAudioHelpR help_rec;

	/* Create the form for the command buttons */
	n = 0;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNbottomOffset, border_width); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, border_width); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightOffset, border_width); ++n;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); ++n;
	wd->options_cmd_pane = XmCreateForm(wd->options_form,
					    "OptionsCommandPane", args, n);
	if (wd->options_cmd_pane == NULL)
		return(False);

	/* Create the separator. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
	XtSetArg(args[n], XmNheight, 5); ++n;
	sep = XtCreateWidget("button_separator", xmSeparatorWidgetClass,
			      wd->options_cmd_pane, args, n);
	if (sep == NULL)
		return(False);
	XtManageChild(sep);

	/* Create a row column widget that will hold all the buttons. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, sep); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNbottomOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, 15); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightOffset, 15); ++n;
	XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); ++n;
	XtSetArg(args[n], XmNnumColumns, 1); ++n;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
	XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); ++n;
	command_rowcol = XtCreateWidget("command_rowcol", 
					xmRowColumnWidgetClass,
					wd->options_cmd_pane, args, n);
	if (command_rowcol == NULL)
		return(False);

	/* Create the OK button. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 37, "OK"));
	n = 0;
	XtSetArg(args[n], XmNrecomputeSize, True); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	wd->options_ok = XtCreateWidget("ok_button", xmPushButtonWidgetClass,
				        command_rowcol, args, n);
	XmStringFree(label_str);
	if (wd->options_ok == NULL)
		return(False);
	XtAddCallback(wd->options_ok, XmNactivateCallback, OptionsSetCb, 
		      wd);
	XtManageChild(wd->options_ok);

	/* Create the apply button. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 38, 
						    "Apply"));
	n = 0;
	XtSetArg(args[n], XmNrecomputeSize, True); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	apply_but = XtCreateWidget("apply_button", xmPushButtonWidgetClass,
				   command_rowcol, args, n);
	if (apply_but == NULL)
		return(False);
	XmStringFree(label_str);
	XtManageChild(apply_but);
	XtAddCallback(apply_but, XmNactivateCallback, OptionsSetCb, 
		      wd);

	/* Create the Reset button. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 39, 
						    "Reset"));
	n = 0;
	XtSetArg(args[n], XmNrecomputeSize, True); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	reset_but = XtCreateWidget("reset_button", xmPushButtonWidgetClass,
				   command_rowcol, args, n);
	if (reset_but == NULL)
		return(False);
	XtAddCallback(reset_but, XmNactivateCallback, OptionsResetCb,
		      wd);
	XmStringFree(label_str);
	XtManageChild(reset_but);

	/* Create the Cancel button. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 40, 
						    "Cancel"));
	n = 0;
	XtSetArg(args[n], XmNrecomputeSize, True); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); ++n;
	wd->options_cancel = XtCreateWidget("cancel_button", 
					    xmPushButtonWidgetClass,
					    command_rowcol, args, n);
	if (wd->options_cancel == NULL)
		return(False);
	XtAddCallback(wd->options_cancel, XmNactivateCallback, 
		      OptionsCancelCb, wd);
	XmStringFree(label_str);
	XtManageChild(wd->options_cancel);

	/* Create the Help button. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 7, 
						    "Help"));
	n = 0;
	XtSetArg(args[n], XmNrecomputeSize, True); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	help_but = XtCreateWidget("help_button", xmPushButtonWidgetClass,
				   command_rowcol, args, n);
	if (help_but == NULL)
		return(False);
	help_rec.help_parent_win = wd->options_dlog;
	help_rec.help_event = HELP_AUDIO_OPTIONS_EVENT;
	help_rec.wd = wd;
	XtAddCallback(help_but, XmNactivateCallback, HelpCb,
		      (XtPointer) &help_rec);
	XmStringFree(label_str);
	XtManageChild(help_but);
	XtManageChild(command_rowcol);

	/* Manage the command buttons. */
	XtManageChild(wd->options_cmd_pane);
	return(True);
}

/*----------------- Functions availble outside this file ---------------------*/

/* Create a new top-level window.  If loadtype is LOAD_EMPTY, name_or_buf and
 * len are ignored.  If loadtype is LOAD_FILE, name_or_buf should point to
 * the name of the file to load, and len is ignored.  If loadtype is
 * LOAD_BUFFER, name_or_buf is a pointer to the data buffer and len is its
 * length.
 */

Boolean
SDtAuNewWindow(LoadType loadtype, char *name_or_buf, int len)
{
	Pixmap          iconPixmap;
	Pixmap          iconMask;
	Arg             args[20];
	int             n;
	XmString        label_xmstring = NULL;
	static SDtAudioHelpR help_rec;

	/* Drag and Drop transfer function. */
	XtCallbackRec   transfer_rec[1];

	wd = NewData();
	if (wd == (WindowData *) NULL)
		return(False);

	/* Retrieve user's default values before we start creating our
	 * widgets.
	 */
	SDtAuInitOptions(wd);


	/* Create top level shell */
	n = 0;
	XtSetArg(args[n], XtNallowShellResize, True); ++n;
	XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); ++n;
	wd->toplevel = XtCreatePopupShell("sdtaudio", topLevelShellWidgetClass,
					  appShell, args, n);
	if (wd->toplevel == NULL)
		return(False);

	/* Create main window */
	n = 0;
	XtSetArg(args[0], XmNallowShellResize, True); ++n;
	wd->mainWindow = XmCreateMainWindow(wd->toplevel, "mainWindow", 
					    args, n);
	if (wd->mainWindow == NULL)
		return(False);
	XtManageChild(wd->mainWindow);
	help_rec.help_parent_win = wd->mainWindow;
	help_rec.help_event = HELP_AUDIO_MAIN_WIN_EVENT;
	help_rec.wd = wd;
	XtAddCallback(wd->mainWindow, XmNhelpCallback, HelpCb,
		      (XtPointer) &help_rec);

	/* Add the Drag and Drop functionality. */
	transfer_rec[0].callback = (XtCallbackProc) DndTransferCB;
	transfer_rec[0].closure = (XtPointer) wd;
	DtDndVaDropRegister(wd->mainWindow,
			    DtDND_FILENAME_TRANSFER | DtDND_BUFFER_TRANSFER,
			    XmDROP_COPY, transfer_rec,
			    NULL);

	/* Set window icon */
	dtb_cvt_image_file_to_pixmap(wd->toplevel,
				     IconPixmap,
				     &(iconPixmap));
	XtVaSetValues(wd->toplevel,
		      XmNiconName, appnameString,
		      XmNiconPixmap, iconPixmap,
		      XmNiconMask, iconMask,
		      XmNinitialState, NormalState,
		      NULL);

	/* Create the main sdtaudio GUI. */
	if (!SDtAuCreateBasePanel(wd))
		return(False);
 	if (!SDtAuCreateMenus(wd))
		return(False);
	if (!SDtAuCreateFooter(wd))
		return(False);
	if (!SDtAuCreateControls(wd))
		return(False);
	if (!SDtAuCreateWavePane(wd))
		return(False);
	XtManageChild(wd->base_form);

	/* Create WW font if not already...  */
	if (ww_font == NULL)
		SDtAuInitWWFont(wd->footer_msg);

	XmAddWMProtocolCallback(wd->toplevel, WM_DELETE_WINDOW, 
				ExitCb, (XtPointer) wd);
	XmAddWMProtocolCallback(wd->toplevel, WM_SAVE_YOURSELF, 
				SaveSessionCb, (XtPointer) NULL);

	XtRealizeWidget(wd->toplevel);
	XtPopup(wd->toplevel, XtGrabNone);

	/* Set up hidden/shown panes correctly */
	if (wd->options->showWave == True) {
		SDtAuShowWaveWin(wd);
		wd->show_wave = True;
	} else {
		XtUnmanageChild(wd->wave_pane);
	}

	SDtAuSetMessage(wd->footer_msg, "");
	SDtAuSetMessage(wd->status_msg, "");
	SetTitle(wd->toplevel, NULL);

	/* These should be insensitive initially, until a file is loaded,
	 * or after the user starts recording some audio.
	 */
	XtSetSensitive(wd->prog_scale, False);
	XtSetSensitive(wd->bwd_button, False);
	XtSetSensitive(wd->fwd_button, False);
	XtSetSensitive(wd->play_button, False);
	SDtAuUpdateView();

	/* Set the SIGPOLL handler. */
/* Temporarily disable, since it hangs GUI.
	if (!SDtAuSetSigpoll(wd))
		Fatal(catgets(msgCatalog, 4, 19, 
			      "could not set SIGPOLL handler"));
*/

	switch (loadtype) {

		case LOAD_EMPTY:
			return True;

		case LOAD_FILE:
			return LoadFile(wd, name_or_buf);

		case LOAD_BUFFER:
			return LoadBuffer(wd, name_or_buf, len, (char *) NULL);
	}
}


/* Function	: SDtAuCreateSaveAsDlog
 * Objective	: Create the save as dialog widgets.
 * Arguments	: WindowData *		- the sdtaudio data structure
 * Return Value : Boolean True if the widget was created, False otherwise.
 */
Boolean
SDtAuCreateSaveAsDlog(WindowData *wd)
{
	register int    ac = 0;
	Arg             al[16];
	Widget          shell, menu, form;
	XmString        xmstrings[16];
	Widget          children[24];
	static SDtAudioHelpR help_rec;

	ac = 0;
	XtSetArg(al[ac], XmNallowShellResize, True); ac++;
	XtSetArg(al[ac], XmNtitle,
		 catgets(msgCatalog, 1, 2, "Audio - Save As")); ac++;
	shell = XmCreateDialogShell(wd->mainWindow, "SaveAsShell", al, ac);
	if (shell == NULL)
		return(False);

	/* Set the directory while creating or we'll be stat'ing the
	 * directory twice if we change it again after creating it.
	 * That's why we're using XtVaCreateWidget() instead of
	 * XmCreateFileSelectionBox() which doesn't take XtVaTypedArgs.
	 */
	wd->saveas_dlog =
		XtVaCreateWidget("SaveAsDialog",
				 xmFileSelectionBoxWidgetClass, shell,
				 /* use this if you want to change the dir
				  * while creating XtVaTypedArg, XmNdirectory, 
				  * XtRString, prog->directory, 	 
                                  * strlen(prog->directory) + 1,
				  */
				 XmNdialogStyle, 
				 XmDIALOG_PRIMARY_APPLICATION_MODAL,
				 NULL);
	if (wd->saveas_dlog == NULL)
		return(False);
	XtAddCallback(wd->saveas_dlog, XmNcancelCallback, SaveAsCancelCb, wd);
	XtAddCallback(wd->saveas_dlog, XmNokCallback, SaveAsOkCb, wd);
	help_rec.help_parent_win = wd->saveas_dlog;
	help_rec.help_event = HELP_AUDIO_SAVE_AS_EVENT;
	help_rec.wd = wd;
	XtAddCallback(wd->saveas_dlog, XmNhelpCallback, HelpCb, 
		      (XtPointer) &help_rec);

	/* Create Form for extra widgets.  */
	ac = 0;
	XtSetArg(al[ac], XmNautoUnmanage, False); ac++;
	XtSetArg(al[ac], XmNmarginWidth, 0); ac++;
	form = XmCreateForm(wd->saveas_dlog, "SaveAsForm", al, ac);
	if (form == NULL)
		return(False);

	/*  Create Format Menu */
	ac = 0;
	XtSetArg(al[ac], XmNpacking, XmPACK_COLUMN); ac++;
	XtSetArg(al[ac], XmNorientation, XmVERTICAL); ac++;
	XtSetArg(al[ac], XmNnumColumns, 1); ac++;
	menu = XmCreatePulldownMenu(form, "FormatPulldown", al, ac);
	if (menu == NULL)
		return(False);

	/* Create the audio format options menu. */
	ac = 0;
	XtSetArg(al[ac], XmNsubMenuId, menu); ac++;
	xmstrings[0] = XmStringCreateLocalized(catgets(msgCatalog, 2, 63,
						       "Audio Format:"));
	XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
	wd->format_menu = XmCreateOptionMenu(form, "FormatMenu", al, ac);
	if (wd->format_menu == NULL)
		return(False);
	XmStringFree(xmstrings[0]);

	xmstrings[0] = 
		XmStringCreateLocalized(catgets(msgCatalog, 2, 64, "AU"));
	xmstrings[1] = 
		XmStringCreateLocalized(catgets(msgCatalog, 2, 65, "WAV"));
	xmstrings[2] =
		XmStringCreateLocalized(catgets(msgCatalog, 2, 66, "AIFF"));
	wd->menu_items->AuItem = 
		XtVaCreateWidget("AuItem", xmPushButtonGadgetClass, menu,
				 XmNlabelString, xmstrings[0],
				 NULL);
	wd->menu_items->WavItem =
		XtVaCreateWidget("WavItem", xmPushButtonGadgetClass, menu,
				 XmNlabelString, xmstrings[1],
				 NULL);
	wd->menu_items->AiffItem =
		XtVaCreateWidget("AiffItem", xmPushButtonGadgetClass, menu,
				 XmNlabelString, xmstrings[2],
				 NULL);
	XmStringFree(xmstrings[0]);
	XmStringFree(xmstrings[1]);
	XmStringFree(xmstrings[2]);
	if (wd->menu_items->AuItem == NULL || wd->menu_items->WavItem == NULL
	    || wd->menu_items->AiffItem == NULL)
		return(False);

	/* Manage the widgets */
	ac = 0;
	children[ac++] = wd->menu_items->AuItem;
	children[ac++] = wd->menu_items->WavItem;
	children[ac++] = wd->menu_items->AiffItem;
	XtManageChildren(children, ac);
	XtManageChild(wd->format_menu);
	XtManageChild(form);
}


/* Function	: SDtAuCreateAboutDlog()
 * Objective	: Create the About objects dialog.
 * Arguments	: WindowData *		- the sdtaudio structure
 * Return Value : Widget		- the about dialog widget id
 */
Widget
SDtAuCreateAboutDlog(WindowData * wd)
{
	register int    ac = 0;
	Arg             al[16];
	Widget          children[5];
	Widget          about_label1, about_label2, about_label3;
	XmString        xmstrings[5];
	char            text[512];
	char            actual_filename[MAXPATHLEN];
	Pixel           fg, bg;
	Pixmap          icon = NULL;
	Window          root;
	int             x, y;
	unsigned int    w = 0, h, border, depth;
	Dimension       w1;
	char           *icon_filename;
	Display        *dpy = XtDisplay(wd->toplevel);
	Screen         *screen = XtScreen(wd->toplevel);
	Widget          shell, dialog, about_icon, close_button, form;

	/* Create Shell, Dialog, Forms. */
	ac = 0;
	XtSetArg(al[ac], XmNtitle, catgets(msgCatalog, 1, 13,
					   "About Audio"));
	ac++;
	shell = XmCreateDialogShell(wd->toplevel, "AboutShell", al, ac);
	if (shell == NULL)
		return((Widget) NULL);

	/* Create Message Dialog Box. */
	ac = 0;
	XtSetArg(al[ac], XmNautoUnmanage, False); ac++;
	XtSetArg(al[ac], XmNdialogType, XmDIALOG_TEMPLATE); ac++;
	XtSetArg(al[ac], XmNnoResize, False); ac++;
	dialog = XmCreateMessageBox(shell, "AboutDialog", al, ac);
	if (dialog == NULL)
		return((Widget) NULL);

	/* Create Form */
	ac = 0;
	XtSetArg(al[ac], XmNautoUnmanage, False); ac++;
	XtSetArg(al[ac], XmNfractionBase, 4); ac++;
	form = XmCreateForm(dialog, "AboutForm", al, ac);
	if (form == NULL)
		return((Widget) NULL);

	/* Getting CDE icon, else try local icon */
	icon_filename = XmGetIconFileName(screen, NULL, AUDIO_ABOUT_ICON,
					  NULL, XmUNSPECIFIED_ICON_SIZE);
	actual_filename[0] = '\0';
	if (icon_filename) {

		/* Use bitmap if depth == 1 */
		if (DefaultDepthOfScreen(XtScreen(wd->toplevel)) == 1) {

			int             len = strlen(icon_filename);

			if (len > 3 &&
			    icon_filename[len - 1] == 'm' && 
			    icon_filename[len - 2] == 'p' &&
			    icon_filename[len - 3] == '.') {

				icon_filename[len - 2] = 'b';
				if (access(icon_filename, R_OK) != 0)
					/* put it back if not found */
					icon_filename[len - 2] = 'p';	
			}
		}

		sprintf(actual_filename, "%s", icon_filename);
		free(icon_filename);
	}

	if (actual_filename != NULL) {
		XtVaGetValues(form, XmNforeground, &fg,
			      XmNbackground, &bg, NULL);
		icon = XmGetPixmap(screen, actual_filename, fg, bg);
		free(icon_filename);
		if (icon != XmUNSPECIFIED_PIXMAP)
			XGetGeometry(dpy, icon, &root, &x, &y, &w, &h,
				     &border, &depth);
	}

	about_icon = XtVaCreateWidget("AboutIcon",
				      xmLabelWidgetClass,
				      form,
				      XmNtopAttachment, XmATTACH_FORM,
				      XmNtopOffset, 5,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNleftOffset, 10,
				      NULL);
	if (about_icon == NULL)
		return((Widget) NULL);

	if (icon == XmUNSPECIFIED_PIXMAP) {
		xmstrings[0] = 
			XmStringCreateLocalized(catgets(msgCatalog, 1, 22,
						        "Missing\nGraphics"));
		XtVaSetValues(about_icon,
			      XmNlabelType, XmSTRING,
			      XmNlabelString, xmstrings[0],
			      XmNborderWidth, 1,
			      NULL);
		XmStringFree(xmstrings[0]);
		XtVaGetValues(about_icon, XmNwidth, &w1, NULL);
		w = (int) w1;
	} else {

		XtVaSetValues(about_icon,
			      XmNlabelType, XmPIXMAP,
			      XmNlabelPixmap, icon,
			      NULL);
	}

	/*  Create Audio Version Label */
	ac = 0;
	sprintf(text, catgets(msgCatalog, 2, 69, "Audio Version %s"),
		AUDIO_VERSION);
	xmstrings[0] = XmStringCreateLocalized(text);
	XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNtopOffset, 10); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNleftOffset, w + 10 + 20); ac++;
	about_label1 = XmCreateLabel(form, "AboutLabel1", al, ac);
	XmStringFree(xmstrings[0]);
	if (about_label1 == NULL)
		return((Widget) NULL);

	/* Create Copyright label. */
	ac = 0;
	sprintf(text, catgets(msgCatalog, 1, 70, "Copyright (c) 1996-1997:"));
	xmstrings[0] = XmStringCreateLocalized(text);
	XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, about_label1); ac++;
	XtSetArg(al[ac], XmNtopOffset, 10); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
	XtSetArg(al[ac], XmNleftWidget, about_label1); ac++;
	about_label2 = XmCreateLabel(form, "AboutLabel2", al, ac);
	XmStringFree(xmstrings[0]);
	if (about_label2 == NULL)
		return((Widget) NULL);	ac = 0;


	/* Create Sun Microsystems label. */
	sprintf(text, catgets(msgCatalog, 1, 71, "Sun Microsystems, Inc."));
	xmstrings[0] = XmStringCreateLocalized(text);
	XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, about_label2); ac++;
	XtSetArg(al[ac], XmNtopOffset, 3); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
	XtSetArg(al[ac], XmNleftWidget, about_label1); ac++;
	XtSetArg(al[ac], XmNleftOffset, 25); ac++;
	about_label3 = XmCreateLabel(form, "AboutLabel3", al, ac);
	XmStringFree(xmstrings[0]);
	if (about_label3 == NULL)
		return((Widget) NULL);	ac = 0;

	/* Create Close button */
	ac = 0;
	xmstrings[0] = XmStringCreateLocalized(catgets(msgCatalog, 2, 72,
						       "Close"));
	XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
	close_button = XmCreatePushButton(dialog, "AboutCloseButton",
					  al, ac);
	XmStringFree(xmstrings[0]);
	if (close_button == NULL)
		return((Widget) NULL);

	/* Set Close to be default button. */
	XtVaSetValues(dialog, XmNdefaultButton, close_button, NULL);

	/* Set callbacks on buttons */
	XtAddCallback(close_button, XmNactivateCallback,
		      GenericCancelCb, dialog);

	/* Manage the children */
	ac = 0;
	children[ac++] = about_icon;
	children[ac++] = about_label1;
	children[ac++] = about_label2;
	children[ac++] = about_label3;
	XtManageChildren(children, ac);

	ac = 0;
	children[ac++] = form;
	children[ac++] = close_button;
	XtManageChildren(children, ac);
	XtManageChild(dialog);

	return dialog;
}


/* Function	: SDtAuCreateInfoDlog()
 * Objective	: Create the info dialog that will contain info about
 *		  the audio file.
 * Arguments	: WindowData *		- the sdtaudio data structure
 * Return Value : Boolean True if the window is created, false otherwise.
 */
Boolean
SDtAuCreateInfoDlog(WindowData * wd)
{
	register int    ac;
	Widget          children[16];
	Arg             al[10];
	Widget          filename_label, filesize_label;
	Widget		filelen_label, nchannels_label;
	Widget          samrate_label, encoding_label;
	Widget          close_button, help_button, shell, form;
	XmString        label_str = NULL;
	static SDtAudioHelpR	help_rec;

	/* Shell. */
	ac = 0;
	XtSetArg(al[ac], XmNallowShellResize, True); ac++;
	XtSetArg(al[ac], XmNtitle,
		 catgets(msgCatalog, 2, 56, "Audio - File Information")); ac++;
	shell = XmCreateDialogShell(wd->mainWindow, "FileInfoShell", al, ac);
	if (shell == NULL)
		return(False);

	/* Message Dialog. */
	ac = 0;
	XtSetArg(al[ac], XmNautoUnmanage, False); ac++;
	XtSetArg(al[ac], XmNdialogType, XmDIALOG_TEMPLATE); ac++;
	XtSetArg(al[ac], XmNnoResize, False); ac++;
	XtSetArg(al[ac], XmNfractionBase, 100); ac++;
	wd->info_dlog = XmCreateMessageBox(shell, "FileInfoDialog", al, ac);
	if (wd->info_dlog == NULL)
		return(False);

	/* Create Form. */
	ac = 0;
	XtSetArg(al[ac], XmNautoUnmanage, False); ac++;
	XtSetArg(al[ac], XmNfractionBase, 100); ac++;
	form = XmCreateForm(wd->info_dlog, "FileInfoForm", al, ac);
	if (form == NULL)
		return(False);

	/* File Name label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 44,
						    "File Name:"));
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNrightPosition, 50); ac++;
	XtSetArg(al[ac], XmNrightOffset, 10); ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_END); ac++;
	filename_label = XmCreateLabel(form, "FileNameLabel", al, ac);
	XmStringFree(label_str);
	if (filename_label == NULL)
		return(False);

	/* File Size label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 45,
						    "File Size:"));
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, filename_label); ac++;
	XtSetArg(al[ac], XmNtopOffset, 5); ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_END); ac++;
	XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNrightPosition, 50); ac++;
	XtSetArg(al[ac], XmNrightOffset, 10); ac++;
	filesize_label = XmCreateLabel(form, "FileSizeLabel", al, ac);
	XmStringFree(label_str);
	if (filesize_label == NULL)
		return(False);

	/* File Length label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 46,
						    "File Length:"));
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, filesize_label); ac++;
	XtSetArg(al[ac], XmNtopOffset, 5); ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_END); ac++;
	XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNrightPosition, 50); ac++;
	XtSetArg(al[ac], XmNrightOffset, 10); ac++;
	filelen_label = XmCreateLabel(form, "FileLengthLabel", al, ac);
	XmStringFree(label_str);
	if (filelen_label == NULL)
		return(False);

	/* Number of Channels label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 47,
						    "Number of Channels:"));
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, filelen_label); ac++;
	XtSetArg(al[ac], XmNtopOffset, 5); ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_END); ac++;
	XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNrightPosition, 50); ac++;
	XtSetArg(al[ac], XmNrightOffset, 10); ac++;
	nchannels_label = XmCreateLabel(form, "NumChannelsLabel", al, ac);
	XmStringFree(label_str);
	if (nchannels_label == NULL)
		return(False);

	/* Sample Rate label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 48,
						    "Sample Rate:"));
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, nchannels_label); ac++;
	XtSetArg(al[ac], XmNtopOffset, 5); ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_END); ac++;
	XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNrightPosition, 50); ac++;
	XtSetArg(al[ac], XmNrightOffset, 10); ac++;
	samrate_label = XmCreateLabel(form, "SampleRateLabel", al, ac);
	XmStringFree(label_str);
	if (samrate_label == NULL)
		return(False);

	/* Encoding label. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 49,
						    "Encoding:"));
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, samrate_label); ac++;
	XtSetArg(al[ac], XmNtopOffset, 5); ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_END); ac++;
	XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNrightPosition, 50); ac++;
	XtSetArg(al[ac], XmNrightOffset, 10); ac++;
	encoding_label = XmCreateLabel(form, "EncodingLabel", al, ac);
	XmStringFree(label_str);
	if (encoding_label == NULL)
		return(False);

	/* File Name value. */
	label_str = XmStringCreateLocalized(" ");
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	XtSetArg(al[ac], XmNrecomputeSize, True); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNleftPosition, 50); ac++;
	wd->info_name = XmCreateLabel(form, "FileNameValue", al, ac);
	if (wd->info_name == NULL)
		return(False);

	/* File Size value. */
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	XtSetArg(al[ac], XmNrecomputeSize, True); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, wd->info_name); ac++;
	XtSetArg(al[ac], XmNtopOffset, 5); ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNleftPosition, 50); ac++;
	wd->info_size = XmCreateLabel(form, "FileSizeValue", al, ac);
	if (wd->info_size == NULL)
		return(False);

	/* File Length value. */
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	XtSetArg(al[ac], XmNrecomputeSize, True); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, wd->info_size); ac++;
	XtSetArg(al[ac], XmNtopOffset, 5); ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNleftPosition, 50); ac++;
	wd->info_length = XmCreateLabel(form, "FileLengthValue", al, ac);
	if (wd->info_length == NULL)
		return(False);

	/* Number of Channels value. */
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	XtSetArg(al[ac], XmNrecomputeSize, True); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, wd->info_length); ac++;
	XtSetArg(al[ac], XmNtopOffset, 5); ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNleftPosition, 50); ac++;
	wd->info_channels = XmCreateLabel(form, "NumChannelsValue", al, ac);
	if (wd->info_channels == NULL)
		return(False);

	/* Sample Rate value. */
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	XtSetArg(al[ac], XmNrecomputeSize, True); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, wd->info_channels); ac++;
	XtSetArg(al[ac], XmNtopOffset, 5); ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNleftPosition, 50); ac++;
	wd->info_sample_rate = XmCreateLabel(form, "SampleRateValue", al, ac);
	if (wd->info_sample_rate == NULL)
		return(False);

	/* Encoding value */
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	XtSetArg(al[ac], XmNrecomputeSize, True); ac++;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, wd->info_sample_rate); ac++;
	XtSetArg(al[ac], XmNtopOffset, 5); ac++;
	XtSetArg(al[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNleftPosition, 50); ac++;
	wd->info_encoding = XmCreateLabel(form, "EncodingValue", al, ac);
	XmStringFree(label_str);
	if (wd->info_encoding == NULL)
		return(False);

	/* Close Button */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 72,
						    "Close"));
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	close_button = XmCreatePushButton(wd->info_dlog, "InfoCloseButton",
					  al, ac);
	XmStringFree(label_str);
	if (close_button == NULL)
		return(False);
	XtAddCallback(close_button, XmNactivateCallback, GenericCancelCb,
		      wd->info_dlog);

	/* Help Button. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 7, "Help"));
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, label_str); ac++;
	help_button = XmCreatePushButton(wd->info_dlog, "InfoHelpButton",
					 al, ac);
	XmStringFree(label_str);
	if (help_button == NULL)
		return(False);
	help_rec.help_parent_win = wd->info_dlog;
	help_rec.help_event = HELP_AUDIO_INFO_EVENT;
	XtAddCallback(help_button, XmNactivateCallback, HelpCb, 
		      (XtPointer) &help_rec);

	/* Set Close to be default button. */
	XtVaSetValues(wd->info_dlog, XmNdefaultButton, close_button, NULL);

	/* Manage the widgets. */
	ac = 0;
	children[ac++] = filename_label;
	children[ac++] = filesize_label;
	children[ac++] = filelen_label;
	children[ac++] = nchannels_label;
	children[ac++] = samrate_label;
	children[ac++] = encoding_label;
	children[ac++] = wd->info_name;
	children[ac++] = wd->info_size;
	children[ac++] = wd->info_length;
	children[ac++] = wd->info_channels;
	children[ac++] = wd->info_sample_rate;
	children[ac++] = wd->info_encoding;
	XtManageChildren(children, ac);

	ac = 0;
	children[ac++] = form;
	children[ac++] = close_button;
	children[ac++] = help_button;
	XtManageChildren(children, ac);
	return(True);
}


/* Function	: SDtAuCreateOptionsDlog()
 * Objective	: Create the widgets that make up the sdtaudio Options
 *		  dialog.
 * Arguments	: WindowData *		- the sdtaudio data structure
 * Return Value : Boolean True if the widget was successfully created,
 *		  False otherwise.
 */
Boolean 
SDtAuCreateOptionsDlog(WindowData * wd)
{
	Arg             args[25];
	int             n = 0;
	XmString        label_str = NULL;
	Widget		category_pulldown, sep;
	Widget		wids[4];
	Dimension	form_width, form_height, object_width, object_height;
	Dimension	border_width = 1;


	/* Create the Options dialog shell. */
	n = 0;
	XtSetArg(args[n], XmNallowShellResize, True);
	XtSetArg(args[n], XmNtitle, catgets(msgCatalog, 3, 56, 
					    "Audio - Options"));
	++n;
	XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); ++n;
	wd->options_dlog = XmCreateDialogShell(wd->toplevel, "OptionsDialog",
                                               args, n);
	if (wd->options_dlog == NULL)
		return(False);

	/* Want our own WM_DELETE_WINDOW routine for the options dialog. */
	XmAddWMProtocolCallback(wd->options_dlog, WM_DELETE_WINDOW,
			        OptionsCancelCb, (XtPointer) wd);

	/* Create the main form of the Options dialog window. */
	n = 0;
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); ++n;
	XtSetArg(args[n], XmNnoResize, True);
	wd->options_form = XmCreateForm(wd->options_dlog, "OptionsForm",
				    args, n);
	if (wd->options_form == NULL)
		return(False);

	/* Create the category pulldown menu that will be attached
	 * to the options menu.
	 */
	category_pulldown = 
		XmCreatePulldownMenu(wd->options_form, 
				     "OptionsCategoryPulldown", NULL, 0);
	if (category_pulldown == NULL)
		return(False);

	/* Create the Startup menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 1, 33, 
						    "Startup"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->menu_items->StartupItem =
		XmCreatePushButton(category_pulldown, "StartupItem",
				   args, n);
	XmStringFree(label_str);
	if (wd->menu_items->StartupItem == NULL)
		return(False);
	XtAddCallback(wd->menu_items->StartupItem, XmNactivateCallback,
		      OptionsDlogPaneCb, wd);
	XtManageChild(wd->menu_items->StartupItem);

	/* Create the Play menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 1, 34, 
						    "Play"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->menu_items->PlayItem =
		XmCreatePushButton(category_pulldown, "PlayItem", args, n);
	XmStringFree(label_str);
	if (wd->menu_items->PlayItem == NULL)
		return(False);
	XtAddCallback(wd->menu_items->PlayItem, XmNactivateCallback,
		      OptionsDlogPaneCb, wd);
	XtManageChild(wd->menu_items->PlayItem);

	/* Now create the Record menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 1, 35,
					    "Record"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->menu_items->RecordItem =
		XmCreatePushButton(category_pulldown, "RecordItem",
			           args, n);
	XmStringFree(label_str);
	if (wd->menu_items->RecordItem == NULL)
		return(False);
	XtAddCallback(wd->menu_items->RecordItem, XmNactivateCallback,
		      OptionsDlogPaneCb, wd);
	XtManageChild(wd->menu_items->RecordItem);

	/* Now create the Devices menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 1, 36,
					    "Devices"));
	n = 0;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->menu_items->DevicesItem =
		XmCreatePushButton(category_pulldown, "DevicesItem",
			           args, n);
	XmStringFree(label_str);
	if (wd->menu_items->DevicesItem == NULL)
		return(False);
	XtAddCallback(wd->menu_items->DevicesItem, XmNactivateCallback,
		      OptionsDlogPaneCb, wd);
	XtManageChild(wd->menu_items->DevicesItem);

	/* Create the category options submenu. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 1,
						    "Category:"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNsubMenuId, category_pulldown); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->options_category = XmCreateOptionMenu(wd->options_form, 
						  "CategoryMenu", args, n);
	XmStringFree(label_str);
	if (wd->options_category == NULL)
		return(False);
	XtManageChild(wd->options_category);

	/* Create a separator. */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->options_category); ++n;
	XtSetArg(args[n], XmNtopOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, border_width); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightOffset, border_width); ++n;
	XtSetArg(args[n], XmNheight, 5); ++n;
	sep = XmCreateSeparator(wd->options_form, "Separator", args, n);
	if (sep == NULL)
		return(False);
	XtManageChild(sep);

	/* Create the start-up options pane. */
	if (!SDtAuCreateStartupOptions(wd, sep))
		return(False);

	/* Create the play options pane. */
	if (!SDtAuCreatePlayOptions(wd, sep))
		return(False);

	/* Create the record options pane. */
	if (!SDtAuCreateRecordOptions(wd, sep))
		return(False);

	/* Create the devices options pane. */
	if (!SDtAuCreateDevicesOptions(wd, sep))
		return(False);

	/* Create the command buttons pane. */
	if (!SDtAuCreateCmdButtonsPane(wd))
		return(False);

	/* Manage the form. */
	XtManageChild(wd->options_form);

	/* Set the default button and cancel button. */
	XtVaSetValues(wd->options_form,
		      XmNdefaultButton, wd->options_ok,
		      XmNcancelButton, wd->options_cancel,
		      NULL);

	/* Get the height of the command button pane - this will 
	 * be used to add to the minimum height of the options
	 * dialog.
	 */
	XtVaGetValues(wd->options_cmd_pane,
		      XmNheight, &object_height,
		      NULL);

	/* Make all the underlying panes the same size, so that
	 * doesn't resize when switching from one category to the next.
	 */
	n = 0;
	wids[n] = wd->opt_startup_pane; ++n;
	wids[n] = wd->opt_play_pane; ++n;
	wids[n] = wd->opt_record_pane; ++n;
	wids[n] = wd->opt_devices_pane; ++n;
	SDtAuSetSameDimension(wids, n, XmNheight);

	/* Attach the command pane after the pane's sizes have been
	 * calculated, and manage the form.
	 */
	XtVaSetValues(wd->options_cmd_pane,
		      XmNtopAttachment, XmATTACH_WIDGET,
		      XmNtopWidget, wd->opt_startup_pane,
		      XmNtopOffset, 15,
		      NULL);

	/* Center the category options menu by calculating the offset
	 * based on (size of form - size of options menu) / 2.
	 */
	XtVaGetValues(wd->options_form,
		      XmNwidth, &form_width,
		      XmNheight, &form_height,
		      NULL);
	XtVaGetValues(wd->options_category,
		      XmNwidth, &object_width,
		      NULL);
	XtVaSetValues(wd->options_category,
		      XmNleftOffset, (int) ((form_width - object_width) / 2),
		      NULL);

	/* Hack - size the dialog shell so that the command pane can be
	 * seen.  20 refers to the offset for both top and bottom.
	 */
	XtVaSetValues(wd->options_dlog,
		      XmNminHeight, form_height + object_height + 20,
		      XmNmaxHeight, form_height + object_height + 20,
		      XmNminWidth, form_width,
		      XmNmaxWidth, form_width,
		      NULL);
	return(True);
}



/* Function	: SDtAuCreateRecordValuesDlog()
 * Objective	: Create the widgets that are required for the recording
 *		  values dialog.  The recording values dialog comes up
 *		  when the user has "Ask me when I start recording" set
 *		  for their options, and the user has pressed the record
 *		  button.  This dialog inquires from the user the recording
 *		  quality and format to use when recording.
 * Arguments	: WindowData *		- sdtaudio data structure
 * Return Value : Boolean True if the dialog is successfully created, False
 *		  otherwise.
 */
Boolean
SDtAuCreateRecordValuesDlog(WindowData *wd)
{
	Widget          form, quality_pd, format_pd;
	Widget		menu_voice, menu_cd, menu_dat;
	Widget		menu_au, menu_wav, menu_aiff;
	Widget		wids[8];
	Arg             args[20];
        register int    n;
	XmString        ok, cancel, help, label_str;
	static SDtAudioHelpR help_rec;

	/* Create strings for dialog's default buttons and main label. */
	ok = XmStringCreateLocalized(catgets(msgCatalog, 1, 35, "Record"));
	cancel =  XmStringCreateLocalized(catgets(msgCatalog, 2, 40, "Cancel"));
	help =  XmStringCreateLocalized(catgets(msgCatalog, 2, 7, "Help"));
	label_str =  XmStringCreateLocalized(catgets(msgCatalog, 2, 41, 
				"Specify audio recording parameters:"));

	/* Create Message Dialog Box. */
	n = 0;
	XtSetArg(args[n], XmNautoUnmanage, False); ++n;
	XtSetArg(args[n], XmNnoResize, True); ++n;
	XtSetArg(args[n], XmNokLabelString, ok); ++n;
	XtSetArg(args[n], XmNcancelLabelString, cancel); ++n;
	XtSetArg(args[n], XmNhelpLabelString, help); ++n;
	XtSetArg(args[n], XmNtitle,
		 catgets(msgCatalog, 2, 53, "Audio - Set Recording Options"));
	++n;
	wd->recval_dlog = XmCreateTemplateDialog(wd->toplevel, 
						 "RecordingValuesDialog",
						 args, n);
	XmStringFree(ok);
	XmStringFree(cancel);
	XmStringFree(help);
	if (wd->recval_dlog == NULL)
		return(False);
	XtAddCallback(wd->recval_dlog, XmNcancelCallback, SetRecordValuesCb, 
		      (XtPointer) wd);
	XtAddCallback(wd->recval_dlog, XmNokCallback, SetRecordValuesCb, 
		      (XtPointer) wd);
	help_rec.help_parent_win = wd->recval_dlog;
	help_rec.help_event = HELP_AUDIO_SET_RECORD_VALUES_EVENT;
	help_rec.wd = wd;
	XtAddCallback(wd->recval_dlog, XmNhelpCallback, HelpCb, 
		      (XtPointer) &help_rec);

	/* Create the form that is a child of the dialog.  This form
	 * will contain the values that can be set.
	 */
	n = 0;
	XtSetArg(args[n], XmNautoUnmanage, False); ++n;
	XtSetArg(args[n], XmNmarginWidth, 0); ++n;
	form = XmCreateForm(wd->recval_dlog, "RecordValuesForm", args, n);
	if (form == NULL)
		return(False);

	/* Create the recording quality pulldown menu. */
	quality_pd = XmCreatePulldownMenu(form, "recqual_pulldown", NULL, 0);
	if (quality_pd == NULL)
		return(False);

	/* Create the voice menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 32, 
						    "Voice"));
	menu_voice = XtVaCreateWidget("voice_menu_item",
				      xmPushButtonGadgetClass, quality_pd,
				      XmNlabelString, label_str,
				      NULL);
	XmStringFree(label_str);
	if (menu_voice == NULL)
		return(False);
	XtManageChild(menu_voice);


	/* Create the CD menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 33, "CD"));
	menu_cd = XtVaCreateWidget("cd_menu_item",
				   xmPushButtonGadgetClass, quality_pd,
				   XmNlabelString, label_str,
				   NULL);
	XmStringFree(label_str);
	if (menu_cd == NULL)
		return(False);
	XtManageChild(menu_cd);

	/* Create the DAT menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 68, "DAT"));
	menu_dat = XtVaCreateWidget("dat_menu_item",
				    xmPushButtonGadgetClass, quality_pd,
				    XmNlabelString, label_str,
				    NULL);
	XmStringFree(label_str);
	if (menu_dat == NULL)
		return(False);
	XtManageChild(menu_dat);

	/* Create the audio quality options menu. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 31,
						    "Audio Quality:"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNtopOffset, 15); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, 20); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightOffset, 20); ++n;
	XtSetArg(args[n], XmNsubMenuId, quality_pd); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->recval_quality = XmCreateOptionMenu(form, "recqual_option", 
						args, n);
	XmStringFree(label_str);
	if (wd->recval_quality == NULL)
		return(False);

	/* Create the audio format pulldown menu. */
	format_pd = XmCreatePulldownMenu(form, "audformat_pulldown", NULL, 0);
	if (format_pd == NULL)
		return(False);

	/* Create the AU menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 64, "AU"));
	menu_au = XtVaCreateWidget("au_menu_item",
				    xmPushButtonGadgetClass, format_pd,
				    XmNlabelString, label_str,
				    NULL);
	XmStringFree(label_str);
	if (menu_au == NULL)
		return(False);
	XtManageChild(menu_au);

	/* Create the WAV menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 65, "WAV"));
	menu_wav = XtVaCreateWidget("wav_menu_item",
				    xmPushButtonGadgetClass, format_pd,
				    XmNlabelString, label_str,
				    NULL);
	XmStringFree(label_str);
	if (menu_wav == NULL)
		return(False);
	XtManageChild(menu_wav);

	/* Create the AIFF menu item. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 66, "AIFF"));
	menu_aiff = XtVaCreateWidget("aiff_menu_item",
				     xmPushButtonGadgetClass, format_pd,
				     XmNlabelString, label_str,
				     NULL);
	XmStringFree(label_str);
	if (menu_aiff == NULL)
		return(False);
	XtManageChild(menu_aiff);

	/* Create the audio format options menu. */
	label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 63,
						    "Audio Format:"));
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(args[n], XmNtopWidget, wd->recval_quality); ++n;
	XtSetArg(args[n], XmNtopOffset, 20); ++n;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNbottomOffset, 10); ++n;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNleftOffset, 20); ++n;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(args[n], XmNrightOffset, 20); ++n;
	XtSetArg(args[n], XmNsubMenuId, format_pd); ++n;
	XtSetArg(args[n], XmNlabelString, label_str); ++n;
	wd->recval_format = XmCreateOptionMenu(form, "audformat_option",
					       args, n);
	XmStringFree(label_str);
	if (wd->recval_format == NULL)
		return(False);

	/* Make all the toggle buttons the same size! */
	n = 0;
	wids[n] = menu_voice; ++n;
	wids[n] = menu_cd; ++n;
	wids[n] = menu_dat; ++n;
	wids[n] = menu_au; ++n;
	wids[n] = menu_wav; ++n;
	wids[n] = menu_aiff; ++n;
	wids[n] = wd->recval_quality; ++n;
	wids[n] = wd->recval_format; ++n;
	SDtAuSetSameDimension(wids, n, XmNwidth);

	/* Manage the options widget and form. */
	XtManageChild(wd->recval_quality);
	XtManageChild(wd->recval_format);
	XtManageChild(form);

	return(True);
}

