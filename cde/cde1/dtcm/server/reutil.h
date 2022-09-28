/* $XConsortium: reutil.h /main/cde1_maint/1 1995/07/17 20:17:38 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)reutil.h	1.9 96/09/23 Sun Microsystems, Inc."

#ifndef _REUTIL_H
#define _REUTIL_H

#include "rerule.h"
#include "repeat.h"

#define EOT	2147483647

#define RE_DAILY(re)    (re->re_data.re_daily)
#define RE_WEEKLY(re)   (re->re_data.re_weekly)
#define RE_MONTHLY(re)  (re->re_data.re_monthly)
#define RE_YEARLY(re)   (re->re_data.re_yearly)
#define RES_DSTATE(res) (res->res_data.ds)
#define RES_WSTATE(res) (res->res_data.ws)
#define RES_MSTATE(res) (res->res_data.ms)
#define RES_YSTATE(res) (res->res_data.ys)
#define SAME_DAY(tm1, tm2)  (((tm1)->tm_year == (tm2)->tm_year && \
                             (tm1)->tm_mon == (tm2)->tm_mon && \
                             (tm1)->tm_mday == (tm2)->tm_mday))
#define SAME_MONTH(tm1, tm2) (((tm1)->tm_year == (tm2)->tm_year && \
                              (tm1)->tm_mon == (tm2)->tm_mon))
#define TIME_OF_DAY(tm)   ((tm)->tm_hour * 60 * 60 + \
                         (tm)->tm_min * 60 + \
                         (tm)->tm_sec)
#define TIMEOFMONTH(tm) (((tm)->tm_mday - 1) * 24 * 60 * 60 + \
			 (tm)->tm_hour * 60 * 60 + \
                         (tm)->tm_min * 60 + \
                         (tm)->tm_sec)
#define HOURTOSEC(time) ((time/100) * 60 * 60 + \
                         (time%100) * 60)

extern RepeatEvent* parse_rule(char*);
extern int GetWDayDiff(const int, const int);
extern int DayExists(const int, const int, const int);
extern WeekNumber GetWeekNumber(const Tick);
extern WeekNumber DayToWeekNumber(const Tick);
extern Tick WeekNumberToDay(const Tick, const WeekNumber, const WeekDay);
extern int OccurenceExists(const WeekDayTime *, const unsigned int, const Tick);
extern unsigned int DayOfMonth(const int, const int, const int);
extern int InTimeRange(const unsigned int, const Duration);
extern int same_week(struct tm *, struct tm *);

extern Tick ClosestTick(const Tick, const Tick, RepeatEvent *,
                        RepeatEventState **);
extern Tick NextTick(const Tick, const Tick, RepeatEvent *,
                        RepeatEventState *);
extern Tick PrevTick(const Tick, const Tick, RepeatEvent *,
                        RepeatEventState *);
extern Tick LastTick(const Tick, RepeatEvent *);
extern int CountEvents(Tick, RepeatEvent *re, CSA_date_time_entry *);
extern Tick DeriveNewStartTime(const Tick, RepeatEvent *old_re, const Tick, 
			const Tick, RepeatEvent *new_re);


#endif /* _REUTIL_H */
