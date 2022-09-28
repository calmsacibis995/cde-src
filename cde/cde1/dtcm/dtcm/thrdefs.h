
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)thrdefs.h	1.2 96/11/12 Sun Microsystems, Inc."

/*
 * By default threads are enabled in dtcm.  We are using Solaris threads.
 * Posix threads can be enabled by defining PTHREAD.  The tool can also
 * be compiled without threads by defining NOTHREADS.
 */

#ifndef NOTHREADS
#ifdef PTHREAD
#include <pthread.h>
#else /* PTHREAD */
#include <thread.h>
#endif /* PTHREAD */

extern int cm_thread_create(void * (*)(void *), void *);
extern void cm_thread_exit(void *);

#ifdef PTHREAD
extern int cm_mutex_init(pthread_mutex_t *);
extern int cm_mutex_lock(pthread_mutex_t *);
extern int cm_mutex_unlock(pthread_mutex_t *);
extern int cm_mutex_destroy(pthread_mutex_t *);
#else
extern int cm_mutex_init(mutex_t *);
extern int cm_mutex_lock(mutex_t *);
extern int cm_mutex_unlock(mutex_t *);
extern int cm_mutex_destroy(mutex_t *);
#endif /* PTHREAD */


#endif /* NOTHREADS */

