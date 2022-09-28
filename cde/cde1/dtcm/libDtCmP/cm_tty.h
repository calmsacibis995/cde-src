/*******************************************************************************
**
**  cm_tty.h
**
**  #pragma ident "@(#)cm_tty.h	1.40 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: cm_tty.h /main/cde1_maint/3 1995/10/03 12:28:28 barstow $
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

#ifndef CM_TTY_H
#define CM_TTY_H

#include <csa.h>
#include <nl_types.h>
#include "ansi_c.h"
#include "timeops.h"
#include "util.h"
#include "props.h"

#define DTCM_CAT        "dtcm"

/*
 * Enumerated type for use with appointment file parsing routines.  For text,
 * case is ignored!
 */
typedef enum {
	NOT_A_KEY,
	APPOINTMENT_START,	/* "** Calendar Appointment **" */
	DATE_KEY,		/* "Date:" */
	START_KEY,		/* "Time:", "Start:", "From:" */
	STOP_KEY,		/* "Until:", "Stop:", "To:" */
	DURATION_KEY,		/* "Duration:" */
	WHAT_KEY,		/* "What:" */
	REPEAT_KEY,		/* "Repeat:" */
	FOR_KEY,		/* "For:" */
	NEW_APPT_KEY		/* "cm_appt_def:string:begin" */
} Parse_key_op;

/*
 * Types used in the validation process for each appointment
 */
typedef enum {
	COULD_NOT_OPEN_FILE,
	CANCEL_APPT,
	INVALID_DATE,
	INVALID_START,
	INVALID_STOP,
	MISSING_DATE,
	MISSING_START,
	MISSING_WHAT,
	INVALID_NOTIME_APPT,
	REPEAT_FOR_MISMATCH,
	INVALID_TIME_DUE,
	INVALID_TIME,
	MISSING_TIME,
	VALID_APPT
} Validate_op;

/*
 * Enumerated types used to access static strings for appointment information.
 */
typedef enum {
	ONE_TIME,
	DAILY,
	WEEKLY,
	EVERY_TWO_WEEKS,
	MONTHLY_BY_DATE,
	MONTHLY_BY_WEEKDAY,
	YEARLY,
	MONDAY_THRU_FRIDAY,
	MON_WED_FRI,
	TUESDAY_THURSDAY,
	REPEAT_EVERY
} Repeat_menu_op;

typedef enum {
	TWO,
	THREE,
	FOUR,
	FIVE,
	SIX,
	SEVEN,
	EIGHT,
	NINE,
	TEN,
	ELEVEN,
	TWELVE,
	THIRTEEN,
	FOURTEEN,
	FOR_EVER
} For_menu_op;

typedef enum {
	TIME_MINS,
	TIME_HRS,
	TIME_DAYS
} Time_scope_menu_op;

typedef enum {
	REPEAT_DAYS,
	REPEAT_WEEKS,
	REPEAT_MONTHS
} Repeat_scope_menu_op;

typedef enum {
	SUNDAY,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY
} Days_op;

typedef enum {
	JANUARY,
	FEBRUARY,
	MARCH,
	APRIL,
	MAY,
	JUNE,
	JULY,
	AUGUST,
	SEPTEMBER,
	OCTOBER,
	NOVEMBER,
	DECEMBER
} Months_op;

#define NO_TIME  (-1)
#define ALL_DAY  (-2)

/*
 * External function definitions
 */
extern char		*boolean_str	(boolean_t);
extern int		cm_tty_delete	(nl_catd, CSA_session_handle, int, int, CSA_entry_handle*);
extern void		cm_tty_format_header(Props*, Tick, char*);
extern int		cm_tty_insert	(nl_catd, CSA_session_handle, int, char*, char*,
						   char*, char*, char*, char*,
						   char*, Props*, char*);
extern void		cm_tty_load_props(Props**);
extern int		cm_tty_lookup	(nl_catd, CSA_session_handle, int, char*, char*,
						   CSA_entry_handle**, Props*);
extern boolean_t	convert_boolean_str(char*);
extern CSA_sint32	convert_privacy_str(char*);
extern int		convert_privacy_str_to_op(char*);
extern SeparatorType	convert_separator_str(char*);
extern Time_scope_menu_op convert_time_scope_str(char*);
extern char		*day_str	(Days_op);
extern char		*default_repeat_cnt_str(Repeat_menu_op);
extern char		*default_repeat_scope_str(nl_catd, Repeat_menu_op);
extern char		*for_str	(For_menu_op);
extern char		*get_datemsg	(OrderingType,
						   SeparatorType);
extern Parse_key_op	identify_parse_key(char*);
extern void		load_appt_defaults(Dtcm_appointment*, Props*);
extern void		load_reminder_props(Dtcm_appointment*, Props*);
extern char		*month_str	(Months_op);
extern Validate_op	parse_appt_from_file(nl_catd, char*, CmDataList*,
						   Props*, boolean_t(*)(void*), 
						   void*, int);
extern char		*parse_attrs_to_string(Dtcm_appointment*, Props*,
						   char*);
extern char		*attrs_to_string(CSA_attribute *, int);
extern char		*create_rfc_message(char *, char *, char**, int);
extern char		*parse_appt_to_string(CSA_session_handle, CSA_entry_handle, Props*, int);
extern char		*periodstr_from_period(CSA_sint32, int);
extern char		*privacy_str	(int);
extern char		*privacy_str_old(int);
extern char		*repeat_str	(nl_catd, Repeat_menu_op);
extern char		*repeat_scope_str(nl_catd, Repeat_scope_menu_op);
extern char		*privacy_str_411(int);
extern char		*separator_str	(SeparatorType);
extern void		str_to_period	(char*, CSA_sint32*, int*);
extern int		timescopestring_to_tick(char*);
extern char		*time_scope_str	(Time_scope_menu_op);
extern char		*time_scope_str_i18n(nl_catd, Time_scope_menu_op);
extern boolean_t	valid_time	(Props*, char*);
extern Validate_op	validate_appt	(nl_catd, Dtcm_appointment*, 
						   char*, char*, char*, int, 
						   char*, char*, char*,
						   boolean_t(*)(void*), void*,
						   int);
extern Validate_op	validate_dssw	(Dtcm_appointment*, char*,
						   char*, char*, int, char*,
						   boolean_t(*)(void*), void*);
extern Validate_op	validate_rfp	(nl_catd, Dtcm_appointment*, 
						   char*, char*, int);
extern Validate_op	validate_reminders(Dtcm_appointment*);

#endif
