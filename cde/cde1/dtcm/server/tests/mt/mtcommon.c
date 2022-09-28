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

#include <mtcommon.h>

#ifdef PTHREAD
pthread_mutex_t	threads_lock;
pthread_cond_t	cond_done;
#else
mutex_t	threads_lock;
cond_t	cond_done;
#endif

void *
create_test_threads(int num, void *(*func)(), ThrData data[])
{
	int	i;

	for (i = 0; i < num; i++) {
		mttest_thr_create(func, (void *)&data[i], NULL);
	}

	mttest_mutex_lock(&threads_lock);
	mttest_cond_wait(&cond_done, &threads_lock);
	mttest_mutex_unlock(&threads_lock);
}

#ifdef PTHREAD
int mttest_mutex_lock(pthread_mutex_t *mp)
{
	return (pthread_mutex_lock(mp));
}

int mttest_mutex_unlock(pthread_mutex_t *mp)
{
	return (pthread_mutex_unlock(mp));
}

int
mttest_cond_wait(pthread_cond_t *cvp, pthread_mutex_t *mp)
{
	return (pthread_cond_wait(cvp, mp));
}

int
mttest_cond_signal(pthread_cond_t *cvp)
{
	return (pthread_cond_signal(cvp));
}

int
mttest_cond_broadcast(pthread_cond_t *cvp)
{
	return (pthread_cond_broadcast(cvp));
}

pthread_t
mttest_thr_self()
{
	return (pthread_self());
}

int
mttest_thr_create(
	void * (*start_func)(void *),
	void *arg,
	pthread_t *new_id)
{
	return (pthread_create(new_id, NULL, start_func, arg));
}

#else /* PTHREAD */

int mttest_mutex_lock(mutex_t *mp)
{
	return (mutex_lock(mp));
}

int mttest_mutex_unlock(mutex_t *mp)
{
	return (mutex_unlock(mp));
}

int
mttest_cond_wait(cond_t *cvp, mutex_t *mp)
{
	return (cond_wait(cvp, mp));
}

int
mttest_cond_signal(cond_t *cvp)
{
	return (cond_signal(cvp));
}

int
mttest_cond_broadcast(cond_t *cvp)
{
	return (cond_broadcast(cvp));
}

thread_t
mttest_thr_self()
{
	return (thr_self());
}

int
mttest_thr_create(
	void * (*start_func)(void *),
	void *arg,
	thread_t *new_id)
{
	return (thr_create(NULL, 0, start_func, arg,
		0, new_id));
}

#endif /* PTHREAD */


