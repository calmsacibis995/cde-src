/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)mutex.h	1.1 96/09/23 SMI"

#ifndef _MUTEX_H
#define _MUTEX_H

/* Verbosity turns on mutex debugging messages. */ 
#ifdef VERBOSITY

/* XA_LOCK() and XA_UNLOCK() functions. */
#define XA_LOCK(wd, label) \
fprintf(stderr, "XA LOCK REQUEST: %s()\n", label); \
xa_lock(wd); \
fprintf(stderr, "XA LOCK GRANTED AND HELD: %s()\n", label)

#define XA_UNLOCK(wd, label) \
fprintf(stderr, "XA UNLOCK REQUEST: %s()\n", label); \
xa_unlock(wd); \
fprintf(stderr, "XA UNLOCK GRANTED: %s()\n", label)

/* AT_MUTEX_LOCK() and AT_MUTEX_UNLOCK() functions. */
#define AT_MUTEX_LOCK(wd, label) \
fprintf(stderr, "AT LOCK REQUEST: %s()\n", label); \
at_mutex_lock(&(wd->state_mtx)); \
fprintf(stderr, "AT LOCK GRANTED AND HELD: %s()\n", label)

#define AT_MUTEX_UNLOCK(wd, label) \
fprintf(stderr, "AT UNLOCK REQUEST: %s()\n", label); \
at_mutex_unlock(&(wd->state_mtx)); \
fprintf(stderr, "AT UNLOCK GRANTED: %s()\n", label)

#else

/* XA_LOCK() and XA_UNLOCK() functions. */
#define XA_LOCK(wd, label) xa_lock(wd)
#define XA_UNLOCK(wd, label) xa_unlock(wd)

/* AT_MUTEX_LOCK() and AT_MUTEX_UNLOCK() functions. */
#define AT_MUTEX_LOCK(wd, label) at_mutex_lock(&(wd->state_mtx))
#define AT_MUTEX_UNLOCK(wd, label) at_mutex_unlock(&(wd->state_mtx))

#endif	/* VERBOSITY */


#endif	/* _MUTEX_H */
