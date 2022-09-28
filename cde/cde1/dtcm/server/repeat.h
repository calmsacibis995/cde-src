/* $XConsortium: repeat.h /main/cde1_maint/3 1995/10/10 13:35:31 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _REPEAT_H
#define _REPEAT_H

#pragma ident "@(#)repeat.h	1.11 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "rtable4.h"
#include "rerule.h"

typedef time_t	Tick;

extern int	monthdays[];

extern void init_time();

extern time_t _DtCms_closest_tick_v4(time_t target,
					time_t ftick,
					Period_4 period,
					int *ordinal);

extern time_t _DtCms_last_tick_v4(time_t ftick,
				     Period_4 period,
				     int ntimes);

extern time_t _DtCms_next_tick_v4(time_t tick, Period_4 period);

extern time_t _DtCms_prev_tick_v4(time_t tick, Period_4 period);

extern time_t _DtCms_first_tick_v4(time_t t, Period_4 period, int ordinal);

extern void _DtCms_adjust_appt_startdate(Appt_4 *appt);

extern time_t next_nmins(time_t t, int m);

extern time_t next_ndays(time_t t, int n);

extern int _DtCms_marked_4_cancellation(Appt_4 *a, int i);

extern int _DtCms_get_new_ntimes_v4(Period_4 period,
				       time_t tick,
				       int ninstance);

extern int _DtCms_get_ninstance_v4(Appt_4 *appt);

extern int _DtCms_in_repeater(Id_4 *key,
				 Appt_4 *p_appt,
				 boolean_t dont_care_cancel);

extern int      monthlength            (Tick);
extern int      leapyr                 (int);
extern int      fdom                   (Tick);
extern int      ldom                   (Tick);

/*
 * returns the tick of the beginning of the day
 */
extern time_t _DtCmsBeginOfDay(time_t t);

/*
 * return the tick since begin of day
 */
extern time_t _DtCmsTimeOfDay(time_t t);

/*
 * routines to deal with recurrence rule and exception dates
 */
extern boolean_t _DtCmsInExceptionList(cms_entry *eptr, time_t tick);

#endif
