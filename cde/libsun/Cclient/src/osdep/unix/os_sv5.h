/*
 * @(#)os_sv5.h	1.16 97/06/13
 *
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

#ifndef _OS_SV5_H
#define _OS_SV5_H
#include "UnixDefs.h"

#include <ctype.h>
#include <sys/tiuser.h>
#include <sys/stropts.h>
#include <sys/poll.h>
#include <time.h>
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
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/utime.h>
#include <sys/systeminfo.h>

#include "misc.h"
#include "mail.h"
#include "tcp_sv5.h"


#ifdef __cplusplus
extern "C" {
#endif

#define MAILFILE "/var/mail/%s"
#define ACTIVEFILE "/usr/share/news/active"
#define NEWSSPOOL "/var/spool/news"
#define NEWSRC strcat (strcpy (tmp,getpwuid (geteuid ())->pw_dir),"/.newsrc")

#define IOWAIT 100		/* .1 seconds */
#define MAXWAIT (140*1000)       /* 2 min 20 sec  in milliseconds */
#define MSGWAIT (20*1000)       /* 20 seconds */
#define IO1STTIMEOUT ((MAXWAIT - MSGWAIT)/1000)
#define IO2NDTIMEOUT (IO1STTIMEOUT - 1)

/*
 * Many versions of SysV get this wrong
 *
 * Different names, equivalent things in BSD and SysV.
 * L_SET is defined for some strange reason in <sys/file.h> on SVR4.
 */
#ifndef L_SET
#define L_SET SEEK_SET
#endif
#define L_INCR SEEK_CUR
#define L_XTND SEEK_END

#define direct dirent
/* #define random lrand48 */

/* For flock() emulation */
#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8


#define CBTIMEOUT 0
#define CBSTRDEAD 1
#define CBWAITING 2
#define CBDISKIO  3

/* Function prototypes */
#include "env_unix.h"
#include "fs.h"
#include "ftl.h"
#include "nl.h"
#include "tcp.h"

void 	rfc822_date(char * date);
void 	rfc822_from_date(char * date);

int 	utimes(char *file, struct timeval *tvp);
long 	gethostid(void);
long 	random(void);
int 	scandir(char * dirname,
		struct dirent *** namelist,
		int (*select) (),
		int (*compar) ());

int 	bsd_flock(int fd, int operation, MAILSTREAM * stream);
int 	flock(int fd, int operation);
long 	elapsed_ms(void);
void 	set_mime_boundary(char * buf);
char * 	unique_hwid();

/* New function calls */
void 	set_disallow_flag(TCPSTREAM * stream, int value);
void 	set_long_maxwait(TCPSTREAM * stream, int value);
void 	set_outgoing_log(TCPSTREAM * stream, char *fname);
void 	set_tcpstream_stream(TCPSTREAM *, void *);
char *	get_outgoing_log(TCPSTREAM * stream);
int 	outgoing_from_done(void * stream);
char *	local_host_name(void * stream);
void 	clear_tcp_iocounter (TCPSTREAM * tcpstr);
int 	tcp_flushed_input (TCPSTREAM * stream);
void 	os_clear_send_urgent(TCPSTREAM * stream);
void 	os_increment_send_urgent(TCPSTREAM * stream);

/* for mmap(..) stuff. */
char *	fs_mmap(int * fd, long len, char ** name);
int 	fs_munmap(int fd, char *name,  char * buf, long len);
int 	fs_mmapput(int fd, char c);

#ifdef __cplusplus
}
#endif

extern int sys_nerr;
extern char *sys_errlist[];

#define toint(c)	((c)-'0')
#define isodigit(c)	(((unsigned)(c)>=060)&((unsigned)(c)<=067))
#define DIR_SIZE(d) 	(d->d_reclen)


#endif /* _OS_SV5_H */
