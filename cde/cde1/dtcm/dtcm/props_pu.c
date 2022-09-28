/* $XConsortium: props_pu.c /main/cde1_maint/5 1995/11/16 18:22:11 barstow $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)props_pu.c	1.111 96/06/17 Sun Microsystems, Inc."

#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include <sys/param.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/DialogS.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/SeparatoG.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Dt/ComboBox.h>
#include <Dt/SpinBox.h>
#include <Dt/HourGlass.h>
#include <csa.h>
#include "props_pu.h"
#include "editor.h"
#include "group_editor.h"
#include "ps_graphics.h"
#include "todo.h"
#include "calendar.h"
#include "browser.h"
#include "dayglance.h"
#include "deskset.h"
#include "rfp.h"
#include "help.h"
#include "getdate.h"

static void p_gap_clear_toggles(Props_pu *p);
static void p_gap_center_cat_menu(Widget, Widget);
static void p_create_v4_group_access_pane(Props_pu *);
extern void p_create_v5_group_access_pane(Props_pu *);
extern void v5_gap_clear_pending_change(Props_pu *p);
extern Dimension ComputeMaxWidth(Widget, Widget, Widget, Widget);

/*******************************************************************************
**
**  Constants
**
*******************************************************************************/
#define NAME_LEN	23
#define PERMISSION_LEN	6

static XmString	Pane_XmStrs[DATE_FORMAT_PANE + 1];

/*******************************************************************************
**
**  Functions static to props_pu.c
**
*******************************************************************************/



static void
v4_gap_pending_change(Widget w, XtPointer data, XtPointer cbs) {
        Props_pu *p = (Props_pu *) data;
 
        if (p->v4_gap_pending_message_up == False) {
                p->v4_gap_pending_message_up = True;
                XtSetSensitive(p->gap_add_button, True);
        }
}
 
static void
v4_gap_clear_pending_change(Props_pu *p) {
 
        if (p->v4_gap_pending_message_up == True) {
                p->v4_gap_pending_message_up = False;
                XtSetSensitive(p->gap_add_button, False);
        }
}

extern void
p_set_change(Props_pu *p) {

	if (!p)
		return;

	p->changed_flag = True;

	if (p->ok_button)
		XtSetSensitive(p->ok_button, True);
	if (p->apply_button)
		XtSetSensitive(p->apply_button, True);
	if (p->defaults_button)
		XtSetSensitive(p->defaults_button, True);
	if (p->base_form_mgr)
        	XtVaSetValues(p->base_form_mgr, XmNdefaultButton, p->apply_button, NULL);
}

static void
p_clear_change(Props_pu *p) {

	p->changed_flag = False;

	XtSetSensitive(p->ok_button, False);
	XtSetSensitive(p->apply_button, False);
	XtSetSensitive(p->defaults_button, False);
	if (p->cal->my_cal_version >= DATAVER4) {
		char	*str;
		if (*(str = (char *)XmTextGetString(p->gap_user_text))=='\0') {
			v5_gap_clear_pending_change(p);
		} else {
			XtFree(str);
			if (p->v5_gap_pending_message_up == False) {
				p->v5_gap_pending_message_up = True;
				XtSetSensitive(p->gap2_add_button, True);
			}
		}
        	XtSetSensitive(p->gap2_change_button, False);
	} else
		v4_gap_clear_pending_change(p);

        XtSetSensitive(p->gap_remove_button, False);
        XtVaSetValues(p->base_form_mgr, XmNdefaultButton, p->close_button, NULL);
}
static void
p_mark_change(Widget w, XtPointer data, XtPointer cbs) {

        Props_pu        *p = (Props_pu *)data;

	p_set_change(p);
}

static void
set_menu_marker(Widget menu, Props_pu *p) {

	Widget		submenu;
	WidgetList	children;
	Cardinal	num_c;

	XtVaGetValues(menu, XmNsubMenuId, &submenu, NULL);
        XtVaGetValues(submenu, 
		XmNchildren, &children, 
		XmNnumChildren, &num_c, 
		NULL);

	while (num_c > 0)
		XtAddCallback(children[--num_c], XmNactivateCallback, p_mark_change, p);
}

#define GAP 5

static void
p_create_editor_pane(
	Calendar 	*c)
{
	int		 ac;
	Arg		 args[10];
	char		 buf[MAXNAMELEN];
	Props		*p = (Props *)c->properties;
	Widget		 ep_form_mgr;
	Widget		 text_field;
	Props_pu	*pu = (Props_pu *)c->properties_pu;
	XmString	 xmstr;
	Dimension	 max_left_label_width;
	Widget		 widgets[7];

	/*
        **  Base form
	*/
	ep_form_mgr = XtVaCreateWidget("ep_form_mgr",
		xmFormWidgetClass, pu->base_form_mgr,
		XmNautoUnmanage, 	False,
	        XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		pu->separator1,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNrightAttachment, 	XmATTACH_FORM,
		XmNbottomOffset,	10,
		NULL);
	pu->pane_list[EDITOR_PANE] = ep_form_mgr;

	/*
	**  Default start time and duration for appointments - use DSSW widget
	**  for start time stuff.
	*/
	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 644,
						"Appointment Times:"));
        pu->ep_appt_label = XtVaCreateWidget("apptTimes",
		xmLabelGadgetClass, ep_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_FORM,
                XmNtopOffset, 		GAP,
		XmNleftAttachment, 	XmATTACH_FORM,
                XmNleftOffset, 		GAP,
                NULL);
	XmStringFree(xmstr);

	build_dssw(&pu->ep_dssw, c, ep_form_mgr, False, False);
	XtUnmanageChild(pu->ep_dssw.what_scrollwindow);

	widgets[0] = pu->ep_dssw.start_label;
	widgets[1] = pu->ep_dssw.start_text;
	widgets[2] = pu->ep_dssw.start_menu;
	widgets[3] = pu->ep_dssw.start_ampm_rc_mgr;

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 645,
						"Duration:"));
        pu->ep_dur_label = widgets[4] = XtVaCreateWidget("duration",
		xmLabelGadgetClass, pu->ep_dssw.dssw_form_mgr,
		XmNlabelString, xmstr,
                NULL);
	XmStringFree(xmstr);

	max_left_label_width = ComputeMaxWidth(pu->ep_dur_label, 
					       pu->ep_dssw.start_label, 
					       NULL, NULL) + 2 * GAP;
	ac = 0;
	XtSetArg(args[ac], DtNcomboBoxType, 	DtDROP_DOWN_COMBO_BOX); ++ac;
	XtSetArg(args[ac], XmNleftAttachment, 	XmATTACH_FORM); ++ac;
	XtSetArg(args[ac], XmNleftOffset, 	max_left_label_width); ++ac;
	XtSetArg(args[ac], XmNtopAttachment, 	XmATTACH_FORM); ++ac;
	XtSetArg(args[ac], XmNtopOffset, 	GAP); 	++ac;
	XtSetArg(args[ac], XmNvisibleItemCount, 5); ++ac;
	pu->ep_dur_cb = widgets[5] = DtCreateComboBox(pu->ep_dssw.dssw_form_mgr,
					 "ep_dur_cb", args, ac);
	XtVaSetValues(pu->ep_dur_cb, XmNwidth, 75, NULL);

	for(ac = 15; ac <= 90; ac += 15) {
		if (ac != 75) {
			sprintf(buf, "%d", ac);
			xmstr = XmStringCreateLocalized(buf);
			DtComboBoxAddItem((Widget)pu->ep_dur_cb,
					  xmstr, 0, False);
			XmStringFree(xmstr);
		}
	}

	XtVaGetValues(pu->ep_dur_cb, DtNtextField, &text_field, 0);
	XtVaSetValues(pu->ep_dur_label,
                XmNbottomAttachment,    XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget,        pu->ep_dur_cb,
                XmNbottomOffset,        GAP,
                XmNrightAttachment,     XmATTACH_WIDGET,
                XmNrightWidget,         pu->ep_dur_cb,
                XmNrightOffset,         GAP,
                NULL);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 646, "Minutes"));
        pu->ep_dur_message = widgets[6] = XtVaCreateWidget("message",
		xmLabelGadgetClass, pu->ep_dssw.dssw_form_mgr,
		XmNlabelString, 	xmstr,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		pu->ep_dur_cb,
		XmNleftOffset, 		GAP,
		XmNtopAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNtopWidget, 		pu->ep_dur_label,
                NULL);
	XmStringFree(xmstr);

	XtVaSetValues(pu->ep_dssw.start_text,
		XmNleftAttachment,      XmATTACH_FORM,
                XmNleftOffset,          max_left_label_width + 1,
		XmNtopAttachment,       XmATTACH_WIDGET,
		XmNtopWidget,           pu->ep_dur_cb,
		XmNtopOffset,           GAP,
		NULL);

	XtVaSetValues(pu->ep_dssw.start_label,
		XmNbottomAttachment,    XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget,        pu->ep_dssw.start_text,
                XmNbottomOffset,        GAP,
		XmNrightAttachment, 	XmATTACH_WIDGET,
		XmNrightWidget, 	pu->ep_dssw.start_text,
		XmNrightOffset, 	GAP,
		NULL);

	XtVaSetValues(pu->ep_dssw.dssw_form_mgr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		pu->ep_appt_label,
		XmNtopOffset, 		10,
		XmNrightAttachment, 	XmATTACH_FORM,
		XmNleftAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNleftWidget, 		pu->ep_appt_label,
		XmNleftOffset, 		0,
		NULL);
	XtManageChildren(widgets, 7);

	XtVaGetValues(pu->ep_dur_cb, DtNtextField, &text_field, 0);
	XtAddCallback(text_field, XmNvalueChangedCallback, p_mark_change, pu);
	XtAddCallback(pu->ep_dur_cb, DtNselectionCallback, p_mark_change, pu);

	/*
	 *  Reminders widget (self contained)
	 */
	build_reminders(&pu->ep_reminders, c, ep_form_mgr);
	XtVaSetValues(pu->ep_reminders.bfpm_form_mgr,
	        XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, pu->ep_dssw.dssw_form_mgr,
		XmNtopOffset, 5,
		XmNleftAttachment, XmATTACH_FORM,
		NULL);

	/*
	**  Build the stuff for privacy ...
	*/
	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 390, "Privacy:"));
        pu->privacy_label = XtVaCreateWidget("privacy",
		xmLabelGadgetClass, 	pu->ep_reminders.bfpm_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		pu->ep_reminders.mailto_label,
		XmNtopOffset, 		30,
		XmNleftAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNleftWidget, 		pu->ep_reminders.mailto_label,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNbottomOffset,	10,
                NULL);
	XmStringFree(xmstr);

	pu->privacy_menu = create_privacy_menu(pu->ep_reminders.bfpm_form_mgr);
	XtVaSetValues(pu->privacy_menu,
		XmNuserData, 		0,
		XmNtopAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNtopWidget, 		pu->privacy_label,
		XmNtopOffset, 		-2,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		pu->privacy_label,
		XmNleftOffset, 		5,
		NULL);
	ManageChildren(pu->ep_reminders.bfpm_form_mgr);

	XtAddCallback(pu->privacy_menu, 
			DtNselectionCallback, p_mark_change, pu);

	XtAddCallback(pu->ep_dssw.start_text, 
			XmNvalueChangedCallback, p_mark_change, pu);
	XtAddCallback(pu->ep_dssw.start_am, 
			XmNvalueChangedCallback, p_mark_change, pu);
	XtAddCallback(pu->ep_dssw.start_pm, 
			XmNvalueChangedCallback, p_mark_change, pu);
	XtAddCallback(pu->ep_reminders.beep_text, 
			XmNvalueChangedCallback, p_mark_change, pu);
	XtAddCallback(pu->ep_reminders.beep_toggle, 
			XmNvalueChangedCallback, p_mark_change, pu);

	set_menu_marker(pu->ep_reminders.beep_menu, pu);
	XtAddCallback(pu->ep_reminders.flash_text, 
			XmNvalueChangedCallback, p_mark_change, pu);
	XtAddCallback(pu->ep_reminders.flash_toggle, 
			XmNvalueChangedCallback, p_mark_change, pu);
	set_menu_marker(pu->ep_reminders.flash_menu, pu);
	XtAddCallback(pu->ep_reminders.mail_text, 
			XmNvalueChangedCallback, p_mark_change, pu);
	XtAddCallback(pu->ep_reminders.mail_toggle, 
			XmNvalueChangedCallback, p_mark_change, pu);
	set_menu_marker(pu->ep_reminders.mail_menu, pu);
	XtAddCallback(pu->ep_reminders.mailto_text, 
			XmNvalueChangedCallback, p_mark_change, pu);
	XtAddCallback(pu->ep_reminders.popup_text, 
			XmNvalueChangedCallback, p_mark_change, pu);
	XtAddCallback(pu->ep_reminders.popup_toggle, 
			XmNvalueChangedCallback, p_mark_change, pu);
	set_menu_marker(pu->ep_reminders.popup_menu, pu);

	ManageChildren(ep_form_mgr);
}

static void
format_hour(char buffer[], int hour, DisplayType dt, Calendar *c)

{
	if (dt == HOUR12) {
		if ((hour == 24) || (hour == 0))
			cm_strcpy(buffer, catgets(c->DT_catd, 1, 758,"midnight"));
		else if (hour > 12)
				sprintf(buffer, "%2d:00 %s", hour-12,
					catgets(c->DT_catd, 1, 3, "pm"));
		else
				sprintf(buffer, "%2d:00 %s", hour,
					catgets(c->DT_catd, 1, 4, "am"));
	}
	else
		sprintf(buffer, " %02d00", hour);
}

static void
set_scale_value(Widget w, int val, Props_pu *p)

{
	char			label_buffer[50];
	XmString		xmstr;
	Calendar		*c = p->cal;
	Props			*props = (Props *) c->properties;

	format_hour(label_buffer, val, (DisplayType) get_int_prop(props, CP_DEFAULTDISP), c);

	xmstr = XmStringCreateLocalized(label_buffer);

	XtVaSetValues(w,
		XmNlabelString, xmstr,
		NULL);

	XmStringFree(xmstr);
}


/*
**  Functions related to the display pane
*/
static void
p_dp_scale_mod_proc(Widget w, XtPointer data, XtPointer cbs) {
	int			val;
	Props_pu		*p = (Props_pu *)data;
	XmScaleCallbackStruct	*scbs = (XmScaleCallbackStruct *)cbs;

	if (!p)
		return;

	if (w == p->dp_beg_scale) {
		XmScaleGetValue(p->dp_end_scale, &val);
		if (val < scbs->value)
		{
			XmScaleSetValue(p->dp_end_scale, scbs->value + 1);
			set_scale_value(p->dp_end_value, scbs->value + 1, p);
		}
		set_scale_value(p->dp_beg_value, scbs->value, p);


	} else {
		XmScaleGetValue(p->dp_beg_scale, &val);
		if (val > scbs->value)
		{
			XmScaleSetValue(p->dp_beg_scale, scbs->value - 1);
			set_scale_value(p->dp_beg_value, scbs->value - 1, p);
		}
		set_scale_value(p->dp_end_value, scbs->value, p);

	}

	p_set_change(p);
}

static void
p_create_display_pane(Props_pu *p) {
	Calendar 	*c = calendar;
	Widget 		dp_form_mgr;
	XmString	xmstr;
	Widget		widgets[2];

	/*
        **  Base form
	*/
	dp_form_mgr = XtVaCreateWidget("dp_form_mgr",
		xmFormWidgetClass, p->base_form_mgr,
		XmNautoUnmanage, 	False,
	        XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->separator1,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNrightAttachment, 	XmATTACH_FORM,
		XmNbottomAttachment, 	XmATTACH_WIDGET,
		XmNbottomOffset, 	10,
		XmNbottomWidget, 	p->separator2,
		NULL);
	p->pane_list[DISPLAY_PANE] = dp_form_mgr;

	/*
	**  User Calendar Location label and text field
	*/
	xmstr = XmStringCreateLocalized(
			catgets(c->DT_catd, 1, 391, "User Calendar Location:"));
        p->dp_loc_label = XtVaCreateWidget("location",
		xmLabelGadgetClass, dp_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_FORM,
		XmNtopOffset, 		12,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		5,
                NULL);
	XmStringFree(xmstr);

        p->dp_loc_text = XtVaCreateWidget("dp_loc_text",
		xmTextWidgetClass, dp_form_mgr,
		XmNcolumns, 		15,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	p->dp_loc_label,
		XmNbottomOffset, 	-4,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->dp_loc_label,
		XmNleftOffset, 		10,
                NULL);

	XtAddCallback(p->dp_loc_text, 
			XmNvalueChangedCallback, p_mark_change, p);

	/*
	**  Day Boundaries label and beginning/ending sliders
	*/

	/* There is some additional, very odd layout stuff that 
	   happens elsewhere.  This is because sliders do not align 
	   well with text widgets at all.  We want to align the 
	   centers of the text and slider widgets.  This is done by 
	   finding the relative heights of the widgets, and using half 
	   the differenmce as a bottom offset.  Why don't you see that 
	   below?  Well, it's because the stupid toolkit reports the 
	   height of a slider as 0 unless that slider is mapped.  Thus, 
	   this bit of alignment cannot happen until the panel with these 
	   items is mapped.  Pretty stupid.  Thus the positioning happens 
	   just after the panel is mapped. */
	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 392, "Day Boundaries:"));
        p->dp_day_label = XtVaCreateWidget("dayBoundaries",
		xmLabelGadgetClass, dp_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->dp_loc_text,
		XmNtopOffset, 		10,
		XmNrightAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNrightWidget, 	p->dp_loc_label,
                NULL);
	XmStringFree(xmstr);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 393, "Begin:"));
        p->dp_beg_label = XtVaCreateWidget("begin",
		xmLabelGadgetClass, dp_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->dp_day_label,
		XmNtopOffset, 		12,
		XmNrightAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNrightWidget, 	p->dp_day_label,
                NULL);
	XmStringFree(xmstr);

	p->dp_beg_scale = XtVaCreateWidget("dp_beg_scale",
		xmScaleWidgetClass, dp_form_mgr,
		XmNorientation, 	XmHORIZONTAL,
		XmNmaximum, 		23,
		XmNminimum, 		0,
		XmNvalue, 		0,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	p->dp_beg_label,
		XmNbottomOffset, 	3,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->dp_beg_label,
		XmNleftOffset, 		10,
		XmNscaleMultiple, 	1,
		NULL);
	XtAddCallback(p->dp_beg_scale, XmNvalueChangedCallback,
		      p_dp_scale_mod_proc, p);
	XtAddCallback(p->dp_beg_scale, XmNdragCallback,
		      p_dp_scale_mod_proc, p);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 759, "value"));
        p->dp_beg_value = XtVaCreateWidget("begin",
		xmLabelGadgetClass, dp_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->dp_day_label,
		XmNtopOffset, 		12,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->dp_beg_scale,
                NULL);
	XmStringFree(xmstr);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 394, "End:"));
        p->dp_end_label = XtVaCreateWidget("end",
		xmLabelGadgetClass, dp_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->dp_beg_scale,
		XmNtopOffset, 		12,
		XmNrightAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNrightWidget, 	p->dp_beg_label,
                NULL);
	XmStringFree(xmstr);

	p->dp_end_scale = XtVaCreateWidget("dp_end_scale",
		xmScaleWidgetClass, dp_form_mgr,
		XmNorientation, 	XmHORIZONTAL,
		XmNmaximum, 		24,
		XmNminimum, 		1,
		XmNvalue, 		1,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	p->dp_end_label,
		XmNbottomOffset, 	3,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->dp_end_label,
		XmNleftOffset, 		10,
		XmNscaleMultiple, 	1,
		NULL);
	XtAddCallback(p->dp_end_scale, XmNvalueChangedCallback,
		      p_dp_scale_mod_proc, p);
	XtAddCallback(p->dp_end_scale, XmNdragCallback,
		      p_dp_scale_mod_proc, p);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 760, "value"));
        p->dp_end_value = XtVaCreateWidget("begin",
		xmLabelGadgetClass, dp_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNtopWidget, 		p->dp_end_label,
		XmNtopOffset, 		0,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->dp_end_scale,
                NULL);
	XmStringFree(xmstr);

	/*
	**  12/24 Hour Display label and toggle buttons
	*/
	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 395, "Hour Display:"));
        p->dp_hour_label = XtVaCreateWidget("hourDisplay",
		xmLabelGadgetClass, dp_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->dp_end_scale,
		XmNtopOffset, 		12,
		XmNrightAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNrightWidget, 	p->dp_end_label,
                NULL);
	XmStringFree(xmstr);

	p->dp_hour_rc_mgr = XtVaCreateWidget("dp_hour_rc_mgr",
                xmRowColumnWidgetClass, dp_form_mgr, 
                XmNpacking, 		XmPACK_COLUMN,
                XmNorientation, 	XmHORIZONTAL,
                XmNradioBehavior, 	True,
                XmNisHomogeneous, 	True,
                XmNentryClass, 		xmToggleButtonGadgetClass,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	p->dp_hour_label,
		XmNbottomOffset, 	-5,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->dp_hour_label,
		XmNleftOffset, 		10,
                NULL);
 
	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 396, "12 Hour"));
        p->dp_hour12_toggle = widgets[0] = XtVaCreateWidget("twelveHour",
                xmToggleButtonGadgetClass, p->dp_hour_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);      
	XmStringFree(xmstr);

	XtAddCallback(p->dp_hour12_toggle, 
			XmNvalueChangedCallback, p_mark_change, p);
 
	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 397, "24 Hour"));
        p->dp_hour24_toggle = widgets[1] = XtVaCreateWidget("twentyFourHour",
                xmToggleButtonGadgetClass, p->dp_hour_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(p->dp_hour24_toggle, 
			XmNvalueChangedCallback, p_mark_change, p);
 
        XtManageChildren(widgets, 2);

	/*
	**  Default view label and toggle buttons
	*/
	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 398, "Default View:"));
        p->dp_view_label = XtVaCreateWidget("defaultView:",
		xmLabelGadgetClass, dp_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->dp_hour_rc_mgr,
		XmNtopOffset, 		12,
		XmNrightAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNrightWidget, 	p->dp_hour_label,
                NULL);
	XmStringFree(xmstr);

	p->dp_view_rc_mgr = XtVaCreateWidget("dp_view_rc_mgr",
                xmRowColumnWidgetClass, dp_form_mgr,
                XmNpacking, 		XmPACK_COLUMN,
                XmNorientation, 	XmHORIZONTAL,
                XmNradioBehavior, 	True,
                XmNisHomogeneous, 	True,
                XmNentryClass, 		xmToggleButtonGadgetClass,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	p->dp_view_label,
		XmNbottomOffset, 	-5,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->dp_view_label,
		XmNleftOffset, 		10,
                NULL);
 
 
	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 402, "Day"));
        p->dp_view_day_toggle = XtVaCreateWidget("day",
                xmToggleButtonGadgetClass, p->dp_view_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);      
	XmStringFree(xmstr);

	XtAddCallback(p->dp_view_day_toggle, 
			XmNvalueChangedCallback, p_mark_change, p);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 401, "Week"));
        p->dp_view_week_toggle = XtVaCreateWidget("week",
                xmToggleButtonGadgetClass, p->dp_view_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);      
	XmStringFree(xmstr);

	XtAddCallback(p->dp_view_week_toggle, 
			XmNvalueChangedCallback, p_mark_change, p);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 400, "Month"));
        p->dp_view_month_toggle = XtVaCreateWidget("month",
                xmToggleButtonGadgetClass, p->dp_view_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);      
	XmStringFree(xmstr);

	XtAddCallback(p->dp_view_month_toggle, 
			XmNvalueChangedCallback, p_mark_change, p);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 399, "Year"));
        p->dp_view_year_toggle = XtVaCreateWidget("year",
                xmToggleButtonGadgetClass, p->dp_view_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);      
	XmStringFree(xmstr);

	XtAddCallback(p->dp_view_year_toggle, 
			XmNvalueChangedCallback, p_mark_change, p);
 
	ManageChildren(p->dp_view_rc_mgr);

	/*
	**  Initial Calendar View label and text field
	*/
	xmstr = XmStringCreateLocalized(
			catgets(c->DT_catd, 1, 403, "Initial Calendar View:"));
        p->dp_init_view_label = XtVaCreateWidget("initialView",
		xmLabelGadgetClass, dp_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->dp_view_rc_mgr,
		XmNtopOffset, 		12,
		XmNrightAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNrightWidget, 	p->dp_view_label,
                NULL);
	XmStringFree(xmstr);

        p->dp_init_view_text = XtVaCreateWidget("dp_init_view_text",
		xmTextWidgetClass, dp_form_mgr,
		XmNcolumns, 		25,
		XmNtopAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNtopWidget, 		p->dp_init_view_label,
		XmNtopOffset, 		-3,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->dp_init_view_label,
		XmNleftOffset, 		10,
#ifndef FNS
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNbottomOffset,	10,
#endif /* FNS */
                NULL);

	XtAddCallback(p->dp_init_view_text, 
			XmNvalueChangedCallback, p_mark_change, p);

	ManageChildren(dp_form_mgr);

#ifdef FNS

        xmstr = XmStringCreateLocalized(
                        catgets(c->DT_catd, 1, 2106, "Calendar Names:"));

        p->dp_use_fns_label = XtVaCreateWidget(
		"calNames",
		xmLabelGadgetClass, dp_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->dp_init_view_label,
		XmNtopOffset, 		12,
		XmNrightAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNrightWidget, 	p->dp_init_view_label,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNbottomOffset,	10,
                NULL);

	XmStringFree(xmstr);

        xmstr = XmStringCreateLocalized(
                        catgets(c->DT_catd, 1, 2107, "Use Federated Naming"));

        p->dp_use_fns_toggle = XtVaCreateWidget(
		"useFNS",
                xmToggleButtonGadgetClass, dp_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNtopWidget, 		p->dp_use_fns_label,
		XmNtopOffset, 		-3,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->dp_use_fns_label,
		XmNleftOffset, 		10,
                NULL);
	XmStringFree(xmstr);
	XtAddCallback(p->dp_use_fns_toggle, 
			XmNvalueChangedCallback, p_mark_change, p);
        ManageChildren(dp_form_mgr);
#endif

}

extern unsigned int
p_gap_convert_permissions(
	Props_pu 	*p)
{
	Calendar	*c = calendar;
	unsigned int	 perm_val = 0;

	/*
	 * V5DEP: If this is a V5 server w/V1 backing store probably should use
	 * old access flags.
	 */
	if (p->cal->my_cal_version >= DATAVER4) {
		if (XmToggleButtonGadgetGetState(p->gap2_public_tog[GAP_VIEW]))
			perm_val |= CSA_VIEW_PUBLIC_ENTRIES;
		if (XmToggleButtonGadgetGetState(
					p->gap2_public_tog[GAP_INSERT]))
			perm_val |= CSA_INSERT_PUBLIC_ENTRIES;
		if (XmToggleButtonGadgetGetState(
					p->gap2_public_tog[GAP_CHANGE]))
			perm_val |= CSA_CHANGE_PUBLIC_ENTRIES;
		if (XmToggleButtonGadgetGetState(
					p->gap2_semi_tog[GAP_VIEW]))
			perm_val |= CSA_VIEW_CONFIDENTIAL_ENTRIES;
		if (XmToggleButtonGadgetGetState(
					p->gap2_semi_tog[GAP_INSERT]))
			perm_val |= CSA_INSERT_CONFIDENTIAL_ENTRIES;
		if (XmToggleButtonGadgetGetState(
					p->gap2_semi_tog[GAP_CHANGE]))
			perm_val |= CSA_CHANGE_CONFIDENTIAL_ENTRIES;
		if (XmToggleButtonGadgetGetState(
					p->gap2_private_tog[GAP_VIEW]))
			perm_val |= CSA_VIEW_PRIVATE_ENTRIES;
		if (XmToggleButtonGadgetGetState(
					p->gap2_private_tog[GAP_INSERT]))
			perm_val |= CSA_INSERT_PRIVATE_ENTRIES;
		if (XmToggleButtonGadgetGetState(
					p->gap2_private_tog[GAP_CHANGE]))
			perm_val |= CSA_CHANGE_PRIVATE_ENTRIES;
		if (perm_val)
			perm_val |= CSA_ORGANIZER_RIGHTS;
	} else {
		if (XmToggleButtonGadgetGetState(p->gap_browse_toggle))
			perm_val |= CSA_X_DT_BROWSE_ACCESS;
		if (XmToggleButtonGadgetGetState(p->gap_insert_toggle))
			perm_val |= CSA_X_DT_INSERT_ACCESS;
		if (XmToggleButtonGadgetGetState(p->gap_delete_toggle))
			perm_val |= CSA_X_DT_DELETE_ACCESS;
	}

	if (perm_val == 0) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 750, 
			      "Calendar : Error - Access List and Permissions"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 405,
			      "Please select at least one permission.        "));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, 
				"Continue"));
		dialog_popup(p->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			BUTTON_HELP, ACCESS_RIGHTS_ERROR_HELP,
			DIALOG_IMAGE, p->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
	}

	return perm_val;
}

static char*
p_gap_create_perm_str(CSA_flags perm_val) {
	int	i = 0;
	char	c_value[PERMISSION_LEN + 1];

	if (perm_val & CSA_X_DT_BROWSE_ACCESS) {
		c_value[i++] = 'B';
		c_value[i++] = ' ';
	}
	if (perm_val & CSA_X_DT_INSERT_ACCESS) {
		c_value[i++] = 'I';
		c_value[i++] = ' ';
	}
	if (perm_val & CSA_X_DT_DELETE_ACCESS) {
		c_value[i++] = 'D';
		c_value[i++] = ' ';
	}
	c_value[i] = '\0';
	return cm_strdup(c_value);
}

static void
p_gap_create_entry_string(Props_pu *p, char *who, char *p_str, XmString *val) {
	char		*buf;
        int             name_length;

        name_length = cm_strlen(who); 
        if (name_length <  NAME_LEN) {
           name_length = NAME_LEN;
        }
	   
        buf = (char *)XtMalloc(name_length + PERMISSION_LEN + 2);
        sprintf(buf, "%-*s %s", name_length, who, p_str);
	*val = XmStringCreateLocalized(buf);
	free(buf);
}

extern void
uname_error_popup (char *uname, Props_pu *p) 
{

        Calendar *c = calendar;

        char *title = XtNewString(catgets(c->DT_catd, 1, 750,
                      "Calendar : Error - Access List and Permissions")) ;

        char *text = XtNewString(catgets(c->DT_catd, 1, 684,
                      "User Names may not have embedded blanks or tabs."));

        char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95,
                                                   "Continue"));

        dialog_popup(p->frame,
                        DIALOG_TITLE, title,
                        DIALOG_TEXT, text,
                        BUTTON_IDENT, 1, ident1,
                        BUTTON_HELP, ACCESS_RIGHTS_ERROR_HELP,
                        DIALOG_IMAGE, p->xm_error_pixmap,
                        NULL);
        XtFree(ident1);
        XtFree(text);
        XtFree(title);
        return;
}

static void
p_gap_add_proc(Widget w, XtPointer client_data, XtPointer callback_struct) {
	int		*item_list = NULL, item_cnt = 0;
	CSA_flags	pt;
	char		*who, *p_str;
	Props_pu	*p = (Props_pu *)client_data;
	XmString	xmstr;
	CSA_access_list	step = p->gap_list, last = NULL, new_e;

	/*
	**  First, get the necessary strings from the UI
	*/
	if (!(pt = p_gap_convert_permissions(p)))
		return;
	p_str = p_gap_create_perm_str(pt);
	who = (char *)XmTextGetString(p->gap_user_text);
        
        if (embedded_blank(who)) {
           uname_error_popup(who, p);
           XtFree(who);
           return;
        }
	p_gap_create_entry_string(p, who, p_str, &xmstr);
	free(p_str);

	/*
	**  Either add or change the item on the UI and to the Access_Entry
	**  linked list.
	*/
	new_e = (CSA_access_list)XtCalloc(1, sizeof(CSA_access_rights));
	new_e->user = (CSA_calendar_user *)XtCalloc(1, 
						sizeof(CSA_calendar_user));
	new_e->user->user_name = cm_strdup(who);
	new_e->rights = pt;
	new_e->next = NULL;
	XtFree(who);

	if (XmListGetSelectedPos(p->gap_access_list, &item_list, &item_cnt)) {
		/*
		**  We're changing the value, find the item in the linked list
		*/
		item_cnt = 1;
		while(item_cnt < *item_list) {
			last = step;
			step = step->next;
			++item_cnt;
		}
		if (last)
			last->next = new_e;
		else
			p->gap_list = new_e;

		new_e->next = step->next;
		free(step->user);
		free(step);

		XmListReplaceItemsPos(p->gap_access_list, &xmstr, 1, *item_list);
		XtFree((XtPointer)item_list);
	} else {
		/*
		**  Add the Access_Entry item to the end of the list
		*/
		if (!p->gap_list)
			p->gap_list = new_e;
		else {
			while (step->next)
				step = step->next;
			step->next = new_e;
		}
		XmListAddItem(p->gap_access_list, xmstr, 0);
	}
	XmStringFree(xmstr);
	p_set_change(p);
}

extern void
p_gap_remove_proc(Widget w, XtPointer client_data, XtPointer callback_struct) {
	Calendar *c = calendar;
	int		*item_list = NULL, item_cnt = 0;
	Props_pu	*p = (Props_pu *)client_data;
	CSA_access_list	step = p->gap_list, last = NULL;

	if (XmListGetSelectedPos(p->gap_access_list, &item_list, &item_cnt)) {
		/*
		**  Find and delete the Access_Entry from the list
		*/
		item_cnt = 1;
		while(item_cnt < *item_list) {
			last = step;
			step = step->next;
			++item_cnt;
		}
		if (last)
			last->next = step->next;
		else
			p->gap_list = step->next;

		free(step->user);
		free(step);

		XmListDeleteItemsPos(p->gap_access_list, 1, *item_list);
		XmTextSetString(p->gap_user_text, "\0");
		p_gap_clear_toggles(p);
		p_set_change(p);
		if (p->cal->my_cal_version >= DATAVER4) {
			v5_gap_clear_pending_change(p);
        		XtSetSensitive(p->gap2_change_button, False);
        		XtSetSensitive(p->gap_remove_button, False);
		}  else
			v4_gap_clear_pending_change(p);
		XtFree((XtPointer)item_list);
	}
	else {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 750, "Calendar : Error - Access List and Permissions"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 408,  "Please select a name first.                   "));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(p->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			DIALOG_IMAGE, p->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
	}
}

extern void
gap_list_select_proc(Widget w, XtPointer client_data, XtPointer data) {
	int		*item_list = NULL, item_cnt = 0;
	Props_pu	*p = (Props_pu *)client_data;
	CSA_access_list	step = p->gap_list;

	if (!XmListGetSelectedPos(p->gap_access_list, &item_list, &item_cnt))
		return;
 
        if (item_cnt <= 0) {
		if (p->cal->my_cal_version >= DATAVER4)
        		XtSetSensitive(p->gap2_change_button, False);
                XtSetSensitive(p->gap_remove_button, False);
        } else {
		if (p->cal->my_cal_version >= DATAVER4)
        		XtSetSensitive(p->gap2_change_button, True);
                XtSetSensitive(p->gap_remove_button, True);
	}

	item_cnt = 1;
	while(step && item_cnt < *item_list) {
		step = step->next;
		++item_cnt;
	}
	XmTextSetString(p->gap_user_text, step->user->user_name);

	/*
 	 * V5DEP: If this is a V5 server w/V1 backing store probably
	 * should use old access flags.
 	 */
	if (p->cal->my_cal_version >= DATAVER4) {
		XmToggleButtonGadgetSetState(p->gap2_public_tog[GAP_VIEW],
			(step->rights & CSA_VIEW_PUBLIC_ENTRIES) ? True : False,
			True);
		XmToggleButtonGadgetSetState(p->gap2_public_tog[GAP_INSERT],
			(step->rights & CSA_INSERT_PUBLIC_ENTRIES) ?
				True : False, True);
		XmToggleButtonGadgetSetState(p->gap2_public_tog[GAP_CHANGE],
			(step->rights & CSA_CHANGE_PUBLIC_ENTRIES) ? 
				True : False, True);
		XmToggleButtonGadgetSetState(p->gap2_semi_tog[GAP_VIEW],
			(step->rights & CSA_VIEW_CONFIDENTIAL_ENTRIES) ?
				True : False, True);
		XmToggleButtonGadgetSetState(p->gap2_semi_tog[GAP_INSERT],
			(step->rights & CSA_INSERT_CONFIDENTIAL_ENTRIES) ?
				True : False, True);
		XmToggleButtonGadgetSetState(p->gap2_semi_tog[GAP_CHANGE],
			(step->rights & CSA_CHANGE_CONFIDENTIAL_ENTRIES) ? 
				True : False, True);
		XmToggleButtonGadgetSetState(p->gap2_private_tog[GAP_VIEW],
			(step->rights & CSA_VIEW_PRIVATE_ENTRIES) ? 
				True : False, True);
		XmToggleButtonGadgetSetState(p->gap2_private_tog[GAP_INSERT],
			(step->rights & CSA_INSERT_PRIVATE_ENTRIES) ?
				True : False, True);
		XmToggleButtonGadgetSetState(p->gap2_private_tog[GAP_CHANGE],
			(step->rights & CSA_CHANGE_PRIVATE_ENTRIES) ? 
				True : False, True);
	} else {
		XmToggleButtonGadgetSetState(p->gap_browse_toggle,
			(step->rights & CSA_X_DT_BROWSE_ACCESS) ? True : False,
			True);
		XmToggleButtonGadgetSetState(p->gap_insert_toggle,
			(step->rights & CSA_X_DT_INSERT_ACCESS) ? True : False,
			True);
		XmToggleButtonGadgetSetState(p->gap_delete_toggle,
			(step->rights & CSA_X_DT_DELETE_ACCESS) ? True : False,
			True);
	}

	XtFree((XtPointer)item_list);
}



static void
p_create_v4_group_access_pane(Props_pu *p) {
	Calendar *c = calendar;
	char	*buf;
	Widget	gap_form_mgr;
	XmString	xmstr;

	/*
        **  Base form
	*/
	gap_form_mgr = XtVaCreateWidget("gap_form_mgr",
		xmFormWidgetClass, p->base_form_mgr,
		XmNautoUnmanage, 	False,
	        XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->separator1,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNrightAttachment, 	XmATTACH_FORM,
                XmNbottomAttachment, 	XmATTACH_WIDGET,
                XmNbottomOffset, 	10,
                XmNbottomWidget, 	p->separator2,
		NULL);
	p->pane_list[GROUP_ACCESS_PANE] = gap_form_mgr;

	/*
	**  User Name label, text field, and add button
	*/
	xmstr = XmStringCreateLocalized(
			catgets(c->DT_catd, 1, 410, "User Name:"));
        p->gap_user_label = XtVaCreateWidget("userName",
		xmLabelGadgetClass, gap_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_FORM,
		XmNtopOffset, 		12,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		5,
                NULL);
	XmStringFree(xmstr);

        p->gap_user_text = XtVaCreateWidget("gap_user_text",
		xmTextWidgetClass, gap_form_mgr,
		XmNcolumns, 		NAME_LEN,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	p->gap_user_label,
		XmNbottomOffset, 	-4,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->gap_user_label,
		XmNleftOffset, 		10,
                NULL);

	XtAddCallback(p->gap_user_text, 
			XmNvalueChangedCallback, v4_gap_pending_change, 
			(XtPointer)p);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 411, "Add Name"));
        p->gap_add_button = XtVaCreateWidget("addName",
		xmPushButtonGadgetClass, gap_form_mgr,
		XmNlabelString, 	xmstr,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	p->gap_user_label,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->gap_user_text,
		XmNleftOffset, 		10,
		XmNnavigationType, 	XmTAB_GROUP,
		XmNsensitive, 		False,
                NULL);
	XmStringFree(xmstr);
	XtAddCallback(p->gap_add_button, 
			XmNactivateCallback, p_gap_add_proc, p);

	/*
	**  Access list stuff and remove button
	*/
	{
	  char *user = XtNewString(catgets(c->DT_catd, 1, 983, "User Name"));
	  char *perm = XtNewString(catgets(c->DT_catd, 1, 413, "Permissions"));
	  buf = (char *)calloc(1, cm_strlen(user) + cm_strlen(perm) + NAME_LEN);
	  sprintf(buf, "%-*s%s", NAME_LEN, user, perm);
	  xmstr = XmStringCreateLocalized(buf);
	  XtFree(perm);
	  XtFree(user);
	}

        p->gap_access_header = XtVaCreateManagedWidget("gap_access_header",
		xmLabelGadgetClass, gap_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->gap_user_label,
		XmNtopOffset, 		10,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		5,
                NULL);
	XmStringFree(xmstr);
	free(buf);

	p->gap_list = NULL;
	p->gap_access_list = (Widget)XmCreateScrolledList(gap_form_mgr,
		"gap_access_list", NULL, 0);
	p->gap_access_list_sw = XtParent(p->gap_access_list);
        XtVaSetValues(p->gap_access_list_sw,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->gap_access_header,
		XmNtopOffset, 		5,
		XmNleftAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNleftWidget, 		p->gap_access_header,
		XmNrightAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNrightWidget, 	p->gap_user_text,
		NULL);
	XtVaSetValues(p->gap_access_list,
                XmNvisibleItemCount, 	10,
		XmNdoubleClickInterval, 5,
                NULL);
	XtAddCallback(p->gap_access_list,
		XmNbrowseSelectionCallback, gap_list_select_proc, p);
	XtManageChild(p->gap_access_list);

	xmstr = XmStringCreateLocalized(
			catgets(c->DT_catd, 1, 414, "Remove Name"));
        p->gap_remove_button = XtVaCreateWidget("removeName",
		xmPushButtonGadgetClass, gap_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNtopWidget, 		p->gap_access_list_sw,
		XmNleftAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNleftWidget, 		p->gap_add_button,
		XmNnavigationType, 	XmTAB_GROUP,
		XmNsensitive, 		False,
                NULL);
	XmStringFree(xmstr);
	XtAddCallback(p->gap_remove_button,
		XmNactivateCallback, p_gap_remove_proc, p);

	xmstr = XmStringCreateLocalized(
	       catgets(c->DT_catd, 1, 415, "B = Browse I = Insert D = Delete"));
        p->gap_access_msg = XtVaCreateWidget("browseInsertDelete",
		xmLabelGadgetClass, gap_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->gap_access_list_sw,
		XmNtopOffset, 		5,
		XmNleftAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNleftWidget, 		p->gap_access_list_sw,
                NULL);
	XmStringFree(xmstr);

	/*
	**  Finally, permissions label and choice toggle list
	*/
	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 416, "Permissions:"));
        p->gap_perm_label = XtVaCreateWidget("permissions",
		xmLabelGadgetClass, gap_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->gap_access_msg,
		XmNtopOffset, 		12,
		XmNleftAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNleftWidget, 		p->gap_access_msg,
                NULL);
	XmStringFree(xmstr);

	p->gap_perm_rc_mgr = XtVaCreateWidget("gap_perm_rc_mgr",
		xmRowColumnWidgetClass, gap_form_mgr,
		XmNpacking, 		XmPACK_COLUMN,
		XmNorientation, 	XmHORIZONTAL,
		XmNradioBehavior, 	False,
		XmNisHomogeneous, 	True,
		XmNindicatorType, 	XmN_OF_MANY,
		XmNentryClass, 		xmToggleButtonGadgetClass,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	p->gap_perm_label,
		XmNbottomOffset, 	-5,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->gap_perm_label,
		XmNleftOffset, 		5,
		NULL);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 417, "Browse"));
        p->gap_browse_toggle = XtVaCreateWidget("browse", 
		xmToggleButtonGadgetClass, p->gap_perm_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);
	
	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 342, "Insert"));
        p->gap_insert_toggle = XtVaCreateWidget("insert", 
		xmToggleButtonGadgetClass, p->gap_perm_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);
	
	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 419, "Delete"));
        p->gap_delete_toggle = XtVaCreateWidget("delete", 
		xmToggleButtonGadgetClass, p->gap_perm_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	ManageChildren(p->gap_perm_rc_mgr);
	ManageChildren(gap_form_mgr);
}

/*
**  Functions related to the printer ops pane
*/
static void
p_dest_toggle_proc(Widget w, XtPointer client_data, XtPointer data) {
	Props_pu	*p = (Props_pu *)client_data;

	if (XmToggleButtonGadgetGetState(p->pop_dest_print_toggle)) {
		XtUnmanageChild(p->pop_file_label);
		XtUnmanageChild(p->pop_file_text);
		XtUnmanageChild(p->pop_directory_label);
		XtUnmanageChild(p->pop_directory_text);
		XtManageChild(p->pop_printer_text);
		XtManageChild(p->pop_printer_label);
		XtManageChild(p->pop_options_text);
		XtManageChild(p->pop_options_label);
		XtVaSetValues(p->pop_width_text,
			XmNtopWidget, 		p->pop_options_text,
			NULL);
	} else {
		XtUnmanageChild(p->pop_options_label);
		XtUnmanageChild(p->pop_options_text);
		XtUnmanageChild(p->pop_printer_label);
		XtUnmanageChild(p->pop_printer_text);
		XtManageChild(p->pop_directory_text);
		XtManageChild(p->pop_directory_label);
		XtManageChild(p->pop_file_text);
		XtManageChild(p->pop_file_label);
		XtVaSetValues(p->pop_width_text,
			XmNtopWidget, 		p->pop_file_text,
			NULL);
	}
	p_set_change(p);
}


static Boolean
all_digits(char *string) {

	while (*string) {
		if (!isdigit(*string++))
			return(False);
	}

	return(True);
}

static void
p_copies_spin_callback(Widget w, XtPointer data, XtPointer cbs) {

	Props_pu 	*p = (Props_pu *) data;
	int		value = atoi(XmTextGetString(w));
	Display         *dpy = XtDisplayOfObject(w);


	if (all_digits(XmTextGetString(w)) == False) {
                value = 1;
                XBell(dpy, 50);
        }
	else if ((value < 1 ) || (value > 99)) {
		value = 1;
		XBell(dpy, 50);
	}

	XtVaSetValues(p->pop_copies_spin, DtNposition, value, NULL);
}

static void
p_units_spin_callback(Widget w, XtPointer data, XtPointer cbs) {

	Props_pu 	*p = (Props_pu *) data;
	int		value = atoi(XmTextGetString(w));
	Display         *dpy = XtDisplayOfObject(w);

	if (all_digits(XmTextGetString(w)) == False) {
                value = 1;
                XBell(dpy, 50);
        }
	else if ((value < 1 ) || (value > 99)) {
		value = 1;
		XBell(dpy, 50);
	}

	XtVaSetValues(p->pop_units_spin, DtNposition, value, NULL);
}

static void
p_create_printer_ops_pane(
	Props_pu 	*p)
 {
	Calendar 	*c = calendar;
	Widget		 pop_form_mgr,
			 widgets[25];
	WidgetList	 children;
	int		 i = 0,
			 j = 0,
			 n;
	XmString	 xmstr;
	Widget		 text_field;
	Dimension	 max_left_label_width,
			 width1, width2;

	/*
        **  Base form
	*/
	pop_form_mgr = XtVaCreateWidget("pop_form_mgr",
		xmFormWidgetClass, p->base_form_mgr,
		XmNautoUnmanage, 	False,
	        XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->separator1,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNrightAttachment, 	XmATTACH_FORM,
		XmNbottomAttachment, 	XmATTACH_WIDGET,
		XmNbottomOffset, 	10,
		XmNbottomWidget, 	p->separator2,
		NULL);
	p->pane_list[PRINTER_OPS_PANE] = pop_form_mgr;

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 420, "Destination:"));
       	p->pop_dest_label = XtVaCreateWidget("destination",
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 423, "Printer:"));
        p->pop_printer_label = XtVaCreateWidget("printer",
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 424, "Directory:"));
        p->pop_directory_label = XtVaCreateWidget("directory",
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 425, "Options:"));
        p->pop_options_label = XtVaCreateWidget("options",
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 426, "File:"));
        p->pop_file_label = XtVaCreateWidget("file",
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 427, "Width:"));
        p->pop_width_label = XtVaCreateWidget("width",
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 429, "Position:"));
        p->pop_pos_label = XtVaCreateWidget("position",
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 432, "Units:"));
        p->pop_units_label = XtVaCreateWidget("units",
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 434, "Include:"));
        p->pop_privacy_label = XtVaCreateWidget("include", 
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	/*
	 * Find the widest label to be used as an offset.
	 */
	width1 = ComputeMaxWidth(p->pop_dest_label,
				 p->pop_printer_label,
				 p->pop_directory_label,
				 p->pop_options_label);
	width2 = ComputeMaxWidth(p->pop_file_label,
				 p->pop_width_label,
				 p->pop_pos_label,
				 p->pop_units_label);

	if (width2 > width1) width1 = width2;

	width2 = ComputeMaxWidth(p->pop_privacy_label, NULL, NULL, NULL);

	if (width2 > width1)
		max_left_label_width = width2 + 2 * GAP;
	else
		max_left_label_width = width1 + 2 * GAP;
						

        p->pop_dest_rc_mgr = XtVaCreateWidget("pop_dest_rc_mgr",
		xmRowColumnWidgetClass, pop_form_mgr,
		XmNpacking, 		XmPACK_COLUMN,
		XmNorientation, 	XmHORIZONTAL,
		XmNradioBehavior, 	True,
		XmNisHomogeneous, 	True,
		XmNentryClass, 		xmToggleButtonGadgetClass,
		XmNtopAttachment, 	XmATTACH_FORM,
		XmNtopOffset, 		GAP,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		max_left_label_width,
		NULL);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 421, "Printer"));
        p->pop_dest_print_toggle = XtVaCreateWidget("printer",
                xmToggleButtonGadgetClass, p->pop_dest_rc_mgr,
		XmNlabelString, 	xmstr,
		XmNset, 		True,
                NULL);
	XmStringFree(xmstr);
	XtAddCallback(p->pop_dest_print_toggle,
		XmNvalueChangedCallback, p_dest_toggle_proc, p);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 422, "File"));
        p->pop_dest_file_toggle = XtVaCreateWidget("file",
                xmToggleButtonGadgetClass, p->pop_dest_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);
	XtAddCallback(p->pop_dest_file_toggle,
		XmNvalueChangedCallback, p_dest_toggle_proc, p);

	ManageChildren(p->pop_dest_rc_mgr);

        XtVaSetValues(p->pop_dest_label,
                XmNbottomAttachment,    XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget,        p->pop_dest_rc_mgr,
                XmNbottomOffset,        GAP,
                XmNrightAttachment,     XmATTACH_WIDGET,
                XmNrightWidget,         p->pop_dest_rc_mgr,
                XmNrightOffset,         GAP,
                NULL);

	/*
	**  Printer/directory label and text field
	*/
        p->pop_printer_text = XtVaCreateWidget("pop_printer_text",
		xmTextWidgetClass, 	pop_form_mgr,
                XmNcolumns, 		20,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->pop_dest_rc_mgr,
		XmNtopOffset, 		GAP,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		max_left_label_width,
                NULL);

	XtAddCallback(p->pop_printer_text, 
				XmNvalueChangedCallback, p_mark_change, p);

        XtVaSetValues(p->pop_printer_label,
                XmNbottomAttachment,    XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget,        p->pop_printer_text,
                XmNbottomOffset,        GAP,
                XmNrightAttachment,     XmATTACH_WIDGET,
                XmNrightWidget,         p->pop_printer_text,
                XmNrightOffset,         GAP,
                NULL);

        p->pop_directory_text = XtVaCreateWidget("pop_directory_text",
		xmTextWidgetClass, pop_form_mgr,
                XmNcolumns, 		20,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->pop_dest_rc_mgr,
		XmNtopOffset, 		GAP,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		max_left_label_width,
                NULL);

	XtAddCallback(p->pop_directory_text, 
				XmNvalueChangedCallback, p_mark_change, p);

        XtVaSetValues(p->pop_directory_label,
                XmNbottomAttachment,    XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget,        p->pop_directory_text,
                XmNbottomOffset,        GAP,
                XmNrightAttachment,     XmATTACH_WIDGET,
                XmNrightWidget,         p->pop_directory_text,
                XmNrightOffset,         GAP,
                NULL);

	/*
	**  Options/file label and text field
	*/
        p->pop_options_text = XtVaCreateWidget("pop_options_text",
		xmTextWidgetClass, pop_form_mgr,
                XmNcolumns, 		20,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->pop_printer_text,
		XmNtopOffset, 		GAP,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		max_left_label_width,
                NULL);

	XtAddCallback(p->pop_options_text, 
				XmNvalueChangedCallback, p_mark_change, p);

        XtVaSetValues(p->pop_options_label,
                XmNbottomAttachment,    XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget,        p->pop_options_text,
                XmNbottomOffset,        GAP,
                XmNrightAttachment,     XmATTACH_WIDGET,
                XmNrightWidget,         p->pop_options_text,
                XmNrightOffset,         GAP,
                NULL);

        p->pop_file_text = XtVaCreateWidget("pop_file_text",
		xmTextWidgetClass, pop_form_mgr,
                XmNcolumns, 		20,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->pop_directory_text,
		XmNtopOffset, 		GAP,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		max_left_label_width,
                NULL);

	XtAddCallback(p->pop_file_text, 
				XmNvalueChangedCallback, p_mark_change, p);

        XtVaSetValues(p->pop_file_label,
                XmNbottomAttachment,    XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget,        p->pop_file_text,
                XmNbottomOffset,        GAP,
                XmNrightAttachment,     XmATTACH_WIDGET,
                XmNrightWidget,         p->pop_file_text,
                XmNrightOffset,         GAP,
                NULL);

	/*
	**  Width and height labels and text fields
	*/
        p->pop_width_text = XtVaCreateWidget("pop_width_text",
		xmTextWidgetClass, pop_form_mgr,
                XmNcolumns, 		10,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->pop_options_text,
		XmNtopOffset, 		GAP,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		max_left_label_width,
                NULL);

	XtAddCallback(p->pop_width_text, 
				XmNvalueChangedCallback, p_mark_change, p);

        XtVaSetValues(p->pop_width_label,
                XmNbottomAttachment,    XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget,        p->pop_width_text,
                XmNbottomOffset,        GAP,
                XmNrightAttachment,     XmATTACH_WIDGET,
                XmNrightWidget,         p->pop_width_text,
                XmNrightOffset,         GAP,
                NULL);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 428, "Height:"));
        p->pop_height_label = XtVaCreateWidget("height",
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNtopWidget, 		p->pop_width_label,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->pop_width_text,
		XmNleftOffset, 		GAP,
                NULL);
	XmStringFree(xmstr);

        p->pop_height_text = XtVaCreateWidget("pop_height_text",
		xmTextWidgetClass, pop_form_mgr,
                XmNcolumns, 		10,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	p->pop_width_text,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->pop_height_label,
		XmNleftOffset, 		GAP,
                NULL);

	XtAddCallback(p->pop_height_text, 
				XmNvalueChangedCallback, p_mark_change, p);

	/*
	**  Position label and text fields
	*/
        p->pop_pos_left_text = XtVaCreateWidget("pop_pos_left_text",
		xmTextWidgetClass, pop_form_mgr,
                XmNcolumns, 		10,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->pop_width_text,
		XmNtopOffset, 		GAP,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		max_left_label_width,
                NULL);

	XtAddCallback(p->pop_pos_left_text, 
				XmNvalueChangedCallback, p_mark_change, p);

	xmstr = XmStringCreateLocalized(
			catgets(c->DT_catd, 1, 430, "Inches from left"));
        p->pop_pos_left_label = XtVaCreateWidget("inchesFromLeft",
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNtopWidget, 		p->pop_pos_left_text,
		XmNleftAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNleftWidget, 		p->pop_height_label,
                NULL);
	XmStringFree(xmstr);

        XtVaSetValues(p->pop_pos_label,
                XmNbottomAttachment,    XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget,        p->pop_pos_left_text,
                XmNbottomOffset,        GAP,
                XmNrightAttachment,     XmATTACH_WIDGET,
                XmNrightWidget,         p->pop_pos_left_text,
                XmNrightOffset,         GAP,
                NULL);

        p->pop_pos_bottom_text = XtVaCreateWidget("pop_pos_bottom_text",
		xmTextWidgetClass, pop_form_mgr,
                XmNcolumns, 		10,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->pop_pos_left_text,
		XmNtopOffset, 		GAP,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		max_left_label_width,
                NULL);

	XtAddCallback(p->pop_pos_bottom_text, 
				XmNvalueChangedCallback, p_mark_change, p);

	xmstr = XmStringCreateLocalized(
			catgets(c->DT_catd, 1, 431, "Inches from bottom"));
        p->pop_pos_bottom_label = XtVaCreateWidget("inchesFromBottom",
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNtopWidget, 		p->pop_pos_bottom_text,
		XmNleftAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNleftWidget, 		p->pop_height_label,
                NULL);
	XmStringFree(xmstr);

	/*
	**  Units and copies labels and text fields
	*/
   	p->pop_units_spin = DtCreateSpinBox(pop_form_mgr, "unitsSpin", NULL, 0);
	XtVaSetValues(p->pop_units_spin,
		DtNminimumValue, 	1,
		DtNposition, 		1,
		DtNmaximumValue, 	99,
		DtNspinBoxChildType, 	DtNUMERIC,
		DtNmaxLength, 		6,
		DtNeditable, 		True,
		DtNwrap, 		True,
		DtNcolumns, 		2,
		XmNshadowThickness, 	0,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->pop_pos_bottom_text,
		XmNtopOffset, 		GAP,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		max_left_label_width - 2,
		NULL);

	XtVaGetValues(p->pop_units_spin, DtNtextField, &text_field, NULL);
	XtAddCallback(text_field, 
			XmNvalueChangedCallback, p_units_spin_callback, p);

        XtVaSetValues(p->pop_units_label,
                XmNbottomAttachment,    XmATTACH_OPPOSITE_WIDGET,
                XmNbottomWidget,        p->pop_units_spin,
                XmNbottomOffset,        GAP,
                XmNrightAttachment,     XmATTACH_WIDGET,
                XmNrightWidget,         p->pop_units_spin,
                XmNrightOffset,         GAP,
                NULL);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 433, "Copies:"));
        p->pop_copies_label = XtVaCreateWidget("copies",
		xmLabelGadgetClass, pop_form_mgr,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNtopWidget, 		p->pop_units_label,
		XmNrightAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNrightWidget, 	p->pop_height_label,
                NULL);
	XmStringFree(xmstr);

   	p->pop_copies_spin = DtCreateSpinBox(pop_form_mgr, "copiesSpin", 
								NULL, 0);
	XtVaSetValues(p->pop_copies_spin,
		DtNminimumValue, 	1,
		DtNposition, 		1,
		DtNmaximumValue, 	99,
		DtNspinBoxChildType, 	DtNUMERIC,
		DtNmaxLength, 		2,
		DtNeditable, 		True,
		DtNwrap, 		True,
		DtNcolumns, 		2,
		XmNshadowThickness, 	0,
		XmNtopAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNtopWidget, 		p->pop_units_spin,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		p->pop_copies_label,
		XmNleftOffset, 		GAP,
		NULL);

	XtAddCallback(p->pop_copies_spin, 
				XmNvalueChangedCallback, p_mark_change, p);

	XtVaGetValues(p->pop_copies_spin, DtNtextField, &text_field, NULL);

	XtAddCallback(text_field, XmNvalueChangedCallback, 
				p_copies_spin_callback, p);
	XtAddCallback(text_field, XmNvalueChangedCallback, p_mark_change, p);

	XtAddCallback(p->pop_units_spin, 
				XmNvalueChangedCallback, p_mark_change, p);
	XtVaGetValues(p->pop_units_spin, DtNtextField, &text_field, NULL);
	XtAddCallback(text_field, XmNvalueChangedCallback, p_mark_change, p);

	/*
	**  Include label and toggle choice
	*/
	p->pop_privacy_rc_mgr = XtVaCreateWidget("pop_privacy_rc_mgr",
                xmRowColumnWidgetClass, pop_form_mgr,
                XmNpacking, 		XmPACK_COLUMN,
                XmNorientation, 	XmVERTICAL,
                XmNradioBehavior, 	False,
                XmNisHomogeneous, 	True,
		XmNindicatorType, 	XmN_OF_MANY,
                XmNentryClass, 		xmToggleButtonGadgetClass,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->pop_units_spin,
		XmNtopOffset, 		GAP,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		max_left_label_width - 7,
                NULL);
 
	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 435, 
						"Show Time and Text Appts"));
        p->pop_privacy_public_toggle = XtVaCreateWidget(
		"showTimeText",
                xmToggleButtonGadgetClass, p->pop_privacy_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(p->pop_privacy_public_toggle, 
				XmNvalueChangedCallback, p_mark_change, p);
 
	xmstr = XmStringCreateLocalized( catgets(c->DT_catd, 1, 436, 
						"Show Time Only Appts"));
        p->pop_privacy_semi_toggle = XtVaCreateWidget(
		"showTimeOnly",
                xmToggleButtonGadgetClass, p->pop_privacy_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(p->pop_privacy_semi_toggle, 
				XmNvalueChangedCallback, p_mark_change, p);
 
	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 437, 
						"Show Nothing Appts"));
        p->pop_privacy_private_toggle = XtVaCreateWidget(
		"showNothing",
                xmToggleButtonGadgetClass, p->pop_privacy_rc_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(p->pop_privacy_private_toggle, 
				XmNvalueChangedCallback, p_mark_change, p);
 
        ManageChildren(p->pop_privacy_rc_mgr);

        XtVaSetValues(p->pop_privacy_label,
                XmNtopAttachment,	XmATTACH_OPPOSITE_WIDGET,
                XmNtopWidget,		p->pop_privacy_rc_mgr,
                XmNtopOffset,		GAP,
                XmNrightAttachment,     XmATTACH_OPPOSITE_WIDGET,
                XmNrightWidget,         p->pop_units_label,
                NULL);

        XtVaGetValues(pop_form_mgr,
                XmNchildren,            &children,
                XmNnumChildren,         &n,
                NULL);
        /* We don't want to manage some of the widgets */
        for (i = 0; i < n; i++) {
                if ((children[i] == p->pop_directory_label) ||
                    (children[i] == p->pop_file_label) ||
                    (children[i] == p->pop_directory_text) ||
                    (children[i] == p->pop_file_text))
                    continue;
                widgets[j++] = children[i];
        }
        XtManageChildren(widgets, n - 4);
}

/*
**  Functions related to the date format pane
*/
static void
p_create_date_format_pane(
	Props_pu 	*p) 
{
	Calendar 	*c = calendar;
	Widget		 dfp_form_mgr;
	XmString	 xmstr;
	Dimension	 max_left_label_width;

	/*
        **  Base form
	*/
	dfp_form_mgr = XtVaCreateWidget("dfp_form_mgr",
		xmFormWidgetClass, p->base_form_mgr,
		XmNautoUnmanage, 	False,
	        XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->separator1,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNrightAttachment, 	XmATTACH_FORM,
                XmNbottomAttachment, 	XmATTACH_WIDGET,
                XmNbottomOffset, 	10,
                XmNbottomWidget, 	p->separator2,
		NULL);
	p->pane_list[DATE_FORMAT_PANE] = dfp_form_mgr;

	/*
	**  Date ordering label and choice toggle widget
	*/
	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 438, "Date Ordering:"));
       	p->dfp_order_label = XtVaCreateWidget("dateOrdering",
		xmLabelGadgetClass, dfp_form_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 442, "Date Separator:"));
       	p->dfp_sep_label = XtVaCreateWidget("dateSeparator",
		xmLabelGadgetClass, dfp_form_mgr,
		XmNlabelString, 	xmstr,
                NULL);
	XmStringFree(xmstr);

	max_left_label_width = ComputeMaxWidth(p->dfp_order_label, 
					       p->dfp_sep_label,
					       NULL, NULL) + 2 * GAP;

        p->dfp_order_rc_mgr = XtVaCreateWidget("dfp_order_rc_mgr",
		xmRowColumnWidgetClass, dfp_form_mgr,
		XmNpacking, 		XmPACK_COLUMN,
		XmNorientation, 	XmVERTICAL,
		XmNradioBehavior, 	True,
		XmNisHomogeneous, 	True,
		XmNentryClass, 		xmToggleButtonGadgetClass,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		max_left_label_width,
		XmNtopAttachment, 	XmATTACH_FORM,
		XmNtopOffset, 		GAP,
		NULL);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 439, "MM | DD | YYYY"));
        p->dfp_order_mmddyy_toggle = XtVaCreateWidget("mmddyy",
                xmToggleButtonGadgetClass, p->dfp_order_rc_mgr,
		XmNlabelString, xmstr,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(p->dfp_order_mmddyy_toggle, 
				XmNvalueChangedCallback, p_mark_change, p);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 440, "DD | MM | YYYY"));
        p->dfp_order_ddmmyy_toggle = XtVaCreateWidget("ddmmyy",
                xmToggleButtonGadgetClass, p->dfp_order_rc_mgr,
		XmNlabelString, xmstr,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(p->dfp_order_ddmmyy_toggle, 
				XmNvalueChangedCallback, p_mark_change, p);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 441, "YYYY | MM | DD"));
        p->dfp_order_yymmdd_toggle = XtVaCreateWidget("yymmdd",
                xmToggleButtonGadgetClass, p->dfp_order_rc_mgr,
		XmNlabelString, xmstr,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(p->dfp_order_yymmdd_toggle, 
				XmNvalueChangedCallback, p_mark_change, p);

        ManageChildren(p->dfp_order_rc_mgr);

	/*
	**  Date separator label and choice toggle widget
	*/
        p->dfp_sep_rc_mgr = XtVaCreateWidget("dfp_sep_rc_mgr",
		xmRowColumnWidgetClass, dfp_form_mgr,
		XmNpacking, 		XmPACK_COLUMN,
		XmNorientation, 	XmVERTICAL,
		XmNradioBehavior, 	True,
		XmNisHomogeneous, 	True,
		XmNentryClass, 		xmToggleButtonGadgetClass,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		max_left_label_width,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		p->dfp_order_rc_mgr,
		XmNtopOffset, 		GAP,
		NULL);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 443, "Blank"));
        p->dfp_sep_blank_toggle = XtVaCreateWidget("blank",
                xmToggleButtonGadgetClass, p->dfp_sep_rc_mgr,
		XmNlabelString, xmstr,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(p->dfp_sep_blank_toggle, 
				XmNvalueChangedCallback, p_mark_change, p);

	xmstr = XmStringCreateLocalized("/");
        p->dfp_sep_slash_toggle = XtVaCreateWidget("oblique",
                xmToggleButtonGadgetClass, p->dfp_sep_rc_mgr,
		XmNlabelString, xmstr,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(p->dfp_sep_slash_toggle, 
				XmNvalueChangedCallback, p_mark_change, p);

	xmstr = XmStringCreateLocalized(".");
        p->dfp_sep_period_toggle = XtVaCreateWidget("period",
                xmToggleButtonGadgetClass, p->dfp_sep_rc_mgr,
		XmNlabelString, xmstr,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(p->dfp_sep_period_toggle, 
				XmNvalueChangedCallback, p_mark_change, p);

	xmstr = XmStringCreateLocalized("-");
        p->dfp_sep_dash_toggle = XtVaCreateWidget("hyphen",
                xmToggleButtonGadgetClass, p->dfp_sep_rc_mgr,
		XmNlabelString, xmstr,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(p->dfp_sep_dash_toggle, 
				XmNvalueChangedCallback, p_mark_change, p);

        XtVaSetValues(p->dfp_order_label,
                XmNtopAttachment,    	XmATTACH_OPPOSITE_WIDGET,
                XmNtopWidget,         	p->dfp_order_rc_mgr,
                XmNtopOffset,		GAP,
                XmNrightAttachment,     XmATTACH_WIDGET,
                XmNrightWidget,         p->dfp_order_rc_mgr,
                XmNrightOffset,         GAP,
                NULL);

        XtVaSetValues(p->dfp_sep_label,
                XmNtopAttachment,    	XmATTACH_OPPOSITE_WIDGET,
                XmNtopWidget,         	p->dfp_sep_rc_mgr,
                XmNtopOffset,		GAP,
                XmNrightAttachment,     XmATTACH_WIDGET,
                XmNrightWidget,         p->dfp_sep_rc_mgr,
                XmNrightOffset,         GAP,
                NULL);

        ManageChildren(p->dfp_sep_rc_mgr);
        ManageChildren(dfp_form_mgr);
}

/*
**  Callback from the category option menu
*/
static void
p_category_notify(Widget w, XtPointer client_data, XtPointer call_data) {
	Widget		base_form_mgr = XtParent(XtParent(XtParent(w)));
	PaneType	item_no = (PaneType)client_data;
	Props_pu	*p;
	Calendar	*c;
	Dimension	label_height, slider_height;

	/*
	**  Note this cheesy hack.  The convenience functions allow you NO
	**  method which which to attach user data to the buttons on menus ...
	**  so the user data on the base form was set to contain a pointer to
	**  the Props_pu structure and we go up through the heirarchical levels
	**  using the widget parameter to retrieve this pointer.
	**
	**  This sucks, but Motif sucks worse and I could find no other method.
	*/
	XtVaGetValues(base_form_mgr, XmNuserData, &p,
		NULL);
	if (!p)
		return;

	/* determine if there are uncommited changes in the property 
	   sheet that need to be flushed. */

	c = p->cal;
	if (p_flush_changes(c) == False) {
        	XtVaSetValues(XmOptionButtonGadget(p->category_menu), 
			      XmNlabelString, Pane_XmStrs[p->last_props_pane],
			      NULL);

		return;
	}

	/*
	**  Performance optimization - don't create each panel until it's
	**  asked for (except for the editor pane which is the first one ...).
	**
	**  Motif, however, is not liking this optimization (bug in toolkit? I
	**  would guess yes ...).  It's not resizing the pop-up correctly and
	**  it's leaving a number of fields blank ... it doesn't do this if the
	**  panes are all created before the window is poped up ... so all the
	**  panes are created before and this "if" statement will never evaluate
	**  to true.  When (or if) this is fixed in the toolkit, remove the
	**  create statements from the p_make_props_pu function.
	*/
	if (!p->pane_list[item_no]) {
		switch(item_no) {
		case DISPLAY_PANE:
			p_create_display_pane(p);
			break;
		case GROUP_ACCESS_PANE:
			if (p->cal->my_cal_version < DATAVER4) 
				p_create_v4_group_access_pane(p);
			else
				p_create_v5_group_access_pane(p);
			break;
		case PRINTER_OPS_PANE:
			p_create_printer_ops_pane(p);
			break;
		case DATE_FORMAT_PANE:
			p_create_date_format_pane(p);
			break;
		case EDITOR_PANE:
		default:
			break;
		}
	}
	if (!p->pane_list[item_no])
		return;

	XtUnmanageChild(p->pane_list[p->last_props_pane]);
	XtVaSetValues(p->separator2,
		XmNtopWidget, p->pane_list[item_no],
		NULL);
	XtManageChild(p->pane_list[item_no]);
	p->last_props_pane = item_no;


	/* code for display settings */

	XtVaGetValues(p->dp_beg_label, XmNheight, &label_height, NULL);
	XtVaGetValues(p->dp_beg_scale, XmNheight, &slider_height, NULL);
	XtVaSetValues(p->dp_beg_scale, XmNbottomOffset, ((int) label_height - (int) slider_height)/2, NULL);
	XtVaSetValues(p->dp_end_scale, XmNbottomOffset, ((int) label_height - (int) slider_height)/2, NULL);


	/* Center Category menu */
	p_gap_center_cat_menu(p->pane_list[p->last_props_pane],
			      p->category_menu);
}

extern void
set_editor_defaults_later(XtPointer client_data, XtIntervalId *interval_id) {
	Calendar	*c = (Calendar *)client_data;

	if (((Editor *) c->editor)->base_form_mgr) {
		Editor *e = (Editor *) c->editor;
	
		set_editor_defaults(e, 0, 0, False);
	}

	if (((GEditor *) c->geditor)->base_form_mgr) {
		GEditor *ge = (GEditor *) c->geditor;
	
		set_geditor_defaults(ge, 0, 0);
	}

	if (((ToDo *) c->todo)->base_form_mgr) {
		ToDo *t = (ToDo *)c->todo;

		set_todo_defaults(t);
	}
}

static Boolean
p_save_changes(Calendar *c) {

	Props		*p = (Props *)c->properties;
	Props_pu	*pu = (Props_pu *)c->properties_pu;
	int		redisplay_mask;
	int		val;
	Tick		current_mb_date = 0;
	char		*date = NULL;
	OrderingType 	ot;
        SeparatorType 	st;
	Browser 	*b = (Browser*)c->browser;
	char 		buf[BUFSIZ];

	if (b) {
        	ot = get_int_prop(p, CP_DATEORDERING);
        	st = get_int_prop(p, CP_DATESEPARATOR);
 
		date = get_date_from_widget(c->view->date, b->datetext, ot, st);

		if (date)
			current_mb_date = cm_getdate(date, NULL);
	}

	if (get_props_from_ui(pu, p, &redisplay_mask) == True) {
		return(False);
	}

	if (!save_props(p)) {
		char *title = XtNewString(catgets(c->DT_catd, 1, 454, "Calendar : Error - Options"));
		char *text = XtNewString(catgets(c->DT_catd, 1, 445, "An error occurred writing to ~/.desksetdefaults.\nMake sure you have permission to write to this file."));
		char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			BUTTON_HELP, DESKSET_DEFAULTS_ERROR_HELP,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		return(False);
	}

	_DtTurnOnHourGlass(pu->frame);

	if (redisplay_mask & PROPS_REDO_DISPLAY) {
		Dimension width, height;

		/* If this bit is set then it means that things controlling 
		   the day boundaries, or some detail of date representation 
		   has changed.  All the things that paint get changed here.  
		   This is the main canvas, the multi-browse window, and 
		   the todo and appointment list dialogs. */

		XtVaGetValues(c->canvas, 
				XmNwidth, 	&width, 
				XmNheight, 	&height, 
				NULL);
		cache_dims(c, width, height);

		if (c->view->glance != yearGlance) 
			paint_canvas(c, NULL, RENDER_CLEAR_FIRST);

		if (c->browser)
			mb_refigure_chart(c);

		/* reformat the display values in the property sheet.  
		   The property sheet must exist is we have to refigure 
		   out the hour mode, so we don't need to check whether 
		   it exists or not. */

		XmScaleGetValue(pu->dp_end_scale, &val);
		set_scale_value(pu->dp_end_value, val, pu);
		XmScaleGetValue(pu->dp_beg_scale, &val);
		set_scale_value(pu->dp_beg_value, val, pu);

		if (c->editor && ((Editor *) c->editor)->view_list)
			build_editor_view((Editor *) c->editor, ((Editor *) (c->editor))->view_list_glance, True);

		if (c->todo && ((ToDo *) c->todo)->view_frame)
			build_todo_view((ToDo *) c->todo, ((ToDo *) (c->todo))->view_list_glance, True);


		/* redo date entry in the multi-browser */	

		if (b && current_mb_date) {
        		ot = get_int_prop(p, CP_DATEORDERING);
        		st = get_int_prop(p, CP_DATESEPARATOR);
        		format_tick(b->date, ot, st, buf);
        		XmTextSetString(b->datetext, buf);
		}
	}

        if (redisplay_mask & PROPS_REDO_EDITORS) {

		if (((Editor *) c->editor)->base_form_mgr) {

			Editor *e = (Editor *) c->editor;
			DSSW *dssw = &((Editor *) c->editor)->dssw;

			set_dssw_menus(dssw, p);

			add_all_appt((Editor *) c->editor, NULL);


		}

		if (((GEditor *) c->geditor)->base_form_mgr) {
			GEditor *ge = (GEditor *) c->geditor;
			DSSW *dssw = &((GEditor *) c->geditor)->dssw;

                        set_dssw_menus(dssw, p);

			add_all_gappt(ge);

		}
	

		if (((ToDo *) c->todo)->base_form_mgr) {
			ToDo *t = (ToDo *)c->todo;
			DSSW *dssw = &((ToDo *) c->todo)->dssw;

			set_dssw_menus(dssw, p);

			add_all_todo((ToDo *) c->todo, NULL);
		}

		set_dssw_menus(&pu->ep_dssw, p);

		XtVaSetValues(c->browse_button, XmNsubMenuId, NULL, NULL);
		update_browse_menu_names(c);
		XtVaSetValues(c->browse_button, XmNsubMenuId, c->browse_menu, NULL);
		if (c->browser)
			browser_reset_list(c);
	
		/* This is really hateful.  The deal is that something 
		   about the code that redefines the start and stop time 
		   menus has a timing hole in it.  If I reset the values 
		   for these two menus before the application returns to 
		   the toolkit, the values will get clobbered later.  
		   There seems to be no way to force the toolkit to 
		   complete the restrucutring of the menus immediately, 
		   so I have to add a timer proc to reset the defaults on 
		   the dialog a little bit after returning to the notifier 
		   loop.  Urk.  This really needs to be fixed some day. */

		XtAppAddTimeOut(c->xcontext->app, 50, set_editor_defaults_later, c);

	}

        if (redisplay_mask & PROPS_REDO_PRINT_DIALOG) {
	}

	p_clear_change(pu);

	_DtTurnOffHourGlass(pu->frame);

	return(True);
}

/*
**  Button callback procs
*/
static void
p_apply_proc(Widget w, XtPointer client_data, XtPointer data) {
	Calendar	*c = (Calendar *)client_data;

	p_save_changes(c);
}

static void
p_ok_proc(
	Widget 		 w,
	XtPointer 	 client_data, 
	XtPointer 	 data)
{
	Calendar	*c = (Calendar *)client_data;
	Props_pu	*pu = (Props_pu *)c->properties_pu;

	p_apply_proc(w, client_data, data);
	if (p_save_changes(c) == True)
		XtPopdown(pu->frame);
}

static void
p_defaults_proc(Widget w, XtPointer client_data, XtPointer data) {
	int		start, stop;
	Calendar	*c = (Calendar *)client_data;
	Props		*p = (Props *)c->properties;
	Props_pu	*pu = (Props_pu *)c->properties_pu;

	if (!read_props(p)) {
		char *title = XtNewString(catgets(c->DT_catd, 1, 454, "Calendar : Error - Options"));
		char *text = XtNewString(catgets(c->DT_catd, 1, 448, "An error occurred reading from ~/.desksetdefaults.\nMake sure you have permission to read this file."));
		char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			BUTTON_HELP, DESKSET_DEFAULTS_ERROR_HELP,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
	} else {
		if ((start = get_int_prop(p, CP_DAYBEGIN)) < 0)
			start = 0;
		else if (start > 22)
			start = 22;
		if ((stop = get_int_prop(p, CP_DAYEND)) <= start)
			stop = start + 1;
		else if (stop > 23)
			stop = 23;
		set_int_prop(p, CP_DAYBEGIN, start);
		set_int_prop(p, CP_DAYEND, stop);

		set_props_on_ui(pu, p);
	}
}

static void
p_close_proc(Widget w, XtPointer client_data, XtPointer data) {
	Props_pu *p = (Props_pu *)client_data;

	XtPopdown(p->frame);
}

extern Boolean
p_flush_changes(Calendar *c) {
	Props		*p = (Props *)c->properties;
	Props_pu	*pu = (Props_pu *)c->properties_pu;
	static int	answer;
	int		redisplay_mask;
	int		val;

	if (pu->changed_flag == True) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 450, "Calendar : Options - Help"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 451, "You have made unsaved changes.\nYou may save your changes, discard your changes, \nor return to your previous place in the dialog."));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 452, "Save"));
	  	char *ident2 = XtNewString(catgets(c->DT_catd, 1, 700, "Discard"));
	  	char *ident3 = XtNewString(catgets(c->DT_catd, 1, 923, "Cancel"));
		answer = dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			BUTTON_IDENT, 2, ident2,
			BUTTON_IDENT, 3, ident3,
			DIALOG_IMAGE, pu->xm_warning_pixmap,
			NULL);
		XtFree(ident3);
		XtFree(ident2);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		if (answer == 1) {

			return(p_save_changes(c));
		}
		else if (answer == 2) {
			set_props_on_ui(pu, p);
			return(True);
		}
		else if (answer == 3) {
			return(False);
		}
	}
	else
		return(True);
}

static void
p_quit_handler(Widget w, XtPointer cdata, XtPointer data) {
	Calendar	*c = (Calendar *)cdata;
	Props_pu	*pu = (Props_pu *)c->properties_pu;

	if (p_flush_changes(c) == True)
		XtPopdown(pu->frame);
}

static void
p_make_props_pu(Calendar *c) {
	char		*buf;
	char		*popuplabel = 
			     catgets(c->DT_catd, 1, 458, "Calendar : Options");
	PaneType	pt;
	Props_pu	*p = (Props_pu *)c->properties_pu;
	XmString	label;
	XmString	xmstr;
	Widget		widgets[10];

	/*
	**  An array of panes will be used to "flip" through the different
	**  property choice sheets
	*/
	p->cal = c;
	p->pane_list = (Widget *)XtCalloc(1, 
					sizeof(Widget)*(DATE_FORMAT_PANE +1));

	/*
        **  Create the base frame and form manager
	*/
        buf = (char *)XtMalloc(cm_strlen(popuplabel)
			      + cm_strlen(c->calname) + 4);
        sprintf(buf, "%s - %s", popuplabel, c->calname);

	p->frame = XtVaCreatePopupShell("frame",
		xmDialogShellWidgetClass, c->frame,
		XmNdeleteResponse, XmDO_NOTHING,
		XmNtitle, buf,
		XmNallowShellResize, True,
		NULL);
	free(buf);
	setup_quit_handler(p->frame, p_quit_handler, (caddr_t)c);

	p->base_form_mgr = XtVaCreateWidget("base_form_mgr",
		xmFormWidgetClass, p->frame,
		XmNautoUnmanage, False,
		XmNhorizontalSpacing, 1,
		XmNverticalSpacing, 1,
		XmNuserData, p,
		XmNresizable, False,
		XmNfractionBase, 29,
		XmNtopAttachment, XmATTACH_FORM,
		XmNtopOffset, 0,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNbottomOffset, 0,
		XmNrightAttachment, XmATTACH_FORM,
		XmNrightOffset, 0,
		XmNleftAttachment, XmATTACH_FORM,
		XmNleftOffset, 	0,
		NULL);

	/*
	**  Now the "top" portion - the category menu and the first separator
	*/

	Pane_XmStrs[EDITOR_PANE] = 
	  XmStringCreateLocalized(catgets(c->DT_catd, 1, 762, 
					  "Editor Defaults"));
	Pane_XmStrs[DISPLAY_PANE] = 
	  XmStringCreateLocalized(catgets(c->DT_catd, 1, 763, 
					  "Display Settings"));
	Pane_XmStrs[GROUP_ACCESS_PANE] = 
	  XmStringCreateLocalized(catgets(c->DT_catd, 1, 764, 
					  "Access List and Permissions"));
	Pane_XmStrs[PRINTER_OPS_PANE] = 
	  XmStringCreateLocalized(catgets(c->DT_catd, 1, 765, 
					  "Printer Settings"));
	Pane_XmStrs[DATE_FORMAT_PANE] = 
	  XmStringCreateLocalized(catgets(c->DT_catd, 1, 766, 
					  "Date Format"));

	label = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 459, "Category:"));

	p->category_menu = widgets[0] = 
		XmVaCreateSimpleOptionMenu(p->base_form_mgr,
		"category_menu",  label, NULL, 0, p_category_notify,
		XmVaPUSHBUTTON, Pane_XmStrs[EDITOR_PANE], NULL, NULL, NULL,
		XmVaPUSHBUTTON, Pane_XmStrs[DISPLAY_PANE], NULL, NULL, NULL,
		XmVaPUSHBUTTON, Pane_XmStrs[GROUP_ACCESS_PANE], NULL, NULL,NULL,
		XmVaPUSHBUTTON, Pane_XmStrs[PRINTER_OPS_PANE], NULL, NULL, NULL,
		XmVaPUSHBUTTON, Pane_XmStrs[DATE_FORMAT_PANE], NULL, NULL, NULL,
                XmNtopAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_FORM,
                XmNrightAttachment, XmATTACH_NONE,
		XmNnavigationType, XmTAB_GROUP,
		NULL);
	XmStringFree(label);

        p->separator1 = widgets[1] = XtVaCreateWidget("separator1",
                xmSeparatorGadgetClass, p->base_form_mgr,
                XmNleftAttachment, XmATTACH_FORM,
                XmNrightAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, p->category_menu,
                NULL);

	/*
	**  Now create the "bottom" part - the separator and choice buttons
	*/
	p_create_editor_pane(c);

        p->separator2 = widgets[2] = XtVaCreateWidget("separator2",
                xmSeparatorGadgetClass, p->base_form_mgr,
                XmNleftAttachment, XmATTACH_FORM,
                XmNrightAttachment, XmATTACH_FORM,
                NULL);

	/*
	 * To get the pane to resize instead of the buttons at the bottom
	 * of the option sheet we attach the bottom of the pane to the
	 * separator instead of the other way around.
	 */
	XtVaSetValues(p->pane_list[EDITOR_PANE],
		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomOffset, 10,
		XmNbottomWidget, p->separator2,
		NULL);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 655, "OK"));
	p->ok_button = widgets[3] = XtVaCreateWidget("ok",
		xmPushButtonGadgetClass, p->base_form_mgr,
		XmNlabelString, xmstr,
		XmNrecomputeSize,	False,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNbottomOffset, 7,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 2,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 7,
		XmNrightOffset, 5,
		NULL);
	XmStringFree(xmstr);
	XtAddCallback(p->ok_button, XmNactivateCallback, p_ok_proc, c);

	XtVaSetValues(p->separator2,
		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomOffset, 10,
		XmNbottomWidget, p->ok_button,
		NULL);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 460, "Apply"));
	p->apply_button = widgets[4] = XtVaCreateWidget("apply",
		xmPushButtonGadgetClass, p->base_form_mgr,
		XmNlabelString, xmstr,
		XmNrecomputeSize,	False,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNbottomOffset, 7,
		XmNtopWidget, p->separator2,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 7,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 12,
		XmNrightOffset, 5,
		NULL);
	XmStringFree(xmstr);
	XtAddCallback(p->apply_button, XmNactivateCallback, p_apply_proc, c);
 
	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 691, "Reset"));
	p->defaults_button = widgets[5] = XtVaCreateWidget("reset",
		xmPushButtonGadgetClass, p->base_form_mgr,
		XmNlabelString, xmstr,
		XmNrecomputeSize,	False,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNbottomOffset, 7,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 12,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 17,
		XmNrightOffset, 5,
		NULL);
	XmStringFree(xmstr);
	XtAddCallback(p->defaults_button,
		XmNactivateCallback, p_defaults_proc, c);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 923, "Cancel"));
	p->close_button = widgets[6] = XtVaCreateWidget("close",
		xmPushButtonGadgetClass, p->base_form_mgr,
		XmNlabelString, xmstr,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNbottomOffset, 7,
		XmNtopWidget, p->separator2,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 17,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 22,
		XmNrightOffset, 5,
		NULL);
	XmStringFree(xmstr);
	XtAddCallback(p->close_button, XmNactivateCallback, p_close_proc, p);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 77, "Help"));
	p->help_button = widgets[7] = XtVaCreateWidget("help",
		xmPushButtonGadgetClass, p->base_form_mgr,
		XmNlabelString, xmstr,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNbottomOffset, 7,
		XmNtopWidget, p->separator2,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 22,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 27,
		NULL);
	XmStringFree(xmstr);
	XtAddCallback(p->help_button, XmNactivateCallback,
		(XtCallbackProc)help_cb, OPTIONS_HELP_BUTTON);
        XtAddCallback(p->base_form_mgr, XmNhelpCallback,
                (XtCallbackProc)help_cb, (XtPointer) OPTIONS_HELP_BUTTON);


	/*
	**  Last, create the remaining panes
	*/
	p_create_display_pane(p);
	if (p->cal->my_cal_version < DATAVER4) 
		p_create_v4_group_access_pane(p);
	else
		p_create_v5_group_access_pane(p);

	p_create_printer_ops_pane(p);
	p_create_date_format_pane(p);

	/*
	**  Manage the uppermost children (first pane is Editor Defaults)
	*/
        p->last_props_pane = EDITOR_PANE;
	widgets[8] = p->pane_list[p->last_props_pane];

	/* set default button */
        XtVaSetValues(p->base_form_mgr, XmNcancelButton, p->close_button, NULL);

	XtManageChildren(widgets, 9);
	XtManageChild(p->base_form_mgr);

	/* Center Category menu */
	p_gap_center_cat_menu(p->pane_list[p->last_props_pane],
			      p->category_menu);
}

static Boolean
string_is_number(char *str) {


	if (blank_buf(str)) {
		return(False);
	}

	while (str && *str) {
		if (!isdigit(*str) && *str != ' ' && *str != '\t')
			return(False);

		str++;
	}

	return(True);
}

static Boolean
string_is_time(char *str) {

	Boolean	found_colon = False;
	int	found_first_number = 0;
	int	found_second_number = 0;

	if (blank_buf(str)) {
		return(False);
	}

	/* a time is a pretty structured beast */

	while (*str) {
		if ((*str == ' ') || (*str == '\t')) {
		}
		else if (*str == ':') {
			if (found_colon == True)
				return(False);
			else if (found_first_number == False)
				return(False);
			else
				found_colon = True;
		}
		else if (isdigit(*str)) {
			if (found_colon == True)
				found_second_number++;
			else
				found_first_number++;
		}
		else
			return(False);

		str++;
	}
	
	if ((found_colon == True) && 
	    ((((found_first_number > 2) || (found_first_number < 1)) || (found_second_number != 2))))
		return(False);
	else if ((found_colon == False) && ((found_first_number > 4) || found_first_number < 3))
		return(False);

	return(True);
}


/*******************************************************************************
**
**  Functions external to props_pu.c
**
*******************************************************************************/
extern Boolean
get_editor_vals_from_ui(Props_pu *pu, Props *p) {
	int		i, hr, min;
	char		buf[5];
	Widget		text;
	DisplayType	dt = get_int_prop(p, CP_DEFAULTDISP);
	Reminders_val	*val;
	char		*msg, *dur_txt;
	Reminder_val_op status;
	Calendar	*c = pu->cal;

	XtVaGetValues(pu->ep_dur_cb, DtNtextField, &text, NULL);
	dur_txt = XmTextGetString(text);

	if (string_is_number(dur_txt) == False) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 1079, "Calendar : Error - Editor Settings"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 819, "The Duration value must be an integer with no sign.\nYour properties settings have not been saved."));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));

		XtFree(dur_txt);
		dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			BUTTON_HELP, DURATION_ERROR_HELP,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		return(True);
	}

	if (atoi(dur_txt) > (60 *24)) {
		char *title = XtNewString(catgets(c->DT_catd, 1, 1079, "Calendar : Error - Editor Settings"));
		char *text = XtNewString(catgets(c->DT_catd, 1, 916, "The Duration value must be less than 24 hours.\nYour properties settings have not been saved."));
		char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));

		XtFree(dur_txt);
		dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			BUTTON_HELP, DURATION_ERROR_HELP,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		return(True);
	}

	get_dssw_times(&pu->ep_dssw);

	if (string_is_time(pu->ep_dssw.start_val.val) == False) {
		char *title = XtNewString(catgets(c->DT_catd, 1, 1079, "Calendar : Error - Editor Settings"));
		char *text = XtNewString(catgets(c->DT_catd, 1, 820, "The start value must be a valid time.\nYour properties settings have not been saved."));
		char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			BUTTON_HELP, START_ERROR_HELP,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		return(True);
	}


	strncpy(buf, pu->ep_dssw.start_val.val, 2);
	buf[2] = '\0';
	hr = atoi(buf) % 24;

	if (dt == HOUR12) {
		if (!XmToggleButtonGetState(pu->ep_dssw.start_am)) {
			if (hr != 12)
				hr += 12;
		} else if (hr == 12)
			hr = 0;
		strncpy(buf, &pu->ep_dssw.start_val.val[3], 2);
		buf[2] = '\0';
	} else {
		strncpy(buf, &pu->ep_dssw.start_val.val[2], 2);
		buf[2] = '\0';
	}
	min = atoi(buf) % 60;
	set_int_prop(p, CP_APPTBEGIN, (hr * minsec) + min);

	XtVaGetValues(pu->ep_dur_cb, DtNtextField, &text, NULL);
	if (text) {
		min = atoi(XmTextGetString(text));
		set_int_prop(p, CP_APPTDURATION, min);
	}

	if ((status = get_reminders_vals(&pu->ep_reminders, False)) != REMINDERS_OK) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 1079, "Calendar : Error - Editor Settings"));
	  	char *text;
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		switch (status) {

		case ADVANCE_BLANK : text = XtNewString(catgets(c->DT_catd, 1, 908, "You may not set a blank reminder value.\nYour properties settings have not been saved."));
			break;

		case ADVANCE_CONTAINS_BLANK : text = XtNewString(catgets(c->DT_catd, 1, 909, "You may not set a reminder value with an embedded blank.\nYour properties settings have not been saved."));
			break;

		case ADVANCE_NONNUMERIC : text = XtNewString(catgets(c->DT_catd, 1, 910, "Reminder values must be a number with an optional sign.\nYour properties settings have not been saved."));
			break;

		}
		dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			BUTTON_HELP, ADVANCE_ERROR_HELP,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		return(True);
	}

	val = &pu->ep_reminders.beep_val;
	set_char_prop(p, CP_BEEPON, boolean_str(val->selected));
	set_int_prop(p, CP_BEEPADV, val->scope_val);
	set_char_prop(p, CP_BEEPUNIT, (char *)time_scope_str(val->scope));

	val = &pu->ep_reminders.flash_val;
	set_char_prop(p, CP_FLASHON, boolean_str(val->selected));
	set_int_prop(p, CP_FLASHADV, val->scope_val);
	set_char_prop(p, CP_FLASHUNIT, (char *)time_scope_str(val->scope));

	val = &pu->ep_reminders.popup_val;
	set_char_prop(p, CP_OPENON, boolean_str(val->selected));
	set_int_prop(p, CP_OPENADV, val->scope_val);
	set_char_prop(p, CP_OPENUNIT, (char *)time_scope_str(val->scope));

	val = &pu->ep_reminders.mail_val;
	set_char_prop(p, CP_MAILON, boolean_str(val->selected));
	set_int_prop(p, CP_MAILADV, val->scope_val);
	set_char_prop(p, CP_MAILUNIT, (char *)time_scope_str(val->scope));
	set_char_prop(p, CP_MAILTO, pu->ep_reminders.mailto_val);

	XtVaGetValues(pu->privacy_menu, DtNselectedPosition, &i, NULL);
	set_char_prop(p, CP_PRIVACY, privacy_str(i));

	return(False);
}

extern Boolean
get_display_vals_from_ui(Props_pu *pu, Props *p) {
	int	val;
	char	*str;
	Calendar *c = pu->cal;

	set_char_prop(p, CP_CALLOC, str = XmTextGetString(pu->dp_loc_text));
	if (blank_buf(str) || embedded_blank(str)) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 1083, "Calendar : Error - Display Settings"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 772, "You may not save a blank User Calendar Location\nor one that contains blanks.\nYour properties settings have not been saved."));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			BUTTON_HELP, CAL_LOCATION_ERROR_HELP,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		XtFree(str);
		return(True);
	
	}
	XtFree(str);

	XtVaGetValues(pu->dp_beg_scale, XmNvalue, &val,
		NULL);
	set_int_prop(p, CP_DAYBEGIN, val);
	XtVaGetValues(pu->dp_end_scale, XmNvalue, &val,
		NULL);
	set_int_prop(p, CP_DAYEND, val);

	set_int_prop(p, CP_DEFAULTDISP, XmToggleButtonGadgetGetState(
		pu->dp_hour24_toggle) ? HOUR24 : HOUR12);

	if (XmToggleButtonGadgetGetState(pu->dp_view_day_toggle))
		val = (int)DAY_VIEW;
	else if (XmToggleButtonGadgetGetState(pu->dp_view_month_toggle))
		val = (int)MONTH_VIEW;
	else if (XmToggleButtonGadgetGetState(pu->dp_view_week_toggle))
		val = (int)WEEK_VIEW;
	else if (XmToggleButtonGadgetGetState(pu->dp_view_year_toggle))
		val = (int)YEAR_VIEW;
	set_int_prop(p, CP_DEFAULTVIEW, val);

#ifdef FNS
	val = XmToggleButtonGadgetGetState(pu->dp_use_fns_toggle);
	set_int_prop(p, CP_USEFNS, val);
#endif /* FNS */

	set_char_prop(p, CP_DEFAULTCAL,
		str = XmTextGetString(pu->dp_init_view_text));

	if (blank_buf(str) || embedded_blank(str)) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 1083, "Calendar : Error - Display Settings"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 773, "You may not save a blank Initial Calendar View\nor one that contains blanks.\nYour properties settings have not been saved."));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		XtFree(str);
		return(True);
	
	}
	XtFree(str);

	return(False);
}

extern Boolean
get_gap_vals_from_ui(Props_pu *pu, Props *p) {
	CSA_return_code	stat;
	Dtcm_calendar	*cal;

	cal = allocate_cal_struct(appt_write, 
				    	pu->cal->my_cal_version,
					CSA_CAL_ATTR_ACCESS_LIST_I, 
					NULL);
	cal->access_list->value->item.access_list_value = pu->gap_list;
	pu->gap_list = NULL;

	stat = csa_update_calendar_attributes(pu->cal->my_cal_handle, cal->count, cal->attrs, NULL);
	backend_err_msg(pu->frame, pu->cal->calname, stat, pu->xm_error_pixmap);
	free_cal_struct(&cal);
	set_gap_vals_on_ui(pu, p);
	return(False);
}

typedef enum   {stopped, 
		first_number, 
		leading_decimal, 
		found_decimal,
		second_number,
		completed_number} float_state;

static Boolean
bad_float(char *str) {

	register char	*s_ptr;
	float_state	state;

	/* check a string to see if it represents a reasonable 
	   floating point number.  Reasonable, in this case means that 
	   there are digits, potentially a decimal point, and only 
	   digits after the decimal point. */

	if (blank_buf(str) || embedded_blank(str)) 
		return(True);

	s_ptr = str;
	while (*s_ptr) {
		if (!isdigit(*s_ptr) && (*s_ptr != ' ') && (*s_ptr != '.'))
			return(True);
		s_ptr++;
	}

	s_ptr = str;
	state = stopped;

	while  (*s_ptr) {

		if (isdigit(*s_ptr)) {
			if (state == stopped)
				state =  first_number;
			else if ((state == found_decimal) ||
				 (state == leading_decimal))
				state = second_number;
			else if (state == completed_number)
				return(True);
			
		}
		else if (*s_ptr == '.') {
			if (state == stopped)
				state = leading_decimal;
			else if (state == first_number)
				state = found_decimal;
			else
				return(True);
		}
		else if ((*s_ptr == ' ') || (*s_ptr == '\t')) {
			if ((state == first_number) ||
			    (state == second_number))
				state = completed_number;
			else if ((state == leading_decimal) ||
				 (state == found_decimal))
				return(True);
		}

		s_ptr++;
	}

	return(False);
}

extern Boolean
get_printer_vals_from_ui(Props_pu *pu, Props *p) {
	char		*str;
	int		plevel = 0;
	int		copies, units;
	char		buf[BUFSIZ];
	Calendar 	*c = pu->cal;

	/* try to validate all of the floating point numbers in the 
	   printer options pane. */


	str = XmTextGetString(pu->pop_width_text);
	if (bad_float(str) == True) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 1085, "Calendar : Error - Printer Settings"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 821, "You have specified an illegal print width value.\nYour properties settings have not been saved."));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			BUTTON_HELP, PRINT_HELP_BUTTON,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		XtFree(str);
		return(True);
	}
	XtFree(str);

	str = XmTextGetString(pu->pop_height_text);
	if (bad_float(str) == True) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 1085, "Calendar : Error - Printer Settings"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 822, "You have specified an illegal print height value.\nYour properties settings have not been saved."));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			BUTTON_HELP, PRINT_HELP_BUTTON,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		XtFree(str);
		return(True);
	}
	XtFree(str);

	str = XmTextGetString(pu->pop_pos_left_text);
	set_char_prop(p, CP_PRINTPOSXOFF, str);
	if (bad_float(str) == True) {
		char *title = XtNewString(catgets(c->DT_catd, 1, 1085, "Calendar : Error - Printer Settings"));
		char *text = XtNewString(catgets(c->DT_catd, 1, 911, "You have specified an illegal position from left offset value.\nYour properties settings have not been saved."));
		char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			BUTTON_HELP, PRINT_HELP_BUTTON,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		XtFree(str);
		return(True);
	}
	XtFree(str);

	str = XmTextGetString(pu->pop_pos_bottom_text);
	if (bad_float(str) == True) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 1085, "Calendar : Error - Printer Settings"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 912, "You have specified an illegal position from right offset value.\nYour properties settings have not been saved."));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(pu->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			BUTTON_HELP, PRINT_HELP_BUTTON,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		XtFree(str);
		return(True);
	}
	XtFree(str);


	if (XmToggleButtonGadgetGetState(pu->pop_dest_print_toggle))
		set_int_prop(p, CP_PRINTDEST, 0);
	else if (XmToggleButtonGadgetGetState(pu->pop_dest_file_toggle))
		set_int_prop(p, CP_PRINTDEST, 1);

	str = XmTextGetString(pu->pop_printer_text);
	set_char_prop(p, CP_PRINTERNAME, str);
	XtFree(str);

	str = XmTextGetString(pu->pop_directory_text);
	set_char_prop(p, CP_PRINTDIRNAME, str);
	XtFree(str);

	str = XmTextGetString(pu->pop_options_text);
	set_char_prop(p, CP_PRINTOPTIONS, str);
	XtFree(str);

	str = XmTextGetString(pu->pop_file_text);
	set_char_prop(p, CP_PRINTFILENAME, str);
	XtFree(str);

	str = XmTextGetString(pu->pop_width_text);
	set_char_prop(p, CP_PRINTWIDTH, str);
	XtFree(str);

	str = XmTextGetString(pu->pop_height_text);
	set_char_prop(p, CP_PRINTHEIGHT, str);
	XtFree(str);

	str = XmTextGetString(pu->pop_pos_left_text);
	set_char_prop(p, CP_PRINTPOSXOFF, str);
	XtFree(str);

	str = XmTextGetString(pu->pop_pos_bottom_text);
	set_char_prop(p, CP_PRINTPOSYOFF, str);
	XtFree(str);

	XtVaGetValues(pu->pop_units_spin, DtNposition, &units, NULL);
	sprintf(buf,"%d", units);
	set_char_prop(p, CP_PRINTMONTHS, buf);

	XtVaGetValues(pu->pop_copies_spin, DtNposition, &copies, NULL);
	sprintf(buf,"%d", copies);
	set_char_prop(p, CP_PRINTCOPIES, buf);

	/*
	**  For backward compatability with the XView toggles (which used or'd
	**  values) - we must continue ...
	*/
	if (XmToggleButtonGadgetGetState(pu->pop_privacy_public_toggle))
		plevel |= PRINT_PUBLIC;
	if (XmToggleButtonGadgetGetState(pu->pop_privacy_semi_toggle))
		plevel |= PRINT_SEMIPRIVATE;
	if (XmToggleButtonGadgetGetState(pu->pop_privacy_private_toggle))
		plevel |= PRINT_PRIVATE;
	set_int_prop(p, CP_PRINTPRIVACY, plevel);

	return(False);
}

extern Boolean
get_date_vals_from_ui(Props_pu *pu, Props *p) {
	if (XmToggleButtonGadgetGetState(pu->dfp_order_mmddyy_toggle))
		set_int_prop(p, CP_DATEORDERING, (int)ORDER_MDY);
	else if (XmToggleButtonGadgetGetState(pu->dfp_order_ddmmyy_toggle))
		set_int_prop(p, CP_DATEORDERING, (int)ORDER_DMY);
	else if (XmToggleButtonGadgetGetState(pu->dfp_order_yymmdd_toggle))
		set_int_prop(p, CP_DATEORDERING, (int)ORDER_YMD);

	if (XmToggleButtonGadgetGetState(pu->dfp_sep_blank_toggle))
		set_int_prop(p, CP_DATESEPARATOR, (int)SEPARATOR_BLANK);
	else if (XmToggleButtonGadgetGetState(pu->dfp_sep_slash_toggle))
		set_int_prop(p, CP_DATESEPARATOR, (int)SEPARATOR_SLASH);
	else if (XmToggleButtonGadgetGetState(pu->dfp_sep_period_toggle))
		set_int_prop(p, CP_DATESEPARATOR, (int)SEPARATOR_DOT);
	else if (XmToggleButtonGadgetGetState(pu->dfp_sep_dash_toggle))
		set_int_prop(p, CP_DATESEPARATOR, (int)SEPARATOR_DASH);

	return(False);
}

extern Boolean
get_props_from_ui(Props_pu *pu, Props *p, int *redisplay_mask) {

	*redisplay_mask = 0;

	/* we only ever have to get the values for one of the categories, 
	   as changing the nanes causes the values for the existing 
	   category to be flushed. */

	switch(pu->last_props_pane) {
	case DISPLAY_PANE:
		if (get_display_vals_from_ui(pu, p) == True)
			return(True);
		else
			*redisplay_mask |= (PROPS_REDO_EDITORS | PROPS_REDO_DISPLAY);
		break;
	case GROUP_ACCESS_PANE:
		if (get_gap_vals_from_ui(pu, p) == True)
                		return(True);
		break;
	case PRINTER_OPS_PANE:
		if (get_printer_vals_from_ui(pu, p) == True)
			return(True);
		else
			*redisplay_mask |= PROPS_REDO_PRINT_DIALOG;
		break;
	case DATE_FORMAT_PANE:
		if (get_date_vals_from_ui(pu, p) == True)
                	return(True);
       		else
			*redisplay_mask |= (PROPS_REDO_EDITORS | PROPS_REDO_DISPLAY);
		break;
	case EDITOR_PANE:
		if (get_editor_vals_from_ui(pu, p) == True)
			return(True);
		else
			*redisplay_mask |= PROPS_REDO_EDITORS;
	default:
		break;
	}
	return(False);
}

extern void
set_editor_vals_on_ui(Props_pu *pu, Props *p) {
	int		i;
	char		buf[MAXNAMELEN];
	Widget		text;
	Reminders_val	*val;

	set_dssw_defaults(&pu->ep_dssw, now(), True);
	XtVaGetValues(pu->ep_dur_cb, DtNtextField, &text, NULL);
	if (text) {
		sprintf(buf, "%d", get_int_prop(p, CP_APPTDURATION));
		XmTextSetString(text, buf);
	}

	val = &pu->ep_reminders.beep_val;
	val->selected =	convert_boolean_str(get_char_prop(p, CP_BEEPON));
	val->scope = convert_time_scope_str(get_char_prop(p, CP_BEEPUNIT));
	val->scope_val = get_int_prop(p, CP_BEEPADV);

	val = &pu->ep_reminders.flash_val;
	val->selected =	convert_boolean_str(get_char_prop(p, CP_FLASHON));
	val->scope = convert_time_scope_str(get_char_prop(p, CP_FLASHUNIT));
	val->scope_val = get_int_prop(p, CP_FLASHADV);

	val = &pu->ep_reminders.popup_val;
	val->selected =	convert_boolean_str(get_char_prop(p, CP_OPENON));
	val->scope = convert_time_scope_str(get_char_prop(p, CP_OPENUNIT));
	val->scope_val = get_int_prop(p, CP_OPENADV);

	val = &pu->ep_reminders.mail_val;
	val->selected =	convert_boolean_str(get_char_prop(p, CP_MAILON));
	val->scope = convert_time_scope_str(get_char_prop(p, CP_MAILUNIT));
	val->scope_val = get_int_prop(p, CP_MAILADV);
	strcpy(pu->ep_reminders.mailto_val, get_char_prop(p, CP_MAILTO));

	set_reminders_vals(&pu->ep_reminders, False);

	i = convert_privacy_str_to_op(get_char_prop(p, CP_PRIVACY));
	XtVaSetValues(pu->privacy_menu, DtNselectedPosition, i, NULL);
}

extern void
set_display_vals_on_ui(Props_pu *pu, Props *p) {
	XmTextSetString(pu->dp_loc_text, get_char_prop(p, CP_CALLOC));

	XtVaSetValues(pu->dp_beg_scale, XmNvalue, get_int_prop(p, CP_DAYBEGIN),
		NULL);
	set_scale_value(pu->dp_beg_value, get_int_prop(p, CP_DAYBEGIN), pu);

	XtVaSetValues(pu->dp_end_scale, XmNvalue, get_int_prop(p, CP_DAYEND),
		NULL);
	set_scale_value(pu->dp_end_value, get_int_prop(p, CP_DAYEND), pu);

	XmToggleButtonGadgetSetState((get_int_prop(p, CP_DEFAULTDISP) == HOUR12)
		? pu->dp_hour12_toggle : pu->dp_hour24_toggle, True, True);

	switch(get_int_prop(p, CP_DEFAULTVIEW)) {
	case DAY_VIEW:
		XmToggleButtonGadgetSetState(pu->dp_view_day_toggle,
			True, True);
		break;
	case WEEK_VIEW:
		XmToggleButtonGadgetSetState(pu->dp_view_week_toggle,
			True, True);
		break;
	case YEAR_VIEW:
		XmToggleButtonGadgetSetState(pu->dp_view_year_toggle,
			True, True);
		break;
	case MONTH_VIEW:
	default:
		XmToggleButtonGadgetSetState(pu->dp_view_month_toggle,
			True, True);
		break;
	}
#ifdef FNS
	if (get_int_prop(p, CP_USEFNS))
		XmToggleButtonGadgetSetState(pu->dp_use_fns_toggle, True, True);
#endif /* FNS */

	XmTextSetString(pu->dp_init_view_text, get_char_prop(p, CP_DEFAULTCAL));
}

extern void
set_gap_vals_on_ui(Props_pu *pu, Props *p) {
	char		*p_str;
	XmString	xmstr;
	CSA_return_code	stat;
	Dtcm_calendar	*cal;
	CSA_access_list	step = NULL, holder = NULL, last;

	cal = allocate_cal_struct(appt_read, 
				    	pu->cal->my_cal_version,
					CSA_CAL_ATTR_ACCESS_LIST_I, 
					NULL);
	stat = query_cal_struct(pu->cal->my_cal_handle, cal);
	backend_err_msg(pu->frame, pu->cal->calname, stat, pu->xm_error_pixmap);
	if (stat != CSA_SUCCESS) {
		free_cal_struct(&cal);
		return;
	}
	pu->gap_list = cal->access_list->value->item.access_list_value;
	cal->access_list->value->item.access_list_value = NULL;

	XmListDeleteAllItems(pu->gap_access_list);
	step = pu->gap_list;
	while (step) {
		if (pu->cal->my_cal_version < DATAVER4) {
			p_str = p_gap_create_perm_str(step->rights);
			p_gap_create_entry_string(pu, step->user->user_name, 
							p_str, &xmstr);
			free(p_str);
		} else
			xmstr = XmStringCreateLocalized(step->user->user_name);

		XmListAddItem(pu->gap_access_list, xmstr, 0);
		XmStringFree(xmstr);
		step = step->next;
	}
	free_cal_struct(&cal);
}

extern void
set_printer_vals_on_ui(Props_pu *pu, Props *p) {
	int	plevel;

	XmToggleButtonGadgetSetState((get_int_prop(p, CP_PRINTDEST) == 0)
		? pu->pop_dest_print_toggle : pu->pop_dest_file_toggle,
		True, True);

	XmTextSetString(pu->pop_printer_text,
		get_char_prop(p, CP_PRINTERNAME));
	XmTextSetString(pu->pop_directory_text,
		get_char_prop(p, CP_PRINTDIRNAME));

	XmTextSetString(pu->pop_options_text,
		get_char_prop(p, CP_PRINTOPTIONS));
	XmTextSetString(pu->pop_file_text,
		get_char_prop(p, CP_PRINTFILENAME));

	XmTextSetString(pu->pop_width_text,
		get_char_prop(p, CP_PRINTWIDTH));
	XmTextSetString(pu->pop_height_text,
		get_char_prop(p, CP_PRINTHEIGHT));

	XmTextSetString(pu->pop_pos_left_text,
		get_char_prop(p, CP_PRINTPOSXOFF));
	XmTextSetString(pu->pop_pos_bottom_text,
		get_char_prop(p, CP_PRINTPOSYOFF));

	plevel = atoi(get_char_prop(p, CP_PRINTMONTHS));
	if ((plevel < 1) || (plevel > 100))
		plevel = 1;

	XtVaSetValues(pu->pop_units_spin, 
			DtNposition, plevel,
			NULL);

	plevel = atoi(get_char_prop(p, CP_PRINTCOPIES));
	if ((plevel < 1) || (plevel > 100))
		plevel = 1;

	XtVaSetValues(pu->pop_copies_spin, 
			DtNposition, plevel,
			NULL);

	/*
	**  For backward compatability with the XView toggles (which used or'd
	**  values) - we must continue ...
	*/
	plevel = get_int_prop(p, CP_PRINTPRIVACY);
	if (plevel & 1)
		XmToggleButtonGadgetSetState(pu->pop_privacy_public_toggle,
			True, True);
	if (plevel & 2)
		XmToggleButtonGadgetSetState(pu->pop_privacy_semi_toggle,
			True, True);
	if (plevel & 4)
		XmToggleButtonGadgetSetState(pu->pop_privacy_private_toggle,
			True, True);
}

extern void
set_date_vals_on_ui(Props_pu *pu, Props *p) {
	SeparatorType	st = get_int_prop(p, CP_DATESEPARATOR);
	OrderingType	ot = get_int_prop(p, CP_DATEORDERING);

	switch(ot) {
	case ORDER_MDY:
		XmToggleButtonGadgetSetState(pu->dfp_order_mmddyy_toggle,
			True, True);
		break;
	case ORDER_DMY:
		XmToggleButtonGadgetSetState(pu->dfp_order_ddmmyy_toggle,
			True, True);
		break;
	case ORDER_YMD:
	default:
		XmToggleButtonGadgetSetState(pu->dfp_order_yymmdd_toggle,
			True, True);
		break;
	}

	switch(st) {
	case SEPARATOR_BLANK:
		XmToggleButtonGadgetSetState(pu->dfp_sep_blank_toggle,
			True, True);
		break;
	case SEPARATOR_SLASH:
		XmToggleButtonGadgetSetState(pu->dfp_sep_slash_toggle,
			True, True);
		break;
	case SEPARATOR_DOT:
		XmToggleButtonGadgetSetState(pu->dfp_sep_period_toggle,
			True, True);
		break;
	case SEPARATOR_DASH:
		XmToggleButtonGadgetSetState(pu->dfp_sep_dash_toggle,
			True, True);
		break;
	}
}

extern void
set_props_on_ui(Props_pu *pu, Props *p) {
	set_editor_vals_on_ui(pu, p);
	set_display_vals_on_ui(pu, p);
	set_gap_vals_on_ui(pu, p);
	set_printer_vals_on_ui(pu, p);
	set_date_vals_on_ui(pu, p);
	p_clear_change(pu);
}

extern void
show_props_pu(Calendar *c) {
	Props       	*p = (Props *)c->properties;
	Props_pu       	*pu = (Props_pu *)c->properties_pu;

	if (!p || !pu)
		return;
	if (!pu->frame)
		p_make_props_pu(c);
	set_props_on_ui(pu, p);
	XtPopup(pu->frame, XtGrabNone);
	if (!props_pu_showing(pu))
		ds_position_popup(c->frame, pu->frame, DS_POPUP_LOR);
}


extern Boolean
props_pu_showing(Props_pu *p) {
	if (p && p->base_form_mgr)
		return XtIsManaged(p->base_form_mgr);
	return False;
}

static void
p_gap_clear_toggles(Props_pu *p)
{

	/*
 	 * V5DEP: If this is a V5 server w/V1 backing store probably
	 * should use old access flags.
 	 */
	if (p->cal->my_cal_version >= DATAVER4) {
		XmToggleButtonGadgetSetState(p->gap2_public_tog[GAP_VIEW],
			False, True);
		XmToggleButtonGadgetSetState(p->gap2_public_tog[GAP_INSERT],
			False, True);
		XmToggleButtonGadgetSetState(p->gap2_public_tog[GAP_CHANGE],
			False, True);
		XmToggleButtonGadgetSetState(p->gap2_semi_tog[GAP_VIEW],
			False, True);
		XmToggleButtonGadgetSetState(p->gap2_semi_tog[GAP_INSERT],
			False, True);
		XmToggleButtonGadgetSetState(p->gap2_semi_tog[GAP_CHANGE],
			False, True);
		XmToggleButtonGadgetSetState(p->gap2_private_tog[GAP_VIEW],
			False, True);
		XmToggleButtonGadgetSetState(p->gap2_private_tog[GAP_INSERT],
			False, True);
		XmToggleButtonGadgetSetState(p->gap2_private_tog[GAP_CHANGE],
			False, True);
	} else {
		XmToggleButtonGadgetSetState(p->gap_browse_toggle,
			False, True);
		XmToggleButtonGadgetSetState(p->gap_insert_toggle,
			False, True);
		XmToggleButtonGadgetSetState(p->gap_delete_toggle,
			False, True);
	}
}

static void
p_gap_center_cat_menu(
	Widget		frame,
	Widget		menu)
{
	Dimension	frame_width,
			menu_width;

	XtVaGetValues(frame,
		XmNwidth,       &frame_width,
		NULL);

	XtVaGetValues(menu,
		XmNwidth,       &menu_width,
		NULL);

	XtVaSetValues(menu,
		XmNleftOffset,	frame_width/2 - menu_width/2,
		NULL);
}
