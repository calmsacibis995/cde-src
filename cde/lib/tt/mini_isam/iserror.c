/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * iserror.c
 *
 * Description:
 *   	NetISAM error handling functions.
 *
 */

#include "isam_impl.h"
#include <errno.h>
#include <syslog.h>
#include <stdlib.h>
#ifdef __ultrix__
#define LOG_USER LOG_INFO
#endif
/*
 * _isfatal_error(msg)
 *
 * Fatal error. Display message and terminate program.
 */

static int (*fatal_error_user_handler)();    /* set by iscntl(..,ISCNTL_FATAL,..) */

void 
_isfatal_error(msg, str_err)
    char	*msg;
    char	*str_err;
{
  int		logerr;
  int		saved_errno = errno;

  if (fatal_error_user_handler) {
    logerr = fatal_error_user_handler(msg); /* User returns 1 in order
					     * to use syslog() 
					     */
  } else {
    logerr = 1;
  }

  if (logerr) {
    openlog("ToolTalk/ISAM", LOG_PID, LOG_USER);

    if (str_err) {
    	syslog(LOG_ERR, "Fatal error: %s - %s", msg, str_err);
    } else {
    	syslog(LOG_ERR, "Fatal error: %s", msg);
    }

    closelog();
  }
  exit (1);
}

void
_isam_warning(msg)
    char	*msg;
{
    openlog("ToolTalk/ISAM", LOG_PID, LOG_USER);
    syslog(LOG_ERR, msg);
}

/* Set user specified fatal_error handler */
int  (*_isfatal_error_set_func(func))()
    int		(*func)();
{
    int		(*oldfunc)();

    oldfunc = fatal_error_user_handler;
    fatal_error_user_handler = func;

    return (oldfunc);
}

/*
 * _setiserrno2(error, isstat1, isstat2)
 *
 * Set iserrno variable.
 */

void
_setiserrno2(error, is1, is2)
    int		error;
    int		is1, is2;
{
    iserrno = error;
    isstat1 = is1;
    isstat2 = is2;
}

/*
 * _seterr_errcode(errcode)
 *
 * Set all error and status variable from errcode structure.
 */

void
_seterr_errcode(errcode)
    register struct errcode	*errcode;
{
    iserrno = errcode->iserrno;
    isstat1 = errcode->isstat[0];
    isstat2 = errcode->isstat[1];
    isstat3 = errcode->isstat[2];
    isstat4 = errcode->isstat[3];
}
