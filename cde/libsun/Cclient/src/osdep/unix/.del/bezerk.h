/*
 * Program:	Berkeley mail routines
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	20 December 1989
 * Last Edited:	25 May 1995
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


/* Dedication:
 *  This file is dedicated with affection to those Merry Marvels of Musical
 * Madness . . .
 *  ->  The Incomparable Leland Stanford Junior University Marching Band  <-
 * who entertain, awaken, and outrage Stanford fans in the fact of repeated
 * losing seasons and shattered Rose Bowl dreams [Cardinal just don't have
 * HUSKY FEVER!!!].
 *
 */

/* Validate line
 * Accepts: pointer to candidate string to validate as a From header
 *	    return pointer to end of date/time field
 *	    return pointer to offset from t of time (hours of ``mmm dd hh:mm'')
 *	    return pointer to offset from t of time zone (if non-zero)
 * Returns: t,ti,zn set if valid From string, else ti is NIL
 */

#define VALID(s,x,ti,zn) {						\
  ti = 0;								\
  if ((*s == 'F') && (s[1] == 'r') && (s[2] == 'o') && (s[3] == 'm') &&	\
      (s[4] == ' ')) {							\
    for (x = s + 5; *x && *x != '\n'; x++);				\
    if (*x) {								\
      if (x - s >= 41) {						\
	for (zn = -1; x[zn] != ' '; zn--);				\
	if ((x[zn-1] == 'm') && (x[zn-2] == 'o') && (x[zn-3] == 'r') &&	\
	    (x[zn-4] == 'f') && (x[zn-5] == ' ') && (x[zn-6] == 'e') &&	\
	    (x[zn-7] == 't') && (x[zn-8] == 'o') && (x[zn-9] == 'm') &&	\
	    (x[zn-10] == 'e') && (x[zn-11] == 'r') && (x[zn-12] == ' '))\
	  x += zn - 12;							\
      }									\
      if (x - s >= 27) {						\
	if (x[-5] == ' ') {						\
	  if (x[-8] == ':') zn = 0,ti = -5;				\
	  else if (x[-9] == ' ') ti = zn = -9;				\
	  else if ((x[-11] == ' ') && ((x[-10]=='+') || (x[-10]=='-')))	\
	    ti = zn = -11;						\
	}								\
	else if (x[-4] == ' ') {					\
	  if (x[-9] == ' ') zn = -4,ti = -9;				\
	}								\
	else if (x[-6] == ' ') {					\
	  if ((x[-11] == ' ') && ((x[-5] == '+') || (x[-5] == '-')))	\
	    zn = -6,ti = -11;						\
	}								\
	if (ti && !((x[ti - 3] == ':') &&				\
		    (x[ti -= ((x[ti - 6] == ':') ? 9 : 6)] == ' ') &&	\
		    (x[ti - 3] == ' ') && (x[ti - 7] == ' ') &&		\
		    (x[ti - 11] == ' '))) ti = 0;			\
      }									\
    }									\
  }									\
}

/* You are not expected to understand this macro, but read the next page if
 * you are not faint of heart.
 *
 * Known formats to the VALID macro are:
 * 		From user Wed Dec  2 05:53 1992
 * BSD		From user Wed Dec  2 05:53:22 1992
 * SysV		From user Wed Dec  2 05:53 PST 1992
 * rn		From user Wed Dec  2 05:53:22 PST 1992
 *		From user Wed Dec  2 05:53 -0700 1992
 *		From user Wed Dec  2 05:53:22 -0700 1992
 *		From user Wed Dec  2 05:53 1992 PST
 *		From user Wed Dec  2 05:53:22 1992 PST
 *		From user Wed Dec  2 05:53 1992 -0700
 * Solaris	From user Wed Dec  2 05:53:22 1992 -0700
 *
 * Plus all of the above with `` remote from xxx'' after it. Thank you very
 * much, smail and Solaris, for making my life considerably more complicated.
 */

/*
 * What?  You want to understand the VALID macro anyway?  Alright, since you
 * insist.  Actually, it isn't really all that difficult, provided that you
 * take it step by step.
 *
 * Line 1	Initializes the return ti value to failure (0);
 * Lines 2-3	Validates that the 1st-5th characters are ``From ''.
 * Lines 4-5	Validates that there is an end of line and points x at it.
 * Lines 6-13	First checks to see if the line is at least 41 characters long.
 *		If so, it scans backwards to find the rightmost space.  From
 *		that point, it scans backwards to see if the string matches
 *		`` remote from''.  If so, it sets x to point to the space at
 *		the start of the string.
 * Line 14	Makes sure that there are at least 27 characters in the line.
 * Lines 15-20	Checks if the date/time ends with the year (there is a space
 *		five characters back).  If there is a colon three characters
 *		further back, there is no timezone field, so zn is set to 0
 *		and ti is set in front of the year.  Otherwise, there must
 *		either to be a space four characters back for a three-letter
 *		timezone, or a space six characters back followed by a + or -
 *		for a numeric timezone; in either case, zn and ti become the
 *		offset of the space immediately before it.
 * Lines 21-23	Are the failure case for line 14.  If there is a space four
 *		characters back, it is a three-letter timezone; there must be a
 *		space for the year nine characters back.  zn is the zone
 *		offset; ti is the offset of the space.
 * Lines 24-27	Are the failure case for line 20.  If there is a space six
 *		characters back, it is a numeric timezone; there must be a
 *		space eleven characters back and a + or - five characters back.
 *		zn is the zone offset; ti is the offset of the space.
 * Line 28-31	If ti is valid, make sure that the string before ti is of the
 *		form www mmm dd hh:mm or www mmm dd hh:mm:ss, otherwise
 *		invalidate ti.  There must be a colon three characters back
 *		and a space six or nine	characters back (depending upon
 *		whether or not the character six characters back is a colon).
 *		There must be a space three characters further back (in front
 *		of the day), one seven characters back (in front of the month),
 *		and one eleven characters back (in front of the day of week).
 *		ti is set to be the offset of the space before the time.
 *
 * Why a macro?  It gets invoked a *lot* in a tight loop.  On some of the
 * newer pipelined machines it is faster being open-coded than it would be if
 * subroutines are called.
 *
 * Why does it scan backwards from the end of the line, instead of doing the
 * much easier forward scan?  There is no deterministic way to parse the
 * ``user'' field, because it may contain unquoted spaces!  Yes, I tested it to
 * see if unquoted spaces were possible.  They are, and I've encountered enough
 * evil mail to be totally unwilling to trust that ``it will never happen''.
 */

/* Build parameters */

#define KODRETRY 15		/* kiss-of-death retry in seconds */
#define LOCKTIMEOUT 5		/* lock timeout in minutes */
#define CHUNK 32768		/* read-in chunk size */


/* Status string */

#define STATUS "Status: RO\nX-Status: DFAT\nX-UID: 0000000000\n\n\n"

/* BEZERK per-message cache information */

typedef struct file_cache {
  char *header;			/* pointer to RFC 822 header */
  unsigned long headersize;	/* size of RFC 822 header */
  char *body;			/* pointer to message body */
  unsigned long bodysize;	/* size of message body */
  char status[sizeof (STATUS)];	/* status flags */
  char internal[1];		/* start of internal header and data */
} FILECACHE;


/* BEZERK I/O stream local data */
	
typedef struct bezerk_local {
  unsigned int dirty : 1;	/* disk copy needs updating */
  int ld;			/* lock file descriptor */
  char *name;			/* local file name for recycle case */
  char *lname;			/* lock file name */
  off_t filesize;		/* file size parsed */
  time_t filetime;		/* last file time */
  unsigned long cachesize;	/* size of local cache */
  FILECACHE **msgs;		/* pointers to message-specific information */
  char *buf;			/* temporary buffer */
  unsigned long buflen;		/* current size of temporary buffer */
} BEZERKLOCAL;

/* DEFINE THIS HERE. WJY */
#define flock bsd_flock

/* Convenient access to local data */

#define LOCAL ((BEZERKLOCAL *) stream->local)

/* Function prototypes */

DRIVER *bezerk_valid (char *name);
long bezerk_isvalid (char *name,char *tmp);
long bezerk_isvalid_fd (int fd,char *tmp);
void *bezerk_parameters (long function,void *value);
void bezerk_list (MAILSTREAM *stream,char *ref,char *pat);
void bezerk_lsub (MAILSTREAM *stream,char *ref,char *pat);
long bezerk_create (MAILSTREAM *stream,char *mailbox);
long bezerk_delete (MAILSTREAM *stream,char *mailbox);
long bezerk_rename (MAILSTREAM *stream,char *old,char *new);
MAILSTREAM *bezerk_open (MAILSTREAM *stream);
void bezerk_close (MAILSTREAM *stream,long options);
void bezerk_fetchfast (MAILSTREAM *stream,char *sequence,long flags);
void bezerk_fetchflags (MAILSTREAM *stream,char *sequence,long flags);
ENVELOPE *bezerk_fetchstructure (MAILSTREAM *stream,unsigned long msgno,
				 BODY **body,long flags);
char *bezerk_fetchheader (MAILSTREAM *stream,unsigned long msgno,
			  STRINGLIST *lines,unsigned long *len,long flags);
char *bezerk_fetchtext (MAILSTREAM *stream,unsigned long msgno,
			unsigned long *len,long flags);
char *bezerk_fetchbody (MAILSTREAM *stream,unsigned long msgno,char *sec,
			unsigned long *len,long flags);
void bezerk_setflag (MAILSTREAM *stream,char *sequence,char *flag,long flags);
void bezerk_clearflag(MAILSTREAM *stream,char *sequence,char *flag,long flags);
long bezerk_ping (MAILSTREAM *stream);
void bezerk_check (MAILSTREAM *stream);
void bezerk_expunge (MAILSTREAM *stream);
long bezerk_copy(MAILSTREAM *stream,char *sequence,char *mailbox,long options);
long bezerk_append (MAILSTREAM *stream,char *mailbox,char *flags,char *date,
		    STRING *message);
long bezerk_append_putc (int fd,char *s,int *i,char c);
void bezerk_gc (MAILSTREAM *stream,long gcflags);

void bezerk_abort (MAILSTREAM *stream);
char *bezerk_file (char *dst,char *name);
int bezerk_lock (char *file,int flags,int mode,char *lock,int op);
void bezerk_unlock (int fd,MAILSTREAM *stream,char *lock);
int bezerk_parse (MAILSTREAM *stream,char *lock,int op);
char *bezerk_eom (char *som,char *sod,long i);
int bezerk_extend (MAILSTREAM *stream,int fd,char *error);
void bezerk_save (MAILSTREAM *stream,int fd);
char *bezerk_pseudo (MAILSTREAM *stream,char *hdr);
int bezerk_copy_messages (MAILSTREAM *stream,char *mailbox);
int bezerk_write_message (int fd,FILECACHE *m);
void bezerk_update_status (char *status,MESSAGECACHE *elt);

/* SUN imap4 */
char *bezerk_fabricate_from (MAILSTREAM *stream, long msgno);
int bezerk_set1flag (MAILSTREAM *stream,char *sequence,char *flag,long flags);
int bezerk_clear1flag(MAILSTREAM *stream,char *sequence,char *flag,long flags);
