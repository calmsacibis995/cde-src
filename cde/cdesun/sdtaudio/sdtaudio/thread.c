/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#include <thread.h>
#include <Xm/XmAll.h>
#include "sdtaudio.h"
#include "thrdefs.h"

/*****************************************************************************/
/*
 * at_mutex_lock
 *
 * lock the mutex
 */
int at_mutex_lock(mutex_t *mp)
{
  return(mutex_lock(mp));
}
/*****************************************************************************/
/*
 * at_mutex_unlock
 *
 * unlock the mutex
 */
int at_mutex_unlock(mutex_t *mp)
{
  return(mutex_unlock(mp));
}
/*****************************************************************************/
int at_thread_create(void *(*start_routine)(void *), void *arg)
{
  return (thr_create(NULL, 0, start_routine, (void *)arg, 0, NULL));
}
/*****************************************************************************/
/*
 * xa_lock
 *
 * lock the window data structure's audio data
 */
int xa_lock(WindowData *wd)
{
  return(at_mutex_lock(&(wd->xa_mtx)));
}
/*****************************************************************************/
/*
 * xa_unlock
 *
 * unlock the window data structure's audio data
 */
int xa_unlock(WindowData *wd)
{
    return(at_mutex_unlock(&(wd->xa_mtx)));
}

