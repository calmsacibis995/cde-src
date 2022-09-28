/*
 * Program:	Operating-system dependent routines -- SVR4 version
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	10 April 1992
 * Last Edited:	4 December 1992
 *
 * Copyright 1992 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appears in all copies and that both the
 * above copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  This software is made
 * available "as is", and
 * THE UNIVERSITY OF WASHINGTON DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT LIMITATION ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, AND IN
 * NO EVENT SHALL THE UNIVERSITY OF WASHINGTON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
#include "UnixDefs.h"
#include "tcp_sv5.h"
#include "mail.h"
#include "os_sv5.h"

#undef flock

#include <ctype.h>
#include <sys/tiuser.h>
#include <sys/stropts.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/uio.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <regexpr.h>
#include <errno.h>
#include <pwd.h>
#include <shadow.h>
#include <syslog.h>
#include "misc.h"

extern int sys_nerr;
extern char *sys_errlist[];

#define toint(c)	((c)-'0')
#define isodigit(c)	(((unsigned)(c)>=060)&((unsigned)(c)<=067))
#define DIR_SIZE(d) 	(d->d_reclen)

/* Included source files  */

#include "fs_unix.c"		/* fs_get() etc ..*/
#include "ftl_unix.c"		/* fatal() */
#include "nl_unix.c"		/* strcrlf*() stuff */
#include "env_unix.c"		/* misc. environmental trucs. */
#include "tcp_sv5.c"		/* tcp for solaris version 5. */
#include "log_sv5.c"		/* server_login () */
#include "gr_waitp.c"		/* grim reaper. Touer une tache. */
#include "flock.c"		/* bsd_flock() */
#include "flock_sv5.c"		/* sol_flock() USES lockf() */
#include "gettime.c"		/* gettime/etc ... */
#include "scandir.c"		/* scandir/etc ... */
#include "tz_sv5.c"		/* time zone */
#include "gethstid.c"		/* gethostid() */
#include "mmap_sv5.c"		/* solaris mmap() stuff */
#include "misc_sv5.c"		/* misc. solaris: EG elapsed_msg() */

