/*******************************************************************************
**
**  browser.h
**
**  #pragma ident "@(#)browser.h	1.18 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: browser.h /main/cde1_maint/1 1995/07/14 23:03:39 drk $
**
**  RESTRICTED CONFIDENTIAL INFORMATION:
**
**  The information in this document is subject to special
**  restrictions in a confidential disclosure agreement between
**  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
**  document outside HP, IBM, Sun, USL, SCO, or Univel without
**  Sun's specific written approval.  This document and all copies
**  and derivative works thereof must be returned or destroyed at
**  Sun's request.
**
**  Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
**
*******************************************************************************/

/*									*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#ifndef _BROWSER_H
#define _BROWSER_H

#include "ansi_c.h"
#include "timeops.h"
#include "calendar.h"

typedef struct {
	Widget          frame;
	Widget		outer_pane;
	Widget		inner_pane;
	Widget		upper_form;
	Widget		lower_form;
	Widget		action;
	Widget		cancel;
	Widget		canvas;
	Widget		edit_list;
	Widget		helpbutton;
	Widget		list_label;
	Widget		browse_list;
	Widget		browse_list_sw;
	Widget		bgoto;
	Widget		gotomenu;
	Widget		schedule;
	Widget		mail;
	Widget		message_text;
        caddr_t         current_selection;      
	Widget		box;
        Widget      	datetext;
        Widget      	datefield;
        Tick            date;
        int     	col_sel;
        int     	row_sel;
        Tick     	begin_week_tick;
        int     	canvas_w;
        int     	canvas_h;
        int     	chart_width;
        int     	chart_height;
        int     	boxh;
        int     	boxw;
        int     	chart_x;
        int     	chart_y;
        Tick    	begin_day_tick;
        Tick    	end_day_tick;
        Tick    	begin_hr_tick;
        Tick    	end_hr_tick;
	new_XContext    *xcontext;
	char	 	*multi_array;
	int		segs_in_array;
        Pixmap 		busy_icon;
	Boolean		add_to_array;
} Browser;

#define BOX_SEG 4 
#define MINS_IN_SEG (60/BOX_SEG)

#define MB_GOTO 0
#define MB_PREVWEEK 1
#define MB_THISWEEK 2
#define MB_NEXTWEEK 3
#define MB_PREVMONTH 4
#define MB_NEXTMONTH 5

extern void		br_display		(Calendar*);
extern Boolean		browser_exists		();
extern void		browser_reset_list	(Calendar*);
extern Boolean		browser_showing		();
extern void		make_browser		();
extern void		mb_draw_chartgrid	();
extern void		mb_draw_appts		();
extern void		mb_update_segs		();
extern void		mb_update_busyicon	();
extern void		mb_init_blist		();
extern void		mb_init_canvas		();
extern void		mb_deregister_names	();
extern void		mb_refresh_canvas	();
extern void		sched_proc		();
extern void		set_default_reminders	();
extern void		set_default_scope_plus	();
extern void		set_default_what	();
extern void		mb_refigure_chart	(Calendar *);

#endif
