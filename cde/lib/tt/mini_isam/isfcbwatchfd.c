/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * isfcbwatchfd.c
 *
 * Description:
 *	Watch the limit on UNIX file descriptors used by the FCB module.
 */

#if defined(_AIX)
#define _POSIX_SYSCONF
#endif 

#if defined(_AIX)
#define _POSIX_SYSCONF
#endif 

#include "isam_impl.h"
#include <sys/time.h>
#include <sys/resource.h>
#ifdef _POSIX_SYSCONF
#include <unistd.h>
#endif /* _POSIX_SYSCONF */

static int _limit = MAXFCB_UNIXFD;	     /* Imposed limit */
static int _in_use = 0;			     /* Current number of 
					      * open file descriptors
					      */

/*
 * _watchfd_incr(n)
 *
 * Register that n additional UNIX file descriptors were open.
 *
 * Return the new number of open file descriptors.
 */

int
_watchfd_incr(int n)
{
    _in_use += n;
    assert(_in_use <= _limit);
    return (_in_use);
}


/*
 * _watch_decr(n)
 *
 * Register that n open file descriptors were closed.
 *
 * Return the new number of open file descriptors.
 */

int
_watchfd_decr(int n)
{
    _in_use -= n;
    assert(_in_use >= 0);
    return (_in_use);
}


/*
 * _watch_check()
 *
 * Return number of fd that are still available.
 */

int
_watchfd_check(void)
{
    return (_limit - _in_use);
}


/*
 * _watchfd_max_set(n)
 *
 * Set the maximum number of UNIX fds that may be comsumed by ISAM files.
 */

int
_watchfd_max_set(int n)
{
    int		oldlimit = _limit;

#ifdef _POSIX_SYSCONF
    if (n < 3 || n > sysconf(_SC_OPEN_MAX)) {
#else
    int dtab_size = 0;
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
      dtab_size = rl.rlim_cur;

    if (n < 3 || n > dtab_size) {
#endif
	_setiserrno2(EBADARG, '9', '0');
	return (ISERROR);
    }

    _limit = n;
    return (oldlimit);
}

/*
 * _watchfd_max_get()
 *
 * Get the maximum number of UNIX fds that may be comsumed by ISAM files.
 */

int
_watchfd_max_get(void)
{
    return (_limit);
}
