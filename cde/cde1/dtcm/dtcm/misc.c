/* $XConsortium: misc.c /main/cde1_maint/5 1995/11/14 16:58:43 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)misc.c	1.79 96/11/12 Sun Microsystems, Inc."

#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <Xm/PushBG.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/SeparatoG.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <Xm/MwmUtil.h>
#include <Dt/ComboBox.h>
#include <Dt/Help.h>
#include "misc.h"
#include "help.h"
#include "datefield.h"
#include "util.h"
#include "calendar.h"
#include "props_pu.h"
#include "timeops.h"
#include "contract_xbm"
#include "expand_xbm"
#include "postup_xbm"
#include "xm_error_xbm"
#include "xm_information_xbm"
#include "xm_question_xbm"
#include "xm_warning_xbm"

#ifdef FNS
#include "cmfns.h"
#endif /* FNS */

/*******************************************************************************
**
**  External functions
**
*******************************************************************************/
/*
**  Widget building functions
*/
extern Widget
create_repeat_menu(Widget parent, XtCallbackProc cb_func, XtPointer data) {

  int			ac;
  Arg			args[5];
  Widget			cb;
  XmString		xmstr;
  Repeat_menu_op		i;
  extern			Calendar *calendar;

  ac = 0;
  XtSetArg(args[ac], DtNcomboBoxType, DtDROP_DOWN_LIST); ++ac;
  cb = DtCreateComboBox(parent, "repeat_cb", args, ac);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 200, "One Time"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 853, "Daily"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 854, "Weekly"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 855, "Every Two Weeks"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 856, "Monthly By Date"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 857, "Monthly By Weekday"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 858, "Yearly"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 859, "Monday Thru Friday"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 860, "Mon, Wed, Fri"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 861, "Tuesday, Thursday"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 901, "Repeat Every ..."));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  if (cb_func) {
    XtAddCallback(cb, DtNselectionCallback, cb_func, data);
  }

  return cb;
}

extern Widget
create_for_menu(Widget parent) {

  extern Calendar	*	calendar;
  int				ac;
  Arg				args[5];
  Widget			cb;
  Widget			list;
  Widget			text;
  XmString			xmstr;
  For_menu_op			i;

  ac = 0;
  XtSetArg(args[ac], DtNcomboBoxType, DtDROP_DOWN_COMBO_BOX); ++ac;
  cb = DtCreateComboBox(parent, "for_cb", args, ac);

  /*
   * This is a hack until the sizing bug in the combo box is fixed.
   * To get it to size correctly, you must managed it and then reset
   * the width.
   */
  XtVaSetValues(cb, XmNwidth, 80, NULL);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 863, "2"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 864, "3"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 865, "4"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 866, "5"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 867, "6"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 868, "7"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 869, "8"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 870, "9"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 871, "10"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 872, "11"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 873, "12"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 874, "13"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 875, "14"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 876, "forever"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  XtVaGetValues(cb, DtNlist, &list, DtNtextField, &text, NULL);
  XtVaSetValues(list, XmNsensitive, False, NULL);
  XtVaSetValues(text, XmNsensitive, False, NULL);

  return cb;
}

extern Widget
create_privacy_menu(Widget parent) {
  extern Calendar *	calendar;
  int			ac;
  int			i;
  Arg			args[5];
  Widget		cb;
  XmString		xmstr;

  ac = 0;
  XtSetArg(args[ac], DtNcomboBoxType, DtDROP_DOWN_LIST); ++ac;
  XtSetArg(args[ac], XmNvisibleItemCount, 3); ++ac;
  cb = DtCreateComboBox(parent, "privacy_cb", args, ac);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 721, "Others See Time And Text"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 722, "Others See Time Only"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(calendar->DT_catd, 1, 723, "Others See Nothing"));
  DtComboBoxAddItem((Widget)cb, xmstr, 0, False);
  XmStringFree(xmstr);

  return cb;
}


extern void
set_time_submenu(Widget 	parent, 
		 Widget 	option_menu, 
		 Props 	*	p,
		 XtCallbackProc callback, 
		 XtPointer 	client_data, 
		 Boolean	nt, 
		 Boolean 	ad, 
		 Widget **	widget_list, 
		 int 	*	widget_count) {

  Calendar	*	c = calendar;
  XmString		xmstr;
  int			i;
  int			j;
  int			allocation_counter = 0;
  int			beg = get_int_prop(p, CP_DAYBEGIN);
  int			end = get_int_prop(p, CP_DAYEND);
  char			buf[4];
  char			buf2[10];
  Widget		menu;
  Widget		menuitem;

  DisplayType	dt = get_int_prop(p, CP_DEFAULTDISP);

  XtVaGetValues(option_menu, XmNsubMenuId, &menu, NULL);

  for (i = 0; i < *widget_count; i++) {
    XtDestroyWidget((*widget_list)[i]);
  }

  if (*widget_list) {
    free(*widget_list);
  }

  *widget_count = ((end - beg) * 4);

  if (nt == True) {
    (*widget_count)++;
  }

  if (ad == True) {
    (*widget_count)++;
  }

  *widget_list = calloc(sizeof(Widget), *widget_count);

  /*
  **  Loop through and create the time menu using the starting and
  **  ending times from the properties.
  */
  for (i = beg; i < end; i++) {
    if (dt == HOUR12) {
      if (i > 12) {
	sprintf(buf, "%2d:", i-12);
      } else if (i == 0) {
	cm_strcpy(buf, "12:");
      } else {
	sprintf(buf, "%2d:", i);	
      }
    } else {
      sprintf(buf, "%02d", i);
    }
    
    for (j = 0; j <= 45; j += 15) {
      sprintf(buf2, "%s%02d", buf, j);
      (*widget_list)[allocation_counter++] =
	menuitem = XtVaCreateWidget(buf2,
				    xmPushButtonGadgetClass,
				    menu,
				    XmNuserData,
				    (i * hrsec) + (j * minsec),
				    NULL);
      if (callback) {
	XtAddCallback(menuitem, XmNactivateCallback,
		      callback, client_data);
      }
    }
  }

  if (nt) {
    xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 353, "No Time"));
    (*widget_list)[allocation_counter++] =
      menuitem = XtVaCreateWidget("noTime",
				  xmPushButtonGadgetClass, menu,
				  XmNlabelString, xmstr,
				  XmNuserData, NO_TIME,
				  NULL);
    XmStringFree(xmstr);
    if (callback) {
      XtAddCallback(menuitem, XmNactivateCallback,
		    callback, client_data);
    }
  }

  if (ad) {
    xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 354, "All Day"));
    (*widget_list)[allocation_counter++] =
      menuitem = XtVaCreateWidget("allDay",
				  xmPushButtonGadgetClass, menu,
				  XmNlabelString, xmstr,
				  XmNuserData, ALL_DAY,
				  NULL);
    XmStringFree(xmstr);
    if (callback) {
      XtAddCallback(menuitem, XmNactivateCallback,
		    callback, client_data);
    }
  }

  XtManageChildren(*widget_list, allocation_counter);

  return;
}

extern Widget
create_start_stop_time_menu(Widget		parent, 
			    XmString		label,
			    XtCallbackProc	callback, 
			    XtPointer		client_data, 
			    Props	*	p, 
			    Boolean		nt, 
			    Boolean		ad, 
			    Widget	**	widget_list, 
			    int		*	widget_count) {

  int		ac;
  Arg		args[5];
  Widget	menu;
  Widget	option_m;
  int		beg = get_int_prop(p, CP_DAYBEGIN);
  int		end = get_int_prop(p, CP_DAYEND);

  menu = XmCreatePulldownMenu(parent, "pulldown", NULL, 0);
  XtVaSetValues(menu,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmHORIZONTAL,
		XmNnumColumns, (end - beg) + 1,
		NULL); 
  ac = 0;
  XtSetArg(args[ac], XmNsubMenuId, menu); ++ac;
  XtSetArg(args[ac], XmNlabelString, label); ++ac;
  option_m = XmCreateOptionMenu(parent, "option_m", args, ac);

  set_time_submenu(parent, option_m, p, callback, 
		   client_data, nt, ad, widget_list, widget_count);
  
  return option_m;
}

extern Widget
create_time_scope_menu(Widget		parent, 
		       XmString 	label, 
		       XtCallbackProc	callback, 
		       XtPointer	client_data) {

  int			ac;
  Arg			args[5];
  Widget		menu;
  Widget		option_m;
  Widget		menuitems[3];
  XmString		xmstr;
  Calendar	*	c = calendar;

  menu = XmCreatePulldownMenu(parent, "pulldown", NULL, 0);
  XtVaSetValues(menu,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmHORIZONTAL,
		XmNnumColumns, TIME_DAYS + 1,
		NULL);

  ac = 0;
  XtSetArg(args[ac], XmNsubMenuId, menu); ++ac;
  XtSetArg(args[ac], XmNlabelString, label); ++ac;
  option_m = XmCreateOptionMenu(parent, "option_m", args, ac);


  xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 877, "Mins"));
  menuitems[0] = XtVaCreateWidget("timescope0",
				  xmPushButtonGadgetClass, menu,
				  XmNlabelString, xmstr,
				  XmNuserData, 0,
				  NULL);
  if (callback) {
    XtAddCallback(menuitems[0], XmNactivateCallback,
		  callback, client_data);
  }

  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 878, "Hrs"));
  menuitems[1] = XtVaCreateWidget("timescope1",
				  xmPushButtonGadgetClass, menu,
				  XmNlabelString, xmstr,
				  XmNuserData, 1,
				  NULL);
  if (callback) {
    XtAddCallback(menuitems[1], XmNactivateCallback,
		  callback, client_data);
  }
  XmStringFree(xmstr);

  xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 879, "Days"));
  menuitems[2] = XtVaCreateWidget("timescope2",
				  xmPushButtonGadgetClass, menu,
				  XmNlabelString, xmstr,
				  XmNuserData, 2,
				  NULL);
  if (callback) {
    XtAddCallback(menuitems[2], XmNactivateCallback,
		  callback, client_data);
  }
  XmStringFree(xmstr);

  XtVaSetValues(XmOptionButtonGadget(option_m), XmNsensitive, False, NULL);

  XtManageChildren((WidgetList)menuitems, 3);

  return option_m;
}

extern Widget
create_repeat_scope_menu(Widget parent, XmString label,
			 XtCallbackProc callback, XtPointer client_data) {

  int			ac;
  Arg			args[5];
  char			buf[MAXNAMELEN];
  Widget		menu;
  Widget		option_m;
  Widget		menuitems[3];
  XmString		xmstr;
  Repeat_scope_menu_op	i;
  Calendar	*	c = calendar;

  menu = XmCreatePulldownMenu(parent, "pulldown", NULL, 0);
  XtVaSetValues(menu,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmHORIZONTAL,
		XmNnumColumns, REPEAT_MONTHS + 1,
		NULL);
  
  ac = 0;
  XtSetArg(args[ac], XmNsubMenuId, menu); ++ac;
  XtSetArg(args[ac], XmNlabelString, label); ++ac;
  option_m = XmCreateOptionMenu(parent, "option_m", args, ac);

  sprintf(buf, "repeatscope%d", REPEAT_DAYS); 
  xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 816, "Days"));;
  menuitems[0] = XtVaCreateWidget(buf,
				  xmPushButtonGadgetClass, menu,
				  XmNlabelString, xmstr,
				  XmNuserData, REPEAT_DAYS,
				  NULL);
  if (callback) {
    XtAddCallback(menuitems[0], XmNactivateCallback,
		  callback, client_data);
  }
  XmStringFree(xmstr);

  sprintf(buf, "repeatscope%d", REPEAT_WEEKS); 
  xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 817, "Weeks"));;
  menuitems[1] = XtVaCreateWidget(buf,
				  xmPushButtonGadgetClass, menu,
				  XmNlabelString, xmstr,
				  XmNuserData, REPEAT_WEEKS,
				  NULL);
  if (callback) {
    XtAddCallback(menuitems[1], XmNactivateCallback,
		  callback, client_data);
  }
  XmStringFree(xmstr);

  sprintf(buf, "repeatscope%d", REPEAT_MONTHS); 
  xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 818, "Months"));;
  menuitems[2] = XtVaCreateWidget(buf,
				  xmPushButtonGadgetClass, menu,
				  XmNlabelString, xmstr,
				  XmNuserData, REPEAT_MONTHS,
				  NULL);
  if (callback) {
    XtAddCallback(menuitems[2], XmNactivateCallback,
		  callback, client_data);
  }
  XmStringFree(xmstr);

  XtManageChildren((WidgetList)menuitems, 3);

  return option_m;
}

static void
fill_pixmap(XImage *xi, int w, int h, unsigned char *data)
{
  xi->width = w;
  xi->height = h;
  xi->data = (char *) data;
  xi->xoffset = 0;
  xi->format = XYBitmap;
  xi->byte_order = MSBFirst;
  xi->bitmap_pad = 8;
  xi->bitmap_bit_order = LSBFirst;
  xi->bitmap_unit = 8;
  xi->depth = 1;
  xi->bytes_per_line = (int)((w - 1) / 8) + 1;
  xi->obdata = NULL;
}

extern void
create_all_pixmaps(Props_pu *p, Widget w)
{
  Pixel           fg, bg;
  Screen          *s;
  static XImage   ic, ei, ci, li, ri, eri, ii, pui, qi, wi;

  s = XtScreen(w);
  XtVaGetValues(w, XmNforeground, &fg, XmNbackground, &bg, NULL);

  fill_pixmap(&ei, expand_xbm_width, expand_xbm_height, expand_xbm_bits);
  if (XmInstallImage(&ei, "x_pixmap")) {
    p->expand_pixmap = XmGetPixmap(s, "x_pixmap", fg, bg);
  }

  fill_pixmap(&ci, contract_xbm_width, contract_xbm_height,contract_xbm_bits);
  if (XmInstallImage(&ci, "c_pixmap")) {
    p->contract_pixmap = XmGetPixmap(s, "c_pixmap", fg, bg);
  }

  fill_pixmap(&pui, postup_xbm_width, postup_xbm_height, postup_xbm_bits);
  if (XmInstallImage(&pui, "postup_pixmap")) {
    p->postup_pixmap = XmGetPixmap(s, "postup_pixmap", fg, bg);
  }

  fill_pixmap(&eri, xm_error_xbm_width, xm_error_xbm_height, xm_error_xbm_bits);
  if (XmInstallImage(&eri, "err_pixmap")) {
    p->xm_error_pixmap = XmGetPixmap(s, "err_pixmap", fg, bg);
  }
 
  fill_pixmap(&ii, xm_information_xbm_width, xm_information_xbm_height,
	      xm_information_xbm_bits);
  if (XmInstallImage(&ii, "info_pixmap")) {
    p->xm_info_pixmap = XmGetPixmap(s, "info_pixmap", fg, bg);
  }
 
  fill_pixmap(&qi, xm_question_xbm_width, xm_question_xbm_height,
	      xm_question_xbm_bits);
  if (XmInstallImage(&qi, "q_pixmap")) {
    p->xm_question_pixmap = XmGetPixmap(s, "q_pixmap", fg, bg);
  }
 
  fill_pixmap(&wi, xm_warning_xbm_width, xm_warning_xbm_height,
	      xm_warning_xbm_bits);
  if (XmInstallImage(&wi, "w_pixmap")) {
    p->xm_warning_pixmap = XmGetPixmap(s, "w_pixmap", fg, bg);
  }

  if ((p->drag_icon_xbm = XmGetPixmap(s, "DtCMdnd.m.pm", fg, bg)) == XmUNSPECIFIED_PIXMAP) {
    p->drag_icon_xbm = NULL;
  }
  return;
}


/*
**  I18N related routine
**  Parameter:  A pointer to a multibyte string or a pointer to NULL.
**  Return Value:  Returns a pointer to the next multibyte character.
**  Usage:  If the actual argument is non NULL then a pointer to the first
**          multibyte character is returned.  If the actual argument is NULL
**          then a pointer to the next multibyte character is returned.  The
**          parameter scheme is very much like strtok();
**  CAUTION:  If the calling function uses the return value then it should
**            make a copy.  The return value is a static buffer that may
**            be overwritten or freed on subsequent calls to this routine.
*/
extern char *
cm_mbchar(char *str) {

  static char *	string;
  static char *	string_head;
  static char *	buf;
  int		num_byte = 0;
 
  if ( str != NULL ) {
    if ( string != NULL ) {
      free(string_head);
      string_head = NULL;
      string = NULL;
    }
    string = (char *)cm_strdup(str);
    string_head = string;
  }
  if ( buf != NULL ) {
    free(buf);
    buf = NULL;
  }
  if ( string == '\0' ) {
    free(string_head);
    string_head = NULL;
  } else {
    num_byte = mblen(string, MB_LEN_MAX);
    buf = (char *)malloc(num_byte+1);
    strncpy(buf, string, num_byte);
    buf[num_byte] = '\0';
    string += num_byte;
  }
 
  return buf;
}

/*
**  Callback for generic dialogs to set the answer so the program can continue
**  in a normal flow ...
**
**  This is really frickin brain dead.
**
**  Client data will be set to the button id stored as user data in the widget.
*/
static void
response(Widget w, XtPointer client_data, XtPointer cbs) {
  XtVaGetValues(w, XmNuserData, (int *)client_data, NULL);
}

/*
**  Generic dialog box creator
*/
extern int
dialog_popup(Widget parent, ...) {
  int			button_id;
  int			t_cnt = 0;
  int			b_cnt = 0;
  char		*	text_str = NULL;
  char		*	ptr;
  char			buf[MAXNAMELEN];
  char		*	help_str;
  Pixmap		px;
  va_list		pvar;
  Widget		frame, form, image, sep, text, last_text, button;
  Display	*	dpy = XtDisplayOfObject(parent);
  XmString		xmstr;
  Dialog_create_op	op;
  extern XtAppContext	app;
  Calendar 	*	c = calendar;

  static long		answer;

  extern XtCallbackProc 	help_cb(Widget, XtPointer, XtPointer);

  frame = XtVaCreatePopupShell("generic_dialog",
			       xmDialogShellWidgetClass, parent,
			       XmNallowShellResize, True,
			       XmNmwmFunctions, MWM_FUNC_MOVE | MWM_FUNC_CLOSE,
			       NULL);

  form = XtVaCreateWidget("form",
			  xmFormWidgetClass, frame,
			  XmNhorizontalSpacing, 5,
			  XmNverticalSpacing, 5,
			  XmNmarginWidth, 0,
			  XmNfractionBase, 100,
			  XmNautoUnmanage,        False,
			  NULL);

  image = XtVaCreateWidget("image",
			   xmLabelGadgetClass, form,
			   XmNlabelType, XmPIXMAP,
			   XmNtopAttachment, XmATTACH_FORM,
			   XmNleftAttachment, XmATTACH_FORM,
			   NULL);

  sep = XtVaCreateWidget("sep",
			 xmSeparatorGadgetClass, form,
			 XmNtopAttachment, XmATTACH_WIDGET,
			 XmNleftAttachment, XmATTACH_FORM,
			 XmNrightAttachment, XmATTACH_FORM,
			 NULL);

  va_start(pvar, parent);
  op = va_arg(pvar, Dialog_create_op);
  while(op) {
    switch(op) {
    case DIALOG_TITLE:
      ptr = va_arg(pvar, char *);
      XtVaSetValues(frame, XtNtitle, ptr,
		    NULL);
      break;

    case DIALOG_TEXT:
      ptr = va_arg(pvar, char *);
      if (text_str) {
	free(text_str);
      }
      text_str = cm_strdup(ptr);
      break;

    case BUTTON_IDENT:
    case BUTTON_INSENSITIVE:
      ++b_cnt;
      button_id = va_arg(pvar, int);
      ptr = va_arg(pvar, char *);
      xmstr = XmStringCreateLocalized(ptr);
      sprintf(buf, "button%d", b_cnt);
      button = XtVaCreateWidget(buf,
				xmPushButtonGadgetClass, form,
				XmNlabelString, xmstr,
				XmNleftAttachment, XmATTACH_POSITION,
				XmNleftPosition, (b_cnt * 2) - 1,
				XmNrightAttachment, XmATTACH_POSITION,
				XmNrightPosition, (b_cnt * 2) + 1,
				XmNrightOffset,	5,
				XmNuserData, button_id,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, sep,
				XmNtopOffset, 10,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNbottomOffset, 10,
				NULL);
      if (op == BUTTON_INSENSITIVE) {
	XtSetSensitive(button, False);
      }
      XtAddCallback(button, XmNactivateCallback,
		    response,	&answer);
      XmStringFree(xmstr);
      break;

    case BUTTON_HELP:
      ++b_cnt;
      help_str = cm_strdup(va_arg(pvar, char *));
      xmstr = XmStringCreateLocalized(catgets(c->DT_catd, 1, 77, "Help"));
      button = XtVaCreateWidget("help_button",
				xmPushButtonGadgetClass, form,
				XmNlabelString, xmstr,
				XmNleftAttachment, XmATTACH_POSITION,
				XmNleftPosition, (b_cnt * 2) - 1,
				XmNrightAttachment, XmATTACH_POSITION,
				XmNrightPosition, (b_cnt * 2) + 1,
				XmNrightOffset,	5,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, sep,
				XmNtopOffset, 10,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNbottomOffset, 10,
				NULL);
      XtAddCallback(button, XmNactivateCallback,
		    (XtCallbackProc)help_cb, help_str);
      XmStringFree(xmstr);
      break;

    case DIALOG_IMAGE:
      px = va_arg(pvar, Pixmap);
      XtVaSetValues(image, XmNlabelPixmap, px, NULL);
      break;

    default:
      break;
    }
    op = va_arg(pvar, Dialog_create_op);
  }
  va_end(pvar);
  XtVaSetValues(form, XmNfractionBase, (b_cnt + 1) * 2, NULL);

  ptr = strtok(text_str, "\n");
  while (ptr) {
    ++t_cnt;
    sprintf(buf, "text_label%d", t_cnt);
    xmstr = XmStringCreateLocalized(ptr);
    text = XtVaCreateWidget(buf,
			    xmLabelGadgetClass, form,
			    XmNalignment, XmALIGNMENT_BEGINNING,
			    XmNlabelString, xmstr,
			    XmNleftAttachment, XmATTACH_WIDGET,
			    XmNleftWidget, image,
			    XmNrightAttachment, XmATTACH_FORM,
			    NULL);
    XmStringFree(xmstr);

    if (t_cnt == 1) {
      XtVaSetValues(text, XmNtopAttachment, XmATTACH_FORM, NULL);
    } else {
      XtVaSetValues(text, XmNtopAttachment, XmATTACH_WIDGET,
		    XmNtopWidget, last_text,
		    NULL);
    }
    last_text = text;

    ptr = strtok(NULL, "\n");
  }
  free(text_str);

  XtVaSetValues(sep,
		XmNtopWidget, last_text,
		XmNtopOffset, 10,
		NULL);
  ManageChildren(form);
  XtManageChild(form);

  XBell(dpy, 50);
  answer = 0;
  XtPopup(frame, XtGrabExclusive);
  while (answer == 0) {
    XtAppProcessEvent(app, XtIMXEvent | XtIMAll);
  }
  XtPopdown(frame);

  return answer;
}

/*
**  This routine returns the true boundaries of a particular view, i.e,
**  12 midnight - 11:59:59 pm.  It does not return the boundaries for doing
**  lookups, i.e., 11:59:59 pm - 12 pm (lookup ranges must encompass true range)
*/
extern void
get_range(Glance glance, time_t date, time_t *start, time_t *stop) {

  int	day_of_week;

  switch(glance) {
  case monthGlance:
    *start = lowerbound(last_ndays(date, dom(date)-1));
    *stop = next_ndays(*start, monthdays[month(date)-1])-1;
    break;

  case weekGlance:
    day_of_week = (((day_of_week = dow(date)) == 0)	?
		   6 : --day_of_week);
    *start = lowerbound(date - (day_of_week * daysec));
    *stop = next_ndays(*start, 7)-1;
    break;

  case dayGlance:
    *start = lowerbound(date);
    *stop = next_nhours(*start, 24)-1;
    break;

  case yearGlance:
        *start = lowerbound(jan1(date));
	*stop = lowerbound(nextjan1(date));
	break;

	default:
	  *start = lowerbound(jan1(date));
	  *stop = lowerbound(nextjan1(date));
	  break;
  }
  return;
}

/*
**  Standard set of error messages for the editors (errors from data entry
**  validation and from the backend).
*/
extern void
editor_err_msg(Widget frame, char *name, Validate_op op, Pixmap p) {

  Calendar	*	c = calendar;
  char			buf[MAXNAMELEN];
  char			buf2[MAXNAMELEN * 2];
  char			error_buf[64];
  char		*	title;
  char		*	ident1;
  int			help_button = True;

  switch(op) {
  case COULD_NOT_OPEN_FILE:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 355,
		    "Unable to open callog file.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, CMSD_ERROR_HELP);
    break;

  case CANCEL_APPT:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 356,
		    "Per user request, Operation was cancelled.\nCalendar"));
    help_button = False;
    break;

  case INVALID_DATE:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 357,
		    "Invalid value in DATE field.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, DATE_ERROR_HELP);
    break;

  case INVALID_START:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 358,
		    "Invalid value in START field.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, START_ERROR_HELP);
    break;

  case INVALID_TIME_DUE:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 1011,
		    "Invalid value in the \"Time Due\" field.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, START_ERROR_HELP);
    break;

  case INVALID_STOP:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 359,
		    "Invalid value in END field.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, STOP_ERROR_HELP);
    break;

  case MISSING_DATE:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 360,
		    "You must provide a DATE value.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, MISSING_FIELD_ERROR_HELP);
    break;

  case MISSING_START:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 361,
		    "You must provide a START value.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, MISSING_FIELD_ERROR_HELP);
    break;

  case MISSING_WHAT:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 362,
		    "You must provide a WHAT value.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, MISSING_FIELD_ERROR_HELP);
    break;

  case REPEAT_FOR_MISMATCH:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 363,
		    "Invalid or mismatched REPEAT and FOR values.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, REPEAT_FOR_ERROR_HELP);
    break;

  case INVALID_NOTIME_APPT:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 724,
		    "No Time appointments must have a WHAT value specified.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, NO_TIME_ERROR_HELP);
    break;

  case INVALID_TIME:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 1104,
		    "Invalid value in TIME DUE field.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, START_ERROR_HELP);
    break;

  case MISSING_TIME:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 1105,
		    "You must provide a TIME DUE value.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, MISSING_FIELD_ERROR_HELP);
    break;

  case VALID_APPT:
    /**FALLTHRU*/

  default:
    return;
  }

  sprintf(buf2, "%s %s", buf, name);
  title = XtNewString(catgets(c->DT_catd, 1, 914,
			      "Calendar : Error - Editor"));
  ident1 = XtNewString(catgets(c->DT_catd, 1, 95, 
			       "Continue"));

  if (help_button) {
    dialog_popup(frame, DIALOG_TITLE, title,
		 DIALOG_TEXT, buf2,
		 BUTTON_IDENT, 1, ident1,
		 BUTTON_HELP, error_buf,
		 DIALOG_IMAGE, p,
		 NULL);
  } else {
    ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
    dialog_popup(frame, DIALOG_TITLE, title,
		 DIALOG_TEXT, buf2,
		 BUTTON_IDENT, 1, ident1,
		 DIALOG_IMAGE, p,
		 NULL);
  }
  XtFree(ident1);
  XtFree(title);
  
  return;
}

extern void
backend_err_msg(Widget 		 frame, 
		char 		*name, 
		CSA_return_code	 status, 
		Pixmap 		 p)
{
  Calendar	*	c = calendar;
  char			buf[MAXNAMELEN];
  char			buf2[MAXNAMELEN * 2];
  char			error_buf[64];
  char		*	title;
  char		*	ident1;
  int		 	help_button = True;

  sprintf(error_buf, BACK_END_ERROR_HELP);

  switch(status) {
  case CSA_SUCCESS:
    return;

  case CSA_X_DT_E_NFS_MOUNTED_CALLOG:
   sprintf(buf, catgets(c->DT_catd, 1, 2120, "NFS Mounted Callog (Calendar) file Not Supported."));
   help_button = False;
   break;

  case CSA_E_CALENDAR_EXISTS:
    sprintf(buf, catgets(c->DT_catd, 1, 367, "Calendar already exists.\nOperation was cancelled.  Calendar"));
    help_button = False;
    break;

  case CSA_E_CALENDAR_NOT_EXIST:
    sprintf(buf, catgets(c->DT_catd, 1, 368, "Calendar does not exist.\nOperation was cancelled.  Calendar"));
    help_button = False;
    break;

  case CSA_X_DT_E_ENTRY_NOT_FOUND:
    sprintf(buf, catgets(c->DT_catd, 1, 725, "Calendar Entry does not exist.\nOperation was cancelled.  Calendar"));
    break;

  case CSA_E_INVALID_ENTRY_HANDLE:
    sprintf(buf, catgets(c->DT_catd, 1, 881, "Internal Error #1: Invalid calendar/entry handle.\nOperation was cancelled.  Calendar"));
    break;

  case CSA_E_INVALID_SESSION_HANDLE:
    sprintf(buf, catgets(c->DT_catd, 1, 882, "Internal Error #2: Invalid session handle.\nOperation was cancelled.  Calendar"));
    break;

  case CSA_E_NO_AUTHORITY:
    sprintf(buf, catgets(c->DT_catd, 1, 370, "Authorization error.  Permission denied.\nOperation was cancelled.  Calendar"));
    help_button = False;
    break;

  case CSA_E_INVALID_PARAMETER:
    sprintf(buf, catgets(c->DT_catd, 1, 883, "Internal Error #3: Invalid data value.\nOperation was cancelled.  Calendar"));
    break;

  case CSA_E_READONLY:
    sprintf(buf, catgets(c->DT_catd, 1, 884, "Internal Error #4: Value specified for a read-only attribute.\nOperation was cancelled.  Calendar"));
    break;

  case CSA_E_INVALID_ATTRIBUTE_VALUE:
    sprintf(buf, catgets(c->DT_catd, 1, 885, "Internal Error #5: Incorrect data type specified for an attribute value.\nOperation was cancelled.  Calendar"));
    break;

  case CSA_E_UNSUPPORTED_ATTRIBUTE:
    sprintf(buf, catgets(c->DT_catd, 1, 886, "Internal Error #6: Specified attribute is not supported on this calendar version.\nOperation was cancelled.  Calendar"));
    break;

  case CSA_E_NOT_SUPPORTED:
    sprintf(buf, catgets(c->DT_catd, 1, 887, "Internal Error #7: Specified function is not supported on this calendar version.\nOperation was cancelled.  Calendar"));
    break;

  case CSA_E_INVALID_ENUM:
    sprintf(buf, catgets(c->DT_catd, 1, 888, "Internal Error #8: Invalid operator specified.\nOperation was cancelled.  Calendar"));
    break;

  case CSA_E_INSUFFICIENT_MEMORY:
    sprintf(buf, catgets(c->DT_catd, 1, 378, "Memory allocation error - not enough memory.\nOperation was cancelled.  Calendar"));
    sprintf(error_buf, MEMORY_ALLOC_ERROR_HELP);
    break;

  case CSA_E_DISK_FULL:
    sprintf(buf, catgets(c->DT_catd, 1, 907, "No space left for calendar file.\nOperation was cancelled.  Calendar"));
    break;

  case CSA_E_SERVICE_UNAVAILABLE:
    sprintf(buf, catgets(c->DT_catd, 1, 921, "Couldn't access calendar: "));
    break;

  case CSA_X_DT_E_INVALID_SERVER_LOCATION:
    sprintf(buf, catgets(c->DT_catd, 1, 641, "Server failed to get network address\nfor the specified location:  invalid hostname."));
    break;

  case CSA_X_DT_E_SERVICE_NOT_REGISTERED:
    sprintf(buf, catgets(c->DT_catd, 1, 642, "rpc.cmsd daemon is not registered.\nPlease verify rpc.cmsd is installed correctly on your system."));
    sprintf(error_buf, CMSD_ERROR_HELP);
    break;

  case CSA_X_DT_E_SERVER_TIMEOUT:
    sprintf(buf, catgets(c->DT_catd, 1, 727, "Your request timed out\nPlease verify rpc.cmsd is installed correctly on your system."));
    sprintf(error_buf, CMSD_ERROR_HELP);
    break;

  case CSA_E_FAILURE:
    sprintf(buf, catgets(c->DT_catd, 1, 643, "Couldn't access calendar. Calendar"));
    break;

#ifdef FNS
  case CMFNS_E_NAME_NOT_BOUND:
    sprintf(buf, catgets(c->DT_catd, 1, 1071,
			 "Calendar %s is not registered in the namespace.\nNeed to supply %s@<hostname>.\nOperation was cancelled. Calendar"),
	    name, name);
    break;
#endif /* FNS */

  default:
    sprintf(buf,
	    catgets(c->DT_catd, 1, 383,
		    "Unknown error(%u).\nOperation was cancelled.  Calendar"),
	    status);
    break;
  }

  sprintf(buf2, "%s %s", buf, name);
  title = XtNewString(catgets(c->DT_catd, 1, 890,
				    "Calendar : Error - Services"));
  ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
  if (help_button) {
    dialog_popup(frame, DIALOG_TITLE, title,
		 DIALOG_TEXT, buf2,
		 BUTTON_IDENT, 1, ident1,
		 BUTTON_HELP, error_buf,
		 DIALOG_IMAGE, p,
		 NULL);
  } else {
    dialog_popup(frame, DIALOG_TITLE, title,
		 DIALOG_TEXT, buf2,
		 BUTTON_IDENT, 1, ident1,
		 DIALOG_IMAGE, p,
		 NULL);
  }
  XtFree(ident1);
  XtFree(title);

  return;
}

/*
 * This function is called by the parse_appt_from_file routine if the end_date
 * was before the start date for the appointment - this queries the user as to
 * her intentions
 */
extern boolean_t
query_user(void *user_data) {
  Calendar	*	c = (Calendar *)user_data;
  Props_pu	*	p = (Props_pu *)c->properties_pu;
  boolean_t		ret = _B_FALSE;
  char			buf[MAXNAMELEN];
  char		*	title;
  char		*	ident1;
  char		*	ident2;
  static int		answer;

  sprintf(buf, "%s", catgets(c->DT_catd, 1, 386,
			     "That appointment has an end time earlier than its begin time.\nDo you want to schedule it into the next day?"));

  title = XtNewString(catgets(c->DT_catd, 1, 248,
			      "Calendar : Schedule Appointment"));

  ident1 = XtNewString(catgets(c->DT_catd, 1, 923, "Cancel"));
  ident2 = XtNewString(catgets(c->DT_catd, 1, 389, "Next Day"));
  answer = dialog_popup(c->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, buf,
			BUTTON_IDENT, 1, ident1,
			BUTTON_IDENT, 2, ident2,
			DIALOG_IMAGE, p->xm_question_pixmap,
			NULL);
  XtFree(ident2);
  XtFree(ident1);
  XtFree(title);
  if (answer == 2) {
    ret = _B_TRUE;
  }

  return ret;
}

extern void
set_message(Widget w, char *msg) {
  
  XmString	xmstr;

  /*
   * If msg is NULL or a null string set it to be a space.  This
   * prevents label widgets from shrinking (which they do when they
   * have no text
   */
  if (msg == NULL || *msg == '\0') {
    msg = " ";
  }

  if (w) {
    xmstr = XmStringCreateLocalized(msg);
    XtVaSetValues(w, XmNlabelString, xmstr, NULL);
    XmStringFree(xmstr);
  }
  return;
}

extern void
setup_quit_handler(Widget w, XtCallbackProc cb, caddr_t user_data) {

  Atom delete_window;

  delete_window = XmInternAtom(XtDisplay(w), "WM_DELETE_WINDOW", False);
  XmAddWMProtocolCallback(w, delete_window, cb, user_data);

  return;
}

int
ds_is_double_click(last_event, event )
XEvent           *last_event;
XEvent           *event;
{
  static  int             time_threshold;
  static  int             dist_threshold;
  static  short           first_time      = TRUE;
  short                   ret_value       = FALSE;
  int                     delta_time;
  int                     delta_x, delta_y;

  if (last_event == NULL || event == NULL) {
    return ret_value;
  }

  /* first time this is called init the thresholds */
  if( first_time ) {
    /* Get time threshold in miliseconds */
    time_threshold = 800;
    dist_threshold = 4;
    first_time      = FALSE;
  }
  /* only deal with the down events */
  if (event->type == ButtonRelease) {
    return ret_value;
  }

  if ((event->xbutton.type == ButtonPress) &&
      (last_event->xbutton.type == ButtonPress)) {
 
    delta_time = event->xbutton.time - last_event->xbutton.time;
    delta_time += event->xbutton.time;
    delta_time -= last_event->xbutton.time;
 
    /* is the time within bounds? */
    if( delta_time <= time_threshold ) {
 
      /* check to see if the distance is ok */
      delta_x = (last_event->xbutton.x > event->xbutton.x ?
		 last_event->xbutton.x - event->xbutton.x :
		 event->xbutton.x - last_event->xbutton.x);
      
      delta_y = (last_event->xbutton.y > event->xbutton.y ?
		 last_event->xbutton.y - event->xbutton.y :
		 event->xbutton.y - last_event->xbutton.y);
      if( delta_x <= dist_threshold &&
	  delta_y <= dist_threshold )
	ret_value       = TRUE;
    }
  }
  return ret_value;
}        


/*
 * Select the contents of a text item.
 *
 */
extern int
cm_select_text(Widget field, Time time) {

  XmTextPosition	last;

  last = XmTextFieldGetLastPosition(field);

  if (last > 0) {
    XmTextFieldSetSelection(field, 0, last, time);
  }

  return last;
}

extern Dimension
ComputeMaxWidth(Widget		w1,
		Widget		w2,
		Widget		w3,
		Widget		w4)
{
  Dimension	width1;
  Dimension	width2;
  Widget	w[5];
  int		i;

  if (!w1) {
    return 0;
  }

  w[0] = w1;
  w[1] = w2;
  w[2] = w3;
  w[3] = w4;
  w[4] = (Widget) NULL;

  XtVaGetValues(w1,
		XmNwidth,		&width1,
		NULL);

  for (i = 1; w[i]; i++) {

    XtVaGetValues(w[i],
		  XmNwidth,	&width2,
		  NULL);

    if ((int)width2 > (int)width1) {
      width1 = width2;
    }
  }

  return width1;
}

extern void
ManageChildren(Widget		w)
{
  WidgetList	children;
  int		n;

  XtVaGetValues(w,
		XmNchildren,            &children,
		XmNnumChildren,         &n,
		NULL);
  XtManageChildren(children, n);

  return;
}
