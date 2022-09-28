/*******************************************************************************
**
**  util.h
**
**  #pragma ident "@(#)util.h	1.43 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: util.h /main/cde1_maint/4 1995/10/10 13:27:03 pascale $
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

#ifndef _UTIL_H
#define _UTIL_H

#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <csa.h>
#include "ansi_c.h"
#include "timeops.h"
#include "props.h"


#define DATAVER1        1       /* RPC versions 1 & 2 */
#define DATAVER2        2       /* RPC version 3 */
#define DATAVER3        3       /* RPC version 4 and RPC version 5 with
                                   old file store */
#define DATAVER4        4       /* RPC version 5 with new data store */
#define DATAVER_ARCHIVE -1	/* archive version.  Nothing is read_only */


#define MAXSTRING	80
#define CERROR		1
#define PWERROR		2
#define FILEERROR	3
#define SPACEERROR	4
#define DOTS(a)		(a[0]=='.' && (a[1]==0 || (a[1]=='.' && a[2]==0)))

typedef struct lines {
	char		*s;
	struct lines	*next;
} Lines;

typedef enum {appt_read, appt_write} Allocation_reason;

/*
**  Structure for list of properties
*/
typedef struct Pentry {
        char            *property_name;
        char            *property_value;
        struct Pentry   *next;
} Pentry;

typedef enum {
        ORDER_MDY,
        ORDER_DMY,
        ORDER_YMD
} OrderingType;
 
typedef enum {
        SEPARATOR_BLANK,
        SEPARATOR_SLASH,
        SEPARATOR_DOT,
        SEPARATOR_DASH
} SeparatorType;

/*
 * This convenience structure is used by the utility functions to set pointers
 * to the actual data array so hard coding indexes into the array elsewhere
 * in the front end isn't necessary.
 */
typedef struct {
	/*
	 * Read-only attributes
	 */
	CSA_attribute	*identifier;
	CSA_attribute	*modified_time;
	CSA_attribute	*author;
	CSA_attribute	*number_recurrence;

	/*
	 * Read-write attributes
	 */
	CSA_attribute	*time;
	CSA_attribute	*type;
	CSA_attribute	*subtype;
	CSA_attribute	*private;
	CSA_attribute	*end_time;
	CSA_attribute	*show_time;
	CSA_attribute	*what;
	CSA_attribute	*state;
	CSA_attribute	*repeat_type;
	CSA_attribute	*repeat_times;
	CSA_attribute	*repeat_interval;
	CSA_attribute	*repeat_week_num;
	CSA_attribute	*recurrence_rule;
	CSA_attribute	*beep;
	CSA_attribute	*flash;
	CSA_attribute	*mail;
	CSA_attribute	*popup;
	CSA_attribute	*sequence_end_date;
	CSA_attribute	*charset;

	/*
	 * The actual name array and count
	 */

	CSA_attribute_reference *names;
	int		num_names;

	/*
	 * The actual data array and count
	 */
	CSA_attribute	*attrs;
	CSA_uint32	count;

	/* Whether the structure has been filled in with a query */

	int		filled;
	Allocation_reason reason;
	
	/* the version of the back end this appointment structure 
	   was intended for */

	int		version;
} Dtcm_appointment;

/*
 * This convenience structure is used by the entry_to_cal, attrs_to_cal, and
 * cal_to_attrs utility functions
 */
typedef struct {
	/*
	 * Read-only attributes
	 */
	CSA_attribute	*cal_name;
	CSA_attribute	*server_version;
	CSA_attribute	*num_entries;
	CSA_attribute	*cal_size;

	/*
	 * Read-write attributes
	 */
	CSA_attribute	*access_list;
	CSA_attribute	*time_zone;

	/*
	 * The actual name array and count
	 */

	CSA_attribute_reference *names;
	int		num_names;

	/*
	 * The actual data array and count
	 */
	CSA_attribute		*attrs;
	CSA_uint32		count;

	/* Whether the structure has been filled in with a query */

	int		filled;
	Allocation_reason reason;
	
	/* the version of the back end this calendar structure 
	   was intended for */

	int		version;
} Dtcm_calendar;

/*
 * Utility functions to to provide pointers into the arrays passed back and
 * forth to the backend.  These functions and structures make hard coding
 * indexes into the arrays unnecessary.
 */
#ifndef _AIX
extern Dtcm_appointment	*allocate_appt_struct(Allocation_reason , int, ...);
#endif /* _AIX */
extern CSA_return_code  query_appt_struct      (CSA_session_handle, 
						   CSA_entry_handle, 
						   Dtcm_appointment *);
#ifndef _AIX
extern Dtcm_calendar	*allocate_cal_struct(Allocation_reason, int, ...);
#endif /* _AIX */
extern CSA_return_code  query_cal_struct       (CSA_session_handle, 
						   Dtcm_calendar *);
extern void		free_appt_struct(Dtcm_appointment**);
extern void		free_cal_struct	(Dtcm_calendar**);

extern boolean_t	cal_ident_index_ro(int, int);
extern boolean_t	entry_ident_index_ro(int, int);
extern CSA_enum		cal_ident_index_tag(int);
extern CSA_enum		entry_ident_index_tag(int);
extern boolean_t	ident_name_ro(char*, int);
extern void		initialize_cal_attr(int, CSA_attribute*,
						   Allocation_reason, int);
extern void		initialize_entry_attr(int, CSA_attribute*,
						   Allocation_reason, int);
extern void		set_appt_links	(Dtcm_appointment*);
extern void		set_cal_links	(Dtcm_calendar*);
extern void		scrub_cal_attr_list(Dtcm_calendar*);
extern void		setup_range	(CSA_attribute**,
						   CSA_enum**, int*, time_t,
						   time_t, CSA_sint32, CSA_sint32,
						   boolean_t, int);
extern void		free_range	(CSA_attribute**,
						   CSA_enum**, int);

/*
 * Other utilty functions
 */
extern int		blank_buf	(char*);
extern int		embedded_blank	(char*);
extern char		*cm_def_printer	();
extern char		*cm_get_credentials();
extern char		*cm_get_local_domain();
extern char		*cm_get_local_host();
extern char		*cm_get_uname	();
extern char		*cm_pqtarget	(char*);
extern char		*cm_strcat	(char*, char*);
extern char		*cm_strcpy	(char*, char*);
extern char		*cm_strdup	(char*);
extern int		cm_strlen	(char*);
extern char		*cm_target2domain(char*);
extern char		*cm_target2host	(char*);
extern char		*cm_target2location(char*);
extern char		*cm_target2name	(char*);
extern char		*cr_to_str	(char*);
extern void		destroy_lines	(Lines*);
extern void		expand_esc_chars(char*);
extern char		*get_head	(char*, char);
extern char		*get_tail	(char*, char);
extern void		print_tick	(time_t);
extern boolean_t	same_path	(char*, char*);
extern boolean_t	same_user	(char*, char*);
extern char		*str_to_cr	(char*);
extern char		*strncat	();
extern char		*substr		(char*, int, int n);
extern void		syserr		(char*, int, int, int);
extern Lines		*text_to_lines	(char*, int);
extern int		get_data_version(CSA_session_handle);
extern int		get_server_version(CSA_session_handle);
extern CSA_sint32	privacy_set	(Dtcm_appointment *);
extern CSA_sint32	showtime_set	(Dtcm_appointment *);
extern int		max		(int, int);
extern int		min		(int, int);
extern int              parse_date             (OrderingType, SeparatorType, char*, char*, char*, char*);

extern int              datestr2mdy            (char*, OrderingType, SeparatorType, char*);
extern void             format_tick            (time_t, OrderingType, SeparatorType, char*);
extern void		format_time	(Tick, DisplayType, char*);
extern void		mark_set_entry_charset(char *, char *, int version,
						   Boolean *);

/*
 * In Motif you can't associate user data with items in a list.  To get around
 * this we have the following simple functions that maintain a list of
 * user data.  We use the intrinscs coding style to reinforce the
 * relationship these routines have to the XmList* functions.
 */
typedef struct _CmDataItem {
	struct _CmDataItem	*next;
	void			*data;
} CmDataItem;

typedef struct _CmDataList {
	CmDataItem	*head;
	CmDataItem	*tail;
	int		count;
} CmDataList;

extern CmDataList	*CmDataListCreate(void);
extern void		CmDataListDestroy(CmDataList *, int);
extern int		CmDataListAdd	(CmDataList *, void *, int);
extern void		*CmDataListGetData(CmDataList *, int);
extern void		*CmDataListDeletePos(CmDataList *, int, int);
extern void		CmDataListDeleteAll(CmDataList *, int);


#endif
