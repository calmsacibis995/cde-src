/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#ifndef SDTAUDIO_THREAD_H

extern int at_mutex_lock(mutex_t *);
extern int at_mutex_unlock(mutex_t *);
extern int xa_lock(WindowData *);
extern int xa_unlock(WindowData *);
int at_thread_create(void * (*)(void *), void *arg);

#endif
