/*SNOTICE*****************************************************************
**************************************************************************
*
*                   Common Desktop Environment
* 
* (c) Copyright 1993, 1994 Hewlett-Packard Company 
* (c) Copyright 1993, 1994 International Business Machines Corp.             
* (c) Copyright 1993, 1994 Sun Microsystems, Inc.
* (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
*                                                                    
* 
*                     RESTRICTED RIGHTS LEGEND                              
* 
* Use, duplication, or disclosure by the U.S. Government is subject to
* restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
* Technical Data and Computer Software clause in DFARS 252.227-7013.  Rights
* for non-DOD U.S. Government Departments and Agencies are as set forth in
* FAR 52.227-19(c)(1,2).
* 
* Hewlett-Packard Company, 3000 Hanover Street, Palo Alto, CA 94304 U.S.A.
* International Business Machines Corp., Route 100, Somers, NY 10589 U.S.A. 
* Sun Microsystems, Inc., 2550 Garcia Avenue, Mountain View, CA 94043 U.S.A.
* Unix System Labs, Inc., 190 River Road, Summit, NJ 07901 U.S.A.
*
*****************************************************************ENOTICE*/

#ifndef _MTCOMMON_H
#define _MTCOMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/utsname.h>

#include "csa.h"
#include "cm.h"
#include "rtable4.h"

#ifndef _NO_PROTO
#define P(args)	args
#else
#define P(args)	()
#endif			

#define _DtCMS_DEFAULT_DIR      "/usr/spool/calendar"

#ifdef DEBUG
extern  int     debug;
#endif /* DEBUG */	

typedef struct {
	CSA_return_code		stat;
	CSA_string		server;
	CSA_string		cal_addr;
	CSA_session_handle	cal;
	CSA_uint32		num_names;
	CSA_attribute_reference	*attr_names;
	CSA_calendar_user	*cal_names;
	CSA_uint32		num_attrs;
	CSA_attribute		*attrs;
	CSA_enum		*ops;
	CSA_uint32		num_entries;
	CSA_entry_handle	*entries;
	CSA_entry_handle	entry;
	CSA_date_time_range	timerange;
	CSA_uint32		num_rems;
	CSA_reminder_reference	*rems;
	time_t			tick;
	CSA_flags		reason;
	CSA_callback		callback;
	CSA_buffer		client_data;

        /* Fields for Server API Test */
        char                    **cal_list;
        pid_t                   pid;
        int                     index;
        cms_reminder_ref        *refs;
        cms_entry               *cms_entry;
        cms_attribute           *cms_attrs;
        Table_Res_4             *res;
        Access_Entry_4          *access;
        void                    *rqstp;
        int                     key1;
        int                     key2;
        cms_get_entry_attr_res_item *cms_item;
        cms_attribute           *cms_new_attrs;
        Table_Res_List_4        *res_list;
} ThrData;

#ifdef PTHREAD

#include <pthread.h>

/* global data */
extern pthread_mutex_t	threads_lock;
extern pthread_cond_t	cond_done;

int mttest_mutex_lock P((pthread_mutex_t *mp));
int mttest_mutex_unlock P((pthread_mutex_t *mp));
int mttest_cond_wait P((pthread_cond_t *cvp, pthread_mutex_t *mp));
int mttest_cond_signal P((pthread_cond_t *cvp));
int mttest_cond_broadcast P((pthread_cond_t *cvp));
pthread_t mttest_thr_self P(());
int mttest_thr_create P((void * (*start_func)(void *), void *arg,
			pthread_t *new_id));

#else /* PTHREAD */

#include <thread.h>
#include <synch.h>

/* global data */
extern mutex_t	threads_lock;
extern cond_t	cond_done;

int mttest_mutex_lock P((mutex_t *mp));
int mttest_mutex_unlock P((mutex_t *mp));
int mttest_cond_wait P((cond_t *cvp, mutex_t *mp));
int mttest_cond_signal P((cond_t *cvp));
int mttest_cond_broadcast P((cond_t *cvp));
thread_t mttest_thr_self P(());
int mttest_thr_create P((void * (*start_func)(void *), void *arg,
			thread_t *new_id));

#endif /* PTHREAD */

void *	create_test_threads P((int num, void *(*func)(), ThrData data[]));

#endif /* _MTCOMMON_H */

