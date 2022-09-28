/*
 * Program:	Dummy routines
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	9 May 1991
 * Last Edited:	27 June 1995
 *
 * Copyright 1995 by the University of Washington
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
#ifndef _DUMMY_H_
#define _DUMMY_H_

/* Function prototypes */
#include "UnixDefs.h"

DRIVER *dummy_valid (char *name, MAILSTREAM *stream);
void *dummy_parameters (MAILSTREAM *stream, long function,void *value);
void dummy_scan (MAILSTREAM *stream,char *ref,char *pat,char *contents);
void dummy_list (MAILSTREAM *stream,char *ref,char *pat);
void dummy_lsub (MAILSTREAM *stream,char *ref,char *pat);
void dummy_list_work (MAILSTREAM *stream,char *dir,char *pat,char *contents,
		      long level);
void dummy_listed (MAILSTREAM *stream,char delimiter,char *name,
		   long attributes,char *contents);
long dummy_create (MAILSTREAM *stream,char *mailbox);
long dummy_delete (MAILSTREAM *stream,char *mailbox);
long dummy_rename (MAILSTREAM *stream,char *old,char *new);
MAILSTREAM *dummy_open (MAILSTREAM *stream);
void dummy_close (MAILSTREAM *stream,long options);
void dummy_fetchfast (MAILSTREAM *stream,char *sequence,long flags);
void dummy_fetchflags (MAILSTREAM *stream,char *sequence,long flags);
ENVELOPE *dummy_fetchstructure (MAILSTREAM *stream,unsigned long msgno,
				BODY **body,long flags);
char *dummy_fetchheader (MAILSTREAM *stream,unsigned long msgno,
			 STRINGLIST *lines,unsigned long *len,long flags);
char *dummy_fetchtext (MAILSTREAM *stream,unsigned long msgno,
		       unsigned long *len,long flags);
char *dummy_fetchbody (MAILSTREAM *stream,unsigned long msgno,char *sec,
		       unsigned long *len,long flags);
unsigned long dummy_uid (MAILSTREAM *stream,unsigned long msgno);
void dummy_setflag (MAILSTREAM *stream,char *sequence,char *flag,long flags);
void dummy_clearflag (MAILSTREAM *stream,char *sequence,char *flag,long flags);
void dummy_search (MAILSTREAM *stream,char *charset,SEARCHPGM *pgm,long flags);
long dummy_ping (MAILSTREAM *stream);
void dummy_check (MAILSTREAM *stream);
void dummy_expunge (MAILSTREAM *stream);
long dummy_copy (MAILSTREAM *stream,char *sequence,char *mailbox,long options);
long dummy_append(MAILSTREAM	* stream,
		  char 		* mailbox,
		  char 		* flags,
		  char 		* date,
		  char		* xUnixFrom,
		  STRING 	* message);
void dummy_gc (MAILSTREAM *stream,long gcflags);
char *dummy_file (char *dst,char *name, MAILSTREAM *stream);
long dummy_canonicalize (char *tmp,char *ref,char *pat);

/* SUN imap4 KEEP THESE */
char *dummy_fabricate_from (MAILSTREAM *stream, long msgno);
int dummy_set1flag (MAILSTREAM *stream,char *sequence,char *flag,long flags);
int dummy_clear1flag (MAILSTREAM *stream,char *sequence,char *flag,long flags);
int dummy_sunversion(MAILSTREAM *stream); /* VF */
void dummy_checkpoint(MAILSTREAM *stream, unsigned long min_msgno, long flags);  /* VF */
void dummy_urgent(MAILSTREAM *stream);
int dummy_echo (MAILSTREAM *stream);
void dummy_disconnect(MAILSTREAM *stream); /* VF */
void dummy_clear_iocount (MAILSTREAM *stream); /* ML */
void dummy_fetchshort(MAILSTREAM *stream, unsigned long msgno, SHORTINFO *sinfo, 
		     long flags);
void dummy_checksum (MAILSTREAM *stream);
int dummy_validchecksum (MAILSTREAM *stream,
			 unsigned short checksum,
			 unsigned long mbox_size);
int dummy_sync_msgno (MAILSTREAM *stream,char *uidseq);
int dummy_imap4(MAILSTREAM *stream);
void dummy_global_init(MAILSTREAM *stream);
void dummy_global_free(MAILSTREAM *stream);

#endif /*_DUMMY_H_*/
