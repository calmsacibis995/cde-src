/*******************************************************************************
**
**  calendar.h
**
**  #pragma ident "@(#)calendar.h	1.65 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: calendar.h /main/cde1_maint/4 1995/10/10 13:22:38 pascale $
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

#ifndef _CALENDAR_H
#define _CALENDAR_H

#include <X11/Intrinsic.h>
#include <csa.h>
#include <nl_types.h>
#include <Xm/Xm.h>
#include "ansi_c.h"
#include "util.h"
#include "timeops.h"
#include "props.h"
#include "print.h"

/* from gettext.h */

#define MSGFILE_ERROR     "SUNW_DESKSET_CM_ERR"
#define MSGFILE_LABEL     "SUNW_DESKSET_CM_LABEL"
#define MSGFILE_MESSAGE   "SUNW_DESKSET_CM_MSG"

/* from graphics.h */

#define DARKGREY        1
#define DIMGREY         2
#define GREY            3
#define LIGHTGREY       4
#define RED                     5
#define MIDGREY 150

#define FAKE_MONOCHROME	1

#define DTCM_CAT	"dtcm"

typedef enum {gr_solid, gr_short_dotted, gr_dotted,
              gr_dot_dashed, gr_short_dashed,
              gr_long_dashed, gr_odd_dashed} GR_Line_Style;

typedef enum {gr_mono, gr_color} GR_depth;

typedef enum {BOLD, MEDIUM} Font_Weight;
 
typedef enum {
	yearGlance, monthGlance, weekGlance, dayGlance
} Glance;

typedef enum {
	RENDER_REPAINT, RENDER_CLEAR_FIRST, RENDER_UNMAP
} Render_Type;

typedef struct {Tick	start_time;
		Tick	end_time;
		int	show_time;
		char	*summary;} Paint_cache;

typedef struct {
	Pixel           foreground;     /* X Default Foreground Color */
	Pixel           background;     /* X Default Background Color */
	XID             xid;            /* XID */
	Pixel		hilight_color;	/* Default hilight color */
	/*Drawable        drawable;*/       /* X Drawable */
	XtAppContext	app;
        Display         *display;       /* X Display */

        /*
         * gc is a general purpose GC which is configured on the fly for
         * infrequent operations.  For frequent operations we use one of
         * the pre-configured GCs which we never change.
         */
        GC              gc;             /* X Graphics Context */
        XGCValues       *gcvals;        /* X Graphics Context Values */
        GC              draw_gc;        /* GC for drawing  */
        GC              clear_gc;       /* GC for clearing */
        GC              invert_gc;      /* GC for inverting */

        int             screen_depth;   /* screen depth */
        XColor          colorcell_del[RED+1];/* rgb colors */
} new_XContext;

typedef enum {
	main_win, browser, tempbrowser
} WindowType;


typedef struct {
	int			nwks;
	int			boxh;
	int			boxw;
	int			winw;
	int			winh;
	int			outside_margin;
	int			inside_margin;
	int			topoffset;
	time_t			date;			
	time_t			olddate;
	Glance			glance;
	caddr_t			day_info;	/* in dayglance.c */
	caddr_t			week_info;	/* in weekglance.c */
	caddr_t			month_info;	/* in monthglance.c */
	caddr_t			year_info;	/* in yearglance.c */
	char*			current_calendar; /* address in calendarA.c */
	char*			current_calname;  /* in calendarA.c */
	CSA_reminder_reference	*next_alarm;	/* in appt.c */
	CSA_uint32		next_alarm_count;
	int			rings;		/* number of rings to do in reminder */
	int			flashes;	/* number of flashes to do in reminder */
	XtIntervalId		next_alarm_id;
	caddr_t			current_selection; /* in select.c */
	char*			wm_cmdstr;	/* WM_COMMAND format 
						   version of argv list */
	int			wm_cmdstrlen;	/* Size of the wm_cmdstr 
						   buffer */
	char			*sessionfile;	/* file name (if any)
						   provided by -session 
						   parameter */
	XrmDatabase		sessiondb;	/* X database that is 
						   constructed by the 
						   session file */


} View;

typedef struct _Cal_Font {
	XmFontType	cf_type;
	union {
		XFontStruct	*cf_font;
		XFontSet	 cf_fontset;
	} f;
} Cal_Font;

typedef struct {
	Cal_Font  	*labelfont;
	Cal_Font  	*viewfont;
	Cal_Font  	*boldfont;
	Cal_Font  	*iconfont;
	Cal_Font  	*userfont;
} Cal_Fonts;

typedef struct {
	Widget	panel;
	Widget	button1;		/* unused	*/
	Widget	button2;		/* view button  */
	Widget	button3;		/* edit button  */
	Widget	button4;		/* browse button*/
	Widget	button5;		/* print button */
	Widget	button6;		/* unused	*/
	Widget	button7;		/* unused	*/
	Widget	button8;		/* prev button  */
	Widget	button9;		/* today button */
	Widget	button10;		/* next button  */
	Widget	menu_bar;
} Items;

typedef struct {
	int		rc_ts; 		/* timestamp of rc file: browse */
	int		version; 	/* data version of the browsing cal */
	WindowType last_canvas_touched; /* either browser or main */
} General; 

/*
 * Datatype for application-defined resources.  This is filled in by Xt during
 * XtVaAppInitialize (for the cmdline args) and during XtGetApplicationResources
 * for the app-defaults values.  An instance of this is carried around in
 * calendar->app_data to provide access to properties data.
 *
 * Historical note: old properties (.desksetdefaults) should be mapped into
 * this, and (if they need to be written out) taken form this.
 */
typedef struct {
        String  	default_view;
        String  	default_calendar;
        String  	default_printer;
	String  	labelfont;
	XmFontList  	labelfontlist;
	XmFontList  	viewfontlist;
	XmFontList  	boldfontlist;
	XmFontList  	iconfontlist;
	String  	userfont;
	XmFontList  	userfontlist;
	String  	systemfont;
	XmFontList  	systemfontlist;
	String  	app_font_family;
	String  	session_file;
	Boolean		debug;
} DtCmAppResources, *DtCmAppResourcesPtr;

typedef struct {
	unsigned int	icon_width;
	unsigned int	icon_height;
	unsigned int	icon_depth;
	Pixmap		icon;
	Pixmap		icon_mask;
} IconInfo;

typedef struct {
	nl_catd		DT_catd;	/* catgets file descriptor */
	char		*app_locale;	/* the CDE platform indpendent	*/
					/* locale name of the locale	*/
					/* the application is running in */
	CSA_session_handle cal_handle;	/* handle to the browsing calendar */
	CSA_flags	user_access;	/* access wrt browsing cal */
	Boolean		set_charset;	/* whether to set the entry charset
					 * attribute when inserting into
					 * the browsing calendar
					 */
	CSA_session_handle my_cal_handle; /* my calendar's handle */
	CSA_flags	my_access;
	int		my_cal_version;	/* my calendar data version */
	Boolean		my_set_charset; /* whether to set the entry
					 * charset attribute when
					 * inserting into my calendar
					 */
	Widget		frame;
	Widget		panel;
	Items		*items;
	General		*general;	/* stuff general to the calendar */
	Widget		form;		/* main container */
	Widget		toolbar;	/* frame for toolbar */
	Widget		canvas;		/* main canvas for graphics*/
	Widget		menu_bar;	/* top of menu hierarchy */
	Widget		browse_menu;	/* cached for quick access */
	Widget		browse_button;	/* for updating browse menu */
	Widget		message_text;	/* status at foot of main window */
	Widget		previous;	/* arrow to step back in time */
	Widget		today;		/* button to jump to "now" */
	Widget		next;		/* arrow to step forward in time */
	Widget		radio;		/* container for toolbar scope btns */
	Widget		day_scope;	/* scope buttns in tool bar */
	Widget		week_scope;
	Widget		month_scope;
	Widget		year_scope;
	Widget		appt_btn;	/* Quick access to appt editor */
	Widget		todo_btn;	/* Quick access to todo editor */
	Widget		todo_edit_menu;	/* Menu access to todo editor */
	Widget		todo_view_menu;	/* Menu access to todo list */
	Widget		month1;		/* month panels in day view */
	Widget		month2;
	Widget		month3;
	Widget		postup;		/* in alarm.c */
	Widget		mainhelpdialog;
	Widget		about_dialog;
	Widget		credits_dialog;
	View		*view;		/* cached info about current display */
	DtCmAppResourcesPtr app_data;	/* Xt-populated, listed above */
	caddr_t		properties;	/* in props.c */
	caddr_t		properties_pu;	/* in props_pu.c */
	caddr_t		editor;		/* in editor.c */
	caddr_t		geditor;	/* in group_editor.c */
	caddr_t		browser;	/* in browser.c */
	caddr_t		browselist;	/* in blist.c */
	caddr_t		todo;		/* in todo.c */
	caddr_t		alist;		/* in alist.c */
	caddr_t		print_opts;	/* in printer.c */
	caddr_t		user;		/* calendar driver */
	char*		calname;	/* calendar name */
	char*		caladdr;	/* calendar address */
	new_XContext	*xcontext;	/* XID, Display, GC, GCVals */
	caddr_t 	find;		/* in find.c */
	caddr_t 	goTo;		/* in goto.c */
	caddr_t 	tempbr;		/* in tempbr.c */
	caddr_t         mail;		/* in mail.c */
	caddr_t		timezone;	/* in timezone.c */
	caddr_t		print_data;	/* in print.c - valid when dialog up */
	char		*tt_procid;	/* Tooltalk process id, used for 
					   tooltalk intiialization */
	Paint_cache	*paint_cache;
	int		paint_cache_size;
	Cal_Fonts	*fonts;
	IconInfo	*icon;
	IconInfo	*icon_inverted;
#ifndef NOTHREADS
	char		*last_cal_requested; /* Name of the last calendar
					      * requested to be viewed.  Used
					      * for threading.
					      */
#endif /* NOTHREADS */
} Calendar;

extern Calendar	*calendar;
extern int _csa_iso8601_to_tick(char *, time_t*);
extern int _csa_tick_to_iso8601(time_t, char *);

#define MAXBUFLEN               4096

#define CMS_VERS_2  2
#define CMS_VERS_3  3
#define CMS_VERS_4  4

/* define to figure out if two XRectangle structures intersect each other */

#define myrect_intersectsrect(r1,r2) \
        ((int)(r1)->x<=(int)(r2)->x+(int)(r2)->width && \
        (int)(r1)->y<=(int)(r2)->y+(int)(r2)->height &&  \
         (int)(r2)->x<=(int)(r1)->x+(int)(r1)->width && \
         (int)(r2)->y<=(int)(r1)->y+(int)(r1)->height)


extern void		cache_dims	(Calendar*, Dimension,
						   Dimension);
extern char*		cm_get_relname	();
extern Boolean		in_range	(time_t, time_t, time_t);
extern int		report_err	(char*);
extern void		reset_alarm	(Calendar*);
extern CSA_return_code	reset_timer	(Calendar*);
extern CSA_return_code	paint_canvas	(Calendar*, XRectangle*, 
						   Render_Type);
extern void		switch_it	(Calendar*, char* name,
						   char* addr, WindowType);
extern void		switch_it2	(Calendar*, char* name, 
						   char* addr, WindowType);
extern int		yyyerror	(char*);
extern void		gr_clear_area	(new_XContext*, int, int,
						   Dimension, Dimension);
extern void		gr_clear_box	(new_XContext*, int, int, int,
						   int);
extern void		gr_make_grayshade(new_XContext*, int, int, int,
						   int, int);
extern void		gr_make_gray	(new_XContext*, int, int, int,
						   int, int);
extern void		gr_make_rgbcolor(new_XContext*, Colormap, int,
						   int, int, int, int, int,
						   int);
extern int		gr_nchars	(int, char*, Cal_Font *);
extern void		gr_draw_rgb_box	(new_XContext*, int, int, int,
						   int, int, int, Colormap);
extern void		gr_dissolve_box	(new_XContext*, int, int, int,
						   int, int);
extern int		gr_center	(int, char*, Cal_Font *);
extern void		gr_text		(new_XContext*, int, int,
						   Cal_Font *, char*,
						   XRectangle*);
extern void		gr_text_rgb	(new_XContext*, int, int,
						   Cal_Font *, char*, Pixel,
						   Colormap, XRectangle*);
extern void		gr_draw_line	(new_XContext*, int, int, int,
						   int, GR_Line_Style,
						   XRectangle*);
extern void		gr_draw_box	(new_XContext*, int, int, int,
						   int, XRectangle*);
extern void		gr_draw_glyph	(new_XContext*, new_XContext*,
						   Pixmap, int, int, int, int);
extern void		gr_set_clip_rectangles(new_XContext*, int, int,
						   XRectangle*, int, int);
extern void		gr_clear_clip_rectangles(new_XContext*);
extern Boolean		gr_init		(new_XContext*, Widget);
extern new_XContext	*gr_create_xcontext(Calendar*, Widget, GR_depth,
						   XtAppContext);
extern Boolean		today_inrange	(Calendar*, time_t);
extern void		update_browse_menu_names(Calendar*);
extern void		destroy_paint_cache(Paint_cache *, int);
extern void		invalidate_cache(Calendar *);

extern CSA_return_code	allocate_paint_cache(CSA_entry_handle *, int, Paint_cache **);

extern Boolean 		fontlist_to_font(XmFontList, Cal_Font *);
extern void 		CalTextExtents	(Cal_Font *, char *, int, 
						   int *, int *, int *, int *);
extern void 		CalDrawString	(Display *, Drawable,
						   Cal_Font *, GC, int, int,
						   char *, int);
extern void 		CalFontExtents	(Cal_Font *, 
						   XFontSetExtents *);

#endif
