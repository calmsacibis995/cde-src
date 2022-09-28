/*
 * @(#)tcp.h	1.4 97/05/19
 *
 * Program:	TCP/IP routines
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
 * Last Edited:	7 September 1994
 *
 * Copyright 1994 by the University of Washington
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
 */
#ifndef _TCP_H_
#define _TCP_H_
#include "UnixDefs.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Dummy definition overridden by TCP routines */
/* Function prototypes */
TCPSTREAM *	tcp_open(char 	    *	host,
			 char 	    *	service,
			 long 		port, 
			 MAILSTREAM *	stream);
TCPSTREAM *	tcp_aopen(char * host, char * service);
unsigned long 	tcp_port(TCPSTREAM    * stream);
boolean_t	tcp_getdata(TCPSTREAM * stream);
long 		tcp_getbuffer(TCPSTREAM *	stream,
			      unsigned long 	size,
			      char 	*	buffer);
long 		tcp_sout(TCPSTREAM * stream,
			 char 	   * string,
			 unsigned long size);
void *		tcp_parameters(MAILSTREAM * stream,
			       long 	    function,
			       void 	  * value);
long 		tcp_soutr(TCPSTREAM * stream, char * string);
void 		tcp_close(TCPSTREAM * stream);
char *		tcp_getline(TCPSTREAM * stream);
char *		tcp_host(TCPSTREAM * stream);
char *		tcp_localhost(TCPSTREAM * stream);
char *		tcp_clienthost(char * dst);

#ifdef __cplusplus
}
#endif

#endif /* _TCP_H_ */
