/*
 * @(#)smtp.h	1.4 97/05/19
 *
 * Program:	Simple Mail Transfer Protocol (SMTP) routines
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	27 July 1988
 * Last Edited:	Oct 26 1996
 *
 * Sponsorship:	The original version of this work was developed in the
 *		Symbolic Systems Resources Group of the Knowledge Systems
 *		Laboratory at Stanford University in 1987-88, and was funded
 *		by the Biomedical Research Technology Program of the National
 *		Institutes of Health under grant number RR-00785.
 *
 * Original version Copyright 1988 by The Leland Stanford Junior University
 * Copyright 1995 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * above copyright notices and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington or The
 * Leland Stanford Junior University not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written prior
 * permission.  This software is made available "as is", and
 * THE UNIVERSITY OF WASHINGTON AND THE LELAND STANFORD JUNIOR UNIVERSITY
 * DISCLAIM ALL WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO THIS SOFTWARE,
 * INCLUDING WITHOUT LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE UNIVERSITY OF
 * WASHINGTON OR THE LELAND STANFORD JUNIOR UNIVERSITY BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, TORT (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifndef _SMTP_H_
#define _SMTP_H_
#include "UnixDefs.h"


#ifdef __cplusplus
extern "C" {
#endif

/* Constants */

#define SMTPTCPPORT (long) 25	/* assigned TCP contact port */
#define SMTPGREET (long) 220	/* SMTP successful greeting */
#define SMTPOK (long) 250	/* SMTP OK code */
#define SMTPREADY (long) 354	/* SMTP ready for data */
#define SMTPSOFTFATAL (long) 421/* SMTP soft fatal code */
#define SMTPHARDERROR (long) 554/* SMTP miscellaneous hard failure */


/* SMTP open options */

#define SOP_DEBUG (long) 1	/* debug protocol negotiations */
#define SOP_ESMTP (long) 2	/* ESMTP requested */

/* Body format types for Sun */
#define MIMEBF   1              /* standard MIME  */
#define SUNATTBF 2		/* Sun attachments */
#define SUNATTBF_TOFILE 3	/* Send data to file plutot qu'a SMTP */

/* Function prototypes */

SENDSTREAM *	smtp_open(char ** hostlist, long options, MAILSTREAM * stream);
SENDSTREAM *	smtp_close(SENDSTREAM * stream);
long 		smtp_mail(SENDSTREAM 	* stream,
			  char 	     	* type,
			  ENVELOPE   	* msg,
			  BODY 	     	* body,
			  char 		* outgoing_log,
			  int 		  body_format,
			  int 		  return_receipt,
			  HEADERLIST 	* header_list,
			  MAILSTREAM 	* ms);
void 		smtp_debug(SENDSTREAM * stream);
void	 	smtp_nodebug(SENDSTREAM * stream);
void	 	smtp_rcpt(SENDSTREAM * stream, ADDRESS * adr, boolean_t * error);
long		smtp_send(SENDSTREAM * stream, char * command, char * args);
long		smtp_reply(SENDSTREAM * stream);
long 		smtp_ehlo(SENDSTREAM * stream,char * host);
long	 	smtp_fake(SENDSTREAM * stream, long code, char * text);
long 		smtp_soutr(void * stream, char * s, MAILSTREAM * ms);

#ifdef __cplusplus
}
#endif

#endif /* _SMTP_H_ */
