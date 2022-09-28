/*******************************************************************************
**
**  monthglance.c
**
**  $XConsortium: monthglance.c /main/cde1_maint/3 1995/10/03 12:24:38 barstow $
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

#pragma ident "@(#)monthglance.c	1.88 96/11/12 SMI"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h> /* MAXPATHLEN defined here */
#ifdef SVR4
#include <sys/utsname.h> /* SYS_NMLN */
#endif /* SVR4 */
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <rpc/rpc.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleBG.h>
#include <Dt/HourGlass.h>
#include <Xm/List.h>
#include "calendar.h"
#include "util.h"
#include "timeops.h"
#include "datefield.h"
#include "ps_graphics.h"
#include "props.h"
#include "select.h"
#include "editor.h"
#include "group_editor.h"
#include "monthglance.h"
#include "weekglance.h"
#include "format.h"
#include "browser.h"
#include "blist.h"
#include "dayglance.h"
#include "yearglance.h"
#include "todo.h"
#include "find.h"
#include "goto.h"
#include "tempbr.h"

static void 	paint_day_entries(Tick, int, int, int, int,
				  Paint_cache *, CSA_uint32, XRectangle *);
static Boolean	allocated(Calendar *);
static void	allocator(Calendar *);
static void	deallocator(Calendar *);
extern void	layout_children(Calendar *);
static void	display_header(Calendar *);
static void	quick_button_cb(Widget, XtPointer, XtPointer);
static Boolean	print_month ( Calendar *, int, FILE *, Tick, Props *, Boolean);
static int	count_month_pages(Calendar *, Tick, int);
static void	paint_daynames(Calendar *, XRectangle *);
static void	paint_month(Calendar *, Tick, XRectangle *);
static void	unmanage_children(Calendar *);
static void	manage_children(Calendar *);

static int
compute_pad_width(
	Cal_Font	*pf)
{
	int 		 nop,
			 width;

	CalTextExtents(pf, "1", 1, &nop, &nop, &width, &nop);

	return width;
}

static void
paint_day_entries(
	Tick	day,
	int	x,
	int	y,
	int	w,
	int	h,
	Paint_cache *cache,
	CSA_uint32 appt_total,
	XRectangle *rect)
{
        char *buf;
        int i, loop, nlines = 0, maxchars;
        Calendar *c=calendar;
        Props *p = (Props*)c->properties;
	DisplayType dt = get_int_prop(p, CP_DEFAULTDISP);
        Lines *lines=NULL;
	Tick start_tick;
        new_XContext *xc    = 	c->xcontext;
	Cal_Font *pf =		c->fonts->viewfont;
	XFontSetExtents fontextents;
        int pfy;
        int maxlines;
        Tick lower      = 	lowerbound(day);
        Tick upper      = 	next_ndays(day, 1);

	CalFontExtents(pf, &fontextents);
        pfy = fontextents.max_ink_extent.height;
        maxlines = h / pfy;

	/* loop thru the list of appointments twice, first displaying 
	   the no time appointments, and then the appointments with times 
	   associated. */

	for (loop = 0; loop < 2; loop++) {
		for (i = 0; i < appt_total; i++) {
			start_tick = cache[i].start_time;
	                if (start_tick >= lower && 
			    start_tick < upper && 
			    (loop == cache[i].show_time)) {                        
				if (nlines < maxlines) {
					Boolean	pad;
					static int pad_width = 0;

	                                lines = 
					     text_to_lines(cache[i].summary, 1);
	                                if (lines != NULL) {
	                                        buf = XtCalloc(1,
						      cm_strlen(lines->s) + 18);
	                                        pad = format_line(start_tick, 
							    lines->s, buf, 
							    start_tick, 
							    cache[i].show_time, 
							    dt);
	                                }
	                                else {
	                                        buf = XtCalloc(1, 15);
	                                        pad = format_line(start_tick, 
							    (char*)NULL, buf, 
							    start_tick, 
							    cache[i].show_time,
							    dt);
	                                }
	                                destroy_lines(lines); lines=NULL;

					if (!pad_width) {
						pad_width = 
							compute_pad_width(pf);
					}
					if (pad) {
	                                	maxchars = gr_nchars(
								w - pad_width, 
								buf, pf);
	                                	if (cm_strlen(buf) > maxchars)
							buf[maxchars] = NULL;
	                                	(void)gr_text(xc, x + pad_width,
							      y, pf, buf, rect);
					} else {
	                                	maxchars = gr_nchars(w, buf, 
								     pf);
	                                	if (cm_strlen(buf) > maxchars)
							buf[maxchars] = NULL;
	                                	(void)gr_text(xc, x, y, pf, 
							      buf, rect);
					}
	                                free(buf); buf=NULL;
	                                y = y + pfy;
	                                nlines++;
	                        }
	                }
	                else
				continue;
	        }
	}
}

extern void
month_button(Widget widget, XtPointer data, XtPointer cbs)
{
	Calendar *c = calendar;

	if (c->view->glance == monthGlance)
		return;

	XtUnmapWidget(c->canvas);

	switch (c->view->glance) {
		case dayGlance:
			c->view->glance = monthGlance;
			cleanup_after_dayview(c);
			break;
		case weekGlance:
			c->view->glance = monthGlance;
			cleanup_after_weekview(c);
			break;
		case yearGlance:
			cleanup_after_yearview(c);
			c->view->glance = monthGlance;
			break;
		default:
			break;
	}
        prepare_to_paint_monthview (c, NULL);

	XtMapWidget(c->canvas);
}

/*
 * Do the actual month grid, headers and appointments
 */
static void
paint_month(Calendar *c, Tick key, XRectangle *rect)
{
        Month    	*m = (Month *)c->view->month_info;
        int 		x, y, i, j;
	CSA_uint32 	a_total;
        int 		firstdom, boxw, boxh, dayname_height, margin, default_height;
        Tick 		day;
        struct tm 	tm;
        new_XContext 	*xc;
	Dimension 	btn_ht=0, btn_w=0;
	time_t 		start, stop;
	CSA_enum 	*ops;
	CSA_attribute 	*range_attrs;
	CSA_entry_handle *list = NULL;
	XFontSetExtents fontextents;
 
        tm              = *localtime(&key);
        tm.tm_mday      = 1;
#ifdef SVR4
        tm.tm_isdst      = -1;
        day             = mktime(&tm);
#else
        day             = timelocal(&tm);
#endif /* SVR4 */
        m->ndays        = ((tm.tm_mon==1) && leapyr(tm.tm_year+1900))? 29 :
                                monthdays[tm.tm_mon];
        tm              = *localtime(&day);
        firstdom             = tm.tm_wday;
        boxw            = calendar->view->boxw;
        boxh            = calendar->view->boxh;
        margin          = calendar->view->outside_margin;
	dayname_height  = ((Month *) calendar->view->month_info)->dayname_height;
        xc              = calendar->xcontext;
 

	if (c->paint_cache == NULL) {
        	start = (time_t) lowerbound(day);
        	stop = (time_t) last_dom(day);
		setup_range(&range_attrs, &ops, &j, start, stop, CSA_TYPE_EVENT,
		    	NULL, _B_FALSE, c->general->version);
		csa_list_entries(c->cal_handle, j, range_attrs, ops, &a_total, &list, NULL);
	
		free_range(&range_attrs, &ops, j);
		allocate_paint_cache(list, a_total, &c->paint_cache);
		c->paint_cache_size = a_total;
		csa_free(list);
	}

	CalFontExtents(c->fonts->viewfont, &fontextents);
	default_height = fontextents.max_logical_extent.height;
        x = firstdom; y = 0;
 
        for(i = 1; i <= m->ndays; i++) {
		int box_origin_x=0, box_origin_y=0;

		box_origin_x = (x * boxw) + margin;
		box_origin_y = (y * boxh) + dayname_height + c->view->topoffset;

		XtVaGetValues(m->hot_button[i-1], XmNheight, &btn_ht,
			XmNwidth, &btn_w, NULL);

		/*
		 * Paint the list of calendar entries for this day
		 */
		paint_day_entries(day,
			box_origin_x + 2,
                        box_origin_y + default_height + btn_ht, 
			boxw - 4, boxh - btn_ht - 4,
			c->paint_cache, c->paint_cache_size, rect);

                day = nextday(day);
                x++;
                if (x > 6 & i != m->ndays) {
                        x = 0;
                        y++;
                }
        }
}

/*
 * Set the label header for the view to current month name
 */
static void
display_header(Calendar *c)
{
	Month 		*m = (Month *) c->view->month_info;
	char 		buf[BUFSIZ];
	XmString 	str;
	Position 	x, y;
	struct tm 	*tm_ret;
	Tick 		tmptick = c->view->date;

/* label */
	tm_ret = localtime(&tmptick);

	/* NL_COMMENT
	   Attention Translator:

	   This string is used in the calendar month view.  In the C locale
	   it has the form:

		July, 1995

	   strftime conversion string: "%B, %Y" is used.

	   Use the appropriate strftime conversion for your locale.
	*/
#ifdef AIX_ILS
	sprintf( buf, "%s, %d", months[tm_ret->tm_mon+1],
		tm_ret->tm_year + 1900 );
#else /* AIX_ILS */
	(void) strftime(buf, BUFSIZ, catgets(c->DT_catd, 1, 1091, "%B, %Y"), 
									tm_ret);
#endif /* AIX_ILS */

	str = XmStringCreateLocalized(buf);
	XtVaSetValues(m->month_label, XmNlabelString, str, NULL);
	XmStringFree(str);

/* positioning */
	x = c->view->outside_margin; y = 15;
	XtVaSetValues(m->month_label, XtNx, x, XtNy, y, NULL);

/* now manage it */
	if (!XtIsManaged(m->month_label)) {
		XtManageChild(m->month_label);
	}
}

static void
paint_daynames(Calendar *c, XRectangle    *rect)
{
        int 		i, middle;
        int 		boxw = c->view->boxw;
        int 		margin = c->view->outside_margin;
        int 		dayname_height = 
				((Month *)c->view->month_info)->dayname_height;
        Cal_Font 	*pf = c->fonts->viewfont;
        new_XContext 	*xc = c->xcontext;

        for(i = 0; i < 7; i++) {
                gr_draw_box(xc, (boxw * i)+margin,
                        c->view->topoffset,
                        boxw, dayname_height, rect);
                middle = gr_center(boxw, days[i], pf);
                gr_text(xc, (boxw*i)+middle+margin, 
				c->view->topoffset+dayname_height-3,
                        	pf, days[i], rect);
        }
}

extern void
paint_grid(Calendar *c, XRectangle *rect)
{
        int i;
        int boxh        = c->view->boxh;
        int boxw        = c->view->boxw;
        int nrows       = c->view->nwks;
        int margin      = c->view->outside_margin;
        int dayname_height = ((Month *)c->view->month_info)->dayname_height;
        int rightmargin = margin + 7 * boxw;
        int bottomargin = c->view->topoffset+ dayname_height+boxh*nrows;
        new_XContext *xc    = c->xcontext;

        /* horizontal */                   
        for (i = 1; i <= nrows; i++) {
                gr_draw_line(xc, margin,
                        (i*boxh)+c->view->topoffset+dayname_height,
                         rightmargin, i*boxh+c->view->topoffset+dayname_height,
                        gr_solid, rect);
        }
 
        /* vertical */
        for (i = 0; i < 8; i++) {
                gr_draw_line(xc, margin+(i*boxw),
                        c->view->topoffset+dayname_height,
                        margin+(i*boxw), bottomargin, gr_solid, rect);
        }
        /* embolden grid outline */
        gr_draw_box(xc, margin-1,
                c->view->topoffset-1, 7*boxw+2,
                        nrows*boxh+2+dayname_height, rect);
}

extern void
layout_month(
	Calendar	*c,
	Tick		 date)
{
	Month		*m = (Month *)c->view->month_info;
	int		 i, x, y,
			 firstdom,
			 boxw = calendar->view->boxw,
			 boxh = calendar->view->boxh,
			 margin = calendar->view->outside_margin,
			 dayname_height = m->dayname_height;
	struct	tm	 tm;
	Tick		 day;

	tm = *localtime(&date);
	tm.tm_mday = 1;
#ifdef SVR4
	tm.tm_isdst = -1;
	day = mktime(&tm);
#else
	day = timelocal(&tm);
#endif /* SVR4 */
	m->ndays = ((tm.tm_mon==1) && 
		    leapyr(tm.tm_year+1900))? 29 : monthdays[tm.tm_mon];
	tm = *localtime(&day);
	firstdom = tm.tm_wday;

	x = firstdom;
	y = 0;
	for(i = 1; i <= m->ndays; i++) {
		int	box_origin_x = 0,
			box_origin_y = 0;

		box_origin_x = (x * boxw) + margin;
		box_origin_y = (y * boxh) + dayname_height + c->view->topoffset;

		m->button_loc[i-1].x = box_origin_x + 3;
		m->button_loc[i-1].y = box_origin_y + 3;

		x++;
		if (x > 6 & i != m->ndays) {
			x = 0;
			y++;
		}
		XtMoveWidget(m->hot_button[i-1], 
				m->button_loc[i-1].x, m->button_loc[i-1].y);
	}

	/* Unmanage any unneeded navigator buttons (ie. 29, 30, 31) */
	for (i=m->ndays; i<31; i++)
		if (XtIsManaged(m->hot_button[i]))
			XtUnmanageChild(m->hot_button[i]);
}

extern void
repaint_damaged_month(
	Calendar	*c,
	XRectangle	*rect)
{
	/* allocate Month memory & widgets if necessary */
	if (!allocated(c)) {
		XFontSetExtents 	boldfontextents;

		allocator(c);
		XmToggleButtonGadgetSetState(c->month_scope, True, False);
		CalFontExtents(c->fonts->boldfont, &boldfontextents);
		((Month *) c->view->month_info)->dayname_height =  
				boldfontextents.max_logical_extent.height + 6;
		layout_month(c, c->view->date); 
		display_header(c);
		manage_children(c);
	}

        paint_daynames(c, rect);
        paint_grid(c, rect);
        paint_month(c, c->view->date, rect);
        calendar_select(c, daySelect, (caddr_t)NULL);
}

extern void
prepare_to_paint_monthview(Calendar *c, XRectangle *rect)
{
        Dimension 		w, h;
        c = calendar;

	XmToggleButtonGadgetSetState(c->month_scope, True, False);
        XtVaGetValues(c->canvas, XmNwidth, &w, XmNheight, &h, NULL);
        cache_dims(c, w, h);

	/* allocate Month memory & widgets if necessary */
	if (!allocated(c)) {
		XFontSetExtents 	boldfontextents;

		allocator(c);
		CalFontExtents(c->fonts->boldfont, &boldfontextents);
		((Month *) c->view->month_info)->dayname_height = 
				boldfontextents.max_logical_extent.height + 6;
	}
 
	/*
	 * need to unmanage buttons while drawing to avoid many exposures
	 * when they are moved
	 */
	display_header(c);
	layout_month(c, c->view->date); 
        gr_clear_area(c->xcontext, 0, 0, c->view->winw, c->view->winh);
	manage_children(c);
}

/* ADDED FOR PRINTING */
extern void
get_time_str (Dtcm_appointment *appt, char *buf)
{
        Calendar 	*c = calendar;
        Props 		*p = (Props*)c->properties;
	DisplayType 	dt = get_int_prop(p, CP_DEFAULTDISP);
        struct tm 	*tm;
        int 		hr, mn;
	Tick 		start_tick;

	_csa_iso8601_to_tick(appt->time->value->item.date_time_value, &start_tick);
	buf[0] = NULL;

        if (appt==NULL || !showtime_set(appt))
                return;

        tm = localtime(&start_tick);
        hr = tm->tm_hour;
        mn = tm->tm_min;

        if (dt == HOUR12) {
                adjust_hour(&hr);
                (void) sprintf(buf, "%2d:%02d ", hr, mn);
        }
        else
                (void) sprintf(buf, "%02d%02d ", hr, mn);
}

extern void
month_event(XEvent *event)
{
        static int 	lastcol, lastrow;
        static XEvent 	lastevent;
        int 		x, y, j, toffset;
        int 		boxw, boxh, row, col, margin, dayname_height;
        Tick 		date;
	Calendar 	*c = calendar;
        Editor 		*e = (Editor*)c->editor;
        ToDo 		*t = (ToDo*)c->todo;
        GEditor 	*ge = (GEditor*)c->geditor;

        boxw    = c->view->boxw;
        boxh    = c->view->boxh;
        margin  = c->view->outside_margin;
        dayname_height  = ((Month *) c->view->month_info)->dayname_height;
        date    = c->view->date;
        x       = event->xbutton.x;
        y       = event->xbutton.y;
        toffset = c->view->topoffset;

        if (boxw == 0)
                col = 0;
        else
                col     = (x-margin)/boxw;
        if  (boxh == 0)
                row = 0;
        else
                row     = (y-c->view->topoffset-dayname_height)/boxh;

        /* boundary conditions */
        if (x < margin || col > 6 || y < (dayname_height+toffset) || row > c->view->nwks-1) {
                calendar_deselect(c);
                lastcol=0; lastrow=0;
                return;
        }

	/* boundary check for click in invalid cell */
	if (((col < fdom(date)) && (row==0)) ||
	    ((col > ldom(date)) && (row==c->view->nwks - 1))) {
		calendar_deselect(c);
		lastcol=0; lastrow=0;
		return;
	}

	switch(event->type) {
	case MotionNotify:
               if (col !=lastcol || row !=lastrow) {
                        calendar_deselect(c);
                        j = xytoclock(col+1, row+1, date);
                        if (j > 0) {
                                c->view->olddate = c->view->date;
                                c->view->date = j;
                                calendar_select(c, daySelect, (caddr_t)NULL);
                        }
                        lastcol=col;
                        lastrow=row;
                }
                break;
      case ButtonPress:
		if (ds_is_double_click(&lastevent, event)) {
			_DtTurnOnHourGlass(c->frame);
			if (lastcol == col && lastrow == row)
				show_editor(c, 0, 0, False);
			_DtTurnOffHourGlass(c->frame);
		}
		else {
			calendar_deselect(c);
			j = xytoclock(col+1, row+1, date);
			if (j > 0) {
			   c->view->olddate = c->view->date;
			   c->view->date = j;
			   calendar_select(c, daySelect, (caddr_t)NULL);
			}

			if (editor_showing(e)) {
				int	*pos_list,
					 pos_cnt;

				if (XmListGetSelectedPos(e->appt_list,
							 &pos_list, &pos_cnt)) {					XtFree((void *)pos_list);
					set_editor_defaults(e, 0, 0, False);
				} else {
					Props *p = (Props *)e->cal->properties;

					set_date_in_widget(e->cal->view->date, 
						   	   e->dssw.date_text,
						   	   get_int_prop(p, 
							     CP_DATEORDERING),
						   	   get_int_prop(p, 
							     CP_DATESEPARATOR));
				}
				add_all_appt(e, NULL);
			}
		}
		if (todo_showing(t)) {
			int	*pos_list,
				 pos_cnt;

			if (XmListGetSelectedPos(t->todo_list,
						 &pos_list, &pos_cnt)) {
				XtFree((void *)pos_list);
				set_todo_defaults(t);
			} else {
				Props *p = (Props *)t->cal->properties;

				set_date_in_widget(t->cal->view->date, 
					   	   t->dssw.date_text,
					   	   get_int_prop(p, 
							CP_DATEORDERING),
					   	   get_int_prop(p, 
							CP_DATESEPARATOR));
			}
			add_all_todo(t, NULL);
		}
		if (geditor_showing(ge)) {
			Props           *p = (Props *)ge->cal->properties;

			set_date_in_widget(ge->cal->view->date, 
					   ge->dssw.date_text,
					   get_int_prop(p, CP_DATEORDERING),
					   get_int_prop(p, CP_DATESEPARATOR));
			add_all_gappt(ge);
		}
		lastcol=col;
		lastrow=row;
                break;
        default:
        break;
        };              /* switch */
        lastevent = *event;
}
 
static int
count_month_pages(Calendar *c, Tick start_date, int lines_per_box)
{
        int 			i, j;
        int 			rows, pages;
        struct 	tm 		tm;
        int 			day, ndays, num_appts, max = 0;
	CSA_uint32 		a_total;
	time_t 			start, stop;
        CSA_entry_handle 	*list;
	CSA_attribute 		*range_attrs;
	CSA_enum 		*ops;

        tm    = *localtime(&start_date);
        tm.tm_mday = 1;
#ifdef SVR4
        tm.tm_isdst = -1;
        day   = (int)mktime(&tm);
#else
        day   = (int)timelocal(&tm);
#endif /* SVR4 */
        ndays = ((tm.tm_mon==1) && leapyr(tm.tm_year+1900))? 29 :
                        monthdays[tm.tm_mon];
 
        /* print days of the week at the top */
        rows = numwks (start_date);
 
        /* need minimum 5 rows to paint miniature months */
        if (rows == 4) rows++;
 
        /* print the times and text of appts */
        for (i = 1; i <= ndays; i++)
        {
                /* setup a time limit for appts searched */
                start = (time_t) lowerbound (day);
                stop = (time_t) next_ndays(day, 1) - 1;
		setup_range(&range_attrs, &ops, &j, start, stop,
			    CSA_TYPE_EVENT, NULL, _B_FALSE, c->general->version);
		csa_list_entries(c->cal_handle, j, range_attrs, ops, &a_total, &list, NULL);
		free_range(&range_attrs, &ops, j);

                num_appts = count_month_appts(list, a_total, c);
                if (num_appts > max)
                        max = num_appts;
 
                day = nextday(day);
                csa_free(list);
        }
 
 
        pages = max / lines_per_box;
        if ((max % lines_per_box) > 0)
                pages++;

        return(pages);   
}

 
static Boolean
print_month ( Calendar *c,
    int num_page,
    FILE *fp,
    Tick first_date,
    Props *p,
    Boolean first)
{
        int 		rows, total_pages, i, j, lines_per_box;
        time_t 		lo_hour, hi_hour;
        char 		buf[50];
        struct tm 	tm;
        int 		ndays, dom, num_appts;
        Boolean 	more, done = False, all_done = True;
        static Tick 	tick = 0;
        Tick 		day;
	OrderingType 	ot = get_int_prop(p, CP_DATEORDERING);
        CSA_entry_handle *list;
        CSA_attribute 	*range_attrs;
	CSA_enum 	*ops;
        CSA_uint32 	a_total;

        if (first)
                tick = first_date;
        if (num_page > 1)
                tick = prevmonth_exactday(tick);
 
        /* print days of the week at the top */
        rows = numwks (tick);
 
        /* need minimum 5 rows to paint miniature months */
        if (rows == 4) rows++;
 
        if (rows == 5)
                lines_per_box = 7;
        else
                lines_per_box = 5;
 
        if (num_page == 1)
                total_pages = count_month_pages(c, tick, lines_per_box);
 
        tm    = *localtime(&tick);
        tm.tm_mday = 1;
#ifdef SVR4
        tm.tm_isdst = -1;
        day   = (int)mktime(&tm);
#else
        day   = (int)timelocal(&tm);
#endif /* SVR4 */
        ndays = ((tm.tm_mon==1) && leapyr(tm.tm_year+1900))? 29 :
                        monthdays[tm.tm_mon];
        tm   = *localtime(&day);
        dom  = tm.tm_wday;
        ps_init_printer(fp, LANDSCAPE);
        ps_init_month(fp);
 
        /* print month & year on top */
        format_date(tick, ot, buf, 0, 0, 0);
        ps_print_header(fp, buf);
 
        /* print days of week in boxes at top, and monthly grid */
        ps_month_daynames(fp, rows, num_page, total_pages);
 
        /* print the times and text of appts */
        for (i = 1; i <= ndays; i++)
        {
 
                /* setup a time limit for appts searched */
                lo_hour = (time_t)lowerbound (day);
                hi_hour = (time_t) next_ndays(day, 1) - 1;
		setup_range(&range_attrs, &ops, &j, lo_hour, hi_hour,
			    CSA_TYPE_EVENT, NULL, _B_FALSE, c->general->version);
		csa_list_entries(c->cal_handle, j, range_attrs, ops, &a_total, &list, NULL);
		free_range(&range_attrs, &ops, j);
                num_appts = count_month_appts(list, a_total, c);
                if (num_appts > (lines_per_box * num_page))
                        more = True;
                else
                        more = False;
 
                /* clear box & print date */
                ps_month_timeslots(fp, i, dom++, more);
                         
                /* print out times and appts */
                done = ps_print_month_appts (fp, list, a_total, num_page, hi_hour, lines_per_box, monthGlance);
                if (!done)
                        all_done = False;
 
                if (dom >= 7) dom = 0;
                day = nextday(day);
                csa_free(list);
        }
       /* paint miniature previous & next month */
        ps_print_little_months(fp, tick);
 
        ps_finish_printer(fp);
        tick = nextmonth(tick);
         
        return(all_done);
}

extern void
print_month_pages(Calendar *c)
{

        Props 		*p = (Props *)c->properties;
        register Tick 	first_date = c->view->date;
        int 		n = get_int_prop(p, CP_PRINTMONTHS);
        Boolean 	done = False, first = True;
        int 		num_page = 1;
        FILE 		*fp;

        if (n <= 0)
                n = 1;

        if ((fp=ps_open_file(c, print_from_default)) == NULL)
                return;
        for (; n > 0; n--) {

                while (!done) {
                        done = print_month(c, num_page, fp, first_date, p, first);
                        num_page++;
                        first = False;
                }

                done = False;
                num_page = 1;
        }
        if (fp)
                fclose(fp);
        ps_print_file(c, print_from_default);
}

extern void
print_month_range(Calendar *c, Tick start_tick, Tick end_tick)
{

        Props 		*p = (Props *)c->properties;
        register Tick 	end, first_date = start_tick;
        int 		n = get_int_prop(p, CP_PRINTMONTHS);
        Boolean 	done = False, first = True;
        int 		num_page = 1;
        FILE 		*fp;

	/* counting up the month pages is a little trickier than in the day
	   and week cases.  Months will have variable numbers of seconds in 
	   them, simple dividing the difference by the number of seconds
	   per month doesn't work.  We'll brute force this by grinding thru,
	   adding seconds until we pass the end tick time. */

	n = 0;
	end = first_dom(start_tick);
	do {
        	end = nextmonth(end);
		n++;
	} while (end < end_tick);

        if (n <= 0)
		return;

        if ((fp=ps_open_file(c, print_from_dialog)) == NULL)
                return;
        for (; n > 0; n--) {

                while (!done) {
                        done = print_month(c, num_page, fp, first_date, p, first);
                        num_page++;
                        first = False;
                }

                done = False;
                num_page = 1;
        }
        if (fp)
                fclose(fp);
        ps_print_file(c, print_from_dialog);
}
 
count_month_appts(CSA_entry_handle *list, int a_total, Calendar *c)
{
        int 			count = 0, i, meoval;
        Props 			*pr = (Props*)c->properties;
	Dtcm_appointment 	*appt;
	CSA_return_code 	stat;
 
        meoval = get_int_prop(pr, CP_PRINTPRIVACY);
 
	appt = allocate_appt_struct(appt_read, 
				    	c->general->version,
					CSA_ENTRY_ATTR_CLASSIFICATION_I, 
					NULL);
        for (i = 0; i < a_total; i++) {

		stat = query_appt_struct(c->cal_handle, list[i], appt);
		if (stat != CSA_SUCCESS) {
			free_appt_struct(&appt);
			return 0;
                }

		if ((privacy_set(appt) == CSA_CLASS_PUBLIC) && !(meoval & PRINT_PUBLIC))
			continue;
		else if ((privacy_set(appt) == CSA_CLASS_CONFIDENTIAL) && !(meoval & PRINT_SEMIPRIVATE))
			continue;
		else if ((privacy_set(appt) == CSA_CLASS_PRIVATE) && !(meoval & PRINT_PRIVATE))
			continue;


                count++;
        }
	free_appt_struct(&appt);
        return(count);
}        

/*
 * Handler for "hot" buttons to navigate to day view
 * Client data is index of activated button [0..30]
 */
static void
quick_button_cb(Widget widget, XtPointer client, XtPointer call)
{
        Calendar *c = calendar;
	int	 dom = (int) client;   

	c->view->olddate = c->view->date;
	c->view->date = first_dom(c->view->date) + (dom * daysec);

	calendar_select(c, daySelect, NULL);
 
	cleanup_after_monthview(c);
	
	c->view->glance = dayGlance;
        init_mo(c);
        (void)init_dayview(c);

        paint_day(c);
}

static void
manage_children(Calendar *c)
{
	int 	i;
	Month 	*m = (Month *)c->view->month_info;
 
        /* manage the header widget */
        XtManageChild(m->month_label);
	XtManageChildren(m->hot_button, m->ndays);
}

static void
unmanage_children(Calendar *c)
{
	int 	i;
	Month 	*m = (Month *)c->view->month_info;
 
        /* unmanage the header widget */
        XtUnmanageChild(m->month_label);
	XtUnmanageChildren(m->hot_button, 31);
}

/*
 * Clean up anything left around that won't be removed on
 * switching to another view, such as the hot buttons for
 * navigation to day view.
 */
extern void
cleanup_after_monthview(Calendar *c)
{

	invalidate_cache(c);

	XtUnmapWidget(c->canvas);
	unmanage_children(c);

	XmToggleButtonGadgetSetState(c->month_scope, False, False);

	/*
	 * Deallocating everything causes all this view's memory to be
	 * freed (at least all we have allocated in this application)
	 * at the expense of rebuilding the view next time it is used.
	 * Commenting it out improves usability and avoids any risk
	 * of memory leakage from Xt and Motif.
	 */
/*
	deallocator(c);
*/

	XtMapWidget(c->canvas);
}

/*
 * check whether month view is allocated yet
 */
static Boolean
allocated(Calendar *c)
{
	if (c->view->month_info != NULL)
		return True;
	else
		return False;
}

/*
 * allocate storage & subwidgets used by month view
 */ static void 
allocator(Calendar *c)
{
	int 	n;
	char 	buf[BUFSIZ];
        Month 	*m;
	XmString str;

	/* main storage for other month data */
	calendar->view->month_info = (caddr_t) XtCalloc(1, sizeof(Month));
        m = (Month *)c->view->month_info;

	m->button_loc = (XPoint *) XtCalloc(31, sizeof(XPoint));

	/* label for month & navigation button to year */
	m->month_label = XmCreateLabel(c->canvas, "monthLabel", NULL, 0);
	
	/* navigation buttons to day view */
	m->hot_button = (Widget *) XtCalloc(31, sizeof(Widget));
	for (n=0; n<31; n++) {
		sprintf(buf, "%d", n+1);
		str = XmStringCreateLocalized(buf);
		sprintf(buf, "month2day%d", n);
		m->hot_button[n] =
			XmCreatePushButton(c->canvas, buf, NULL, 0);
		XtVaSetValues(m->hot_button[n], XmNlabelString, str, NULL);
		XtAddCallback(m->hot_button[n],XmNactivateCallback, 
			quick_button_cb, (XtPointer)n);
		XmStringFree(str);
	}
}
