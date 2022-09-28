/*******************************************************************************
**
**  format.c
**
**  $XConsortium: format.c /main/cde1_maint/4 1995/10/10 13:24:29 pascale $
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

#pragma ident "@(#)format.c	1.32 96/11/12 SMI"

#include <stdio.h>
#include <sys/time.h>
#if defined(sun) && defined(_XOPEN_SOURCE)
#include <time.h>
#endif
#include <Xm/Xm.h>
#include <csa.h>
#include "calendar.h"
#include "util.h"
#include "cm_tty.h"
#include "timeops.h"
#include "format.h"
#include "gettext.h"
#include "datefield.h"
#include "props.h"

/*
 *  Convert tick to m, d, y string.  e.g. May 1, 1988
 */
extern void
format_date(Tick t, OrderingType order, char *buf, int day_and_date,
	int and_date, int full_day)
{
	int m, d, y, wd;
	struct tm *tm;

	buf[0]=NULL;
	tm = localtime(&t);
	m = tm->tm_mon+1;
	d = tm->tm_mday;
	y = tm->tm_year + 1900;
	wd = tm->tm_wday;

	switch(order) {
                case ORDER_DMY:     
			/* STRING_EXTRACTION SUNW_DESKSET_CM_MSG :
			 *
			 * %s, %d %s %4d is a form of the date format. 
			 * Change this to reflect your local convention.
			 * eg. In the German locale, "%s, %d %s %4d" may be changed
			 * to "%s. %d %s %4d".
			 */
			if (day_and_date)
				if (full_day)
					(void)sprintf(buf, "%s, %d %s %4d", 
						days2[wd], d, months[m], y);
				else
					(void)sprintf(buf, "%s, %d %s %4d", 
						days[wd], d, months[m], y);
			else if (and_date)
				(void)sprintf(buf, "%d %s %4d", d, months[m], y);

			else 
				(void) sprintf(buf, "%s %4d", months[m], y);
                        break;
                case ORDER_YMD:    
			if (day_and_date) 
				if (full_day)
					(void)sprintf(buf, "%s, %4d %s %d", 
						days2[wd], y, months[m], d);
				else
					(void)sprintf(buf, "%s, %4d %s %d", 
						days[wd], y, months[m], d);
			else if (and_date)
				(void)sprintf(buf, "%4d %s %d", y, months[m], d);
			else 
				(void) sprintf(buf, "%4d %s", y, months[m]);
                        break;
                case ORDER_MDY:   
                default:
			if (day_and_date) 
				if (full_day)
					(void)sprintf(buf, "%s, %s %d %4d", 
						days2[wd], months[m], d, y);
				else
					(void)sprintf(buf, "%s, %s %d %4d", 
						days[wd], months[m], d, y);
			else if (and_date)
					(void)sprintf(buf, "%s %d %4d", 
						months[m], d, y);
			else 
				(void) sprintf(buf, "%s %4d", months[m], y);
                        break;
	}

}

/*
 *  Convert tick to m/d
 */
extern void
format_date3(Tick t, OrderingType order, SeparatorType sep, char *buf) 
{
	int m, d;
	struct tm *tm;  
	char *str = (char *) separator_str(sep);

	buf[0]=NULL;
	tm = localtime(&t); 

	m = tm->tm_mon+1; 
	d = tm->tm_mday;
	switch(order) {
                case ORDER_YMD:    
                case ORDER_MDY:   
                default:
			(void) sprintf(buf, "%d%s%.2d", m, str, d);
                        break;
                case ORDER_DMY:     
			(void) sprintf(buf, "%.2d%s%d", d, str, m);
                        break;
	}
}

/*
 *  Format 1 line of appt data.  Returns True if begin hour is < 10 - thus
 *  padding needs to be done.
 */
extern Boolean
format_line(Tick tick,
    char *what, 
    char *buf, 
    int end_tick, 
    Boolean showtime,
    DisplayType display)
{
        int hr, hr1, mn, mn1;
        Boolean am=True;
        Boolean am_end=True;
	struct tm *tm;
	Boolean	pad = FALSE;
	Calendar *c = calendar;

        if (buf==NULL) return pad;
        buf[0]=NULL;
	tm = localtime(&tick);
        hr = tm->tm_hour;
        mn = tm->tm_min;
        if (showtime) {
		if (display == HOUR12)
			am = adjust_hour(&hr);
                if (end_tick && end_tick != tick) {
                        hr1 = hour(end_tick);
                        mn1 = minute(end_tick);
			if (display == HOUR12) {
				am_end = adjust_hour(&hr1);
				if (am_end != am) {
                        	   (void) sprintf(buf, 
					"%d:%.2d - %d:%.2d%s ", 
					hr, mn,
					hr1, mn1, 
					am_end ? 
					   catgets(c->DT_catd, 1, 4, "am") : 
					   catgets(c->DT_catd, 1, 3, "pm"));
				} else {
                        		(void) sprintf(buf, "%d:%.2d - %d:%.2d ", 
							hr, mn, hr1, mn1);
				}
				if (hr < 10) pad = TRUE;
			}
			else
                        	(void) sprintf(buf, "%02d%02d - %02d%02d ", 
						hr, mn, hr1, mn1);
                }
                else {
                	/* Check to see if there are 2 digits in
                   	in initial time format. If so, pad with
                   	1 space; if not 2.  The font is not fixed
                   	width, so I have to line it up myself.. */
			if (display == HOUR12) {
                        	if (hr > 9)
                               		(char *)sprintf(buf, "%2d:%.2d%s ", 
							hr, mn, am ? "a" : "p");
                        	else {
                               		(char *)sprintf(buf, "%d:%.2d%s ", 
							hr, mn, am ? "a" : "p");
					pad = TRUE;
				}
			}
			else
				 (char *)sprintf(buf, "%02d%02d ", hr, mn);
                }
        }
	if (what)
		(void) cm_strcat(buf, what);

	return pad;
}

/*
 *  Format 2 lines of appt data
 */
extern void
format_maxchars(Dtcm_appointment *appt, char *buf1, int maxchars,
		DisplayType display) {
        Tick    tick, end_tick = 0;
        int     hour1, min1, hour2, min2;
        Lines   *lines;
        char    *s1, *s2;
	struct tm *tm;

 
	_csa_iso8601_to_tick(appt->time->value->item.string_value, &tick);
	if (appt->end_time)
		_csa_iso8601_to_tick(appt->end_time->value->item.string_value,
			&end_tick);
        *buf1 = NULL;
        if (appt == NULL || appt->what->value->item.string_value == NULL) return;
        tm = localtime(&tick);
        hour1 = tm->tm_hour;
        min1  = tm->tm_min;
	if (showtime_set(appt)) {
        	s1 = s2 = "am";
		if (display == HOUR12 && !adjust_hour(&hour1))
			s1="pm";

		if (end_tick) {
			hour2 = hour(end_tick);
			if (display == HOUR12 && !adjust_hour(&hour2))
				s2="pm";
	 
			min2 = minute(end_tick);
		}

		if (end_tick == 0 || hour1 == hour2 && min1 == min2) {
			if (display == HOUR24) 
				sprintf(buf1, "%02d%02d  ", hour1, min1);
			else
				sprintf(buf1, "%d:%.2d%s  ", hour1, min1, s1);
		}
		else {
			if (display == HOUR12)
				sprintf(buf1, "%d:%.2d%s-%d:%.2d%s  ", 
					hour1, min1, s1, hour2, min2, s2);
			else
				sprintf(buf1, "%02d%02d-%02d%02d  ", 
					hour1, min1, hour2, min2);
		}
	}
 
        lines = (Lines *) text_to_lines(appt->what->value->item.string_value, 10);
 
	while (lines != NULL) {
		if ((cm_strlen(buf1) + cm_strlen(lines->s)) < (maxchars-3)) {
                	cm_strcat(buf1, lines->s);
			lines = lines->next;
			if (lines != NULL) 
                		cm_strcat(buf1, " - ");
		}
		else {
			strncat(buf1, lines->s, (maxchars - cm_strlen(buf1)-1));
			break;
		}
	}
        destroy_lines(lines);
}


/*
 *  Format 2 lines of appt data
 */
extern void
format_line2(Dtcm_appointment *appt, char *buf1, char *buf2,
	     DisplayType display) {
        Tick    tick, end_tick = 0;
        int     hour1, min1, hour2, min2;
        Lines   *lines;
        char    *s1, *s2;
        struct tm *tm;

	_csa_iso8601_to_tick(appt->time->value->item.string_value, &tick);	
	if (appt->end_time)
		_csa_iso8601_to_tick(appt->end_time->value->item.string_value,
			&end_tick);	
 
        /*
         * Extract an appointment and format it into 2 lines of no more
         * then maxchars
         */
        *buf1 = *buf2 = NULL;
        if (appt == NULL || appt->what->value->item.string_value == NULL) return;
        tm = localtime(&tick);
        hour1 = tm->tm_hour;
        min1  = tm->tm_min;
 
        if (!showtime_set(appt)) {
                lines = (Lines *) text_to_lines(appt->what->value->item.string_value, 1);           
                if (lines==NULL) return;
                strncpy(buf2, lines->s, 256);
                destroy_lines(lines);
                return;
        }
 
        s1 = s2 = "am";
        if (display == HOUR12 && !adjust_hour(&hour1))
                s1="pm";

	if (end_tick) {
        	hour2 = hour(end_tick);
        	min2 = minute(end_tick);
        	if (display == HOUR12 && !adjust_hour(&hour2))
                        s2="pm";
	}

        if (end_tick == 0 ||
	    (hour1 == hour2 && min1 == min2 && (strcmp(s1, s2) == 0))) {
                if (display == HOUR24)
                        sprintf(buf1, "%02d%.2d", hour1, min1);
                else
                        sprintf(buf1, "%d:%.2d%s", hour1, min1, s1);
        }
        else {
                if (display == HOUR12)
                        sprintf(buf1, "%d:%.2d%s-%d:%.2d%s", hour1, min1, s1,
                                 hour2, min2, s2);
                else
                        sprintf(buf1, "%02d%02d-%02d%02d", hour1, min1,
                                 hour2, min2);
        }

          
        lines = (Lines *) text_to_lines(appt->what->value->item.string_value, 1);
         
        if (lines == NULL || lines->s == NULL ||                        
                (cm_strlen(lines->s) == 1 && lines->s[0] == ' '))
                buf2[0] = NULL;
        else
                sprintf(buf2, " %s", lines->s);
        destroy_lines(lines);
}


/*
 *  Format the appointment in the character array passed (assumed to be pointing
 *  to allocated space) to contain the time followed by the what string.  The
 *  appointment string is truncated at "max" chars or at DEFAULT_APPT_LEN if
 *  max is 0.
 */
extern void
format_appt(Dtcm_appointment *appt, char *b, DisplayType display, int max) {
        int		hr, mn, len, i = 0, j = 0;
	long		tick;
	struct tm	*tm;
	register char		*what_ptr;
 
	if (!appt || !b)
		return;

	_csa_iso8601_to_tick(appt->time->value->item.string_value, &tick);
	tm = localtime((time_t *)&tick);
        hr = tm->tm_hour;
        mn = tm->tm_min;

        if (showtime_set(appt)) {
		if (display == HOUR12) {
			adjust_hour(&hr);
                	sprintf(b, "%2d:%02d ", hr, mn);
		} else
                	sprintf(b, "%02d%02d ", hr, mn);
		i = cm_strlen(b);
        }

	if (appt->what->value->item.string_value) {
		if (max <= 0)
			max = DEFAULT_APPT_LEN;
		len = max - i;
		what_ptr = appt->what->value->item.string_value;
		while ((i < len) && *what_ptr != '\n' && *what_ptr)
			b[i++] = *what_ptr++;
		b[i] = '\0';
	}
}

/*
 *  Format the appointment in the character array passed (assumed to be pointing
 *  to allocated space) to contain the formatted string for the group
 *  appointment editor.  The string is truncated at "max" chars or at
 *  DEFAULT_GAPPT_LEN if max is 0.
 */
extern void
format_gappt(Dtcm_appointment *appt, char *name, char *b, DisplayType display,
	    int max) {
        int		hr, mn, i, j;
	long		tick;
	char		*what_ptr;
	struct tm	*tm;
 
	if (!appt || !b)
		return;

	_csa_iso8601_to_tick(appt->time->value->item.string_value, &tick);
        if ((tick > 0) && showtime_set(appt)) {
		tm = localtime((time_t *)&tick);
		hr = tm->tm_hour;
		mn = tm->tm_min;

		if (display == HOUR12) {
			adjust_hour(&hr);
			sprintf(b, "%2d:%02d ", hr, mn);
		} else
			sprintf(b, "%02d%02d  ", hr, mn);
        } else
		sprintf(b, "%6s", " ");

	if (max <= 0)
		max = DEFAULT_GAPPT_LEN;
	i = cm_strlen(b);

	j = 0;
	while (j < 10 && i < max && name && name[j])
		b[i++] = name[j++];
	while (j < 11 && i < max)
		b[i++] = ' ', ++j;

	if (i >= max) {
		b[i - 1] = '\0';
		return;
	}
	b[i] = '\0';

	if (appt->what->value->item.string_value) {
		what_ptr = appt->what->value->item.string_value;
		while (i < max && *what_ptr != '\n' && *what_ptr)
			b[i++] = *what_ptr++;
		b[i] = '\0';
	}
}
