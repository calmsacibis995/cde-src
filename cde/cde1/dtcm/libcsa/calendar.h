/* $XConsortium: calendar.h /main/cde1_maint/3 1995/10/10 13:27:56 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CALENDAR_H
#define _CALENDAR_H

#pragma ident "@(#)calendar.h	1.19 97/01/31 Sun Microsystems, Inc."

/*
 * This file contains the internal data structure of calendars.
 */

#include "ansi_c.h"
#include "csa.h"
#include "nametbl.h"
#include "attr.h"
#include "cm.h"

#include <pthread.h>

typedef struct {
	pthread_mutex_t	lock;
	pthread_cond_t	my_turn;
	pthread_t	owner;
	int		owner_count;
	int		wait_count;		
} _SessionLock;

typedef struct cbq {
	CSA_flags	reason;
	CSA_callback	handler;
	CSA_buffer	client_data;
	struct cbq	*next;
	struct cbq	*prev;
} _DtCmCallbackEntry;

/*
 * Calendar object structure
 */
typedef struct cal {
	CSA_session_handle	handle;
	int			rpc_version;	/* server version */
	int			file_version;
	_DtCmNameTable		*cal_tbl;
	_DtCmNameTable		*entry_tbl;
	char			*name;		/* calendar@location */
	char			*location;
	boolean_t		async_process;	/* asynchronous update enabled*/
	CSA_flags		all_reasons;	/* all events registered */
	CSA_flags		do_reasons;	/* events to process */
	_DtCmCallbackEntry	*cb_list;
	CSA_uint32		num_attrs;
	cms_attribute		*attrs;
	boolean_t		got_attrs;
	CSA_flags		access;
	boolean_t		delete;
	_SessionLock		slock;
	pthread_mutex_t		lock;		/* synchronize access */
	struct cal		*next;
} Calendar;

/* linked list of active calendars */
extern Calendar _DtCm_active_cal_list;

/* function prototypes */

extern Calendar * _DtCm_new_Calendar(const char *calenadr);

extern void _DtCm_free_Calendar(Calendar *cal);

extern void _DtCm_put_calendar_in_list(Calendar *cal);

extern void _DtCm_remove_calendar_from_list(Calendar *cal);

extern CSA_return_code _DtCm_get_calendar(CSA_session_handle calhandle,
					  boolean_t not_again,
					  Calendar **calptr);

/* release session lock */
extern void _DtCm_release_calendar(Calendar *cal);

extern CSA_return_code _DtCm_list_old_cal_attr_names(Calendar *cal,
						     CSA_uint32 *num_names_r,
						     char **names_r[]);

extern CSA_return_code _DtCm_set_cal_attr(Calendar *cal,
					  CSA_attribute attr);

extern CSA_return_code _DtCm_get_all_cal_attrs(Calendar *cal,
					       CSA_uint32 *num_attrs,
					       CSA_attribute **attrs);

extern CSA_return_code _DtCm_get_cal_attrs_by_name(Calendar *cal,
						   CSA_uint32 num_names,
						   CSA_attribute_reference *names,
						   CSA_uint32 *num_attrs,
						   CSA_attribute **attrs);

extern void _DtCm_reset_cal_attrs(Calendar *cal);

#endif
