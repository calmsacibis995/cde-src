/*
 * Program:	IMAP4 server
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	5 November 1990
 * Last Edited:	January 1997
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
 * MAJOR edits/extensions by William J. Yeager, SunSoft, Inc.
 * Copyright (c) 1995-1996 SunSoft, Inc.
 */

/* Parameter files */
#include <tnf/probe.h>
#include <sys/procfs.h>
#include <sys/time.h>		/* must be before osdep.h */
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <locale.h>
/* SUN Imap4: includes for socket io */
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include "mail.h"
#include "rfc822.h"
#include "os_sv5.h"
#include "misc.h"

extern int errno;		/* just in case */
#define TNF			/* For tnf performance testing */

/* Daemon files */
#define ALERTFILE "/etc/imapd.alert"
#define ANOFILE "/etc/anonymous.newsgroups"


/* Autologout timer */
#define TIMEOUT 60*30


/* Login tries */
#define LOGTRY 3


/* Length of literal stack (I hope 20 is plenty) */

#define LITSTKLEN 20


/* Size of temporary buffers */
#define TMPLEN (4*8192)			/* 12-july-96 WJY */


/* Server states */

#define LOGIN 0
#define SELECT 1
#define OPEN 2
#define LOGOUT 3

/* Global storage */
/* >= 8.1 is Sun version vel */
/* Edit History
 * 8.2.2     Added "Status: RO" to pseudo message
 *           29-jan-96  Bill Yeager
 * 8.2.3     Fixed RONLY open problem in solaris.c
 * 8.2.4     Added SCAN command and updated authentication code.
 *           13-Feb-96  Bill Yeager
 * 8.2.5     Updated rfc822 parsing.
 * 8.2.6     MailTool BUG. Made a change in rfc822.c to help. Jamais plus!
 * 8.2.7     Fix MailTool Attachment content-length bug 
 * 8.2.8     FT_INTERNAL for data
 *  -- Name change Solstice IMAP server: recyle version --
 * 0.2       New caching solaris driver with FT_INTERNAL
 * 0.5       SIMS -- indexing - mmap
 * 0.5.1     Debug tracing added.
 * 0.5.2     Fixed delete/rename for index files. 
 * 0.5.3     Fixed le probleme du baiser de mort(kiss of death)
 * 0.5.4     Bug with respect to mixing headers with Status goop.
 *           Heap no longer used in solaris_extent(tashbook)
 * 0.5.5     Rename imapd locks on expunge rename.
 *           Keep alive during expunge mmaps.
 *           fabricate from fixed(m->internal not viable)
 * 0.5.6     Clean up solaris_parse() code for rewrite.
 *           Fix bug with cycles size in solaric_checksum.c.
 *           Fix copy to message with NO pseudo-message bug
 *             Here, we do NOT copy an empty X-UID: header.
 *           SUNVERSION && copy: All flags but SEEN are passed.
 *           X-Dt-Delete-Time:  sets elt->deleted [dtmail truc]
 *           Fixed rfc822_size problem (added in deltas)
 *           Fixed ro and r/w access:
 *             ro sees no new mail and CAN read index on first pass.
 *             rw with rdonly(mult. processes) can see new mail(no expunge)
 *             rw if shared access cannot be gotten(SIGNAL refused/missed). Exit.
 * 0.5.7    +-Flags did not send checksum. Fixed. Not visible to Roam.
 *            Side-effect: fixed do_status_checksum() to send imap2bis in some
 *            places.
 * 0.5.8     rfc822 size bug fixed(do not include status merde)
 *           rfc822 MIME parsing fixed to correctly handle a part
 *           with 2 content-types.
 * 0.5.9     Error message if expunge fails creating tmp file
 * 0.6.0     Fix race condition for session lock in expunge.
 * 0.6.1     Fix V3 sun attach. without X-Sun-Content-Type: header field.
 *               See sun_mime.c.
 *           Change PERMANENTFLAGS message if none exist.
 *           Put keep alive timer in store flags
 *           Fix stupid V3 trans bug(MINE). Need body set.
 * 0.6.2     Prevent soft links with session locks.
 *           Added Crispins hitching post code to kill NFS username.lock race.
 *           Added code to NOOP lockf() for NFS mounted file systems.
 *           Parsing bug in solaris_eom (message from Alex to Mikel!!)
 * 1.0       Version change for FCS code freeze.
 *           o Fixed solaris_append to not prefix "From" with ">" if content-length found.
 *           o appended unique hw_id to tmp file for expunge(NFS uniqueness)     
 *           o If (empty message found), then a trivial one is generated.
 * 1.0       Protection on session lockfile always 0666.
 *           Make sure g+s bit works right for process access.
 *           Make sure access() is used for copy/append/rename
 *           Increased parse buffer to 64K for searches.
 * 1.0.1     Fixed crash if expunge attempt / MailTool started AFTER imapd.
 *           This causes no harm. solaris_parse() aborts if lock fails.
 * 1.0.2     solaris_create () full path did not work with access().
 *              Need to backtrack until path exists before checking access.
 *           solaris_readhdr() make sure the header ends in "\n\n".
 *           Under 2.4 a body of size 0 crashed the server. FIXED!!
 * 1.0.3     Increase cmdbuf to 32K (should be dynamic)
 * 1.0.4     LATT_MARKED added to list reply. See dummy.c.
 * 1.0.5     Search bug in mail.c fixed(mail_filter problem)
 * 1.0.6     &perr passed to solaris in solaris.c
 * 1.0.7     stream == NIL in group access calls fixed.
 *           error msg bug in mail_parse_flags()
 *           Large buffer opt. for HUGE attachments in solaris_parse.
 * 1.0.8     Fetch envelopes without body parse. 25-aout-96. WJY
 *           Fix shortinfo bug. Missing ")". cache content-type in solaris.c
 *           Fsync flags back on check only.
 * 1.0.9     VALID.h with improved algorithm (by lilly)
 *
 * 1.0.10    optimiztion of store flags. lock mved out of loop.
 * 1.0.11    Fixed \draft NOT sent back as flag.
 *           read/write index file entry as a block.
 * 1.0.12    Rewrote parsing loop. Full parses done in tmpfile now.
 *           Only pmap buffers rather than caching all.1
 *           Notification messages syntax fixes.
 * 1.0.13    NULL parsing for NFS files.
 *           BUG FIX(VERY OLD IN solaris.c) with seek_base for new msgs.
 *           do_status_checksum call passes stream now. Needed
 *               stream->dead check.
 *           NULL parsing on ANY file during open.
 *           Typo in solaris_parse() caused an error calling
 *             if (solaris_readmsg(..)). Should have been 
 *             if (!solaris_readmsg(..)) ...
 *           read_null_char bug: Needed to check for word boundary.
 *           NON-KEPT MIME translations:
 *             a) Header buffer was stepped on in rfc822.header.lines.
 *             b) Status merde was returned in header fetch.
 *             BOTH FIXED.
 * 1.0.14   fixed rfc822.header.lines in EG (flags rfc822.header.lines (to))
 *
 * 2.0      BETA release of 1.0.14
 * 2.0.1    BETA stream->rcvkor set if signal to change to readonly received
 *          Root login not allowed by MU_SERVER_LOGIN flag.
 * 2.0.2    BETA error msg update in solaris.c.
 * 2.0.3    BETA binary attachment byte count fixed
 *               add_count_lfs(..) fixed to ignore existing CRLF
 * 2.0.4    BETA "ln -s" bugs fixed, and mmapping of index file.
 * 2.0.5    BETA fix for 2.0.3 for binary parts. Bad data sent.
 *               STATUS command seg fault (no err msg) if mailbox 
 *               does not exist.
 * 2.0.6    BETA fix for access/modify time in solaris.c
 * 2.0.7    BETA *NP* for NIS+ login. log_sv5.c
 * 2.0      FCS  code freeze 9-dec-96.
 *          a) Added error number: "* OK [READ-ONLY] 128 Now READ-ONLY"
 * 2.0      Fixes for HUGE mailbox and expunge
 *          @ in version marks this change.
 *
 *
 * NOTE: 1.0.1 and 1.0.2 set back to 1.0 pour notre merveilleux stoppage
 *       de CD frabrication!! C'est une degueulasse. Bill. 13-juin-96
 *
 * [10-01-96 clin] strtok_r is used to ensure MT-Safe.
 * [10-21-96 clin] Merge of Bill's before 10/18/96 
 * [10-28-96 clin] Pass a mailstream in all mail_open calls.
 * [10-29-96 clin] Pass a mailstream in the following calls:
 *		mail_create, mail_subscribe, mail_unsubscribe,
 *		mail_delete, mail_rename, mail_lsub,mail_append_full	 
 *
 *	   	Call mail_stream_create in do_select --
 *	   	we can call it here or call it in mail_open
 *		but it seems to be simplier just to call it here.
 *
 * [10-30-96 clin] After a day-worth of work, I realized that
 *		it's not practical to declare one independent global
 *		data structure and passing it along. The difficulties
 *		surface when you have to pass a extra pointer in
 *		all the mm_XXX calls. These mm_XXX (i.e. mm_notify)
 *		are intertwined in many c-client library routines. 
 *		Passing one global data structure pointer means passing 
 *		it all over the place in the c-client library routines 
 *		which is a pain!  Therefore, the solution is to put the
 *		imapdvars on the stream.  The global is defined in mail.h.   
 *
 *		We should really call mail_stream_create in mail_open
 *		because we need to create a new stream sometime from
 *		mail_open.
 *		
 * [10-31-96 clin] It's OK to pass a NIL in mail_open now. 
 *
 * [11-05-96 clin] The correct thing to do is to define a global data
 *		structure and pass it alone to all the routines that
 *		need to access the imapdvars. For routines mm_xxx, we
 *		tag the imapdvars onto the stream prior calling the routine.
 *		This way we don't have to pass an extra pointer in those
 *		routines. Note the mm_xxx routines can be called in the
 *		c-client library. Thus pass an extra pointer is a bad idea. 
 *
 *		The imapdvars will be tagged onto the stream prior calling
 *		the mail_xxx calls which are the only routines referring
 *		to mm_xxx routines at their stack.
 *
 *		memcpy is used to accomplish this load/unload business.
 * 
 *		I'm thinking pass stream along the routines as well. On
 *		the stream, we have the imapdvars needed for mm_xxx calls.
 *
 *		imapd_global_init is defined in imapd.c.
 *
 * [11-06-96 clin] Most imapdvars are on the global data structure.
 *		imapdvars   used in routine
 *		-------	  -------------
 *		response: mm_log 
 *		lsterr:	  mm_log
 *		nmsgs:    mm_exists 
 *		recent:	  mm_exists
 *		pass:	  mm_login
 *		trycreate:  mm_notify
 *		finding:  mm_list_work
 *
 * [11-08-96 clin] Pass a Mailstream in mm_log, including
 *		c-client routines.
 * [11-09-96 clin] Put imapdvars->lsterr on imapdvars. Updated it if it's
 *		changed in mm_log.  The imapdvars are downloaded
 *		off the stream after they've been changed. 
 *		
 * [11-11-96 clin] Put litsp and litstk on the "imapdvars".
 *
 *		Because we have no way of telling when the
 *		imapdvars on the stream are referenced, we should
 *		upgrade them on the stream each time they're
 *		upgraded on the "imapdvars".
 *
 * [11-12-96 clin] Download the imapdvars from "stream" to "imapdvars"
 *		at the end of case statement. Notice that we only
 *		download the imapdvars from "stream" to "imapdvars" once,
 *		but upload the imapdvars from "imapdvars" to "stream"
 *		many times.
 *		
 *		Put response, cmd, arg, nmsgs on the "imapdvars".
 *		
 *		Define routine "up_load_imapdvars".		
 *
 *		Put sunversion_set on "imapdvars" - we shouldn't need
 *		to worry about up loading or down-loading the variable
 *		since it's only set once on the "imapdvars". Once it's
 *		set, it's true forever. Since we do a memcpy to down
 *		load all the imapdvars when creating the stream, this
 *		should be current as well on the stream.
 *		
 * [11-14-96 clin] Call down_load_imapdvars to load the imapdvars from
 *		iv->tstream to stream if mail_open is called.
 *	
 *		Put tstream on the stream since whenever tstream
 *		is accessed, a stream should always exists.
 *
 * [11-15-96 clin] We can't put stream on the "imapdvars" since
 *		we don't pass "imapdvars" in mm_xxx routines and
 *		these routines need to access the stream. Therefore,
 *		we declare the stream locally and independently and
 *		pass it along all the routines that need to access it.
 *		 
 *		We need to return stream in do_select because this
 *		is one routine that can flush the stream.
 *
 *		Pass two extra arguments in f[k] in fetch routine
 *		to avoid the core dumps.
 *
 * [11-18-96 clin] Remove one of the two stream parameters passed
 *		in mm_diskerror; one should be sufficient. Just be
 *		sure it's not a NIL.
 *
 * [11-19-96 clin] Add a new global - g_stream - a link to the local
 *		variable stream and used in server_traps routines.
 *		We can't pass anything in those routines. Thus we
 *		need these imapdvars, for now. 
 *
 *		Pass a stream in fatal because fatal calls mm_fatal
 *		which needs a stream. Do the same in all c-client
 *		library routines as well.
 *
 * [11-21-96 clin] Use iv->finding for stream. 
 *
 * [12-04-96 clin] Pass a mailstream in all mail_open calls.
 *		We can't pass a NIL in mail_open 'cause we need to
 *		access imapd_vars down the line.
 * 
 * [12-05-96 clin] Pass tstream in mail_list, mail_lsub, etc.
 *
 * [12-06-96 clin] Test stream_status after mail_open.
 *		Pass tstream in all mail_xxx calls. We need to
 *		do this to avoid the original stream being altered. 
 *
 * [12-20-96 clin] Call imapd_global_init here instead of in mail.c.
 *		Define imapd_vars on stream to be a "void" type.
 *		Cast it in each routine prior referring. 
 *
 * [01-02-97 clin] Add a new routine: imapd_var_free.
 *
 * [01-06-97 clin] Merge with Bill's L10N stuff.
 *		imap4rev1 added to imapd_vars. 
 * 
 * [01-07-97 clin] Call imapd_var_free before calling mail_close in
 *		do_select.
 *
 * [01-07-97 clin] Remove the last five parameters from mail_open.
 *
 * [01-08-97 clin] Use mail_stream_flush instead of mail_close on
 *		the dummy streams.  Also call imapd_var_free before
 *		all mail_close calls. 
 *
 * [01-09-97 clin] I re-structured the global hidding by doing the
 *		followings: Define one giant data structure "imapd_var"
 *		which holds all the globals. The pointer of imapdvar is
 *		initialized on the stack in main(). It's then being
 *		passed onto all subsequent routines. For the mm_xxx
 *		calls, we attach the pointer of this "imapdvar" onto
 *		userdata which is done during mail_stream_create.
 *		Because the pointer is never freed, variables on the
 *		structure can be updated freely. 
 *
 *		Call mail_close if the stream is opened; else call
 *		mail_stream_flush. 
 *
 *[01-10-97 clin] Initialize the flags variable in do_status_cmd.
 *
 *[01-13-97 clin] Test for stream before casting in mm_log.
 *
 *[01-14-97 clin] Implemented imapd_var_free.
 *
 *[01-15-97 clin] Check for stream_status in do_status_cmd. Check
 *		  for NIL stream in all mm_xxx calls.
 *
 *[01-16-97 clin] Fix lsub command. Use imapdvars instead of iv. 
 *		  Free append_buf at the end.
 *
 *[01-22-97 clin] More S_OPENED test added.
 *
 *[01-29-97 clin] Bill's fix: always set imapdvars->tstream to NIL.
 *
 */
/* version number of this server */
char *version = "Solstice (tm) Internet Mail Server (tm) - MT2.1";

typedef struct imapd_vars {

  struct driver *curdriver;    /*note current driver */
  time_t alerttime;            /* time of last alert */
  int state;                   /* server state */
  int mackludge;               /* MacMS kludge */
  int trycreate;               /* saw a trycreate */

  int finding;            /* doing old FIND command */
  int anonymous;          /* non-zero if anonymous */
  int logtry;             /* login tries */
  long kodcount;          /* set if KOD has happened already */
  long baiser;            /* used with la-dessous */

  long lire;              /* used with la-dessous */
  long rocount;           /* set if RONLY signal has happened twice */
  long nmsgs;             /* number of messages */
  long recent;            /* number of recent messages */
  char *host;             /* local host name */

  char *user;             /* user name */
  char *pass;             /* password */
  char *home;             /* home directory */
  char *cmdbuf;           /* command buffer */
  char *tag;              /* tag portion of command */

  char *cmd;               /* command portion of command */
  char *arg;               /* pointer to current argument of command */
  char *lsterr;            /* last error message from c-client */
  char *response;          /* command response */
  char *ronlystr;          /* for catching read-only signal */

  int litsp;               /* literal stack pointer */
  char *litstk[LITSTKLEN]; /* stack to hold literals */
  int sunversion_set;      /* True if received a sunversion command */
  int imap4_sunversion;    /* True if received IMAP4SUNVERSION  */
  int imap4rev1;     	   /* True if we are talking to a revision 1 client */
  int keep_mime;           /* True if we keep MIME translations */

  char *append_buf;        /* buffer for appends */
  int append_fd;           /* file descriptor for append */
  int append_len;          /* Length of mapped area */
  MAILSTREAM *tstream; 	   /* a tmp stream */
} IMAPD_VARS;

/* Still Globals: */
long baiser = 0;                /*used with above signal */
char *ronlystr = NIL;           /* for catching read-only signal */
MAILSTREAM *g_stream; 		/* Used in server_traps routines */

/* Response texts */

char *win = "%s OK %s completed\015\012";
char *altwin = "%s OK %s\015\012";
char *lose = "%s NO %s failed: %s\015\012";
char *losetry = "%s NO [TRYCREATE] %s failed: %s\015\012";
char *misarg = "%s BAD Missing required argument to %s\015\012";
char *badfnd = "%s BAD FIND option unrecognized: %s\015\012";
char *noques = "%s NO FIND %s with ? or %% wildcard not supported\015\012";
char *badarg = "%s BAD <Argument given to %s when none expected\015\012";
char *badseq = "%s BAD Bogus sequence in %s\015\012";
char *badatt = "%s BAD Bogus attribute list in %s\015\012";
char *badlit = "%s BAD Bogus literal count in %s\015\012";
char *toobig = "%s BAD Command line too long\015\012";
char *nulcmd = "* BAD Null command\015\012";
char *argrdy = "+ Ready for argument\015\012";

/* Function prototypes */
void main (int argc,char *argv[]);
void clkint ( int i);
void kodint ( int i);
void roint  ( int i);
void slurp (char *s,int n, IMAPD_VARS *imapdvars, MAILSTREAM *stream );
char inchar (IMAPD_VARS *imapdvars, MAILSTREAM *stream  );
void *flush (IMAPD_VARS *imapdvars, MAILSTREAM *stream );
char *snarf (char **arg, IMAPD_VARS *imapdvars, MAILSTREAM *stream );
char *snarf_list (char **arg, IMAPD_VARS *imapdvars, 
			MAILSTREAM *stream );
void new_flags (MAILSTREAM *s,char *tmp);
char *parse_astring (char **arg,unsigned long *i,char *del, 
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
STRINGLIST *parse_stringlist (char **s,int *list, 
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
long parse_criteria (SEARCHPGM *pgm,char **arg,unsigned long maxmsg,
     unsigned long maxuid, IMAPD_VARS *imapdvars, MAILSTREAM *stream );
long parse_criterion (SEARCHPGM *pgm,char **arg,unsigned long msgmsg,
 unsigned long maxuid,IMAPD_VARS *imapdvars, MAILSTREAM *stream );
long crit_date (unsigned short *date,char **arg);
long crit_date_work (unsigned short *date,char **arg);
long crit_set (SEARCHSET **set,char **arg,unsigned long maxima);
long crit_number (unsigned long *number,char **arg);
long crit_string (STRINGLIST **string,char **arg, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch (char *t,unsigned long uid,int reset_sendcksum,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_bodystructure (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_body (unsigned long i,STRINGLIST *a, 
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_body_part (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_body_part_peek (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_envelope (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_encoding (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void changed_flags (unsigned long i,int f,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_rfc822_header_lines (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_rfc822_header_lines_not (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_flags (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void put_flag (int *c,char *s);
void fetch_internaldate (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_uid (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_rfc822 (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_rfc822_peek (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_rfc822_header (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_rfc822_size (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_rfc822_text (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void fetch_rfc822_text_peek (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void penv (ENVELOPE *env, IMAPD_VARS *imapdvars);
void pbodystructure (BODY *body, IMAPD_VARS *imapdvars);
void pbody (BODY *body, IMAPD_VARS *imapdvars);
void pstring (char *s);
void paddr (ADDRESS *a);
long cstring (char *s);
long caddr (ADDRESS *a);
long nameok (char *ref,char *name, IMAPD_VARS *imapdvars);
char *imap_responder (void *challenge,unsigned long clen,
unsigned long *rlen, void *imapdvars, MAILSTREAM *stream );
void mm_list_work (char *what,char delimiter,char *name,long attributes,
		MAILSTREAM *stream);
/* SUN IMAP4 - 12 juin 95 WJY */
#define LOGURENTMERDE
void fetch_shortinfo(long i, char *s, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void sunversion (char **arg, IMAPD_VARS *imapdvars, 
	MAILSTREAM *stream );
void select_input (int infd, IMAPD_VARS *imapdvars,
		MAILSTREAM *stream);
int put_the_text (int fd, char *text, long n, int binary);
void log_urgent_info(char *str);
void open_urgent_info(void);
void valid_checksum(char *s, IMAPD_VARS *imapdvars,
		MAILSTREAM *stream);
void do_status_cmd(char *mailbox, char *args, char **lasts, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream );
int add_count_lfs(char *data,unsigned long len);
int get_body_encoding(char *section,BODY *body);

/* WJY: make some subroutines here to simplify the server code */
void *do_select (char *tmp, IMAPD_VARS *imapdvars, 
		MAILSTREAM *stream );
void do_append (IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void do_store (char *arg, int uid, IMAPD_VARS *imapdvars,
		MAILSTREAM *stream );
void fetch_1flag(char *cmd, char *sequence, char *flag, int uid,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream );
void do_status_checksum(MAILSTREAM *s,int format);
#define PCMDLEN 32
void cmd_copy(char *cmd,char *buf);
void enable_msacct(void);
void *global_init(IMAPD_VARS *imapdvars);
void up_load_imapdvars(IMAPD_VARS *imapdvars, MAILSTREAM *stream);
void down_load_imapdvars(MAILSTREAM *tstream, MAILSTREAM *stream);
void *imapd_var_init(); 
void imapd_var_free(IMAPD_VARS *imapdvars);

int	append_snarf(char **arg, char **name, IMAPD_VARS *, MAILSTREAM *);
int	fs_mumap(int fd, char *name, caddr_t buf, size_t len);
int	safe_write(int fd, char *buf, long nbytes);

int	received_urgent_data (int infd, int outfd);
int	reply_to_urgent(int inFd, int outFd);

#define FORMATIMAP4STATUS 1
#define FORMAT2BIS        2

int checksum_format = FORMATIMAP4STATUS;
/* Sun Imap4 end */

/* Main program */
void main (int argc,char *argv[])
{
  unsigned long i,uid;
  char *s,*t,*u,*v,tmp[MAILTMPLEN];
  struct stat sbuf;
  hrtime_t r0,t0;
  longlong_t r1,t1;
  char probe_cmd[PCMDLEN+1], *lasts;
  unsigned short stream_flag;
  IMAPD_VARS *imapdvars;
  MAILSTREAM *stream = NIL;	 /* mailbox stream */
  IMAPD_VARS *iv;  	 	 /* pointer to imapd_vars */
  int default_sig = 0;

#include "linkage.c"
  /* for our locale for gettext calls - use system locale - 24-dec-96 */
  setlocale(LC_MESSAGES,NIL);

  /* for timing stuff */
  enable_msacct();

  imapdvars = imapd_var_init();     /* Initialize imapd variables. */
     /* Create a stream first and pass the variable pointer onto the stream. */
  stream = mail_stream_create((void*)imapdvars,NIL,NIL,NIL,NIL); 

  openlog ("imapd",LOG_PID,LOG_MAIL);
  imapdvars->host = cpystr (mylocalhost(stream));/* get local host name */
  rfc822_date (imapdvars->cmdbuf);		/* get date/time now */
  i = MU_NOTLOGGEDIN;
  s = myusername_full (&i,stream);	/* get user name and flags */
  switch (i) {
  case MU_NOTLOGGEDIN:
    t = "OK";			/* not logged in, ordinary startup */
    break;
  case MU_ANONYMOUS:
    imapdvars->anonymous = T;	/* anonymous user, fall into default */
  case MU_LOGGEDIN:
    t = "PREAUTH";		/* already logged in, pre-authorized */
    imapdvars->user = cpystr (s);		/* copy user name */
    imapdvars->pass = cpystr ("*");	/* set fake password */
    imapdvars->state= SELECT;		/* enter select state */
    syslog (LOG_INFO,gettext("Preauthenticated user=%.80s host=%.80s"),
	    imapdvars->anonymous ? gettext("anonymous") : imapdvars->user,
	    tcp_clienthost (tmp));
    break;
  default:
    fatal ("Unknown state from myusername_full()", stream);
  }
  /* SUN imap4 Allocate the initial append buffer WJY 30-sep-94 */
  imapdvars->append_buf = NIL;

  printf ("* %s %s %s - at %s\015\012",
	  t,imapdvars->host,version,imapdvars->cmdbuf);
  fflush (stdout);		/* dump output buffer */
  server_traps (clkint, kodint,roint);/* set up traps */
  do {				/* command processing loop */
	/* Stream should never be NIL and should carry the most
	current imapdvars.  -clin */
     if (!stream ) 
	stream = mail_stream_create((void *)imapdvars,NIL,NIL,NIL,NIL);
    g_stream = stream;  
	/* g_stream - a global is used in server_traps routines. -clin */
    slurp (imapdvars->cmdbuf,TMPLEN-1, imapdvars, stream);	/* slurp command */
				/* no more last error or literal */
    if (imapdvars->lsterr) fs_give ((void **) &imapdvars->lsterr);

    while (imapdvars->litsp) 
	fs_give ((void **) &imapdvars->litstk[--imapdvars->litsp]);
    cmd_copy(imapdvars->cmdbuf,probe_cmd);  /* duplicate for tnf probes */
				/* find end of line */
    if (!strchr (imapdvars->cmdbuf,'\012')) {
      if (t = strchr (imapdvars->cmdbuf,' ')) *t = '\0';
      flush (imapdvars, stream);	/* flush excess */
      printf (imapdvars->response,t ? imapdvars->cmdbuf : "*");
    }
    else if (!(imapdvars->tag = strtok_r(imapdvars->cmdbuf," \015\012", &lasts))) 
		fputs (nulcmd,stdout);
    else if (!(imapdvars->cmd = strtok_r (NIL," \015\012", &lasts)))
      printf ("%s BAD Missing command\015\012",imapdvars->tag);
    else {			/* parse command */
      imapdvars->response = win;
		/* set default imapdvars->response */
      imapdvars->finding  = NIL;
	/* no longer FINDING */
      ucase (imapdvars->cmd);		/* canonicalize command case */
      /* TRACE start */
      /* PROBE macros for performance */
      TNF_PROBE_0(cmd_start,"imap4","");

      t0 = gethrvtime();	/* cpu time */
      r0 = gethrtime();		/* wall time */
				/* UID command? */
      if (!strcmp (imapdvars->cmd,"UID") && strtok_r (NIL," \015\012",&lasts)) {
	uid = T;		/* a UID command */
	imapdvars->cmd[3] = ' ';		/* restore the space delimiter */
	ucase (imapdvars->cmd);		/* make sure command all uppercase */
      }
      else uid = NIL;		/* not a UID command */
				/* snarf argument */
      imapdvars->arg = strtok_r (NIL,"\015\012",&lasts);
				/* LOGOUT command always valid */
      if (!strcmp (imapdvars->cmd,"LOGOUT")) {
	if (imapdvars->state == OPEN)  
	mail_close (stream)
	else mail_stream_flush(stream); /* still need to flush it. -clin*/
	stream = NIL;
	printf("* BYE %s IMAP4 server terminating connection\015\012",imapdvars->host);
	imapdvars->state = LOGOUT;
      }
				/* kludge for MacMS */
      else if ((!strcmp (imapdvars->cmd,"VERSION")) && 
	(s = snarf (&imapdvars->arg,imapdvars, stream)) && (!imapdvars->arg) &&
	       (i = atol (s)) && (i == 4)) {
	imapdvars->mackludge = T;
	fputs ("* OK [MacMS] The MacMS kludges are enabled\015\012",stdout);
      }
      else if (!strcmp (imapdvars->cmd,"CAPABILITY")) {
	AUTHENTICATOR *auth;
	/* WJY -- Added SUNVERSION -- 12 June 95 */
	fputs ("* CAPABILITY IMAP4 STATUS SCAN IMAP4SUNVERSION SUNVERSION",
	       stdout);
	for (auth = authenticators; auth; auth = auth->next)
	  printf (" AUTH-%s",auth->name);
	fputs ("\015\012",stdout);
      }

				/* dispatch depending upon state */
      else if (strcmp (imapdvars->cmd,"NOOP")) switch (imapdvars->state) {
      case LOGIN:		/* waiting to get logged in */
				/* new style authentication */
	if (!strcmp (imapdvars->cmd,"AUTHENTICATE")) {
	  fs_give ((void **) &imapdvars->user);
	  fs_give ((void **) &imapdvars->pass);
				/* single argument */
	  if (!(s = snarf (&imapdvars->arg, imapdvars, stream)))  
		imapdvars->response = misarg;
	  else if (imapdvars->arg) 
		imapdvars->response = badarg;
	  else if (imapdvars->user = 
	cpystr (mail_auth (s,imap_responder,argc,argv,stream))) {
	    imapdvars->state = SELECT;
	    syslog (LOG_INFO,gettext("Authenticated user=%s host=%s"),
		    imapdvars->user,tcp_clienthost (tmp));
	  }
	  else {
	    imapdvars->response = "%s NO Authentication failure\015\012";
	    sleep (3);		/* slow the cracker down */
	    if (!--imapdvars->logtry) {
	      fputs ("* BYE Too many authentication failures\015\012",stdout);
	      imapdvars->state = LOGOUT;
	      syslog (LOG_INFO,
		      gettext("Excessive authentication failures host=%s"),
		      tcp_clienthost (tmp));
	    }
	    else syslog (LOG_INFO,
			 gettext("Authentication failure host=%s"),
			 tcp_clienthost (tmp));
	  }
	}

				/* plaintext login with password */
	else if (!strcmp (imapdvars->cmd,"LOGIN")) {
	  fs_give ((void **) &imapdvars->user);
	  fs_give ((void **) &imapdvars->pass);
				/* two arguments */
	  if (!((imapdvars->user = cpystr (snarf (&imapdvars->arg, imapdvars,stream))) &&
		(imapdvars->pass = cpystr (snarf (&imapdvars->arg, imapdvars,stream))))) 
		imapdvars->response = misarg;
	  else if (imapdvars->arg)  imapdvars->response = badarg;
			/* see if username and password are OK */
	  else if (server_login (imapdvars->user,imapdvars->pass,argc,argv,stream)) {
	    imapdvars->state = SELECT;
	    syslog (LOG_INFO,
		    gettext("Login user=%s host=%s"),imapdvars->user,
		    tcp_clienthost(tmp));
	  }
				/* nope, see if we allow anonymous */
	  else if (!stat (ANOFILE,&sbuf) && !strcmp (imapdvars->user,"anonymous") &&
		   anonymous_login()) {
	    imapdvars->anonymous = T;	/* note we are anonymous, login as nobody */
	    myusername(stream);	/* give the environment a kick */
	    imapdvars->state = SELECT;	/* make selected */
	    syslog (LOG_INFO,
		    gettext("Login anonymous=%s host=%s"),
                    imapdvars->pass,
		    tcp_clienthost(tmp));
	  }
	  else {
	 imapdvars->response = 
		"%s NO Bad %s user name and/or password\015\012";
	    sleep (3);		/* slow the cracker down */
	    if (!--imapdvars->logtry) {
	      fputs ("* BYE Too many login failures\015\012",stdout);
	      imapdvars->state = LOGOUT;
	      syslog (LOG_INFO,
		      gettext("Excessive login failures user=%s host=%s"),
		      imapdvars->user,tcp_clienthost (tmp));
	    }
	    else syslog (LOG_INFO,
			 gettext("Login failure user=%s host=%s"),
			 imapdvars->user,tcp_clienthost (tmp));
	  }
	} else if  (!strcmp (imapdvars->cmd,"SUNVERSION")) {
	  /* SUN IMAP4 - 12 juin 95 */
	  imapdvars->imap4_sunversion = NIL;
	  sunversion(&imapdvars->arg, imapdvars,stream);
	} else if (!strcmp (imapdvars->cmd, "IMAP4SUNVERSION")) {
	  imapdvars->imap4_sunversion = T;
	  /* Note that imapdvars->imap4_sunversion ==> sunversion but NOT
	   * conversely */
	  sunversion(&imapdvars->arg, imapdvars,stream);
	}
	else 
	  imapdvars->response = 
		"%s BAD Command unrecognized/login please: %s\015\012";
	break;

      case OPEN:		/* valid only when mailbox open */
				/* fetch mailbox attributes */
	if (!strcmp (imapdvars->cmd,"FETCH") || !strcmp (imapdvars->cmd,"UID FETCH")) {
	  if (!(imapdvars->arg && (s = strtok_r (imapdvars->arg," ", &lasts)) && 
		(t = strtok_r(NIL,"\015\012", &lasts)))) 
	    imapdvars->response = misarg;
	  else if (uid ? mail_uid_sequence (stream,s) :
	   mail_sequence (stream,s)) fetch (t,uid,T, imapdvars,stream);
	  else imapdvars->response = badseq;
	}
				/* fetch partial mailbox attributes */
	else if (!strcmp (imapdvars->cmd,"PARTIAL")) {
	  unsigned long msgno,start,count,size,txtsize;
	  /* SUN Imap4 status checksum */
	  stream->send_checksum = NIL;
	  if (!(imapdvars->arg && (msgno = strtol (imapdvars->arg,&s,10)) && 
		(t = strtok_r (s," ", &lasts)) &&
		(s = strtok_r (NIL,"\015\012", &lasts)) && 
		(start = strtol (s,&s,10)) &&
		(count = strtol (s,&s,10)))) 
		imapdvars->response = misarg;
	  else if (s && *s) imapdvars->response = badarg;
	  else if (msgno > stream->nmsgs) imapdvars->response = badseq;
	  else {		/* looks good */
	    int f = mail_elt (stream,msgno)->seen;
	    u = s = NIL;	/* no strings yet */
	    if (!strcmp (ucase (t),"RFC822")) {
				/* have to make a temporary buffer for this */
	      mail_fetchtext_full (stream,msgno,&txtsize,NIL);
	      v = mail_fetchheader_full (stream,msgno,NIL,&size,NIL);
	      s = u = (char *) fs_get (size + txtsize + 1);
	      u[size + txtsize] = '\0';
	      memcpy (u,v,size);
	      v = mail_fetchtext_full (stream,msgno,&txtsize,NIL);
	      memcpy (u + size,v,txtsize);
	    }
	    else if (!strcmp (ucase (t),"RFC822.PEEK")) {
	      mail_fetchtext_full (stream,msgno,&txtsize,FT_PEEK);
	      v = mail_fetchheader_full (stream,msgno,NIL,&size,NIL);
	      s = u = (char *) fs_get (size + txtsize + 1);
	      u[size + txtsize] = '\0';
	      memcpy (u,v,size);
	      v = mail_fetchtext_full (stream,msgno,&txtsize,FT_PEEK);
	      memcpy (u + size,v,txtsize);
	    }
	    else if (!strcmp (t,"RFC822.HEADER"))
	      s = mail_fetchheader_full (stream,msgno,NIL,&size,NIL);
	    else if (!strcmp (t,"RFC822.TEXT"))
	      s = mail_fetchtext_full (stream,msgno,&size,NIL);
	    else if (!strcmp (t,"RFC822.TEXT.PEEK"))
	      s = mail_fetchtext_full (stream,msgno,&size,FT_PEEK);
	    else if (*t == 'B' && t[1] == 'O' && t[2] == 'D' && t[3] == 'Y') {
	      if (t[4] == '[' && (v = strchr (t += 5,']')) && !v[1]) {
		*v = '\0';	/* tie off body part */
		s = mail_fetchbody (stream,msgno,t,&size);
	      }
	      else if (t[4] == '.' && t[5] == 'P' && t[6] == 'E' &&
		       t[7] == 'E' && t[8] == 'K' && t[9] == '[' &&
		       (v = strchr (t += 10,']')) && !v[1]) {
		*v = '\0';	/* tie off body part */
		s = mail_fetchbody_full (stream,msgno,t,&size,FT_PEEK);
	      }
	    }
	    /* Free the cached body */
	    mail_free_cached_buffers_msg(stream,msgno);
	    if (s) {		/* got a string back? */
	      if (size <= --start) s = NIL;
	      else {		/* have string we can make smaller */
		s += start;	/* this is the first byte */
				/* tie off as appropriate */
		if (count < (size - start)) s[count] = '\0';
	      }
	      printf ("* %lu FETCH (%s ",msgno,t);
	      pstring (s);	/* write the string */
	      changed_flags (msgno,f, imapdvars, stream);
	      fputs (")\015\012",stdout);
	      if (u) fs_give ((void **) &u);
	      /* SUN Imap4[PARTIAL] which is dead in REV1 */
	      do_status_checksum(stream,FORMATIMAP4STATUS);
	    }
	    else imapdvars->response = badatt;
	  }
	}

				/* store mailbox attributes */
	else if (!strcmp (imapdvars->cmd,"STORE") || 
		!strcmp (imapdvars->cmd,"UID STORE")) {
	  do_store (imapdvars->arg, uid, imapdvars, stream);
	}
				/* check for new mail */
	else if (!strcmp (imapdvars->cmd,"CHECK")) {
				/* no arguments */
	if (imapdvars->arg) 
	imapdvars->response = badarg;
	  else if (imapdvars->anonymous) mail_ping (stream);
	  else {
	    int format;
	    /* SUN Imap4 status ... */
	    mail_check (stream);
	    format = (imapdvars->imap4_sunversion ? 
			FORMATIMAP4STATUS : FORMAT2BIS);
	    do_status_checksum(stream,format);
	  }
	}
	/* Sun imap4 */
	else if (!strcmp (imapdvars->cmd, "VALIDCHECKSUM")) {
	  if (!imapdvars->arg) imapdvars->response = misarg;
	  else valid_checksum(imapdvars->arg, imapdvars,stream);
	} 
	else if (!strcmp (imapdvars->cmd, "CHECKSUM")) {
	  /* CHECKSUM used by imap2 clients only */
	  if (imapdvars->arg) imapdvars->response = badarg;
	  else do_status_checksum(stream, FORMAT2BIS);
	}
				/* expunge deleted messages */
	else if (!(imapdvars->anonymous || strcmp (imapdvars->cmd,"EXPUNGE"))) {
				/* no arguments */
	  if (imapdvars->arg) imapdvars->response = badarg;
	  else {
	    int format;
	    mail_expunge (stream);
	    /* Sun Imap4: send checksum */
	    format = (imapdvars->imap4_sunversion ? 
		FORMATIMAP4STATUS : FORMAT2BIS);
	    do_status_checksum(stream,format);
	  }
	}
	else if (!imapdvars->anonymous &&	/* copy message(s) */
		 (!strcmp (imapdvars->cmd,"COPY") || 
		!strcmp (imapdvars->cmd,"UID COPY"))) {
	  	imapdvars->trycreate = NIL;   
		/* no trycreate status */
	  if (!((s = snarf (&imapdvars->arg, imapdvars,stream)) && 
		(t = snarf (&imapdvars->arg, imapdvars,stream)))) 
		imapdvars->response = misarg;
	  else if (imapdvars->arg) imapdvars->response = badarg;
	  else if (imapdvars->mackludge) {	
		/* MacMS wants implicit create */
	    if (!(mail_copy_full (stream,s,t,uid ? CP_UID : NIL) ||
		  (imapdvars->trycreate && mail_create (stream,t) &&
		   mail_copy_full (stream,s,t,uid ? CP_UID : NIL)))) {
	      imapdvars->response = lose;
	      if (!imapdvars->lsterr) 
		imapdvars->lsterr = cpystr ("No such destination mailbox");
	    }
	  }
	  else if (!mail_copy_full (stream,s,t,uid ? CP_UID : NIL)) {
	    imapdvars->response = imapdvars->trycreate ? losetry : lose;
	    if (!imapdvars->lsterr) imapdvars->lsterr 
		= cpystr ("No such destination mailbox");
	  } else {		/* Sun Imap4 */
	    int format;
	    format = (imapdvars->imap4_sunversion ? 
		FORMATIMAP4STATUS : FORMAT2BIS);
	    do_status_checksum(stream,format);
	  }
	}
				/* close mailbox */
	else if (!strcmp (imapdvars->cmd,"CLOSE")) {
				/* no arguments */
	  if (imapdvars->arg) imapdvars->response = badarg;
	  else {
	    stream = mail_close_full(stream,imapdvars->anonymous ?
			 NIL : CL_EXPUNGE);
	    imapdvars->state = SELECT;	/* no longer opened */
	  }
	}


				/* search mailbox */
	else if (!strcmp (imapdvars->cmd,"SEARCH") || 
			!strcmp (imapdvars->cmd,"UID SEARCH")) {
	  char *charset = NIL;
	  SEARCHPGM *pgm;
				/* one or more arguments required */
	  if (!imapdvars->arg) 
		imapdvars->response = misarg;
				/* character set specified? */
	  else if ((imapdvars->arg[0] == 'C' || imapdvars->arg[0] == 'c') &&
		   (imapdvars->arg[1] == 'H' || imapdvars->arg[1] == 'h') &&
		   (imapdvars->arg[2] == 'A' || imapdvars->arg[2] == 'a') &&
		   (imapdvars->arg[3] == 'R' || imapdvars->arg[3] == 'r') &&
		   (imapdvars->arg[4] == 'S' || imapdvars->arg[4] == 's') &&
		   (imapdvars->arg[5] == 'E' || imapdvars->arg[5] == 'e') &&
		   (imapdvars->arg[6] == 'T' || imapdvars->arg[6] == 't') &&
		   (imapdvars->arg[7] == ' ' || imapdvars->arg[7] == ' ')) {
	    imapdvars->arg += 8;		/* yes, skip over CHARSET token */
	    if (s = snarf (&imapdvars->arg, imapdvars,stream)) charset = cpystr (s);
	    else {		/* missing character set */
	      imapdvars->response = misarg;
	      break;
	    }
	  }
				/* must have arguments here */
	  if (!(imapdvars->arg && *imapdvars->arg)) 
		imapdvars->response = misarg;
				/* parse criteria */
	  else if (parse_criteria (pgm = mail_newsearchpgm (),&imapdvars->arg,
				   stream->nmsgs,stream->nmsgs ?
   		mail_uid (stream,stream->nmsgs) : 0, imapdvars,stream ) && 
		   !*imapdvars->arg) {
	    mail_search_full (stream,charset,pgm,SE_FREE|SE_SERVER);
	    if (charset) fs_give ((void **) &charset);
	    if (imapdvars->response == win || imapdvars->response == altwin) {
				/* output search results */
	      fputs ("* SEARCH",stdout);
	      for (i = 1; i <= imapdvars->nmsgs; ++i)
		if (mail_elt (stream,i)->searched)
		  printf (" %lu",uid ? mail_uid (stream,i) : i);
	      fputs ("\015\012",stdout);
	      /* Free any cached data that may have been accumulated */
	      for (i = 1; i <= imapdvars->nmsgs; ++i) 
		mail_free_cached_buffers_msg(stream,i);
	    }
	  }
	  else {
	  imapdvars->response = "%s BAD Bogus criteria list in %s\015\012";
	    mail_free_searchpgm (&pgm);
	  }
	}
	else			/* fall into select case */

      case SELECT:		/* valid whenever logged in */
				/* select new mailbox */
	if (!(strcmp (imapdvars->cmd,"SELECT") && strcmp (imapdvars->cmd,"EXAMINE"))) {
          stream = (MAILSTREAM *)do_select(tmp, imapdvars, stream);
	}
				/* APPEND message to mailbox */
	else if (!(imapdvars->anonymous || strcmp (imapdvars->cmd,"APPEND"))) {
	  do_append(imapdvars,stream);
	} 
	else if (!strcmp (imapdvars->cmd, "SUNVERSION")) {
	  /* SUN IMAP4 - 12 juin 95 */
	  imapdvars->imap4_sunversion = NIL;
	  sunversion(&imapdvars->arg, imapdvars,stream);
	} else if (!strcmp (imapdvars->cmd, "IMAP4SUNVERSION")) {
	  imapdvars->imap4_sunversion = T;
	  sunversion(&imapdvars->arg, imapdvars,stream);
	}

				/* list mailboxes */
	else if (!strcmp (imapdvars->cmd,"LIST")) {
				/* get reference and mailbox argument */
	  if (!((s = snarf (&imapdvars->arg, imapdvars,stream)) && 
		(t = snarf_list (&imapdvars->arg, imapdvars,stream)))) 
	    imapdvars->response = misarg;
	  else if (imapdvars->arg) 
	    imapdvars->response = badarg;
			/* make sure imapdvars->anonymous can't do bad things */
	  else if (nameok (s,t, imapdvars)) {
				/* do the list */
	    /* Here we create a tmp stream and pass on the pointer of the imapdvars.
	    Then we pass the tmpstream to the mail_xxx calls. The imapd variables on
	    imapdvars that's being passed with the tmpstream can be updated by the
	    mm_xxx calls. Note that we flush the tmpstream after we're done. But we
	    don't touch the imapd variables on imapdvars. -clin*/  
	    if ( *s == '{' ) 
	    imapdvars->tstream = 
		mail_stream_create((void *)imapdvars,NIL,NIL,NIL,NIL);
	    imapdvars->tstream = (*s == '{') ? 
	    mail_open (imapdvars->tstream,s, OP_HALFOPEN|OP_SERVEROPEN) : stream;

	    mail_list (imapdvars->tstream,s,t);
	/* We'll flush the temporary stream later at the end of the loop -clin*/
	  }
	}
				/* scan mailboxes */
	else if (!strcmp (imapdvars->cmd,"SCAN")) {
				/* get arguments */
	  if (!((s = snarf (&imapdvars->arg,imapdvars,stream)) && 
		(t = snarf_list (&imapdvars->arg,imapdvars,stream)) &&
		(u = snarf (&imapdvars->arg,imapdvars,stream)))) 
		imapdvars->response = misarg;
	  else if (imapdvars->arg) 
		imapdvars->response = badarg;
			/* make sure imapdvars->anonymous can't do bad things */
	  else if (nameok (s,t, imapdvars)) {
				/* do the list */
	    if ( *s == '{' ) 
            imapdvars->tstream = 
		mail_stream_create((void *)&imapdvars,NIL,NIL,NIL,NIL);

	    imapdvars->tstream = (*s == '{') ? 
	    mail_open (imapdvars->tstream,s, 
		OP_HALFOPEN|OP_SERVEROPEN) : stream;
	    mail_scan (imapdvars->tstream,s,t,u);
	/* We'll flush the temporary stream later at the end of the loop -clin*/

	  }
	}
				/* list subscribed mailboxes */
	else if (!strcmp (imapdvars->cmd,"LSUB")) {
				/* get reference and mailbox argument */
	  if (!((s = snarf (&imapdvars->arg,imapdvars,stream)) && 
		(t = snarf_list (&imapdvars->arg,imapdvars,stream))))	
	    imapdvars->response = misarg;
	      else if (imapdvars->arg) 
	    imapdvars->response = badarg;
			/* make sure imapdvars->anonymous can't do bad things */
	  else if (nameok (s,t, imapdvars)) {
            imapdvars->tstream = 
	    mail_stream_create((void *)imapdvars,NIL,NIL,NIL,NIL);

	    mail_lsub (imapdvars->tstream,s,t);/* do the lsub */

	    if ( *s == '{' )
		imapdvars->tstream=mail_open (imapdvars->tstream,s,
		OP_HALFOPEN|OP_SERVEROPEN);
	    else {
		mail_close(imapdvars->tstream);
		imapdvars->tstream = stream;
		}
		if (imapdvars->tstream->stream_status & S_OPENED)
		 mail_lsub (imapdvars->tstream,s,t);
	  }
	}

				/* find mailboxes */
	else if (!strcmp (imapdvars->cmd,"FIND")) {
	  imapdvars->response =  "%s OK FIND %s completed\015\012";
				/* get subcommand and true argument */
	  if (!(imapdvars->arg && 
		(s = strtok_r (imapdvars->arg," \015\012",&lasts)) && 
			(imapdvars->cmd = ucase (s)) &&
		(imapdvars->arg = strtok_r(NIL,"\015\012", &lasts)) && 
		(s = snarf_list (&imapdvars->arg,imapdvars,stream)))) 
	    		imapdvars->response = misarg;	
		/* missing required argument */
	  else if (imapdvars->arg) 
		imapdvars->response = badarg;
				/* punt on single-char wildcards */
	  else if (strpbrk (s,"%?")) 
		imapdvars->response = noques;
	  else if (nameok (NIL,s, imapdvars)) {
	    imapdvars->finding = T;	
				/* note that we are finding */
	    for (t = s; *t; t++) if (*t == '*') *t = '%';
				/* dispatch based on type */
	    if (!strcmp (imapdvars->cmd,"MAILBOXES")) {
	      if (!imapdvars->anonymous) {      
		mail_lsub (stream,NIL,s);

     		if ( *s == '{' ) 
            	imapdvars->tstream=
		mail_stream_create((void *)imapdvars,NIL,NIL,NIL,NIL);

		/* imapdvars->tstream will always be there 
		thus we test for stream_status. -clin */

		if ((*s == '{') && 
		    (imapdvars->tstream = mail_open 
		(imapdvars->tstream ,s,OP_HALFOPEN|OP_SERVEROPEN)) && 
		(imapdvars->tstream->stream_status & S_OPENED)) {
		  mail_lsub (imapdvars->tstream,NIL,s);
		}	

	      }
	    }
	    else if (!strcmp (imapdvars->cmd,"ALL.MAILBOXES")) {
	      mail_list (stream,NIL,s);

     		if ( *s == '{' ) 
            	imapdvars->tstream = 
		mail_stream_create((void *)imapdvars,NIL,NIL,NIL,NIL);

	        if ((*s == '{') && 
		  (imapdvars->tstream = mail_open 
		(imapdvars->tstream,s,OP_HALFOPEN|OP_SERVEROPEN)) && 
		(imapdvars->tstream->stream_status & S_OPENED))
		mail_list (imapdvars->tstream,NIL,s);
	    }
	    else 
		imapdvars->response = badfnd;
			/* flush the temporary stream later. -clin */
	  }
	}

			/* status of mailbox */
	else if (!strcmp (imapdvars->cmd,"STATUS")) {
	  if (!((s = snarf (&imapdvars->arg,imapdvars,stream)) && 
		imapdvars->arg && (*imapdvars->arg++ == '(') &&
		(t = strchr (imapdvars->arg,')')) && (t - imapdvars->arg) && !t[1]))
		imapdvars->response =  misarg;
	  else {
				/* Tie off string */
	    *t = '\0';
				/* First argument */
	    t = strtok_r (ucase (imapdvars->arg)," ", &lasts);
	    /* execute the command  */
	    do_status_cmd(s, t, &lasts, imapdvars,stream);
	  }
	}

				/* subscribe to mailbox */
	else if (!(imapdvars->anonymous || strcmp (imapdvars->cmd,"SUBSCRIBE"))) {
				/* get <mailbox> or MAILBOX <mailbox> */
	  if (!(s = snarf (&imapdvars->arg,imapdvars,stream))) 
		imapdvars->response = misarg;
	  else if (imapdvars->arg && strcmp (ucase (strcpy (tmp,s)),"MAILBOX")) 
	    	imapdvars->response = badarg;
	  else if (imapdvars->arg && !(s = snarf (&imapdvars->arg,imapdvars,stream))) 
		imapdvars->response = misarg;
	  else if (imapdvars->arg) 
		imapdvars->response = badarg;
	  else {

		imapdvars->tstream = 
		mail_stream_create((void *)imapdvars,NIL,NIL,NIL,NIL);
                mail_subscribe (imapdvars->tstream,s);
               }
	}
				/* unsubscribe to mailbox */
	else if (!(imapdvars->anonymous || strcmp (imapdvars->cmd,"UNSUBSCRIBE"))) {
				/* get <mailbox> or MAILBOX <mailbox> */
	  if (!(s = snarf (&imapdvars->arg,imapdvars,stream))) 
		imapdvars->response = misarg;
	  else if (imapdvars->arg && strcmp (ucase (strcpy (tmp,s)),"MAILBOX")) 
	    	imapdvars->response = badarg;
	  else if (imapdvars->arg && !(s = snarf (&imapdvars->arg,imapdvars,stream))) 
		imapdvars->response = misarg;
	  else if (imapdvars->arg) 
		imapdvars->response = badarg;
	  else  {

		imapdvars->tstream = 
		mail_stream_create((void *)imapdvars,NIL,NIL,NIL,NIL);
                mail_unsubscribe (imapdvars->tstream ,s);
                }
	}
				/* create mailbox */
	else if (!(imapdvars->anonymous || strcmp (imapdvars->cmd,"CREATE"))) {
	  if (!(s = snarf (&imapdvars->arg,imapdvars,stream))) 
		imapdvars->response = misarg;
	  else if (imapdvars->arg) 
		imapdvars->response = badarg;
	  else 
		{
		imapdvars->tstream = 
		mail_stream_create((void *)imapdvars,NIL,NIL,NIL,NIL);
		mail_create (imapdvars->tstream,s);
		}
	}
				/* delete mailbox */
	else if (!(imapdvars->anonymous || strcmp (imapdvars->cmd,"DELETE"))) {
	  if (!(s = snarf (&imapdvars->arg,imapdvars,stream))) 
		imapdvars->response = misarg;
	  else if (imapdvars->arg) 
		imapdvars->response = badarg;
	  else {

		imapdvars->tstream = 
		mail_stream_create((void *)imapdvars,NIL,NIL,NIL,NIL);
                mail_delete (imapdvars->tstream,s);
                }
	}
				/* rename mailbox */
	else if (!(imapdvars->anonymous || strcmp (imapdvars->cmd,"RENAME"))) {
	  if (!((s = snarf (&imapdvars->arg,imapdvars,stream )) && 
		(t = snarf (&imapdvars->arg,imapdvars,stream )))) 
		imapdvars->response = misarg;
	  else if (imapdvars->arg) 
		imapdvars->response = badarg;
	  else {
		imapdvars->tstream = 
		mail_stream_create((void *)imapdvars,NIL,NIL,NIL,NIL);
                mail_rename (imapdvars->tstream,s, t);
                }
	}

	else 
	  imapdvars->response = "%s BAD Command unrecognized: %s\015\012";
	break;
      default:
        imapdvars->response = 
		"%s BAD Server in unknown state for %s command\015\012";
	break;
      }

	/* We close or flush tstream IFF tstream != stream -clin*/ 
 	if (stream && imapdvars->tstream && 
		(stream != imapdvars->tstream )) {
	  if ( imapdvars->tstream->stream_status & S_OPENED ) 
 	  mail_close(imapdvars->tstream)
 	  else mail_stream_flush(imapdvars->tstream);
	}
	/* Either way, tstream should be set to NIL by now. -clin*/
	  imapdvars->tstream = NIL;

      if (imapdvars->state== OPEN) {	/* mailbox open? */
	char *saveresponse = imapdvars->response;
	char *savelsterr = imapdvars->lsterr;

	/* SOME DEBUGGING */
	if (stream->lock) {
	  sprintf(tmp,"Stream locked after %s",imapdvars->cmd);
	  mm_log(tmp,WARN, stream);
	}
				/* make sure stream still alive */
	if (!mail_ping (stream)) {
	  printf ("* BYE %s Fatal mailbox error: %s\015\012",imapdvars->host,
		  imapdvars->lsterr ? imapdvars->lsterr : "<unknown>");
	  imapdvars->state= LOGOUT;	/* go away */
	  syslog (LOG_INFO,
		  gettext("Fatal mailbox error user=%.80s host=%.80s mbx=%.80s: %.80s"),
		  imapdvars->user ? imapdvars->user : gettext("???"),tcp_clienthost (tmp),
		  (stream && stream->mailbox) ? stream->mailbox : gettext("???"),
		  imapdvars->lsterr ? imapdvars->lsterr : gettext("<unknown>"));
	}
	else {			/* did driver change? */
	  /* SOME DEBUGGING */
	  if (stream->lock) {
	    sprintf(tmp,"Stream locked after ping call, cmd =  %s",imapdvars->cmd);
	    mm_log(tmp,WARN,stream);
	  }
				/* make sure stream still alive */
	  if (imapdvars->curdriver != stream->dtb) {
	    printf ("* OK [UIDVALIDITY %lu] UID validity status\015\012",
		    stream->uid_validity);
				/* send mailbox flags */
	    new_flags(stream,tmp);
				/* note readonly/write if possible change */
	    if (imapdvars->curdriver) printf ("* OK [READ-%s] 128 Mailbox status\015\012",
				 stream->rdonly ? "ONLY" : "WRITE");
	    imapdvars->curdriver = stream->dtb;
	  }
				/* change in recent messages? */
	  if (imapdvars->recent != stream->recent)
	 printf ("* %lu RECENT\015\012",
		(imapdvars->recent = stream->recent));
				/* output changed flags */
	  for (i = 1; i <= stream->nmsgs; i++) if (mail_elt (stream,i)->spare2) {
	    printf ("* %lu FETCH (",i);
	    fetch_flags (i,NIL, imapdvars,stream);
	    fputs (")\015\012",stdout);
	  }
	}
	/* flush new error to avoid memory leak. -clin */ 
	if (imapdvars->lsterr && !savelsterr )  
	fs_give ((void **) &imapdvars->lsterr); 
	imapdvars->lsterr =  savelsterr;
	imapdvars->response = saveresponse;
      }

				/* output any new alerts */
      if (!stat (ALERTFILE,&sbuf) && (sbuf.st_mtime > imapdvars->alerttime)) {
	FILE *alf = fopen (ALERTFILE,"r");
	int c,lc = '\012';
	if (alf) {		/* only if successful */
	  while ((c = getc (alf)) != EOF) {
	    if (lc == '\012') fputs ("* OK [ALERT] ",stdout);
	    switch (c) {	/* output character */
	    case '\012':
	      fputs ("\015\012",stdout);
	    case '\0':		/* flush nulls */
	      break;
	    default:
	      putchar (c);	/* output all other characters */
	      break;
	    }
	    lc = c;		/* note previous character */
	  }
	  fclose (alf);
	  if (lc != '\012') fputs ("\015\012",stdout);
	  imapdvars->alerttime = sbuf.st_mtime;
	}
      }
				/* get text for alternative win message now */
      /* If we received a read/only signal */
      if (ronlystr) {
	fputs(ronlystr,stdout);
	ronlystr = NIL;
      }

      if (imapdvars->response == altwin) imapdvars->cmd = imapdvars->lsterr;
      /* output imapdvars->response */
      printf (imapdvars->response,imapdvars->tag,imapdvars->cmd,imapdvars->lsterr);
    }
    fflush (stdout);		/* make sure output blatted */
    /* END trace */

    t1._d = (gethrvtime()._d - t0._d)/1000000.0;
    r1._d = (gethrtime()._d - r0._d)/1000000.0;
    TNF_PROBE_3(cmd_end,"imap4","",tnf_string,exec,probe_cmd,
		tnf_long,cpu,t1._l[0],
		tnf_long,wck,r1._l[0]);
  } while (imapdvars->state != LOGOUT);	/* until logged out */
  syslog (LOG_INFO,
	  gettext("Logout user=%s host=%s"),imapdvars->user ? 
	  imapdvars->user : gettext("???"),
	  tcp_clienthost (tmp));
  imapd_var_free(imapdvars);    /* free it to be safe. -clin*/
  exit (0);			/* all done */

}

/* SUN Imap4 - make some subroutines out of the gigantic main () */
void *do_select (char *tmp, IMAPD_VARS *imapdvars, MAILSTREAM *stream) 
{
  char *s;
  IMAPD_VARS *iv = (IMAPD_VARS *)stream->userdata;

				/* single argument */
  if (!(s = snarf (&imapdvars->arg,imapdvars,stream))) 
		imapdvars->response = misarg;
  else if (imapdvars->arg) 
		imapdvars->response = badarg;

  else if (nameok (NIL,s, imapdvars)) {
    long options;
    options = (imapdvars->anonymous ? OP_ANONYMOUS + OP_READONLY : NIL) +
      ((*imapdvars->cmd == 'E') ? OP_READONLY : NIL);
    options |= OP_SERVEROPEN;	/* 1-aout-96: Force "cd $HOME" */
    /* SUN Imap4 12 juin 95 WJY */
    if (imapdvars->keep_mime) options |= OP_KEEP_MIME;
    if (imapdvars->sunversion_set) options |= OP_SUNVERSION;
    imapdvars->curdriver = NIL;	/* no drivers known */
	/*iv->recent needs to be updated as well. -clin */
    imapdvars->recent = -1;
	/* make sure we get an update */
    if ((stream = mail_open (stream,s,options)) &&
	((imapdvars->response == win) || (imapdvars->response == altwin))) {
      /* SUN Imap4 uses baiser (kiss) */
      baiser = 0;	/* Break it up. -clin */
      imapdvars->kodcount = 0;	/* initialize KOD count */
      imapdvars->lire = 0;
      imapdvars->rocount = 0;       /* initialize ReadOnly count */
      imapdvars->state = OPEN;	/* note state open */
      /* note readonly/readwrite */
      imapdvars->response = stream->rdonly ?
	"%s OK [READ-ONLY] %s completed\015\012" :
	  "%s OK [READ-WRITE] %s completed\015\012";
      if (imapdvars->anonymous) syslog (LOG_INFO,
			gettext("Anonymous select of %s host=%s"),
			     stream->mailbox,tcp_clienthost (tmp));
      /* Sun Imap4 -- Send checksum status */
				/* SUN Imap4: sunversion inherited */
      if (imapdvars->sunversion_set) stream->sunvset = T;
      if (imapdvars->sunversion_set) {	/* do status checksum */
	stream->checksum_type = OPENCHECKSUM;
	mail_checksum(stream);
      }
    }
    else {		
	/* Close the stream IFF it was opened. -clin */
      if (stream && (stream->stream_status & S_OPENED ))
      mail_close(stream)
      else mail_stream_flush(stream); 

      stream = NIL;
      imapdvars->state = SELECT;	/* no mailbox open now */
      imapdvars->response = lose;	/* open failed */
      if (!imapdvars->lsterr) imapdvars->lsterr = cpystr("Open failed");
    }
  }
	return stream;
}
/* SUN Imap4:
 * We use mmap rather than malloc() for appending.
 * Append a message to a mailbox */
void do_append (IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  char *s,                      /* mailbox name */
       *t,                      /* text data */
       *u,                      /* any flags/NIL */
       *v;                      /* date/NIL */
  char *aname;                  /* append name */
  unsigned long i;              /* data size */
  IMAPD_VARS *iv = (IMAPD_VARS *)stream->userdata;
  u = v = NIL;		        /* init flags/date */

				/* parse mailbox name */
  if ((s = snarf (&imapdvars->arg,imapdvars,stream)) && imapdvars->arg) {
    if (*imapdvars->arg == '(') {	/* parse optional flag list */
      u = ++imapdvars->arg;	/* pointer to flag list contents */
      while (*imapdvars->arg && (*imapdvars->arg != ')')) imapdvars->arg++;
      if (*imapdvars->arg) *imapdvars->arg++ = '\0';
      if (*imapdvars->arg == ' ') imapdvars->arg++;
    }
                                /* parse optional date */
    if (*imapdvars->arg == '"') v = snarf (&imapdvars->arg,imapdvars,stream);
                                /* parse message */
    if (!imapdvars->arg || (*imapdvars->arg != '{')) {
      imapdvars->response = "%s BAD Missing literal in %s\015\012";
	}
    else if (!(isdigit (imapdvars->arg[1]) && 
		(i = strtol (imapdvars->arg+1,NIL,10)))) 
      imapdvars->response = "%s NO Empty message to %s\015\012";
    else if (!append_snarf (&imapdvars->arg, &aname, imapdvars,stream)) 
		/* get the data */
      imapdvars->response = misarg;
    else if (imapdvars->arg) {
      imapdvars->response = badarg;
      fs_mumap(imapdvars->append_fd, aname, 
	imapdvars->append_buf, imapdvars->append_len);
    } else {		/* append the data */
      STRING st;
      mail_stream_setNIL(stream);	
      INIT (&st, mail_string,(void *) imapdvars->append_buf, i);
	/*Definitely need to pass a non-null stream here - clin */
      if (!mail_append_full (stream, s, u, v, &st)) {
	imapdvars->response = lose;
	/* in case TRYCREATE failure */
	if (!imapdvars->lsterr)  
	  imapdvars->lsterr = cpystr ("11 Cannot open append mailbox");
      }
	mail_stream_unsetNIL(stream);	
      fs_mumap(imapdvars->append_fd, aname, 
	imapdvars->append_buf, imapdvars->append_len);
    }
  }
  else 
    imapdvars->response = misarg;
}
/*
 * do_store: The various store options */
void
do_store (char *arg, int uid, IMAPD_VARS *imapdvars,
		MAILSTREAM *stream) {
  long i;
  char tmp[MAILTMPLEN];
  char *s,                      /* sequence */
       *t,			/* flag to set */
       *u, 
       *v,
       *lasts;                  /* local pointer for strtok_r */
  IMAPD_VARS *iv = (IMAPD_VARS *)stream->userdata;

  void (*f) () = NIL;	
  long flags = uid ? ST_UID : NIL;
  int low_bw = NIL;
  /* must have three arguments */
  if (!(arg && (s = strtok_r (arg," ",&lasts)) && (v = strtok_r (NIL," ",&lasts)) &&
	(t = strtok_r (NIL,"\015\012",&lasts)))) 
	imapdvars->response = misarg;
  else {		/* see if silent store wanted */
    if ((u = strchr (ucase (v),'.')) && !strcmp (u,".SILENT")) {
      flags |= ST_SILENT;
      *u = '\0';
    }
    if (!strcmp (v,"+FLAGS")) f = mail_setflag_full;
    else if (!strcmp (v,"-FLAGS")) f = mail_clearflag_full;
    else if (!strcmp (v,"+1FLAG")) {
      f = mail_set1flag_full;
      low_bw = T;
    } else if (!strcmp (v,"-1FLAG")) {
      f = mail_clear1flag_full;
      low_bw = T;
    } else if (!strcmp (v,"FLAGS")) {
      tmp[0] = '(';	/* start new flag list */
      tmp[1] = '\0';
      for (i = 0; i < NUSERFLAGS; i++)
	if (v = stream->user_flags[i]) strcat (strcat (tmp,v)," ");
				/* append system flags to list */
      strcat (tmp,"\\Answered \\Flagged \\Deleted \\Draft \\Seen)");
				/* gross, but rarely if ever done */
      mail_clearflag_full (stream,s,t,flags);
      f = mail_setflag_full;
    }
    else imapdvars->response = badatt;
    if (f) {		/* if a function was selected */
      (*f) (stream,s,t,flags);
      /* return flags if silence not wanted */
      if (!(flags & ST_SILENT) &&
	  (uid ? mail_uid_sequence (stream,s) :
	   mail_sequence (stream,s))) {
	if (low_bw)
	  fetch_1flag (v, s, t, uid, imapdvars, stream);
	else
	  fetch ("FLAGS",uid,NIL, imapdvars, stream);
      }
    }
  }
}


/* Clock interrupt:
 *   Can only happen while we are inputing data, ie,
 *   is armed/disarmed in slurp().
 */

/* We can't trust the "imapdvars" because it can become NIL.
But we know the stream is still alive at this state. 
Therefore we pass a stream instead and hopefully the imapdvars
on it are still current by the time it gets here. 

  We can't pass a stream either because a void routine is passed
is signal routine. Therefore, we have to leave these three routines 
alone which means we have to define a global stream call "g_stream". 
We set a link of the local stream to the g_stream before calling 
server_traps so that these signal handlers can access the variables 
on the g_stream.  But this is only a temperary solution - the g_stream 
can still be shared by different threads. -clin */

void clkint (int i)
{
  IMAPD_VARS *g_iv = (IMAPD_VARS *) g_stream->userdata;
  char tmp[MAILTMPLEN];
  fputs ("* BYE Autologout; idle for too long\015\012",stdout);
  syslog (LOG_INFO,
	  gettext("Autologout user=%s host=%s"),
	  g_iv->user ? g_iv->user : gettext("???"),
	  tcp_clienthost (tmp));
  fflush (stdout);		/* make sure output blatted */
				/* try to gracefully close the stream */
  if (g_iv->state == OPEN) {
	mail_close (g_stream);
  	g_stream = NIL;
	}
  exit (0);			/* die die die */
}


/* Kiss Of Death interrupt:
 *  Can occur at any time. If ti arrives while we are waiting
 *  to input in select_input(), then it is handled there.
 */
void kodint (int i)
{
  IMAPD_VARS *g_iv = (IMAPD_VARS *) g_stream->userdata;
  char *s;
  /* SUN Imap4:
   * Signals cause read/errors - 
   *  If this signal arrives while we are processing a command,
   *  then select_input() will handle it and exit using
   *  the baiser (kiss) flag */

  if (!g_iv->kodcount++) {		
				/* only do this once please */
				/* must be open for this to work */
    if ( g_stream && (g_iv->state == OPEN) && 
	g_stream->dtb && (s = g_stream->dtb->name)) {
      /* Sun IMAP4 - we currently always EXIT on kod signal:
       *     See select_input() */
      baiser = T;		/* flags signal is caught select_input() */
    }
  }
}
/*
 * Here we receive a signal to set a mail box read-only */
void roint (int i)
{
  IMAPD_VARS *g_iv = (IMAPD_VARS *) g_stream->userdata;
  char *s;
  /* SUN Imap4:
   * Signals cause read/errors - 
   *  If this signal arrives while we are processing a command,
   *  then select_input() will ingnore it.
   */
  if (!g_iv->rocount++) {		
				/* only do this once please */
				/* must be open for this to work */
    if (g_stream && (g_iv->state == OPEN) && 
	g_stream->dtb && (s = g_stream->dtb->name) &&
	(!strcmp (s,"bezerk") || 
	 !strcmp (s,"mbox") ||
	 !strcmp (s,"solaris") || /* SUN Imap4: solaris driver */
	 !strcmp (s,"mmdf"))) {
      	g_iv->lire= T;			
			/* flags signal is caught */
      ronlystr = "* OK [READ-ONLY] 128 Now READ-ONLY, mailbox lock surrendered\015\012";
      g_stream->rdonly = T;	/* make the stream readonly */
      g_stream->rcvkor = T;	/* note reception for mail_ping */
      mail_ping (g_stream);	/* cause it to stick! removes the imap mailbox */
				/* lock */
    }
  }
}


/* Send flags for stream
 * Accepts: MAIL stream
 *	    scratch buffer
 */

void new_flags (MAILSTREAM *stream,char *tmp)
{
  int i,perm_count;
  tmp[0] = '(';			/* start new flag list */
  tmp[1] = '\0';
  for (i = 0; i < NUSERFLAGS; i++)
    if (stream->user_flags[i]) strcat (strcat (tmp,stream->user_flags[i])," ");
				/* append system flags to list */
  strcat (tmp,"\\Answered \\Flagged \\Deleted \\Draft \\Seen)");
				/* output list of flags */
  printf ("* FLAGS %s\015\012* OK [PERMANENTFLAGS (",tmp);
  tmp[0] = tmp[1] ='\0';	/* write permanent flags */
  for (i = 0,perm_count = 0; i < NUSERFLAGS; i++)
    if ((stream->perm_user_flags & (1 << i)) && stream->user_flags[i]) {
      strcat (strcat (tmp," "),stream->user_flags[i]);
      ++perm_count;
    }
  if (stream->kwd_create) { ++perm_count; strcat (tmp," \\*"); }
  if (stream->perm_answered) { ++perm_count; strcat (tmp," \\Answered"); }
  if (stream->perm_flagged) { ++perm_count; strcat (tmp," \\Flagged"); }
  if (stream->perm_deleted) { ++perm_count; strcat (tmp," \\Deleted"); }
  if (stream->perm_draft) { ++perm_count; strcat (tmp," \\Draft"); }
  if (stream->perm_seen) { ++perm_count; strcat (tmp," \\Seen"); }
  /* If no permanent flags, then a slightly different message
   * is sent. */
  if (perm_count > 0)
    printf ("%s)] Permanent flags\015\012",tmp+1);
  else
    printf (")] No permanent flags\015\012");
}


/* Slurp a command line
 * Accepts: buffer pointer
 *	    buffer size
 */

void slurp (char *s,int n, IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  alarm (TIMEOUT);		/* get a command under timeout */
  errno = 0;			/* clear error */
  /* SUN imap4 uses a select first
   *  select_input() has a select and can thus catch signals.
   *  roint/kodint are handled there too.
   */
  select_input(fileno(stdin), imapdvars, stream);
  while (!fgets (s,n,stdin)) {	/* read buffer */
    /*  Could have gotten kod or clock interrupt while fgets()ing:
     *  See the kodint()/clkint() handlers. One of them will handle
     *  the signal tout de suite! */
    if (errno == EINTR) {
      if (baiser) {	/* kod SIGNAL received DURING a command */
	fputs ("* BYE 127 Kiss-of-death received\015\012",stdout);
	fflush (stdout);/* make sure output blatted */
	if (imapdvars->state== OPEN) 
	  mail_close (stream);
	_exit (1);
      }
      errno = 0;
    } else {
      syslog (LOG_INFO,
	      gettext("Connection broken while reading line user=%s host=%s"),
	      imapdvars->user ? imapdvars->user : gettext("???"),tcp_clienthost (s));
      if (imapdvars->state == OPEN) {
	mail_close (stream);
        stream = NIL;
	}
      _exit (1);
    }
  }
  alarm (0);			/* make sure timeout disabled */
}


/* Read a character
 * Returns: character
 */

char inchar (IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  char c, tmp[MAILTMPLEN];
  size_t n;

  /* We must have 1 byte of data  when the select returns */
  select_input(fileno(stdin), imapdvars, stream);
  while ((n = read (fileno(stdin), (void *)&c, 1)) != 1) {
    if (errno == EINTR) 
      /* ignore if some interrupt */
      errno = 0;
    else {
      if (imapdvars->state == OPEN) {
	fputs ("* 127 IMAPD system error during read\015\012", stdout);
	fflush(stdout);
	mail_close(stream);
      }
      syslog (LOG_INFO,
	      gettext("Connection broken while reading char user=%s host=%s"),
	      imapdvars->user ? imapdvars->user : gettext("???"),
	      tcp_clienthost (tmp));      _exit (1);
    }
  }
  return c;
}


/* Flush until newline seen
 * Returns: NIL, always
 */

void *flush (IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{

  while (inchar (imapdvars, stream) != '\012');	
		/* slurp until we find newline */
  imapdvars->response = toobig;
  return NIL;
}

/* Parse an IMAP astring
 * Accepts: pointer to argument text pointer
 *	    pointer to returned size
 *	    pointer to returned delimiter
 * Returns: argument
 */

char *parse_astring (char **arg,unsigned long *size,char *del, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  unsigned long i;
  char *s,*t,*v,tmp[TMPLEN];
  char *lasts;

  if (!*arg) return NIL;	/* better be an argument */
  switch (**arg) {		/* see what the argument is */
  case '\0':			/* catch bogons */
  case ' ':
    return NIL;
  case '"':			/* hunt for trailing quote */
    for (s = t = *arg + 1; *t != '"'; t++) {
      if (*t == '\\') t++;	/* quote next character */
      if (!*t) return NIL;	/* unterminated quoted string! */
    }
    *t++ = '\0';		/* tie off string */
    *size = strlen (s);		/* return size */
    break;
  case '{':			/* literal string */
    s = *arg + 1;		/* get size */
    if (!(isdigit (*s) && (*size = i = strtol (s,&t,10)) && t && (*t == '}') &&
	  !t[1])) return NIL;	/* validate end of literal */
    if (imapdvars->litsp >= LITSTKLEN) {	/* make sure don't overflow stack */

 	mail_stream_setNIL(stream);
        mm_notify (stream,"Too many literals in command",ERROR);
 	mail_stream_unsetNIL(stream);
      return NIL;
    }
    fputs (argrdy,stdout);	/* tell client ready for argument */
    fflush (stdout);		/* dump output buffer */
				/* get a literal buffer */
    s = v = imapdvars->litstk[imapdvars->litsp++] = (char *) fs_get (i+1);
    alarm (TIMEOUT);		/* start timeout */
    while (i--) *v++ = inchar (imapdvars, stream);
    alarm (0);			/* stop timeout */
    *v++ = NIL;			/* make sure string tied off */
    				/* get new command tail */
    slurp ((*arg = v = t),TMPLEN - (t - imapdvars->cmdbuf) - 1, imapdvars, stream);
    if (!strchr (t,'\012')) return flush (imapdvars,stream);
				/* reset strtok_r mechanism, tie off if done */
    if (!strtok_r (t,"\015\012", &lasts)) *t = '\0';
    break;
  default:			/* atom */
    for (s = t = *arg, i = 0;
	 (*t > ' ') && (*t != '(') && (*t != ')') && (*t != '{') &&
	 (*t != '%') && (*t != '*') && (*t != '"') && (*t != '\\'); ++t,++i);
    *size = i;			/* return size */
    break;
  }
  if (*del = *t) {		/* have a delimiter? */
    *t++ = '\0';		/* yes, stomp on it */
    *arg = t;			/* update argument pointer */
  }
  else *arg = NIL;		/* no more arguments */
  return s;
}

/* Snarf a command argument (simple jacket into parse_astring())
 * Accepts: pointer to argument text pointer
 * Returns: argument
 */

char *snarf (char **arg, IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  unsigned long i;
  char c;
  char *s = parse_astring (arg,&i,&c, imapdvars, stream);
  return ((c == ' ') || !c) ? s : NIL;
}

/* Snarf a list command argument (simple jacket into parse_astring())
 * Accepts: pointer to argument text pointer
 * Returns: argument
 */

char *snarf_list (char **arg, IMAPD_VARS *imapdvars, 
			MAILSTREAM *stream)
{
  unsigned long i;
  char c;
  char *s,*t;
  if (!*arg) return NIL;	/* better be an argument */
  switch (**arg) {
  case '\0':			/* catch bogons */
  case ' ':
    return NIL;
  case '"':			/* quoted string? */
  case '{':			/* or literal? */
    s = parse_astring (arg,&i,&c, imapdvars, stream);
    break;
  default:			/* atom and/or wildcard chars */
    for (s = t = *arg, i = 0;
	 (*t > ' ') && (*t != '(') && (*t != ')') && (*t != '{') &&
	 (*t != '"') && (*t != '\\'); ++t,++i);
    if (c = *t) {		/* have a delimiter? */
      *t++ = '\0';		/* stomp on it */
      *arg = t;			/* update argument pointer */
    }
    else *arg = NIL;
    break;
  }
  return ((c == ' ') || !c) ? s : NIL;
}

/* Get a list of header lines
 * Accepts: pointer to string pointer
 *	    pointer to list flag
 * Returns: string list
 */

STRINGLIST *parse_stringlist (char **s,int *list, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  char c = ' ',*t;
  unsigned long i;
  STRINGLIST *ret = NIL,*cur = NIL;
  char *lasts;		/* pointer for strtok_r [10/21/96 ] */ 
  if (*s && **s == '(') {	/* proper list? */
    ++*s;			/* for each item in list */
    while ((c == ' ') && 
	(t = parse_astring (s,&i,&c, imapdvars, stream))) {
				/* get new block */
      if (cur) cur = cur->next = mail_newstringlist ();
      else cur = ret = mail_newstringlist ();
      cur->text = (char *) fs_get (i + 1);
      memcpy (cur->text,t,i);	/* note text */
      cur->size = i;		/* and size */
    }
				/* must be end of list */
    if (c != ')') mail_free_stringlist (&ret);
  }
  if (t = *s) {			/* need to reload strtok_r() state? */
				/* end of a list? */
    if (*list && (*t == ')') && !t[1]) *list = NIL;
    else {
      *--t = ' ';		/* patch a space back in */
      *--t = 'x';		/* and a hokey character before that */
      t = strtok_r (t," ", &lasts); /* reset to *s */
    }
  }
  return ret;
}

/* Parse search criteria
 * Accepts: search program to write criteria into
 *	    pointer to argument text pointer
 *	    maximum message number
 *	    maximum UID
 * Returns: T if success, NIL if error
 */

long parse_criteria (SEARCHPGM *pgm,char **arg,unsigned long maxmsg,
   unsigned long maxuid, IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  if (arg && *arg) {		/* must be an argument */
				/* parse criteria */
    do if (!parse_criterion (pgm,arg,maxmsg,maxuid,imapdvars,stream)) 
		return NIL;
				/* as long as a space delimiter */
    while (**arg == ' ' && (*arg)++);
				/* failed if not end of criteria */
    if (**arg && **arg != ')') return NIL;
  }
  return T;			/* success */
}

/* Parse a search criterion
 * Accepts: search program to write criterion into
 *	    pointer to argument text pointer
 *	    maximum message number
 *	    maximum UID
 * Returns: T if success, NIL if error
 */

long parse_criterion (SEARCHPGM *pgm,char **arg,unsigned long maxmsg,
      unsigned long maxuid, IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  unsigned long i;
  char c = NIL,*s,*t,*v,*tail,*del;
  SEARCHSET **set;
  long ret = NIL;
  if (!(arg && *arg));		/* better be an argument */
  else if (**arg == '(') {	/* list of criteria? */
    (*arg)++;			/* yes, parse the criteria */
    if (parse_criteria (pgm,arg,maxmsg,maxuid,imapdvars, stream) && 
	**arg == ')') {
      (*arg)++;			/* skip closing paren */
      ret = T;			/* successful parse of list */
    }
  }
  else {			/* instantiate un program if needed */
				/* find end of criterion */
    if (!(tail = strpbrk ((s = *arg)," )"))) tail = *arg + strlen (*arg);
    c = *(del = tail);		/* remember the delimiter */
    *del = '\0';		/* tie off criterion */
    switch (*ucase (s)) {	/* dispatch based on character */
    case '*':			/* sequence */
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      if (*(set = &pgm->msgno)){/* already a sequence? */
				/* silly, but not as silly as the client! */
	SEARCHPGMLIST **not = &pgm->not;
	while (*not) not = &(*not)->next;
	*not = mail_newsearchpgmlist ();
	set = &((*not)->pgm->not = mail_newsearchpgmlist ())->pgm->msgno;
      }
      ret = crit_set (set,&s,maxmsg) && (tail == s);
      break;
    case 'A':			/* possible ALL, ANSWERED */
      if (!strcmp (s+1,"LL")) ret = T;
      else if (!strcmp (s+1,"NSWERED")) ret = pgm->answered = T;
      break;
    case 'B':			/* possible BCC, BEFORE, BODY */
      if (!strcmp (s+1,"CC") && c == ' ' && *++tail)
	ret = crit_string (&pgm->bcc,&tail, imapdvars,stream);
      else if (!strcmp (s+1,"EFORE") && c == ' ' && *++tail)
	ret = crit_date (&pgm->before,&tail);
      else if (!strcmp (s+1,"ODY") && c == ' ' && *++tail)
	ret = crit_string (&pgm->body,&tail, imapdvars,stream);
      break;

    case 'C':			/* possible CC */
      if (!strcmp (s+1,"C") && c == ' ' && *++tail)
	ret = crit_string (&pgm->cc,&tail, imapdvars,stream);
      break;
    case 'D':			/* possible DELETED */
      if (!strcmp (s+1,"ELETED")) ret = pgm->deleted = T;
      if (!strcmp (s+1,"RAFT")) ret = pgm->draft = T;
      break;
    case 'F':			/* possible FLAGGED, FROM */
      if (!strcmp (s+1,"LAGGED")) ret = pgm->flagged = T;
      else if (!strcmp (s+1,"ROM") && c == ' ' && *++tail)
	ret = crit_string (&pgm->from,&tail, imapdvars,stream);
      break;
    case 'H':			/* possible HEADER */
      if (!strcmp (s+1,"EADER") && c == ' ' && *(v = tail + 1) &&
	  (s = parse_astring (&v,&i,&c, imapdvars, stream)) 
		&& i && c == ' ' &&
	  (t = parse_astring (&v,&i,&c, imapdvars, stream)) && i) {
	SEARCHHEADER **hdr = &pgm->header;
	while (*hdr) hdr = &(*hdr)->next;
	*hdr = mail_newsearchheader (s);
	(*hdr)->text = cpystr (t);
				/* update tail, restore delimiter */
	*(tail = v ? v - 1 : t + i) = c;
	ret = T;		/* success */
      }
      break;
    case 'K':			/* possible KEYWORD */
      if (!strcmp (s+1,"EYWORD") && c == ' ' && *++tail)
	ret = crit_string (&pgm->keyword,&tail,imapdvars,stream);
      break;
    case 'L':
      if (!strcmp (s+1,"ARGER") && c == ' ' && *++tail)
	ret = crit_number (&pgm->larger,&tail);
      break;
    case 'N':			/* possible NEW, NOT */
      if (!strcmp (s+1,"EW")) ret = pgm->recent = pgm->unseen = T;
      else if (!strcmp (s+1,"OT") && c == ' ' && *++tail) {
	SEARCHPGMLIST **not = &pgm->not;
	while (*not) not = &(*not)->next;
	*not = mail_newsearchpgmlist ();
	ret = parse_criterion ((*not)->pgm,&tail,maxmsg,
		maxuid, imapdvars,stream);
      }
      break;

    case 'O':			/* possible OLD, ON */
      if (!strcmp (s+1,"LD")) ret = pgm->old = T;
      else if (!strcmp (s+1,"N") && c == ' ' && *++tail)
	ret = crit_date (&pgm->on,&tail);
      else if (!strcmp (s+1,"R") && c == ' ') {
	SEARCHOR **or = &pgm->or;
	while (*or) or = &(*or)->next;
	*or = mail_newsearchor ();
	ret = *++tail && parse_criterion ((*or)->first,&tail,
	maxmsg,maxuid,imapdvars,stream ) &&
	  *tail == ' ' && *++tail &&
	    parse_criterion ((*or)->second,&tail,maxmsg,maxuid, 
	imapdvars,stream);
      }
      break;
    case 'R':			/* possible RECENT */
      if (!strcmp (s+1,"ECENT")) ret = pgm->recent = T;
      break;
    case 'S':			/* possible SEEN, SINCE, SUBJECT */
      if (!strcmp (s+1,"EEN")) ret = pgm->seen = T;
      else if (!strcmp (s+1,"ENTBEFORE") && c == ' ' && *++tail)
	ret = crit_date (&pgm->sentbefore,&tail);
      else if (!strcmp (s+1,"ENTON") && c == ' ' && *++tail)
	ret = crit_date (&pgm->senton,&tail);
      else if (!strcmp (s+1,"ENTSINCE") && c == ' ' && *++tail)
	ret = crit_date (&pgm->sentsince,&tail);
      else if (!strcmp (s+1,"INCE") && c == ' ' && *++tail)
	ret = crit_date (&pgm->since,&tail);
      else if (!strcmp (s+1,"MALLER") && c == ' ' && *++tail)
	ret = crit_number (&pgm->smaller,&tail);
      else if (!strcmp (s+1,"UBJECT") && c == ' ' && *++tail)
	ret = crit_string (&pgm->subject,&tail, imapdvars,stream);
      break;
    case 'T':			/* possible TEXT, TO */
      if (!strcmp (s+1,"EXT") && c == ' ' && *++tail)
	ret = crit_string (&pgm->text,&tail,imapdvars,stream);
      else if (!strcmp (s+1,"O") && c == ' ' && *++tail)
	ret = crit_string (&pgm->to,&tail, imapdvars,stream);
      break;

    case 'U':			/* possible UID, UN* */
      if (!strcmp (s+1,"ID") && c== ' ' && *++tail) {
	if (*(set = &pgm->uid)){/* already a sequence? */
				/* silly, but not as silly as the client! */
	  SEARCHPGMLIST **not = &pgm->not;
	  while (*not) not = &(*not)->next;
	  *not = mail_newsearchpgmlist ();
	  set = &((*not)->pgm->not = mail_newsearchpgmlist ())->pgm->uid;
	}
	ret = crit_set (set,&tail,maxuid);
      }
      else if (!strcmp (s+1,"NANSWERED")) ret = pgm->unanswered = T;
      else if (!strcmp (s+1,"NDELETED")) ret = pgm->undeleted = T;
      else if (!strcmp (s+1,"NDRAFT")) ret = pgm->undraft = T;
      else if (!strcmp (s+1,"NFLAGGED")) ret = pgm->unflagged = T;
      else if (!strcmp (s+1,"NKEYWORD") && c == ' ' && *++tail)
	ret = crit_string (&pgm->unkeyword,&tail, imapdvars,stream);
      else if (!strcmp (s+1,"NSEEN")) ret = pgm->unseen = T;
      break;
    default:			/* oh dear */
      break;
    }
    if (ret) {			/* only bother if success */
      *del = c;			/* restore delimiter */
      *arg = tail;		/* update argument pointer */
    }
  }
  return ret;			/* return more to come */
}

/* Parse a search date criterion
 * Accepts: date to write into
 *	    pointer to argument text pointer
 * Returns: T if success, NIL if error
 */

long crit_date (unsigned short *date,char **arg)
{
				/* handle quoted form */
  if (**arg != '"') return crit_date_work (date,arg);
  (*arg)++;			/* skip past opening quote */
  if (!(crit_date_work (date,arg) && (**arg == '"'))) return NIL;
  (*arg)++;			/* skip closing quote */
  return T;
}

/* Worker routine to parse a search date criterion
 * Accepts: date to write into
 *	    pointer to argument text pointer
 * Returns: T if success, NIL if error
 */

long crit_date_work (unsigned short *date,char **arg)
{
  int d,m,y;
  if (isdigit (**arg)) {	/* day */
    d = *(*arg)++ - '0';	/* first digit */
    if (isdigit (**arg)) {	/* if a second digit */
      d *= 10;			/* slide over first digit */
      d += *(*arg)++ - '0';	/* second digit */
    }
    if ((**arg == '-') && (y = *++(*arg))) {
      m = (y >= 'a' ? y - 'a' : y - 'A') * 1024;
      if ((y = *++(*arg))) {
	m += (y >= 'a' ? y - 'a' : y - 'A') * 32;
	if ((y = *++(*arg))) {
	  m += (y >= 'a' ? y - 'a' : y - 'A');
	  switch (m) {		/* determine the month */
	  case (('J'-'A') * 1024) + (('A'-'A') * 32) + ('N'-'A'): m = 1; break;
	  case (('F'-'A') * 1024) + (('E'-'A') * 32) + ('B'-'A'): m = 2; break;
	  case (('M'-'A') * 1024) + (('A'-'A') * 32) + ('R'-'A'): m = 3; break;
	  case (('A'-'A') * 1024) + (('P'-'A') * 32) + ('R'-'A'): m = 4; break;
	  case (('M'-'A') * 1024) + (('A'-'A') * 32) + ('Y'-'A'): m = 5; break;
	  case (('J'-'A') * 1024) + (('U'-'A') * 32) + ('N'-'A'): m = 6; break;
	  case (('J'-'A') * 1024) + (('U'-'A') * 32) + ('L'-'A'): m = 7; break;
	  case (('A'-'A') * 1024) + (('U'-'A') * 32) + ('G'-'A'): m = 8; break;
	  case (('S'-'A') * 1024) + (('E'-'A') * 32) + ('P'-'A'): m = 9; break;
	  case (('O'-'A') * 1024) + (('C'-'A') * 32) + ('T'-'A'): m = 10;break;
	  case (('N'-'A') * 1024) + (('O'-'A') * 32) + ('V'-'A'): m = 11;break;
	  case (('D'-'A') * 1024) + (('E'-'A') * 32) + ('C'-'A'): m = 12;break;
	  default: return NIL;
	  }
	  if ((*++(*arg) == '-') && isdigit (*++(*arg))) {
	    y = 0;		/* init year */
	    do {
	      y *= 10;		/* add this number */
	      y += *(*arg)++ - '0';
	    }
	    while (isdigit (**arg));
				/* minimal validity check of date */
	    if (d < 1 || d > 31 || m < 1 || m > 12 || y < 0) return NIL; 
				/* Tenex/ARPAnet began in 1969 */
	    if (y < 100) y += (y >= (BASEYEAR - 1900)) ? 1900 : 2000;
				/* return value */
	    *date = ((y - BASEYEAR) << 9) + (m << 5) + d;
	    return T;		/* success */
	  }
	}
      }
    }
  }
  return NIL;			/* else error */
}

/* Parse a search set criterion
 * Accepts: set to write into
 *	    pointer to argument text pointer
 *	    maximum value permitted
 * Returns: T if success, NIL if error
 */

long crit_set (SEARCHSET **set,char **arg,unsigned long maxima)
{
  unsigned long i;
  *set = mail_newsearchset ();	/* instantiate a new search set */
  if (**arg == '*') {		/* maxnum? */
    (*arg)++;			/* skip past that number */
    (*set)->first = maxima;
  }
  else if (crit_number (&i,arg) && i) (*set)->first = i;
  else return NIL;		/* bogon */
  switch (**arg) {		/* decide based on delimiter */
  case ':':			/* sequence range */
    if (*++(*arg) == '*') {	/* maxnum? */
      (*arg)++;			/* skip past that number */
      (*set)->last -= maxima;
    }
    else if (crit_number (&i,arg) && i) {
      if (i < (*set)->first) {	/* backwards range */
	(*set)->last = (*set)->first;
	(*set)->first = i;
      }
      else (*set)->last = i;	/* set last number */
    }
    else return NIL;		/* bogon */
    if (**arg != ',') break;	/* drop into comma case if comma seen */
  case ',':
    (*arg)++;			/* skip past delimiter */
    return crit_set (&(*set)->next,arg,maxima);
  default:
    break;
  }
  return T;			/* return success */
}

/* Parse a search number criterion
 * Accepts: number to write into
 *	    pointer to argument text pointer
 * Returns: T if success, NIL if error
 */

long crit_number (unsigned long *number,char **arg)
{
  if (!isdigit (**arg)) return NIL;
  *number = 0;
  while (isdigit (**arg)) {	/* found a digit? */
    *number *= 10;		/* add a decade */
    *number += *(*arg)++ - '0';	/* add number */
  }
  return T;
}


/* Parse a search string criterion
 * Accepts: date to write into
 *	    pointer to argument text pointer
 * Returns: T if success, NIL if error
 */

long crit_string (STRINGLIST **string,char **arg, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  unsigned long i;
  char c;
  char *s = parse_astring (arg,&i,&c, imapdvars, stream);
  if (!s) return NIL;
				/* find tail of list */
  while (*string) string = &(*string)->next;
  *string = mail_newstringlist ();
  (*string)->text = (char *) fs_get (i + 1);
  memcpy ((*string)->text,s,i);
  (*string)->text[i] = '\0';
  (*string)->size = i;
				/* if end of arguments, wrap it up here */
  if (!*arg) *arg = (*string)->text + i;
  else (*--(*arg) = c);		/* back up pointer, restore delimiter */
  return T;
}

/* Fetch message data
 * Accepts: string of data items to be fetched
 *	    UID fetch flag
 */

#define MAXFETCH 100

void fetch (char *t,unsigned long uid,int reset_sendcksum, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  char c,*s,*v, *lasts;
  unsigned long i,k;
  BODY *b;
  STRINGLIST *h;
  int list = NIL;
  int parse_envs = NIL;
  int parse_bodies = NIL;
  void *doing_uid = NIL;
  void (*f[MAXFETCH]) ();
  STRINGLIST *fa[MAXFETCH];
  int format;

  /* Sun IMAP4:  
   *   Some fetches implicitly set flags (rfc822.text), and
   *   thus coerce a checksum to be sent */
  if (reset_sendcksum)
    stream->send_checksum = NIL;
				/* process macros */
  if (!strcmp (ucase (t),"ALL"))
    strcpy (t,"(FLAGS INTERNALDATE RFC822.SIZE ENVELOPE)");
  else if (!strcmp (t,"FULL"))
    strcpy (t,"(FLAGS INTERNALDATE RFC822.SIZE ENVELOPE BODY)");
  else if (!strcmp (t,"FAST")) strcpy (t,"(FLAGS INTERNALDATE RFC822.SIZE)");
  if (list = (*t == '(')) t++;	/* skip open paren */
  k = 0;			/* initial index */
  if (s = strtok_r (t," ", &lasts)) do {/* parse attribute list */
    fa[k] = NIL;
    if (list && (i = strlen (s)) && (s[i-1] == ')')) {
      list = NIL;		/* done with list */
      s[i-1] = '\0';		/* tie off last item */
    }
    if (!strcmp (s,"BODY")) {	/* we will need to parse bodies */
      parse_envs = parse_bodies = T;
      f[k++] = fetch_body;
    }
    else if (!strcmp (s,"BODYSTRUCTURE")) {
      parse_envs = parse_bodies = T;
      f[k++] = fetch_bodystructure;
    }
    else if (!strcmp (s,"ENVELOPE")) {
      parse_envs = T;		/* we will need to parse envelopes */
      f[k++] = fetch_envelope;
    }
    else if (!strcmp (s,"FLAGS")) f[k++] = fetch_flags;
    else if (!strcmp (s,"INTERNALDATE")) f[k++] = fetch_internaldate;
    else if (!strcmp (s,"RFC822")) f[k++] = fetch_rfc822;
    else if (!strcmp (s,"RFC822.PEEK")) f[k++] = fetch_rfc822_peek;
    else if (!strcmp (s,"RFC822.HEADER")) f[k++] = fetch_rfc822_header;
    else if (!strcmp (s,"RFC822.SIZE")) f[k++] = fetch_rfc822_size;
    else if (!strcmp (s,"RFC822.TEXT")) f[k++] = fetch_rfc822_text;
    else if (!strcmp (s,"RFC822.TEXT.PEEK")) f[k++] = fetch_rfc822_text_peek;
    else if (!strcmp (s,"UID")) {
      doing_uid = (void *) (f[k++] = fetch_uid);
    } else if (!strcmp (s,"SHORTINFO")) { 
      /* SUN Imap4 - wjy */
      parse_envs = T;
      parse_bodies = NIL;
      f[k++] = fetch_shortinfo;
      /* Also do UID here if talking imap4 */
      if (imapdvars->imap4_sunversion) {
	fa[k] = NIL;
	doing_uid = (void *) (f[k++] = fetch_uid);
      }
    }

    else if (*s == 'B' && s[1] == 'O' && s[2] == 'D' && s[3] == 'Y' &&
	     s[4] == '[' && (v = strchr (s + 5,']')) && !(*v = v[1])) {
				/* we will need to parse bodies */
      parse_envs = parse_bodies = T;
				/* set argument */
      (fa[k] = mail_newstringlist ())->text = cpystr (s+5);
      fa[k]->size = strlen (s+5);
      f[k++] = fetch_body_part;
    }
    else if (*s == 'B' && s[1] == 'O' && s[2] == 'D' && s[3] == 'Y' &&
	     s[4] == '.' && s[5] == 'P' && s[6] == 'E' && s[7] == 'E' &&
	     s[8] == 'K' && s[9] == '[' && (v = strchr (s + 10,']')) &&
	     !(*v = v[1])) {	/* we will need to parse bodies */
      parse_envs = parse_bodies = T;
				/* set argument */
      (fa[k] = mail_newstringlist ())->text = cpystr (s+10);
      fa[k]->size = strlen (s+10);
      f[k++] = fetch_body_part_peek;
    }
    else if (!strcmp (s,"RFC822.HEADER.LINES") &&
	     (v = strtok_r (NIL,"\015\012", &lasts)) && 
	     (fa[k] = parse_stringlist (&v,&list, imapdvars,stream)))
      f[k++] = fetch_rfc822_header_lines;
    else if (!strcmp (s,"RFC822.HEADER.LINES.NOT") &&
	     (v = strtok_r (NIL,"\015\012", &lasts)) && 
	     (fa[k] = parse_stringlist (&v,&list, imapdvars,stream)))
      f[k++] = fetch_rfc822_header_lines_not;
    else {			/* unknown attribute */
      imapdvars->response = badatt;
      return;
    }
  } while ((s = strtok_r (NIL," ",&lasts)) && (k < MAXFETCH) && list);
  else {
    imapdvars->response = misarg;	/* missing attribute list */
    return;
  }

  if (s) {			/* too many attributes? */
    imapdvars->response = "%s BAD Excessively complex FETCH attribute list\015\012";
    return;
  }
  if (list) {			/* too many attributes? */
    imapdvars->response = "%s BAD Unterminated FETCH attribute list\015\012";
    return;
  }
  if (uid && !doing_uid) {	/* UID fetch must fetch UIDs */
    fa[k] = NIL;
    f[k++] = fetch_uid;		/* implicitly fetch UIDs on UID fetch */
  }
  f[k] = NIL;			/* tie off attribute list */

  for (i = 1; i <= imapdvars->nmsgs; i++)/* c-client clobbers sequence, use spare */
	/* We need to use the imapdvars->nmsgs on the stream here since that's
	the one just being upgraded. -clin */
    mail_elt (stream,i)->spare = mail_elt (stream,i)->sequence;
				/* for each requested message */
  for (i = 1; i <= imapdvars->nmsgs; i++) if (mail_elt (stream,i)->spare) {
				/* parse envelope, set body, do warnings */
    if (parse_envs) mail_fetchenvelope (stream,i);
    if (parse_bodies) mail_fetchstructure (stream,i,&b);
    printf ("* %lu FETCH (",i);	/* leader */
	/* NOTE this:
	(dbx) print f[0]  
	f[0] = &fetch_shortinfo(long i, char *s, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream) at 0x2c6a0
	--This means that f[k] is a pointer to the fetch_xxx functions
	which now require four arguments. -clin */ 

    (*f[0]) (i,fa[0],imapdvars, stream);	/* do first attribute */
    for (k = 1; f[k]; k++) {	/* for each subsequent attribute */
      putchar (' ');		/* delimit with space */
      (*f[k]) (i,fa[k],imapdvars, stream);/* do that attribute */
    }
    fputs (")\015\012",stdout);	/* trailer */
  }
  /* NOW clear the driver's cache */
  for (i = 1; i <= imapdvars->nmsgs; i++) {
    if (mail_elt(stream,i)->spare) 
				/* free the cached data */
      mail_free_cached_buffers_msg(stream,i);
  }      
				/* clean up string arguments */
  for (k = 0; f[k]; k++) if (fa[k]) mail_free_stringlist (&fa[k]);
  /* Check on the status CHECKSUM output */
  format = (imapdvars->imap4_sunversion ? FORMATIMAP4STATUS : FORMAT2BIS);
  do_status_checksum(stream,format);
}

/* Fetch message body structure (extensible)
 * Accepts: message number
 *	    extra argument
 */

void fetch_bodystructure (unsigned long i,STRINGLIST *a, 
		IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  BODY *body;


  mail_fetchstructure (stream,i,&body);
  fputs ("BODYSTRUCTURE ",stdout);
  pbodystructure (body, imapdvars);	/* output body */
}


/* Fetch message body structure (non-extensible)
 * Accepts: message number
 *	    extra argument
 */


void fetch_body (unsigned long i,STRINGLIST *a, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  BODY *body;
  mail_fetchstructure (stream,i,&body);
  fputs ("BODY ",stdout);	/* output attribute */
  pbody (body, imapdvars);	/* output body */
}

/* Fetch message body part
 * Accepts: message number
 *	    extra argument
 */

void fetch_body_part (unsigned long i,STRINGLIST *a, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  char *s = a->text,*data;
  unsigned long j,k;
  BODY *body;
  int f = mail_elt (stream,i)->seen;
  mail_fetchstructure (stream,i,&body);

  printf ("BODY[%s] ",s);	/* output attribute */
  fflush(stdout);
  /* OK, put the text, and catch possible out-of-band
   * interrupt */
  if (body && (data = mail_fetchbody_full (stream,i,s,&j,FT_INTERNAL))) {
    int binary;
    int encoding = get_body_encoding(s,body);
				/* If not ENCBINARY, then count the lfs */
    if (encoding != ENCBINARY) {
      binary = NIL;
      k = add_count_lfs(data,j);
    } else { k = j; binary = T; }
    printf ("{%lu}\015\012", k);	/* and literal string */
    fflush(stdout);
    /* We use the origina len, j, since put_the_text will
     * insert the CRs */
    if (put_the_text (fileno(stdout),data,j, binary))
      changed_flags (i,f,imapdvars,stream);	/* output changed flags */
    else {
      imapdvars->response = lose;
      if (imapdvars->lsterr) fs_give ((void **)&imapdvars->lsterr);
      imapdvars->lsterr = cpystr("Message data flushed");
    }
  }
  else 
    fputs ("NIL",stdout);	/* can't output anything at all */
  fflush(stdout);
}


/* Fetch message body part, peeking
 * Accepts: message number
 *	    extra argument
 */

void fetch_body_part_peek (unsigned long i,STRINGLIST *a, 
		IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  char *s = a->text,*data;
  unsigned long j,k;
  BODY *body;
  int f = mail_elt (stream,i)->seen;
  mail_fetchstructure (stream,i,&body);
  printf ("BODY[%s] ",s);	/* output attribute */
  if (body && (data = mail_fetchbody_full(stream,i,s,&j,FT_PEEK+FT_INTERNAL))) {
    int encoding = get_body_encoding(s,body);
    int binary;
				/* If not ENCBINARY, then count the lfs */
    if (encoding != ENCBINARY) { k = add_count_lfs(data,j); binary = NIL; }
    else { k = j; binary = T; }
    printf ("{%lu}\015\012",k);	/* and literal string */
    fflush(stdout); 
   if (put_the_text (fileno(stdout),data,j,binary))
      changed_flags (i,f,imapdvars,stream);	/* output changed flags */
    else {
      imapdvars->response = lose;
      if (imapdvars->lsterr) fs_give ((void **)&imapdvars->lsterr);
      imapdvars->lsterr = cpystr("Message data flushed");
    }
  }
  else fputs ("NIL",stdout);	/* can't output anything at all */
  fflush(stdout);
}
/*
 * Accepts: section as a string
 *          BODY pointer
 * Returns: body encoding type
 * NOTE: both section and body are valid at entry:
 */
int get_body_encoding(char *s,BODY *b)
{
  int i;
  PART *pt;
  if (!(i = strtol (s,&s,10)))	/* section 0 */
    return ENC7BIT;
  /* ok, search for the section */
  do {				/* until find desired body part */
				/* multipart content? */
    if (b->type == TYPEMULTIPART) {
      pt = b->contents.part;	/* yes, find desired part */
      if (!pt)			/* might be broken MIME */
	return ENC7BIT;		/* should not happen */
      while (--i && (pt = pt->next));
      if (!pt) return ENC7BIT;	/* bad specifier */
				/* note new body, check valid nesting */
      if (((b = &pt->body)->type == TYPEMULTIPART) && !*s) 
	return ENC7BIT;
    } else if (i != 1) return ENC7BIT;/* otherwise must be section 1 */
				/* need to go down further? */
    if (i = *s) {
      switch (b->type) {
      case TYPEMESSAGE:		/* embedded message */
	/* get its body, drop into multipart case */
	b = b->contents.msg.body;
	if ((*s++ == '.') && isdigit(*s) && 
	    (long)(i = strtol (s,&s,10)) >= 0) {
	  if (i == 0) {		/* section 0 */
	    if (*s == NIL) return ENC7BIT;
	    else  return b->encoding;
	  }
	  break;
	} else return ENC7BIT;
      case TYPEMULTIPART:	/* multipart, get next section */
	if ((*s++ == '.') && (i = strtol (s,&s,10)) > 0) break;
      default:			/* bogus subpart specification */
	return ENC7BIT;
      }
    }
  } while (i);
				/* lose if body bogus */
  if ((!b) || b->type == TYPEMULTIPART) return ENC7BIT;
  else return b->encoding;
}

/* Fetch envelope
 * Accepts: message number
 *	    extra argument
 */

void fetch_envelope (unsigned long i,STRINGLIST *a, 
		IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  ENVELOPE *env = mail_fetchenvelope (stream,i);
  fputs ("ENVELOPE ",stdout);	/* output attribute */
  penv (env,imapdvars);		/* output envelope */
}

/* Fetch matching header lines
 * Accepts: message number
 *	    extra argument
 */

void fetch_rfc822_header_lines (unsigned long i,STRINGLIST *a, 
		IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  char c;
  unsigned long size,k;
  char *t = mail_fetchheader_full (stream,i,a,&size,FT_INTERNAL);
  k = add_count_lfs(t,size);
  printf ("RFC822.HEADER {%lu}\015\012",k);
  fflush(stdout);
  /* SUN Imap4 */
  if (!put_the_text (fileno(stdout),t,size,NIL)) {
    imapdvars->response = lose;	/* tell client we've flushed. */
    if (imapdvars->lsterr) fs_give ((void **)&imapdvars->lsterr);
    imapdvars->lsterr = cpystr("Message data flushed");
  }
}


/* Fetch not-matching header lines
 * Accepts: message number
 *	    extra argument
 */

void fetch_rfc822_header_lines_not (unsigned long i,STRINGLIST *a,
		IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  char c;
  unsigned long size,k;
  char *t = mail_fetchheader_full (stream,i,a,&size,FT_NOT+FT_INTERNAL);
  k = add_count_lfs(t,size);
  printf ("RFC822.HEADER {%lu}\015\012",k);
  fflush(stdout);
  /* SUN Imap4 */
  if (!put_the_text (fileno(stdout),t,size,NIL)) {
    imapdvars->response = lose;	/* tell client we've flushed. */
    if (imapdvars->lsterr) fs_give ((void **)&imapdvars->lsterr);
    imapdvars->lsterr = cpystr("Message data flushed");
  }
}

/* Fetch flags
 * Accepts: message number
 *	    extra argument
 */

void fetch_flags (unsigned long i,STRINGLIST *a, 
		IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  unsigned long u;
  char *t,tmp[MAILTMPLEN];
  int c = NIL;
  MESSAGECACHE *elt = mail_elt (stream,i);
  if (!elt->valid) {		/* have valid flags yet? */
    sprintf (tmp,"%lu",i);
    mail_fetchflags (stream,tmp);
  }
  fputs ("FLAGS (",stdout);	/* output attribute */
				/* output system flags */
  if (elt->recent) put_flag (&c,"\\Recent");
  if (elt->seen) put_flag (&c,"\\Seen");
  if (elt->deleted) put_flag (&c,"\\Deleted");
  if (elt->flagged) put_flag (&c,"\\Flagged");
  if (elt->answered) put_flag (&c,"\\Answered");
  if (elt->draft) put_flag (&c,"\\Draft");
  if (u = elt->user_flags) do	/* any user flags? */
    if (t = stream->user_flags[find_rightmost_bit (&u)]) 
	put_flag (&c,t);
  while (u);			/* until no more user flags */
  putchar (')');		/* end of flags */
  elt->spare2 = NIL;		/* we've sent the update */
}


/* Output a flag
 * Accepts: pointer to current delimiter character
 *	    flag to output
 * Changes delimiter character to space
 */

void put_flag (int *c,char *s)
{
  if (*c)
    putchar (*c);		/* put delimiter */
  fputs (s,stdout);		/* dump flag */
  *c = ' ';			/* change delimiter if necessarnew */
}


/* Output flags if was unseen
 * Accepts: message number
 *	    prior value of Seen flag
 */

void changed_flags (unsigned long i,int f, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  if (!f) {			/* was unseen? */
    putchar (' ');		/* yes, delimit with space */
    fetch_flags (i,NIL, imapdvars, stream);/* output flags */
  }
}

/* Fetch message internal date
 * Accepts: message number
 *	    extra argument
 */

void fetch_internaldate (unsigned long i,STRINGLIST *a, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  char tmp[MAILTMPLEN];
  MESSAGECACHE *elt = mail_elt (stream,i);
  if (!elt->day) {		/* have internal date yet? */
    sprintf (tmp,"%lu",i);
    mail_fetchfast (stream,tmp);
  }
  printf ("INTERNALDATE \"%s\"",mail_date (tmp,elt));
}


/* Fetch unique identifier
 * Accepts: message number
 *	    extra argument
 */

void fetch_uid (unsigned long i,STRINGLIST *a, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  printf ("UID %lu",mail_uid (stream,i));
}

/* Yes, I know the double fetch is bletcherous.  But few clients do this. */

/* Fetch complete RFC-822 format message
 * Accepts: message number
 *	    extra argument
 */

void fetch_rfc822 (unsigned long i,STRINGLIST *a, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream )
{
  char *s,c;
  unsigned long size,k;
  int f = mail_elt (stream,i)->seen;
  s = mail_fetchtext_full (stream,i,&size,FT_INTERNAL);
  k = add_count_lfs(s,size);
  s = mail_fetchheader_full (stream,i,NIL,&size,FT_INTERNAL);
  k += add_count_lfs(s,size);
  printf ("RFC822 {%lu}\015\012",k);
  fflush(stdout);

  /* SUN Imap4 - out of band check */
  if (!put_the_text (fileno(stdout),s,size,NIL)) {
    imapdvars->response = lose;	/* received out-of-band BYTE */
    if (imapdvars->lsterr) fs_give ((void **)&imapdvars->lsterr);
    imapdvars->lsterr = cpystr("Message data flushed");
    return;
  }
  s = mail_fetchtext_full (stream,i,&size,FT_INTERNAL);
  if (put_the_text (fileno(stdout),s,size,NIL))
    changed_flags (i,f, imapdvars, stream);/* output changed flags */
  else {
    imapdvars->response = lose;	/* received out-of-band BYTE */
    if (imapdvars->lsterr) fs_give ((void **)&imapdvars->lsterr);
    imapdvars->lsterr = cpystr("Message data flushed");
  }
}


/* Fetch complete RFC-822 format message, peeking
 * Accepts: message number
 *	    extra argument
 */

void fetch_rfc822_peek (unsigned long i,STRINGLIST *a, 
        IMAPD_VARS *imapdvars, MAILSTREAM *stream )
{
  char *s,c;
  unsigned long size,k;
					/* Get the size of the body */
  s = mail_fetchtext_full (stream,i,&size,FT_PEEK+FT_INTERNAL);
  k = add_count_lfs(s,size);
  s = mail_fetchheader_full (stream,i,NIL,&size,FT_INTERNAL);
  k += add_count_lfs(s,size);
  printf ("RFC822 {%lu}\015\012",k);
  fflush(stdout);

  /* SUN Imap4 - out of band check */
  if (!put_the_text (fileno(stdout),s,size,NIL)) {
    imapdvars->response = lose;	/* received out-of-band BYTE */
    if (imapdvars->lsterr) fs_give ((void **)&imapdvars->lsterr);
    imapdvars->lsterr = cpystr("Message data flushed");
    return;
  }
  s = mail_fetchtext_full (stream,i,&size,FT_PEEK+FT_INTERNAL);
  if (!put_the_text (fileno(stdout),s,size,NIL)) {
    imapdvars->response = lose;	/* received out-of-band BYTE */
    if (imapdvars->lsterr) fs_give ((void **)&imapdvars->lsterr);
    imapdvars->lsterr = cpystr("Message data flushed");
  }
}

/* Fetch RFC-822 header
 * Accepts: message number
 *	    extra argument
 */

void fetch_rfc822_header (unsigned long i,STRINGLIST *a, 
       IMAPD_VARS *imapdvars, MAILSTREAM *stream )
{
  char *s,c;
  unsigned long size,k;
  s = mail_fetchheader_full (stream,i,NIL,&size,FT_INTERNAL);
  k = add_count_lfs(s,size);
  printf ("RFC822.HEADER {%lu}\015\012",k);
  fflush(stdout);
  /* SUN Imap4 */
  if (!put_the_text (fileno(stdout),s,size,NIL)) {
    imapdvars->response = lose;	/* tell client we've flushed. */
    if (imapdvars->lsterr) fs_give ((void **)&imapdvars->lsterr);
    imapdvars->lsterr = cpystr("Message data flushed");
  }
}


/* Fetch RFC-822 message length
 * Accepts: message number
 *	    extra argument
 */

void fetch_rfc822_size (unsigned long i,STRINGLIST *a,
           IMAPD_VARS *imapdvars, MAILSTREAM *stream )
{
  MESSAGECACHE *elt = mail_elt (stream,i);
  if (!elt->rfc822_size) {	/* have message size yet? */
    char tmp[MAILTMPLEN];
    sprintf (tmp,"%lu",i);
    mail_fetchfast (stream,tmp);
  }
  printf ("RFC822.SIZE %lu",elt->rfc822_size);
  fflush(stdout);
}

/* Fetch RFC-822 text only
 * Accepts: message number
 *	    extra argument
 */

void fetch_rfc822_text (unsigned long i,STRINGLIST *a,
            IMAPD_VARS *imapdvars, MAILSTREAM *stream )
{
  char c,*s;
  unsigned long size,k;
  int f = mail_elt (stream,i)->seen;
  s = mail_fetchtext_full (stream,i,&size,FT_INTERNAL);
  k = add_count_lfs(s,size);
  printf ("RFC822.TEXT {%lu}\015\012",k);
  fflush(stdout);
  /* SUN Imap4 */
  if (put_the_text (fileno(stdout),s,size,NIL))
    changed_flags (i,f, imapdvars,stream);/* output changed flags */
  else {
    imapdvars->response = lose;	/* tell client we've flushed. */
    if (imapdvars->lsterr) fs_give ((void **)&imapdvars->lsterr);
    imapdvars->lsterr = cpystr("Message data flushed");
  }
}


/* Fetch RFC-822 text only, peeking
 * Accepts: message number
 *	    extra argument
 */

void fetch_rfc822_text_peek (unsigned long i,STRINGLIST *a,
          IMAPD_VARS *imapdvars, MAILSTREAM *stream )
{
  char c,*s;
  unsigned long size,k;
  s = mail_fetchtext_full (stream,i,&size,FT_PEEK+FT_INTERNAL);
  k = add_count_lfs(s,size);
  printf ("RFC822.TEXT {%lu}\015\012",k);
  fflush(stdout);
  /* SUN Imap4 */
  if (!put_the_text (fileno(stdout),s,size,NIL)) {
    imapdvars->response = lose;	/* tell client we've flushed. */
    if (imapdvars->lsterr) fs_give ((void **)&imapdvars->lsterr);
    imapdvars->lsterr = cpystr("Message data flushed");
  }
}

/* Print envelope
 * Accepts: body
 */

void penv (ENVELOPE *env, IMAPD_VARS *imapdvars )
{
  if (env) {			/* only if there is an envelope */
				/* disgusting MacMS kludge */
    if (imapdvars->mackludge) printf ("%lu ",cstring (env->date) + cstring (env->subject)
			   + caddr (env->from) + caddr (env->sender) +
			   caddr (env->reply_to) + caddr (env->to) +
			   caddr (env->cc) + caddr (env->bcc) +
			   cstring (env->in_reply_to) +
			   cstring (env->message_id));
    putchar ('(');		/* delimiter */
    pstring (env->date);	/* output envelope fields */
    putchar (' ');
    pstring (env->subject);
    putchar (' ');
    paddr (env->from);
    putchar (' ');
    paddr (env->sender);
    putchar (' ');
    paddr (env->reply_to);
    putchar (' ');
    paddr (env->to);
    putchar (' ');
    paddr (env->cc);
    putchar (' ');
    paddr (env->bcc);
    putchar (' ');
    pstring (env->in_reply_to);
    putchar (' ');
    pstring (env->message_id);
    putchar (')');		/* end of envelope */
  }
  else fputs ("NIL",stdout);	/* no envelope */
}

/* Print body structure (extensible)
 * Accepts: body
 */

void pbodystructure (BODY *body,IMAPD_VARS *imapdvars )
{
  if (body) {			/* only if there is a body */
    PARAMETER *param;
    PART *part;
    putchar ('(');		/* delimiter */
				/* multipart type? */
    if (body->type == TYPEMULTIPART) {
				/* print each part */
      if (part = body->contents.part)
	for (; part; part = part->next) pbodystructure (&(part->body), imapdvars);
      else fputs ("(\"TEXT\" \"PLAIN\" NIL NIL NIL \"7BIT\" 0 0)",stdout);
      putchar (' ');		/* space delimiter */
      pstring (body->subtype);	/* subtype */
				/* multipart body extension data */
      if (param = body->parameter) {
	fputs (" (",stdout);
	do {
	  pstring (param->attribute);
	  putchar (' ');
	  pstring (param->value);
	  if (param = param->next) putchar (' ');
	} while (param);
	putchar (')');		/* end of parameters */
      }
      else fputs (" NIL",stdout);
    }

    else {			/* non-multipart body type */
      pstring ((char *) body_types[body->type]);
      putchar (' ');
      pstring (body->subtype);
      if (param = body->parameter) {
	fputs (" (",stdout);
	do {
	  pstring (param->attribute);
	  putchar (' ');
	  pstring (param->value);
	  if (param = param->next) putchar (' ');
	} while (param);
	fputs (") ",stdout);
      }
      else fputs (" NIL ",stdout);
      pstring (body->id);
      putchar (' ');
      pstring (body->description);
      putchar (' ');
      pstring ((char *) body_encodings[body->encoding]);
      printf (" %lu",body->size.bytes);
      switch (body->type) {	/* extra stuff depends upon body type */
      case TYPEMESSAGE:
				/* can't do this if not RFC822 */
	if (strcmp (body->subtype,"RFC822")) break;
	putchar (' ');
	penv (body->contents.msg.env, imapdvars);
	putchar (' ');
	pbodystructure (body->contents.msg.body, imapdvars);
      case TYPETEXT:
	printf (" %lu",body->size.lines);
	break;
      default:
	break;
      }
      putchar (' ');
      pstring (body->md5);
    }
    putchar (')');		/* end of body */
  }
  else fputs ("NIL",stdout);	/* no body */
}

/* Print body (non-extensible)
 * Accepts: body
 */

void pbody (BODY *body, IMAPD_VARS *imapdvars )
{
  if (body) {			/* only if there is a body */
    PARAMETER *param;
    PART *part;
    putchar ('(');		/* delimiter */
				/* multipart type? */
    if (body->type == TYPEMULTIPART) {
				/* print each part */
      if (part = body->contents.part)
	for (; part; part = part->next) pbody (&(part->body), imapdvars);
      else fputs ("(\"TEXT\" \"PLAIN\" NIL NIL NIL \"7BIT\" 0 0)",stdout);
      putchar (' ');		/* space delimiter */
      pstring (body->subtype);	/* and finally the subtype */
    }
    else {			/* non-multipart body type */
      pstring ((char *) body_types[body->type]);
      putchar (' ');
      pstring (body->subtype);
      if (param = body->parameter) {
	fputs (" (",stdout);
	do {
	  pstring (param->attribute);
	  putchar (' ');
	  pstring (param->value);
	  if (param = param->next) putchar (' ');
	} while (param);
	fputs (") ",stdout);
      }
      else fputs (" NIL ",stdout);
      pstring (body->id);
      putchar (' ');
      pstring (body->description);
      putchar (' ');
      pstring ((char *) body_encodings[body->encoding]);
      printf (" %lu",body->size.bytes);
      switch (body->type) {	/* extra stuff depends upon body type */
      case TYPEMESSAGE:
				/* can't do this if not RFC822 */
	if (strcmp (body->subtype,"RFC822")) break;
	putchar (' ');
	penv (body->contents.msg.env,imapdvars);
	putchar (' ');
	pbody (body->contents.msg.body, imapdvars);
      case TYPETEXT:
	printf (" %lu",body->size.lines);
	break;
      default:
	break;
      }
    }
    putchar (')');		/* end of body */
  }
  else fputs ("NIL",stdout);	/* no body */
}

/* Print string
 * Accepts: string
 */

void pstring (char *s)
{
  char c,*t;
  if (s) {			/* is there a string? */
				/* must use literal string */
    if (strpbrk (s,"\012\015\"\\")) {
      printf ("{%lu}\015\012",(unsigned long) strlen (s));
      fputs (s,stdout);		/* don't merge this with the printf() */
    }
    else {			/* may use quoted string */
      putchar ('"');		/* don't even think of merging this into a */
      fputs (s,stdout);		/*  printf().  Cretin VAXen can't do a */
      putchar ('"');		/*  printf() of godzilla strings! */
    }
  }
  else fputs ("NIL",stdout);	/* empty string */
}


/* Print address list
 * Accepts: address list
 */

void paddr (ADDRESS *a)
{
  if (a) {			/* have anything in address? */
    putchar ('(');		/* open the address list */
    do {			/* for each address */
      putchar ('(');		/* open the address */
      pstring (a->personal);	/* personal name */
      putchar (' ');
      pstring (a->adl);		/* at-domain-list */
      putchar (' ');
      pstring (a->mailbox);	/* mailbox */
      putchar (' ');
      pstring (a->host);	/* domain name of mailbox's host */
      putchar (')');		/* terminate address */
    } while (a = a->next);	/* until end of address */
    putchar (')');		/* close address list */
  }
  else fputs ("NIL",stdout);	/* empty address */
}

/* Count string and space afterwards
 * Accepts: string
 * Returns: 1 plus length of string
 */

long cstring (char *s)
{
  char tmp[MAILTMPLEN];
  unsigned long i = s ? strlen (s) : 0;
  if (s) {			/* is there a string? */
				/* must use literal string */
    if (strpbrk (s,"\012\015\"%{\\")) {
      sprintf (tmp,"{%lu}\015\012",i);
      i += strlen (tmp);
    }
    else i += 2;		/* quoted string */
  }
  else i += 3;			/* NIL */
  return i + 1;			/* return string plus trailing space */
}


/* Count address list and space afterwards
 * Accepts: address list
 */

long caddr (ADDRESS *a)
{
  long i = 3;			/* open, close, and space */
				/* count strings in address list */
  if (a) do i += 1 + cstring (a->personal) + cstring (a->adl) +
    cstring (a->mailbox) + cstring (a->host);
  while (a = a->next);		/* until end of address */
  else i = 4;			/* NIL plus space */
  return i;			/* return the count */
}

/* Anonymous users may only use these mailboxes in these namespaces */

char *oktab[] = {"#news.", "#ftp/", 0};


/* Check if mailbox name is OK
 * Accepts: reference name
 *	    mailbox name
 */

long nameok (char *ref,char *name, IMAPD_VARS *imapdvars)
{
	/* A case where We don't need to pass a stream. -clin */ 
  int i;
  if (!name) return NIL;	/* failure if missing name */
  if (!imapdvars->anonymous) return T;	
			/* otherwise OK if not imapdvars->anonymous */
				/* validate reference */
  if (ref) for (i = 0; oktab[i]; i++)
    if (!strncmp (ref,oktab[i],strlen (oktab[i]))) {
      if (*name == '#') break;	/* reference OK, any override? */
      else return T;		/* no, we're done */
    }
  for (i = 0; oktab[i]; i++)	/* validate mailbox */
    if (!strncmp (name,oktab[i],strlen (oktab[i]))) return T;
  imapdvars->response = 
		"%s NO Anonymous may not %s this name\015\012";
  return NIL;
}

/* IMAP4 Authentication responder
 * Accepts: challenge
 * Returns: response
 */

#define RESPBUFLEN 8*MAILTMPLEN

char *imap_responder (void *challenge,unsigned long clen,unsigned long *rlen,
      void *imapdvars, MAILSTREAM *stream)
{
  unsigned long i;
  unsigned char *t,resp[RESPBUFLEN];
  t = rfc822_binary (challenge,clen,&i);
  printf ("+ %s",t);
  fflush (stdout);		/* dump output buffer */
				/* slurp imapdvars->response buffer */
  slurp ((char *) resp,RESPBUFLEN-1, imapdvars, stream);
  if (!(t = (unsigned char *) strchr ((char *) resp,'\012'))) 
	return flush(imapdvars,stream);
  if (t[-1] == '\015') --t;	/* remove CR */
  *t = '\0';			/* tie off buffer */
  return (resp[0] != '*') ? (char *)rfc822_base64 (resp,t-resp,rlen ? 
						   rlen : &i) : NIL;
}

/* Co-routines from MAIL library */


/* Message matches a search
 * Accepts: MAIL stream
 *	    message number
 */

void mm_searched (MAILSTREAM *s,unsigned long msgno)
{
				/* nothing to do here */
}


/* Message exists (i.e. there are that many messages in the mailbox)
 * Accepts: MAIL stream
 *	    message number
 */

void mm_exists (MAILSTREAM *stream,unsigned long number)
{
  IMAPD_VARS *iv;
  if (stream ) iv = (IMAPD_VARS *) stream->userdata;
    else return;  /* To avoid core dump. -clin */
  if (stream != iv->tstream) { /* note change in number of messages */
    printf ("* %lu EXISTS\015\012",(iv->nmsgs = number));
    iv->recent = -1;		
			/* make sure fetch new recent count */
  }
}


/* Message expunged
 * Accepts: MAIL stream
 *	    message number
 */
void mm_expunged (MAILSTREAM *stream,unsigned long number)
{
  IMAPD_VARS *iv;

  if (stream ) iv = (IMAPD_VARS *) stream->userdata;
    else return;
  if (stream != iv->tstream) printf ("* %lu EXPUNGE\015\012",number);
}


/* Message status changed
 * Accepts: MAIL stream
 *	    message number
 */

void mm_flags (MAILSTREAM *stream,unsigned long number)
{
  IMAPD_VARS *iv;

  if (stream )  iv = (IMAPD_VARS *) stream->userdata;
    else return;
  if (stream != iv->tstream) mail_elt (stream,number)->spare2 = T;
}

/* Mailbox found
 * Accepts: hierarchy delimiter
 *	    mailbox name
 *	    attributes
 */

void mm_list (MAILSTREAM *stream,char delimiter,char *name,long attributes)
{
  mm_list_work ("LIST",delimiter,name,attributes, stream);
}


/* Subscribed mailbox found
 * Accepts: hierarchy delimiter
 *	    mailbox name
 *	    attributes
 */

void mm_lsub (MAILSTREAM *stream,char delimiter,char *name,long attributes)
{
  mm_list_work ("LSUB",delimiter,name,attributes, stream);
}
/* Mailbox status
 * Accepts: MAIL stream
 *	    mailbox name
 *	    mailbox status
 */

void mm_status (MAILSTREAM *stream,char *mailbox,MAILSTATUS *status)
{
  IMAPD_VARS *iv;
  char tmp[MAILTMPLEN];

  if (stream ) iv = (IMAPD_VARS *) stream->userdata;
    else return;
  tmp[0] = tmp[1] = '\0';
  if (status->flags & SA_MESSAGES)
    sprintf (tmp + strlen (tmp)," MESSAGES %ld",status->messages);
  if (status->flags & SA_RECENT)
    sprintf (tmp + strlen (tmp)," RECENT %ld",status->recent);
  if (status->flags & SA_UNSEEN)
    sprintf (tmp + strlen (tmp)," UNSEEN %ld",status->unseen);
  if (status->flags & SA_UIDNEXT)
    if (iv->imap4rev1)
      sprintf (tmp + strlen (tmp)," UIDNEXT %ld",status->uidnext);
    else
      sprintf (tmp + strlen (tmp)," UID-NEXT %ld",status->uidnext);
  if (status->flags & SA_UIDVALIDITY)
    if (iv->imap4rev1)
      sprintf (tmp+strlen(tmp)," UIDVALIDITY %ld",status->uidvalidity);
    else
      sprintf (tmp+strlen(tmp)," UID-VALIDITY %ld",status->uidvalidity);
  /* Imap4 Sunversion */
  if (status->flags & SA_CHECKSUM)
    sprintf (tmp + strlen(tmp), " CHECKSUM %05d%ld", status->checksum,
	     status->checksum_bytes);
  printf ("* STATUS %s (%s)\015\012",mailbox,tmp+1);
}

/* Worker routine for LIST and LSUB
 * Accepts: name of response
 *	    hierarchy delimiter
 *	    mailbox name
 *	    attributes
 */

void mm_list_work (char *what,char delimiter,char *name,long attributes,
		MAILSTREAM *stream)
{
  IMAPD_VARS *iv;
  char *s,tmp[MAILTMPLEN];
 
  if (stream) iv = (IMAPD_VARS *)stream->userdata;
   else return;

  if (iv->finding) {
    printf ("* MAILBOX %s",name);
    /* Find all.mailboxes terminates directory strings with
     * a "/" */
    if (attributes & LATT_NOSELECT) {
      char *last = name + strlen(name) - 1;
      if (delimiter)
	if (*last != delimiter) putchar (delimiter);
    }
  } else {			/* new form */
    tmp[0] = tmp[1] = '\0';
    if (attributes & LATT_NOINFERIORS) strcat (tmp," \\NoInferiors");
    if (attributes & LATT_NOSELECT) strcat (tmp," \\NoSelect");
    if (attributes & LATT_MARKED) strcat (tmp," \\Marked");
    if (attributes & LATT_UNMARKED) strcat (tmp," \\UnMarked");
    tmp[0] = '(';		/* put in real delimiter */
    if (delimiter) sprintf (tmp + strlen (tmp),") \"%c\"",delimiter);
    else strcat (tmp,") NIL");
    printf ("* %s %s ",what,tmp);
				/* must use literal string */
    if (strpbrk (name,"\012\015\"\\")) {
      printf ("{%lu}\015\012",(unsigned long) strlen (name));
      fputs (name,stdout);	/* don't merge this with the printf() */
    }
    else {			/* either quoted string or atom */
      for (s = name;		/* see if anything that requires quoting */
	   (*s > ' ') && (*s != '(') && (*s != ')') && (*s != '{'); ++s);
      if (*s) {			/* must use quoted string */
	putchar ('"');		/* don't even think of merging this into a */
	fputs (name,stdout);	/*  printf().  Cretin VAXen can't do a */
	putchar ('"');		/*  printf() of godzilla strings! */
      }
      else fputs (name,stdout);	/* else plop down as atomic */
    }
  }
  fputs ("\015\012",stdout);
}

/* Notification event
 * Accepts: MAIL stream
 *	    string to log
 *	    error flag
 */

void mm_notify (MAILSTREAM *stream,char *string,long errflg)
{
  char tmp[MAILTMPLEN];
  IMAPD_VARS *iv;

  if (stream ) iv = (IMAPD_VARS *)stream->userdata;
    else return;

  if (!iv->tstream || (stream != iv->tstream)) switch (errflg) {
  case NIL:		/* information message, set as OK imapdvars->response */
    tmp[11] = '\0';		/* see if TRYCREATE for MacMS kludge */
    if (!strcmp (ucase (strncpy (tmp,string,11)),"[TRYCREATE]")) 
	iv->trycreate = T;
  case PARSE:			/* parse glitch, output unsolicited OK */
    printf ("* OK %s\015\012",string);
    break;
  case WARN:			/* warning, output unsolicited NO (kludge!) */
    printf ("* NO %s\015\012",string);
    break;
  case ERROR:			/* error that broke command */
  default:			/* default should never happen */
    printf ("* BAD %s\015\012",string);
    break;
  }
  fflush(stdout);
}

/* Log an event for the user to see
 * Accepts: string to log
 *	    error flag
 */

void mm_log (char *string,long errflg, MAILSTREAM *stream)
{
/* Note that if the errflg is either PARSE or WARN, then
no imapdvars need to be referenced. Thus in those cases, it
is OK to pass a NIL stream in c-client libraries.! -clin */
  IMAPD_VARS *iv;

/* Since we allow a NIL stream, we'd better test for the
stream before casting it. -clin */

  if (stream) iv = (IMAPD_VARS *)stream->userdata;
 
  switch (errflg) {		/* action depends upon the error flag */
  case NIL:			/* information message, set as OK response */
    if (iv->response == win) {	/* only if no other imapdvars->response yet */
      iv->response = altwin;	/* switch to alternative win message */

	/* NOTICE: the global on the stream is being updated. -clin */
      if (stream && iv->lsterr)  
	fs_give ((void **) &iv->lsterr);
         iv->lsterr= cpystr (string);	

    }
    break;
  case PARSE:			/* parse glitch, output unsolicited OK */
    printf ("* OK [PARSE] %s\015\012",string);
    break;
  case WARN:			/* warning, output unsolicited NO (kludge!) */
    if (strcmp (string,"Mailbox is empty")) printf ("* NO %s\015\012",string);
    break;
  case ERROR:			/* error that broke command */
  default:			/* default should never happen */
    iv->response = lose;		/* set fatality */

    if (stream && iv->lsterr )  
    fs_give ((void **) &iv->lsterr); /* flush old error */
    iv->lsterr = cpystr (string);   /* note last error */

    break;
  }
  fflush(stdout);
}
/*
 * Not used here... */
void mm_log_stream (MAILSTREAM *stream, char *msg)
{
  return;
}

/* Log an event to debugging telemetry
 * Accepts: string to log
 */

void mm_dlog (MAILSTREAM *stream,char *string)
{
  mm_log (string,WARN, stream);		/* shouldn't happen normally */
}

/* Get user name and password for this host
 * Accepts: host name
 *	    where to return user name
 *	    where to return password
 *	    trial count
 */

void mm_login (MAILSTREAM *stream,NETMBX *mbx,char *username,
	       char *password,long trial)
{
  IMAPD_VARS *iv;

  if (stream ) iv = (IMAPD_VARS *)stream->userdata;
    else return;

  strcpy (username,iv->user);/* set user name */
  strcpy (password,iv->pass);/* and password */
}

/* About to enter critical code
 * Accepts: stream
 */

void mm_critical (MAILSTREAM *s)
{
  /* Not doing anything here for now */
}


/* About to exit critical code
 * Accepts: stream
 */

void mm_nocritical (MAILSTREAM *s)
{
  /* Not doing anything here for now */
}


/* Disk error found
 * Accepts: stream
 *	    system error code
 *	    flag indicating that mailbox may be clobbered
 * Returns: abort flag
 */

long mm_diskerror (MAILSTREAM *stream,long errcode,long serious)
{
  IMAPD_VARS *iv;
  char tmp[MAILTMPLEN];

  if (stream ) iv = (IMAPD_VARS *)stream->userdata;
    else return _B_FALSE;
  if (serious) {		/* try your damnest if clobberage likely */
    mm_notify (stream,"Disk error: Retrying ... ",ERROR);
    fflush (stdout);		/* dump output buffer */
    syslog (LOG_ALERT,
	    gettext("Retrying after disk error user=%s host=%s mbx=%s: %s"),
	    iv->user, tcp_clienthost (tmp),
	    (stream && stream->mailbox) ? stream->mailbox : gettext("???"),
	    strerror (errcode));
    alarm (0);			/* make damn sure timeout disabled */
    sleep (60);			/* give it some time to clear up */
    return NIL;
  }
				/* otherwise die before more damage is done */
  printf ("* BYE Aborting due to disk error %s\015\012",strerror (errcode));
  syslog (LOG_ALERT,
	  gettext("Fatal disk error user=%s host=%s mbx=%s: %s"),
	  iv->user,tcp_clienthost (tmp),
	  (stream && stream->mailbox) ? stream->mailbox : gettext("???"),
	  strerror (errcode));
  return T;
}


/* Log a fatal error event
 * Accepts: string to log
 */

void mm_fatal (char *string, MAILSTREAM *stream)
{
  char tmp[MAILTMPLEN];
  IMAPD_VARS *iv;

  if (stream ) iv = (IMAPD_VARS *)stream->userdata;

  printf ("* BYE [ALERT] IMAP4 server crashing: %s\015\012",string);
  if (stream ) /* In case we didn't pass a stream. -clin */
	syslog (LOG_ALERT,
		gettext("Fatal error user=%s host=%s mbx=%s: %s"),
	  	iv->user ? iv->user :
		gettext("???"),tcp_clienthost (tmp),
	  	(stream && stream->mailbox) ? stream->mailbox : gettext("???"),
		string);

}
/* SUN Imap4 */
/* for compatibility with os_SV5: WJY 9-jun-95 */
void mm_tcp_log(MAILSTREAM *stream, char *msg)
{
  return;
}

void mm_io_callback(MAILSTREAM *stream, long nbytes, int type)
{
  return;
}

/* SUN IMAP4 commands here */

/* Received SUNVERSION cmd from client.
 * Update our state */
void sunversion(char **arg, IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  IMAPD_VARS *iv = (IMAPD_VARS *)stream->userdata;
  char *option;
  char *soleil;

  imapdvars->sunversion_set = T; /* Nous sommes le soleil! */
  /* Mabye after the select */
  if (stream && (stream->stream_status & S_OPENED))
    stream->sunvset = T;
  /* see if options present 
   *  1. MIME YES/NO */
  option = snarf(arg, imapdvars, stream);
  if (option && (strcasecmp(option, "MIME") == 0)) {
    option = snarf(arg, imapdvars, stream);
    if (option && (strcasecmp(option, "YES") == 0)) 
      imapdvars->keep_mime = T;		       /* write mime back to mail file  */
    else
      imapdvars->keep_mime = NIL;		       /* default setting */
  } else
    imapdvars->keep_mime = KEEPMIMEDEFAULT;       /* default setting */
  if (imapdvars->keep_mime)
    soleil = "Version du Soleil (Saving MIME translations)";
  else
    soleil = "Version du Soleil (NOT saving  MIME translations)";	    
  mm_notify (stream,soleil,PARSE);              /* Just say OK -- VF */
}

/*
 * stub only */
void mm_cache_size (MAILSTREAM *stream,unsigned long size,
		    int done)
{
  return;
}
void mm_restore_callback (void *userdata, unsigned long size,
			  unsigned long count)
{
  return;
}

/* return 
 * SHORTINFO ("internaldate" (("personal" NIL "mbox" "host")) "Subject" "Bodytype" 
 * "rfc822.size") FLAGS ()
 * Fetch short instead of structure. 16-aug-96. Prevents body
 * parse. */
void fetch_shortinfo(long i, char *s, 
		IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  SHORTINFO sinfo;		/* just get a pointer into the sinfo */
  unsigned short type;
  int text_plain;
  char tmp[TMPLEN];

  mail_fetchshort_full (stream,i,&sinfo,NIL);
  printf("SHORTINFO (");
  pstring(sinfo.date);
  putchar(' ');
  /* See if we have a from. mailbox required */
  if (sinfo.mailbox != NIL) {
    printf("((");
    if (sinfo.personal != NIL) pstring(sinfo.personal);
    else printf("NIL");
    printf(" NIL ");		/* no adl */
    printf("\"%s\" ",sinfo.mailbox); /* required */
    if (sinfo.host != NIL) printf("\"%s\"))",sinfo.host);
    else printf("NIL))");
  } else {
    /* Allez chercher ce truc-la
     * ((NIL NIL mbox host)) from mbox@host */
    char *bien_cache= mail_fabricate_from(stream, i);
    char *mbox;
    char *host= NIL;
    char *la;

    mbox = bien_cache;
    if (mbox) {
      la = strchr(mbox, '@');
      if (la) {
	*la++ = '\0';
	host = la;
      }
    }
    printf("((NIL NIL ");               /* No personal name/adl */
    if (mbox) pstring(mbox);
    else printf("NIL");
    if (host) printf(" \"%s\"))", host);
    else printf(" NIL))");
    fs_give((void **)&bien_cache); /* free the string */
  }
  putchar(' ');
  pstring(sinfo.subject);
  putchar(' ');
  pstring(sinfo.bodytype);
  putchar(' ');
  pstring(sinfo.size);
  putchar(')');				/* terminate shortinfo */
  /* Aussi, on envoit les drapeaux, yes, the flags */
  putchar(' ');	/* d'espace entre nous suffit. */
  fetch_flags (i, NIL, imapdvars, stream);
  fflush(stdout);
}


/* Utility for sending text awaiting possible urgent signal */
static int insert_cr(char *dbuf,char *src,unsigned long srclen)
{
  char *d = dbuf;
  while (srclen--) {
    switch (*src) {
    case '\015':		/* unlikely carriage return */
      *d++ = *src++;		/* copy it and any succeeding linefeed */
      if (srclen && *src == '\012') {
	*d++ = *src++;
	srclen--;
      }
      break;
    case '\012':		/* line feed? */
      *d++ ='\015';		/* yes, prepend a CR, drop into default case */
    default:			/* ordinary chararacter */
      *d++ = *src++;		/* just copy character */
      break;
    }
  }
  return d - dbuf;		/* the length */
}

#define MORCEAU 1024
int put_the_text (int outfd, char *text, long n, int binary)
{
  int morceaux= n/MORCEAU;	/* the chunks to send */
  int le_reste= (n % MORCEAU);	/* n = k*1024 + r. This is r. */
  int i;
  int infd= fileno(stdin);	/* for our input */
  char dbuf[(2*MORCEAU)+16];	/* for the cr inserts */
  unsigned long nchars;
  for (i = 0; i < morceaux; i++) {
    int ecrit;
    if (binary) {
      nchars = MORCEAU;		/* Then no cr stuffing needed */
      ecrit = write(outfd,text,nchars);
    } else {
      nchars = insert_cr(dbuf,text,MORCEAU);
      ecrit = write(outfd,dbuf,nchars);
    }
    if (ecrit != nchars) return T; /* i/o error ... give to upper levels */
    /* Look for an urgent byte */
    if (received_urgent_data(fileno(stdin), outfd))
      return NIL;			/* URGENT RCVD. STOP sending */
    else
      text += MORCEAU;
  }
  if (le_reste) {
    if (binary) 			/* just blat the rest */
      write(outfd,text,le_reste);
    else {				/* insert CRs */
      nchars = insert_cr(dbuf,text,le_reste);
      write(outfd,dbuf,nchars);		/* the rest is sent */
    }
  }
  return T;
}

#ifdef LOGURGENTMERDE

FILE *lurgence;
void open_urgent_info(void)
{
  lurgence = fopen("/tmp/imapd_urgent.log", "w");
}

#else

FILE *lurgence = NIL;

#endif

void log_urgent_info(char *str)
{
#ifdef LOGURGENTMERDE
  if (lurgence) {
    fwrite(str, "%s", lurgence);
    fflush(lurgence);
  }
#else
  syslog (LOG_INFO, "%s", str);
#endif
}

int urgent(int infd, int outfd);

/* See if we received the urgent byte */
int received_urgent_data (int infd, int outfd)
{
  fd_set exceptfds;
  int selectioner;
  int nfds= infd + 1;
  struct timeval tv;

  FD_ZERO (&exceptfds);
  FD_SET (infd, &exceptfds);

  tv.tv_sec = tv.tv_usec = 0;	/* a poll only */
  selectioner = select (nfds, 0, 0, &exceptfds, &tv);
  if (selectioner <= 0) {	/* error or TIMEOUT */
    /* IF an error then upper levels should detect it */
    return NIL;
  } else if (FD_ISSET(infd, &exceptfds)) { /* should be! */
    log_urgent_info(gettext("Urgent byte exception detected"));
    reply_to_urgent(infd, outfd); /* read the urgent byte and send one back */
    return T;
  }
  return NIL;			/* timed out  */
}

/*
 * We read the urgent byte that has arrived, and then
 * send a reply.
 *
 * In this situation there should only be ONE byte in the stream
 * but we have to be careful to still read up to the urgent byte.
 */
#define URGBUFL 64
#define DEUXMINS 2*60*1000

int reply_to_urgent (int infd, int outfd)
{
  char buf[URGBUFL];
  long t0= elapsed_ms();
  long t1;
  char mark_byte[2];
  int apres;

  while (1) {
    int mark= 0;
    int n;

    if (ioctl(infd, SIOCATMARK, &mark) < 0) {
      return 0;			/* ioctl error */
    }
   
    if (mark) {			/* at the mark now */
      break;
    } else {			/* read some more data */
      n = read(infd, buf, URGBUFL); /* read upto mark */
      if (n <= 0) {
	return 0;		/* i/o error. Forget it. */
      }
    }
    t1 = elapsed_ms() - t0;	/* milliseconds bien passe. */
    if (t1 > DEUXMINS)
      return 0;			/* Trop c'est trop. Enough is enough. */
  }
  /* READ the mark byte */
  if (recv(infd, mark_byte, 1, MSG_OOB) < 0) {
    return 0;			/* i/o error */
  } else {
    /* Only send the urgent byte because our callers
     * send the rest of the command reply. This will clear
     * the stream */
    if (send(outfd, "S", 1, MSG_OOB) < 0) {
      log_urgent_info(gettext("Send urgent Failed"));
    }
  }
  /* READ ONE byte after urgent byte to clear URGENT exception.
   * THERE is a solaris bug that necessitates this! Merde!! */
  apres = fgetc(stdin);
  return 1;
}

/*
 * select either input or an out of band byte
 * We assume that the out-of-band byte PRECEEDS any
 * valid input, and so, should not have to flush
 * to read it. We do of course have to try to flush.
 *
 * In this case we have already sent the 
 *  
 *  tag OK FETCH completed
 *
 * response. This will have been flushed in awaiting
 * our urgent reply. SO, we must also send the end
 * of command ")\015\102tag NO .....\015\12" stuff.
 *
 * This is for the case where the client has sent us
 * an urgent byte AFTER we have finished sending the data
 * that it wished to flush because it was still reading
 * the data. We should not receive such a byte under any
 * other conditions. */
void select_input (int infd, IMAPD_VARS *imapdvars,
		MAILSTREAM *stream)
{
  /* This is a case where both imapdvars and stream are needed to
  be passed in because the stream can be closed. After which the
  only place we can access the imapd_vars is from imapdvars. -clin */
 
  fd_set readfds, exceptfds;
  int selectioner;
  int nfds= infd + 1;

  if (baiser) {		/* kod SIGNAL received DURING a command */
    fputs ("* BYE 127 Kiss-of-death received\015\012",stdout);
    fflush (stdout);		/* make sure output blatted */
    if (imapdvars->state == OPEN) mail_close (stream);
    _exit (1);
  }

  while (1) {
    FD_ZERO (&exceptfds);
    FD_ZERO (&readfds);
    FD_SET (infd, &exceptfds);
    FD_SET (infd, &readfds);

    errno = 0;
    selectioner = select (nfds, &readfds, 0, &exceptfds, 0);
    if (selectioner < 0) {	
      if (errno == EINTR) {
	/* rodint/kodint/clkint caught here 
	 * Only act on kiss-of-death. Allez, au revoir.
         */
	if (baiser) {
	  fputs ("* BYE 127 Kiss-of-death received\015\012",stdout);
	  fflush(stdout);
	  if (imapdvars->state == OPEN)  mail_close (stream);
	  _exit(1);
	}
      } else {
	/* other error */
	char tmp[MAILTMPLEN];
	if (imapdvars->state == OPEN) {
	  fputs ("* 127 System error during select\015\012", stdout);
	  fflush(stdout);
	  mail_close (stream);
	}
	syslog (LOG_INFO,
		gettext("Connection broken while reading line user=%s host=%s"),
		imapdvars->user ? imapdvars->user : gettext("???"),
		tcp_clienthost (tmp));
	_exit(1);
      }
    } else if (FD_ISSET(infd, &exceptfds)) { /* out-of-band data  */
      /* 
       * read the urgent byte and send one back */
      if (reply_to_urgent(infd, fileno(stdout))) {
	/*
	 * Now the tail ... */
	printf (")\015\012");
	printf (lose, imapdvars->tag, "FETCH", "Message data flushed");
	fflush(stdout);
      }
    } if (FD_ISSET(infd, &readfds)) {
      /* Have data */
      return;
    }
  }
}

/*
 * Here we will copy the literal string to it's own arbitrary
 * buffer, append_buf, which is global */
int append_snarf (char **arg, char **name, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  long i;
  char *c = *arg;               /* should be {n} */
  char *s = c + 1;              /* should be n} */
  char *t = NIL;
  int success;
  char *lasts;

  if (!c) return NIL;		/* better be an argument */
  switch (*c) {			/* see what the argument is */
  case '\0':			/* catch bogons */
  case ' ':
    return NIL;
  case '"':			/* quoted string */
    return NIL;
  case '{':			/* literal string */
    if (isdigit (*s)) {		/* be sure about that */
      i = strtol (s, &c, 10);	/* get its length(c updated) */
				/* validate end of literal */
      if (*c != '}' || c[1]) return NIL; /* *c == '}' */
      fputs (argrdy,stdout);	/* tell client ready for argument */
      fflush (stdout);		/* dump output buffer */
      /*
       * make sure we have enough room in our append buffer */
      imapdvars->append_len = i + 16;
      imapdvars->append_buf = fs_mmap(&imapdvars->append_fd, i+16, name);
      if (imapdvars->append_fd < 0) return NIL;

      alarm (TIMEOUT);		/* start timeout */
      while (i--) {
	char ac = inchar (imapdvars, stream);
	if (fs_mmapput(imapdvars->append_fd, ac) < 0) {
	  fs_mumap(imapdvars->append_fd, *name, imapdvars->append_buf, imapdvars->append_len);
	  return NIL;
	}
      }
      alarm(0);                 /* Disable the alarm */
                                /* make sure string tied off */
      if (fs_mmapput(imapdvars->append_fd, NIL) < 0) {
	fs_mumap(imapdvars->append_fd, *name, imapdvars->append_buf, imapdvars->append_len);
	return NIL;
      }
      /* read available data possibly up to end-of-cmdbuf */
      slurp ((*arg = c), TMPLEN - (c - imapdvars->cmdbuf) - 1, imapdvars, stream);
      if (!strchr (c,'\012')) {	/* have a newline there? */
	imapdvars->response = toobig;	/* NO! lost it seems */
	fs_mumap(imapdvars->append_fd, *name, imapdvars->append_buf, imapdvars->append_len);
	flush(imapdvars,stream);
	return NIL;
      } else {
	/* reset strtok_r mechanism, tie off if done */	
	if (!strtok_r (c, "\015\012", &lasts)) *c = '\0';
	/* see if we have more data -- should NOT happen */
	if (*c) {
	  *c++ = '\0';
	  *arg = c;
	} else
	  *arg = NIL;
      }
      break;
    }
				/* otherwise fall through (third party IMAP) */
  default:			/* atomic string */
    return NIL;
  }
  return T;
}

/*
 * fetch_1flag: for the "(UID) store <sequence> +/-1FLAG <flag>"
 * command response
 *   sends
 *     * <sequence> FETCH (+/-1FLAG (FLAG))
 *     OR
 *     * <sequence> FETCH (UID <sequnce> +/-1FLAG (FLAG))
 */

void
fetch_1flag (char *cmd, char *sequence, char *flag, int uid, 
	IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
	/*Note that the cmd here is NOT the imapdvars->cmd on imapdvars. -clin */ 
  int noparens;
  int format;
  if (*flag == '(') 
    noparens = T;
  else
    noparens = NIL;
  if (uid) {
    /* First create a msg sequnce from the UID sequence */
    char *msg_sequence = mail_uidseq_to_sequence(stream, sequence);
    /* UID sequence may contain "*"s: We get a compressed sequence
     * back, ie, n,n+1,..,n+j becomes n:n+j  */
    char *uid_sequence = mail_uidseq (stream, sequence);
    if (*uid_sequence && *msg_sequence) {
      if (noparens)
	printf("* %s FETCH (UID %s %s %s)\015\012", msg_sequence, uid_sequence,
	       cmd, flag); 
      else
	printf("* %s FETCH (UID %s %s (%s))\015\012", msg_sequence, uid_sequence,
	       cmd, flag); 
    }
    fs_give ((void **)&uid_sequence);
    fs_give((void **)&msg_sequence);
  } else {
    /* That "*" thing again */
    char *msg_sequence = mail_msgseq (stream, sequence);
    /* If we have received a sunversion command and it is NOT imap4 */
    if (!imapdvars->imap4_sunversion && imapdvars->sunversion_set)
      printf("* %s %s %s\015\012", msg_sequence, cmd, flag); 
    else  /* Imap4 version */
      if (noparens)
	printf("* %s FETCH (%s %s)\015\012", msg_sequence, cmd, flag); 
      else
	printf("* %s FETCH (%s (%s))\015\012", msg_sequence, cmd, flag);
    fs_give((void **)&msg_sequence);
  }
  /* SUN Imap4 -- checksum:
   *  driver that did store sets send_checksum if all went well */
  format = (imapdvars->imap4_sunversion ? FORMATIMAP4STATUS : FORMAT2BIS);
  do_status_checksum(stream,format);
}

/* Call back from the various driver checksum routines with
 * the checksum values:
 * In the imap4 world we send this as a status command:
 * Note: For imap4 STATUS should be used to get the checksum.
 *       This will bypass this code altogether.
 *       From Imap2bis(sun) we do the imap2 checksum. */
void mm_checksum(MAILSTREAM *stream, unsigned short checksum,
		 long nbytes, char *driver)
{
  IMAPD_VARS *iv = (IMAPD_VARS *)stream->userdata;
  if (iv->sunversion_set) {
    switch (checksum_format) {
    default:
    case FORMATIMAP4STATUS:
      if (iv->imap4_sunversion) {
	/* Response to STATUS command/unsolicted to imap4 client
	 * Note: The checksum and number of bytes are combined into one
	 *       number under imap4. */
	if (!iv->imap4rev1)			/* use "-" stuff */
	  printf("* STATUS %s (CHECKSUM %05d%d UID-VALIDITY %ld UID-NEXT %ld)\015\012", 
		 stream->mailbox, checksum, nbytes, stream->uid_validity, 
		 stream->uid_last+1);
	else
	  printf("* STATUS %s (CHECKSUM %05d%d UIDVALIDITY %ld UIDNEXT %ld)\015\012", 
		 stream->mailbox, checksum, nbytes, stream->uid_validity, 
		 stream->uid_last+1);
	break;
      }
    case FORMAT2BIS:
      /* Response to CHECKSUM or imap2 sunversion client:  */
      printf("* CHECKSUM (\"%d\" \"%d\" \"%s\")\015\012", 
	     checksum, nbytes, driver);
      break;
    }
    fflush(stdout);
  }
}
/*
 * coerce an unsolicted status checksum response:
 * Here "CHECKSUM" was requested or we are just
 * sending a checksum to update the client.
 * This results in mm_checksum(..) being called. 
 * sendit is true if stream->send_checksum is True   
 * Driver request after fetch/store or we set it 
 * to True on the call */

void do_status_checksum(MAILSTREAM *stream,int format)
{
   IMAPD_VARS *iv = (IMAPD_VARS *)stream->userdata;
  /* 27-sep-96[*Bill++]: If stream->dead, then quit */
  if (stream->dead) return;
  /* Must be a sunversion client */
  /* If we get here, the stream is alive and has the updated
	sunversion_set. -clin */
  if (!iv->sunversion_set) return;

  /* See if the seen bit was set */
  if (stream->send_checksum) {  /* VF */
    checksum_format = format;
    if (stream->validchecksum) 
      stream->checksum_type = FASTCHECKSUM;
    else
      stream->checksum_type = OPENCHECKSUM;
    stream->checksum_type |= SERVERCALL;
    mail_checksum (stream); /* VF checksum on any write action */
    stream->checksum_type = stream->send_checksum = NIL;
  }
}
/*
 * get the three arugments and dispatch the driver
 * to validate the checksum:
 * Have: "checksum" "mbox_size" "driver_name" for imap2,
 *        ccccclll ...ll  in Imap4. 5 digits checksum and length */
#define CHECKSUMFIELD 0
#define MBOXSIZEFIELD 1
#define DRIVERNMFIELD 2
#define CSLEN 5
void valid_checksum(char *t, IMAPD_VARS *imapdvars,
		MAILSTREAM *stream)
{
  IMAPD_VARS *iv = (IMAPD_VARS *)stream->userdata;
  unsigned long checksum;
  unsigned long mbox_size;
  char *driver_name;
  char *next, *c;
  char *lasts;
  int which= 0;
  int error= NIL;
  int parsing= T;

  /* For imap4 we just send 10 digits as above */
  if (imapdvars->imap4_sunversion)
    next = t;
  else {
    if (*t++ != '\"') {		/* clear leading " */
      imapdvars->response = badatt;
      return;
    }
    next = strtok_r(t, "\"", &lasts);
  }
  while (parsing) {
    if (!next) {
      error = T;
      break;
    }
    switch (which) {
    case CHECKSUMFIELD:
      if (imapdvars->imap4_sunversion) {
	/* have "cccccbbbbbb..b"
	 * Get first five chars as checksum */
	char csbytes[CSLEN+1];
	char d;
	int i;
	for (i = 0; i < CSLEN; i++) {
	  d = *next++;
	  if (!isdigit(d)) {
	    error = T;
	    break;
	  } else
	    csbytes[i] = d;
	}
	if (error) break;
	csbytes[CSLEN] = NIL;
	checksum = strtol(csbytes, NIL, 10);
	if (isdigit(*next)) {
	  /* remaining chars a mbox_size */
	  mbox_size = strtol(next, &c, 10);
	  parsing = NIL;
	  driver_name = stream->dtb->name;
	  continue;
	} else {
	  error = T;
	  break;
	}
      } else {
	/* IMAP2bis truc */
	checksum = strtol(next, &c, 10);
	which = 1;
      }
      break;
    case MBOXSIZEFIELD:
      mbox_size = strtol(next, &c , 10);
      which = 2;
      break;
    case DRIVERNMFIELD:
      driver_name = next;
      error = parsing = NIL;
      continue;
    }
    if (error) break;
    ++c;			/* skip NULL */
    if (*c++ != ' ' && *c++ != '\"') { /* should be " \"" */
      error = T;
      break;
    }
    next = strtok_r(c, "\"", &lasts);
  } /* end parsing */
  if (error) {
    imapdvars->response = badatt;
  } else {
    if (stream->dtb->validchecksum(stream, 
				     checksum, 
				     mbox_size)) 
      printf("* VALIDCHECKSUM VALID\015\012");
    else
      printf("* VALIDCHECKSUM INVALID\015\012");
  }
  fflush(stdout);
}

/*
 * Return the status for the given mailbox */
void do_status_cmd(char *mailbox, char *t, char **lasts,
	IMAPD_VARS *imapdvars, MAILSTREAM *stream)
{
  IMAPD_VARS *iv = (IMAPD_VARS *)stream->userdata;
  long flags = 0; /* It's a good idea to initialize it. -clin */

				/* read flags */
  do {		/* parse each one; unknown generate warning */
    if (!strcmp (t,"MESSAGES")) flags |= SA_MESSAGES;
    else if (!strcmp (t,"RECENT")) flags |= SA_RECENT;
    else if (!strcmp (t,"UNSEEN")) flags |= SA_UNSEEN;
    else if (!strcmp (t,"UID-NEXT") ||
	     !strcmp (t,"UIDNEXT"))	/* revision 1 */
      flags |= SA_UIDNEXT;
    else if (!strcmp (t,"UID-VALIDITY") ||
	     !strcmp (t,"UIDVALIDITY"))	/* revision 1 */
      flags |= SA_UIDVALIDITY;
    else if (!strcmp (t, "CHECKSUM")) {
      flags |= SA_CHECKSUM;
      if (stream && (stream->stream_status & S_OPENED)) {
	if (stream->validchecksum)
	  stream->checksum_type = FASTCHECKSUM;
	else
	  stream->checksum_type = OPENCHECKSUM;
      }
    } else printf ("* NO Unknown status flag %s\015\012",t);
  } while (t = strtok_r (NIL," ", lasts));
  /* Force reparse of open mailbox if it has changed */
  if (imapdvars->state == OPEN) mail_ping (stream);
  if (!mail_status (stream, mailbox, flags)) {
    if (!imapdvars->lsterr) 
	 imapdvars->lsterr = cpystr("No such mailbox"); /* 26-dec-96 */
    	 imapdvars->response = lose;
  }
}

/* Count the line feeds in the data buffer that are NOT 
 * preceded by a CR:
 *  returns original data len + number of such lfs
 *  for which we MUST STUFF CR's */
int add_count_lfs(char *data,unsigned long len)
{ 
  register unsigned long i = len;
  register char *cp = data;
  while (len--) {
    switch (*cp) {
    case '\015':
      ++cp;				/* look for CRLF */
      if (len > 0 && *cp == '\012') 
	++cp;				/* skip the LF */
      break;
    case '\012':
      i += 1;
    default:
      ++cp;
      break;
    }
  }
  return i;
}
 
/* for tnf stuff */
void cmd_copy(char *cmd,char *buf)
{
  char c,*lcmd = cmd,*lb = buf;
  int i;

  for (i = 0,c = *lcmd++; i < PCMDLEN && c && c != '\n' && c != '\015';
       i++,c = *lcmd++)
    *lb++ = c;
  *lb = '\0';
  /* replace ' ' with '_' */
  lb = buf;
  while (c = *lb) {
    if (c == ' ') *lb = '_';
    *lb++;
  }
}

/*
 * open the /proc file and turn on microstate accounting.
 * call before using gethrvtime(3c)
 */
void
enable_msacct(void)
{
	char	procname[32];
	int	pfd, flags;

	(void) sprintf(procname, "/proc/%ld", getpid());
	pfd = open(procname, O_RDWR);
	flags = PR_MSACCT;
	(void) ioctl(pfd, PIOCSET, &flags);
	(void) close(pfd);
}

/*
  Input: imapd_vars data structure
  Return: an initialized imapd_vars.

*/  

void *imapd_var_init()
{
  IMAPD_VARS *imapdvars = (IMAPD_VARS *)fs_get(sizeof(IMAPD_VARS));
  memset ((void *)imapdvars,0,sizeof(IMAPD_VARS));

  imapdvars->curdriver = NIL;
  imapdvars->alerttime = 0;           
  imapdvars->state = LOGIN;              
  imapdvars->mackludge = 0;              
  imapdvars->trycreate = 0;              
  imapdvars->finding = NIL;              
  imapdvars->anonymous = 0;              
  imapdvars->logtry = LOGTRY;            
  imapdvars->kodcount = 0;              
  baiser = 0;                

  imapdvars->lire = 0;                  
  imapdvars->rocount = 0;               
  imapdvars->nmsgs = 0;
  imapdvars->recent = 0;
  imapdvars->host = NIL;

  imapdvars->user = NIL;
  imapdvars->pass = NIL;
  imapdvars->home = NIL;
  imapdvars->tag = NIL;
  imapdvars->cmdbuf = (char *)fs_get(TMPLEN);
  memset ((void *)imapdvars->cmdbuf,0,TMPLEN);

  imapdvars->cmd = NIL;
  imapdvars->arg = NIL;
  imapdvars->lsterr = NIL;
  imapdvars->response = NIL;
  ronlystr = NIL;

  imapdvars->litsp = 0;
  imapdvars->litstk[LITSTKLEN] = NIL;
  imapdvars->sunversion_set = NIL;
  imapdvars->imap4_sunversion = NIL;
  imapdvars->imap4rev1 = NIL;
  imapdvars->keep_mime = KEEPMIMEDEFAULT;

  imapdvars->append_buf = NIL;         
  imapdvars->append_fd  = 0;
  imapdvars->append_len = 0;

  return ( ( IMAPD_VARS *) imapdvars);
}

/*
  Input: imapdvars. 
  Goal:  flush imapdvars. 
*/

void imapd_var_free(IMAPD_VARS *imapdvars)
{

  if (imapdvars) {
     	if (imapdvars->lsterr)
     	fs_give ((void **) &imapdvars->lsterr );

     	if (imapdvars->host)
     	fs_give ((void **) &imapdvars->host );

     	if (imapdvars->user)
     	fs_give ((void **) &imapdvars->user );

     	if (imapdvars->pass)
     	fs_give ((void **) &imapdvars->pass );

     	if (imapdvars->append_buf)
     	fs_give ((void **) &imapdvars->append_buf);

     	fs_give ((void **) &imapdvars);
        }

}
