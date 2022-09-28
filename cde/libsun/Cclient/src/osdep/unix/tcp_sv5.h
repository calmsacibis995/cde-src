/*
 * @(#)tcp_sv5.h	1.11 0
 *
 * Program:	UNIX TCP/IP routines
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	1 August 1988
 * Last Edited:	21 Oct 1996
 *
 * Copyright 1995 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appears in all copies and that both the
 * above copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  This software is made available
 * "as is", and
 * THE UNIVERSITY OF WASHINGTON DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT LIMITATION ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, AND IN
 * NO EVENT SHALL THE UNIVERSITY OF WASHINGTON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * This code is based on the above copyright and is a solaris extension
 * written by 
 *       Copyright (c) SunSoft, Inc. June 7, 1995.
 *
 * Edit log:
 *
 * [12-21-96 ] Define TCP_GLOBALS in this file.
 *
 */
#ifndef _TCP_SV5_H
#define _TCP_SV5_H
 /* TCP input buffer */
#include "UnixDefs.h"
#include <netdb.h>		/* right place to include these two files  */
#define NSS_BUFLEN_PASSWD	1024
#define NSS_BUFLEN_SHADOW	1024
#define NSS_BUFLEN_SERVICES	1024
#define	NSS_BUFLEN_PROTOCOLS	1024

/*#include <nss_dbdefs.h>*/

#ifdef __cplusplus
extern "C" {
#endif

#define BUFLEN 2048

/* TCP I/O stream (must be before osdep.h is included) */
#define TCPSTREAM struct tcp_stream
TCPSTREAM {
  char *host;			/* host name */
  char *localhost;		/* local host name */
  unsigned long port;           /* local port */
  int pid;			/* process ID if piped */
  int tcpsi;			/* input socket */
  int tcpso;			/* output socket */
  int ictr;			/* input counter */
  char *iptr;			/* input pointer */
  char ibuf[BUFLEN];		/* input buffer */
  /*
   * Stuff for callback mechanism */
  void *mailstr;		/* set after open for callback */
  /* flag for allowing io err callbacks */
  int disallow_callback;        /* No callback if true */
  /* logfile name for logging outgoing mail in smtp.c */
  char *outgoing_log;
  int wrote_ihdr;		/* true if internal header has been done */
  /*
   * bytes read/written */
  long io_bytect;		/* cleared in clear_byte_count(...) */
  int os_send_urgent;		/* flag set by imap to send urgent byte */
  int urgent_exit;              /* True if getdata exited urgently */
  int u_flushed_input;		/* if urgent flushing!Oui, beaucoup de merde */
  int max_wait_on;              /* true if we wait a long time for stream dead */
};

typedef long (*tcptimeout_t) (long time); /* need to declare it here
                                because TCP_GLOBALS needs it. */
	
  /* A place to hide all tcp globals */
typedef struct tcp_globals {
  tcptimeout_t tcptimeout; /* TCP timeout handler routine */
  long tcptimeout_read;   /* TCP timeouts, in seconds */
  long tcptimeout_write;
 
} TCP_GLOBALS;

/* Local function prototypes */
long tcp_abort (TCPSTREAM *stream);

/* Other solaris definitions NOT in tcp.h */
#define CBTIMEOUT 0
#define CBSTRDEAD 1
#define CBWAITING 2
#define CBDISKIO  3

void set_disallow_flag (TCPSTREAM *stream, int value);
void set_outgoing_log(TCPSTREAM *stream, char *fname);
void set_tcpstream_stream(TCPSTREAM *, void *);
char *get_outgoing_log(TCPSTREAM *stream);
int outgoing_from_done(void *stream);
char *local_host_name(void *stream);
void clear_tcp_iocounter (TCPSTREAM *tcpstr);
int tcp_flushed_input (TCPSTREAM *stream);
void os_clear_send_urgent(TCPSTREAM *stream);
void os_increment_send_urgent(TCPSTREAM *stream);

#ifdef __cplusplus
}
#endif

#endif /*_TCP_SV5_H*/
