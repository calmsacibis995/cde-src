/*
 * Program:	File routines
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	25 August 1993
 * Last Edited:	15 May 1995
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

/* Build parameters */
#include "UnixDefs.h"


/* Types returned from phile_type() */

#define PTYPEBINARY 0		/* binary data */
#define PTYPETEXT 1		/* textual data */
#define PTYPECRTEXT 2		/* textual data with CR */
#define PTYPE8 4		/* textual 8bit data */
#define PTYPEISO2022JP 8	/* textual Japanese */
#define PTYPEISO2022KR 16	/* textual Korean */


/* PHILE I/O stream local data */
	
typedef struct phile_local {
  ENVELOPE *env;		/* file envelope */
  BODY *body;			/* file body */
  char *buf;			/* buffer for message text */
  unsigned long size;		/* size of buffer */
  char tmp[MAILTMPLEN];		/* temporary buffer */
} PHILELOCAL;


/* Convenient access to local data */

#define LOCAL ((PHILELOCAL *) stream->local)

/* Function prototypes */

DRIVER *phile_valid (char *name);
int phile_isvalid (char *name,char *tmp);
void *phile_parameters (long function,void *value);
void phile_list (MAILSTREAM *stream,char *ref,char *pat);
void phile_lsub (MAILSTREAM *stream,char *ref,char *pat);
long phile_create (MAILSTREAM *stream,char *mailbox);
long phile_delete (MAILSTREAM *stream,char *mailbox);
long phile_rename (MAILSTREAM *stream,char *old,char *new);
MAILSTREAM *phile_open (MAILSTREAM *stream);
int phile_type (unsigned char *s,unsigned long i,unsigned long *j);
void phile_close (MAILSTREAM *stream,long options);
void phile_fetchfast (MAILSTREAM *stream,char *sequence,long flags);
void phile_fetchflags (MAILSTREAM *stream,char *sequence,long flags);
ENVELOPE *phile_fetchstructure (MAILSTREAM *stream,unsigned long msgno,
				BODY **body,long flags);
char *phile_fetchheader (MAILSTREAM *stream,unsigned long msgno,
			 STRINGLIST *lines,unsigned long *len,long flags);
char *phile_fetchtext (MAILSTREAM *stream,unsigned long msgno,
		       unsigned long *len,long flags);
char *phile_fetchbody (MAILSTREAM *stream,unsigned long msgno,char *sec,
		       unsigned long *len,long flags);
void phile_setflag (MAILSTREAM *stream,char *sequence,char *flag,long flags);
void phile_clearflag (MAILSTREAM *stream,char *sequence,char *flag,long flags);
long phile_ping (MAILSTREAM *stream);
void phile_check (MAILSTREAM *stream);
void phile_expunge (MAILSTREAM *stream);
long phile_copy (MAILSTREAM *stream,char *sequence,char *mailbox,long options);
long phile_append (MAILSTREAM *stream,char *mailbox,char *flags,char *date,
		   STRING *message);
void phile_gc (MAILSTREAM *stream,long gcflags);
