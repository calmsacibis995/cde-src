/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#).del-mt.c	1.1 95/12/19 Sun Microsystems, Inc."

#ifndef CSA_NOT_MT_SAFE

#include <time.h>
#include "mt.h"
#include "debug.h"

#ifdef PTHREAD
int _DtCm_mutex_lock(pthread_mutex_t *mp)
{
	DP(("mutex_lock %x\n", mp));
	return (pthread_mutex_lock(mp));
}

int _DtCm_mutex_unlock(pthread_mutex_t *mp)
{
	return (pthread_mutex_unlock(mp));
}

int
_DtCm_cond_wait(pthread_cond_t *cvp, pthread_mutex_t *mp)
{
	return (pthread_cond_wait(cvp, mp));
}

int
_DtCm_cond_signal(pthread_cond_t *cvp)
{
	return (pthread_cond_signal(cvp));
}

int
_DtCm_cond_broadcast(pthread_cond_t *cvp)
{
	return (pthread_cond_broadcast(cvp));
}

pthread_t
_DtCm_thr_self()
{
	return (pthread_self());
}

#else /* PTHREAD */

int _DtCm_mutex_lock(mutex_t *mp)
{
	DP(("mutex_lock %x\n", mp));
	return (mutex_lock(mp));
}

int _DtCm_mutex_unlock(mutex_t *mp)
{
	return (mutex_unlock(mp));
}

int
_DtCm_cond_wait(cond_t *cvp, mutex_t *mp)
{
	return (cond_wait(cvp, mp));
}

int
_DtCm_cond_signal(cond_t *cvp)
{
	return (cond_signal(cvp));
}

int
_DtCm_cond_broadcast(cond_t *cvp)
{
	return (cond_broadcast(cvp));
}

thread_t
_DtCm_thr_self()
{
	return (thr_self());
}

#endif /* PTHREAD */

#endif /* CSA_NOT_MT_SAFE */

