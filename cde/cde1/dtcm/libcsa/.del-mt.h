/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _MT_H
#define _MT_H

#pragma ident "@(#).del-mt.h	1.2 96/11/12 Sun Microsystems, Inc."

#ifndef CSA_NOT_MT_SAFE

#include "ansi_c.h"

#ifdef PTHREAD
#include <pthread.h>

int _DtCm_mutex_lock(pthread_mutex_t *mp);
int _DtCm_mutex_unlock(pthread_mutex_t *mp);
int _DtCm_cond_wait (pthread_cond_t *cvp, pthread_mutex_t *mp);
int _DtCm_cond_signal(pthread_cond_t *cvp);
int _DtCm_cond_broadcast(pthread_cond_t *cvp);
pthread_t _DtCm_thr_self();

#else /* PTHREAD */
#include <thread.h>
#include <synch.h>

int _DtCm_mutex_lock(mutex_t *mp);
int _DtCm_mutex_unlock(mutex_t *mp);
int _DtCm_cond_wait(cond_t *cvp, mutex_t *mp);
int _DtCm_cond_signal(cond_t *cvp);
int _DtCm_cond_broadcast(cond_t *cvp);
thread_t _DtCm_thr_self();

#endif /* PTHREAD */
#endif /* CSA_NOT_MT_SAFE */

#endif /* _MT_H */
