/* $XConsortium: svcmain.c /main/cde1_maint/7 1995/10/10 13:36:36 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)svcmain.c	1.19 96/09/27 Sun Microsystems, Inc."

/* #define TIMING */

#ifdef MT
#define MAXTHREADS 32
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
#ifdef SVR4
#ifndef _NETINET_IN_H
#include <netinet/in.h>
#endif /* _NETINET_IN_H */
#endif

#if defined(SunOS) || defined(USL) || defined(__uxp__)
#include <netconfig.h>
#include <netdir.h>
#include <sys/stropts.h>
#include <tiuser.h>
#endif /* SunOS || USL || __uxp__ */
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <rpc/rpc.h>
#include <sys/file.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>

#ifdef MT
#include <pthread.h>
#endif

#include "rpcextras.h"
#include "log.h"
#include "cmscalendar.h"
#include "repeat.h"
#include "lutil.h"
#include "cmsdata.h"
#include "utility.h"

#ifndef	S_IRWXU
#define	S_IRWXU		(S_IRUSR|S_IWUSR|S_IXUSR)
#endif
#ifndef	S_IRWXG
#define	S_IRWXG		(S_IRGRP|S_IWGRP|S_IXGRP)
#endif
#ifndef	S_IRWXO
#define	S_IRWXO		(S_IROTH|S_IWOTH|S_IXOTH)
#endif

#define S_MASK  (S_INPUT|S_HIPRI|S_ERROR|S_HANGUP)

extern int	standalone; /* 0 if invaked by inetd, 1 otherwise */
extern int      first_garbage_collection_time = 3600 * 24;

extern void garbage_collect();

/* global variables */
int	debug;
char	*pgname;
int	daemon_gid, daemon_uid;

#ifdef DEBUG
debug = 1;
#endif /* DEBUG */

/*
 * By default strict_csa_model is set to false, i.e. the server
 * will simulate OpenWindows behavior and allow users who have only
 * CSA_VIEW_PUBLIC_ENTRIES access to see the time of CSA_CLASS_CONFIDENTIAL
 * entries (actually we only change the content of the summary attribute
 * to "Appointment" for events, "To Do" for todos, "Memo" for memos and
 * "Calendar Entry" for other entry types.
 * The -c command line optiion specifies that we follow csa access model
 * and only allow users with the corresponding access to view the specific
 * class of entries.
 */
boolean_t	strict_csa_model = _B_FALSE;


