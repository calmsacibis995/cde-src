/*******************************************************************************
**
**  ps_graphics.h
**
**  #pragma ident "@(#)ps_graphics.h	1.21 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: ps_graphics.h /main/cde1_maint/3 1995/08/18 09:40:26 mgreess $
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

#include <csa.h>
#include "calendar.h"

#ifndef _PS_GRAPHICS_H
#define _PS_GRAPHICS_H

#define PRINTER_WIDTH	612	/*  8.5"  paper width */
#define PRINTER_HEIGHT	792	/* 11.0"  paper height */
#define PRINTER_HMARGIN	18	/*  0.25" (LaserWriter) non-printing edge */
#define PRINTER_VMARGIN	3	/*  0.04" (LaserWRiter) non-printing edge */
#define INCH		72	/* 72 pts = 1 inch */
#define PORTRAIT	TRUE
#define LANDSCAPE	FALSE
#define CHAR_SIZE	sizeof(char)

typedef enum {print_from_default, print_from_dialog} Print_default;

#define PUBLIC          0 
#define SEMIPRIVATE     1 
#define PRIVATE         2 

/* bit masks for checking privacy for printing */

#define PRINT_PUBLIC		0x1
#define PRINT_SEMIPRIVATE 	0x2
#define PRINT_PRIVATE		0x4

extern void	ps_day_header	(FILE*, int, int, int);
extern void	ps_day_timeslots(FILE*, int, Boolean);
extern void	ps_finish_printer(FILE*);
extern void	ps_finish_std_month();
extern void	ps_finish_std_year();
extern void	ps_init_day	 (FILE*);
extern void	ps_init_list	();
extern void	ps_init_month	 (FILE*);
extern void	ps_init_printer	(FILE*, short);
extern void	ps_init_std_month(FILE*, int, int);
extern void	ps_init_std_year();
extern void	ps_init_week	 (FILE*);
extern void	ps_landscape_mode (FILE*);
extern void	ps_month_daynames(FILE*, int, int, int);
extern void	ps_month_timeslots(FILE*, int, int, Boolean);
extern FILE*	ps_open_file	(Calendar*, Print_default);
extern void	ps_print_file	(Calendar*, Print_default);
extern void	ps_print_header	(FILE*, char*);
extern void	ps_print_list	(Calendar*, CSA_sint32, int);
extern void	ps_print_list_range(Calendar*, CSA_sint32, int, Tick, Tick);
extern void	ps_print_little_months();
extern Boolean	ps_print_month_appts(FILE*, CSA_entry_handle*, int, int, int, int, Glance);
extern Boolean	ps_print_multi_appts(FILE*, CSA_entry_handle*, int, int, int, Glance);
extern void	ps_print_text	(FILE*, char*, int, Glance);
extern void	ps_print_time	(FILE*, char*, Glance);
extern void	ps_print_todo	(FILE*, CSA_entry_handle*, int, 
						CSA_sint32, Glance, int); 
extern void	ps_print_todo_months(FILE*);
extern void	ps_rotate 	(FILE*, int);
#ifndef _AIX
   /*
    * The AIX compliler cannot complile the following statement
    */
extern void	ps_scale 	(FILE*, float, float);
#endif /* _AIX */
extern void	ps_set_font	(FILE*, char*, char*);
extern void	ps_set_fontsize	(FILE*, char*, int);
extern void	ps_std_month_dates(FILE*, int, int);
extern void	ps_std_month_name (FILE*, char*);
extern void	ps_std_month_weekdays(FILE*);
extern void	ps_std_year_name (FILE*, int);
extern void	ps_todo_outline	(FILE*, CSA_sint32);
extern void	ps_translate	(FILE*, int, int);
extern void	ps_week_appt_boxes (FILE*);
extern void	ps_week_daynames(FILE*, char*, Boolean);
extern void	ps_week_header	 (FILE*, char*, int, int);
extern void	ps_week_sched_boxes(FILE*);
extern void	ps_week_sched_draw();
extern void	ps_week_sched_init();

#endif
