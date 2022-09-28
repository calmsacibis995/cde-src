/* $XConsortium: print.c /main/cde1_maint/5 1995/11/14 16:58:56 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)print.c	1.34 96/11/12 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <csa.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/PanedW.h>
#include <Xm/DialogS.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/SashP.h>
#include <Dt/ComboBox.h>
#include <Dt/HourGlass.h>
#include <Dt/SpinBox.h>
#include "util.h"
#include "props.h"
#include "props_pu.h"
#include "getdate.h"
#include "datefield.h"
#include "timeops.h"
#include "calendar.h"
#include "print.h"
#include "todo.h"
#include "help.h"
#include "ps_graphics.h"

/* needed for view-specific print routines */
#include "dayglance.h"
#include "weekglance.h"
#include "monthglance.h"
#include "yearglance.h"

/*
 * private function declarations
 */
static void print_cb(Widget, XtPointer, XtPointer);
static void cancel_cb(Widget, XtPointer, XtPointer);
static void file_toggle_cb(Widget, XtPointer, XtPointer);
static void popup_callback(Widget, XtPointer, XtPointer);
static void report_option_cb(Widget, XtPointer, XtPointer);
static void print_report(Calendar *);
static void sensitize_controls(_DtCmPrintData *);
static void scale_buttons(_DtCmPrintData *);

/*
 * this adjusts the action buttons to be same size
 * ...just as simple to do it this way as have Form
 * do it... called just after button creation time.
 */
static void
scale_buttons(_DtCmPrintData *pd)
{
	Dimension	max_w = 0, help_w, cancel_w, print_w;

	/* calc max width of the action buttons */
	XtVaGetValues(pd->print, XmNwidth, &print_w, NULL);
	if (print_w > max_w)
		max_w = print_w;

	XtVaGetValues(pd->cancel, XmNwidth, &cancel_w, NULL);
	if (cancel_w > max_w)
		max_w = cancel_w;

	XtVaGetValues(pd->help, XmNwidth, &help_w, NULL);
	if (help_w > max_w)
		max_w = help_w;


	/* unconditionally set all action buttons to max width */
	if (max_w > print_w)
		XtVaSetValues(pd->print, XmNwidth, max_w, NULL);

	if (max_w > cancel_w)
		XtVaSetValues(pd->cancel, XmNwidth, max_w, NULL);

	if (max_w > help_w)
		XtVaSetValues(pd->help, XmNwidth, max_w, NULL);

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
 
        _DtCmPrintData  *pd = (_DtCmPrintData *) data;
        int             value = atoi(XmTextGetString(w));
        Display         *dpy = XtDisplayOfObject(w);
 
	if (all_digits(XmTextGetString(w)) == False) {
                value = 1;
                XBell(dpy, 50);
        }
        else if ((value < 1 ) || (value > 99)) {
                value = 1;
                XBell(dpy, 50);
        }
 
        XtVaSetValues(pd->copies_spin, DtNposition, value, NULL);
}


/*
 * post_print_dialog()
 *
 * Create and display the Print dialog.
 * This is created and destroyed
 * on-demand because it is likely to be
 * infrequently used, if ever.
 */
void
post_print_dialog(
	Calendar *c)
{
	XmString 	 xmstr, day_view, week_view, month_view,
			 year_view, appt_list, todo_list, view, days,
			 weeks, months, years;
	Dimension 	 width;
	_DtCmPrintData 	*pd;
	Props 		*p = (Props *) c->properties;
	OrderingType 	 ot = get_int_prop(p, CP_DATEORDERING);
	SeparatorType 	 st = get_int_prop(p, CP_DATESEPARATOR);
	char 		 fnamebuf[BUFSIZ];
	Widget 		 separator1, 
			 text_field,
			 button_form;
	int 		 def_copies;
	char		 *printer_name;
	char		 *print_options;
	char		 *print_file;
	char		 *print_dir;
	char		 *title;

	c->print_data = (caddr_t)XtMalloc(sizeof(_DtCmPrintData));
	pd = (_DtCmPrintData *) c->print_data;

	title = XtNewString(catgets(c->DT_catd, 1, 728, "Calendar : Print"));
	pd->pdb = XtVaCreatePopupShell("Calendar - Print",
		xmDialogShellWidgetClass,
		c->frame,
		XmNtitle, 		title, 
		XmNdeleteResponse, 	XmDESTROY,
		NULL);
	XtFree(title);
	XtAddCallback(pd->pdb, XmNpopupCallback, popup_callback, pd);

	/* Build "Control Area" - the top part */
	pd->control = XtVaCreateWidget("control",
		xmFormWidgetClass, 
		pd->pdb, 
		XmNautoUnmanage, 	False,
		XmNfractionBase, 	3,
		XmNnoResize, 		True,
		NULL);

	pd->printer_text = XmCreateText(pd->control, "printerText", NULL, 0);
	XtVaSetValues(pd->printer_text,
		XmNtopAttachment, 	XmATTACH_FORM,
		XmNtopOffset, 		10,
		XmNleftAttachment, 	XmATTACH_POSITION,
		XmNleftPosition, 	1,
		XmNleftOffset, 		3,
		XmNcolumns, 		30,
		NULL);

	/* Command line overrides property sheet. */
	if (c->app_data->default_printer)
		printer_name = c->app_data->default_printer;
	else
		printer_name = get_char_prop((Props *)c->properties, 
								CP_PRINTERNAME);
	if (printer_name)
		XmTextSetString(pd->printer_text, printer_name);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 729, "Printer Name:"));
	pd->printer_label = XtVaCreateWidget("printerLabel",
		xmLabelGadgetClass, 
		pd->control,
		XmNlabelString, 	xmstr,
		XmNrightAttachment, 	XmATTACH_POSITION,
		XmNrightPosition, 	1,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	pd->printer_text,
		XmNbottomOffset, 	5,
		NULL);
	XmStringFree(xmstr);

	/*
  	 * LATER: set initial position DtNposition to same as default
 	 * number of coies from property sheet.
 	 */

	def_copies = atoi(get_char_prop(p, CP_PRINTCOPIES));
	if ((def_copies < 1) || (def_copies > 99))
		def_copies = 1;

	pd->copies_spin = DtCreateSpinBox(pd->control, "copiesSpin", NULL, 0);
	XtVaSetValues(pd->copies_spin,
		DtNminimumValue, 	1,
		DtNposition, 		def_copies,
		DtNmaximumValue, 	99,
		DtNspinBoxChildType, 	DtNUMERIC,
		DtNmaxLength, 		2,
		DtNeditable, 		True,
		DtNwrap, 		True,
		DtNcolumns, 		2,
		XmNshadowThickness, 	0,
		XmNleftAttachment, 	XmATTACH_POSITION,
		XmNleftPosition, 	1,
		XmNleftOffset,		1,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		pd->printer_text,
		NULL);

	XtVaGetValues(pd->copies_spin, DtNtextField, &text_field, NULL);
	XtAddCallback(text_field, XmNvalueChangedCallback, 
					p_copies_spin_callback, pd);


	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 730, "Copies:"));
	pd->copies_label = XtVaCreateWidget("copiesLabel",
		xmLabelGadgetClass, 
		pd->control,
		XmNlabelString, 	xmstr,
		XmNrightAttachment, 	XmATTACH_POSITION,
		XmNrightPosition, 	1,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	pd->copies_spin,
		XmNbottomOffset, 	5,
		NULL);
	XmStringFree(xmstr);

	pd->sep = XtVaCreateWidget("separator",
		xmSeparatorGadgetClass, 
		pd->control,
		XmNorientation, 	XmHORIZONTAL,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		pd->copies_spin,
		XmNtopOffset, 		10,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNrightAttachment, 	XmATTACH_FORM,
	NULL);

	view = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 976, "Report Type:"));
	day_view = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 977, "Day View"));
	week_view = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 978, "Week View"));
	month_view = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 979, "Month View"));
	year_view = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 980, "Year View"));
	appt_list = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 981, "Appointment List"));
	todo_list = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 982, "To Do List"));

	/*
	 * pick a report type as default based on current main view
	 */
	switch (c->view->glance) {
		case dayGlance : pd->report_type = PR_DAY_VIEW; break;
		case weekGlance : pd->report_type = PR_WEEK_VIEW; break;
		case monthGlance : pd->report_type = PR_MONTH_VIEW; break;
		case yearGlance : pd->report_type = PR_YEAR_VIEW; break;
	}

	/*
	 * remember - this returns a RowColumn widget!
	 */
	pd->report_type_option = XmVaCreateSimpleOptionMenu(pd->control,
		"reportType", view, NULL, pd->report_type, report_option_cb,
		XmVaPUSHBUTTON, 	day_view, NULL, NULL, NULL,
		XmVaPUSHBUTTON, 	week_view, NULL, NULL, NULL,
		XmVaPUSHBUTTON, 	month_view, NULL, NULL, NULL,
		XmVaPUSHBUTTON, 	year_view, NULL, NULL, NULL,
		XmVaPUSHBUTTON, 	appt_list, NULL, NULL, NULL,
		XmVaPUSHBUTTON, 	todo_list, NULL, NULL, NULL,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		pd->sep,
		XmNtopOffset, 		10,
		XmNleftAttachment, 	XmATTACH_POSITION,
		XmNleftPosition, 	1,
		XmNmarginWidth,		0,
		XmNnavigationType, 	XmTAB_GROUP,
		NULL);

	pd->report_type_label = XtVaCreateWidget("reportTypeLabel",
		xmLabelGadgetClass, 
		pd->control,
		XmNlabelString, 	view,
		XmNrightAttachment, 	XmATTACH_POSITION,
		XmNrightPosition, 	1,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	pd->report_type_option,
		XmNbottomOffset, 	5,
		NULL);
   
	XmStringFree(day_view);
	XmStringFree(week_view);
	XmStringFree(month_view);
	XmStringFree(year_view);
	XmStringFree(appt_list);
	XmStringFree(todo_list);
	XmStringFree(view);

	pd->from_text = XmCreateText(pd->control, "fromText", NULL, 0);
	XtVaSetValues(pd->from_text,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		pd->report_type_option,
		XmNleftAttachment, 	XmATTACH_POSITION,
		XmNleftPosition, 	1,
		XmNleftOffset, 		3,
		XmNcolumns, 		30,
		NULL);
	XmTextSetString(pd->from_text, "pd->from_text");
	set_date_in_widget(c->view->date, pd->from_text, ot, st);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 731, "From:"));
	pd->from_label = XtVaCreateWidget("fromLabel",
		xmLabelGadgetClass, 
		pd->control,
		XmNlabelString, 	xmstr,
		XmNrightAttachment, 	XmATTACH_POSITION,
		XmNrightPosition, 	1,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	pd->from_text,
		XmNbottomOffset, 	5,
		NULL);
	XmStringFree(xmstr);

	pd->to_text = XmCreateText(pd->control, "toText", NULL, 0);
	XtVaSetValues(pd->to_text,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		pd->from_text,
		XmNleftAttachment, 	XmATTACH_POSITION,
		XmNleftPosition, 	1,
		XmNleftOffset, 		3,
		XmNcolumns, 		30,
		NULL);
	XmTextSetString(pd->to_text, "pd->to_text");

	set_date_in_widget(c->view->date, pd->to_text, ot, st);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 732, "To:"));
	pd->to_label = XtVaCreateWidget("toLabel",
		xmLabelGadgetClass, 
		pd->control,
		XmNlabelString, 	xmstr,
		XmNrightAttachment, 	XmATTACH_POSITION,
		XmNrightPosition, 	1,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	pd->to_text,
		XmNbottomOffset, 	5,
		NULL);
	XmStringFree(xmstr);

	pd->args_text = XtVaCreateWidget("argsText",
		xmTextWidgetClass, 
		pd->control,
		XmNcolumns, 		30,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		pd->to_text,
		XmNtopOffset, 		5,
		XmNleftAttachment, 	XmATTACH_POSITION,
		XmNleftPosition, 	1,
		XmNleftOffset, 		3,
		NULL);

	print_options = get_char_prop((Props *)c->properties,CP_PRINTOPTIONS);
	if (print_options)
		XmTextSetString(pd->args_text, print_options);

	xmstr = XmStringCreateLocalized(
			catgets(c->DT_catd, 1, 733, " Print Command Options:"));
	pd->args_label = XtVaCreateWidget("argsLabel",
		xmLabelGadgetClass, 
		pd->control,
		XmNlabelString, 	xmstr,
		XmNrightAttachment, 	XmATTACH_POSITION,
		XmNrightPosition, 	1,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	pd->args_text,
		XmNbottomOffset, 	5,
		NULL);
	XmStringFree(xmstr);

	pd->file_text = XtVaCreateWidget("fileText",
		xmTextWidgetClass, 
		pd->control,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		pd->args_text,
		XmNleftAttachment, 	XmATTACH_POSITION,
		XmNleftPosition, 	1,
		XmNleftOffset, 		3,
		XmNcolumns, 		30,
		NULL);

	xmstr = XmStringCreateLocalized(
				catgets(c->DT_catd, 1, 734, "Print to file:"));
	pd->file_toggle = XtVaCreateWidget("fileToggle",
		xmToggleButtonGadgetClass, 
		pd->control,
		XmNindicatorType, 	XmONE_OF_MANY,
		XmNlabelString, 	xmstr,
		XmNrightAttachment, 	XmATTACH_POSITION,
		XmNrightPosition, 	1,
		XmNrightOffset, 	-2,
		XmNbottomAttachment, 	XmATTACH_OPPOSITE_WIDGET,
		XmNbottomWidget, 	pd->file_text,
		XmNnavigationType, 	XmTAB_GROUP,
		NULL);
	XmStringFree(xmstr);
	XtAddCallback(pd->file_toggle, 
				XmNvalueChangedCallback, file_toggle_cb, pd);
	if (get_int_prop(p, CP_PRINTDEST) == 0)
		XmToggleButtonSetState(pd->file_toggle, False, False);
	else
		XmToggleButtonSetState(pd->file_toggle, True, False);

	/* Initialize filename widget to dir+filename from props */

	print_file = get_char_prop((Props *)c->properties,CP_PRINTFILENAME);
	if (!print_file)
		print_file = "";
	print_dir = get_char_prop((Props *)c->properties,CP_PRINTDIRNAME);
	if (!print_dir)
		print_dir = "";

	sprintf(fnamebuf, "%s/%s", print_dir, print_file);

	XmTextSetString(pd->file_text, fnamebuf);
	XmTextSetInsertionPosition(pd->file_text, 
				XmTextGetLastPosition(pd->file_text));

	/* desensitize this and the arg controls if print to file is selected */
	sensitize_controls(pd);

	/* Build "Action Area" - the bottom part */

	separator1 = XtVaCreateWidget("separator1",
		xmSeparatorGadgetClass, 
		pd->control,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNrightAttachment, 	XmATTACH_FORM,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		pd->file_text,
		XmNtopOffset, 		10,
		NULL);

	button_form = XtVaCreateWidget("print_button_form_mgr",
		xmFormWidgetClass,
		pd->control,
		XmNautoUnmanage, 	False,
		XmNfractionBase,        8,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		separator1,
		XmNtopOffset, 		5,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		10,
		XmNrightAttachment, 	XmATTACH_FORM,
		XmNrightOffset, 	10,
		XmNbottomAttachment, 	XmATTACH_FORM,
		XmNbottomOffset, 	5,
		NULL);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 735, "Print"));
	pd->print = XtVaCreateWidget("print",
		xmPushButtonGadgetClass, 
		button_form,
		XmNlabelString, 	xmstr,
                XmNleftAttachment,      XmATTACH_POSITION,
                XmNleftPosition,        1,
                XmNleftOffset,          10,
                XmNrightAttachment,     XmATTACH_POSITION,
                XmNrightPosition,       3,
                XmNrightOffset,         10,
                XmNbottomAttachment,    XmATTACH_FORM,
                XmNtopAttachment,       XmATTACH_FORM,
		NULL);
	XmStringFree(xmstr);
	XtAddCallback(pd->print, XmNactivateCallback, print_cb, (XtPointer)c);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 680, "Close"));
	pd->cancel = XtVaCreateWidget("cancel",
		xmPushButtonGadgetClass, 
		button_form,
		XmNlabelString, xmstr,
                XmNleftAttachment,      XmATTACH_POSITION,
                XmNleftPosition,        3,
                XmNrightAttachment,     XmATTACH_POSITION,
                XmNrightPosition,       5,
                XmNrightOffset,         10,
                XmNbottomAttachment,    XmATTACH_FORM,
                XmNtopAttachment,       XmATTACH_FORM,
		NULL);
	XmStringFree(xmstr);
	XtAddCallback(pd->cancel, XmNactivateCallback, cancel_cb, c);
	XtAddCallback(pd->cancel, XmNdestroyCallback, cancel_cb, c);

	xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 77, "Help"));
	pd->help = XtVaCreateWidget("help",
		xmPushButtonGadgetClass, 
		button_form,
		XmNlabelString, 	xmstr,
                XmNleftAttachment,      XmATTACH_POSITION,
                XmNleftPosition,        5,
                XmNrightAttachment,     XmATTACH_POSITION,
                XmNrightPosition,       7,
                XmNrightOffset,         5,
                XmNbottomAttachment,    XmATTACH_FORM,
                XmNtopAttachment,       XmATTACH_FORM,
		NULL);
	XmStringFree(xmstr);

	scale_buttons(pd);

	XtVaSetValues(pd->control, 
		XmNdefaultButton, 	pd->print, 
		XmNcancelButton, 	pd->cancel,
		NULL);

	XtAddCallback(pd->help, XmNactivateCallback, 
				(XtCallbackProc)help_cb, PRINT_HELP_BUTTON);


	/* turn off traversal on the unused sashes in the paned window. */
        XtVaSetValues(button_form, 
                XmNdefaultButton,               pd->print,
                NULL);
        XtVaSetValues(pd->control, 
                XmNdefaultButton,               pd->print,
                NULL);

	/* manage everything */
	ManageChildren(button_form);
	ManageChildren(pd->control);
	XtManageChild(pd->control);

	/* The report type label is used only to get the width of the label.
 	 * We assume that this is the same width the Option Menu used for
	 * it's label since they used the same string.  When then offset
	 * the Option Menu so that the ``:'' in the label align with the
	 * other ``:''.   Yes, this is a hack, but I don't know how to
	 * access the children of the Option Menu.
	 */
        XtVaGetValues(pd->report_type_label,
		XmNwidth,       &width,
		NULL);
        XtVaSetValues(pd->report_type_option,
		XmNleftOffset,       -width,
		NULL);
	XtDestroyWidget(pd->report_type_label);

	XtRealizeWidget(pd->pdb);

	/* display the dialog on-screen */
	XtPopup(pd->pdb, XtGrabNone);
}

/*
 * Close dialog: pop down and destroy the dialog, and free
 * associated memory.
 */
static void
cancel_cb(Widget w, XtPointer data, XtPointer ignore)
{
   Calendar *c = (Calendar *)data;
   _DtCmPrintData *pd = (_DtCmPrintData *)c->print_data;
   Widget dialog;

   if (pd == NULL)
	return;
   
   dialog = pd->pdb;

   XtPopdown(dialog);
   XtDestroyWidget(dialog);

   /* free and clear the print data structure */
   XtFree((char *)c->print_data);

   /*
    * This is important: the print routines, when gatherng parameters,
    * test c->print_data, and if it's non-NULL, treat it as a valid
    * print_data structure.  It must only be non-null while the dialog
    * exists, to avoid bad references. 
    */
   c->print_data = NULL;
}

/*
 * Print callback: do the deed!
 */
static void
print_cb(Widget w, XtPointer data, XtPointer ignore)
{
   Calendar *c = (Calendar *)data;

   print_report(c);
}

/*
 * file_toggle_cb: control text field sensitivity
 */
static void
file_toggle_cb(Widget w, XtPointer data, XtPointer ignore)
{
   _DtCmPrintData *pd = (_DtCmPrintData *) data;

   sensitize_controls(pd);
}

/*
 * sensitize_controls: Manage the sensitivity of various controls
 * based on whether printing to file or printer.
 */
static void
sensitize_controls(_DtCmPrintData *pd)
{
   Boolean file_only = False;

   file_only = XmToggleButtonGadgetGetState(pd->file_toggle);
   XtSetSensitive(pd->printer_label, file_only ? False : True);
   XtSetSensitive(pd->printer_text, file_only ? False : True);
   XtSetSensitive(pd->copies_label, file_only ? False : True);
   XtSetSensitive(pd->copies_spin, file_only ? False : True);
   XtSetSensitive(pd->file_text, file_only);
   XtSetSensitive(pd->args_label, file_only ? False : True);
   XtSetSensitive(pd->args_text, file_only ? False : True);
}

static void
popup_callback(Widget w, XtPointer client, XtPointer call)
{
   _DtCmPrintData *pd = (_DtCmPrintData *) client;

   Position x, y;

   XtVaGetValues(XtParent(pd->pdb), XmNx, &x, XmNy, &y, NULL);
   XtVaSetValues(pd->pdb, XmNx, x+100, XmNy, y+100, NULL);
}


/*
 * print_report: dispatch the view-specific output routine
 */
static void
print_report(Calendar *c)
{
_DtCmPrintData 	*pd = (_DtCmPrintData *)c->print_data;
	Props 		*p = (Props *) c->properties;
	Props_pu	*pu = (Props_pu *) c->properties_pu;
	int 		r = pd->report_type;
	OrderingType 	ot = get_int_prop(p, CP_DATEORDERING);
	SeparatorType 	st = get_int_prop(p, CP_DATESEPARATOR);
	char 		*start_buf = get_date_from_widget(c->view->date, pd->from_text, ot, st);
	Tick		start_date = (Tick) cm_getdate(start_buf, NULL);
	char 		*end_buf = get_date_from_widget(c->view->date, pd->to_text, ot, st);
	Tick		end_date = (Tick) cm_getdate(end_buf, NULL);



	if ((start_date == DATE_BBOT) || (end_date == DATE_BBOT) || 
	    (start_date == DATE_AEOT) || (end_date == DATE_BBOT)) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 736, 
						"Calendar : Error - Print"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 892,
				"The dates for printing must be between 1969 and 2038."));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(c->frame,
			DIALOG_TITLE, title, 
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		return;
	}
	else if (start_date <= 0) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 736, 
						"Calendar : Error - Print"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 894,
				"Malformed \"From\" date"));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(c->frame,
			DIALOG_TITLE, title, 
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		return;
	}
	else if (end_date <= 0) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 736, 
						"Calendar : Error - Print"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 896,
				"Malformed \"To\" date"));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(c->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		return;
	}
	else if (start_date > end_date) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 736, 
						"Calendar : Error - Print"));
	  	char *text = XtNewString(catgets(c->DT_catd, 1, 898,
				"The \"To\" date for printing must be after the \"From\" date for printing"));
	  	char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(c->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, text,
			BUTTON_IDENT, 1, ident1,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident1);
		XtFree(text);
		XtFree(title);
		return;
	}


   	_DtTurnOnHourGlass(c->frame);

   	switch(r) {
      	case PR_YEAR_VIEW:
         	print_std_year_range(year(start_date), year(end_date));
         	break;
      	case PR_MONTH_VIEW:
         	print_month_range(c, start_date, end_date);
         	break;
      	case PR_WEEK_VIEW:
         	print_week_range(c, start_date, end_date);
         	break;
      	case PR_DAY_VIEW:
         	print_day_range(c, start_date, end_date);
         	break;
      	case PR_APPT_LIST:
         	ps_print_list_range(c, CSA_TYPE_EVENT, (int) c->view->glance, start_date, end_date);
         	break;
      	case PR_TODO_LIST:
         	ps_print_list_range(c, CSA_TYPE_TODO, (int) VIEW_PENDING, start_date, end_date);
         	break;
      	default:
         	print_month_pages(c);
   	}
	
   	_DtTurnOffHourGlass(c->frame);
}

/*
 * report_option_cb
 *
 * callback for report-type option menu.
 * Here we just store the button number of the selected
 * button in the print_data structure, to make life easy
 * when the print callback is invoked.
 */
static void
report_option_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
   int choice = (int) client_data;
   Calendar *c = calendar;
   _DtCmPrintData *pd = (_DtCmPrintData *)c->print_data;

   pd->report_type = choice;
}
