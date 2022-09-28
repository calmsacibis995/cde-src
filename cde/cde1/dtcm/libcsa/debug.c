/* $XConsortium: debug.c /main/cde1_maint/1 1995/07/17 19:56:13 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)debug.c	1.8 97/01/31 Sun Microsystems, Inc."

#include <stdio.h>
#include <time.h> 
#include <syslog.h>
#include <pthread.h>

/*
 * this routine is for debugging purpose only so that
 * we can examine the date in a more human readable format
 * in the debugger
 */
extern void
_DtCm_print_tick(time_t t)
{
#define BUFLEN	26
	char	buf[BUFLEN];
 
        ctime_r(&t, buf);
        (void) fprintf (stderr, "%d %s\n", t, buf);
}

extern void
_DtCm_print_errmsg(const char *msg)
{
	/* inited_lock protect access to inited */
	static pthread_mutex_t inited_lock = PTHREAD_MUTEX_INITIALIZER;
	static boolean_t inited = _B_FALSE;

	pthread_mutex_lock(&inited_lock);

	if (inited == _B_FALSE) {
		openlog("libcsa", 0, 0);
		inited = _B_TRUE;
	}

	pthread_mutex_unlock(&inited_lock);

#ifdef CM_DEBUG
	fprintf(stderr, "libcsa: %s\n", msg);
#else
	syslog(LOG_ERR, "%s\n", msg);
#endif

}

