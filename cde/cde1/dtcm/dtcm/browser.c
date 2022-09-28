/* $XConsortium: browser.c /main/cde1_maint/4 1995/11/14 16:55:34 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)browser.c	1.121 96/11/12 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/param.h>
#include <csa.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/Protocols.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>
#include <Xm/SashP.h>
#include <Dt/HourGlass.h>
#include <Tt/tttk.h>
#include "thrdefs.h"
#include "getdate.h"
#include "calendar.h"
#include "util.h"
#include "timeops.h"
#include "browser.h"
#include "blist.h"
#include "format.h"
#include "datefield.h"
#include "props.h"
#include "props_pu.h"
#include "editor.h"
#include "group_editor.h"
#include "select.h"
#include "help.h"
#ifdef FNS
#include "cmfns.h"
#endif

#define DATESIZ 40
#ifndef NOTHREADS
#define  THREAD_EXISTS(alive) (alive != NULL)
static void mb_logon_thread_exit(BlistData *, int *, XtInputId *);
static void *mb_logon_thread(BrowselistThreadData *);
static void mb_timer_cb(XtPointer, XtIntervalId	*);
extern void bl_list_is_changed(Browselist *);
extern void bl_list_not_changed(Browselist *);
extern void disable_menu_editor(Browselist *, Boolean);
#endif /* NOTHREADS */

extern int debug;

static void mb_resize_proc(Widget, XtPointer, XtPointer);
static void cancel_cb(Widget, XtPointer, XtPointer);
static void popup_cb(Widget, XtPointer, XtPointer);
static void init_browser(Calendar *);
static void bcanvas_repaint(Widget, XtPointer, XtPointer);
static void bcanvas_event(Widget, XtPointer, XtPointer);
static void goto_date_cb(Widget, XtPointer, XtPointer);
static void browselist_from_browser(Widget, XtPointer, XtPointer);
static void gotomenu_cb(Widget, XtPointer, XtPointer);
static void schedule_cb(Widget, XtPointer, XtPointer);
static void mail_cb(Widget, XtPointer, XtPointer);
static void mb_box_notify(Widget, XtPointer, XtPointer);
static void mb_update_handler(CSA_session_handle, CSA_flags, CSA_buffer,
			      CSA_buffer, CSA_extension *);

extern void scrub_attr_list(Dtcm_appointment *);

static void
mb_init_array(Browser *b, int begin, int end) {
        b->segs_in_array = BOX_SEG * (end - begin) * 7;
        b->multi_array = (char*)XtCalloc(1, b->segs_in_array);
}

static void
reset_ticks(Calendar *c, Boolean use_sel_idx) {
	int	beg, end;
	Props	*p = (Props *)c->properties;
        Browser	*b = (Browser*)c->browser;

	beg = get_int_prop(p, CP_DAYBEGIN);
	end = get_int_prop(p, CP_DAYEND);

	if (b->date <= get_bot()) {
		b->date = get_bot();
		if (b->col_sel > 0)
			b->col_sel = b->col_sel - 4;
	}
	if ((b->begin_week_tick = first_dow(b->date)) < get_bot())
		b->begin_week_tick = get_bot();
	if (use_sel_idx) {
		b->begin_day_tick =
			next_ndays(b->begin_week_tick, b->col_sel);
		b->begin_hr_tick =
			next_nhours(b->begin_day_tick, beg + b->row_sel);
	} else {
		b->begin_day_tick = lowerbound(b->date);
		b->begin_hr_tick = next_nhours(b->begin_day_tick, beg);
	}
        b->end_day_tick = upperbound(b->begin_day_tick);
        b->end_hr_tick = next_nhours(b->begin_hr_tick, 1);
}

extern void
br_display(Calendar *c) {
	int		i, *pos_list = NULL, pos_cnt;
        Browser		*b = (Browser *)c->browser;
	BlistData	*bd;
	Browselist	*bl = (Browselist *)c->browselist;
	void		mb_update_array();

	if (!b)
		return;

        for (i = 0; i < b->segs_in_array; i++)
		b->multi_array[i] = 0;
        b->add_to_array = True;

	XmListGetSelectedPos(b->browse_list, &pos_list, &pos_cnt);
	for (i = 0; i < pos_cnt; i++) {
		bd = (BlistData *)CmDataListGetData(bl->blist_data,
						    pos_list[i]);

		destroy_paint_cache(bd->cache, bd->cache_size);
		bd->cache = NULL;
		bd->cache_size = 0;

		if (bd)
			mb_update_array(bd->name, c);
	}
	if (pos_list)
		XtFree((XtPointer)pos_list);

        mb_refresh_canvas(b, c);
}

static void
invalid_date_msg(Calendar *c, Widget widget)
{
	Browser *b = (Browser*)c->browser;
	char *title = XtNewString(catgets(c->DT_catd, 1, 1070, 
				  "Calendar : Error - Compare Calendars"));
	char *text = XtNewString(catgets(c->DT_catd, 1, 20,
					 "Invalid Date In Go To Field."));
	char *ident = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));

	dialog_popup(b->frame,
		DIALOG_TITLE, title,
		DIALOG_TEXT, text,
		BUTTON_IDENT, 1, ident,
		DIALOG_IMAGE, ((Props_pu *)c->properties_pu)->xm_error_pixmap,
		NULL);

	XtFree(ident);
	XtFree(text);
	XtFree(title);
}

void 
set_entry_date(Calendar *c) {
        char *date = NULL;
        Browser *b;
        Props *p;
        OrderingType ot;
        SeparatorType st;
	Tick tick;

        b = (Browser*)c->browser;
        p = (Props*)c->properties;
        ot = get_int_prop(p, CP_DATEORDERING);
        st = get_int_prop(p, CP_DATESEPARATOR);

        date = get_date_from_widget(c->view->date, b->datetext, ot, st);
    	if (date == NULL ) {
		invalid_date_msg(c, b->message_text);
                return;
        }

        tick = cm_getdate(date, NULL);
	if (!timeok(tick)) {
		invalid_date_msg(c, b->message_text);
		return;
	}

        b->date = tick;
        reset_ticks(c, False);
        br_display(c);
}

/*
 * A note about the browser:
 * This custom dialog is built using two nested PanedWindowWidgets.
 * The first child of the outer pane is itself paned (the other
 * child is the dialog's action area).  The inner pane is movable
 * to allow users to reproportion the calendar list versus the free
 * time chart.  Most of the useful widget handles are stored in the
 * Browser structure allocated and returned from here.
 */
extern void
make_browser(Calendar *c)
{
	Browser 	*b;
	Props 		*p = (Props*) c->properties;
	Widget 		separator1;
	Dimension 	w, h, height;
	XmString 	xmstr;
	XmString goto_label, prev_week, this_week, next_week, prev_month, next_month;
	int 		num_children;
	Widget 		*children;
	Atom 		wm_del_atom;
        OrderingType 	ord_t = get_int_prop(p, CP_DATEORDERING);
        SeparatorType 	sep_t = get_int_prop(p, CP_DATESEPARATOR);
	char 		buf[BUFSIZ];
	Widget		text_field_form;
	int		outpane_width, outpane_height;
	int		upform_min,item_count;
	char		*title;

	if (c->browser == NULL) {
                c->browser = (caddr_t)XtCalloc(1, sizeof(Browser));
                b = (Browser*)c->browser;
        }
        else
                b = (Browser*)c->browser;

	b->date = c->view->date;

        mb_init_array(b, get_int_prop(p, CP_DAYBEGIN), get_int_prop(p, CP_DAYEND));
        b->current_selection = (caddr_t) XtCalloc(1, sizeof(Selection));

	/* if the screen is small adjust the max size for width and height
	 * so the shell can be moved up and down using window facility
	 */
	if ((WidthOfScreen(XtScreen(c->frame)) < 360) ||
	    (HeightOfScreen(XtScreen(c->frame)) < 600 ))
	{
		outpane_width = 300;
		outpane_height = 430;
		item_count	= 3;
		upform_min	= 120;
	}
	else
	{
		outpane_width = 360;
		outpane_height = 600;
		item_count	= 8;
		upform_min	= 200;
	}

	title = XtNewString(catgets(c->DT_catd, 1, 1010, 
					"Calendar : Compare Calendars"));
	b->frame = XtVaCreatePopupShell("frame",
                xmDialogShellWidgetClass, c->frame,
                XmNtitle, title,
                XmNallowShellResize, True,
                XmNmappedWhenManaged, False, 
		XmNdeleteResponse, XmDO_NOTHING,
                NULL);
	XtFree(title);
	XtAddCallback(b->frame, XmNpopupCallback, popup_cb, (XtPointer)c);


	/*
	 * Create the outer pane, whose upper part will hold a
	 * nested pane, and whose lower part will hold the actions
	 * and message area
	 */
	b->outer_pane = XtVaCreateManagedWidget("outerPane",
		xmPanedWindowWidgetClass, b->frame,
		XmNsashHeight, 1,
		XmNsashWidth, 1,
		XmNwidth, outpane_width,
		XmNheight, outpane_height,
		NULL);

	b->inner_pane = XtVaCreateManagedWidget("innerPane",
		xmPanedWindowWidgetClass, b->outer_pane,
		XmNallowResize, True,
		NULL);

	b->upper_form = XtVaCreateManagedWidget("upperForm",
		xmFormWidgetClass, b->inner_pane,
		XmNallowResize, True,
		XmNpaneMinimum, upform_min,
		NULL);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 22, "Browse Menu Items"));
        b->list_label = XtVaCreateWidget("browseMenuLabel", 
		xmLabelGadgetClass, b->upper_form,
		XmNlabelString, xmstr,
		XmNleftAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
		XmNtopOffset, 10,
                NULL);
	XmStringFree(xmstr);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 21, "Edit List..."));
        b->edit_list = XtVaCreateWidget("editList",
		xmPushButtonGadgetClass, b->upper_form,
		XmNlabelString, xmstr,
		XmNtopAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_WIDGET,
		XmNleftWidget, b->list_label,
		XmNleftOffset, 80,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(b->edit_list, XmNactivateCallback,
		browselist_from_browser, (XtPointer)c);

	b->browse_list = (Widget)XmCreateScrolledList(b->upper_form,
                "browseList", NULL, 0);
        XtAddCallback(b->browse_list,
		XmNmultipleSelectionCallback, mb_box_notify, (XtPointer)c);
        XtAddCallback(b->browse_list,
		XmNdefaultActionCallback, mb_box_notify, (XtPointer)c);
	XtVaSetValues(b->browse_list,
		XmNselectionPolicy, XmMULTIPLE_SELECT,
               	XmNvisibleItemCount, item_count,
		NULL);

        b->browse_list_sw = XtParent(b->browse_list);
	XtVaSetValues(b->browse_list_sw,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNtopWidget, b->edit_list,
                XmNleftAttachment, XmATTACH_FORM,
		XmNleftOffset, 20,
                XmNrightAttachment, XmATTACH_FORM,
		XmNrightOffset, 20,
		XmNbottomAttachment, XmATTACH_FORM,
                XmNvisualPolicy, XmVARIABLE,
		NULL);


	/*
	 * Create the "go to" option menu for time navigation
	 */
	prev_week =
	   XmStringCreateLocalized(catgets(c->DT_catd, 1, 23, "Prev Week"));
	this_week =
	   XmStringCreateLocalized(catgets(c->DT_catd, 1, 24, "This Week"));
	next_week =
	   XmStringCreateLocalized(catgets(c->DT_catd, 1, 25, "Next Week"));
	prev_month =
	   XmStringCreateLocalized(catgets(c->DT_catd, 1, 26, "Prev Month"));
	next_month =
	   XmStringCreateLocalized(catgets(c->DT_catd, 1, 27, "Next Month"));
	goto_label =
	   XmStringCreateLocalized(catgets(c->DT_catd, 1, 28, "Go To:"));

	/*
	 * remember - this returns a RowColumn widget!
	 */
	b->gotomenu = XmVaCreateSimpleOptionMenu(b->upper_form,
                "goToOptionMenu", goto_label, NULL, 0, gotomenu_cb,
                XmVaPUSHBUTTON, prev_week, NULL, NULL, NULL,
                XmVaPUSHBUTTON, this_week, NULL, NULL, NULL,
                XmVaPUSHBUTTON, next_week, NULL, NULL, NULL,
                XmVaPUSHBUTTON, prev_month, NULL, NULL, NULL,
                XmVaPUSHBUTTON, next_month, NULL, NULL, NULL,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNbottomOffset, 5,
                XmNleftAttachment, XmATTACH_FORM,
		XmNleftOffset, 20,
		XmNuserData, c,
		XmNnavigationType, XmTAB_GROUP,
                NULL);
	XmStringFree(prev_week);
	XmStringFree(this_week);
	XmStringFree(next_week);
	XmStringFree(prev_month);
	XmStringFree(next_month);
	XmStringFree(goto_label);

	text_field_form = XtVaCreateManagedWidget("text_field_form",
		xmFormWidgetClass, b->upper_form,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNbottomOffset, 5,
                XmNleftAttachment, XmATTACH_WIDGET,
                XmNleftWidget, b->gotomenu,
		NULL);

	b->datetext = XtVaCreateWidget("dateText",
		xmTextWidgetClass, text_field_form,
#ifndef AIX_ILS
		XmNcolumns, 10,
#endif /* AIX_ILS */
                NULL);
	XtAddCallback(b->datetext, XmNactivateCallback, goto_date_cb, 
								(XtPointer)c);

	/*
	 * We can now calc the proper offset for the bottom of scrolled
	 * list - allow for a small margin above and below the text field.
	 */
	XtVaGetValues(b->datetext, XmNheight, &height, NULL);
	XtVaSetValues(b->browse_list_sw, XmNbottomOffset, (height + 10), NULL);

	b->lower_form = XtVaCreateManagedWidget("lowerForm",
		xmFormWidgetClass, b->inner_pane,
		XmNallowResize, True,
		XmNpaneMinimum, 200,
		XmNtraversalOn, False,
		NULL);

	/*
	 * create drawing area for chart
	 */
        b->canvas = XtVaCreateManagedWidget("canvas", xmDrawingAreaWidgetClass,
                b->lower_form,
                XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNtraversalOn, False,
                NULL);
       	b->xcontext = gr_create_xcontext(c, b->canvas, gr_color,
		c->xcontext->app);

        XtVaSetValues(b->canvas, XmNheight, 300, NULL);

        XtAddCallback(b->canvas, XmNresizeCallback, mb_resize_proc, (XtPointer)c);
        XtAddCallback(b->canvas, XmNinputCallback, bcanvas_event, (XtPointer)c);
        XtAddCallback(b->canvas, XmNexposeCallback, bcanvas_repaint, (XtPointer)c);

	/*
	 * Create action area of the dialog
	 */
	b->action = XtVaCreateWidget("action",
		xmFormWidgetClass, b->outer_pane,
		NULL);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 29, "Schedule..."));
       	b->schedule = XtVaCreateWidget("schedule",
		xmPushButtonGadgetClass, b->action,
		XmNlabelString, xmstr,
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
                NULL);
	XmStringFree(xmstr);
	XtAddCallback(b->schedule, XmNactivateCallback, schedule_cb, (XtPointer)c);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 30, "Mail..."));
       	b->mail = XtVaCreateWidget("mail",
		xmPushButtonGadgetClass, b->action,
		XmNlabelString, xmstr,
		XmNleftAttachment, XmATTACH_WIDGET,
		XmNleftWidget, b->schedule,
		XmNtopAttachment, XmATTACH_FORM,
                NULL);
	XmStringFree(xmstr);
	XtAddCallback(b->mail, XmNactivateCallback, mail_cb, (XtPointer)c);

	XtSetSensitive(b->mail, c->tt_procid == NULL ? False : True);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 923, "Cancel"));
       	b->cancel = XtVaCreateWidget("cancel",
		xmPushButtonGadgetClass, b->action,
		XmNlabelString, xmstr,
		XmNleftAttachment, XmATTACH_WIDGET,
		XmNleftWidget, b->mail,
		XmNtopAttachment, XmATTACH_FORM,
                NULL);
	XmStringFree(xmstr);
	XtAddCallback(b->cancel, XmNactivateCallback, cancel_cb, (XtPointer)c);

        xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 77, "Help"));
        b->helpbutton = XtVaCreateWidget("help",
		xmPushButtonGadgetClass, b->action,
                XmNlabelString, xmstr,
		XmNleftAttachment, XmATTACH_WIDGET,
		XmNleftWidget, b->cancel,
		XmNtopAttachment, XmATTACH_FORM,
                NULL);
        XmStringFree(xmstr);
        XtAddCallback(b->helpbutton, XmNactivateCallback,
                (XtCallbackProc)help_cb, COMPARE_CALS_HELP_BUTTON);
        XtAddCallback(b->action, XmNhelpCallback,
                (XtCallbackProc)help_cb, (XtPointer) COMPARE_CALS_HELP_BUTTON);

	b->message_text = XtVaCreateWidget("message",
		xmLabelGadgetClass, b->action,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, b->schedule,
		XmNleftAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNalignment, XmALIGNMENT_BEGINNING,
		NULL);

	/*
	 * Fix the size of action area
	 */
	XtVaGetValues(b->schedule, XmNheight, &height, NULL);
	XtVaSetValues(b->action,
		XmNpaneMaximum, (2*height),
		XmNpaneMinimum, (2*height),
		NULL);


	XtVaGetValues(b->outer_pane,
		XmNchildren, &children,
		XmNnumChildren, &num_children,
		NULL);

	while (num_children-- > 0) {
		if (XmIsSash(children[num_children])) {
				XtVaSetValues(children[num_children], XmNtraversalOn, False, NULL);
		}
	}

	XtManageChild(b->edit_list);
	XtManageChild(b->list_label);
	XtManageChild(b->browse_list);
	XtManageChild(b->gotomenu);
	XtManageChild(b->datetext);
	XtManageChild(b->schedule);
	XtManageChild(b->mail);
	XtManageChild(b->cancel);
	XtManageChild(b->helpbutton);
	XtManageChild(b->message_text);
	XtManageChild(b->action);

	gr_init(b->xcontext, b->canvas);

        format_tick(b->date, ord_t, sep_t, buf);
	XmTextSetString(b->datetext, buf);

	set_entry_date(c);

        /*
	 * set default button for dialog
	 */
        XtVaSetValues(b->action, XmNdefaultButton, b->schedule, NULL);
        XtVaSetValues(b->upper_form, XmNdefaultButton, b->schedule, NULL);

	/* We don't want a ``return'' in the text field to trigger the
	 * default action so we create a form around the text field and
	 * designate the text-field as the default button.
	 */
        XtVaSetValues(text_field_form, XmNdefaultButton, b->datetext, NULL);

        XtVaSetValues(b->action, XmNcancelButton, b->cancel, NULL);
        XtVaSetValues(b->upper_form, XmNcancelButton, b->cancel, NULL);
        XmProcessTraversal(b->schedule, XmTRAVERSE_CURRENT);
        XtVaSetValues(b->action, XmNinitialFocus, b->schedule, NULL);

        XtVaSetValues(b->frame, XmNmappedWhenManaged, True, NULL);
	XtRealizeWidget(b->frame);
	/*
	 * Enforce a "reasonable" size.
	 * too narrow and the buttons are erased
	 * too short and the chart will look smashed
	XtVaSetValues(b->frame,
		XmNminWidth, 300,
		XmNminHeight, 600,
		NULL);
	 */

	/*
	* Add a WM protocol callback to handle the
	* case where the window manager closes the dialog.
	* Pass the calendar ptr through client data to allow
	* the callback to get at the shell and destroy it.
	*/
	wm_del_atom = XmInternAtom(XtDisplayOfObject(b->frame),
		"WM_DELETE_WINDOW", False);
	XmAddWMProtocolCallback(b->frame, wm_del_atom, cancel_cb, c);

	init_browser(c);
}

static void 
browselist_from_browser(Widget w, XtPointer client_data, XtPointer call_data)
{
        Calendar *c = (Calendar *)client_data;
        Browselist	*bl;

        bl = (Browselist *)c->browselist;

	show_browselist(c);
}

static void
goto_date_cb(Widget w, XtPointer client_data, XtPointer call_data) 
{
        Calendar *c = (Calendar *)client_data;
	Browser *b;

	set_entry_date(c);
}

static void
goto_unit(Calendar *c, int item_no)
{
        char buf[DATESIZ];
        Browser *b;
        Props *p;
        OrderingType    ord_t;
        SeparatorType   sep_t;

        b = (Browser*)c->browser;
        p = (Props*)c->properties;

	switch (item_no + 1) {
	case MB_PREVWEEK:
		b->date = last_ndays(b->date, 7);
		break;
	case MB_THISWEEK:
		b->date = now();
		break;
	case MB_NEXTWEEK:
		b->date = next_ndays(b->date, 7);
		break;
	case MB_NEXTMONTH:
		b->date = next_ndays(b->date, 28);
		break;
	case MB_PREVMONTH:
		b->date = last_ndays(b->date, 28);
		break;
	default:
		b->date = now();
		break;
	}

        reset_ticks(c, False);

        ord_t = get_int_prop(p, CP_DATEORDERING);
        sep_t = get_int_prop(p, CP_DATESEPARATOR);

        format_tick(b->date, ord_t, sep_t, buf);
	XmTextSetString(b->datetext, buf);
        br_display(c);
}

static void
bcanvas_repaint(Widget w, XtPointer client_data, XtPointer call_data)
{
        Calendar *c = (Calendar *)client_data;
	Browser *b;
	XRectangle clip;
	XEvent ev;
        new_XContext    *xc;
	XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct*)call_data;

	/* repair in full on final exposure (crude optimization scheme) */
	if (cbs->event->xexpose.count != 0)
	   return;

	b = (Browser *)c->browser;
        if ((xc = b->xcontext)==NULL)
                return;

	clip.x = 0;
	clip.y = 0;
	XtVaGetValues(w, XmNwidth, &clip.width, XmNy, &clip.height, NULL);

        gr_set_clip_rectangles(xc, 0, 0, &clip, 1, Unsorted);

        gr_clear_area(b->xcontext, 0, 0, b->canvas_w, b->canvas_h);
        gr_clear_clip_rectangles(xc);
	mb_refresh_canvas(b, c);
 
        XSync(XtDisplay(b->canvas), 0);
}

static void
mb_display_footermess(Browser *b, Calendar *c)
{
        int num_cals;
        char buf[BUFSIZ];

	XtVaGetValues(b->browse_list, XmNselectedItemCount, &num_cals, NULL);
        if (num_cals == 1)
                sprintf(buf, "%d %s", num_cals,
			catgets(c->DT_catd, 1, 31, "Calendar Displayed"));
        else
                sprintf(buf,  "%d %s", num_cals,
			catgets(c->DT_catd, 1, 32, "Calendars Displayed"));
	set_message(b->message_text, buf);
}

static void
browser_to_gaccess_list(Calendar *c) {
	int		i, pos_cnt, *pos_list = NULL;
	GEditor		*ge = (GEditor *)c->geditor;
	Browser		*b = (Browser *)c->browser;
	BlistData	*bd;
	Browselist	*bl = (Browselist *)c->browselist;

	/*
	 *  Add selected items to list box in group appt editor
	 */
	XmListGetSelectedPos(b->browse_list, &pos_list, &pos_cnt);
	for (i = 0; i < pos_cnt; i++) {
		if (bd = (BlistData *)CmDataListGetData(bl->blist_data,
							pos_list[i]))
#ifndef NOTHREADS
			/* If the thread exists then we don't want to
			 * add this calendar to the group editor until
			 * we have logged in.  After the thread exists
			 * there is code that will add the calendar to
			 * the group editor if it exists.
			 */
			if (!THREAD_EXISTS(bd->bd_live))
#endif /* NOTHREADS */
				add_to_gaccess_list(bd->name, bd->cal_handle,
						    bd->user_access, 
						    bd->version,
						    bd->set_charset, ge, True);
	}
	add_all_gappt(ge);
	if (pos_cnt > 0)
		XtFree((XtPointer)pos_list);
}

void mb_update_busystatus(Browser *b, Calendar *c)
{
	int			i, j, r_cnt;
	CSA_uint32		num_entries;
	char			buf[BUFSIZ + 5];
	XmString		xmstr;
        Browselist 		*bl = (Browselist *)c->browselist;
	time_t			start, stop;
	CSA_entry_handle	*entries = NULL;
	BlistData 		*bd;
	CSA_return_code		rstat;
	CSA_enum 		*ops;
	CSA_attribute 		*range_attrs;
	Tick			start_tick, end_tick;

	j = 1;
	while (bd = (BlistData *)CmDataListGetData(bl->blist_data, j)) {
		if (!XmListPosSelected(b->browse_list, j++))
			continue;

		sprintf(buf, "  %s", bd->name);
		start = b->begin_day_tick;
		stop = b->end_hr_tick;

		if (bd->cache == NULL) {
			setup_range(&range_attrs, &ops, &r_cnt, start, stop, 
					CSA_TYPE_EVENT, NULL, _B_FALSE, bd->version);
	        	rstat = csa_list_entries(bd->cal_handle, r_cnt, range_attrs, ops, &num_entries, &entries, NULL);
        			free_range(&range_attrs, &ops, r_cnt);
        		backend_err_msg(b->frame, bd->name, rstat,
                        		((Props_pu *)c->properties_pu)->xm_error_pixmap);
        		if (rstat != CSA_SUCCESS) {
				csa_free(entries);
                		return;
			}

			allocate_paint_cache(entries, num_entries, &bd->cache);
			bd->cache_size = num_entries;
			csa_free(entries);
		}

        	for (i = 0; i < bd->cache_size; i++) {

			start_tick = bd->cache[i].start_time;
			end_tick = bd->cache[i].end_time;
			if ((start_tick+1 <= b->end_hr_tick) && 
		    	    (end_tick-1 
				>= b->begin_hr_tick)) {
				buf[0] = '*';
				break;
			}

		} /* end for */

		xmstr = XmStringCreateLocalized(buf);
		XmListDeletePos(b->browse_list, j-1);
		XmListAddItem(b->browse_list, xmstr, j-1);
		XmListSelectPos(b->browse_list, j-1, False);
		XmStringFree(xmstr);
	} /* end while */
}

static void
bcanvas_event(Widget w, XtPointer client_data, XtPointer call_data)
{
        Calendar *c = (Calendar *)client_data;
	Browser *b;
	XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct*) call_data;
        XEvent  *event = cbs->event;
	Tick start;
        static XEvent lastevent;
	int x, y, boxw, boxh;
	pr_pos xy;

        if ((event->type == ButtonRelease) || (event == NULL))
                return;

	b = (Browser*)c->browser;

        x       = event->xbutton.x;
        y       = event->xbutton.y;

	switch(event->type) {
	case ButtonPress:
                if (x > b->chart_x && y > b->chart_y &&
                        x < (b->chart_x + b->chart_width)
                        && y < (b->chart_y + b->chart_height))
                {
                        browser_deselect(c, b);
                        b->col_sel = (x - b->chart_x) / b->boxw;
                        b->row_sel = (y - b->chart_y) / b->boxh;
                        xy.x = b->col_sel;
                        xy.y = b->row_sel;

			/*
			 *  Don't bring up multi-browser for an invalid date
			 */
			if ((last_ndays(b->date, 2) <= get_bot()) &&
			    (b->col_sel < 3))
				return;
			else if ((next_ndays(b->date, 1) > get_eot()) &&
				 (b->col_sel > 3))
				return;
 
                        reset_ticks(c, True);
                        browser_select(c, b, &xy);
                        if (ds_is_double_click(&lastevent, event)) {
				_DtTurnOnHourGlass(b->frame);
				show_geditor(c, b->begin_hr_tick,
					     b->end_hr_tick);
				browser_to_gaccess_list(c);
				_DtTurnOffHourGlass(b->frame);
                        }
			/* add busyicon */
			mb_update_busystatus(b, c);
		}
                c->general->last_canvas_touched = browser;
        };
        lastevent = *event;
}

static void
schedule_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
        Calendar *c = (Calendar *)client_data;
	Browser *b;
	Props *p;
        OrderingType    ord_t;
        SeparatorType   sep_t;
	XmPushButtonCallbackStruct *cbs = (XmPushButtonCallbackStruct*) call_data;

	b = (Browser *)c->browser;
	p = (Props *)c->properties;
	_DtTurnOnHourGlass(b->frame);
        ord_t = get_int_prop(p, CP_DATEORDERING);
        sep_t = get_int_prop(p, CP_DATESEPARATOR);
	show_geditor(c, b->begin_hr_tick, b->end_hr_tick);
	browser_to_gaccess_list(c);
	_DtTurnOffHourGlass(b->frame);
}


#ifdef FNS
/* 
 * Since mail has not been modified to use FNS yet, it
 * is safest to use the user@host as the mailing address rather
 * than the calendar's FNS name.
 */
#endif /* FNS */

static char *
get_mail_address_list(Calendar *c) {
	int		i, *pos_list = NULL, pos_cnt=0;
        Browser		*b = (Browser *)c->browser;
	BlistData	*bd;
	Browselist	*bl = (Browselist *)c->browselist;
	int		address_len;
	char		*address;
        char            **addrs = NULL;
#ifdef FNS
	char buf[MAXNAMELEN];
#endif /* FNS */

	XmListGetSelectedPos(b->browse_list, &pos_list, &pos_cnt);
        addrs = (char **)calloc(pos_cnt, sizeof(char *));
	for (i = 0, address_len = 0; i < pos_cnt; i++) {
		bd = (BlistData *)CmDataListGetData(bl->blist_data,
						    pos_list[i]);
		if (bd) {
#ifdef FNS
			if (cmfns_use_fns((Props *)c->properties) &&
			    cmfns_lookup_calendar(bd->name, buf, sizeof(buf)) > 0)
				addrs[i] = cm_strdup(buf);
			else
#endif /* FNS */
				addrs[i] = cm_strdup(bd->name);

			address_len += strlen(addrs[i]) + 1;
		}
	}

	address = calloc(address_len+1, 1);
	address[0] = NULL;

	for (i = 0; i < pos_cnt; i++) {
		if (addrs[i]) {
			strcat(address, addrs[i]);
			if (i < (pos_cnt-1))
                          strcat(address, ",");
			free(addrs[i]);
		}
	}
	free(addrs);

	if (pos_list)
		XtFree((XtPointer)pos_list);

	return(address);
}

static Tt_message
reply_cb(Tt_message m, void *c_data, Tttk_op op, unsigned char *contents, int len, char *file)
{
        char *client_procID = tt_message_handler(m);
        if ( debug && (client_procID != NULL) ) {
                fprintf(stderr, "DEBUG: reply_cb():client_procID = %s\n", client_procID);
                fprintf(stderr, "DEBUG: reply_cb():message_op = %s\n", tt_message_op(m));
        }
        return(m);
}

static void
mail_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
        Calendar 	*c = (Calendar *)client_data;
	Props		*p = (Props *) c->properties;
        Browser		*b = (Browser *)c->browser;
        Tt_message      msg;
        Tt_status       status;
        char            *appointment_buf;
	Dtcm_appointment        *appt;
	char		*address = get_mail_address_list(c);
	char		*address_list[1];
	char		*mime_buf;
 
        /* Send ToolTalk message to bring up compose GUI with buffer as attachme
nt */

	appt = allocate_appt_struct(appt_write, DATAVER_ARCHIVE, NULL);
	load_appt_defaults(appt, p);

	appt->time->value->item.date_time_value = malloc(BUFSIZ);
	_csa_tick_to_iso8601(b->begin_hr_tick, appt->time->value->item.date_time_value);
	appt->end_time->value->item.date_time_value = malloc(BUFSIZ);
	_csa_tick_to_iso8601(b->end_hr_tick, appt->end_time->value->item.date_time_value);

	/* set up the start time from the dialog */

	scrub_attr_list(appt);
 
        appointment_buf = parse_attrs_to_string(appt, p, attrs_to_string(appt->attrs, appt->count));

	free_appt_struct(&appt);

	address_list[0] = appointment_buf;
 
        mime_buf = create_rfc_message(address, "message", address_list, 1);

        msg = ttmedia_load(0, (Ttmedia_load_msg_cb)reply_cb, NULL, TTME_MAIL_EDIT, "RFC_822_MESSAGE", (unsigned char *)mime_buf, strlen(mime_buf), NULL, "dtcm_appointment_attachment", 0);
 
        status = tt_message_send(msg);

	free(appointment_buf);
	free(mime_buf);
	free(address);
}

static void
gotomenu_cb(Widget w, XtPointer data, XtPointer cbs) 
{
	int	item_no = (int) data;
	/* should really be getting this from the widget */
	Calendar *c = calendar;

	goto_unit(c, item_no);
}

extern void
mb_update_segs(Browser *b, Tick tick, Tick dur, int *start_index, int *end_index)
{
        int     num_segs, i, start, start_hour, duration, nday;
        Props *p;
 
        p = (Props*)calendar->properties;
 
        start_hour = hour(tick);
        
        if (start_hour >= get_int_prop(p, CP_DAYEND)) {
                *start_index = -1;
                *end_index = -1;
                return;
        }
 
        if (start_hour < get_int_prop(p, CP_DAYBEGIN)) {
                start = 0;
                duration = dur - ((double)(get_int_prop(p, CP_DAYBEGIN) -
                 ((double)start_hour + (double)minute(tick)/(double)60))
                        * (double)hrsec);
        } else{
                start = ((double)(start_hour - get_int_prop(p, CP_DAYBEGIN)) *
                        (double)60 + (double)minute(tick));
                duration = dur;
        }
 
        if (duration <= 0) {
                *start_index = -1;
                *end_index = -1;
                return;
        }
        nday = (nday=dow(tick))==0? 6: nday-1;
        num_segs = (double)start / (double)MINS_IN_SEG;
        *start_index = (double)start / (double)MINS_IN_SEG + (nday * (b->segs_in_array/7));
        if (start - (num_segs * MINS_IN_SEG) > 7)
                (*start_index)++;
        num_segs = ((double)duration / (double)60 / (double)MINS_IN_SEG);
        *end_index = num_segs + *start_index;
        if (((double)duration/(double)60-MINS_IN_SEG*num_segs) > 7)
                (*end_index)++;
 
        if (*end_index > (i = ((nday + 1) * (b->segs_in_array / 7))) )
                *end_index = i;
 
        for (i = *start_index; i < *end_index; i++)
                if (b->add_to_array)
                        b->multi_array[i]++;
                else if (b->multi_array[i] > 0)
                        b->multi_array[i]--;
}

void
mb_update_array(char *entry_text, Calendar *c)
{
	int			start_ind, end_ind, i, r_cnt;
	time_t			start, stop;
        Browser			*b = (Browser *)c->browser;
        CSA_entry_handle	*entries = NULL;
	BlistData		*bd;
        CSA_return_code		rstat;
        Browselist		*bl = (Browselist *)c->browselist;
	CSA_enum		*ops;
	CSA_attribute		*range_attrs;
	CSA_uint32 		num_entries;
	Tick			start_tick, end_tick;

	/*
	 *  Search for the entry text in our list of calendar handles
	 */
	i = 1;
	while ((bd = (BlistData *)CmDataListGetData(bl->blist_data, i++))
	       && strcmp(bd->name, entry_text) != 0);
	if (!bd)
		return;

        start = b->begin_week_tick;
        stop = next_ndays(b->begin_week_tick, 7) - 1;

	if (bd->cache == NULL) {
		setup_range(&range_attrs, &ops, &r_cnt, start, 
		    	stop, CSA_TYPE_EVENT,
		    	NULL, _B_FALSE, bd->version);
        	rstat = csa_list_entries(bd->cal_handle, r_cnt, range_attrs, 
					ops, &num_entries, &entries, NULL);
		free_range(&range_attrs, &ops, r_cnt);
       
                if ((b->add_to_array == 0 ) && (rstat == CSA_E_INVALID_SESSION_HANDLE)) rstat = 0;   

        	backend_err_msg(b->frame, bd->name, rstat,
				((Props_pu *)c->properties_pu)->xm_error_pixmap);
		if (rstat != CSA_SUCCESS)
                	return;

		allocate_paint_cache(entries, num_entries, &bd->cache);
		bd->cache_size = num_entries;
		csa_free(entries);

	}

        for (i = 0; i < bd->cache_size; i++) {

		start_tick = bd->cache[i].start_time;
		end_tick = bd->cache[i].end_time;

                mb_update_segs(b, start_tick,
			       end_tick - start_tick, &start_ind,
			       &end_ind);
        }
}

#ifdef FNS
static int valid_cm_addr_format(const char *str)
{
	return (strchr(str, '@') != 0);
}

static Boolean
valid_cal_addr(Calendar *c, const char *target_addr)
{
	if (!cmfns_use_fns((Props *)c->properties) ||
	    valid_cm_addr_format(target_addr))
		return True;	/* Calendar address already in valid format */
	else
		return False;
}
#endif /* FNS */

static Boolean
register_names(char *target, Calendar *c)
{
	int		i;
	Props_pu	*p = (Props_pu *)c->properties_pu;
	Browser		*b = (Browser*)c->browser;
	BlistData	*bd = NULL;
        Browselist	*bl = (Browselist *)c->browselist;
        CSA_return_code	rstat;
	CSA_session_handle	cal = NULL;
	CSA_flags		user_access;
	CSA_calendar_user	csa_user;
	int			version;
	Boolean			set_charset;
	CSA_flags		flags = NULL;
	CSA_extension   	cb_ext;
	CSA_extension		logon_ext[2];

	if (blank_buf(target))
		return False;

	i = 1;
	while ((bd = (BlistData *)CmDataListGetData(bl->blist_data, i++))
	       && strcmp(bd->name, target) != 0);
	if (!bd) {
	        char *title = XtNewString(catgets(c->DT_catd, 1, 1070,
				"Calendar : Error - Compare Calendars"));
		char *text = XtNewString(catgets(c->DT_catd, 1, 607,
				"Internal error registering calendar name."));
		char *ident = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(b->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident,
			DIALOG_IMAGE, p->xm_error_pixmap,
			NULL);
		XtFree(ident);
		XtFree(text);
		XtFree(title);
		return False;
	}
	if (bd->cal_handle)
		return True;

	if (strcmp(c->caladdr, target) == 0 ||
	    strcmp(c->calname, target) == 0) {
		cal = c->my_cal_handle;
		user_access = c->my_access;
		version = c->my_cal_version;
		set_charset = c->my_set_charset;
        } else if (strcmp(c->view->current_calendar, target) == 0 ||
		   strcmp(c->view->current_calname, target)) {
		cal = c->cal_handle;
		user_access = c->user_access;
		version = c->general->version;
		set_charset = c->set_charset;
	} else {
		csa_user.calendar_address = target;

		csa_user.calendar_user_extensions = NULL;
		csa_user.user_name = csa_user.calendar_address;
		csa_user.user_type = 0;

		/* get user acess */
		logon_ext[0].item_code = CSA_X_DT_GET_USER_ACCESS_EXT;
		logon_ext[0].item_data = 0;
		logon_ext[0].item_reference = NULL;
		logon_ext[0].extension_flags = 0;

		/* get calendar's character set attribute */
		logon_ext[1].item_code = CSA_X_DT_GET_CAL_CHARSET_EXT;
		logon_ext[1].item_data = 0;
		logon_ext[1].item_reference = NULL;
		logon_ext[1].extension_flags = CSA_EXT_LAST_ELEMENT;

		rstat = csa_logon(NULL, &csa_user, NULL, NULL, NULL, &cal,
			logon_ext);
 
        	if (rstat != CSA_SUCCESS) {
#ifdef FNS
			/* retrofit error to be more meaningful for FNS case */
			if (rstat == CSA_E_INVALID_PARAMETER &&
			    !valid_cal_addr(c, target))
				rstat = CMFNS_E_NAME_NOT_BOUND;
#endif /* FNS */
			backend_err_msg(b->frame, target, rstat,
			       ((Props_pu *)c->properties_pu)->xm_error_pixmap);
			return False;
        	}

		user_access = logon_ext[0].item_data;
		version = get_data_version(cal);
		mark_set_entry_charset(c->app_locale,
				(char *)logon_ext[1].item_reference,
				version, &set_charset);
		csa_free(logon_ext[1].item_reference);
	}

	/* register for activity notification */
	flags = CSA_CB_ENTRY_ADDED | CSA_CB_ENTRY_UPDATED |
		CSA_CB_ENTRY_DELETED;
	cb_ext.item_code = CSA_X_XT_APP_CONTEXT_EXT;
	cb_ext.item_data = (CSA_uint32)c->xcontext->app;
	cb_ext.extension_flags = CSA_EXT_LAST_ELEMENT;

	rstat = csa_register_callback(cal, flags, mb_update_handler, NULL, &cb_ext);
	if (rstat != CSA_SUCCESS) {
		backend_err_msg(b->frame, target, rstat,
			((Props_pu *)c->properties_pu)->xm_error_pixmap);
		return False;
	}

	bd->cal_handle = cal;
	bd->user_access = user_access;
	bd->version = version;
	bd->set_charset = set_charset;

        return True;
}

extern void
mb_deregister_names(char *name, Calendar *c)
{
	int		i;
	Browser		*b = (Browser *)c->browser;
	Props_pu	*p = (Props_pu *)c->properties_pu;
	BlistData	*bd;
	Browselist	*bl = (Browselist *)c->browselist;
        CSA_return_code	rstat;

	i = 1;
	while ((bd = (BlistData *)CmDataListGetData(bl->blist_data, i))
	       && strcmp(bd->name, name) != 0)
		++i;
	if (!bd)
		return;

	destroy_paint_cache(bd->cache, bd->cache_size);
	bd->cache = NULL;
	bd->cache_size = 0;

	if (bd->cal_handle != c->my_cal_handle &&
	    bd->cal_handle != c->cal_handle && bd->cal_handle) {
		rstat = csa_logoff(bd->cal_handle, NULL);

                if ((b->add_to_array == 0 ) && (rstat == CSA_E_INVALID_SESSION_HANDLE)) rstat = 0;

		if (rstat != CSA_SUCCESS) {
			backend_err_msg(b->frame, bd->name, rstat,
					p->xm_error_pixmap);
		}
		bd->cal_handle = NULL;
		blist_clean(bl, False);
        }
}

#ifndef NOTHREADS
/*
 * Our threads model is to launch a thread to do the logon portion of the
 * browse operation.  After the thread has performed the csa_logon() it
 * sends the name of the calendar that was logged into via pipe back to
 * the main thread.  The main thread then pulls over the data for that
 * calendar.  In order to avoid launching several threads if the user quickly
 * clicks on the same calendar address multiple times we check to see
 * if a thread exists for that calendar before we launch a new thread
 * to logon onto the calendar.  Additionally after the logon has happen
 * but before the threads sends the information back on the pipe it checks
 * to see if the information is still wanted.  If it is not, then the
 * thread performs a csa_logoff() and then exists without sending any
 * data back to the main thread.
 */

static Boolean
last_thread(
	Browselist	*bl)
{
	BlistData	*bd;
	int		 i;

	/* Determine if there are other threads still running.  We know
	 * thread is running if bd_live is set.
	 */

	i = 1;
	while (bd = (BlistData *)CmDataListGetData(bl->blist_data, i++)) {
		if (bd->bd_live)
			return False;
	}
	return True;
}

/* This takes a browselist and a boolean.  The boolean instructs the
 * code as to whether it should enable the menu editor only when the
 * last thread exists or if it should enable the menu editor right now.
 */
static void enable_menu_editor(
	Browselist	*bl,
	Boolean		 now)
{
	/* Reenable the Menu Editor if the multibrowser does not have any
	 * pending threads.
	 */
	if (bl->frame) {
		if (now || last_thread(bl)) {
			XmTextFieldSetEditable(bl->username, True);
			XtSetSensitive(bl->browse_list, True);
			/*
			set_message(bl->message, 
				   catgets(calendar->DT_catd, 1, -1,
				   	"Editor has been enabled."));
			*/
			set_message(bl->message, " "); 
			if (bl->bl_turn_ok_on) {
				XtSetSensitive(bl->ok_button, True);
				XtSetSensitive(bl->apply_button, True);
				XtSetSensitive(bl->reset_button, True);
				XtVaSetValues(bl->form, 
					XmNdefaultButton, bl->apply_button, 
					NULL);
			}

			if (bl->bl_turn_remove_on)
				XtSetSensitive(bl->remove_button, True);

			if (bl->bl_turn_add_on)
				XtSetSensitive(bl->add_button, True);
			bl->bl_suspended = False;
		}
	} else {
		if (now || last_thread(bl))
			bl->bl_suspended = False;
	}
}

extern void disable_menu_editor(
	Browselist	*bl,
	Boolean		 now)
{
	if (bl->frame && (!bl->bl_suspended || now)) {
		bl->bl_suspended = True;
		set_message(bl->message, catgets(calendar->DT_catd, 1, 1093,
			   "Menu Editor suspended during browse operation."));
		XmTextFieldSetEditable(bl->username, False);
		XtSetSensitive(bl->browse_list, False);
		if (XtIsSensitive(bl->ok_button)) {
			XtSetSensitive(bl->ok_button, False);
			XtSetSensitive(bl->apply_button, False);
			XtSetSensitive(bl->reset_button, False);
			XtVaSetValues(bl->form, 
				XmNdefaultButton, bl->cancel_button, NULL);
			bl->bl_turn_ok_on = True;
		} else
			bl->bl_turn_ok_on = False;

		if (XtIsSensitive(bl->remove_button)) {
			XtSetSensitive(bl->remove_button, False);
			bl->bl_turn_remove_on = True;
		} else
			bl->bl_turn_remove_on = False;

		if (XtIsSensitive(bl->add_button)) {
			XtSetSensitive(bl->add_button, False);
			bl->bl_turn_add_on = True;
		} else
			bl->bl_turn_add_on = False;

	} else if (!bl->frame) {
		bl->bl_turn_ok_on = False;
		bl->bl_turn_remove_on = False;
		bl->bl_turn_add_on = False;
	}
	bl->bl_suspended = True;
}

static void
mb_box_notify(Widget widget, XtPointer client_data, XtPointer call_data)
{
	XmListCallbackStruct *cbs = (XmListCallbackStruct *)call_data;
        Calendar *c = (Calendar *)client_data;
	int		i;
	char		*addr;
        Browser       	*b;
        GEditor        	*e;
	BlistData	*bd;
	Browselist	*bl;
	XmString 	xmstr;
	char		name[BUFSIZ+5],
			buf[MAXNAMELEN];
#ifdef FNS
	int		rcode;
	char		*fns_name;
#endif

        b = (Browser*)c->browser;
        bl = (Browselist*)c->browselist;
        e = (GEditor*)c->geditor;

	i = 0;
	while (i < cbs->selected_item_count &&
	       cbs->item_position != cbs->selected_item_positions[i])
		++i;
	b->add_to_array = (i < cbs->selected_item_count) ? True : False;

	if ((bd = (BlistData *)CmDataListGetData(bl->blist_data,
						   cbs->item_position)) == NULL)
		return;

	/* erase busy status if it was busy because it was deselected */
	if (!XmListPosSelected(b->browse_list, cbs->item_position)) {
		sprintf(name, "  %s", bd->name);
		xmstr = XmStringCreateLocalized(name);
		XmListDeletePos(b->browse_list, cbs->item_position);
		XmListAddItem(b->browse_list, xmstr, cbs->item_position);
		XmStringFree(xmstr);
	} 	

#ifdef FNS
	rcode = -1;
	if (cmfns_use_fns((Props *)c->properties)) {
		/* Yes!  Try to use it */
		rcode = cmfns_lookup_calendar(bd->name, buf, sizeof(buf));
	}

	if (rcode > 0)
		addr = buf;
	else 
#endif
		addr = bd->name;

	bd->bd_selected = XmListPosSelected(b->browse_list, cbs->item_position);
	if (bd->bd_selected)
		disable_menu_editor(bl, False);

	cm_mutex_lock(&bl->bl_data_op_lock);

	if (!THREAD_EXISTS(bd->bd_live) &&
	    bd->bd_selected) {
		BrowselistThreadData	*tdata;
		int			 pfd[2];

		if (!bl->bl_timer) {
			bl->bl_timer = XtAppAddTimeOut(c->xcontext->app, 1000, 
					               mb_timer_cb, 
						       (XtPointer)bl);
		}

		if (pipe(pfd) < 0) {
			sprintf(buf, catgets(c->DT_catd, 1, 1094,
				"Login failed: internal error - bad pipe."));
			set_message(c->message_text, buf);
			return;
		}

		tdata = (BrowselistThreadData *)
				malloc(sizeof(BrowselistThreadData));
		tdata->btd_blist_item = bd;
		tdata->btd_blist = bl;
		tdata->btd_calendar = c; 
		tdata->btd_pfd[0] = pfd[0];
		tdata->btd_pfd[1] = pfd[1];
		bd->bd_in_pfd = pfd[0];
		bd->bd_stat = CSA_SUCCESS;

		bd->bd_id = XtAppAddInput(c->xcontext->app, pfd[0],
                                        (XtPointer)XtInputReadMask,
                                        (XtInputCallbackProc)
							mb_logon_thread_exit,
                                        (XtPointer)bd);
		bd->bd_live = True;
                (void)cm_thread_create((void*(*)(void*))mb_logon_thread,
					(void *)tdata);
        } else if ((!THREAD_EXISTS(bd->bd_live)) && 
                   (!bd->bd_selected)) {
                /*
		 * Must update the array before we deregister names because we
		 * close the calendar handle when we deregister.
		 */
		mb_update_array(bd->name, c);
		mb_deregister_names(bd->name, c);
		if (geditor_showing(e))
			remove_from_gaccess_list(addr, e);
		mb_refresh_canvas(b, c);
        } else if ((THREAD_EXISTS(bd->bd_live)) && 
                   (!bd->bd_selected)) {
		if (bd->bd_id) {
                	XtRemoveInput(bd->bd_id);
			bd->bd_id = NULL;
		}
		bd->bd_live = NULL;
		enable_menu_editor(bl, False);
		mb_refresh_canvas(b, c);
        } else if ((THREAD_EXISTS(bd->bd_live)) && 
                   (bd->bd_selected)) {
                bd->bd_id = XtAppAddInput(c->xcontext->app, bd->bd_in_pfd,
				(XtPointer)XtInputReadMask,
				(XtInputCallbackProc)mb_logon_thread_exit,
				(XtPointer)bd);
		if (!bl->bl_timer) {
			bl->bl_timer = XtAppAddTimeOut(c->xcontext->app, 1000, 
					               mb_timer_cb, 
						       (XtPointer)bl);
		}
        }

        cm_mutex_unlock(&bl->bl_data_op_lock);
}

/*
void bd_dump(
	Browselist	*bl)
{
	int 		 i = 0;
	BlistData	*bd;

	while (bd = (BlistData *)CmDataListGetData(bl->blist_data, i++)) {
		fprintf(stderr,"# %d:\n", i);
		fprintf (stderr, "\tname:\t%s\n", bd->name);
		fprintf (stderr, "\tcal:\t%d\n", bd->cal_handle);
		fprintf (stderr, "\ttag:\t%d\n", bd->tag);
		fprintf (stderr, "\ttid:\t%d\n", bd->bd_live);
		fprintf (stderr, "\tselected:\t%d\n\n", bd->bd_selected);
	}
}
*/

static void *mb_logon_thread(
	BrowselistThreadData	*tdata)
{
	BlistData		*bd = tdata->btd_blist_item;
	CSA_return_code          rstat = CSA_SUCCESS;
#ifdef FNS
	char		buf[MAXNAMELEN];
	/* should really be getting this from the widget */
	Calendar *c = calendar;
#endif

	/* Do logon stuff */	
	cm_mutex_lock(&tdata->btd_blist->bl_data_op_lock);
	if ((!bd->cal_handle) &&
	    (strcmp(tdata->btd_calendar->calname, bd->name) == 0) ||
	    (strcmp(tdata->btd_calendar->caladdr, bd->name) == 0)) {
		/* XXX: c->calname could change! */
		bd->cal_handle = tdata->btd_calendar->my_cal_handle;
		bd->user_access = tdata->btd_calendar->my_access;
		bd->version = tdata->btd_calendar->my_cal_version;
		bd->set_charset = tdata->btd_calendar->my_set_charset;
	} else if ((!bd->cal_handle) &&
		   (strcmp(tdata->btd_calendar->view->current_calendar, 
							bd->name) == 0) ||
		   (strcmp(tdata->btd_calendar->view->current_calname, 
							bd->name) == 0)) {
		bd->cal_handle = tdata->btd_calendar->cal_handle;
		bd->user_access = tdata->btd_calendar->user_access;
		bd->version = tdata->btd_calendar->general->version;
		bd->set_charset = tdata->btd_calendar->set_charset;
	} else if (!bd->cal_handle) {
		CSA_calendar_user	 csa_user;
		CSA_extension		 logon_ext[2];

        	cm_mutex_unlock(&tdata->btd_blist->bl_data_op_lock);

#ifdef FNS
		if (cmfns_use_fns((Props *)c->properties) &&
		    cmfns_lookup_calendar(bd->name, buf, sizeof(buf)) > 0)
			csa_user.calendar_address = buf;
		else
#endif /* FNS */
		csa_user.calendar_address = bd->name;

		csa_user.calendar_user_extensions = NULL;
		csa_user.user_name = csa_user.calendar_address;
		csa_user.user_type = 0;

		/* get user acess */
		logon_ext[0].item_code = CSA_X_DT_GET_USER_ACCESS_EXT;
		logon_ext[0].item_data = 0;
		logon_ext[0].item_reference = NULL;
		logon_ext[0].extension_flags = 0;

		/* get calendar's character set attribute */
		logon_ext[1].item_code = CSA_X_DT_GET_CAL_CHARSET_EXT;
		logon_ext[1].item_data = 0;
		logon_ext[1].item_reference = NULL;
		logon_ext[1].extension_flags = CSA_EXT_LAST_ELEMENT;

		rstat = csa_logon(NULL, &csa_user, NULL, NULL, NULL, 
						&bd->cal_handle, logon_ext);

		cm_mutex_lock(&tdata->btd_blist->bl_data_op_lock);

#ifdef FNS 
		/* Retrofit error if required */
		if (rstat == CSA_E_INVALID_PARAMETER &&
		    !valid_cal_addr(c, csa_user.calendar_address))
			rstat = CMFNS_E_NAME_NOT_BOUND;
#endif /* FNS */

		bd->bd_stat = rstat;
		if (rstat == CSA_SUCCESS) {
			bd->user_access = logon_ext[0].item_data;
			bd->version = get_data_version(bd->cal_handle);
			mark_set_entry_charset(calendar->app_locale,
				(char *)logon_ext[1].item_reference,
				bd->version, &bd->set_charset);
			csa_free(logon_ext[1].item_reference);
		}
	}

	if (bd->bd_selected) {
		char	buf[MAXPATHLEN];

		strcpy(buf, bd->name);
		write(tdata->btd_pfd[1], (void *)buf, strlen(buf));
		close (tdata->btd_pfd[1]);
	} else {
		/* The main thread doesn't want the data anymore */
		close (tdata->btd_pfd[0]);
		close (tdata->btd_pfd[1]);
		if (rstat == CSA_SUCCESS)
			csa_logoff(bd->cal_handle, NULL);
		bd->cal_handle = NULL;
	}

	/* Clear the thread id so THREAD_EXISTS() works */
	bd->bd_live = NULL;

        cm_mutex_unlock(&tdata->btd_blist->bl_data_op_lock);
	free(tdata);

	cm_thread_exit((void *)0);
	return(0);
}

static void mb_logon_thread_exit(
        BlistData       *bd,
	int		*source,
	XtInputId	*id)
{
	char		 cal_name[MAXPATHLEN];
	Browser		*b = (Browser *)calendar->browser;
	Browselist	*bl;
	CSA_flags	 flags = NULL;
	CSA_extension	 cb_ext;
	GEditor		*e = (GEditor *)calendar->geditor;

        bl = (Browselist*)calendar->browselist;

	/* Read the data from the thread to find out which calendar was
	 * logged into.
	 */
	memset(cal_name, '\0', MAXNAMELEN);
	if (read(*source, cal_name, MAXPATHLEN) == 0) {
		XtRemoveInput(*id);
		close(*source);
		return;
	}

	XtRemoveInput(*id);
	close(*source);

	/* The browser window has been cancled since this thread was started.
	 * So we just return here.
	 */
	if (!calendar->browser) {
		if (bd->bd_stat != CSA_SUCCESS)
			bd->cal_handle = NULL;
		return;
	}

	/* Reenable the Menu Editor if the multibrowser does not have any
	 * pending threads.
	 */
	enable_menu_editor(bl, False);

	if (bd->bd_stat == CSA_SUCCESS) {
		/* register for activity notification */
		flags = CSA_CB_ENTRY_ADDED | CSA_CB_ENTRY_UPDATED |
			CSA_CB_ENTRY_DELETED;
		cb_ext.item_code = CSA_X_XT_APP_CONTEXT_EXT;
		cb_ext.item_data = (CSA_uint32)calendar->xcontext->app;
		cb_ext.extension_flags = CSA_EXT_LAST_ELEMENT;
 
		/* even csa_register_callback failed, we still
		 * want to display the calendar
		 */
		csa_register_callback(bd->cal_handle, flags, 
				     mb_update_handler, NULL, &cb_ext);
	} else {
		XmString	xmstr;
		char		buf[MAXPATHLEN];

                sprintf(buf, "  %s", cal_name);
		xmstr = XmStringCreateLocalized(buf);
		XmListDeselectItem(b->browse_list, xmstr);
		XmStringFree(xmstr);
		backend_err_msg(b->frame, cal_name, bd->bd_stat,
			       ((Props_pu *)
				     calendar->properties_pu)->xm_error_pixmap);
		return;
	}

	if (geditor_showing(e)) {
		add_to_gaccess_list(cal_name, bd->cal_handle, bd->user_access,
				    bd->version, bd->set_charset, e, True);
		add_all_gappt(e);
	}
	_DtTurnOnHourGlass(calendar->frame);
	_DtTurnOnHourGlass(b->frame);
	mb_update_array(cal_name, calendar);
	mb_refresh_canvas(b ,calendar);
	_DtTurnOffHourGlass(b->frame);
	_DtTurnOffHourGlass(calendar->frame);
}

static void mb_timer_cb(
	XtPointer	 client_data,
	XtIntervalId	*id)
{
        Browselist      *bl = (Browselist *)client_data;
	BlistData	*bd;
	Browser		*b;
	static int	 position = 1;
	int		 looped = position,
			 loop = False;

        b = (Browser*)calendar->browser;

	/* The Multibrowser has been taken down - do nothing. */
	if (!b || !b->frame) return;

	bl->bl_timer = NULL;

	while (!loop) {
		if ((bd = (BlistData *)CmDataListGetData(bl->blist_data, 
						    position)) != NULL) {
			position++;
			if (bd->bd_live && bd->bd_selected) {
				char	buf[256];

				sprintf(buf, "Retrieving calendar: %s", 
					bd->name);
				set_message(b->message_text, buf);
				bl->bl_timer = 
					XtAppAddTimeOut(calendar->xcontext->app,
							1000, mb_timer_cb, 
							(XtPointer)bl);
				return;
			}
		} else {
			/* No data so we must be at the end of the list.
			 * Start over at the beginning.
			 */
			position = 1;
		}
		if (looped == position)
			loop = True;
	}
}
	

#else /* NOTHREADS */

static void
mb_box_notify(Widget widget, XtPointer client_data, XtPointer call_data)
{
	XmListCallbackStruct *cbs = (XmListCallbackStruct *)call_data;
        Calendar *c = (Calendar *)client_data;
	int		i;
	char		*addr;
        Browser       	*b;
        GEditor        	*e;
	BlistData	*bd;
	Browselist	*bl;
	XmString 	xmstr;
	char		name[BUFSIZ+5];
#ifdef FNS
	int		rcode;
	char		*fns_name, buf[MAXNAMELEN];
#endif

        b = (Browser*)c->browser;
        bl = (Browselist*)c->browselist;
        e = (GEditor*)c->geditor;

	i = 0;
	while (i < cbs->selected_item_count &&
	       cbs->item_position != cbs->selected_item_positions[i])
		++i;
	b->add_to_array = (i < cbs->selected_item_count) ? True : False;

	if ((bd = (BlistData *)CmDataListGetData(bl->blist_data,
						   cbs->item_position)) == NULL)
		return;
	/* erase busy status if it was busy because it was deselected */
	if (!XmListPosSelected(b->browse_list, cbs->item_position)) {
		sprintf(name, "  %s", bd->name);
		xmstr = XmStringCreateLocalized(name);
		XmListDeletePos(b->browse_list, cbs->item_position);
		XmListAddItem(b->browse_list, xmstr, cbs->item_position);
		XmStringFree(xmstr);
	} 	

#ifdef FNS
	rcode = -1;
	if (cmfns_use_fns((Props *)c->properties)) {
		/* Yes!  Try to use it */
		rcode = cmfns_lookup_calendar(bd->name, buf, sizeof(buf));
	}

	if (rcode > 0)
		addr = buf;
	else 
#endif
		addr = bd->name;

	_DtTurnOnHourGlass(b->frame);
	if (b->add_to_array) {
		if (!register_names(bd->name, c)) {
			XmListDeselectPos(b->browse_list, cbs->item_position);
			_DtTurnOffHourGlass(b->frame);
			return;
		}
        	if (geditor_showing(e)) {
                	add_to_gaccess_list(addr, bd->cal_handle,
				bd->user_access, bd->version, bd->set_charset,
				e, True);
			add_all_gappt(e);
		}
		mb_update_array(bd->name, c);
	} else {
		/*
		 * Must update the array before we deregister names because we
		 * close the calendar handle when we deregister.
		 */
		mb_update_array(bd->name, c);
		mb_deregister_names(bd->name, c);
        	if (geditor_showing(e))
                	remove_from_gaccess_list(addr, e);
	}
        mb_refresh_canvas(b, c);
	_DtTurnOffHourGlass(b->frame);
}
#endif /* NOTHREADS */

extern void
mb_clear_selected_calendar(
	char		*name,
	Calendar	*c)
{
	GEditor		*e = (GEditor*)c->geditor;
	Browser         *b = (Browser *)c->browser;
	char		*addr;

#ifdef FNS
	int		rcode = -1;
	char		buf[MAXNAMELEN];

	rcode = -1;
	if (cmfns_use_fns((Props *)c->properties)) {
		/* Yes!  Try to use it */
		rcode = cmfns_lookup_calendar(name, buf, sizeof(buf));
	}

	if (rcode > 0)
		addr = buf;
	else 
#endif
		addr = name;

	/*
	 * Must update the array before we deregister names because we
	 * close the calendar handle when we deregister.
	 */
	b->add_to_array = False;
	mb_update_array(name, c);
        if (geditor_showing(e))
               	remove_from_gaccess_list(addr, e);
	mb_deregister_names(name, c);
        mb_refresh_canvas(c->browser, c);
}

extern void
mb_init_canvas(Calendar *c)
{
        Browser 	*b = (Browser*)c->browser;
	Browselist	*bl = (Browselist *)c->browselist;
	BlistData	*bd;
	int		i;

        b->add_to_array = True;
        gr_clear_area(b->xcontext, 0, 0, b->canvas_w, b->canvas_h);
	register_names(c->calname, c);
        mb_update_array(c->calname, c);

	/*
	 *  Search for the entry text in our list of calendar handles
	 */

	i = 1;
	while ((bd = (BlistData *)CmDataListGetData(bl->blist_data, i++))
	       && strcmp(bd->name, c->calname) != 0);

	if (!bd)
		return;

	XmListSelectPos(b->browse_list, i - 1, False);
}


extern void
mb_init_datefield(Browser *b, Calendar *c)
{
        char		*date;
        Props		*p = (Props *)c->properties;
	OrderingType	ot = get_int_prop(p, CP_DATEORDERING);
	SeparatorType	st = get_int_prop(p, CP_DATESEPARATOR);

	date = XmTextGetString(b->datetext);
	if (!date || *date == '\0') {
        	date = get_date_from_widget(c->view->date, b->datetext, ot, st);
		if (date != NULL)
			XmTextSetString(b->datetext, date);
	}
}
static void
mb_init_browchart(Browser *b, Calendar *c)
{
        int		char_width, char_height, day_len, day_of_week;
        int		label_height, label_width;
        Props		*p = (Props *)c->properties;
	Dimension	canvas_width, canvas_height;
	XFontSetExtents fontextents;
 
	mb_init_datefield(b, c);
	XtVaGetValues(b->canvas,
		      XmNwidth, &canvas_width,
		      XmNheight, &canvas_height,
		      NULL);
	b->canvas_w = (int)canvas_width;
	b->canvas_h = (int)canvas_height;
	CalFontExtents(c->fonts->labelfont, &fontextents);
        char_height = fontextents.max_ink_extent.height;
    	char_width = fontextents.max_ink_extent.width;
        label_height = char_height * 2;
        label_width = char_width + 2;
        b->chart_height =
		b->canvas_h - (c->view->outside_margin * 2) - label_height - 5;
        b->chart_width =
		b->canvas_w - (c->view->outside_margin * 2) - label_width;
        b->boxw = b->chart_width / 7;
        b->chart_width = b->boxw * 7;

        day_len = get_int_prop(p, CP_DAYEND) - get_int_prop(p, CP_DAYBEGIN);
        b->boxh = b->chart_height / day_len;

        /*
	 * Make sure boxh is evenly divisable by BOX_SEG
	 */
        b->boxh -= (b->boxh % BOX_SEG);
        b->chart_height = b->boxh * day_len;
        b->chart_x = c->view->outside_margin + label_width;
        b->chart_y = c->view->outside_margin + label_height + char_height;
}       

extern void
mb_draw_chartgrid(Browser *b, Calendar *c)
{
        int    		x, y;
        int     	n;
        Props   	*p = (Props*)c->properties;
	XFontSetExtents fontextents;
        int     	char_height, char_width;
        char    	label[5], buf[160];
        new_XContext 	*xc = b->xcontext;
        int 		dayy, dayweek;
        Tick 		daytick;
	DisplayType 	dt;
	int		nop;
	int		s_width;
 
 
	CalFontExtents(c->fonts->viewfont, &fontextents);
	char_height = fontextents.max_logical_extent.height;
	char_width = fontextents.max_logical_extent.width;

        /*      Draw chart. It'll be filled in later.
                Draw grid lines and hourly labels.      */
        x = b->chart_x;
        y = b->chart_y;
 
        /* clear header */
        gr_clear_area(xc, 0, 0, b->canvas_w, b->chart_y);
        label[0] = '\0';

        /* draw hour labels */
        for (n = get_int_prop(p, CP_DAYBEGIN); n <= get_int_prop(p, CP_DAYEND); n++) {
	
		dt = get_int_prop(p, CP_DEFAULTDISP);
                if (dt == HOUR12)
                        sprintf(label, "%2d", n > 12 ? n - 12 : n);
                else
                        sprintf(label, "%2d", n);
                gr_text(xc, c->view->outside_margin-char_width, y+3,
                        c->fonts->viewfont, label, NULL);
                gr_draw_line(xc, x, y, x + b->chart_width,
                         y, gr_solid, NULL);
                y += b->boxh;
        }
 
        /*
         * Draw vertical lines and day labels
         */
        y = b->chart_y;
        dayy = y - char_height - 4;
        dayweek = dow(b->date);
        daytick = last_ndays(b->date, dayweek == 0 ? 6 : dayweek-1);
 
        /* draw month */
        format_date(b->begin_week_tick+1, get_int_prop(p, CP_DATEORDERING), buf, 0, 0, 0);
        gr_text(xc, c->view->outside_margin+4,
                 dayy-char_height-4, c->fonts->labelfont, buf, NULL);
 
        for (n = 0; n < 7; n++) {
		if (daytick >= get_bot() && daytick < get_eot()) {

			CalTextExtents(c->fonts->viewfont, days3[n+1], strlen(days3[n+1]), &nop, &nop, &s_width, &nop);

                	gr_text(xc, b->chart_x + (b->boxw * n) + ((b->boxw - s_width)/2),
                        	dayy, c->fonts->viewfont, days3[n+1], NULL);

			CalTextExtents(c->fonts->viewfont, numbers[dom(daytick)], strlen(numbers[dom(daytick)]), &nop, &nop, &s_width, &nop);

                	gr_text(xc, b->chart_x + (b->boxw * n) + ((b->boxw - s_width)/2),
                        	y - char_height / 2, c->fonts->viewfont,
                        	numbers[dom(daytick)], NULL);
		}
                daytick += daysec;
                gr_draw_line(xc, b->chart_x + (b->boxw * n),
                        y, b->chart_x + (b->boxw * n),
                        y + b->chart_height, gr_solid, NULL);
        }
 
        /*
         * Draw box around the whole thing.
         */
        gr_draw_box(xc, b->chart_x, b->chart_y, b->chart_width, b->chart_height, NULL);
        gr_draw_box(xc, b->chart_x-1, b->chart_y-1, b->chart_width+2, b->chart_height+2, NULL);
}

extern void
mb_draw_appts(Browser *b, int start, int end, Calendar *c)
{
        int		x, y, h, i, end_of_day;
	Boolean		outofbounds = False;
	Colormap	cms;

	if (next_ndays(b->date, 1) > get_eot())
		outofbounds = True;
        XtVaGetValues(b->canvas, XmNcolormap, &cms, NULL);

        h = (b->boxh/BOX_SEG);
        end_of_day = (b->segs_in_array / 7);
 
        y = b->chart_y + (start % end_of_day) * h;
        x = b->chart_x + (start/end_of_day * b->boxw);
 
        i = start;
        while (i < end) {
                if (b->multi_array[i] <= 0) {
                        gr_clear_area(b->xcontext, x, y, b->boxw, h);
                        y += h;
                        i++;
                }
                else if (b->multi_array[i] == 1) {
                        /* batch up for one repaint */
                        if ( ((i+1) < b->segs_in_array)
                                 && b->multi_array[i+1] == 1 &&
                                ( ((i+1) % end_of_day) != 0)) {
                                h += (b->boxh/BOX_SEG);
                                if (++i < end)
                                        continue;
                        }
                        if ((c->xcontext->screen_depth < 8) || FAKE_MONOCHROME)
                                gr_make_gray(b->xcontext, x, y, b->boxw, h, 25);
                        else
                                gr_make_grayshade(b->xcontext, x, y, b->boxw, h,
                                                        LIGHTGREY);
                        y += h;
                        h = (b->boxh/BOX_SEG);
                        i++;
                }
                else if (b->multi_array[i] == 2) {
                        /* batch up for one repaint */
                        if ( ((i+1) < b->segs_in_array)
                                 && b->multi_array[i+1] == 2 &&
                                ( ((i+1) % end_of_day) != 0) ) {
                                h += (b->boxh/BOX_SEG);
                                if (++i < end)
                                        continue;
                        }
                        if ((c->xcontext->screen_depth < 8) || FAKE_MONOCHROME)
                                gr_make_gray(b->xcontext, x, y, b->boxw, h, 50);
                        else
                                gr_make_rgbcolor(b->xcontext, cms, x, y,
						 b->boxw, h, MIDGREY, MIDGREY,
						 MIDGREY);
                        y += h;
                        h = (b->boxh/BOX_SEG);
                        i++;
                }
                else if (b->multi_array[i] >= 3) {
                        /* batch up for one repaint */
                        if ( ((i+1) < b->segs_in_array)
                                && b->multi_array[i+1] >= 3 &&
                                ( ((i+1) % end_of_day) != 0) ) {
                                h += (b->boxh/BOX_SEG);
                                if (++i < end)
                                        continue;
                        }
                        if ((c->xcontext->screen_depth < 8) || FAKE_MONOCHROME)
                                gr_make_gray(b->xcontext, x, y, b->boxw, h, 75);
                        else
                                gr_make_rgbcolor(b->xcontext, cms, x, y,
						 b->boxw, h, DIMGREY, DIMGREY,
						 DIMGREY);
                        y += h;
                        h = (b->boxh/BOX_SEG);
                        i++;
                }
                if (i != 0 && ((i % end_of_day) == 0)) {
                        x += b->boxw;
                        y = b->chart_y;
                        h = (b->boxh/BOX_SEG);
                }
	if (outofbounds && i > 4)
		break;
        }

        browser_select(c, b, NULL);
}

extern void
mb_refresh_canvas(Browser *b, Calendar *c)
{
        mb_draw_appts(b, 0, b->segs_in_array, c);
        mb_draw_chartgrid(b, c);
        mb_display_footermess(b, c);
}

void
mb_resize_proc(Widget w, XtPointer client_data, XtPointer call)
{
        Dimension width, height;
        Calendar *c = (Calendar *)client_data;
	Browser *b;

        XtVaGetValues(w, XmNwidth, &width, XmNheight, &height, NULL);
        b = (Browser*)c->browser;
        gr_clear_area(b->xcontext, 0, 0, width, height);
        mb_init_browchart(b, c);
        mb_refresh_canvas(b, c);
}

void
mb_refigure_chart(Calendar *c) {
	mb_resize_proc(((Browser *)c->browser)->canvas, c, NULL);
}

extern void
browser_reset_list(Calendar *c) {
	int		i;
	Browser		*b = (Browser *)c->browser;
	BlistData	*bd;
	Browselist	*bl = (Browselist *)c->browselist;
	XmStringTable	list_selected_items, selected_items;
	int		selected_items_count;

	XtVaGetValues(b->browse_list,
		XmNselectedItemCount,	&selected_items_count,
		XmNselectedItems,	&list_selected_items,
		NULL);

	selected_items = (XmStringTable)calloc(selected_items_count, 
							sizeof(XmStringTable));
	for (i = 0; i < selected_items_count; i++)
		selected_items[i] = XmStringCopy(list_selected_items[i]);

	XtVaSetValues(b->upper_form, XmNresizePolicy, XmRESIZE_NONE, NULL);
	/*
	 * When a user removes a calendar from the menu we remove it
	 * from the multi-browser.  If the calendar happens to be selected
	 * in the multi-browser then we must deselect it and clean up
	 * the browser.  That's what this first loops does.
	 */
	for (i = 1; i <= bl->blist_data->count; i++) {
		bd = (BlistData *)CmDataListGetData(bl->blist_data, i);
		if (bd && bd->tag != BLIST_ACTIVE && bd->cal_handle) {
			mb_clear_selected_calendar(bd->name, c);
			/* We need to reset this to one because the blist_data
			 * has changed which may cause us to miss an item.
			 */
			i = 1;
		}
	}
	blist_clean(bl, False);

	XmListDeleteAllItems(b->browse_list);
	for (i = 1; i <= bl->blist_data->count; i++) {
		bd = (BlistData *)CmDataListGetData(bl->blist_data, i);
		if (bd && bd->name) {
			char 		buf[BUFSIZ + 5];
			XmString	xmstr;

			sprintf(buf, "  %s", bd->name); 
			xmstr = XmStringCreateLocalized(buf);
			if (!XmListItemExists(b->browse_list, xmstr))
				XmListAddItem(b->browse_list, xmstr, 0);
			XmStringFree(xmstr);
		}
	}
	XtVaSetValues(b->upper_form, XmNresizePolicy, XmRESIZE_ANY, NULL);
	
	/*
	 * Reselect the items that were selected before we changed the
	 * contents of the mb.
	 */
	for (i = 0; i < selected_items_count; i++) {
		int 	*pos_list, 
			 pos_cnt;

		if (XmListGetMatchPos(b->browse_list, selected_items[i],
				      &pos_list, &pos_cnt))
			XmListSelectPos(b->browse_list, pos_list[0], False);
		XmStringFree(selected_items[i]);
	}

	if (selected_items)
		free(selected_items);
}

extern void
init_browser(Calendar *c)
{
	pr_pos	xy;
        Browser *b = (Browser*)c->browser;
 
        browser_reset_list(c);
        b->row_sel = b->col_sel = 0;
        mb_init_browchart(b, c);
        mb_init_canvas(c);
        mb_refresh_canvas(b, c);
	xy.x = dow(b->date) - 1;
	xy.y = 0;
	browser_select(c, b, &xy);
}

static void
cancel_cb(Widget w, XtPointer client, XtPointer call)
{
        Calendar 	*c = (Calendar *)client;
	Browser 	*b = (Browser *)c->browser;
	Browselist	*bl = (Browselist *)c->browselist;

	XtPopdown(b->frame);

#ifndef NOTHREADS
	enable_menu_editor(bl, True);
#endif /* NOTHREADS */

	XtDestroyWidget(b->frame);
	XtFree(b->multi_array); 
	XtFree(c->browser); c->browser = NULL;
}

static void
popup_cb(Widget w, XtPointer client, XtPointer call)
{
        Calendar *c = (Calendar *)client;
	Browser *b = (Browser *)c->browser;
	Position x, y;

	XtVaGetValues(c->frame, XmNx, &x, XmNy, &y, NULL);
	XtVaSetValues(b->frame, XmNx, x+100, XmNy, y+100, NULL);
}

/*
 * This is the CSA_callback called from the CSA library when
 * an update occurs on a calendar to which we are logged on,
 * and have registered interest.  Registered in register_names.
 * When calendar is logged off, any registered callbacks for it
 * are destroyed automagically.
 */
static void
mb_update_handler(CSA_session_handle cal, CSA_flags reason,
               CSA_buffer call_data, CSA_buffer client_data, CSA_extension *ext)
{
        Calendar        *c = calendar;
	Browser         *b = (Browser *)c->browser;

        /* sync whatever needs sync'ing */
	if (b) {
		br_display(c);
		if (geditor_showing((GEditor *)c->geditor))
			add_all_gappt((GEditor *)c->geditor);
	}
 
}

