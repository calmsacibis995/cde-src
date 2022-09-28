/*
 * @(#)solaris.c	1.63 97/07/08
 *
 * Program:	Solaris Mail Routines
 *
 *              Based on the bezerk.c code by:
 *
 *              Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * The major changes for the solaris.c are 
 *     1. Using the Content-Length: header field to find the
 *        start of the next message.
 *     2. Hooks to code to do Sun-Attachments to MIME conversion
 *     3. Rewrites of several routines to accompany (2).
 *     4. Other changes to handle disconnected use, eg, the checksum
 *        consistency check.
 *     5. 1-apr-96: No memory caching required
 *                  write flags in place
 *                  use pmap for large buffers
 *     6. 22-jul-96: allow tool talk locking
 *     7. 22-jul-96: allow client to set group permissions
 *     8. 7-aout-96: Fix stream == NIL in the group access stuff.
 *     9. 13-aout-96: Use content length in parsing loop.
 *    10. 16-aout-96: Do not do body parse in short info fetch.
 *    11.  3-sept-96: Body adjustment bug for text MIME translations.
 *    12.  3-sept-96: OP_NOKODSIGNAL for pop callers. Immediate open fail.
 *    13. 12-sept-96: Fixed 43 byte error in message 0 (when pseudo header
 *                    is present and mailbox was "empty")
 *    14. 14-sept-96: Removed locking from inner loop of set/clear flag merde.
 *
 *    15. 16-sept-96: NULLs in nfs mounted files
 *    16. 17-sept-96: read/write index entries as a WHOLE.
 *    17. 20-sept-96: Rewrite of initial parse to use tmp files.
 *                    Min. 4/8K pmap pages.
 *    18. 25-sept-96: NULL thing ...
 *                    VERY OLD BUG -- Must increment seek base 
 *                    IF garbage chars are found between messages.
 *    19.  6-oct-96:  Fixed (3) bugs reported by Esther:
 *            bug in a) read_null_chars
 *                   b) Added to always read nulls on open_parse
 *            bug reported by John Cooper in fetchheader( ..lines..)
 *              if NOT keeping mime. Fixed.
 *            bug by Bill: NOT KEPT Mime translations now FILTER out
 *              Status merde from the header in fetchheader.
 *    20.  8-oct-96: Fixed solaris_rewrite_msgs(..) to update
 *           fc->real_mem_len; (was 43 too small : status merde)
 *           This left 43 NULLs at the end of an expunged/copied msg.
 *    21. 10-oct-96: fixed VALID to deal with rfc822 From lines.
 *    22. 11-oct-96: replace NULLs with '\n' during open
 *    23. 14-oct-96: Fix to fetch section 0 of a MESSAGE/rfc822
 *
 *++++++++++++++++++ MT-Safe FIXES ++++++++++++++++++++++++++++++++++++++++
 *    24. [10-04-96]: ctime_r implemented for MT-safe.
 *    25. [10-21-96]: Passing stream to the following routines:
 *			solaris_parameters.
 *			solaris_valid,
 *			solaris_isvalid,
 *			dummy_file calls,
 *			solaris_open
 *			create_null_msg
 *
 *			Added a new routine: solaris_global_init.
 *			stream->stream_status bit test in:
 *			solaris_open 
 *			
 *			Passing stream with stream_status = NIL to:
 *			mail_parameters
 *			
 *			Globals put onto the stream:
 *			solaris_fromwidget
 *    26. [10-25-96]: Call memset in solaris_global_init. 
 *    27. [11-01-96]: Pass a NIL stream in all mm_diskerror calls.  
 *    28. [11-08-96]: Pass a stream in all mm_log calls.  
 *    29. [11-18-96]: Removed the NIL stream in mm_diskerror.  
 *    30. [11-19-96]: Pass a NIL stream in fatal - it's OK to pass
 *			a NIL here since the calling routines don't contain
 *			a stream. Also if it gets in fatal, nothing matters.
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *    31. 25-oct-96: pmap get failed[Out of Virtual Memory]
 *    32. 26-oct-96: Keep flags on copy
 *                   stream->nousersig prevents usr1,usr2 on open
 *    33. 17-oct-96: solaris_short_structure - Fixed m->mime_tr BUG.
 *    34. 14-nov-96  line 664 i == 0 ==> i = 0
 *    35. 14-nov-96: links and relative paths in the linkee.
 *    36. 14-nov-96: Gary Gere's mmap enhancement's for index file.
 *    37. 21-nov-96: Do time checks % 100 in open parse.
 *    38. 27-nov-96: Gary Gere mmap for open read. client API only.
 *    39.  6-dec-96: Bug in unmapping when stream->local = NIL.
 *    40  16-dec-96: call to solaris_checksum_msg resized PARSE_BUF
 *                   during parse. This caused seg fault on a message
 *                   whose header was larger than 64k. Fixed.
 *    41. 18-dec-96: Fixed mailbox.lock code in solaris.lock
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *    42. [12-31-96 ] Define solaris_globals in solaris.h.
 *    43. [01-01-97 ] Add a new routine solaris_global_free.
 *    44. [01-02-97 ] Use mail_stream_setNIL for passing a NIL stream in
 *		      mail_parameters.
 *    45. [01-13-97 ] Merge Bill's solaris_open code: attach solarisproto
 *		      to stream->dtb. 
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *    46.  2-jan-97: if access returns err == EROFS, then make RO.
 *    47.  2-jan-97: Check st_nlinks == 0. IF so. ABORT in
 *                   solaris_parse. MailTool can rename mailbox.
 *    48.  3-jan-97  close fd in solaris append.
 *    49.  8-jan-97  if ((i % 17) == 0) in building tmp file
 *    50.  9-jan-97  effectively made 38 (i % 1), ie, always check
 *                   added count down in update_index.
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *    51. [01-16-97 ] Free LOCAL->iname in solaris_abort.
 *    52. [01-22-97 ] Test (stream_status & S_OPENED) instead of stream. 
 *    53. [01-28-97 ] Cooper's find: (access(path,permissions) == 0) in
 *		      solaris_path_access. 
 *
 * Still, this is done in the original spirit engendered by Mark, a long
 * time friend and collegue.
 *
 * C'est bien vrai que moi je prefere La France aux Etats Unis de temps en
 * temps. Bah ouai, c'est le vin, le repas, est surtout mes amis francais
 * qui me rendent affole de ce pays...
 *
 * 
 * Date:	20 December 1989
 * Last Edited: 1997	
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

/*
 * Error messages(not a complete list encore!)
 * 105 Can't create SESSION lock, access is readonly
 * 106 Mailbox %s is locked, will override in %d seconds...
 * 107 Trying to get SESSION lock from process %ld
 * 108 Mailbox access is readonly
 * 109 Mailbox is empty
 * 115 Broken hidden \"From\" line msg#%d: Aborting(PARSE)
 * 116 Illegal Mailbox format detected(Try restarting)
 * 117 Mailbox shrank from %d to %d bytes, aborted
 * 131 Server busy(STORE)
 * 154 Cannot acquire imap session lock: select failed!
 * 159 New Mail: NULLS detected in %smailbox
 * 160 NULLS removed from %smailbox(PARSING)
 * 161 Mailbox in use by another client
 * 162 Can't set real gid:  %s
 * 163 Unparsable date
 * 164 %d UIDS changed during parse
 * 165 Error creating %s: %s
 * 166 Read error(VM problem?): %s
 * 167 Trying to get destination mailbox lock
 * 168 Mailbox.lock cannot be removed
 * 169 Mailbox removed by another process. Cannot continue.
 */

#include "UnixDefs.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <limits.h>
#include <utime.h>
#include <sys/file.h>
#include "mail.h"
#include "os_sv5.h"
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/statvfs.h>
#include "solaris.h"
#include "rfc822.h"
#include "misc.h"
#include "sun_mime.h"
#include "dummy.h"
#if (_POSIX_C_SOURCE -0 >= 199506L)
#include <pthread.h>
#endif

extern int madvise(caddr_t addr, size_t len, int advice);

static const int	MININDEXREQUIREDSIZE	= 131072;/* 2^17 bytes */
static const int	PSEUDOLEN		= 317;

static const char	*	PSEUDOEND = "--END+PSEUDO--\n";
                                           /*123456789012345*/
static int		PSEUDOENDLEN		= 15;

/* keep this even */

static const char	*	PSEUDOFROMLINE = "From <IMAP4.psuedo.sims>";
static const char 	*	INDEXFILEPREFIX = "_dt_index";
static const char 	*	OLDINDEXFILEPREFIX = "_SIMS_index";
static const char	*	INDEXVERSION = "1005";
static const char	*	STATUS =
  "Status: RO\nX-Status: DFAT\nX-UID: 0000000000\n\n\n";
/* 1234567890  12345678901234  12345678901234567 */
static const int	STATUSLEN		= 10;
static const int	XSTATUSLEN		= 14;
static const int	XUIDLEN			= 17;
static const int	STATUSTOTAL = 10 + 14 + 17 + 4;

/* This holds our architecture string sysinfo(SI_ARCHITECTURE,...) */
static char		solarisArch[10+1];

/* This one from sysinfo(SI_HW_SERIAL) */
static const char *	hw_id;

/* These exist so that if mmap() fails, we do not need swap/malloc to print */
static const char *	eacces = "EACCES - Permission denied";
static const char *	eagain = "EAGAIN - No more processes, LWPs,"
				 "swap, or memory";
static const char *	ebadf = "EBADF - File descriptor in bad state";
static const char *	einval = "EINVAL - Invalid argument";
static const char *	enodev = "ENODEV - No such device";
static const char *	enomem = "ENOMEM - Not enough space";
static const char *	enxio = "ENXIO -  No such device or address";
static const char *	msg167 = "167 Trying to get destination "
				 "mailbox lock. ";
static const char *	msg169 = "Mailbox removed by another process."
				 " Cannot continue. ";

/* If we are using PTHREADS, then this is the once lock for solaris_once() */
#if (_POSIX_C_SOURCE >= 199506L)
static pthread_once_t	solarisOnceVar = PTHREAD_ONCE_INIT;
#endif

/* Do not return these headers from solaris_fetchheader() */
static char * filter_out[] = {
  "Status",
  "X-Status",
  "X-UID",
  "X-IMAP",
  NULL
};

typedef enum {
  EOM1 = 1,
  EOM2,
  EOM3
} EOM_t;

typedef enum {
  UNLINKSUCCEEDED = 1,
  UNLINKFAILED,
  UNLINKNOTDONE
} UnLink_t;


static char * solaris_eom (char *som,
			   char *sod,
			   long i,
			   long *body_size,
			   int *n_crs,
			   int *encore,
			   int *skipping,
			   unsigned long *body_pos,
			   EOM_t *eomType,
			   int *ok_content_len,
			   int *try_content_len,
			   int *un_de_plus,
			   long buf_len,
			   char **clen_eom,
			   long *from_offset,
			   int *from_renter,
			   unsigned long  *msg_size,
			   int *last_msg);

/* Driver dispatch used by MAIL */

DRIVER solarisdriver = {
  "solaris",			/* driver name */
  DR_LOCAL|DR_MAIL,		/* driver flags */
  (DRIVER *) NULL,		/* next driver */
  solaris_valid,		/* mailbox is valid for us */
  solaris_parameters,		/* manipulate parameters */
  solaris_scan,			/* scan mailboxes */
  solaris_list,			/* list mailboxes !NEW! */
  solaris_lsub,			/* list subscribed mailboxes !NEW! */
  solaris_subscribe,		/* subscribe to mailbox */
  solaris_unsubscribe,		/* unsubscribe from mailbox */
  solaris_create,		/* create mailbox */
  solaris_delete,		/* delete mailbox */
  solaris_rename,		/* rename mailbox */
  NULL,				/* status */
  solaris_open,			/* open mailbox */
  solaris_close,		/* close mailbox */
  solaris_fetchfast,		/* fetch message "fast" attributes */
  solaris_fetchflags,		/* fetch message flags */
  solaris_fetchstructure,	/* fetch message envelopes */
  solaris_fetchheader,		/* fetch message header only */
  solaris_fetchtext,		/* fetch message body only */
  solaris_fetchbody,		/* fetch message body section */
  NULL,				/* unique identifier */
  solaris_setflag,		/* set message flag */
  solaris_clearflag,		/* clear message flag */
  NULL,				/* search for message based on criteria */
  solaris_ping,			/* ping mailbox to see if still alive */
  solaris_check,		/* check for new messages */
  solaris_expunge,		/* expunge deleted messages */
  solaris_copy,			/* copy messages to another mailbox */
  solaris_append,		/* append string message to mailbox */
  solaris_gc,			/* garbage collect stream */
  /* SUN Imap4 here */
  solaris_fab_from,		/* the hidden from */
  solaris_set1flag,		/* optimization for set/clear */
  solaris_clear1flag,           /* clear one flag */
  /* dummies - ne rien a faire */
  dummy_sunversion,
  NULL,
  dummy_echo,
  dummy_clear_iocount,
  dummy_urgent,
  solaris_fetchshort,		/* fetch short attributes */
  solaris_checksum,		/* checksum mailbox */
  solaris_validchecksum, 	/* validate checksum */
  dummy_sync_msgno,		/* no sync on local mboxes */
  dummy_imap4,			/* always what the "con" dummy says */
  solaris_free_body,            /* free the body */
  solaris_fetchenvelope,	/* envelope only. No body parse. */
  solaris_global_init,          /* init solaris globals */
  solaris_global_free           /* free solaris globals */
};

/* For small internal buffers */
#define	smallBUFLEN		256
const int NULLS_IN_NEWMAIL =	-2;

/* #define CHECKSUM_DEBUG */
#ifdef CHECKSUM_DEBUG
void solaris_print_chksum_summary(MAILSTREAM *stream, char *name);
#endif

/* prototype stream */
MAILSTREAM solarisproto = {&solarisdriver};

MAILSTREAM *mailstd_proto = &solarisproto;

/* forward declarations */
static void solaris_update_status(char *status,MESSAGECACHE *elt, boolean_t doUid);
static solaris_readmsg(MAILSTREAM *stream,int fd,FILECACHE *m,char *lock,
		       int during_parse);
static boolean_t write_the_status(MAILSTREAM	* stream,
				  unsigned long   msgno,
				  int 		  fd,
				  MESSAGECACHE  * elt,
				  FILECACHE 	* fcache,
				  boolean_t	  update);
static boolean_t solaris_write_status_flags(MAILSTREAM   * stream,
					    unsigned long  msgno,
					    MESSAGECACHE * elt,
					    int 	   fd);
static boolean_t solaris_write_status(MAILSTREAM	* stream,
				      unsigned long 	  msgno,
				      MESSAGECACHE 	* elt);
static void report_and_avorter(MAILSTREAM *stream,int fd,char *lock);

static int	anon_fd = -1;
static int	pagesize;
static void	chunk_init(void);
static int solaris_create_tmp(MAILSTREAM *stream,int fd,long index0,
			      char *tmpname,char *rootname,int *doux);
static solaris_exp_save_msgs (MAILSTREAM *stream,int fd,int tmp_fd,long index0,
			      struct stat *sbuf,char *lock);
static solaris_rewrite_msgs (MAILSTREAM *stream,int fd,int tmp_fd,long index0,
			     long nmsgs,struct stat *sbuf,char *lock);
static void solaris_save_msgs(MAILSTREAM *stream,int fd,long index0, long nmsgs, 
			      struct stat *sbuf);
static create_index(MAILSTREAM *stream,struct stat *sb);
static void remove_old_index_files(char *mailbox, boolean_t newNameIsInvalid);
static boolean_t write_index_entry(MAILSTREAM *   stream,
				   FILECACHE  *   fcache,
				   MESSAGECACHE * elt,
				   boolean_t	  serial);
static update_index(MAILSTREAM *stream);
static boolean_t read_index_entry(MAILSTREAM *stream,int id,INDEXENTRY *ie,
				  unsigned long msgno,
				  int serial);
static boolean_t read_index_hdr(MAILSTREAM *stream,INDEXHDR *ihdr);
static char *create_index_name(MAILSTREAM *stream,char *file);
static char *create_index_name_name(char *base,char *file);
static boolean_t valid_index_hdr(MAILSTREAM 	* stream,
				 INDEXHDR 	* ihdr,
				 struct stat 	* sb);
static boolean_t lock_index(MAILSTREAM * stream, int id,int op);
static void unlock_index(MAILSTREAM * stream, int id);
static void recreate_index_file(MAILSTREAM *stream);
static filetimes_hdr(MAILSTREAM *stream);
static boolean_t update_index_hdr(MAILSTREAM * stream);
static boolean_t update_index_entry(MAILSTREAM	 * stream,
				    FILECACHE 	 * fcache,
				    MESSAGECACHE * elt);
static void mmap_index_file(MAILSTREAM *stream, int readOnly);
static void unmap_index_file(MAILSTREAM *stream);
static void mmap_mbox(MAILSTREAM *stream);
static void unmap_mbox(MAILSTREAM *stream);
static char *create_null_msg(FILECACHE *m, MAILSTREAM *stream);
static boolean_t solaris_setftimes(int fd,MAILSTREAM *stream);
static int solaris_mbox_access(MAILSTREAM *stream, char *mbox, int permissions,
			       int *err);
static int solaris_path_access(MAILSTREAM *stream, char *path, int permissions,
			       int *err);
static int solaris_clear_group_permissions(MAILSTREAM *stream);
static int solaris_lockf(MAILSTREAM *stream,char *mbox);
static boolean_t solaris_nfs_mounted(int fd);
static int read_null_chars(char *s,long i,char **nullptr);
static void replace_nulls(char *nullptr,unsigned long n);
int other_is_valid(char *s, int *zn, int *ti);
int rfc822_valid(char *s, char *x, int *ti, int *zn, int *rv);

static update_filetimes(MAILSTREAM *stream);
static boolean_t write_index_hdr(MAILSTREAM *stream,int fd,unsigned long new);

int safe_writev(int fd, const struct iovec *, int len);

int solaris_quick_checksum(MAILSTREAM *stream, long msgno);
int ndigits(unsigned long v);

static const char	*	xUnixFromHeader = "X-Unix-From";

/*
 * Initalize any solaris global variables - once.
 * Callable from pthread_once().
 */
void
solaris_once()
{
  struct utsname	uts;

  const char	*	sunos = "SunOS";
  const char	*	only = 
    "Sorry, this version only runs on SunOS (Solaris)";

#if (SUNOS < 56)
  const char	*	release = "5.4";
  const char	*	msg =
    "Sorry, this version only runs on SunOS 5.4 and above.";

#else

  const char	*	release = "5.6";
  const char	*	msg =
    "Sorry, this version only runs on SunOS 5.6 and above.";

#endif

  /* Check that the OS is Solaris and is a supported version */
  if (uname(&uts) > -1) {	/* get the OS description */

    if (strcmp(uts.sysname, "SunOS") != 0) {
      fatal((char*)only, NULL);
    } else {

      if (strcmp(uts.release, release) < 0) {
	fatal((char*)msg, NULL);
      } else {

	sysinfo(SI_ARCHITECTURE, solarisArch, sizeof(solarisArch)-1);
	PSEUDOENDLEN = strlen(PSEUDOEND);
	pagesize = sysconf(_SC_PAGESIZE);

	if ((anon_fd = open("/dev/zero", O_RDWR)) == -1) {
	  perror("open()");
	  fatal("PMAP: Cannot open /dev/zero", NULL);
	}
	hw_id = unique_hwid();
      }
    }
  }
  return;
}

static boolean_t
solaris_flock(MAILSTREAM * stream, int fd, int operation)
{
  boolean_t	gotLock = _B_FALSE;

  if (solaris_nfs_mounted(fd)) {
    /*
     * We have to fake this. The code really needs not to
     * try non-local folder fcntl().
     */
    gotLock = _B_TRUE;

  } else {

    flock_t	fcntlData;
    boolean_t	validOp = _B_FALSE;
    boolean_t	eintrRetry = _B_FALSE;
    int		intResults = 0;
    int		cmd;

    fcntlData.l_whence = 0;       /* Lock the entire file */
    fcntlData.l_start = (off_t)0;
    fcntlData.l_len = (off_t)0;
 
    if ((operation & LOCK_NB) == LOCK_NB) {
      cmd = F_SETLK;
    } else {
      cmd = F_SETLKW;     /* LOCK - WAIT for other lock to go away */
    }
 
    switch (operation & ~LOCK_NB) {       /* translate to lockf() operation */
    case LOCK_EX:                 /* exclusive */
    case LOCK_SH:                 /* shared */
      fcntlData.l_type = F_RDLCK | F_WRLCK;
      validOp = _B_TRUE;
      break;
 
    case LOCK_UN:                 /* unlock */
      /*fcntlData.l_type = F_ULOCK | F_UNLKSYS;*/
      fcntlData.l_type = F_UNLCK;
      validOp = _B_TRUE;
      break;
 
    default:
      errno = EINVAL;
    }

    do {
      if (validOp) {
	intResults = fcntl(fd, cmd, &fcntlData);       /* do the lockf() */
	if (intResults != -1) {
	  gotLock = _B_TRUE;/* Man page says fcntl() will != -1 on success */
	}
      }

      if (!gotLock) {
	char	errMsg[1024];

	switch(errno) {

	case '0':
	  break;

	case EINTR:
	  eintrRetry = _B_TRUE;
	  break;

	default:
	  if (stream != NULL) {
	    strcpy(errMsg, msg167);
	    strcat(errMsg, strerror(errno));
	    mm_log(errMsg, WARN, stream);
	  }
	  break;
	}
      }
    } while (eintrRetry);
  }
  return(gotLock);
}

/*
 * Solaris mail validate mailbox
 * Accepts: mailbox name
 * Returns: our driver if name is valid, NIL otherwise
 */
DRIVER *
solaris_valid(char * name, MAILSTREAM * stream)
{
  char tmp[MAILTMPLEN];
  return(solaris_isvalid(name ,tmp, stream, _B_FALSE) ? &solarisdriver : NULL);
}

static int
getUnixFrom(FILECACHE * m, char * buf)
{
  char		*	endOfUnixFrom = NULL;
  int			len = 0;

  endOfUnixFrom = strchr(m->full_header, '\n');

  if (endOfUnixFrom != NULL) {
    len = endOfUnixFrom - (m->full_header + 5);	/* 5 == strlen("From "); */
    sprintf(buf, "%s: ", xUnixFromHeader);
    strncat(buf, m->full_header + 5, len);
    strcat(buf, "\r\n");
  }
  return(strlen(buf));
}


/*
 * Solaris mail test for valid mailbox
 * Accepts: mailbox name
 * If keepFd is true, it returns the FD.
 *
 * Returns (if !keepFd): _B_TRUE if valid, _B_FALSE otherwise.
 *	   (if  keepFd) : fd of the folder if valid, -1 otherwise.
 *
 * Side effect: errno = 0 if existent and EMPTY.
 */
int
solaris_isvalid(char * name, char * tmp, MAILSTREAM * stream, boolean_t keepFd)
{
  int			fd;
  int			ret = _B_FALSE;
  char	*		t;
  char			file[MAILTMPLEN];
  struct stat		sbuf;
  struct utimbuf	tp;

  errno = EINVAL;		/* assume invalid argument */

  /* Make sure it is not a remote file name */
  if ((*name != '{') && !((*name == '*') && (name[1] == '{'))) {
    t = dummy_file(file, name, stream);
    if (t != NULL) {
      if (stat(t, &sbuf) == 0) {

	/* If it ends with a '/' it can break strcat(...) later */
	if (name[strlen(name)-1] == '/') {
	  name[strlen(name)-1] = '\0';
	}

	/* If the file exists, Try to open it */
	if ((fd = open(file, O_RDONLY, 0)) >= 0) {
	  if (!sbuf.st_size) {
	    errno = 0;		/* empty file - accept it... */
	    ret =_B_TRUE;
	  
	  } else {		/* error -1 for invalid format */
	    if (!(ret = solaris_isvalid_fd(fd, tmp))) {
	      errno = -1;
	    }
	  }

	  /* Return the FD if 'keepFd' */
	  if (!keepFd) {
	    close (fd);			/* close the file */
	  }

	  if (keepFd && ret) {
	    ret = fd;
	  }

	  tp.actime = sbuf.st_atime;	/* preserve atime and mtime */
	  tp.modtime = sbuf.st_mtime;
	  utime(file, &tp);			/* set the times */
	}
      }
    }
  }
  return(ret);				/* return what we should */
}

/*
 * Solaris mail test for valid mailbox
 * Accepts: file descriptor
 *	    scratch buffer
 * Returns: _B_TRUE if valid, _B_FALSE otherwise
 */
long
solaris_isvalid_fd(int fd, char * tmp)
{
  int 			zn;
  int			rv;
  int		       	ret = 0;	/* Default to failure */

  char		*	s;
  char		*	t;
  char			c = '\n';

  memset (tmp,'\0',MAILTMPLEN);
  if (read(fd, tmp, MAILTMPLEN-1) >= 0) {
    for (s = tmp; (*s == '\n') || (*s == ' ') || (*s == '\t');) {
      c = *s++;
    }
    if (c == '\n') {
      VALID(s, &t, &ret, &zn, &rv);
    }
  }
  if (ret != 0) {
    return(_B_TRUE);
  }
  return(_B_FALSE);
}

/*
 * Solaris manipulate driver parameters
 * Accepts: function code
 *	    function-dependent value
 * Returns: function-dependent return value
 */
void	*
solaris_parameters(MAILSTREAM * stream, long function, void * value)
{
  SOLARIS_GLOBALS *	sg = (SOLARIS_GLOBALS *)stream->solaris_globals;

  switch ((int) function) {

  case SET_FROMWIDGET:
    sg->solaris_fromwidget = (long) value;
    break;

  case GET_FROMWIDGET:
    value = (void *)sg->solaris_fromwidget;
    break;

  case SET_LOCKTIMEOUT:
    sg->lock_timeout
      = (long)value < DEFAULTLOCKTIMEOUT ? DEFAULTLOCKTIMEOUT : (long)value;
    break;

  default:
    value = NULL;		/* error case */
    break;
  }
  return(value);
}

/*
 * Solaris mail scan mailboxes
 * Accepts: mail stream
 *	    reference
 *	    pattern to search
 *	    string to scan
 */
void
solaris_scan(MAILSTREAM *stream,char *ref,char *pat,char *contents)
{
  dummy_scan(stream, ref, pat, contents);
}

/*
 * Solaris mail list mailboxes
 * Accepts: mail stream
 *	    reference
 *	    pattern to search
 */
void
solaris_list(MAILSTREAM *stream,char *ref,char *pat)
{
  /*
   * We pass the stream because it has cached object
   * information required by C++
   */
  dummy_list(stream, ref, pat);
}

/* 
 * Solaris mail list subscribed mailboxes
 * Accepts: mail stream
 *	    reference
 *	    pattern to search
 */
void
solaris_lsub (MAILSTREAM *stream,char *ref,char *pat)
{
  dummy_lsub(stream, ref, pat);
}

/*
 * Solaris mail subscribe to mailbox
 * Accepts: mail stream
 *	    mailbox to add to subscription list
 * Returns: T on success, NIL on failure
 */
long
solaris_subscribe (MAILSTREAM * stream, char * mailbox)
{
  char		tmp[PATH_MAX+1];
  return sm_subscribe(dummy_file(tmp, mailbox, stream), stream);
}


/*
 * Solaris mail unsubscribe to mailbox
 * Accepts: mail stream
 *	    mailbox to delete from subscription list
 * Returns: T on success, NIL on failure
 */
long
solaris_unsubscribe (MAILSTREAM * stream, char * mailbox)
{
  char		tmp[PATH_MAX+1];
  return sm_unsubscribe(dummy_file(tmp, mailbox, stream), stream);
}

/*
 * Solaris mail create mailbox
 * Accepts: MAIL stream
 *	    mailbox name to create
 * Returns: _B_TRUE on success, _B_FALSE on failure
 */
long
solaris_create(MAILSTREAM * stream, char * mailbox)
{
  char		tmp[PATH_MAX+1];
  char *	slash;
  int 		err;

  dummy_file(tmp, mailbox, stream);

  /*
   * back track until we find a directory we can access
   * write (may not exist)
   */
  slash = strrchr(tmp,'/');
  /* existence check */
  while (slash  && slash != tmp) {
    *slash = '\0';
    if (solaris_path_access(stream, tmp, F_OK, &err)) {
      /* back track until exists */
      slash = strrchr(tmp,'/');
      if (slash == tmp) {
	tmp[1] = '\0';
	break;	/* Yikes. The root! */
      }
    } else break;
  }
  solaris_clear_group_permissions(stream); 
  if (solaris_path_access(stream, tmp, W_OK, &err)) {
    sprintf(tmp,"158 Access denied for %s, err= %s",  mailbox, strerror(err));
    mm_log(tmp, WARN, stream);
    mm_log("Access denied", ERROR, stream);
    return _B_FALSE;
  } else {
    int ret =  dummy_create(stream, mailbox);
    solaris_clear_group_permissions(stream);
    return ret;
  }
}


/* 
 * Solaris mail delete mailbox
 * Accepts: MAIL stream
 *	    mailbox name to delete
 * Returns: T on success, NIL on failure
 */
long
solaris_delete(MAILSTREAM * stream, char * mailbox)
{
  return solaris_rename(stream, mailbox, NIL);
}

/*
 * Solaris mail rename mailbox
 * Accepts: MAIL stream
 *	    old mailbox name
 *	    new mailbox name (or NIL for delete)
 * Returns: T on success, NIL on failure
 */
long
solaris_rename(MAILSTREAM * stream, char * old, char * new)
{
  long		ret = _B_TRUE;
  char 		tmp[PATH_MAX+1];
  char		file[PATH_MAX+1];
  char		lock[PATH_MAX+1];
  char		lockx[PATH_MAX+1];
  char		src_index[PATH_MAX+1];

  int		fd;
  int		ld = -1;
  int		err;
  struct stat	tstat;
  int		protection;
 
  mail_stream_setNIL(stream);
  protection = (int) mail_parameters(stream ,GET_LOCKPROTECTION, NIL);
  mail_stream_unsetNIL(stream);

  if (solaris_path_access(stream,dummy_file(file, old, stream), W_OK, &err)) {
    mm_log("158 Access denied!", WARN, stream);
    mm_log("Access denied!", ERROR, stream);
    return(_B_FALSE);
  } else if (new && 
	     solaris_path_access(stream,
				 dummy_file(file, new, stream),
				 F_OK,
				 &err) == 0) {
    /* File exists. Rename denied. */
    mm_log("158 Access denied[File exists]!", WARN, stream);
    mm_log("Access denied[File exists]!", ERROR, stream);
    return(_B_FALSE);
  }

  dummy_file(file, old, stream);
  ld = solaris_isvalid(file, tmp, stream, _B_TRUE);
  
  /* lock out other c-clients */
  if (ld > -1 && !solaris_flock(stream, ld, LOCK_EX|LOCK_NB)) {
    close (ld);			/* couldn't lock, give up on it then */
    sprintf(tmp,"Mailbox is in use by another process.");
    mm_log(tmp,ERROR,stream);
    solaris_clear_group_permissions(stream);
    return _B_FALSE;
  }
  solaris_clear_group_permissions(stream);

  /* lock out non c-client applications */
  if ((fd = solaris_nowait_lock(stream, file, O_WRONLY,S_IREAD|S_IWRITE,
				lockx, LOCK_EX)) < 0) {
    sprintf(tmp,"Cannot lock destination mailbox, %s", strerror(errno));
    mm_log(tmp,ERROR,stream);
    close(ld);
    return(_B_FALSE);
  }

  /* Now get the index file */
  create_index_name_name(file, src_index);

  if (file[0] == '\0') {
    sprintf(tmp, "Cannot rename mailbox, %s", strerror (errno));
    mm_log(tmp,ERROR,stream);
    ret = _B_FALSE;
  } else if (new) {

    /* do the rename or delete operation */
    if (rename(file, dummy_file(tmp, new, stream)) < 0) {
      sprintf(tmp,"Cannot rename mailbox, %s", strerror(errno));
      mm_log(tmp, ERROR, stream);
      ret = NIL;
    }
  } else if(unlink(file) < 0) {/* DELETE command */
    sprintf(tmp, "Cannot delete mailbox, %s", strerror(errno));
    mm_log(tmp, ERROR, stream);
    ret = NIL;			/* set failure */
  } 
  
  /* drop the index file if it exists */
  if (ret && stat(src_index,&tstat) == 0) {
    unlink(src_index);
  }
  solaris_unlock(fd, NIL, lockx);/* unlock  mailbox */
  close(fd);
  solaris_flock(stream, ld, LOCK_UN);	/* release c-client lock lock */
  close(ld);			/* close c-client lock */
  unlink(lock);			/* and delete it */
  return (ret);			/* return success */
}

/*
 * Solaris mail open
 * Accepts: Stream to open
 * Returns: Stream on success, NIL on failure
 */
MAILSTREAM	*
solaris_open(MAILSTREAM * stream)
{
  long 			i;
  int			fd;
  char 			small[smallBUFLEN];
  char 			tmp[MAXPATHLEN];
  char			tfile[MAXPATHLEN];
  char			locktmp[MAXPATHLEN];
  char			lock[MAXPATHLEN];

  mailcache_t 	    mc = (mailcache_t)mail_parameters (stream,GET_CACHE,NIL);
  struct stat 		sbuf;
  long 			retry;			/* For unlocking */
  int 			protection;
  boolean_t		bises = _B_FALSE;
  boolean_t		isHome;
  char 		*	mhome;
  char 		*	perr;
  int 			err;
  SOLARIS_GLOBALS *	sg = (SOLARIS_GLOBALS *)stream->solaris_globals;

  mail_stream_setNIL(stream);
  protection = (int) mail_parameters(stream, GET_LOCKPROTECTION, NIL);
  mail_stream_unsetNIL(stream);

  /*
   * return prototype as dtb for OP_PROTOTYPE call:
   *   We are called by mail_open. If the user has asked for
   *   a prototype stream, then the stream has NOT been initialized,
   *   stream->stream_status NOT yet set.
   */
  if (stream->stream_status & S_PROTOTYPE) {
    stream->dtb = (DRIVER *)&solarisproto;
    return(NULL);
  }
				/* canonicalize the stream mailbox name */
  retry = stream->silent ? 1 : KODRETRY; /* Unlocking */
  if (((SOLARISLOCAL*)stream->local)) {			/* close old file if stream being recycled */
    solaris_close(stream, NIL);/* dump and save the changes */
    stream->dtb =&solarisdriver;/* reattach this driver */
    mail_free_cache(stream);	/* clean up cache */
  }

  /* get canonicalized mailbox name */
  dummy_file(tfile, stream->mailbox, stream);
  fs_give ((void **) &stream->mailbox);
  stream->mailbox = cpystr(tfile);

  /* now get the canonical file name for "INBOX" */
  dummy_file(tmp, "INBOX", stream);

  /* Allocate driver local data space */
  stream->local = fs_get(sizeof(SOLARISLOCAL));

  /*
   * (2) If lockf is a session lock and that cannot be done, punt
   * The call will set ((SOLARISLOCAL*)stream->local)->fd and leave it open, or set
   * ((SOLARISLOCAL*)stream->local)->fd = -1 if it fails. 
   */
  if (stream->session_lockf && !solaris_lockf(stream, tfile)) {
    fs_give((void **)&stream->local);
    mm_log ("161 Mailbox in use by another client",WARN,NIL);
    mm_log_stream(stream,"161 Mailbox in use by another client");/*ERROR*/
    return NULL;
  }

  /*
   * See if inbox. We send a kiss of death to other processes
   * that have the inbox open
   */
  if (strcasecmp(tmp, tfile) == 0) {
    ((SOLARISLOCAL*)stream->local)->inbox = _B_TRUE;
  } else {
    gid_t r_gid = getgid();
    gid_t e_gid;

    /* We only use the effective gid for INBOX */
    if (setgid(r_gid) < 0) {
      sprintf (small, "162 Can't set real gid:  %s", strerror (errno));
      mm_log(small, WARN,NIL);
      mm_log_stream(stream, small);		/* ERROR */
    }	
    ((SOLARISLOCAL*)stream->local)->inbox = NULL;
  }

  /* Is this on the home dir */
  mhome = myhomedir(stream);
  if (strncmp(mhome, tfile, strlen(mhome)) == 0) {
    isHome = _B_TRUE;
  } else {
    isHome = _B_FALSE;
  }

  /*
   * connect to home directory. Security reason. Any core will
   * be left on the users home dir (only for the server)
   */
  if (stream->server_open) {
    chdir(mhome);
  }

  /*
   * You may wonder why ((SOLARISLOCAL*)stream->local)->name is needed.  It isn't at all obvious from
   * the code.  The problem is that when a stream is recycled with another
   * mailbox of the same type, the driver's close method isn't called because
   * it could be IMAP and closing then would defeat the entire point of
   * recycling.  Hence there is code in the file drivers to call the close
   * method such as what appears above.  The problem is, by this point,
   * mail_open() has already changed the stream->mailbox name to point to the
   * new name, and solaris_close() needs the old name.
   */
  ((SOLARISLOCAL*)stream->local)->name = strdup(tfile);	/* local copy for recycle case */
				/* build name of our lock file */
  /*
   * We cannot get the lockname if the mailbox.lock is set.
   * This is the beginning of a bug fix. Check if mailbox.lock is
   * set, and if so, do what solaris_lock does with it ..
   */
  strcat(dummy_file(lock, tfile, stream),".lock");

  ((SOLARISLOCAL*)stream->local)->filesize = 0;/* initialize file information */
  ((SOLARISLOCAL*)stream->local)->ino = 0;
  ((SOLARISLOCAL*)stream->local)->uid = -1;
  ((SOLARISLOCAL*)stream->local)->gid = -1;
  
  
  ((SOLARISLOCAL*)stream->local)->filetime = 0;          /* last written */
  ((SOLARISLOCAL*)stream->local)->accesstime =  0;       /* last read */

  /* Initialize our cache for the primary parse */
  ((SOLARISLOCAL*)stream->local)->cachesize = SOLARISCACHEINIT + CACHEINCREMENT;
  (*mc) (stream,SOLARISCACHEINIT,CH_SIZE); /* create c-client internal elements */

  ((SOLARISLOCAL*)stream->local)->msgs =         	/* create new solaris message cache */
    (FILECACHE **) fs_get (((SOLARISLOCAL*)stream->local)->cachesize * sizeof (FILECACHE *));

  for (i = 0; i < ((SOLARISLOCAL*)stream->local)->cachesize; ++i) {
    ((SOLARISLOCAL*)stream->local)->msgs[i] = NULL;
  }

  ((SOLARISLOCAL*)stream->local)->buflen = data_buflen(CHUNK,CHUNK);
  ((SOLARISLOCAL*)stream->local)->buf = (char *) fs_get (((SOLARISLOCAL*)stream->local)->buflen);
  stream->sequence++;		/* bump sequence number */

  ((SOLARISLOCAL*)stream->local)->dirty = NIL;		/* no update yet */
  if (!stream->session_lockf)	/* session lockf opens the file */
    ((SOLARISLOCAL*)stream->local)->fd = -1;		/* not yet opened */
  ((SOLARISLOCAL*)stream->local)->id = -1;               /* meme chose */

  ((SOLARISLOCAL*)stream->local)->id_mapregion = NULL;	/* address of mmap() region for index file */
  ((SOLARISLOCAL*)stream->local)->id_mapregion_size = 0; /* size of same */
  ((SOLARISLOCAL*)stream->local)->mbox_mapregion = NULL;	/* address of mmap() */
  ((SOLARISLOCAL*)stream->local)->mbox_mapregion_size = 0;

  ((SOLARISLOCAL*)stream->local)->iname = NIL;		/* index name */
  ((SOLARISLOCAL*)stream->local)->read_index = NIL;      /* not read yet */
  ((SOLARISLOCAL*)stream->local)->index_hdr_dirty = NIL; /* clear flag */
  ((SOLARISLOCAL*)stream->local)->fsync = NIL;		/* force flags back to disk */
  ((SOLARISLOCAL*)stream->local)->fullck_done = NIL;     /* not yet done */
  stream->validchecksum = NIL;  /*  meme truc */
				/* allocate initial parsing buffer */
  solaris_get (CHUNK,PARSE_DATA);  /* parse buffer */
  ((SOLARISLOCAL*)stream->local)->max_pchunk = PARSE_SIZE;  /* initial size */
  solaris_get (CHUNK,BODY_DATA);   /* body buffer */
  ((SOLARISLOCAL*)stream->local)->body_data_cached = NIL;   /* first chunk does not count */
  solaris_get (CHUNK,HDR_DATA);    /* body buffer */
  ((SOLARISLOCAL*)stream->local)->hdr_data_cached = NIL;    /* first chunk does not count */
  solaris_get (CHUNK,TMP_DATA);    /* tmp buffer */
  ((SOLARISLOCAL*)stream->local)->tmp_data_cached = NIL;    /* first chunk does not count */
  ((SOLARISLOCAL*)stream->local)->read_access = NIL;        /* assume read/write */
				   /* check for read only*/
  if (solaris_mbox_access (stream,((SOLARISLOCAL*)stream->local)->name,W_OK,&err) && 
      (err == EACCES || err == EROFS)) {
    stream->rdonly = ((SOLARISLOCAL*)stream->local)->read_access= _B_TRUE;
  } else if (!stream->rdonly) {	/* examine command?  */
    /*NOP*/
  } else {
    ((SOLARISLOCAL*)stream->local)->read_access= _B_TRUE;/* Examine command */
  }

  /* abort if can't get RW silent stream */
  if (stream->silent && !stream->rdonly) {
    solaris_abort(stream, NIL);
    return NIL;
  } else {			/* clear group access permissions  */
    solaris_clear_group_permissions(stream);
  }

  /* parse mailbox  reset UID validity */
  stream->uid_validity = stream->uid_last = 0;
  stream->nmsgs = stream->recent = 0;
	
  /* parse mailbox can set readonly. */
  ((SOLARISLOCAL*)stream->local)->pseudo_size = PSEUDOLEN;/* pseudo msg size */

  /* parse opens the mail box */
  sg->lock_timeout = LOCKFILELIFETIME + 7;	/* [few more seconds] */
  if ((fd = solaris_parse(stream, tfile, LOCK_SH, _B_TRUE,
			  "open", &perr)) >= 0) {
    sg->lock_timeout = DEFAULTLOCKTIMEOUT;
    mail_unlock (stream);
    solaris_unlock (fd,stream,tfile);
    unmap_mbox(stream);	
  } else if (fd < 0) {	        /* FATAL  */
    sg->lock_timeout = DEFAULTLOCKTIMEOUT;
    /* Try unmap: May have already been done */
    unmap_mbox(stream);
    return NIL;
  }
  
  if (!((SOLARISLOCAL*)stream->local)) return NIL;	/* failure if stream died */
  /*
   * solars_parse(..) can implicitly set the stream TEMPORARILY
   * to readonly if one cannot get the lock RW there. Otherwise, 
   * see if we forced read_access during open.
   */

  /* make sure upper level knows readonly */
  if (!stream->rdonly) {
    stream->rdonly = ((SOLARISLOCAL*)stream->local)->read_access | bises;
  }

  /* notify about empty mailbox */
  if (!stream->nmsgs) {
    if (!stream->silent)
      mm_log ("109 Mailbox is empty",NIL, stream);
    mm_log_stream(stream,"109 Mailbox is empty"); /* ERROR */
  }
  if (!stream->rdonly)
    stream->perm_seen = stream->perm_deleted =
      stream->perm_flagged = stream->perm_answered = stream->perm_draft= _B_TRUE;
				/* for our minimum calculation */
  stream->min_dirty_msgno = stream->nmsgs + 1;
  stream->dead = NIL;		/* stream is open */
  return stream;		/* return stream alive to caller */
}


/*
 * Solaris mail close
 * Accepts: MAIL stream
 */
void
solaris_close(MAILSTREAM * stream, long options)
{
  if (stream && ((SOLARISLOCAL*)stream->local)) {
    int 	silent = stream->silent;

    stream->silent= _B_TRUE;	/* note this stream is dying */
    solaris_check(stream);	/* dump final checkpoint */
    if (options & CL_EXPUNGE) {
      solaris_expunge(stream);
    }
    stream->silent = silent;	/* restore previous status */
    solaris_abort(stream, NIL);	/* now punt the file and local data */
  }
  return;
}


/*
 * Solaris mail fetch message header
 * Accepts: MAIL stream
 *	    message # to fetch
 * Returns: message header in RFC822 format
 */
char *
solaris_fetchheader(MAILSTREAM    * stream,
		    unsigned long   msgno,
		    STRINGLIST 	  * lines,
		    unsigned long * len,
		    long 	    flags)
{
  FILECACHE 	*	m;
  boolean_t		mime_tr;
  boolean_t		not_out;
  STRINGLIST	*	local_lines;
  unsigned long 	anInt;
  STRINGLIST	*	unixList;
  char		*	hdr;
  char			unixFromLine[1024];
  int			fromLen;

  if (flags & FT_UID) {		/* UID form of call */
    for (anInt = 1; anInt <= stream->nmsgs; anInt++) {
      if (mail_uid(stream,anInt) == msgno) {
	return solaris_fetchheader(stream, anInt, lines, len, flags & ~FT_UID);
      }
    }
    return(NULL);		/* didn't find the UID */
  }

  /* Use the MIME translation if present */
  m = ((SOLARISLOCAL*)stream->local)->msgs[msgno - 1];
  local_lines = NIL;
  if (m->mime_tr) {
    m = m->mime_tr;
    mime_tr = _B_TRUE;
    /*
     * if no lines, then we filter out "filter_out" (above).
     * Since these are not "parsed out" in the mime tr. FILECACHE.
     */
    if (!lines) {
      char 		**	ignoreMe = filter_out;
      char		*	tptr;
      STRINGLIST 	**	slist = &local_lines;

      if (!(flags & FT_NOT)) {
	flags |= FT_NOT;	/* filter them OUT */
	not_out = _B_TRUE;
      } else {
	not_out = _B_FALSE;
      }
      for (tptr = *ignoreMe; tptr; tptr = *++ignoreMe) {
	*slist = mail_newstringlist();
	(*slist)->text = strdup(tptr);
	(*slist)->size = strlen(tptr);
	slist = &(*slist)->next;
      }
      lines = local_lines;
    }
  } else {
    mime_tr = _B_FALSE;
    if (!m->msg_cached) {
      if (!m->hdr_cached) {
	if(!solaris_acl_header(stream,m)) {
	  if (len) {
	    *len = 0;
	  }
	  return(NULL);
	} else {
	  m->hdr_cached = _B_TRUE;
	}
      }
    }
  }
  if (!(flags & FT_INTERNAL)) {
    /* copy the string */
    anInt=strcrlfcpy(&((SOLARISLOCAL*)stream->local)->buf,
		     &((SOLARISLOCAL*)stream->local)->buflen,
		     m->header,
		     m->rfc822_hdrsize);
    if (lines) {
      anInt = mail_filter(((SOLARISLOCAL*)stream->local)->buf,
			  anInt,
			  lines,
			  flags);

      /* If X-Unix-From not in the filter response, check if wanted */
      if (strstr(((SOLARISLOCAL*)stream->local)->buf,
		 xUnixFromHeader) == NULL) {

	for (unixList = lines; unixList != NULL; unixList = (*unixList).next) {
	  int	  oldlen = strlen(((SOLARISLOCAL*)stream->local)->buf);

	  if (strcasecmp(xUnixFromHeader, (*unixList).text) == 0) {
	    memset(unixFromLine, '\0', sizeof(unixFromLine));
	    fromLen = getUnixFrom(m, unixFromLine);
	    if (anInt + fromLen > ((SOLARISLOCAL*)stream->local)->buflen) {
	      char	* newData = calloc(1, anInt + fromLen + 1);

	      /* Copy over the old data */
	      memcpy(newData, ((SOLARISLOCAL*)stream->local)->buf, oldlen - 2);
	      free(((SOLARISLOCAL*)stream->local)->buf);
	      ((SOLARISLOCAL*)stream->local)->buf = newData;
	      ((SOLARISLOCAL*)stream->local)->buflen = anInt + fromLen;
	    }
	    /*	    ((SOLARISLOCAL*)stream->local)->buf[oldlen-2] = '\0';*/
	    strncat(((SOLARISLOCAL*)stream->local)->buf,
		    unixFromLine,
		    fromLen);
	    strcat(((SOLARISLOCAL*)stream->local)->buf, "\r\n");
	    anInt += fromLen;
	    break;
	  }
	}
      }
    }
    hdr = ((SOLARISLOCAL*)stream->local)->buf;	/* return processed copy */
  } else {			/* just return internal pointers */
    hdr = m->header;
    anInt = m->rfc822_hdrsize;
    if (lines) {		/* must parse header data */
      if (mime_tr) {
	/*
	 * Make a copy of the header because the filter writes
         * in the buffer which is PERMANENT data.
	 */
	if (HDR_SIZE < anInt) {
	  solaris_give(HDR_DATA);
	  solaris_get(anInt,HDR_DATA);
	  ((SOLARISLOCAL*)stream->local)->hdr_data_cached = _B_TRUE;
	}
	memcpy((void *)HDR_BASE,(void *)m->header,(size_t)anInt);
	hdr = HDR_BASE;
      } 
      anInt = mail_filter(hdr, anInt, lines, flags);
      
      /* If X-Unix-From did not come back from the filter, check if needed */
      if (strstr(hdr, xUnixFromHeader) == NULL) {
	for (unixList = lines; unixList != NULL; unixList = (*unixList).next) {
	  int	  oldlen = strlen(((SOLARISLOCAL*)stream->local)->buf);

	  if (strcasecmp(xUnixFromHeader, (*unixList).text) == 0) {
	    memset(unixFromLine, '\0', sizeof(unixFromLine));
	    fromLen = getUnixFrom(m, unixFromLine);
	    if ( anInt + fromLen > ((SOLARISLOCAL*)stream->local)->buflen) {
	      char	* newData = calloc(1, anInt + fromLen + 1);
	    
	      /* Copy over the old data */
	      memcpy(newData, ((SOLARISLOCAL*)stream->local)->buf, oldlen - 2);
	      free(((SOLARISLOCAL*)stream->local)->buf);
	      ((SOLARISLOCAL*)stream->local)->buf = newData;
	      ((SOLARISLOCAL*)stream->local)->buflen = anInt + fromLen;
	    }
	    ((SOLARISLOCAL*)stream->local)->buf[oldlen-2] = '\0';
	    strncat(((SOLARISLOCAL*)stream->local)->buf,
		    unixFromLine,
		    fromLen);
	    strcat(((SOLARISLOCAL*)stream->local)->buf, "\r\n");
	    anInt += fromLen;
	    break;
	  }
	}
      }
    }
  }

  if (local_lines) {		/* did local filter  */
    mail_free_stringlist(&local_lines);
    if (not_out) {
      flags &= ~FT_NOT;
    }
  }
  if (len) {
    *len = anInt;
  }
  return(hdr);
}


/*
 * Solaris mail fetch message text (body only)
 * Accepts: MAIL stream
 *	    message # to fetch
 * Returns: message text in RFC822 format
 */
char *
solaris_fetchtext(MAILSTREAM 	*	stream,
		  unsigned long 	msgno,
		  unsigned long *	len,
		  long 			flags)
{
  char 		*	txt;
  unsigned long 	i;
  FILECACHE 	*	fcache;
  MESSAGECACHE 	*	elt;

  if (flags & FT_UID) {		/* UID form of call */
    for (i = 1; i <= stream->nmsgs; i++) {
      if (mail_uid (stream,i) == msgno) {
	return(solaris_fetchtext(stream, i, len, flags & ~FT_UID));
      }
    }
    return(NULL);			/* didn't find the UID */
  }
  elt = mail_elt(stream,msgno);		/* get message status */
  /* get filecache */
  fcache = ((SOLARISLOCAL*)stream->local)->msgs[msgno - 1];

  /* Use the MIME translation if present */
  if (fcache->mime_tr) {
    fcache = fcache->mime_tr;
  } else if (!fcache->msg_cached) {
    if (!fcache->body_cached) {
      if (!solaris_acl_body(stream, fcache)) {
	if (len) {
	  *len = 0;
	}
	return(NULL);
      } else {
	fcache->body_cached = _B_TRUE;
      }
    }
  }

  /* if message not seen */
  if (!(flags & FT_PEEK) && !elt->seen) {
    elt->seen= _B_TRUE;		/* mark message as seen */

    /* recalculate Status/X-Status lines */
    if (!solaris_write_status(stream, msgno, elt)) {
      /* note stream is now dirty */
      ((SOLARISLOCAL*)stream->local)->dirty = _B_TRUE;

      if (msgno < stream->min_dirty_msgno) {	/* note minimum */
	stream->min_dirty_msgno = msgno;
      }
      stream->send_checksum = NIL;
    } else {
      /* See if we need to checksum */
      if (stream->sunvset) { /* caller has requested checksum changes */
	/* Changed to quick checksum */
	stream->send_checksum= _B_TRUE; /* tell the caller it is REQUIRED */
      }
      update_filetimes(stream);
    }
  }
  if (flags & FT_INTERNAL) {	/* internal data OK? */
    txt = fcache->body;
    i = fcache->bodysize;
  } else {			/* need to process data */
    i = strcrlfcpy(&((SOLARISLOCAL*)stream->local)->buf,
		   &((SOLARISLOCAL*)stream->local)->buflen,
		   fcache->body,
		   fcache->bodysize);
    txt = ((SOLARISLOCAL*)stream->local)->buf;
  }
  if (len) {
    *len = i;		/* return size */
  }
  return(txt);
}

/* Solaris mail fetch structure
 * Accepts: MAIL stream
 *	    message # to fetch
 *	    pointer to return body
 * Returns: envelope of this message, body returned in body value
 *
 * Fetches the "fast" information as well
 */

ENVELOPE *solaris_fetchstructure (MAILSTREAM *stream, unsigned long msgno, 
				  BODY **body,
				  long flags)
{
  ENVELOPE **env;
  BODY **b;
  STRING bs;
  LONGCACHE *lelt;
  FILECACHE *m;
  long i;

  if (flags & FT_UID) {		/* UID form of call */
    for (i = 1; i <= stream->nmsgs; i++)
      if (mail_uid (stream, i) == msgno)
	return solaris_fetchstructure (stream,i,body,flags & ~FT_UID);
    return NIL;			/* didn't find the UID */
  }
  if (stream->scache) {		/* short cache */
    if (msgno != stream->msgno){/* flush old poop if a different message */
      mail_free_envelope (&stream->env);
      mail_free_body (&stream->body);
    }
    stream->msgno = msgno;
    env = &stream->env;		/* get pointers to envelope and body */
    b = &stream->body;
  }
  else {			/* long cache */
    lelt = mail_lelt (stream,msgno);
    env = &lelt->env;		/* get pointers to envelope and body */
    b = &lelt->body;
  }
  if ((body && !*b) || !*env) {	/* have the poop we need? */
    mail_free_envelope (env);	/* flush old envelope and body */
    mail_free_body (b);
    /* o If m has an associated MIME translation, then
     *   we use it instead.
     * o If the msg in not cached, then cache it.
     */
    m = ((SOLARISLOCAL*)stream->local)->msgs[msgno-1];
    if (m->mime_tr) m = m->mime_tr;
    else if (!m->msg_cached) {
      if (!m->hdr_cached) {	/* read the header */
	if (!solaris_acl_header(stream,m)) {
	  if (body) *body = NIL;
	  return NIL;
	} else m->hdr_cached= _B_TRUE;
      }
      if (!m->body_cached) {	/* read the body */
	if (!solaris_acl_body(stream,m)) {
	  if (body) *body = NIL;
	  return NIL;
	} else m->body_cached= _B_TRUE;
      }
    }
    i = max (m->rfc822_hdrsize,m->bodysize);
    if (i > TMP_SIZE) {	/* make sure enough buffer space */
      ((SOLARISLOCAL*)stream->local)->tmp_data_cached= _B_TRUE;
      solaris_give(TMP_DATA);
      solaris_get (i,TMP_DATA);
    }
    INIT (&bs,mail_string,(void *) m->body,m->bodysize);
				/* parse envelope and body */
    solaris_parse_msg (env,body ? b : NIL,m->header,m->rfc822_hdrsize,&bs,
		       mylocalhost(stream),
		       TMP_BASE,
		       m->broken_content_len,
		       stream);
  }
  if (body) *body = *b;		/* return the body */
  return *env;			/* return the envelope */
}
/*
 * fetch envelope and body info for fetch short BUT do not 
 * do the body parse. Saves big overhead in startup.
 * The body is always NIL, ie, *body = NIL as passed */
ENVELOPE *
solaris_short_structure (MAILSTREAM *stream, unsigned long msgno, 
				  u_short *content_type,long flags)
{
  ENVELOPE	**	env = NULL;
  LONGCACHE	*	lelt = NULL;
  FILECACHE	*	m = NULL;
  BODY 		*	body = NULL;
  long 			i;
  boolean_t		mime_trans;

  if (flags & FT_UID) {		/* UID form of call */
    for (i = 1; i <= stream->nmsgs; i++) {
      if (mail_uid(stream, i) == msgno) {
	return solaris_short_structure(stream, i, content_type,
				       flags & ~FT_UID);
      }
    }
    return NULL;			/* didn't find the UID */
  }
  if (stream->scache) {		/* short cache */
    if (msgno != stream->msgno){/* flush old poop if a different message */
      mail_free_envelope(&stream->env);
    }
    stream->msgno = msgno;
    env = &stream->env;		/* get pointers to envelope and body */
  } else {			/* long cache */
    lelt = mail_lelt(stream,msgno);
    env = &lelt->env;		/* get pointers to envelope and body */
  }
  m = ((SOLARISLOCAL*)stream->local)->msgs[msgno-1];
  if (m->mime_tr) {
    m = m->mime_tr;
    mime_trans = _B_TRUE;
  } else {
    mime_trans = _B_FALSE;
  }

  if (m->content_type == TYPENOTSET || !*env) {	/* have the poop we need? */
    mail_free_envelope(env);	/* flush old envelope */
    /*
     * o If m has an associated MIME translation, then we use it instead.
     *
     * o If the msg in not cached, then cache it.
     */
    if (!mime_trans && !m->msg_cached) {
      if (!m->hdr_cached) {	/* read the header ONLY. NO body parse */
	if (!solaris_acl_header(stream,m)) {
	  return NULL;
	} else {
	  m->hdr_cached = _B_TRUE;
	}
      }
    }
    i = m->rfc822_hdrsize;
    if (i > TMP_SIZE) {	/* make sure enough buffer space */
      ((SOLARISLOCAL*)stream->local)->tmp_data_cached = _B_TRUE;
      solaris_give(TMP_DATA);
      solaris_get(i, TMP_DATA);
    }
    /*
     * parse envelope and body
     *
     * NOTE: we pass a NULL body string struct. This prevents
     * a FULL content parse.
     */
    solaris_parse_msg(env, &body, m->header, m->rfc822_hdrsize, NIL,
		      mylocalhost(stream),
		      TMP_BASE,
		      m->broken_content_len,
		      stream);
    m->content_type = body->type;
    mail_free_body(&body);	/* free this temporary body */    
  }
  *content_type = m->content_type;
  return *env;			/* return the envelope */
}

/* Solaris fetch message body as a structure
 * Accepts: Mail stream
 *	    message # to fetch
 *	    section specifier
 *	    pointer to length
 * Returns: pointer to section of message body
 * NOTE: mail_fetchstructure must be call BEFORE this call.
 */
char *solaris_fetchbody (MAILSTREAM *stream, unsigned long m, char *s,
			 unsigned long *len, long flags)
{
  BODY *b;
  PART *pt;
  unsigned long i,size = 0;	/* size added for TYPEMESSAGE */
  unsigned short enc = ENC7BIT;
  FILECACHE *fc;
  char *base,*dataptr;
  unsigned long offset = 0;
  MESSAGECACHE *elt;

  if (flags & FT_UID) {		/* UID form of call */
    for (i = 1; i <= stream->nmsgs; i++)
      if (mail_uid (stream,i) == m)
	return solaris_fetchbody (stream,i,s,len,flags & ~FT_UID);
    return NIL;			/* didn't find the UID */
  }
  elt = mail_elt (stream,m);
  if (!(solaris_fetchstructure (stream, m, &b, NIL) && b && s && *s &&
	isdigit (*s))) return NIL;
  if (!(i = strtol (s,&s,10)))	/* section 0 */
    return *s ? NIL : solaris_fetchheader (stream, m, NIL, len, flags);
  do {				/* until find desired body part */
				/* multipart content? */
    if (b->type == TYPEMULTIPART) {
      pt = b->contents.part;	/* yes, find desired part */
      if (!pt)			/* might be broken MIME */
	return NIL;
      while (--i && (pt = pt->next));
      if (!pt) return NIL;	/* bad specifier */
				/* note new body, check valid nesting */
      if (((b = &pt->body)->type == TYPEMULTIPART) && !*s) return NIL;
      offset = pt->offset;	/* get new offset */
    }
    else if (i != 1) return NIL;/* otherwise must be section 1 */
				/* need to go down further? */
    if (i = *s) switch (b->type) {
    case TYPEMESSAGE:		/* embedded message */
				/* check syntax of section ".<digit>" */
      if ((*s++ != '.') || !isdigit (*s)) return NIL;
				/* Body or section 0 (header)  */
      if (i = strtol (s,&s,10)) { 
	offset = b->contents.msg.offset;
	b = b->contents.msg.body;/* get its body */
      } else {			/* want the header */
	size = b->contents.msg.hdrsize;
	b = NIL;		/* do NOT want body */
      }
      break;
    case TYPEMULTIPART:		/* multipart, get next section */
      if ((*s++ == '.') && (i = strtol (s,&s,10)) > 0) break;
    default:			/* bogus subpart specification */
      return NIL;
    }
  } while (i);
				
  if (b) {			/* lose if body bogus */
    if (b->type == TYPEMULTIPART) return NIL;
    size = b->size.ibytes;	/* set size and encoding */
    enc = b->encoding;
  } else if (size == 0) return NIL;/* Not getting header. Lose */
				/* Take care of status */
  if (!(flags & FT_PEEK) && !elt->seen) {/* if message not seen */
    elt->seen= _B_TRUE;		/* mark message as seen */
				/* recalculate Status/X-Status lines */
    if (!solaris_write_status(stream,m,elt)) {
      ((SOLARISLOCAL*)stream->local)->dirty= _B_TRUE;		/* note stream is now dirty */
      if (m < stream->min_dirty_msgno) /* note minimum */
	stream->min_dirty_msgno = m;
      stream->send_checksum = NIL;
    } else {
      /* See if we need to checksum */
      if (stream->sunvset) { /* caller has requested checksum changes */
	/* Changed to quick checksum: 5 feb 95 Toutes les trois */
	stream->send_checksum= _B_TRUE; /* tell the caller it is REQUIRED */
      }
      update_filetimes(stream);
    }
  }
  /*
   * o If m has an associated MIME translation, then we use it instead.
   * o If the msg in not cached, then cache it.
   */
  fc = ((SOLARISLOCAL*)stream->local)->msgs[m-1];
  if (fc->mime_tr) base = fc->mime_tr->body;
  else {
    if (!fc->msg_cached) 
      if (!fc->body_cached) {	/* read the body */
	if (!solaris_acl_body(stream,fc)) {
	  return NIL;
	} else fc->body_cached= _B_TRUE;
      }
    base = fc->body;
  }
  /* Check for raw here  */
  dataptr = base+offset;	/* select desired data */
  if (flags & FT_INTERNAL) {	/* directly from our buffers  */
    if (len) *len = size;
    return dataptr;		/* the data itself */
  } else			/* Must get contents (returns a copy) */
    return rfc822_contents (&((SOLARISLOCAL*)stream->local)->buf,&((SOLARISLOCAL*)stream->local)->buflen,len,dataptr,
			    size,enc);
}

/*
 * NOthing to do here -- only necessary for imap2 driver */
void solaris_clear_iocount(MAILSTREAM *stream)
{
  return;
}

/*
 * Just prepare the status for a subsequent update
 * when we can write the file. If readonly, then the
 * status is set in memory. That is all.  */
static void
solaris_prepare_status(MAILSTREAM *stream,long i,MESSAGECACHE *elt)
{
  FILECACHE *m = ((SOLARISLOCAL*)stream->local)->msgs[i-1];
  /* first update the status */
  solaris_update_status(m->status, elt, _B_TRUE);
  m->dirty= _B_TRUE;
  ((SOLARISLOCAL*)stream->local)->dirty= _B_TRUE;		/* note stream is now dirty */
  if (i < stream->min_dirty_msgno) /* note minimum */
    stream->min_dirty_msgno = i;
  stream->send_checksum = NIL;
}

/* Solaris mail set flag
 * Accepts: MAIL stream
 *	    sequence
 *	    flag(s)
 */

void solaris_setflag (MAILSTREAM *stream, char *sequence, char *flag, long flags)
{
  char lockx[MAILTMPLEN];
  int fd;
  int locked;			/* True if we locked the file */
  MESSAGECACHE *elt;
  long i;
  long time0, time1, time_elapsed;
  short f =  mail_parse_flags (stream,flag, (unsigned long *)&i);
  if (!f) return;		/* no-op if no flags to modify */
				/* get sequence and loop on it */
				/* lock out non c-client applications */
#ifdef DONT_CACHE
  if ((fd = solaris_lock (stream, ((SOLARISLOCAL*)stream->local)->name,O_WRONLY,S_IREAD|S_IWRITE,
			  lockx,LOCK_EX,T)) < 0) { /* QUICK lock here */
    sprintf (((SOLARISLOCAL*)stream->local)->buf,"102 Can't lock mailbox: %s",strerror (errno));
    mm_log (((SOLARISLOCAL*)stream->local)->buf,WARN,stream);
    locked = NIL;
  } else locked= _B_TRUE;
#else
  locked = NIL;
#endif
				/* loop through the sequence  */
  if ((flags & ST_UID) ? mail_uid_sequence (stream,sequence) :
      mail_sequence (stream,sequence)) {
    time0 = elapsed_ms();
    for (i = 1; i <= stream->nmsgs; i++) {
      if ((i % 501) == 0) {
	time1 = elapsed_ms();
	time_elapsed = time1 - time0;
	if (time_elapsed >= ACKTIME) {
	  mm_notify (stream,"131 busy(STORE)",PARSE);
	  mm_log_stream(stream,"131 busy"); /* ERROR */
	  time0 = elapsed_ms();     /* reset timer */
	}
      }
      if ((elt = mail_elt (stream,i))->sequence) {
				/* set all requested flags */
	if (f&fSEEN) elt->seen= _B_TRUE;
	if (f&fDELETED) elt->deleted= _B_TRUE;
	if (f&fFLAGGED) elt->flagged= _B_TRUE;
	if (f&fANSWERED) elt->answered= _B_TRUE;
	if (f&fDRAFT) elt->draft= _B_TRUE;
	/*
         * If not locked, then set the status and postpone writing
         * until the next successful check */
	if (!locked) {
	  solaris_prepare_status(stream,i,elt);
	  continue;
	}
				/* recalculate Status/X-Status lines */
	if (!solaris_write_status_flags(stream,i,elt,fd)) {
	  ((SOLARISLOCAL*)stream->local)->dirty= _B_TRUE;		/* note stream is now dirty */
	  if (i < stream->min_dirty_msgno) /* note minimum */
	    stream->min_dirty_msgno = i;
	  stream->send_checksum = NIL;
	} else 
	  stream->send_checksum= _B_TRUE;
      }
    }
					/* update if we wrote something */
    if (locked) update_filetimes(stream);
  }
  if (locked)
    solaris_unlock(fd,stream,lockx);
}

/*
 * special optimization for setting one flag, and minmizing
 * the information one must return.
 */
int solaris_set1flag (MAILSTREAM *stream,char *sequence,char *flag, long flags)
{
  char lockx[MAILTMPLEN];
  MESSAGECACHE *elt;
  long i;
  int ret,fd,locked;
  long time0, time1, time_elapsed;
  short f = mail_parse_flags (stream, flag, (unsigned long *)&i);
  if (!f) return NIL;		/* no-op if no flags to modify */
				/* Lock up the mail file */
#ifdef DONT_CACHE
  if ((fd = solaris_lock (stream, ((SOLARISLOCAL*)stream->local)->name,O_WRONLY,S_IREAD|S_IWRITE,
			  lockx,LOCK_EX,T)) < 0) { /* QUICK lock here */
    sprintf (((SOLARISLOCAL*)stream->local)->buf,"102 Can't lock mailbox: %s",strerror (errno));
    mm_log (((SOLARISLOCAL*)stream->local)->buf,WARN, stream);
    locked = NIL;
  } else locked= _B_TRUE;
#else
  locked = NIL;
#endif
				/* get sequence and loop on it */
  if ((flags & ST_UID) ? mail_uid_sequence (stream,sequence) :
      mail_sequence (stream,sequence)) {
    time0 = elapsed_ms();   
    for (i = 1; i <= stream->nmsgs; i++) {
      if ((i % 501) == 0) {
	time1 = elapsed_ms();
	time_elapsed = time1 - time0;
	if (time_elapsed >= ACKTIME) {
	  mm_notify (stream,"131 busy(STORE)",PARSE);
	  mm_log_stream(stream,"131 busy"); /* ERROR */
	  time0 = elapsed_ms();     /* reset timer */
	}
      }
      if ((elt = mail_elt (stream,i))->sequence) {
				/* set all requested flags */
	if (f&fSEEN) elt->seen= _B_TRUE;
	if (f&fDELETED) elt->deleted= _B_TRUE;
	if (f&fFLAGGED) elt->flagged= _B_TRUE;
	if (f&fANSWERED) elt->answered= _B_TRUE;
	if (f&fDRAFT) elt->draft= _B_TRUE;
	/*
         * If not locked, then set the status and postpone writing
         * until the next successful check */
	if (!locked) {
	  solaris_prepare_status(stream,i,elt);
	  continue;
	}
				/* recalculate Status/X-Status lines */
	if (!solaris_write_status_flags(stream,i,elt,fd)) {
	  ((SOLARISLOCAL*)stream->local)->dirty= _B_TRUE;		/* note stream is now dirty */
	  if (i < stream->min_dirty_msgno) /* note minimum */
	    stream->min_dirty_msgno = i;
	  stream->send_checksum = NIL;
	} else 
	  stream->send_checksum= _B_TRUE;
      }
    }
    ret= _B_TRUE;
  } else
    ret = NIL;
  if (locked) {
    if (ret) update_filetimes(stream);
    solaris_unlock(fd,stream,lockx);
  }
  return ret;
}
/*
 * And the clear one flag too */
int solaris_clear1flag (MAILSTREAM *stream,char *sequence,char *flag, long flags)
{
  char lockx[MAILTMPLEN];
  int ret,fd,locked;
  MESSAGECACHE *elt;
  long i;
  long time0, time1, time_elapsed;
  short f = mail_parse_flags (stream, flag, (unsigned long *)&i);
  if (!f) return NIL;		/* no-op if no flags to modify */
				/* Lock up the mail file */
#ifdef DONT_CACHE
  if ((fd = solaris_lock (stream, ((SOLARISLOCAL*)stream->local)->name,O_WRONLY,S_IREAD|S_IWRITE,
			  lockx,LOCK_EX,T)) < 0) { /* QUICK lock here */
    sprintf (((SOLARISLOCAL*)stream->local)->buf,"102 Can't lock mailbox: %s",strerror (errno));
    mm_log (((SOLARISLOCAL*)stream->local)->buf,WARN,stream);
    locked = NIL;
  } else locked= _B_TRUE;
#else
  locked = NIL;
#endif
				/* get sequence and loop on it */
  if ((flags & ST_UID) ? mail_uid_sequence (stream,sequence) :
      mail_sequence (stream,sequence)) {
    time0 = elapsed_ms();  
    for (i = 1; i <= stream->nmsgs; i++) {
      if ((i % 501) == 0) {
	time1 = elapsed_ms();
	time_elapsed = time1 - time0;
	if (time_elapsed >= ACKTIME) {
	  mm_notify (stream,"131 busy(STORE)",PARSE);
	  mm_log_stream(stream,"131 busy"); /* ERROR */
	  time0 = elapsed_ms();     /* reset timer */
	}
      }
      if ((elt = mail_elt (stream,i))->sequence) {
				/* set all requested flags */
	if (f&fSEEN) elt->seen = NIL;
	if (f&fDELETED) elt->deleted = NIL;
	if (f&fFLAGGED) elt->flagged = NIL;
	if (f&fANSWERED) elt->answered = NIL;
	if (f&fDRAFT) elt->draft = NIL;
	/*
         * If not locked, then set the status and postpone writing
         * until the next successful check */
	if (!locked) {
	  solaris_prepare_status(stream,i,elt);
	  continue;
	}
				/* recalculate Status/X-Status lines */
	if (!solaris_write_status_flags(stream,i,elt,fd)) {
	  ((SOLARISLOCAL*)stream->local)->dirty= _B_TRUE;		/* note stream is now dirty */
	  if (i < stream->min_dirty_msgno) /* note minimum */
	    stream->min_dirty_msgno = i;
	  stream->send_checksum = NIL;
	} else 
	  stream->send_checksum= _B_TRUE;
      }
    }
    ret= _B_TRUE;
  } else
    ret = NIL;
  if (locked) {
    if (ret) update_filetimes(stream);
    solaris_unlock(fd,stream,lockx);
  }
  return ret;
}

/* Solaris mail clear flag
 * Accepts: MAIL stream
 *	    sequence
 *	    flag(s)
 */

void solaris_clearflag (MAILSTREAM *stream,char *sequence,char *flag,long flags)
{
  char lockx[MAILTMPLEN];
  MESSAGECACHE *elt;
  long i;
  int fd,locked;
  long time0, time1, time_elapsed;
  unsigned short checksum;
  long nbytes;
  short f = mail_parse_flags (stream,flag, (unsigned long *)&i);
  if (!f) return;		/* no-op if no flags to modify */
				/* Lock up the mail file */
#ifndef DONT_CACHE
  if ((fd = solaris_lock (stream, ((SOLARISLOCAL*)stream->local)->name,O_WRONLY,S_IREAD|S_IWRITE,
			  lockx,LOCK_EX,T)) < 0) { /* QUICK lock here */
    sprintf (((SOLARISLOCAL*)stream->local)->buf,"102 Can't lock mailbox: %s",strerror (errno));
    mm_log (((SOLARISLOCAL*)stream->local)->buf,WARN,stream);
    locked = NIL;
  } else locked= _B_TRUE;
#else
    locked = NIL;
#endif
				/* get sequence and loop on it */
  if ((flags & ST_UID) ? mail_uid_sequence (stream,sequence) :
      mail_sequence (stream,sequence)) {
    time0 = elapsed_ms();
    for (i = 1; i <= stream->nmsgs; i++) {
      if ((i % 501) == 0) {
	time1 = elapsed_ms();
	time_elapsed = time1 - time0;
	if (time_elapsed >= ACKTIME) {
	  mm_notify (stream,"131 busy(STORE)",PARSE);
	  mm_log_stream(stream,"131 busy"); /* ERROR */
	  time0 = elapsed_ms();     /* reset timer */
	}
      }
      if ((elt = mail_elt (stream,i))->sequence) {
	/* clear all requested flags */
	if (f&fSEEN) elt->seen = NIL;
	if (f&fDELETED) elt->deleted = NIL;
	if (f&fFLAGGED) elt->flagged = NIL;
	if (f&fANSWERED) elt->answered = NIL;
	if (f&fDRAFT) elt->draft = NIL;
	/*
         * If not locked, then set the status and postpone writing
         * until the next successful check */
	if (!locked) {
	  solaris_prepare_status(stream,i,elt);
	  continue;
	}
	/* recalculate Status/X-Status lines */
	if (!solaris_write_status_flags(stream,i,elt,fd)) {
	  ((SOLARISLOCAL*)stream->local)->dirty= _B_TRUE;		/* note stream is now dirty */
	  if (i < stream->min_dirty_msgno) /* note minimum */
	    stream->min_dirty_msgno = i;
	  stream->send_checksum = NIL;
	} else 
	  stream->send_checksum= _B_TRUE;
      }
    }
    if (locked)				/* update only if we wrote something */
      update_filetimes(stream);
  }
  if (locked)
    solaris_unlock(fd,stream,lockx);
}


/*
 * Modified so status is ALWAYS a fixed length for
 * writing in place.
 */
static void
solaris_update_status(char * status, MESSAGECACHE * elt, boolean_t doUid)
{
  strcpy(status, STATUS);

  if (elt->seen) {
    status[8] = 'R';
  } else {
    status[8] = ' ';
  }

  if (elt->deleted) {
    status[21] = 'D';
  } else {
    status[21] = '$';
  }

  if (elt->flagged) {
    status[22] = 'F';
  } else {
    status[22] = '$';
  }

  if (elt->answered) {
    status[23] = 'A';
  } else {
    status[23] = '$';
  }

  if (elt->draft) {
    status[24] = 'T';
  } else {
    status[24] = '$';
  }

  status[33] = '\0';
  if (doUid) {
#define UIDLEN 32
    char	tmp[UIDLEN];

    sprintf(tmp, "%010d\n\n", elt->uid);
    strcat(status, tmp);
  } else {
    strcat(status, "\n\n");
  }

  return;
}

/*
 * Passed fd here for flag fetching in a LOOP.
 * Locking is done outside of the loop
 */
static boolean_t
solaris_write_status_flags(MAILSTREAM 	* stream,
			   unsigned long  msgno,
			   MESSAGECACHE * elt,
			   int 		  fd)
{
  boolean_t 	ret;
  FILECACHE *	fcache = ((SOLARISLOCAL*)stream->local)->msgs[msgno-1];

  /* first update the status */
  solaris_update_status(fcache->status, elt, _B_TRUE);

  /* ONLY if r/w access and r/w permission */
  if (((SOLARISLOCAL*)stream->local)->read_access || stream->rdonly) {
    return(_B_FALSE);
  }
  ret = write_the_status(stream, msgno, fd, elt, fcache, _B_FALSE);

  /* Try later. But maybe we are having disk problems. */
  if (!ret) {
    fcache->dirty = _B_TRUE;
  }
  return ret;
}
/*
 * Write the status BACK to the file in place. Here we lock.
 */
static boolean_t
solaris_write_status(MAILSTREAM 	* stream,
		     unsigned long 	  msgno,
		     MESSAGECACHE 	* elt)
{
  char 		lockx[MAILTMPLEN];
  int 		fd;
  boolean_t	ret;
  FILECACHE *	fcache = ((SOLARISLOCAL*)stream->local)->msgs[msgno-1];

  /* first update the status */
  solaris_update_status(fcache->status, elt, _B_TRUE);

  /* ONLY if r/w access and r/w permission */
  if (((SOLARISLOCAL*)stream->local)->read_access || stream->rdonly) {
    return(_B_FALSE);
  }

  if ((fd = solaris_lock(stream,
			 ((SOLARISLOCAL*)stream->local)->name,
			 O_WRONLY,S_IREAD|S_IWRITE,
			 lockx,
			 LOCK_EX,
			 _B_TRUE)) < 0) { /* QUICK lock here */

    sprintf(((SOLARISLOCAL*)stream->local)->buf,
	    "102 Can't lock mailbox: %s",
	    strerror(errno));
    mm_log(((SOLARISLOCAL*)stream->local)->buf, WARN, stream);
    fcache->dirty = _B_TRUE;		/* dirty element. Try later. */
    solaris_unlock(fd, stream, lockx);
    return(_B_FALSE);
  }
  ret = write_the_status(stream, msgno, fd, elt, fcache, _B_FALSE);
  solaris_unlock(fd, stream, lockx);

  /* Try later. But maybe we are having disk problems. */
  if (!ret) {
    fcache->dirty = _B_TRUE;
  }
  return(ret);
}

/*
 * Write the status back, and update if required
 */
static boolean_t
write_the_status(MAILSTREAM 	* stream,
		 unsigned long 	  msgno,
		 int 		  fd,
		 MESSAGECACHE 	* elt,
		 FILECACHE 	* fcache,
		 boolean_t	  update) 
{
  off_t  		seekoffset;
  int 			len;

  /* first update the status */
  if (update) {
    solaris_update_status(fcache->status, elt, _B_TRUE);
  }

  /* Cannot write if we have read access only */
  if (((SOLARISLOCAL*)stream->local)->read_access || stream->rdonly) {
    return(_B_TRUE);
  }

  /* Seek to the status truc */
  seekoffset = fcache->seek_offset;
  seekoffset += fcache->status_offset;

  if (lseek(fd, seekoffset, L_SET) < 0) {
    return(_B_FALSE);
  }

  /* OK, write the status */
  len = strlen(fcache->status);
  if (safe_write(fd, fcache->status, len) < 0) {
    return(_B_FALSE);

  } else {
    int 	err;

    /* Checksum if necessary(sun-clients only) AND update the index entry */
    if (stream->sunvset) {
      solaris_quick_checksum(stream, msgno);
    }

    if (solaris_mbox_access(stream,stream->mailbox,W_OK,&err) == 0) {
      update_index_entry(stream, fcache, elt);
      solaris_clear_group_permissions(stream);
    }

    /* Force header update in check/close */
    ((SOLARISLOCAL*)stream->local)->index_hdr_dirty = _B_TRUE;
    ((SOLARISLOCAL*)stream->local)->fsync = _B_TRUE;
    return(_B_TRUE);
  }
}

/*
 * Solaris mail ping mailbox
 * Accepts: MAIL stream
 * Returns: _B_TRUE if stream alive, else _B_FALSE.
 * No-op for readonly files, since read/writer can expunge it from under us!
 */
long
solaris_ping(MAILSTREAM *stream)
{
  boolean_t	results = _B_FALSE;
  char		lock[MAILTMPLEN];
  int 		fd;
  struct stat 	sbuf;

  /* Something fatal happened */
  if (stream->dead) {
    return(_B_FALSE);
  }

  /* make sure it is alright to do this at all */
  if (!stream->lock) {
    /*
     * get current mailbox size.
     * parse if mailbox changed 
     */
    stat(((SOLARISLOCAL*)stream->local)->name,&sbuf);
    if (sbuf.st_size != ((SOLARISLOCAL*)stream->local)->filesize
	|| sbuf.st_ino != ((SOLARISLOCAL*)stream->local)->ino) {

      char *	perr = "";
      int lock_op = (((SOLARISLOCAL*)stream->local)->read_access ? LOCK_SH : LOCK_EX);
      if ((fd = solaris_parse(stream, lock, lock_op, NIL, "ping", &perr)) >= 0) {
	/* unlock mailbox */
	solaris_unlock(fd,stream,lock);
	mail_unlock(stream);	/* and stream */
	
      } else if (fd != NULLS_IN_NEWMAIL) {
	
	char	small[smallBUFLEN];

	sprintf(small, "Ping failed, fd = %d, error = %s", fd, perr);
	mm_log(small, WARN, stream);
	return(_B_FALSE);

      } else {
	/*
	 * NULLs - Still can see OLD mail.
	 * So, do not ABORT
	 */
	if (!stream->silent) {
	  mm_log("System error",ERROR, stream);
	}
	return(_B_TRUE);
      }
    }
  }
  if (!stream->silent)

    /* Periodic NOOPs can clear the read only flag */
    if (((SOLARISLOCAL*)stream->local)
	&& (((SOLARISLOCAL*)stream->local)->read_access
		  || stream->rdonly)) {
      mm_log("[READ-ONLY] 111 NOOP completed", NIL, stream);
    } else {
      mm_log("111 NOOP completed", NIL, stream);
    }

  /* return if still alive */
  return (stream->local != NULL ? _B_TRUE : _B_FALSE);
}

/*
 * Save any changed messages status that are in the cache.
 */
static void
save_cache(MAILSTREAM * stream)
{
  boolean_t	update = _B_TRUE;		/* update index header */
  unsigned long nmsgs = stream->nmsgs;

  if (!stream->rdonly && !((SOLARISLOCAL*)stream->local)->read_access) {
    if (((SOLARISLOCAL*)stream->local)->dirty) {

      unsigned long 		msgNum;
      int 			noErrors = _B_TRUE;

      for (msgNum = stream->min_dirty_msgno; msgNum <= nmsgs; ++msgNum) {
	FILECACHE 	* fcache
	  = ((SOLARISLOCAL*)stream->local)->msgs[msgNum-1];

	if (fcache->dirty) {
	  MESSAGECACHE * elt = mail_elt(stream, msgNum);

	  fcache->dirty = _B_FALSE;
	  if (!write_the_status(stream,
				msgNum,
				((SOLARISLOCAL*)stream->local)->fd,
				elt,
				fcache,
				_B_TRUE)) {
	    mm_notify(stream,
		      "Disk write failure updating status flags",
		      WARN);
	    noErrors = _B_FALSE;
	    stream->min_dirty_msgno = msgNum;
	    break;
	  }
	}
      }
      if (noErrors) {
	((SOLARISLOCAL*)stream->local)->dirty = _B_TRUE;
	stream->min_dirty_msgno = stream->nmsgs + 1;
      }
    }

    /* 
     *If no new messages || index needs updating:
     *      no new messages:  Update the time stamps
     *   index needs update:  checksum has changed by flag write
     *   MUST have write access 
     */
    if ((nmsgs == stream->nmsgs)
	|| ((SOLARISLOCAL*)stream->local)->index_hdr_dirty) {
      
      int 	err;

      ((SOLARISLOCAL*)stream->local)->index_hdr_dirty = _B_FALSE;
      if (solaris_mbox_access(stream, stream->mailbox, W_OK, &err) == 0) {
	update_index_hdr(stream);
	((SOLARISLOCAL*)stream->local)->fsync= _B_TRUE;
	update = _B_FALSE;		/* no need to update */
	solaris_clear_group_permissions(stream);
      }
    }

    /* See if an fsync of flags/index-info is required */
    if (((SOLARISLOCAL*)stream->local)->fsync) {
      fsync(((SOLARISLOCAL*)stream->local)->fd);		/* mailbox */
      if (((SOLARISLOCAL*)stream->local)->id) {
	fsync(((SOLARISLOCAL*)stream->local)->id); /* index file */
      }
      ((SOLARISLOCAL*)stream->local)->fsync = _B_FALSE;
    }
  }
  if (update) {
    update_index_hdr(stream);	/* update index hdir if necessary */
  }
  return;
}

/*
 * Solaris mail check mailbox
 * Accepts: MAIL stream
 * No-op for readonly files, since read/writer can expunge it from under us!
 *
 * Dtmail 
 */
void
solaris_check(MAILSTREAM * stream)
{
  char 		lock[MAILTMPLEN];
  int 		fd;
  unsigned long nmsgs = stream->nmsgs;
  int 		lockop = (((SOLARISLOCAL*)stream->local)->read_access
			  ? LOCK_SH : LOCK_EX);
  char 	*	perr;
  boolean_t	update = _B_TRUE;		/* update index header */

  save_cache(stream);

  /*
   * parse and lock mailbox 
   *   solaris_parse WILL save any new messages
   *   on a check.
   * BUT, dirty flags are not written back.
   * We now do that in place.
   * ((SOLARISLOCAL*)stream->local)->read_access MEAN NEVER WRITE ACCESS
   * We can always parse - even if read access,
   * but can not write back flags.
   */
  if ((fd = solaris_parse(stream, lock, lockop, NIL, "check", &perr)) >= 0) {

    /* NOW write back any flag changes that are around. */
    if (!stream->rdonly && !((SOLARISLOCAL*)stream->local)->read_access) {
      if (((SOLARISLOCAL*)stream->local)->dirty) {
	unsigned long 		msgNum;
	int 			noErrors = _B_TRUE;

	for (msgNum = stream->min_dirty_msgno; msgNum <= nmsgs; ++msgNum) {
	  FILECACHE 	* fcache
	    = ((SOLARISLOCAL*)stream->local)->msgs[msgNum-1];

	  if (fcache->dirty) {
	    MESSAGECACHE * elt = mail_elt(stream, msgNum);

	    fcache->dirty = _B_FALSE;
	    if (!write_the_status(stream, msgNum, fd, elt, fcache, _B_TRUE)) {
	      mm_notify(stream,
			"Disk write failure updating status flags",
			WARN);
	      noErrors = _B_FALSE;
	      stream->min_dirty_msgno = msgNum;
	      break;
	    }
	  }
	}
	if (noErrors) {
	  ((SOLARISLOCAL*)stream->local)->dirty = _B_TRUE;
	  stream->min_dirty_msgno = stream->nmsgs + 1;
	}
      }

      /* 
       *If no new messages || index needs updating:
       *      no new messages:  Update the time stamps
       *   index needs update:  checksum has changed by flag write
       *   MUST have write access 
       */
      if ((nmsgs == stream->nmsgs)
	  || ((SOLARISLOCAL*)stream->local)->index_hdr_dirty) {

	int 	err;

	((SOLARISLOCAL*)stream->local)->index_hdr_dirty = _B_FALSE;
	if (solaris_mbox_access(stream, stream->mailbox, W_OK, &err) == 0) {
	  update_index_hdr(stream);
	  ((SOLARISLOCAL*)stream->local)->fsync= _B_TRUE;
	  update = _B_FALSE;		/* no need to update */
	  solaris_clear_group_permissions(stream);
	}
      }

      /* See if an fsync of flags/index-info is required */
      if (((SOLARISLOCAL*)stream->local)->fsync) {
	fsync(fd);		/* mailbox */
	if (((SOLARISLOCAL*)stream->local)->id) {
	  fsync(((SOLARISLOCAL*)stream->local)->id); /* index file */
	}
	((SOLARISLOCAL*)stream->local)->fsync = _B_FALSE;
      }
    }

    /*
     * flush locks: 
     * NOTE: solaris_unlock updates access times so NO NEW MAIL
     *       will be reported by csh/biff etc ...
     */
    solaris_unlock (fd,stream,lock);
    if (update) {
      update_index_hdr(stream);	/* update index hdir if necessary */
    }
    mail_unlock (stream);
  }

  if (!stream->silent) {
    /* Periodic checks can clear the read only flag */
    if (((SOLARISLOCAL*)stream->local) 
	&& (((SOLARISLOCAL*)stream->local)->read_access
	    || stream->rdonly)) {
      mm_log("[READ-ONLY] 111 Check completed", NIL, stream);
    } else {
      mm_log("111 Check completed", NIL,  stream);
    }
  }
  return;
}
      


static int
sol_mksession_lock(MAILSTREAM * stream, char * name,char * imapname, int * fd)
{
#ifdef OLD_CRUFT
  char		small[SMALLBUFLEN];
  int 		protection;

  mail_stream_setNIL(stream);
  protection = (int)mail_parameters(stream,GET_LOCKPROTECTION,NIL);
  mail_stream_unsetNIL(stream);
  lockname(imapname,name);       /* get lockname */
  *fd = open(imapname,O_RDWR|O_CREAT,protection);
  /* Just in case of failure. We will go read only */
  if (*fd < 0 || !solaris_flock(stream, *fd, LOCK_EX|LOCK_NB)) 
    return NIL;
  else { 
    int i;
    /* Write our pid to the new lockfile */
    sprintf(small,"%d",getpid());
    i = strlen(small) + 1;
    safe_write(*fd,small,i);
    ftruncate(*fd,i);
    fsync(*fd);			/* sync back to disk */
    return(_B_TRUE);
  }
#endif
  return(_B_TRUE);
}

/*
 * Solaris mail expunge mailbox
 * Accepts: MAIL stream
 * Note: One must have unconditional access to
 *       expunge. 
 */
void
solaris_expunge(MAILSTREAM * stream)
{
  int 			fd;
  int			valid;
  long 			first_deleted = 1;
  long 			n_expunged = 0;
  unsigned long 	recent;
  MESSAGECACHE	*	elt;
  char 		*	resultMsg = "No messages deleted, so no update needed";
  char 			lock[PATH_MAX+1];
  char 			tmpname[PATH_MAX+1];
  char 		*	perr;
  /* if logical link, then real file name */
  char 			rootname[PATH_MAX+1];

  if (!stream->rdonly && !((SOLARISLOCAL*)stream->local)->read_access) {

    /*
     * parse and lock mailbox
     *
     * solaris_parse can implicitly set the stream readonly
     * if we can't get a shared lock on the mail file itself.
     * Thus, the test is 2nd in the && below
     */
    if ((fd = solaris_parse(stream, lock, LOCK_EX, NIL,
			    "expunge", &perr)) >= 0 &&
	!stream->rdonly) {

      int 		err;

      /* May need group access if running under client API */
      if (solaris_mbox_access(stream, stream->mailbox, W_OK, &err)) {
	if (!stream->silent) {
	  mm_log("158 Access denied!", NIL, stream);
	}
	solaris_unlock(fd, stream, lock);
	mail_unlock(stream);
	return;
      }

      recent = stream->recent;	/* get recent now that new ones parsed */
      while ((valid = (first_deleted <= stream->nmsgs)) && 
	     !(elt = mail_elt(stream, first_deleted))->deleted) {
	first_deleted++;	/* find first deleted message */
      }

      if (valid) {		/* ?found one, then EXPUNGE */
	/*
	 * make sure we can do the worst case thing:
	 * 0 or msgno of predecessor of first
	 */
	int 		write_start = first_deleted  - 1;
	struct stat 	sbuf;
	int 		tmp_fd;
	int 		success;
	boolean_t	soft = _B_FALSE;    	/* True if "soft" link */

	/*
	 * deleted msg.
	 * dump checkpoint of the results.
	 *
	 * if we deleted the first message, then we need to reset
	 * the seek base of message 1 for the save.
	 * Otherwise, we start the save with "write_start" which
	 * is the first deleted msgno.
	 *
	 * The point is that we only rewrite WHAT is necessary in the file
	 * rather than the entire file. Huge I/O savings for large mail files.
	 */
	if (write_start == 0) {		/* deleting message 1 first */
	  ((SOLARISLOCAL*)stream->local)->msgs[0]->seek_offset
	    = ((SOLARISLOCAL*)stream->local)->pseudo_size+1;
	}

	/*
	 * make the tmp file in which the expunge takes place 
	 */
	tmp_fd = solaris_create_tmp(stream, fd,
				    write_start, tmpname,
				    rootname, (int*)&soft);

	/*
	 * If our expunge succeed, then
	 *   (1) Compress the cache
	 *   (2) Annouce the results
	 */
	if (success = (tmp_fd >= 0)) {

	  /*
	   * flock so the solaris_unlock succeeds.
	   * DtMail: session locking can override.
	   */
	  solaris_flock(stream, tmp_fd, LOCK_EX);
	  if (success = solaris_exp_save_msgs(stream, fd,
					      tmp_fd, write_start,
					      &sbuf, lock)) {

	    int 		msgNum;
	    int			j;
	    char 		imaplock[PATH_MAX+1];
	    int 		lock_fd;
	    boolean_t 		nolock;
	    char	*	dest;

	    /*
	     * create session lock before rename frees the inode of
             * the current mailbox. Use inode of tmp file to create lockname.
	     */
	    if (!sol_mksession_lock(stream, tmpname, imaplock, &lock_fd)) {
	      stream->rdonly = _B_TRUE;
	      nolock = _B_TRUE;
	    } else {
	      nolock = _B_FALSE;
	    }

	    /* Get the destination name */
	    dest = (soft ? rootname : stream->mailbox);

	    fchown(tmp_fd, ((SOLARISLOCAL*)stream->local)->uid, -1);
	    fchown(tmp_fd, -1, ((SOLARISLOCAL*)stream->local)->gid);

	    /* rename the tmp file */
	    if (success = (rename(tmpname, dest) == 0)) {
	      ((SOLARISLOCAL*)stream->local)->filesize
		= ((SOLARISLOCAL*)stream->local)->tmp_filesize;

	      ((SOLARISLOCAL*)stream->local)->filetime = sbuf.st_mtime;

	      /* stream no longer dirty */
	      ((SOLARISLOCAL*)stream->local)->dirty = NIL;

	      /* Our new file descriptor */
	      ((SOLARISLOCAL*)stream->local)->fd = tmp_fd;

	      close(fd);
	      fd = tmp_fd;	/*  For solaris_unlock */

	      /* update seek offsets */
	      for (j = write_start; j < stream->nmsgs; j++) {
		FILECACHE *	fc = ((SOLARISLOCAL*)stream->local)->msgs[j];
		fc->seek_offset = fc->tmp_seek_offset;
	      }

	      msgNum = first_deleted;
	      do {		/* flush deleted messages */
		if ((elt = mail_elt(stream, msgNum))->deleted) {
		  FILECACHE *	fc;
 				
		  /* if recent, note one less recent message */
		  if (elt->recent) {
		    --recent;
		  }

		  /* flush local cache entry */
		  fc = ((SOLARISLOCAL*)stream->local)->msgs[msgNum - 1];
		  if (fc->mime_tr) { /* flush mime translation. */
		    fs_give((void **)&fc->mime_tr);
		  }
		  solaris_free_fcache(stream, msgNum - 1);

		  /* compress the cache  */
		  for (j = msgNum; j < stream->nmsgs; j++) {
		    ((SOLARISLOCAL*)stream->local)->msgs[j - 1]
		      = ((SOLARISLOCAL*)stream->local)->msgs[j];
		  }

		  ((SOLARISLOCAL*)stream->local)->msgs[stream->nmsgs-1] = NULL;

		  /* notify our caller */
		  mail_expunged(stream, msgNum);
		  n_expunged++;	/* count another expunged message */
		} else {
		  msgNum++;	/* otherwise try next message */
		}
	      } while (msgNum <= stream->nmsgs);

	      /*
	       * update the index file:
	       * here will just recreate it.
	       */
	      recreate_index_file(stream);

	    } else {
	      if (!stream->silent) {
		mm_log("153 Expunge aborted: rename failed", ERROR, stream);
	      }
	    }
	  }
	}

	if (!success) {	/* Toss the tempfile. */	
	  close(tmp_fd);
	  unlink(tmpname);
	  sprintf(((SOLARISLOCAL*)stream->local)->buf,
		  "155 Expunge aborted, error creating tmp file: %s",
		  strerror(errno));
	  mm_log(((SOLARISLOCAL*)stream->local)->buf, WARN, stream);
	}
	sprintf((resultMsg = ((SOLARISLOCAL*)stream->local)->buf),
		"Expunged %d messages", n_expunged);
      }

      /* notify upper level, free locks */
      mail_exists(stream, stream->nmsgs);
      mail_recent(stream, recent);

      /* reset the min out of range */
      stream->min_dirty_msgno = stream->nmsgs + 1;

      /* DtMail: solaris unlock takes care of session lockf */
      solaris_unlock(fd, stream,lock); 
      solaris_clear_group_permissions(stream);

    } else {
      if (stream->rdonly) {		/* parse ok -- mailbox now rdonly */
	resultMsg = "108 Expunge ignored on readonly mailbox";
      } else {
	resultMsg = "102 Cannot lock mailbox";
      }
    }

    /*
     * Release the mail lock if the parse succeeded which leaves 
     * the mail lock set.
     */
    if (fd >= 0) {
      mail_unlock(stream);
    }
  } else {
    resultMsg = "108 Expunge ignored on readonly mailbox";
  }

  if (((SOLARISLOCAL*)stream->local) && !stream->silent) {
    mm_log(resultMsg, NIL, stream);
    mm_log_stream(stream, resultMsg);
  }
  return;
}

/*
 * Solaris mail copy message(s)
 * Accepts: MAIL stream
 *	    sequence
 *	    destination mailbox
 * Returns: T if copy successful, else NIL
 */
long
solaris_copy(MAILSTREAM * stream,char * sequence, char * mailbox, long options)
{
  /* copy the messages */
  long 			i;
  MESSAGECACHE *	elt;

  if (!(((options & CP_UID)
	 ? mail_uid_sequence(stream, sequence)
	 : mail_sequence (stream, sequence))
	&& solaris_copy_messages(stream, mailbox))) {
    return(_B_FALSE);
  }

  /* delete all requested messages */
  if (options & CP_MOVE) {
    for (i = 1; i <= stream->nmsgs; i++)
      if ((elt = mail_elt(stream, i))->sequence) {
	elt->deleted= _B_TRUE;		/* mark message deleted */

	/* recalculate Status/X-Status lines */
	if (!solaris_write_status(stream, i, elt)) {
	  /* note stream is now dirty */
	  ((SOLARISLOCAL*)stream->local)->dirty= _B_TRUE;
	  if (i < stream->min_dirty_msgno) { /* note minimum */
	    stream->min_dirty_msgno = i;
	  }
	  stream->send_checksum = NIL;
	} else {
	  stream->send_checksum= _B_TRUE;
	}
      }
    update_filetimes(stream);
  }
  return(_B_TRUE);
}

static
update_filetimes(MAILSTREAM *stream)
{
  struct stat 	sb;
  stat(stream->mailbox, &sb);
  ((SOLARISLOCAL*)stream->local)->filetime = sb.st_mtime;
  ((SOLARISLOCAL*)stream->local)->accesstime = sb.st_atime;

  return(0);
}

/*
 *    Took append code from c-client in imap-3.1
 *    and made appropriate modifications for the solaris driver.
 *
 * Solaris   mail append character
 * Accepts: file descriptor
 *	    output buffer
 *	    pointer to current size of output buffer
 *	    character to append
 * Returns: _B_TRUE if append successful, else _B_FALSE
 */

long
solaris_append_putc(int fd, char * s, int * i, char c)
{
  s[(*i)++] = c;
  if (*i == BUFLEN) {		/* dump if buffer filled */
    if (safe_write(fd, s, *i) < 0) {
      return(_B_FALSE);
    }
    *i = 0;			/* reset */
  }
  return(_B_TRUE);
}

/*
 * Solaris  mail append message from stringstruct
 * Accepts: MAIL stream
 *	    destination mailbox
 *	    stringstruct of messages to append
 * Returns: _B_TRUE if append successful, else _B_FALSE
 */
long
solaris_append(MAILSTREAM	*	stream,
	       char 		*	mailbox,
	       char 		*	flags,
	       char 		*	date,
	       char		*	xUnixFrom,
	       STRING 		*	message)
{
  /* No ">" if content-length found */
  boolean_t		content_len_found = _B_FALSE;
  boolean_t		ok = _B_TRUE;
  boolean_t		lf_lf = _B_FALSE;	/* Found "\n\n" */
  struct stat 		sbuf;
  struct utimbuf	tp;
  MESSAGECACHE 		elt;
  int			i;
  int			fd;
  int			ti;
  int			zn;
  int			rv;
  int 			err;
  char 		*	s;
  char		*	x;
  char			buf[BUFLEN];
  char			file[PATH_MAX+1];
  char			lock[PATH_MAX+1];
  char 			c = '\n';
  char			d = 0;
  long 			j;
  long			n;
  time_t 		t = time (0);
  unsigned long 	size = SIZE(message);
  short 		shortFlags = mail_parse_flags(stream,
						      flags,
						      (unsigned long *)&j);
  /* check access */
  if (solaris_path_access(stream,
			  dummy_file (file,mailbox,stream),
			  W_OK, &err)) {
    if (errno == ENOENT) {
      solaris_invalid_error(stream,mailbox, "copy", NIL);
    } else {
      mm_log("158 Access denied!", WARN, stream);
      mm_log("Access denied!", ERROR, stream);
    }
    return(_B_FALSE);
  } else {
    solaris_clear_group_permissions(stream);
  }

  /* parse date if given */
  if (date && !mail_parse_date(&elt, date)) {
    sprintf(buf, "Bad date in append: %s", date);
    mm_log(buf, ERROR, stream);
    return(_B_FALSE);
  }

  /* make sure valid mailbox */
  if (!solaris_isvalid(mailbox, buf, stream, _B_FALSE)) {
    /* Print message. Returns _B_FALSE if one cannot continue */
    if (!solaris_invalid_error(stream, mailbox, "append", _B_TRUE)) {
      return(_B_FALSE);
    }
  }

  /* Note that locking internally uses access checks */
  if ((fd = solaris_nowait_lock(stream,
				dummy_file(file, mailbox, stream),
				O_WRONLY|O_APPEND|O_CREAT,
				S_IREAD|S_IWRITE,
				lock,
				LOCK_EX)) < 0) {
    sprintf(buf,"158 Access denied on append: %s", strerror(errno));
    mm_log(buf, WARN, stream);
    mm_log("Access denied", ERROR, stream);
    return(_B_FALSE);
  }

  mm_critical(stream);		/* go critical */
  stat(stream->mailbox, &sbuf);		/* get current file size */

  /* Append the X-Unix-From: line, if provided */
  if (xUnixFrom != NULL) {
    char	*	colon = xUnixFrom;
    boolean_t		hasXUnixFromHeader = _B_FALSE;

    /* If 'X-Unix-From:' was passed down, skip it and get the right side */
    if (strncmp(xUnixFromHeader, xUnixFrom, strlen(xUnixFromHeader)) == 0) {
      hasXUnixFromHeader = _B_TRUE;
    }

    if (hasXUnixFromHeader) {
      colon = strchr(xUnixFrom, ':');
      colon++;
    }
      
    sprintf(buf, "From %s\n", colon);
    if (!hasXUnixFromHeader) {
      strcat(buf, xUnixFromHeader);
      strcat(buf, ": ");
    }
    strcat(buf, xUnixFrom);
    strcat(buf, "\n");
  } else {
    sprintf(buf, "From %s@%s ", myusername(stream), mylocalhost (stream));
  }

  /* write the date given */
  if (date) {
    mail_cdate(buf + strlen (buf), &elt);

  } else {
    char	buf_time[26+1];    /* otherwise write the time now */

#if (_POSIX_C_SOURCE - 0 >= 199506L)
    strcat(buf, ctime_r(&t, buf_time));
#else
    strcat(buf, ctime_r(&t, buf_time, 26));
#endif
  }

  sprintf(buf + strlen (buf),"Status: %sO\nX-Status: %s%s%s%s\n",
	  shortFlags & fSEEN 	 ? "R" : " ",
	  shortFlags & fDELETED  ? "D" : "$",
	  shortFlags & fFLAGGED  ? "F" : "$",
	  shortFlags & fANSWERED ? "A" : "$",
	  shortFlags & fDRAFT 	 ? "T" : "$");

  i = strlen(buf);		/* initial buffer space used */
  content_len_found = _B_FALSE;

  while (ok && size--) {	/* copy text, tossing out CR's */

    /* if at start of line */
    if ((c == '\n') && (size > 5)) {
      n = GETPOS (message);	/* prepend a broket if needed */
      if ((SNX(message) == 'C') && (SNX(message) == 'o') &&
	  (SNX(message) == 'n') && (SNX(message) == 't') &&
	  (SNX(message) == 'e') && (SNX(message) == 'n') &&
	  (SNX(message) == 't') && (SNX(message) == '-') &&
	  (SNX(message) == 'L') && (SNX(message) == 'e') &&
	  (SNX(message) == 'n') && (SNX(message) == 'g') &&
	  (SNX(message) == 't') && (SNX(message) == 'h') &&
	  (SNX(message) == ':') && (SNX(message) == ' ') &&
	  !lf_lf) {

	content_len_found = _B_TRUE;
      }

      SETPOS (message,n);	/* restore position as needed */
      if ((SNX(message) == 'F') && (SNX(message) == 'r') &&
	  (SNX(message) == 'o') && (SNX(message) == 'm') &&
	  (SNX(message) == ' ') &&
	  !content_len_found) { /* we believe content-length. No broke */

	SOLARIS_GLOBALS *sg = (SOLARIS_GLOBALS *)stream->solaris_globals;

	/* always write widget if unconditional */
	if (sg->solaris_fromwidget) {
	  ok = solaris_append_putc (fd, buf, &i, '>');

	} else {		/* hairier test, count length of this line */

	  for (j = 6; (j < size) && (SNX(message) != '\n'); j++);

	  if (j < size) {	/* copy line */
	    SETPOS(message, n);	/* restore position */
	    x = s = (char *) fs_get(j + 1);
	    while (j--) {
	      if ((c = SNX (message)) != '\015') {
		*x++ = c;
	      }
	    }

	    *x = '\0';		/* tie off line */
	    VALID(s, &x, &ti, &zn, &rv);/* see if looks like need a widget */
	    if (ti) {
	      ok = solaris_append_putc(fd, buf, &i, '>');
	    }
	    fs_give((void **) &s);
	  }
	}
      }
      SETPOS(message,n);	/* restore position as needed */
    }
				/* copy another character */
    if ((c = SNX(message)) != '\015') {
      ok = solaris_append_putc(fd, buf, &i, c);
      if (d == '\n' && c == '\n') {
	lf_lf = _B_TRUE;
      }
      d = c;
    }
  }

  /* write trailing newline */
  if (ok) {
    ok = solaris_append_putc(fd, buf, &i, '\n');
  }

  if (!(ok && (ok = (safe_write(fd, buf, i) >= 0))
	&& (ok = !fsync(fd)))) {
    sprintf(buf, "Message append failed: %s", strerror(errno));
    mm_log(buf, ERROR, stream);
    ftruncate(fd, sbuf.st_size);
  }

  tp.actime = sbuf.st_atime;	/* preserve atime */
  tp.modtime = time(0);		/* set mtime to now */
  utime(file, &tp);		/* set the times */
  solaris_unlock(fd, NIL, lock);/* unlock */
  close(fd);			/* close mailbox */
  mm_nocritical(stream);	/* release critical */
  return(ok);			/* return success */
}

/*
 * Prints an error message for mailbox "cmd" operation.
 *  inboxok allows Return T on ENOENT  error.
 * Accepts:
 *  mailbox   mailbox name
 *  cmd       which cmd called, eg, copy
 *  inboxok   T if one can continue on non-existent INBOX
 * Returns:
 *  NIL       if the error is not continuable.
 *    T       otherwise.
 */
int
solaris_invalid_error(MAILSTREAM * stream,
		      char 	 * mailbox,
		      char 	 * cmd,
		      int	   inboxok)
{
  char buf[MAILTMPLEN];
  switch (errno) {
  case ENOENT:			/* no such file? */
    if (strcmp (ucase (strcpy (buf,mailbox)), "INBOX")) {
      /* NOT INBOX */
      sprintf(buf, "[TRYCREATE] Must create mailbox before %s", cmd);
      mm_notify (stream, buf, NIL);
      return NIL;
    } else if (inboxok) return(_B_TRUE);
    else return NIL;
  case 0:			/* merely empty file? */
    break;
  case EINVAL:
    sprintf (buf, "Invalid Solaris-format mailbox name: %s",mailbox);
    mm_log (buf,ERROR,stream);
    return NIL;
  case EACCES:
    sprintf (buf, "158 Access denied: %s",mailbox);
    mm_log(buf,WARN, stream);
    mm_log ("Access denied",ERROR, stream);
    return NIL;
  default:
    sprintf (buf, "120 Not a Solaris-format mailbox: %s",mailbox);
    mm_log (buf,WARN,stream);
    mm_log ("Not a Solaris-format mailbox",ERROR, stream);
    return NIL;
  }
  return(_B_TRUE);
}

/* Solaris garbage collect stream
 * Accepts: Mail Stream
 *          garbage collection flags
 */
void solaris_gc (MAILSTREAM *stream, long gcflags)
{
  return;			/* Pas encore implemente. */
}



/* INTERNAL ROUTINES */

/*
 * Solaris mail abort stream
 * Accepts: MAIL stream
 */
void
solaris_abort(MAILSTREAM * stream, int rm_index)
{
  long 		i;
  int 		err;

  /*
   * Some cleanup must be done whether or not we have permissions.
   * We should since we opened the mailbox. Just try and set them.
   */
  solaris_mbox_access(stream, stream->mailbox, W_OK, &err);

  if (((SOLARISLOCAL*)stream->local)) {		/* only if a file is open */
    if (((SOLARISLOCAL*)stream->local)->fd >= 0) {

      /* DtMail: unlock if lockf on the session */
      if (stream->session_lockf) {
	solaris_flock(stream, ((SOLARISLOCAL*)stream->local)->fd, LOCK_UN);
      }
      close(((SOLARISLOCAL*)stream->local)->fd); /* close the mail file */
    }

    if (((SOLARISLOCAL*)stream->local)->name) {
      fs_give ((void **) &((SOLARISLOCAL*)stream->local)->name);
    }

    if (((SOLARISLOCAL*)stream->local)->id >= 0) {
      unlock_index(stream, ((SOLARISLOCAL*)stream->local)->id);
      close(((SOLARISLOCAL*)stream->local)->id);
      /*
       * Remove the index file if necessary
       */
      if (rm_index && ((SOLARISLOCAL*)stream->local)->iname) {
	unlink(((SOLARISLOCAL*)stream->local)->iname);
      }

      /* Free it. */	
      if (((SOLARISLOCAL*)stream->local)->iname) {
	fs_give((void **)&((SOLARISLOCAL*)stream->local)->iname);
      }
    }

    /*
     * Clear any group permissions if applicable
     */
    if (stream) {
      solaris_clear_group_permissions(stream);
    }

    if (((SOLARISLOCAL*)stream->local)->msgs) {		/* free local cache */

      for (i = 0; i < stream->nmsgs; ++i) {
	FILECACHE *	fc = ((SOLARISLOCAL*)stream->local)->msgs[i];

        /* If we have a mime translation, free it first */
	if (fc->mime_tr) {
	  fs_give((void **) &fc->mime_tr);
	}

	/* Now, free the cache itself */
	solaris_free_fcache(stream,i);
      }
      /* Now free the cache pointers */
      fs_give((void **)&((SOLARISLOCAL*)stream->local)->msgs);
    }

    /* free local text buffers */
    if (((SOLARISLOCAL*)stream->local)->buf) {
      fs_give ((void **) &((SOLARISLOCAL*)stream->local)->buf);
    }

    /* pmapped data */
    solaris_give(PARSE_DATA);
    solaris_give(BODY_DATA);
    solaris_give(TMP_DATA);
    solaris_give(HDR_DATA);
			
    /* nuke the local data */
    fs_give((void **) &stream->local);
    stream->dtb = NULL;		/* log out the DTB */
    stream->dead = _B_TRUE;            /* stream useless */
  }
  return;
}

/*
 * Calculate the ceiling multiple of CHUNKS:
 *   (k + chunk - 1)/chunk, k >= 0, PSPARE > 0.
 */
#define PSPARE 16
static unsigned long
data_buflen(unsigned long nbytes,int chunk)
{
  unsigned long len = nbytes + PSPARE + chunk - 1;
  len = len/chunk;
  len *= chunk;
  return len;
}

static boolean_t
not_pseudo_msg(char * s, int rml)
{
  if (rml != PSEUDOLEN) {
    return(_B_TRUE);
  }

  /* OK, look at the data */
  if (strncasecmp(s, PSEUDOFROMLINE, PSEUDOENDLEN) != 0) {
    return(_B_TRUE);
  } else {

    /* check the body */
    char 	*	cul = s + rml-PSEUDOENDLEN;

    if (strncasecmp(cul, PSEUDOEND, PSEUDOENDLEN) != 0) {
      return(_B_TRUE);
    } else {
      return(_B_FALSE);
    }
  }
}

/*
 * Check to see if the write date has changed, AND,
 * the read date has changed
 */
static possibly_written(MAILSTREAM *stream,struct stat *sb)
{
  if (((SOLARISLOCAL*)stream->local)->filetime == 0 || ((SOLARISLOCAL*)stream->local)->accesstime == 0)
    return NIL;
  else 
    if (((SOLARISLOCAL*)stream->local)->filetime != sb->st_mtime && 
	((SOLARISLOCAL*)stream->local)->accesstime != sb->st_atime)
      return(_B_TRUE);
    else return NIL;
}

static void clear_status(MAILSTREAM *stream,unsigned long msgno)
{
  MESSAGECACHE *elt = mail_elt(stream,msgno);
  elt->seen =  elt->deleted = elt->flagged = elt->answered = elt->draft = NIL;
  elt->uid = NIL;
}

static boolean_t
set_filecache(MAILSTREAM *stream,unsigned long nmsgs)
{
  unsigned long msgNumber;

  mailcache_t mc = (mailcache_t)mail_parameters (stream,GET_CACHE,NIL);
  /* OK to pass just the stream */
  boolean_t abort = _B_FALSE;

  if (nmsgs >= ((SOLARISLOCAL*)stream->local)->cachesize) {
    (*mc) (stream,nmsgs,CH_SIZE);
    /* Expand our LOCAL file cache */
    ((SOLARISLOCAL*)stream->local)->cachesize = nmsgs + CACHEINCREMENT;
    /* resize: already have a cache */
    fs_resize ((void **)&((SOLARISLOCAL*)stream->local)->msgs,((SOLARISLOCAL*)stream->local)->cachesize*sizeof (FILECACHE *));
  }

  /* OK, read our index file, and set our cache elements */
  mmap_index_file(stream,T);
  for (msgNumber = 0; msgNumber < nmsgs; msgNumber++) {
    FILECACHE *	f;
    INDEXENTRY 	ie;

    f = ((SOLARISLOCAL*)stream->local)->msgs[msgNumber] = (FILECACHE *)fs_get(sizeof(FILECACHE));
    memset(f,0,sizeof(FILECACHE));

    if (read_index_entry(stream,((SOLARISLOCAL*)stream->local)->id,&ie,msgNumber+1,T)) {

      MESSAGECACHE *	elt = mail_elt(stream, msgNumber+1);

      elt->seen = ie.seen;
      elt->deleted = ie.deleted;
      elt->flagged = ie.flagged;
      elt->answered = ie.flagged;
      elt->draft = ie.draft;
      elt->uid = ie.uid;
      elt->valid= _B_TRUE;
      solaris_update_status(f->status, elt, _B_TRUE);
      f->content_type = TYPENOTSET;
      f->headersize = ie.headersize;
      f->rfc822_hdrsize = ie.rfc822_hdrsize;
      f->bodysize = ie.bodysize;
      f->body_offset = ie.body_offset;
      f->status_offset = ie.status_offset;
      f->real_mem_len = ie.real_mem_len;
      elt->rfc822_size = f->rfc822_size = ie.rfc822_size;
      f->seek_offset = ie.seek_offset;
      f->chksum = ie.chksum;
      f->status_chksum = ie.status_chksum;
      f->chksum_size = ie.chksum_size;
      f->status_len = ie.status_len;
      f->quick_chksum_cycles = ie.quick_chksum_cycles;
      elt->hours = ie.hours;
      elt->minutes = ie.minutes;
      elt->seconds = ie.seconds;
      elt->zoccident = ie.zoccident;
      elt->zhours = ie.zhours;
      elt->zminutes = ie.zminutes;
      elt->day = ie.day;
      elt->month = ie.month;
      elt->year = ie.year;
    } else {
      abort = _B_TRUE;
      break;
    }
  }
  unmap_index_file(stream);

  if (abort) {
    int 	msg;
    for (msg = 0; msg <= msgNumber; ++msg) {
      fs_give((void **)&((SOLARISLOCAL*)stream->local)->msgs[msg]);
      clear_status(stream,msg+1);
    }
    return(_B_FALSE);
  } else {
    return(_B_TRUE);
  }
  /*NOTREACHED*/
}

/* solaris mail parse and lock mailbox
 *
 * Uses "Content-length: " to find start of next message.
 *
 * Accepts: MAIL stream
 *	    space to write lock file name
 * Returns: file descriptor if parse OK, mailbox is locked shared
 *	    -1 if failure, stream aborted
 */

int
solaris_parse(MAILSTREAM * stream,
	      char * lock,
	      int lockop,
	      int openparse,
	      char * caller,
	      char ** perr)
{

  INDEXHDR 	ihdr;	/* index file header */
  STRING 	bs;
  MESSAGECACHE * elt;
  mailcache_t 	mc = (mailcache_t)mail_parameters(stream, GET_CACHE, NIL);
  struct stat 	sbuf;

  /* True iff we parse the pseudo-message */
  boolean_t	pseudo_parsed = _B_FALSE;

  /* Forces a rewrite of pseudo msg */
  boolean_t	must_rewrite_pseudo = _B_FALSE;

  /* if caching forced because of error */
  boolean_t	caching_forced = _B_FALSE;

  /* _B_TRUE if mesasge found with bad status fields */
  boolean_t	rewrite_status = _B_FALSE;

  unsigned long nuids_changed = 0;/* counts changed uids */
  unsigned long parset0 = 0;
  unsigned long	parset1;
  long 		delta;
  long		i;
  long		j;
  long		is;
  long		is1;
  long 		prevuid = 0;
  long 		nmsgs_at_entry = stream->nmsgs;
  long 		nmsgs = stream->nmsgs;
  long 		newcnt = 0;	/* new messages this pass */
  long 		time0;		/* Seek correction for broken pseudo msg */
  long		time1;
  long		time_elapsed;

  int 		fd;
  int 		ti;
  int		zn;
  int		rv;
  int 		c = '\n';	/* Marks NEW data - first parse pass */
  int 		translations = 0;/* Counts sun attachments trans. to MIME */
  int 		min_rewrite_msgno = 0;     /* where rewrite starts */
  int		WR_access;	/* set for each pass */
  int		flags;		/* open flags */
  int		write_file; 	/* True if we have new msgs to update */
  int 		err;

  char	*	s;
  char	*	s1;
  char	*	t;
  char	*	e;
  char 		small[SMALLBUFLEN];

  /* Bad news if we are locked. Log it for debugging */
  if (stream->lock) {
    sprintf(small, "%s called solaris_parse with lock set!", caller);
    mm_log(small, WARN, stream);
  }
  mail_lock(stream);		 /* guard against recursion or pingers */
				 /* open and lock mailbox (shared OK) */
  if (((SOLARISLOCAL*)stream->local)->read_access) {
    flags = O_RDONLY;
  } else {
    flags = O_RDWR;  
  }

  if ((fd = solaris_lock(stream,
			 ((SOLARISLOCAL*)stream->local)->name,
			 flags,
			 NIL,
			 lock,
			 lockop,
			 NIL)) < 0) {
    if (((SOLARISLOCAL*)stream->local)->fd < 0) {
      sprintf(((SOLARISLOCAL*)stream->local)->buf,
	      "154 Mailbox open failed, %s",
	      strerror(errno));
      if (!stream->silent) {
	mm_log(((SOLARISLOCAL*)stream->local)->buf, ERROR, stream);
	/* ERROR */
	mm_log_stream(stream, ((SOLARISLOCAL*)stream->local)->buf);
      }
    } else {
      if (!stream->silent) {
	sprintf(((SOLARISLOCAL*)stream->local)->buf, 
		"102 Cannot lock mailbox");
	mm_log(((SOLARISLOCAL*)stream->local)->buf, ERROR, stream);
	/* ERROR */
	mm_log_stream(stream, ((SOLARISLOCAL*)stream->local)->buf);
      }
    }

    /*
     * In sunversion, allow the server connection to remain open,
     * but in state SELECT
     */
    solaris_unlock(fd, stream, lock);
    mail_unlock(stream);
    *perr = "solaris_lock failed";
    return -1;
  }

  /*
   * only if open time parse, cause the mailbox to be
   * mapped into memory to speed initial startup
   */
  if (openparse) {
      mmap_mbox (stream);
  }
  stat(stream->mailbox, &sbuf);		/* get status */

  /*
   * Check to see if the st_nlink count is 0.
   * This means the file has been unlinked possibly by mailtool.
   * In this case, we abort - leaving the file intact.
   */
  if (sbuf.st_nlink == 0) {	/* Yikes!  */
    if (!stream->silent) {
      sprintf(((SOLARISLOCAL*)stream->local)->buf,
	      "169 Mailbox modified by another process. "
	      "Cannot continue!");
      mm_notify(stream, ((SOLARISLOCAL*)stream->local)->buf, WARN);
      mm_log_stream(stream, ((SOLARISLOCAL*)stream->local)->buf);
      sprintf(((SOLARISLOCAL*)stream->local)->buf,
	      "Mailbox modified by another process. "
	      "Cannot continue!");
      mm_log(((SOLARISLOCAL*)stream->local)->buf, ERROR, stream);
    }
    solaris_unlock(fd, stream, lock);
    mail_unlock(stream);
    solaris_abort(stream, _B_TRUE);		/* remove index file too. */
    return -1;
  }

  /* 
   * Set our access for this parse:
   * 1. One can do anything if one has write access,ie, write permissions,
   *    and !stream->rdonly (another imap user sharing the mbox).
   */
  WR_access = !((SOLARISLOCAL*)stream->local)->read_access;

  /*
   * Set our group oermissions: Must work because solaris_lock fails
   * if group permissions cannot be set
   */
  solaris_mbox_access(stream,
		      ((SOLARISLOCAL*)stream->local)->name,
		      (WR_access ? W_OK : R_OK),
		      &err);
  
  /* Cannot keep mime translations IF read access only */
  if (!WR_access) {
    stream->keep_mime = NIL;
  }

  /*
   * the current state of the index file is fine.
   *  1. We've locked out any changes doing the initial open.
   *
   *  2. We can use as a model of even our read/only mail file,
   *  Unless you do not keep MIME translations. IN this case, we
   *  must always reparse to restranslate. C'est enmerdant!
   *
   *  WHAT must be always remembered is that since NOT KEEPING MIME
   *  requires a complete PARSE everytime a file is opened to translate
   *  V3 into MIME, that then indexing is impossible. Tant pis.
   *
   * If stream->rdonly (multiple processes accessing r/w), then we can
   * still read the index, but will not translate MIME later.
   * ALREADY translated messages are indexed.
   */

  if (((WR_access
	&& stream->keep_mime) 
       || ((SOLARISLOCAL*)stream->local)->read_access)
      && stream->nmsgs == 0) {

    /* read in our index file */
    if (read_index_hdr(stream, &ihdr) 
	&& valid_index_hdr(stream, &ihdr, &sbuf)) {

      /* preallocate and update our FILECACHE */
      mm_notify(stream, "Read index file",PARSE);

      if (set_filecache(stream, ihdr.nmsgs)) {

	/* ok, we now have our FILECACHE initialized */
	stream->nmsgs = ihdr.nmsgs;
	((SOLARISLOCAL*)stream->local)->filesize = ihdr.filesize;
	((SOLARISLOCAL*)stream->local)->ino = ihdr.ino;
	((SOLARISLOCAL*)stream->local)->filetime = ihdr.modify_time;
	((SOLARISLOCAL*)stream->local)->accesstime = ihdr.access_time;
	stream->uid_validity = ihdr.uid_validity;
	stream->uid_last = ihdr.uid_last;
	stream->checksum = ihdr.checksum;
	stream->mbox_size = ihdr.mbox_size;
	nmsgs_at_entry = stream->nmsgs;
	nmsgs = stream->nmsgs;
	((SOLARISLOCAL*)stream->local)->read_index = _B_TRUE;
      }
    } else {	
      /*
       * Remove OLD index files, and new ones 
       * if they are an out dated version.
       */
      remove_old_index_files(stream->mailbox, _B_TRUE);
      /* Close the FD */
      if (((SOLARISLOCAL*)stream->local)->id > -1) {
	close(((SOLARISLOCAL*)stream->local)->id);
	((SOLARISLOCAL*)stream->local)->id = -1;
      }
      unmap_index_file(stream);	/* No harm if it is not mapped */
    }
  }

  /*
   * If we've read the index file(first pass), then
   * assume the pseudo-message has been read
   */
  ((SOLARISLOCAL*)stream->local)->pseudo_dirty = NIL; 

  /* calculate change in size */
  delta = sbuf.st_size - ((SOLARISLOCAL*)stream->local)->filesize;

  /* If we have new data, and this is NOT the initial parse,
   * and WE DO NOT have write access, then we cannot parse
   * for NEW messages. But, if we have access and not permission
   * then:
   *    1. Right now we are using !WR_acess rather than !((SOLARISLOCAL*)stream->local)->read_access.
   *    2. With a little more work we can allow FULL APPEND with write permission,
   *    and stream->rdonly.
   *        Issues: We are rdonly BECAUSE there are at least TWO active users.
   *          We cannot update the index_file. Too difficult for other user.        
   *        We do update the hidden message since it was read on open, or
   *        It is now being read (stream->nmsgs > 0).
   */
  if (delta > 0 && ((SOLARISLOCAL*)stream->local)->read_access && stream->nmsgs > 0)
    delta = 0;
  if (delta < 0) {
    sprintf(((SOLARISLOCAL*)stream->local)->buf,
	    "117 Mailbox shrank from %d to %d bytes, aborted",
	    ((SOLARISLOCAL*)stream->local)->filesize,
	    sbuf.st_size);

    if (!stream->silent) {
      mm_notify(stream,((SOLARISLOCAL*)stream->local)->buf, WARN);
      /* this is pretty bad */
      mm_log(((SOLARISLOCAL*)stream->local)->buf, ERROR, stream);
      /* ERROR */
      mm_log_stream(stream, ((SOLARISLOCAL*)stream->local)->buf);
    }
    solaris_unlock(fd, stream, lock);
    mail_unlock(stream);
    *perr = "File shrank";
    solaris_clear_group_permissions(stream);

    /* abort the connection and remove the index file */
    solaris_abort(stream, _B_TRUE);
    return -1;

  } else if (delta == 0) {

    mail_exists(stream, nmsgs);	/* notify upper level of new mailbox size */
    mail_recent(stream, stream->recent);/* and of change in recent count */

    /* Clear excess cache size */
    for (i = nmsgs; i < ((SOLARISLOCAL*)stream->local)->cachesize; i++) {
      ((SOLARISLOCAL*)stream->local)->msgs[i] = NIL;
    }

    /*
     * If we are opening, and  no new messages, then 
     *   (1) Did not read index: do a FULL checksum 
     *   (2) READ INDEX:         do a FAST checksum
     */
    if (openparse) {
      if (((SOLARISLOCAL*)stream->local)->read_index) {
	stream->checksum_type = FASTCHECKSUM;
      } else {
	stream->checksum_type = OPENCHECKSUM;
      }
      stream->checksum_type |= STATUSCHECKSUM|PARSECALL;
      solaris_checksum(stream);
      ((SOLARISLOCAL*)stream->local)->fullck_done= _B_TRUE;
    }
    solaris_clear_group_permissions(stream);
    return fd;

  } else {		/* !! We have data to process !! */

    /* 
     * Solaris eom passed parameters as pointers 
     */
    long b_size= -1;		/* used by solaris_eom as a value  */
    int b_size_not_used= _B_TRUE;    /* True: b_size used in buffer allocation */
    int n_chrs= 0;		/* used by solaris_eom as a counter */
    int again;			/* used by solaris_eom as a boolean */
    int skip= NIL;		/* used by solaris_eom as a  boolean */
    unsigned long bpos;		/* saved position in buffer */
    EOM_t cas= EOM1;		/* case in eom routine */
    int ok_content_len= 0;	/* flags OK content lengths when found */
    int try_content_len= 0;	/* Says content-len there. Mal ou bien. */
    int un_de_plus= 0;		/* for a very bizarre mailtool bug */
    char *clen_eom;		/* end of message if content length used */
    long from_offset;		/* for reentering when "From .." was incomplete */
    int from_renter= 0;		/* flags reentering */
    unsigned long msg_size;     /* msg size if content length is valid */
    ssize_t nr;			/* number of bytes read */
    int last_msg= 0;            /* True if found last msg && content leng */
    /* End of passed parameters */
    unsigned long seek_base;	/* for computing seek offsets of msgs */
    int loc_msgno= 0;		/* counter of msgno if initial open */
    int caching_on;             /* cache rest. Force rewrite. */
    int nallocd = 0;		/* debugging tmp */
    lseek(fd, ((SOLARISLOCAL*)stream->local)->filesize, L_SET);
    seek_base = ((SOLARISLOCAL*)stream->local)->filesize; /* for seek offets */

    s = s1 = PARSE_BASE;	/* initial read-in location */
    i = 0;			/* initial unparsed read-in count */
    time0 = elapsed_ms();	/* for our "Server busy ..." messages */
    parset0 = time0;		/* time total parse */
				/* Cacle ALL new messages */
    if (stream->nmsgs > 0) {
      write_file = caching_on= _B_TRUE;
				/* Start with the first NEW msg# */
      min_rewrite_msgno = stream->nmsgs + 1;
    } else write_file = caching_on = NIL;
    do {			/* parse the mail file  */
      char *smsg;		/* end of message when found */
      long buf_size;
      long bytes_in_buf = s1 - s;
      char *nullptr;
      int nulls_found = 0;	/* True if nulls found in a buffer of data */
      i = min (CHUNK, delta);	/* calculate read-in size */
      /* Since b_size > 0 means we KNOW the body size,
       * we use it ONCE if it is > i:
       */
      if (b_size > 0 && b_size_not_used) {
				/* b_size may exceed delta */
	unsigned long clength = min(b_size,delta);
	i = (clength > i ? clength : i);
	b_size_not_used = NIL;
      }
				/* increase the read-in buffer if necessary */
      buf_size = i + bytes_in_buf;
      if (buf_size >= PARSE_SIZE) {
	unsigned long new_size;
	is = s - PARSE_BASE;    /* note former start of message position */
	is1 = s1 - PARSE_BASE;	/* and start of new data position */
	if (bytes_in_buf) 
	  solaris_resize (PARSE_DATA,buf_size);
	else {			/* fs_resize would do an unnecessary copy */
	  solaris_give(PARSE_DATA);
	  solaris_get(buf_size,PARSE_DATA);
	}
	s = PARSE_BASE + is;	/* new start of message */
	s1 = PARSE_BASE + is1;	/* new start of new data */
      }
      if ((nr = read (fd,s1,i)) < 0) {	/* read a chunk of new text */
	sprintf (((SOLARISLOCAL*)stream->local)->buf,"206 Read error(VM problem?): %s",
		 strerror (errno));
	report_and_avorter(stream,fd,lock);
	*perr = "Read error";
	solaris_clear_group_permissions(stream);
	return -1;
      }  else			/* nr <= i. Use nr just in case.  */
	i = nr;
      /* Possible NULLs:
       *    1)  NFS mounted files (even new mail) OR
       *    2)  During an openparse. 
       *    Since NULLs can cause serious problems we check
       */
      if ((openparse || solaris_nfs_mounted(fd)) && read_null_chars(s1,i,&nullptr)) {
	if (openparse) {	/* Replace with SPACES */
	  unsigned long ntocheck = i;
	  nulls_found= _B_TRUE;	/* Mark for rewrite */
	  ntocheck -= nullptr - s1;/* bytes left in buffer starting with null */
	  replace_nulls(nullptr,ntocheck);
#ifdef NO_ONE_CARES
	  sprintf(((SOLARISLOCAL*)stream->local)->buf,
		  "160 NULLS removed from %smailbox(PARSING)",
		  (char *)(solaris_nfs_mounted(fd) ? "NFS mounted " : " "));
	  *perr = "(NULLS removed from mailbox)";
	  if (!stream->silent) {
	    mm_notify(stream, ((SOLARISLOCAL*)stream->local)->buf, WARN);
	  }
	  mm_log_stream(stream,
			((SOLARISLOCAL*)stream->local)->buf); /* ERROR */
#endif
	} else {		/*  Abort new mail processing. Leave mailbox */
				/*  open. User can close it or read old mail */
	  long ii;		/* free new cache elements */
	  for (ii = stream->nmsgs; ii < nmsgs; ii++) {
	    FILECACHE *fc = ((SOLARISLOCAL*)stream->local)->msgs[ii];
	    /* If we have a mime translation, free it first */
	    if (fc->mime_tr) {
	      fs_give ((void **) &fc->mime_tr);
	    }
	    /* Now, free the cache itself */
	    solaris_free_fcache(stream,ii);
	  }
#ifdef NO_ONE_CARES
	  sprintf(((SOLARISLOCAL*)stream->local)->buf,
		  "159 New Mail: NULLS detected in %smailbox",
		  (char *)(solaris_nfs_mounted(fd) ? "NFS mounted " : " "));
	  *perr = "(NULLS detected in mailbox)";
	  mm_notify(stream, ((SOLARISLOCAL*)stream->local)->buf, WARN);
	  if (!stream->silent) {
	    mm_log("System error", ERROR, stream);
	    mm_notify(stream,((SOLARISLOCAL*)stream->local)->buf, WARN);
	  }
	  mm_log_stream(stream,((SOLARISLOCAL*)stream->local)->buf); /* ERROR */
#endif
	  solaris_clear_group_permissions(stream);
	  solaris_unlock (fd,stream,lock);
	  mail_unlock (stream);
	  return NULLS_IN_NEWMAIL;
	}
      }
      s1[i] = '\0';		/* tie off chunk */
      delta -= i;		/* account for data read in */
      if (c) {			/* validate newly-appended data */
				/* skip leading whitespace */
	while ((*s == '\n') || (*s == ' ') || (*s == '\t')) {
	  c = *s++;		/* yes, skip the damn thing */
	  s1++;
	  seek_base += 1;	/* Skip garbage! */
	  if (!--i) break;	/* only whitespace was appended?? */
	}
				/* see new data is valid */
	if (c == '\n') VALID (s, &t, &ti, &zn, &rv);
        if (!ti) {		/* invalid data? */
	  sprintf (((SOLARISLOCAL*)stream->local)->buf,
		   "116 Illegal Mailbox format detected(Try restarting)");
	  report_and_avorter(stream,fd,lock);
	  *perr = "Initial hidden From corrupted";
	  solaris_clear_group_permissions(stream);
	  return -1;
	}
	c = NIL;		/* don't need to do this again */
      }

      /* KEEP alive if doing a long read */
      if ((nmsgs % 101) == 0) {
	time1 = elapsed_ms();
	time_elapsed = time1 - time0;
	if (time_elapsed >= ACKTIME) {
	  sprintf(small,"131 busy(READ %d msgs)",nmsgs);
	  mm_log_stream(stream,"131 busy"); /* ERROR */
	  mm_notify (stream,small,PARSE);
	  time0 = elapsed_ms();     /* reset timer */
	}
      }
    

      /*
       * smsg == start of next message, delta == data left TO READ in file.
       *
       * smsg   delta  action
       * ----   -----  ------
       *
       * !NIL     > 0  Found start of next message
       * !NIL    == 0  Found start of next message
       *  NIL     > 0  Read more data to look for start of message
       *  NIL    == 0  end-of-file : Last message parsed
       */
      while ((smsg  = solaris_eom(s,
				  s1,
				  i, 
				  &b_size,
				  &n_chrs,
				  &again,
				  &skip,
				  &bpos,
				  &cas,
				  &ok_content_len, 
				  &try_content_len,
				  &un_de_plus,
				  delta,
				  &clen_eom,
				  &from_offset,
				  &from_renter,
				  &msg_size,
				  &last_msg)) 
	     || !delta) {
	char *chatte;
	int bad_content_len;	/* T if !e and b_size >= 0 */
	long seeker;
	long rml;
	char *m_eom;		/* Points to end-of-message */
	FILECACHE *n;
	char *pos1,*pos_eol;
	char c_at_pos1,c_at_pos_eol;
	unsigned long saved_rml;
	int rfc822_size_done = NIL;

	if ((nmsgs % 103) == 0) {
	  time1 = elapsed_ms();
	  time_elapsed = time1 - time0;
	  if (time_elapsed >= ACKTIME) {
	    sprintf(small,"131 busy(READ %d msgs)",nmsgs);
	    mm_log_stream(stream,"131 busy"); /* ERROR */
	    mm_notify (stream,small,PARSE);
	    time0 = elapsed_ms();     /* reset timer */
	  }
	}
    
	try_content_len = NIL;	/* for next msg */
	from_renter = NIL;	/* for next msg  */
	loc_msgno += 1;		/* counts msgno if initial open */
	b_size_not_used= _B_TRUE;    /* reset this parameter */
	/*
         * b_size >= 0 means we found a content length.
         * ok_content_len means it is OK. */
	if (!ok_content_len && b_size >= 0) { /* then, dans la merde! */
	  long msgno;
	  if (((SOLARISLOCAL*)stream->local)->filesize > 0)
	    msgno = loc_msgno + stream->nmsgs;
	  else 
	    msgno = loc_msgno - 1;
	  bad_content_len= _B_TRUE;
	  b_size = -1;		/* not used */
	} else
	  bad_content_len = NIL;

	skip = NIL;		/* Not skipping  */
	cas = EOM1;		/* The first case */
	n_chrs = 0;		/* clear for next message */

	nmsgs++;		/* yes, have a new message */
	if (nmsgs >= ((SOLARISLOCAL*)stream->local)->cachesize) {/* need to expand cache? */
	  /* Expand our internal c-client cache (uses CACHEINCREMENT) */
	  (*mc) (stream,nmsgs,CH_SIZE);
	  /* Expand our LOCAL file cache */
	  ((SOLARISLOCAL*)stream->local)->cachesize = nmsgs + CACHEINCREMENT;
	                        /* resize: already have a cache */
	  fs_resize ((void **)&((SOLARISLOCAL*)stream->local)->msgs,((SOLARISLOCAL*)stream->local)->cachesize*sizeof (FILECACHE *));
	}
	/*
	 * If our content length is correct, then the
         * length of our message does NOT depend on where
         * the next mesasge starts. */
	if (ok_content_len) {
	  rml = clen_eom - s;
	  /* If the body size is 0 AND last message in file
           * we need one extra byte.  */
	  if (WR_access && un_de_plus) {	/* fixes a mail tool bug. */
	    rml += 1;
	    caching_forced = caching_on= _B_TRUE;   /* cache rest. Force rewrite. */
	    if ( min_rewrite_msgno == 0) 
	      min_rewrite_msgno = nmsgs;
	  }
	  if (smsg) {		/* beginning of next message really */
	    seeker = (smsg - s); /* calculate message length */
	  } else 
	    seeker = strlen (s);/* otherwise is remainder of data */
	  /* Now some stuff for the new pseudo msg 0 */
	  if (nmsgs == 1 && stream->nmsgs == 0) {
	    /* May be the pseudo msg. We check its size. */ 
	    if (WR_access && not_pseudo_msg(s,rml)) {
	      must_rewrite_pseudo= _B_TRUE;
	      min_rewrite_msgno = 1;
	      caching_forced = caching_on= _B_TRUE;   /* gotta cache the truc entier! */
	    }
	    ((SOLARISLOCAL*)stream->local)->first_msg_len = rml;
	  }
	} else {
	  if (smsg) {
	    /* We assume that the message pointed to by e
             * was correctly appened with a blank line
             * between it and its predecessor. We NULL that
	     * blank line as our end-of-message marker */
	    rml = (smsg - s) - 1; /* calculate message length */
	    seeker = rml + 1;	/* to NEXT message */
	    if (WR_access && un_de_plus) {/* fix a mailtool bug... */
	      rml += 1;		/* don't ask :-). for 0 length msgs */
	      caching_forced = caching_on= _B_TRUE; /* cache rest. Force rewrite. */
	      if (min_rewrite_msgno == 0)
		min_rewrite_msgno = nmsgs;
	    }
	  } else {
	    /* Here we have the last message, and the length
	     * will be strlen(s) less one cuz when we write
             * this message we append an extra '\n' in the file,
	     * and need to leave room for it */
	    rml = strlen(s) - 1;   /* otherwise is remainder of data */
	    seeker = rml;
	  }
	  /* This takes care of case where there IS NO BODY */
	  if (rml < bpos) bpos = rml;
	  /* Now some stuff for the new pseudo msg 0 */
	  if (WR_access && nmsgs == 1 && stream->nmsgs == 0 &&
	      !((SOLARISLOCAL*)stream->local)->read_index) {/* Pseudo OK IF we read the index  */
	    /* OUR pseudo-msg MUST have a content length!
             * Something is broken.
             */
	    must_rewrite_pseudo= _B_TRUE;
	    min_rewrite_msgno = 1;
	    caching_forced = caching_on= _B_TRUE;
	    ((SOLARISLOCAL*)stream->local)->first_msg_len = PSEUDOLEN;
	  }
	}
	/* Save our FILECACHE */
	((SOLARISLOCAL*)stream->local)->msgs[nmsgs - 1] = (FILECACHE *)fs_get(sizeof(FILECACHE));
	n = ((SOLARISLOCAL*)stream->local)->msgs[nmsgs - 1];
	memset(n,0,sizeof(FILECACHE));	/* CLEAR the beast  */
	n->content_type = TYPENOTSET;
	/* Temporarily point our internal message cache
         * to the message in our parsing buffer. 
         * Do end-of-message stuff and save for restore. */
	n->internal = s;
	saved_rml = rml;
	if (s[rml-1] != '\n') {
	  c_at_pos1 = s[rml];
	  pos1 = &s[rml];
	  s[rml++] = '\n';
	} else pos1 = NIL;
	pos_eol = &s[rml];
	c_at_pos_eol = s[rml];
	s[rml] = '\0';
	n->eom = pos_eol;

	ok_content_len = NIL;	/* reset the flag */
	un_de_plus = 0;
	chatte = n->internal;	/* for debugging */
	/* Temporarily parse from the buffer, s */
	n->header = NIL;	/* Not set yet */
	n->headersize = 0;      /* le meme */
	n->content_length = b_size; /* content length if b_size >= 0 */
	/* Overlook this for now - see if we still do a good job of
         * parsing attachments when they are there. */
	n->broken_content_len = NIL; /* bad_content_len; -* keep this in cache too */
	init_mime_trans(&n->slem);  /* for sun attachments */
	b_size = -1;		/* reset flag for eom_search */
	/*
         * Save seek offset to this message. And calulate
         * the offset for the next */
	n->seek_offset = seek_base;
	seek_base +=seeker;	/* add e - s. */
	/* New values for soft caching */
	n->real_mem_len = saved_rml;
	/* Set body offset as found durning the parse */
	n->original_body_offset = n->body_offset = bpos;
	n->original_mem_len = rml;
	n->bodysize = rml - bpos;
	/* Mark nulls found IN BUFFER for later rewrite. 
	 * Can be any one of n mesasges in the buffer.
         * Only applies to openparse,
	 * otherwise we abort when they are found above.
         */
	if (nulls_found) {
	  n->nulls_found= _B_TRUE;
	  caching_forced = caching_on= _B_TRUE; /* cache rest. Force rewrite. */
	  if (min_rewrite_msgno == 0) min_rewrite_msgno = nmsgs;
	} else n->nulls_found = NIL;
	/*
         * WE now (of all things) examine the message to see
         * if we have a sun attachment. If so, we translate
         * the message to MIME (Ce n'est pas mon idee, mais s'ils
	 * veulent me payer pour cela ...)
         *
         * TOO ADD TO THIS FOLIE: We now may keep both a Sun Attachment
         * and its MIME translation, BUT NOT write back the MIME translation
         * to the mail file. In this case, n->mime_tr != NULL. And, this
         * is set in the sun_att_to_mime(..) call. We need to remember
         * to dispose of this when we close the mail box or expunge the msg.
         *   C'est bien la folie de Martin!  */
	n->mime_tr = NIL;
	n->msg_cached = NIL;
	if (sun_att_to_mime(&n,&rml,stream->keep_mime)) {
					/* FILECACHE: may or may not
					 * change[KEEPMIME TRUE changes n] */
	  ((SOLARISLOCAL*)stream->local)->msgs[nmsgs - 1] = n; 
	  /* rml is reset if stream->keep_mime, else pareil(the same). */
	  n->content_type = TYPENOTSET;
	  n->real_mem_len = rml;
	  chatte = n->internal;

	  /* If keeping mime, then we must rewrite our file. */
	  if (WR_access && stream->keep_mime) {
	    translations += 1;	/* count them */
	    n->msg_cached= _B_TRUE;
	    caching_forced = caching_on= _B_TRUE;
	    if (min_rewrite_msgno == 0) min_rewrite_msgno = nmsgs;
	  }
	  /* Now some stuff for the new pseudo msg 0 */
	  if (WR_access && nmsgs == 1 && stream->nmsgs == 0 &&
	      !((SOLARISLOCAL*)stream->local)->read_index) {	/* Psuedo OK if read index */
	    /* OUR pseudo-msg MUST be MIME! */
	    must_rewrite_pseudo= _B_TRUE;
	    min_rewrite_msgno = 1;
	    caching_forced = caching_on= _B_TRUE;
	  }
	  /* Do rfc822 size calc from translated data if we are keeping mime.
           * otherwise it is calculated from the "blob" n->mime_tr" */
	  solaris_rfc822_size(stream,n,n->internal,rml);
	  rfc822_size_done= _B_TRUE;
	}
	/* OK, restore s to original state */
	if (pos1) *pos1 = c_at_pos1;
	*pos_eol = c_at_pos_eol;
	/* If we are caching this beast, then mark it for the WRITE: */
	if (caching_on) {
	  if (!n->msg_cached) {
	    if (min_rewrite_msgno == 0) min_rewrite_msgno = nmsgs;
	  }
	} else {
	  /* Clear internal msg pointer if not cached */
	  n->internal = NULL;
	  n->body = NULL;
	}
	if (!rfc822_size_done) solaris_rfc822_size(stream,n,s,rml);
	n->eom = NULL;	/* no longer used */
	/* end of message checking */
	if (smsg && !last_msg) {/* saw end of message? */
	  i -= smsg - s1;	/* new unparsed data count */
	  s = s1 = smsg;	/* advance to new message */
	} else
	  break;		/* didn't find end-of-message */
      }                         /* while (e || !delta) end */
      /* e == NIL || delta == 0 */
      if (delta) {		/* end of messages not found? */
	s1 += i;		/* end of unparsed data */
	if (s != PARSE_BASE){	/* message doesn't begin at buffer? */
	  i = s1 - s;		/* length of message so far */
	  memmove (PARSE_BASE,s,i);
	  s = PARSE_BASE;	/* message now starts at buffer origin */
	  s1 = s + i;		/* calculate new end of unparsed data */
	}
      }
    } while (delta);		/* until nothing more new to read */
  }

  /* OK, run through all of the messages, parsing the relevant headers
   */
  time0 = elapsed_ms();
  /*
   * If NEW MAIL pass, then each message needs to be rewritten
   * back to the file with updated headers. We get to this point
   * iff
   *    (1) Initial parse of a mail file (no index file) ||
   *          In this case we cache iff the parsing detects something
   *          broken.
   *    (2) NEW MAIL (delta [was]> 0) && nmsgs_at_entry > 0 ||
   *          In this case caching was on during the initial parseing
   *          above.
   *    (3) We read the index file, && NEW MAIL: (delta > 0). 
   *          Here, caching forced in above parse.
   */
  for (i = stream->nmsgs; i < nmsgs; i++) {
    char *merde;
    char *dl;
    FILECACHE *m;
    char *vheader;		/* virtual header unless cached */
    unsigned long rfc822_len;
    int statlen = 0;
    int stest;
    int status,x_status,x_uid;
    int delta_status,delta_xstatus,delta_xuid;
    int r822sDX;
    /* transmettre: server busy messages if necessary */
    if ((i % 101) == 0) {
      time1 = elapsed_ms();
      time_elapsed = time1 - time0;
      if (time_elapsed >= ACKTIME) {
	sprintf(small,"131 busy(parsed %d HEADERS)",i-stream->nmsgs);
	mm_log_stream(stream,"131 busy"); /* ERROR */
	mm_notify (stream,small,PARSE);
	time0 = elapsed_ms();     /* reset timer */
      }
    }
    m = ((SOLARISLOCAL*)stream->local)->msgs[i];  	/* Get cache */
    /* Now decide if we need to read this message
     *   (1) read header if not cached */
    if (!m->msg_cached) {
      if (caching_forced) {
	/* Mark for caching during rewrite */
	if (min_rewrite_msgno == 0) min_rewrite_msgno = i+1;
      } 
      /* ALWAYS READ THE HEADER NOW */
      if (!solaris_readhdr(stream,fd,m,PARSE_BASE,lock)) {
	*perr = "solaris_readhdr";
	solaris_clear_group_permissions(stream);
	return -1;
      } else  m->internal = PARSE_BASE;
    }
    /*
     * This is a bugtrap for bogons in the new message cache, which may happen
     * if memory is corrupted.  Note that in the case of a totally empty
     * message, a newline is appended and counts adjusted.
     */
    merde = m->internal;
    ti = NIL;
    if (s = m->internal) VALID (s, &t, &ti, &zn, &rv);
    if (!ti) {
					/* should NEVER happen */
      sprintf(small,"115 Broken hidden \"From\" line msg#%d: Aborting(PARSE)",i);
      if (!stream->silent) {
	mm_log(small,ERROR, stream);
      }
      mm_log_stream(stream, small);	/* ERROR */
      solaris_unlock(fd, stream, lock);
      solaris_abort(stream, NIL);
      *perr = "Hidden From line corrupted";
      solaris_clear_group_permissions(stream);
      return(-1);
    }
    if (s = strchr (t++,'\n')) {
      vheader = ++s;
    } else {			/* probably totally empty message */
      /* !! Check this out. Should be impossible. !! */
      strcat (t-1,"\n");	/* append newline */
      m->real_mem_len++;	/* adjust count */
      vheader = s = strchr (t-1,'\n') + 1;
    }
    /* initial offset values. Keep pointers IFF cached. */
    m->hdr_offset = vheader - m->internal; /* For caching driver */
    if (m->msg_cached) m->header = vheader;
    else m->header = NULL;
    m->status_offset = 0;
    /* MSG len less internal header*/
    m->status_len = 0;		/* no length yet - used in checksumming */
    newcnt++;			/* count locally recent messages too. */
    (elt = mail_elt (stream,i+1))->recent= _B_TRUE;
    elt->valid= _B_TRUE;
    /* Need fix here for the WEEKLY valid -- illegal dates thing */
    if (rv) {  /* if already an rfc822 valid date */
      char date[64];

      date[2] = date[6] = date[20] = '-';
      date[11] = ' ';
      date[14] = date[17] = ':';
      
      date[7] = t[ti-5]; date[8] = t[ti-4];      /* yyyy */
      date[9] = t[ti-3]; date[10] = t[ti-2]; 
      date[3] = t[ti-9]; date[4] = t[ti-8]; date[5] = t[ti-7];  /* mmm */
      if (t[ti-12] == ',') date[0] = ' ';        /* dd */
      else date[0] = t[ti-12];
      date[1] = t[ti-11];

      date[12] = t[ti]; date[13] = t[ti + 1];    /* hh */
      date[15] = t[ti+3]; date[16] = t[ti+4];    /* mm */
      if (t[ti += 5] == ':') {                   /* ss if present */
	date[18] = t[++ti];
	date[19] = t[++ti];
	ti ++;
      }
      else date[18] = date[19] = '\0';

      t = zn ? (t + zn) : "LCL";                 /* time zone */
      date[21] = *t++; date[22] = *t++; date[23] = *t++;
      if ((date[21] != '+') && (date[21] != '-'))
	date[24] = '\0';
      else {			/* numeric time zone */
	date[24] = *t++; date[25] = *t++;
	date[26] = '\0'; date[20] = ' ';
      }
      
      if (!mail_parse_date (elt,date)) {
	mm_log ("163 Unparsable date",WARN, stream); 
	mm_log_stream(stream,"163 Unparsable date"); /* ERROR */
      }     
    }
    else if (ti != INVALID_822_DATE) {      
      char tia;
				/* generate plausable IMAPish date string */
      char date[64];
				/* generate plausable IMAPish date string */
      date[2] = date[6] = date[20] = '-';
      date[11] = ' ';
      date[14] = date[17] = ':';
				/* dd */
      if (t[ti -3] == ' ' && t[ti-4] != ' ') {  /* to take care of the case */
	date[0] = ' ';				/* where there is only 1 space */
	date[1] = t[ti - 2];			/* between the month and day */
	tia = ti+1;
      }
      else {
	date[0] = t[ti - 3]; date[1] = t[ti - 2];
	tia = ti;
      }
				/* mmm */
      date[3] = t[tia - 7]; date[4] = t[tia - 6];
      date[5] = t[tia - 5];
				/* hh */
      date[12] = t[ti]; date[13] = t[ti + 1];
				/* mm */
      date[15] = t[ti + 3]; date[16] = t[ti + 4];
      if (t[ti += 5] == ':') {	/* ss if present */
	date[18] = t[++ti];
	date[19] = t[++ti];
	ti++;			/* move to space */
      }
      else date[18] = date[19] = '0';
				/* yy -- advance over timezone if necessary */
      if (zn == ++ti) ti += (((t[zn] == '+') || (t[zn] == '-')) ? 6 : 4);
      date[7] = t[ti]; date[8] = t[ti + 1];
      date[9] = t[ti + 2]; date[10] = t[ti + 3];
				/* zzz */
      t = zn ? (t + zn) : "LCL";
      date[21] = *t++; date[22] = *t++; date[23] = *t++;
      if ((date[21] != '+') && (date[21] != '-'))
	date[24] = '\0';
      else {			/* numeric time zone */
	date[24] = *t++; date[25] = *t++;
	date[26] = '\0'; date[20] = ' ';
      }
				/* set internal date */
      if (!mail_parse_date (elt,date)) {
	mm_log ("163 Unparsable date",WARN,stream); 
	mm_log_stream(stream,"163 Unparsable date"); /* ERROR */
      }
    } else {
      /* Use a convenient birthday - Bon anniversaire Joan! */
      rfc822_date(small);
      mail_parse_date(elt,small);
    }

    dl = e = NIL;		/* no status stuff yet */
    status = x_status = x_uid =  NIL;
    delta_status = delta_xstatus = delta_xuid = 0;
    do {
      int poop = 0;		/* A la MRC. Status cruft. */
      int fake = 0;             /* fake message generated */
      statlen = 0;

      switch (*(t = s)) {	/* look at header lines */
      case '\n':		/* end of header */
	/* (1) IF we are at the first message (i == 0) and it
	 *     is NOT the pseudo message.
         *     This is subtle because !psuedo_parsed means
         *       a) There is no pseudo message
         *          i == 0 ==> mailbox WAS empty. We've never
         *          opened it. (NO psuedo message)
         *          WE NEED TO REWRITE THE ENTIRE MAILBOX.
         *       b) We read our index file.
         *          i == 0 ==> mailbox WAS empty. We've opened
         *          it, and all messages were removed. 
         *          ALL MESSAGES ARE NEW.
         *          WE NEED TO REWRITE THE ENTIRE MAILBOX.
         * (2) If we are at the first message and it is the pseudo message,
	 *     then 
         *     WE NEED TO REWRITE THE ENTIRE MAILBOX <==> !e 
         *     ie, someone message with the pseudo message.
	 * (3) IF the status has not been found
         *     WE NEED TO REWRITE THE ENTIRE MAILBOX STARTING WITH THIS MSG.
	 */
	if (WR_access) {
	  if ((i == 0 && !pseudo_parsed) || !e) {
	    if (min_rewrite_msgno == 0 ||
		  min_rewrite_msgno > i+1) min_rewrite_msgno = i+1;
	    caching_forced= _B_TRUE;
	  }
	  /* WE can rewrite the status headers. Take any changes
           * into account */
	  if (m->text_trans) {
	    unsigned long hdrlen = s - m->internal + 1;
	    /* NOTE: 
	     *  All body length adjustments for *out of place* STATUS
             *  information is accounted for because the message
             *  has been MOVED in place to account for them. The
             *  above header lenght is precise with the exception of
             *  STATUS info that has had SIZE adjustments */
	    m->body_offset += hdrlen;
	    m->bodysize = m->real_mem_len - hdrlen;
	  }
	} else  if (m->text_trans) {/* need to compute body offset */
	  /* NO write offset, so no adjustments can be taken into
	   * acount. We cannot write the headers back */
	  m->body_offset = s - m->internal + 1;
	  m->bodysize = m->real_mem_len - m->body_offset;
	}
	m->header_status = NULL;  /* use offsets */
	/* if body not set, and cached,
	 * then faites-le. This is true for MIME translations. */
	if (m->msg_cached && m->body == NULL)
	  m->body = s + 1;
	if (e) {		/* saw status poop? */
	  /* Offset to status lines */
	  m->status_offset = e - m->internal;
	  *e++ = '\n';		/* Step on status in our buffer */
	  m->headersize = e - m->internal;
	  m->rfc822_hdrsize = m->headersize - m->hdr_offset;
	  m->no_status = NIL;	/* C'est eomType! */
	} else {
	  /* NO status- It *will* be here  */
	  m->status_offset = m->body_offset - 1;
	  m->headersize = m->status_offset + 1;/* set header size */
	  m->rfc822_hdrsize = m->headersize - m->hdr_offset;
				/* Do NOT count the 2nd '\n'. Already counted */
	  if (WR_access) {
	    m->body_offset += STATUSTOTAL - 1;
	    m->real_mem_len += STATUSTOTAL;
	  }
	  if (stream->nmsgs > 0)	/* NEW message. */
	    m->no_status= _B_TRUE;	/* then, no status in the msg! */
	  /* NOTE: status will be written below, caching_forced == T */
	}
	s = NIL;		/* don't scan any further */
	break;
      case '\0':		/* end of message WITHOUT "\n\n"*/
	/* No body found supposedly. But just in case we have some
	 * "unnecessary checks ..." */
	/* (1) IF we are at the first message and it
	 *     is NOT the pseudo message.
	 * (2) IF the status has not been found.
	 *      Cache all */
	poop = 0;		/* For excess status cruft */
	fake = NIL;
	if (WR_access) {
	  if ((i == 0 && !pseudo_parsed) || !e) {
	    /* Weird test for a message that only is a "From .." line:
	     * Note: solaris_readmsg SETS m->header. So, this happens
             * only in the bizarre case where the char following the
             * FIRST line of the message TERMINATES the message. */
	    if (*m->header == '\0') {
	      s = create_null_msg(m, stream);
	      fake= _B_TRUE;
	    }
	    if (min_rewrite_msgno == 0 ||
		min_rewrite_msgno > i+1) min_rewrite_msgno = i+1;
	    if (!e) {
	      m->body_offset += STATUSTOTAL - 1; 
	      m->real_mem_len += STATUSTOTAL;
	      poop = STATUSTOTAL - 1;
	    }
	    caching_forced= _B_TRUE;
	  }
	  if (m->text_trans) {	
	    unsigned long hdrsize = s - m->internal + 1;
	    /* need to compute body offset + any changes in status lines */
	    m->body_offset += hdrsize;
	    /* body size is constant */
	    m->bodysize =  m->real_mem_len - hdrsize;
	  }
	} else if (m->text_trans) {
	  /* NO WR_access: need to compute body offset as is in the file. */
	  m->body_offset = s - m->internal + 1;
	  m->bodysize =  m->real_mem_len - m->body_offset;
	}
				/* as in the mail file(used for rewriting) */
	if (e) {		/* saw status poop? */
	  m->status_offset = e - m->internal;
	  *e++ = '\n';		/* patch in trailing newline */
	  m->headersize = e - m->internal;
	  m->rfc822_hdrsize = m->headersize - m->hdr_offset;
	  if (!m->body) 	/* no body found - Vomir!*/
	    m->no_status = NIL;	/* Note we have a status line */
	} else {
	  if (!m->body || !fake) {
	    /* calculate offset of header., Make sure ONE "\n" found. */
	    char *lstat,*lchar= --s;	/* char preceding null */
	    int incr = 0;
	    if (*lchar != '\n') {  *++lchar = '\n'; ++incr; }
	    else ++lchar;
	    lstat = lchar;
	    *lchar++ = '\n';    /* Now "\n\n" */
	    ++incr;
	    m->body_offset += incr;
	    m->body = lchar;
	    m->bodysize = 0;
	    m->headersize = lstat - m->internal;
	    m->rfc822_hdrsize = m->headersize - m->hdr_offset;
	    m->rfc822_size += incr;
	  } 
	  m->status_offset = m->body_offset - 1 - poop;
	  if (stream->nmsgs > 0) m->no_status= _B_TRUE;
	  m->header_status = NULL;/* use offsets */
	}
	
	s = NIL;		/* don't scan any further */
	break;
      case 'X':			/* possible X-???: line */
	if (s[1] == '-') {
	  dl = s;		/* backup pointer */
	  s += 2;
	}
	/* X-Dt-Delete-Time:  For DtMail compatibility. */
	if (strncasecmp("Dt-Delete-Time:", s, 15) == 0) {
	  elt->deleted= _B_TRUE;
	  break;
	}
	/* possible X-IMAP */
	if (strncasecmp("IMAP:", s, 5) == 0) {
	  unsigned long uidv;
	  dl = NIL;		/* no longer valid  */
	  if (!e) e = t;	/* note deletion point */
	  if (i == 0 && !stream->uid_validity) {
	    s += 5;		/* advance to data */
	    while (*s == ' ') s++;/* flush whitespace */
	    j = 0;		/* slurp UID validity */
	    while (isdigit (*s)) {/* found a digit? */
	      j *= 10;		/* yes, add it in */
	      j += *s++ - '0';
	    }
	    if (!j) break;	/* punt if invalid UID validity */
	    uidv = j;
	    while (*s == ' ') s++;/* flush whitespace */
	    if (isdigit (*s)) {	/* must have UID last too */
	      j = 0;		/* slurp UID last */
	      while (isdigit (*s)) {
		j *= 10;		/* yes, add it in */
		j += *s++ - '0';
	      }
	      /* flush remainder of line */
	      while (*s != '\n') s++;
	      stream->uid_last = j;
	      stream->uid_validity = uidv;
	      pseudo_parsed= _B_TRUE;/* pseudo-header seen */
	    }
	  } 
	  break;
	}
	/* possible X-UID */
	statlen = 2;		/* X-?  */
	if (strncasecmp("UID:", s, 4) == 0) {
	  x_uid= _B_TRUE;            /* x-uid: is present */
	  dl = NIL;		/* no longer valid */
	  if (!e) e = t;	/* note deletion point */
	  /* only believe if have a UID validity */
	  if (stream->uid_validity) {
	    long mmn;
	    statlen += 4;
	    s += 4;		/* advance to UID value */
	    while (*s == ' ') {
	      s++;		/* flush whitespace */
	      statlen += 1;
	    }
	    j = 0;
	    while (isdigit (*s)) {/* found a digit? */
	      statlen += 1;
	      j *= 10;		/* yes, add it in */
	      j += *s++ - '0';
	    }
	    /* flush remainer of line */
	    while (*s != '\n') {
	      statlen += 1;
	      s++;
	    }
	    /* MUST be exactly correct in length */
	    if (statlen != XUIDLEN) {
	      /* take any differences into account for write back */
	      delta_xuid = XUIDLEN - statlen;
	      if (WR_access) {
		if (min_rewrite_msgno == 0 ||
		    min_rewrite_msgno > i+1) min_rewrite_msgno = i+1;
		caching_forced= _B_TRUE;
		rewrite_status= _B_TRUE;
	      }
	    }
	    if (elt->uid)	/* make sure not duplicated */
	      sprintf (((SOLARISLOCAL*)stream->local)->buf,"Message already has UID %ld",elt->uid);
	    else if (j <= prevuid)/* make sure UID doesn't go backwards */
	      sprintf (((SOLARISLOCAL*)stream->local)->buf,"UID less than previous %ld",prevuid);
	    else if (j > stream->uid_last)
	      sprintf (((SOLARISLOCAL*)stream->local)->buf,"UID greater than last %ld",stream->uid_last);
	    else {		/* normal UID case */
	      prevuid = elt->uid = j;
	      break;		/* exit this cruft */
	    } 
	    if (!WR_access) break; /* Cannot update in any case!! */
#ifdef UIDDEBUG
	    sprintf (((SOLARISLOCAL*)stream->local)->buf+strlen(((SOLARISLOCAL*)stream->local)->buf),", UID = %ld, msg = %ld",j,i);
	    mm_log (((SOLARISLOCAL*)stream->local)->buf,WARN,stream);
#endif
	    nuids_changed += 1;   /* Note a uid change */
	    /* restart UID validity */
	    stream->uid_validity = time (0);
	    /* Set min. dirty message number */
	    if (stream->nmsgs > 0)/* i+1 is message number */
	      mmn = i + 1;
	    else
	      mmn = i;		/* i is message number */
	    /* START CACHING WITH THIS MESSAGE CCC */
	    if (min_rewrite_msgno == 0 ||
		min_rewrite_msgno > i+1) min_rewrite_msgno = i+1;
	    caching_forced= _B_TRUE;
	  } else {
	    /* X-UID without having seen the pseudo record:
             *  If i == 0, then this is either the pseudo (broken)
             *  OR no-pseudo message is there. So, it is already staged
             *  for rewritting: We set body increment correctly. 
             *  If pseudo, then it will be ignored.  */
	    while (*s != '\n') { s++; statlen += 1; }
	    /* MUST be exactly correct in length */
	    if (statlen != XUIDLEN) {
	      /* take any differences into account for write back */
	      delta_xuid = XUIDLEN - statlen;
	      if (WR_access) {
		if (min_rewrite_msgno == 0 ||
		    min_rewrite_msgno > i+1) min_rewrite_msgno = i+1;
		caching_forced= _B_TRUE;
		rewrite_status= _B_TRUE;
	      }
	    }
	  }
	  break;		/* done with UID handling */
	}
	/* otherwise fall into S case, then default case */
      case 'S':			/* possible Status: line */
	if (strncasecmp("Status:", s, 7) == 0) {
	  int do_status;
	  if (!e) e = t;	/* note deletion point */
	  dl = NIL;		/* Pas encore valide */
	  /* X-status/status both possible */
	  if (statlen == 2) {
	    stest = XSTATUSLEN;
	    x_status= _B_TRUE;        /* x-status: is present */
	    do_status = NIL;
	  } else {
	    stest = STATUSLEN;
	    status= _B_TRUE;          /* status: is present */
	    do_status= _B_TRUE;
	  }
	  statlen += 7;
	  s += 7;		/* advance to status flags */
	  do switch (*s++) {	/* parse flags */
	  case 'R':		/* message read */
	    elt->seen= _B_TRUE;
	    statlen += 1;
	    break;
	  case 'O':		/* message old */
	    if (elt->recent) {	/* don't do this more than once! */
	      elt->recent = NIL;/* not recent any longer... */
	      newcnt--;		/* non plus nouveau */
	    }
	    statlen += 1;
	    break;
	  case 'D':		/* message deleted */
	    elt->deleted= _B_TRUE;
	    statlen += 1;
	    break;
	  case 'F':		/* message flagged */
	    elt->flagged= _B_TRUE;
	    statlen += 1;
	    break;
	  case 'A':		/* message answered */
	    elt->answered= _B_TRUE;
	    statlen += 1;
	    break;
	  case 'T':		/* message is a draft */
	    elt->draft= _B_TRUE;
	    statlen += 1;
	    break;
	  default:		/* some other crap */
	    statlen += 1;
	    break;
	  } while (*s && *s != '\n');
	  /* recalculate Status/X-Status lines */
	  if (statlen != stest) {
	    /* correspondingly adjust the body offset when done */
	    if (do_status) 
	      delta_status = stest - statlen;
	    else delta_xstatus = stest - statlen;
	    if (WR_access) {
	      if (min_rewrite_msgno == 0 ||
		  min_rewrite_msgno > i+1) min_rewrite_msgno = i+1;
	      caching_forced= _B_TRUE;
	      rewrite_status= _B_TRUE;
	    }
	  }
	  break;
	}
	/* If we fell through from "X-" reset s */
	if (dl) { s = dl; dl = NIL; }

      default:			/* anything else is uninteresting */
	if (WR_access && e) {/* have status stuff to worry about? */
	  size_t len;
	  /* Found a NON status header line among
	   * our status lines. We remove our status line.
	   * Calulate the amount of data to move:
	   *   From s to end-of-message */
	  /* IF NOT CACHED, then cache this message, and its
	   * successors. Take care of mins. !!!
	   * THIS IS THE ONLY CASE WHERE WE MUST CACHE IN THE
	   * PARSE LOOP. WE MUST ADJUST THE ENTIRE MESSAGE. Ecoeurant!*/
	  if (!m->msg_cached) {
	    char *pmsg = m->internal;/* old start-of-message  */
	    if (!solaris_readmsg(stream,fd,m,lock,T)) {
	      *perr = "solaris_readmsg";
	      return -1;
	    } else {		/* recompute e and s */
	      unsigned long s_offset = s - pmsg;
	      unsigned long e_offset = e - pmsg;
	      e = m->internal + e_offset;
	      s = m->internal + s_offset;
	    }
	  }
	  len = m->internal + m->real_mem_len - s;
	  j = s - e;		/* yuck!!  calculate size of delete area */
	  m->real_mem_len -= j;	/* update for new size */
	  /* If text translations, then the FINAL body size calucations
           * for MOVED data are made at END OF HEADER case */
	  if (!m->text_trans)
	    m->body_offset -= j;/* body adjustments too. Avec plaisir (: */
	  /* It may be that the message is cached(mime translations),
           * BUT the body pointer is not yet set(didn't read it above)
	   * The above DELTA is then necessary, but we set the body
           * at the end of the header parse */
	  if (m->body)           /* make sure body is cached */
	    m->body = m->internal + m->body_offset;
	  /*
           * Annulez previous offset changes. The status area is GONE */
	  delta_status = delta_xstatus = delta_xuid = 0;
	  status = x_status = x_uid = NIL;
	  /* move tail of message to start old status */
	  memmove (e,s,len);
	  e[len] = '\0';	/* tie off the end of the data */
	  s = e;		/* BUG fix. Correct s. */
	  e = NIL;		/* no more delete area */
	  caching_forced= _B_TRUE;	/* force caching */
				/* if psuedo, rewrite it because
				 * pseudo_parsed is true iff we did
				 * NOT read our index file. New info */
	  if (i == 0 && pseudo_parsed) must_rewrite_pseudo= _B_TRUE;
	  if (min_rewrite_msgno == 0 ||
	      min_rewrite_msgno > i+1) min_rewrite_msgno = i+1;
	}
	break;
      }                         /* end switch */
    } while (s && (s = strchr (s,'\n')) && s++);
				/* Set precalculated rfc822 size */
    elt->rfc822_size = m->rfc822_size;
				/* assign a UID if no X-UID header found */
    /* i == 0      pseudo_parsed  This IS the pseudo message
     * i == 0      !pseudo_parsed This is a REAL message
     * i != 0                     This is a REAL message */ 
    if (WR_access)
      if (!elt->uid && (i || !pseudo_parsed)) {
	long mnn;
	prevuid = elt->uid = ++stream->uid_last;
	/* Update uid_last and this message */
	((SOLARISLOCAL*)stream->local)->pseudo_dirty = ((SOLARISLOCAL*)stream->local)->dirty= _B_TRUE;
	/* START CACHING WITH THIS MESSAGE CCC */
	if (min_rewrite_msgno == 0 ||
	    min_rewrite_msgno > i+1) min_rewrite_msgno = i+1;
	caching_forced= _B_TRUE;
      }
    if (i && m->no_status) { /* need some status */
      elt->recent= _B_TRUE;
    }
    /* OK, write the status goop to our local cache */
    if (i || !pseudo_parsed || rewrite_status) {
      solaris_update_status (m->status, elt, _B_TRUE);
      rewrite_status = NIL;
    }
    /* Only checksum if write permission AND r/w access */
    if (WR_access && !stream->rdonly) {
      if (i || (i == 0 && !pseudo_parsed)) {/* new mail only */
	/* i+1 is message no and i is a valid FILECACHE index since
	 * the pseudo merde has been taken care of */
	if (stream->server_open) {	/* cksum only if server opened */
	  ((SOLARISLOCAL*)stream->local)->max_pchunk = PARSE_SIZE;/* for pmap in checksuming */
	  solaris_checksum_msg(stream,i+1,fd,lock,NIL);
	  stream->send_checksum= _B_TRUE;/* need to send the checksum */
	}
	m->no_status = NIL;       /* The status is written below */
      }
    }
    /* See if we need to adjust the body offset 
     * because
     *   1. at least one but NOT ALL of the status group is missing.
     *   2. At least one but NOT all has been seen and incorrect size.
     * Ignored for pseudo message. 
     * NOTE: If !WR_access, then we can only change the rfc822_size.
     *       NO deltas can be applied. */
    r822sDX = 0;                  /* 822 size alterations */
    if (e && !(i == 0 && pseudo_parsed)) {
      int deltaX;
      int rdiff = 0;
      if (WR_access) {
	/*
	 * Take care of those of the 3 NOT seen. We
	 * need to rewrite back changes too. */
	if (!status || !x_status || !x_uid) {
	  int increment = 0;
	  /* += (value + 1) because terminating '\n' is not 
	   * included in defined length.
           * If the field is there, then it is counted in
           * our rfc822_size during initial parse. We remove
           * the length of the field in this case sent is is NOT
           * sent to the ui. */ 
	  if (!status) increment += STATUSLEN+1;
	  else			/* subtract from rfc822_size */
	    rdiff += STATUSLEN+2;
	  if (!x_status) increment += XSTATUSLEN+1; 
	  else rdiff += XSTATUSLEN+2;
	  if (!x_uid) increment += XUIDLEN+1;
	  else rdiff += XUIDLEN+2;
	  /* OK, the body will be offset by increment chars after
           * the write back. */
	  m->body_offset += increment;
	  /* Since NONE of these headers is ever sent, we must
	   * remove the contribution to rfc822_size that any
	   * make. Note += 2 above for the CR char */
	  r822sDX = -rdiff;
	  if (min_rewrite_msgno == 0 ||
	      min_rewrite_msgno > i+1) min_rewrite_msgno = i+1;
	  caching_forced= _B_TRUE;
	  rewrite_status= _B_TRUE;

	} else			/* We have all three  */
	  r822sDX = -(STATUSLEN + XSTATUSLEN +  XUIDLEN + 6);
	/* Take care of those SEEN. These have already been cached. */
	deltaX = delta_status + delta_xstatus + delta_xuid;
	m->body_offset += deltaX;
	/* We removed FULL header sizes from rfc822_size. So, we
	 * need to add back any chars that were NOT in the original
	 * status stuff found during the parse */
	r822sDX += deltaX;
      } else {			/* Read only - fix up rfc822.size */
	if (status) rdiff += STATUSLEN+2;
	if (x_status) rdiff += XSTATUSLEN+2;
	if (x_uid) rdiff += XUIDLEN+2;
	r822sDX = -rdiff;
	/* Also, consider differences in parsing. In readonly
	 * they are not part of the body_offset. */
	r822sDX += delta_status + delta_xstatus + delta_xuid;
      }
      /* Any rfc822_size changes */
      m->rfc822_size += r822sDX;
      elt->rfc822_size = m->rfc822_size;
    }
  }                             /* Le cul de "for ()" */
				/* NIL unused FILECACHE **pointers */
  while (i < ((SOLARISLOCAL*)stream->local)->cachesize) ((SOLARISLOCAL*)stream->local)->msgs[i++] = NIL;
				/* Warn if UIDs have been changed */
  if (nuids_changed != 0) {
    sprintf(((SOLARISLOCAL*)stream->local)->buf,"164 %d UIDS changed during parse",nuids_changed);
    if (!stream->silent) mm_log(((SOLARISLOCAL*)stream->local)->buf,WARN,stream);
    mm_log_stream(stream,((SOLARISLOCAL*)stream->local)->buf);	/* ERROR */
  }
  /* We flush the pseudo message iff we really read it. We
   * do not read it if we have read our index file.
   */
  if (pseudo_parsed) {		/* flush pseudo-message if present */
				/* decrement recent count */
    if (mail_elt (stream,1)->recent) --newcnt; /* recent to this call */
				/* flush local cache entry */
    solaris_free_fcache(stream,0);
    for (j = 1; j < nmsgs; j++) {
      /* Left shift the cache since the pseudo message is NOT
       * real */
      ((SOLARISLOCAL*)stream->local)->msgs[j - 1] = ((SOLARISLOCAL*)stream->local)->msgs[j];
    }
    ((SOLARISLOCAL*)stream->local)->msgs[nmsgs - 1] = NIL;
    /* psuedo was msg #1. It is gone now. 0, then none to rewrite:
     *   must_rewrite_pseudo    
     *        True                   min_rewrite_msgno == 1
     *        False                  min_rewrite_msgno == 0 ||
     *                               min_rewrite_msgno >= 2
     */
    if (min_rewrite_msgno > 0) min_rewrite_msgno -= 1;   
    j = stream->silent;		/* note curent silence state */
    stream->silent= _B_TRUE;		/* don't permit any noise */
    /* NOW nous debarasser de elt(1) */
    mail_exists(stream,nmsgs--);/* prevent confusion in mail_expunged() */
    mail_expunged (stream,1);	/* nuke the the elt. Allez aurevoir */
    stream->silent = j;		/* restore former silence state */
  } else if (WR_access && !stream->uid_validity) {
    /* We did not find the pseudo on first open */
				/* start a new UID validity */
    stream->uid_validity = time (0);
    ((SOLARISLOCAL*)stream->local)->pseudo_dirty= _B_TRUE;	/* make dirty to create pseudo-message */
    must_rewrite_pseudo= _B_TRUE;    /* Need to write EVERYTHING */
  }
  /*
   * We must rewrite messages iff write_file == T || must_rewrite_pseudo
   */
  if (WR_access) {
    /* If the pseudo header is broken, the force a write */
    if (must_rewrite_pseudo) ((SOLARISLOCAL*)stream->local)->pseudo_dirty= _B_TRUE;
    if (caching_forced) write_file= _B_TRUE;
    if (must_rewrite_pseudo ||	/* Then entire file rewrite */
	write_file) {	        /* rewrite from msg# it was forced/1st NEW */
      long index;
      int abort = NIL;
      if (must_rewrite_pseudo)	/* must rewrite the ENTIRE mailbox */
	index = 0;
      else {
	/*  CASES here: (((SOLARISLOCAL*)stream->local)->pseudo_dirty ==> caching_forced ==> 
	 *                         min_rewrite_msgno is valid)
         *   ((SOLARISLOCAL*)stream->local)->pseudo_dirty   write_file        index
         *      True                 True           min_rewrite - 1
         *      False                True           min_rewrite - 1
	 */
	index = min_rewrite_msgno - 1;
	if (index == -1) index = 0; /* Just in case of a BUG */
      }
      /*
       * A. NEW Messages:
       *      ((SOLARISLOCAL*)stream->local)->readindex   New messages after indexed max message.
       *      !openparse         Called for other reasons
       *                           ping, check, expunge, checksum
       *    We do the rewrite in place.
       * B. Parsing the ENTIRE file on open:
       *    (1) ((SOLARISLOCAL*)stream->local)->read_index ==> File NOT modified OR NEW MAIL
       *     NOTE: "small files" have no index, therefore
       *    (2) openparse ==> parsing on open
       *    Rewrite in a tmp file and rename for safety.
       *
       * */
      if (((SOLARISLOCAL*)stream->local)->read_index || !openparse) { /* NEW messages only. Just append */
	if (solaris_extend(stream,fd,"Unable to update mailbox",
			   sbuf.st_size)) {
	  /* Cache the new messages for the rewrite */
	  for (i = index; i < nmsgs; ++i) {
	    if (!((SOLARISLOCAL*)stream->local)->msgs[i]->msg_cached) /* could be kept mime trans. */
	      if (!solaris_readmsg(stream,fd,((SOLARISLOCAL*)stream->local)->msgs[i],lock,NIL)) {
		abort= _B_TRUE; break;
	      }
	  }
	  if (!abort) solaris_save_msgs(stream,fd,index,nmsgs,&sbuf);
	} else abort= _B_TRUE;
      } else {
	/* OK. Work to do(Clone of the expunge stuff I did 4 months ago. )
	 *   (1) Copy up to index in a tmp file
         *   (2) Read from index to end and rewrite into a tmp file
	 *   (3) Rename tmp to current mailbox.
	 * ALL THE TIME taking care of LOCKS */
	char tmpname[MAXPATHLEN];
	char rootname[MAXPATHLEN];	/* if logical link, then real file name */
	int soft = 0;			/* true if soft link */
	int tmp_fd = solaris_create_tmp(stream,fd,index,tmpname,rootname,&soft);
	if (tmp_fd < 0) abort= _B_TRUE;
	else {
	  int lock_fd,nolock;   
	  char imaplock[MAXPATHLEN];
	  char lock[MAILTMPLEN];
	  int success;

	  /* flock. rename maintains this lock */
	  solaris_flock(stream, tmp_fd, LOCK_EX);
	  /* Now, write from index0 into the tmp */
	  if (success = solaris_rewrite_msgs(stream,fd,tmp_fd,index,nmsgs,
					     &sbuf,lock)) {
	    char *dest;
					/* OK, have our tmp in place! Super! */
	    /* create session lock before rename frees the inode of
             * the current mailbox. Use inode of tmp file to create lockname. */
	    if (!sol_mksession_lock(stream,tmpname,imaplock,&lock_fd)) {
	      stream->rdonly= _B_TRUE;
	      nolock= _B_TRUE;
	    } else nolock = NIL;
	    /* Bye to old lockfile WHILE we still own the inode of
             * the old mailbox.  */
	    /* Get the destination name */
	    dest = (soft ? rootname : stream->mailbox);
				/* rename the tmp file */
	    if (success = (rename(tmpname,dest) == 0)) {
	      int y;
	      ((SOLARISLOCAL*)stream->local)->filesize = ((SOLARISLOCAL*)stream->local)->tmp_filesize;
	      ((SOLARISLOCAL*)stream->local)->filetime = sbuf.st_mtime;
	      ((SOLARISLOCAL*)stream->local)->dirty = NIL;/* stream no longer dirty */
	      ((SOLARISLOCAL*)stream->local)->fd = tmp_fd;/* Our new file descriptor */
	      close(fd);
	      fd = tmp_fd;	/*  For solaris_unlock */

	      /* update seek offsets */
	      for (y = index; y < nmsgs; y++) {
		FILECACHE *fc = ((SOLARISLOCAL*)stream->local)->msgs[y];
		fc->seek_offset = fc->tmp_seek_offset;
	      }
	    } 
	  }
	  abort = !success;
	}
      }
      if (abort) {
	/* 
	 * we abort on too many disk errors
	 */
	if (openparse) {
	  unmap_mbox(stream);
	}
	solaris_unlock(fd, stream, lock);
	solaris_abort(stream, NIL);
	mail_unlock(stream);
	*perr = "disk errors";
	solaris_clear_group_permissions(stream);
	return -1;
      }
    }
  }

  /*
   * toss large chunks for a pmapped parse buffer:
   * Save max chunk for checksumming:
   *   Initial parse only.
   */
  if (stream->nmsgs == 0) ((SOLARISLOCAL*)stream->local)->max_pchunk = PARSE_SIZE;
  if (PARSE_SIZE > CHUNK) {
    solaris_give(PARSE_DATA);
    solaris_get(CHUNK,PARSE_DATA);
  }
  /* Free all cached messages here. */
  for (j = nmsgs_at_entry; j < nmsgs; j++) {
    FILECACHE *m = ((SOLARISLOCAL*)stream->local)->msgs[j];
    if (m->msg_cached) {	
      /* This is for messages read in solaris_readmsg() above */
      solaris_give(INTERNAL_DATA(m));
      m->msg_cached = NIL;
    }
    m->internal = NIL;		/* set in solaris_save_msgs() */
  }
  if (parset0 != 0) {
    parset1 = elapsed_ms() - parset0;
    parset1 = parset1/1000;
    /* log total parse time */
    sprintf(((SOLARISLOCAL*)stream->local)->buf,"Parse time = %d secs",parset1);
    mm_notify (stream,((SOLARISLOCAL*)stream->local)->buf,PARSE);
  }
  /* update parsed file size and time */
  /* NOTE: sbuf is updated in solaris_save_msgs  */
  ((SOLARISLOCAL*)stream->local)->filesize = sbuf.st_size;
  ((SOLARISLOCAL*)stream->local)->ino = sbuf.st_ino;
  ((SOLARISLOCAL*)stream->local)->uid = sbuf.st_uid;
  ((SOLARISLOCAL*)stream->local)->gid = sbuf.st_gid;
  ((SOLARISLOCAL*)stream->local)->filetime = sbuf.st_mtime;
  ((SOLARISLOCAL*)stream->local)->accesstime = sbuf.st_atime;
  /* mail exists set stream->nmsgs = nmsgs */
  mail_exists (stream, nmsgs);	/* notify upper level of new mailbox size */
  mail_recent (stream, stream->recent+newcnt);	/* and of change in recent messages */
  /* create/update our index file if necessary:
   *    Must have r/w permission and r/w access */
  if (WR_access && !stream->rdonly) {
    /* if checksumming is required, we now do it here
     * to reflect the results in the index file. If we are
     * being called by the checksum routines via the server we do NOTHING */
    if (stream->sunvset && !(stream->checksum_type & SERVERCALL)) {
      if (openparse) {
	if (((SOLARISLOCAL*)stream->local)->read_index) stream->checksum_type = FASTCHECKSUM;
	else stream->checksum_type = OPENCHECKSUM;
	stream->checksum_type |= STATUSCHECKSUM|PARSECALL;
	solaris_checksum(stream);
	((SOLARISLOCAL*)stream->local)->fullck_done= _B_TRUE;
      } else if (newcnt > 0) {
	stream->checksum_type = FASTCHECKSUM|STATUSCHECKSUM|PARSECALL;
	solaris_checksum(stream);
      }
      stream->checksum_type = 0;
    }
    /* Do indexing only if INBOX or LARGER then min required size,
     * and you keep mime. In the latter case, one MUST always reparse
     * to get MIME translations, and thus the index file has no use.
     * C'est embetant, non!  */
    if (stream->keep_mime && 
	(((SOLARISLOCAL*)stream->local)->inbox ||  sbuf.st_size >=  MININDEXREQUIREDSIZE)) {
      if (((SOLARISLOCAL*)stream->local)->id < 0) {
	create_index(stream,&sbuf);
	/* remove any old files because of version changes */
	remove_old_index_files(stream->mailbox, _B_FALSE);
      }
      /* if index is there, then update it if something changed */
      if (((SOLARISLOCAL*)stream->local)->id >= 0) update_index(stream);
    }
  }
  solaris_clear_group_permissions(stream);
  return fd;			/* return the winnage */
}

/* Solaris search for end of message
 * Accepts: som: start of message
 *	    sod: start of new data
 *	    i:   size of new data
 * Returns: pointer to start of new message if one found
 */

#define Word unsigned long
/*
 * In solaris we begin always at start of message. This may
 * cause a rescan of a message if "Content-type: <n>" is not
 * found, and we have not reached the body, ie, "\n\n". This should
 * be extremely rare, and saves an incredible amount of complexity for
 * state maintenance.
 */
char *solaris_eom(char *som,
		  char *sod,
		  long i, 
		  long *body_size,
		  int *n_crs,
		  int *encore,
		  int *skipping,
		  unsigned long *body_pos,
		  EOM_t *eomType,
		  int *ok_content_len,
		  int *try_content_len,
		  int *un_de_plus, /* one more  */
		  long data_left,
		  char **clen_eom,
		  long *from_offset,
		  int *from_renter,
		  unsigned long  *msg_size,
		  int *last_msg)
{
  char *s = sod;		/* start of new message data */
  char *s1,*t;			/* used in VALID(...) */
  int ti,zn,rv;		        /* used in VALID(...) */
  union {
    unsigned long wd;
    char ch[9];
  } wdtest;
  long bytes_left;
  int use_content_len;
  long tries;

  switch (*eomType) {
  case EOM1:			/* looking for content-length */
    s = som;			/* Here always from the start of message */
    /*
     * We need to reset our counter for the body-break: "\n\n" */
    *n_crs = 0;
    while (*s) {
      char c;
      /*
       * Note: This will fail if s[i] == '\0', ie, we find end-of-data */
      if (strncasecmp("content-length:", s, 14) == 0) {
	/*
	 * Now skip to eol - We may have something like
	 * Content-Length: 12<NIL> in which case we need to
	 * exit and get the rest of the message in the buffer,
	 * and then try again ...
	 */
	char *asciiBodySize= &s[15];

	*n_crs = 0;		/* used in search for "\n\n" */
	while (c = *s++) {
	  if (c == '\n') break;
	}
	if (c == '\0')
	  return NIL;		/* END of data */
	/* OK, we have the full line, get the body size */
	*body_size = atol(asciiBodySize);
	*encore = NIL;		/* This is the first time */
	*skipping = NIL;	/* Not yet skipping */
	*try_content_len= _B_TRUE;	/* use if ok ... */
	*ok_content_len = NIL;	/* Not yet good */
	*eomType = EOM2;		/* next case */
	--s;			/* *s == '\n': May just precede the body */
	break;			/* break from outer while */
      } else {                  /* ELSE 1 */
	/* Look for end-of-line beginning with same c as above */
	while (c = *s++) {
	  if (c == '\n') 
	    break;		/* eol */
	}
	/* c and s are consecutive chars */
	/* 1:  (c == '\n'  && *s == '\0') || */
	/* 2:  (c == '\n'  && *s != '\0') || */
	/* 3:  (c == '\0') */
	if (!c || !*s) {
	  return NIL;		/* end-of-this-buf: Still case EOM1-rescan header */
	} else {		/* c == '\n' && *s != '\0': look for "\n\n" */
	  *n_crs = 0;		/* clear for next time. */
	  if (*s == '\n') {	/* && c == `\n` */
	    /* \n\n" - No content length present:
	     * Quit the outer while */
	    *eomType = EOM3;	/* Look for "\nFrom " */
	    s++;		/* First char in body */
	    *body_pos = s - som;
	    break;
	  }
	}
      }			       /* END- else 1: */
    }			       /* END-while(*s)...   */
  case EOM2:
    /* if we found the content-length, then look for "\n\n" header separator */
    if (*body_size >= 0) {      /* found the Content-length: N */
      unsigned long last_byte, eom_byte;
      char c;

      if (!*skipping) {		/* still looking for "\n\n" */
	if (*encore)		/* deja vu */
	  s = sod;		/* start with new data */
	while (c = *s++) {
	  if (c  == '\n')
	    *n_crs += 1;
	  else
	    *n_crs = 0;
	  if (*n_crs == 2)
	    break;
	}
	/* Either we have n_crs == 2 or *s == '\0' || c == '\0' */
	if (!c || *s == '\0') {
	  if (*n_crs == 2) {	/* c != '\0' */
	    *body_pos = s - som; /* offset from start-of-message */
	    if (data_left == 0 && *body_size == 0) {
	      *ok_content_len= _B_TRUE;
	      *clen_eom = s;	/* end-of-message the NULL */
	      return NIL;	/* found (empty) body at end-of-file */
	    }
	    *skipping= _B_TRUE;
	  } else
	    *encore= _B_TRUE;
	  if (data_left == 0)	/* then, can't use content length. */
	    *ok_content_len = NIL;
	  return NIL;
	}
      }				/* END-if */
      /* skip the body - should point to end of buffer of just
       * at the "\n\nFrom ..." IF the data is in the buffer */
      if (*skipping)
	s = som + *body_pos;	/* saved body position (just after "\n\n") */
      /* *s --> first message in the body */
      last_byte = (unsigned long)(sod + i);
      eom_byte = (unsigned long)(s + *body_size);
      if (eom_byte < last_byte) {
	*body_pos = s - som;    /* Start of body: save for later ... */
	s += *body_size;	/* Ayez foi (have faith the file ain't) */
				/* broken (foutu!) */
	*msg_size = eom_byte - (unsigned long)som; /* message size in bytes */
	*ok_content_len= _B_TRUE;    /* Could be if things end well below */
      } else {			/* Need more data to skip to end-of-message */
	*skipping= _B_TRUE;
	*body_pos = s - som;	/* offset from start-of-messge */
	if (data_left == 0)	/* No more data to read */
	  *ok_content_len = NIL; /* NOT ok dude, ie, If we found it. */
	return NIL;
      }
    }
    /* if we didn't find the content-length field, then s points
     * to the first \n before the body, ie, *body_size < 0 */
    *eomType = EOM3;
  case EOM3:
    /*
     * Look for "\nFrom ":
     * (Minor change - Allow "From .." to be 
     *  immediatly after the end of a message when
     *  the content length is given)
     * If we have a content length, then we should be pointing
     * at "\n\n...\nFrom ..." or the content length is bad.
     * It may be that we only find "\n\n ..." which is OK.
     * So we scan looking for chars not equal to this, or
     * the string "\nFrom". If we find bad chars, then we flag
     * the content length as bad. C'est bien moche!
     */
    if (*ok_content_len) {
      char *text;		/* current position in buffer */
      char c;
      int m_crs= 0;

      /* Are we reentering - needed more data? */
      if (*from_renter) 
	s = text = som + *from_offset;
      else
	text = s;

      while (c = *s) {
	if (c == '\n') {
	  ++s;
	  m_crs += 1;
	  continue;
	} else {		/* The while is finished. */
	  if (s[0] != 'F')	/* Must have 'F' */
	    *ok_content_len = NIL;
	  else {		/* Have 'F' */
	    /* In the fou situation where the content-length == 0,
	     * then one can have "\n\nFrom ..." where the "\n\n"
	     * terminates the 0 length message. Rare but can happen.
             * Mailtool causes this insanity by not appending a
             * blank line to new messages of content length 0 !! 
	     */
	    if (m_crs == 0) {		/* none found before "F"  */
	      if (*body_size == 0) { 
		*ok_content_len = NIL;	/* pas du tout! */
		*un_de_plus = 1;	/* helps the caller */
	      }				/* have "\nF" or "F" */
	    }
	    if (*ok_content_len) {
	      char f, *foutu = s;
	      int newline = NIL;
	      /* Make sure we have an entire line in the buffer */
	      while (f = *foutu++) {
		if (f == '\n') {
		  newline = _B_TRUE;
		  break;
		}
	      }
	      /* if no newline then get more chars */
	      if (!newline) {
	        /* Cannot do a valid check. Not enough info.
                 * Save offset from beginnig of buffer of text,
	         * and reenter */
		*from_offset = text - som;
		*from_renter= _B_TRUE;
		/* Make sure this is REAL! Could be out of data */
		if (data_left == 0)
		  *clen_eom = som + *msg_size;
		return NIL;
	      }
	      /* s = "F...." */
	      VALID(s, &t, &ti, &zn, &rv);
	      if (ti) {   /* validate "Le truc" */ 
		/* The true end-of-message if content length is correct, ie,
		 * We found the "From ...." where expected. C'est notre
		 * meilleur coup! */
		*clen_eom = som + *msg_size;
		return s;
	      } else
		*ok_content_len = NIL;
	    }
	  }
	  break;
	}                       
      }				/* done "while (c = *s)" */
      /*
       * if data_left > 0), we haven't finished reading the mail file,
       * and we didn't find our "\nFrom ..." yet, try again.
       * else just return NIL cuz we are done (: */
      if (c == '\0') {                  /* End-of-buffer */
	if (data_left > 0) {		/* Only found 1 or more \n's */
	  *from_offset = text - som;	/* start in same place again  */
	  *from_renter= _B_TRUE;
	} else {
	  /* Next msg's start not found - end-of-file */
	  *clen_eom = som + *msg_size;
	  *last_msg= _B_TRUE;
	}
	return NIL;
      } else {
	/* restore to after "\n\n" for searching form "\nFrom ..." since we */
	/* didn't find it. The content length is BAD */
	s = som + *body_pos;	
      }
    }   /* end if (*ok_content_len) */
    *ok_content_len = NIL;	/* If we found it, then is is bad. */
    while ((s > som) && *s-- != '\n');
    tries = (sod + i) - s;	/* data left in buffer  */
    if (tries > 50) {		/* don't do fast search if very few bytes */
				/* constant for word testing */
      strcpy (wdtest.ch,"AAAA1234");
      if(wdtest.wd == 0x41414141){/* not a 32-bit word machine? */
	register Word m = 0x0a0a0a0a;
				/* any characters before word boundary? */
	while ((long) s & 3) if (*s++ == '\n') {
	  VALID (s, &t, &ti, &zn, &rv);
	  if (ti) return s;
	}
	i = (sod + i) - s;	/* (total number of tries) x 4  */
	do {			/* fast search for newline */
	  if ((0x80808080 & (0x01010101 + (0x7f7f7f7f & ~(m ^ *(Word *) s)))) &&
				/* find rightmost newline in word */
	      ((*(s1 = s + 3) == '\n') || (*--s1 == '\n') ||
	       (*--s1 == '\n') || (*--s1 == '\n'))) {
	    s1++;		/* skip past newline */
	    VALID (s1, &t, &ti, &zn, &rv);	/* see if valid From line */
	    if (ti) 
	      return s1;
	  }
	  s += 4;		/* try next word */
	  i -= 4;		/* count a word checked */
	} while (i > 24);	/* continue until end of plausible string */
      }
    } else {
      /* Less than 50 bytes. Just look for VALID start of message */
      while (tries-- > 24) if (*s++ == '\n') {
	VALID (s, &t, &ti, &zn, &rv);	/* if found start of message... */
	if (ti) return s;	/* return that pointer */
      }
    }
    return NIL;
  }                             /* end-switch */
}


/* Solaris copy messages
 * Accepts: MAIL stream
 *	    mailbox name
 * Returns: T if copy successful else NIL
 */
long solaris_write_message_new_status (int fd, int *e, FILECACHE *m,
				       MESSAGECACHE *elt);
extern long elapsed_ms();
int solaris_copy_messages (MAILSTREAM *stream,char *mailbox)
{
  char file[MAILTMPLEN];
  char lock[MAILTMPLEN],lock1[MAILTMPLEN];
  struct utimbuf tp;
  int fd;
  int err;
  struct stat sbuf;
  long i;
  int ok= _B_TRUE;
  int src_locked = NIL;
  long time0, time1, time_elapsed;
  int foutu; 
  MESSAGECACHE *elt;
  /* initialize the timer */
  time0 = elapsed_ms();
				/* check access */
  if (solaris_path_access (stream,dummy_file(file,mailbox,stream),W_OK,&err)) {
    if (errno == ENOENT) {
      solaris_invalid_error(stream,mailbox,"copy",NIL);
    } else {
      mm_log ("158 Access denied!",WARN,NIL);
      mm_log ("Access denied!", ERROR,stream);
    }
    return NIL;
  } else {
    solaris_clear_group_permissions(stream);
  }
  
  /* make sure valid mailbox */
  if (!solaris_isvalid(mailbox, file, stream, _B_FALSE)) {
    /* Print message. Returns T one can continue */
    solaris_invalid_error(stream, mailbox, "copy", NIL);
    return NIL;
  }
  if ((fd = solaris_nowait_lock (stream,dummy_file (file,mailbox,stream), 
				/* dst locking */
				 O_WRONLY | O_APPEND | O_CREAT,
				 S_IREAD | S_IWRITE, lock, LOCK_EX)) < 0) {
    sprintf (((SOLARISLOCAL*)stream->local)->buf,"158 Access denied on copy, %s",strerror (errno));
    mm_log(((SOLARISLOCAL*)stream->local)->buf,WARN,NIL);
    mm_log ("Access denied!",ERROR,stream);
    return NIL;
  } else if (strcmp(((SOLARISLOCAL*)stream->local)->name,file) != 0) { /* Only lock src if dst != src */
    if (solaris_lock (stream,((SOLARISLOCAL*)stream->local)->name,O_RDONLY,NIL,lock1,LOCK_EX,NIL) < 0) {
      sprintf (((SOLARISLOCAL*)stream->local)->buf,"158 Access denied on copy: %s",strerror (errno));
      mm_log (((SOLARISLOCAL*)stream->local)->buf,WARN,NIL);
      mm_log ("Access denied!",ERROR,stream);
      solaris_unlock(fd,NIL,lock); /* unlock */
      return NIL;
    } else src_locked= _B_TRUE;
  }
  mm_critical (stream);		/* go critical */
  stat(stream->mailbox, &sbuf);		/* get current file size */
				/* write all requested messages to mailbox */
  foutu = NIL;			/* pas encore. Not yet broken. */
  for (i = 1; ok && i <= stream->nmsgs; i++) {
                                /* acknowledge client when 5000 ms passed */
    if ((i % 103) == 0) {
      time1 = elapsed_ms();
      time_elapsed = time1 - time0;
      if (time_elapsed >= ACKTIME) {
	char small[SMALLBUFLEN];
	sprintf(small,"131 busy(COPIED %d msgs)",i);
	mm_log_stream(stream,"131 busy"); /* ERROR */
	mm_notify (stream,small,PARSE);
	time0 = elapsed_ms();     /* reset timer */
      }
    }
				/* set up iov's if message selected */
    elt = mail_elt (stream,i);
    if (elt->sequence) {
      FILECACHE *m = ((SOLARISLOCAL*)stream->local)->msgs[i - 1];
      if (!m->msg_cached) {
	if (!solaris_buffer_msg(stream,((SOLARISLOCAL*)stream->local)->fd,m,lock1,NIL)) {
	  ok = -1;
	  foutu= _B_TRUE;
	} else 
	  m->msg_buffered= _B_TRUE;
      }
      if (!foutu) {
	/* Write msg with status without X-UID */
	solaris_write_message_new_status (fd, &ok, m, elt);
	m->msg_buffered = NIL;
      }
    }
    if (ok < 0) {
      sprintf (((SOLARISLOCAL*)stream->local)->buf,"121 Message copy failed: %s",strerror (errno));
      mm_log (((SOLARISLOCAL*)stream->local)->buf,ERROR, stream);
      ftruncate (fd,sbuf.st_size);
      break;
    } else
      ok= _B_TRUE;
  }
  /* free up our volatile pmapped cache */
  solaris_give(PARSE_DATA);
  solaris_get(CHUNK,PARSE_DATA);
  ((SOLARISLOCAL*)stream->local)->max_pchunk = CHUNK;

  tp.actime = sbuf.st_atime;	/* preserve atime */
  tp.modtime = time (0);	/* set mtime to now */
  utime (file, &tp);		/* set the times */
  fsync(fd);		/* force out the update */
  if (!foutu) solaris_unlock (fd,NIL,lock); /* unlock */
  close(fd);                    /* close the destination */
  if (src_locked) solaris_unlock (((SOLARISLOCAL*)stream->local)->fd,NIL,lock1); /* unlock source */
  mm_nocritical (stream);	/* release critical */
  return ok;			/* return whether or not succeeded */
 }

/* Solaris write message to mailbox
 * Accepts: I/O vector
 *	    I/O vector index
 *	    local cache for this message
 *
 * This routine is the reason why the local cache has a copy of the status.
 * We can be called to dump out the mailbox as part of a stream recycle, since
 * we don't write out the mailbox when flags change and hence an update may be
 * needed.  However, at this point the elt has already become history, so we
 * can't use any information other than what is local to us.
 */

long solaris_write_message (int fd, int *e, FILECACHE *m)
{
  long nwritten;
  int stat_len;
  struct iovec iov[16];
  int i = 0;

  /* !!! BUG HALT for testing !!! */
  if (!m->msg_cached && !m->msg_buffered) {
    fatal ("WRITE of NON-CACHED MSG",NIL);
  }
  
  iov[i].iov_base = m->internal;/* pointer/counter to headers */
				/* length of internal + message headers */
  iov[i].iov_len = m->headersize;
  nwritten = iov[i].iov_len;	/* accumulate the length */
				/* suppress extra newline if present */
  if ((iov[i].iov_base)[iov[i].iov_len - 2] == '\n') {
    iov[i++].iov_len--;
    nwritten -= 1;
  } else i++;			/* unlikely but... */

  iov[i].iov_base = m->status;	/* pointer/counter to status */
  stat_len = strlen (m->status);
  iov[i++].iov_len = stat_len;
  nwritten += stat_len;

  if (m->bodysize) {
    iov[i].iov_base = m->body;	/* pointer/counter to text body */
    iov[i++].iov_len = m->bodysize;
    nwritten += m->bodysize;
  }
  iov[i].iov_base = "\n";	/* pointer/counter to extra newline */
  iov[i++].iov_len = 1;
  nwritten += 1;
  *e = safe_writev (fd, iov, i);
  return nwritten;
}
/*
 * DO NOT append an X-UID. Simpifies parsing in case "hidden message"
 * is NOT there
 */
long
solaris_write_message_new_status(int 		  fd,
				 int 	    	* errorOrCount,
				  FILECACHE	* msg,
				  MESSAGECACHE 	* elt)
{
  long		nwritten;
  struct iovec	iov[16];
  int 		iovOffset = 0;
  char 		status[D_INDEXSTATUSTOTAL+1];

  /* !!! BUG HALT for testing !!! */
  if (!msg->msg_cached && !msg->msg_buffered) {
    fatal("WRITE of NON-CACHED MSG(copy msg)", NIL);
  }

  iov[iovOffset].iov_base = msg->internal;/* pointer/counter to headers */

  /* length of internal + message headers */
  iov[iovOffset].iov_len = msg->headersize;
  nwritten = iov[iovOffset].iov_len;	/* accumulate the length */

  /* suppress extra newline if present */
  if ((iov[iovOffset].iov_base)[iov[iovOffset].iov_len - 2] == '\n') {
    iov[iovOffset++].iov_len--;
    nwritten -= 1;
  } else {
    iovOffset++;			/* unlikely but... */
  }

  /*
   * Set up status:
   */
  solaris_update_status(status, elt, _B_FALSE);

  iov[iovOffset].iov_base = status;	/* pointer/counter to empty status */
  iov[iovOffset].iov_len = strlen(status);
  nwritten += iov[iovOffset++].iov_len;

  if (msg->bodysize) {
    iov[iovOffset].iov_base = msg->body;/* pointer/counter to text body */
    iov[iovOffset++].iov_len = msg->bodysize;
    nwritten += msg->bodysize;
  }

  iov[iovOffset].iov_base = "\n";	/* pointer/counter to extra newline */
  iov[iovOffset++].iov_len = 1;
  nwritten += 1;

  *errorOrCount = safe_writev(fd, iov, iovOffset);

  return nwritten;
}

/*
 * solaris write pseudo msg
 *    saves FIXED length pseudo message.
 */
int
solaris_write_pseudo_msg(MAILSTREAM * stream, int fd, int * pseudo_size)
{
  int 		e;
  int		retry = 5;

  solaris_pseudo(stream, ((SOLARISLOCAL*)stream->local)->buf);

  /* append "\n" message separator */
  ((SOLARISLOCAL*)stream->local)->buf[((SOLARISLOCAL*)stream->local)->pseudo_size] = '\n';

  /* for the extra '\n' */
  *pseudo_size = ((SOLARISLOCAL*)stream->local)->pseudo_size + 1;

  while (retry--) {
    lseek(fd, (off_t)0, L_SET);
    if ((e = safe_write(fd, ((SOLARISLOCAL*)stream->local)->buf,
		   ((SOLARISLOCAL*)stream->local)->pseudo_size + 1) < 0)) {
      sprintf(((SOLARISLOCAL*)stream->local)->buf,
	      "Mailbox pseudo-header write error: %s",
	      strerror (e = errno));
      mm_log(((SOLARISLOCAL*)stream->local)->buf, WARN, stream);
      if (retry > 1) {			/* then try again */
	mm_diskerror(stream, e, _B_TRUE);
      } else {				/* 5 tries. No luck. Abort?  */
	mm_diskerror(stream, e, _B_FALSE);
	return(_B_FALSE);
      }
    } else {
      break;
    }
  }
  return(_B_TRUE);
}

/*
 * solaris save messages
 *          saves possibly only the last part of a mail
 *          file, rather than the entire thing.
 *
 * Accepts: MAIL stream
 *	    mailbox file descriptor
 *          index of filecache of first message to write [msgno - 1]
 *          total number of messages
 *          stat buf to update
 *
 * Mailbox must be readwrite and locked for exclusive access.
 * Do all reads in ONE buffer
 */
static void 
solaris_save_msgs(MAILSTREAM *	stream,
		  int 		fd,
		  long 		index0,
		  long 		nmsgs, 
		  struct stat *	sbuf)
{
  long 		i;
  int 		e;
  int 		retry;
  unsigned long seek_base;
  unsigned long	init_seek_base;
  long		time0;
  long		time1;
  long		time_elapsed;
  int		pseudo_size;

  /* If pseudo is dirty, then write it */
  if (((SOLARISLOCAL*)stream->local)->pseudo_dirty) {
    if (!solaris_write_pseudo_msg(stream, fd, &pseudo_size))
      /* write failed - disk error */
      return;
    ((SOLARISLOCAL*)stream->local)->pseudo_dirty = NIL;	/* clean again */
  } else {
    pseudo_size = ((SOLARISLOCAL*)stream->local)->pseudo_size + 1;
  }

  /* May only have pseudo msg, or rewritting from first message */
  if (nmsgs == 0 || index0 == 0) {
    seek_base = pseudo_size;
  } else {				/* to first message */
    seek_base = ((SOLARISLOCAL*)stream->local)->msgs[index0]->seek_offset;
  }

  /* initialize the timer */
  time0 = elapsed_ms();
  ((SOLARISLOCAL*)stream->local)->filesize = seek_base;  /* upto the first message we write */

  init_seek_base = seek_base;   /* in case of errors */
  do {				/* restart point if failure */
    ssize_t	nwritten;

    retry = 0;		/* no need to retry yet */
    /* seek the first changed message/BOF */
    lseek(fd, seek_base, L_SET);

    /* loop through all NEW messages */
    for (i = index0; i < nmsgs; i++) {
      FILECACHE *	fc = ((SOLARISLOCAL*)stream->local)->msgs[i];

      /* Give back server busy ticks ... */
      if ((i % 101) == 0) {
	time1 = elapsed_ms();
	time_elapsed = time1 - time0;
	if (time_elapsed >= ACKTIME) {
	  char small[SMALLBUFLEN];
	  sprintf(small,"130 busy(SAVED %d msgs)",i-index0);
	  mm_log_stream(stream,"131 busy"); /* ERROR */
	  mm_notify (stream,small, PARSE);
	  time0 = elapsed_ms();     /* reset timer */
	}
      }

      /* The seek offset of this message */
      fc->seek_offset = seek_base;
      /* set up/write iov for this message */
      nwritten =  solaris_write_message(fd, &e, fc);
      seek_base += nwritten;
      if (e < 0) {
	sprintf(((SOLARISLOCAL*)stream->local)->buf,
		"122 Disk error rewriting mailbox: err  %s, nmsgs %d, index0 %d",
		strerror (e = errno), nmsgs, index0);
	mm_log(((SOLARISLOCAL*)stream->local)->buf,WARN,stream);
	mm_diskerror(stream, e, T);
	retry= _B_TRUE;		/* must retry from msg0 */
	seek_base = init_seek_base;
	break;		/* abort this particular try */
      } else {			/* won */
	/* less the '\n' msg separator */
	fc->real_mem_len = nwritten - 1;
	((SOLARISLOCAL*)stream->local)->filesize += e;	/* count these bytes in data */
      }
    }
  } while (retry);		/* repeat if need to try again */

  fsync(fd);		/* sync data back to disk */
  ftruncate(fd, ((SOLARISLOCAL*)stream->local)->filesize);
  stat(stream->mailbox, sbuf);		/* now get updated file time */
  ((SOLARISLOCAL*)stream->local)->filetime = sbuf->st_mtime;
  ((SOLARISLOCAL*)stream->local)->accesstime = sbuf->st_atime;
  ((SOLARISLOCAL*)stream->local)->dirty = NIL;		/* stream no longer dirty */
  mm_nocritical(stream);	/* exit critical */
}

/*
 * Resolve relative paths.
 *  relp  path
 *  dir   absolute parallel directory
 *  relative Return T if relative
 */
static char *resolve_dir(char *relp,char *dir,int *relative)
{
  char *cp = relp;
  /* flush "./" */
  if (*relp == '.' && *(relp+1) == '/') {
    *relative= _B_TRUE;
    cp = ++relp;
    while (*cp == '/') ++cp;		/* ".////" is possible */
    return(resolve_dir(cp,dir,relative));
  } else if (*relp == '.' && *(relp+1) == '.') { /* check "../" */
    char *dp = relp + 2;		/* skip ".." */
    if (*dp = '/') {			/* found "../" */
      /* try backing up dir one level:
       * Initially dir == "/a/b/c/filename"
       */
      char *sp = strrchr(dir,'/');
      if (sp) {
	*sp-- = '\0';			/* skip backwards over '/' */
	sp = strrchr(dir,'/');
      } else return cp;
      if (sp) { ++sp; *sp = '\0'; }
      else return cp;
      ++dp;				/* skip "../" */
      *relative= _B_TRUE;
      return(resolve_dir(dp,dir,relative));
    }
  }
  return cp;
}

/*
 * Follow a logical link and make sure full
 * directory of the last link in the path is
 * the suffix of the resolved link.
 *
 */
static char *suivre(char *s1,char *s2,char *dir,int *doux)
{
  struct stat statbuf;
  size_t mp = MAXPATHLEN;
  lstat(s1, &statbuf);
  if (S_ISLNK(statbuf.st_mode)) {
    int n;
    if ((n = readlink(s1,(void *)s2,mp)) > 0) {
      char *cp,*dp;
      s2[n] = '\0';
      *doux = 1;			/* Soft link found */
      /* See if the directory is relative */
      if (*s2 == '.') {
	int relative = NIL;
	strcpy(dir,s1);			/* make copy of full path */
	s2 = resolve_dir(s2,dir,&relative);
	if (relative) {
	  char tmp[MAXPATHLEN];
	  strcpy(s1,dir);		/* path relative directory */
	  strcpy(tmp,s2);		/* save dir relative path */
	  strcpy(s2,dir);		/* new directory */
	  strcat(s2,tmp);		/* <dir><rest-of-path> */
	  dir[0] = '\0';		/* directory now the suffix */
	}
      }
      dp = strrchr(s1,'/');		/* directory path in original? */
      cp = strrchr(s2,'/');		/* directory path in link? */
      if (dp && !cp) {			/* In original and link a file */
	strcpy(dir,s1);			/* copy directory */
	dp = strrchr(dir,'/');		/* and terminate after last '/' */
	++dp;
	*dp = '\0';
      } else if (!dp && cp)		/* NOT in original and LINK a dir */
	dir[0] = '\0';			/* New directory path to chase */
      return (suivre(s2,s1,dir,doux));	/* swap buffers */
    } else return 0;
  }
  return (s1);
}
/*
 * Follow the link of it is logical */
static char *solaris_follow(char *s1,char *s2,int *soft) 
{
  char dir[MAXPATHLEN];
  char *cp;
  dir[0] = '\0';
  cp = suivre(s1,s2,dir,soft);
  if (dir[0] != '\0') {
    char tmp[MAXPATHLEN];
    strcpy(tmp,cp);
    strcpy(cp,dir);
    strcat(cp,tmp);
  }
  return cp;
}

/*
 * Create tmp file. This will have a copy of all
 * messages upto the message indexed by index0.
 * Called by solaris_expunge and solaris_parse
 *
 * Returns the FD of the tmp file, or -1 on error.
 */
static int
solaris_create_tmp(MAILSTREAM 	* stream,
		   int 		  fd,
		   long		  index0,
		   char		* tmpname,
		   char		* rootname,
		   int 		* doux)
{
  struct stat 		sbuf;

  mode_t 		mode = S_IREAD | S_IWRITE;

  ssize_t 		wlen;

  size_t	 	npages;
  size_t 		the_rest;
  size_t 		len;

  long 			time0;
  long			time1;
  long			time_elapsed;

  int 			tmp_fd;
  int 			i;

  caddr_t 		mmapbase;

  char			buf0[MAXPATHLEN];
  char			buf1[MAXPATHLEN];
  char		*	tn;

#if (_POSIX_C_SOURCE >= 199506L)
  pthread_once(&solarisOnceVar, solaris_once);
#else
  if (solarisArch[0] == '\0') { /* if Not yet initalized */
    solaris_once();
  }
#endif

  time0 = elapsed_ms();

  /* copy mailbox name */
  strcpy(buf0, stream->mailbox);

  /*
   * get the tmp file name. Could be a logical link. We need to resolve
   * it since the expunge takes place in the same directory as where
   * the mail file lives
   */
  tn = solaris_follow(buf0, buf1, doux);
  if (tn == NIL) {
    return(-1);
  } else if (doux) {		/* root name for rename */
    strcpy(rootname, tn);
  }

  /*
   * Get the mode of the original file and preserve it
   */
  if (stat(tn,&sbuf) < 0) {
    return -1;
  }

  /*
   * append a machine id to the root name for machine uniqueness
   * in case NFS mounted 
   */
  len = strlen(tn);

  /* Make sure we can create this name */
  if ((strlen(hw_id) + len + 7) > MAXPATHLEN -1) {
    return -1;
  }

  /*
   * create a unique name
   */
  strcat(tn, hw_id);
  strcat(tn, "XXXXXX");
  mktemp(tn);	/* mktemp() sets string to EMPTY string "" if it fails */
  if (strlen(tn) == len) {
    return(-1);			/* Could not make unique name */
  } else {
    strcpy(tmpname, tn);
  }

  /*
   * Open the tmpfile create/rw
   */
  if (sbuf.st_mode & S_IRGRP) {
    mode |= S_IRGRP;
  }
  if (sbuf.st_mode & S_IWGRP) {
    mode |= S_IWGRP;
  }
  if (sbuf.st_mode & S_IROTH) {
    mode |= S_IROTH;
  }
  if (sbuf.st_mode & S_IWOTH) {
    mode |= S_IWOTH;
  }

  if ((tmp_fd = open(tn, O_RDWR | O_CREAT | O_TRUNC, mode)) < 0) {
    return(-1);
  }

  fchmod(tmp_fd, mode);

  /*
   * We try to set the mailbox back the way it was,
   * if we can't,then we can't.
   */
  (void)fchown(tmp_fd, ((SOLARISLOCAL*)stream->local)->uid, -1);
  (void)fchown(tmp_fd, -1, ((SOLARISLOCAL*)stream->local)->gid);

  /*
   * mmap the source mail file if index > 0:
   *  (1) we compute the size to read in. This is upto but not including
   *      the message indexed by index0.
   *  (2) we map the mail file
   */
  if (index0 == 0) {		/* just write the pseudo message */
    int 	psize;

    if (!solaris_write_pseudo_msg(stream, tmp_fd, &psize)) {
      close(tmp_fd);
      return(-1);
    } else {
      ((SOLARISLOCAL*)stream->local)->pseudo_dirty = NIL;
      return(tmp_fd);
    }
  } else {

    /*
     * The size we mmap is the seek offset to the message
     * indexed by index0 with indexes the first deleted msg:
     * Its seek offset if the length of the file up to but
     * EXCLUDING its data
     */
    len = ((SOLARISLOCAL*)stream->local)->msgs[index0]->seek_offset;
  }
  npages = len / MAXMAPPAGE;
  the_rest = len % MAXMAPPAGE;

  for (len = 0,i = 0; i < npages; ++i) { 

    /* map MAXMAPPAGE bytes */
    if ((mmapbase = mmap(NIL, MAXMAPPAGE, PROT_READ, MAP_PRIVATE,
			 fd, len)) == MAP_FAILED) {
      return(-1);
    }

    /* tell the kernel seq. access OK */
    if (madvise(mmapbase, MAXMAPPAGE, MADV_SEQUENTIAL) < 0) {
      munmap(mmapbase, len);
      close(tmp_fd);
      return(-1);
    }

    /*
     * write the tmp file
     */
    wlen = safe_write(tmp_fd, (void *)mmapbase, MAXMAPPAGE);

    /*
     * unmap the current pages
     */
    munmap(mmapbase, MAXMAPPAGE);
    if (wlen != MAXMAPPAGE) {	/* then the write failed.  */
      close(tmp_fd);
      return(-1);
    }
    len += MAXMAPPAGE;

    /* Notification timer */
    time1 = elapsed_ms();
    time_elapsed = time1 - time0;
    if (time_elapsed >= ACKTIME) {
      char	small[SMALLBUFLEN];

      sprintf(small, "131 busy(TMPFILE(copied %d pages)", i);
      mm_log_stream(stream, "131 busy"); /* ERROR */
      mm_notify(stream, small, PARSE);
      time0 = elapsed_ms();     /* reset timer */
    }
  }

  if (the_rest != 0) {
    /* map MAXMAPPAGE */
    if ((mmapbase = mmap(NIL, the_rest, PROT_READ, MAP_PRIVATE,
			 fd, len)) == MAP_FAILED) {
      return(-1);
    }
    /* tell the kernel seq. access OK */
    if (madvise(mmapbase, the_rest, MADV_SEQUENTIAL) < 0) {
      munmap(mmapbase, len);
      close(tmp_fd);
      return(-1);
    }

    /*
     * write the tmp file
     */
    wlen = safe_write(tmp_fd,(void *)mmapbase, the_rest);

    /*
     * unmap the source mail file
     */
    munmap(mmapbase, the_rest);
    if (wlen != the_rest) {	/* then the write failed.  */
      close(tmp_fd);
      return(-1);
    }
  }
  return(tmp_fd);
}

/*
 * Read messages from mailbox, and write them to the tmp file:
 *  The tmp file contains a copy of all of the messages preceding
 *  the first deleted message.
 */
static
solaris_exp_save_msgs(MAILSTREAM *	stream,
		      int 		fd,
		      int 		tmp_fd,
		      long 		index0,
		      struct stat *	sbuf,
		      char 	*	lock)
{
  long 		i;
  int 		e;
  unsigned long seek_base;
  unsigned long	init_seek_base;
  long 		time0;
  long		time1;
  long		time_elapsed;
  unsigned long max_mbuf = CHUNK;
  long 		nmsgs = stream->nmsgs;
  int 		ret;
  boolean_t	fatal = _B_FALSE;
  boolean_t	resized;
  MESSAGECACHE * elt;

  /* initialize the timer */
  time0 = elapsed_ms();

  /* compute max mmaped buffer size */
  for (i = index0; i < nmsgs; i++) {
    elt = mail_elt(stream, i+1);
    if (!elt->deleted 
	&& ((SOLARISLOCAL*)stream->local)->msgs[i]->real_mem_len > max_mbuf) {

      max_mbuf = ((SOLARISLOCAL*)stream->local)->msgs[i]->real_mem_len;
    }
  }

  /* allocate the pmapped buffer for input */
  if (PARSE_SIZE < max_mbuf + BUFF_EXTRA) {
    solaris_give(PARSE_DATA);
    solaris_get(max_mbuf+BUFF_EXTRA, PARSE_DATA);
    resized = _B_TRUE;

  } else {
    resized = _B_FALSE;
  }

  /* Seek first deleted message */
  seek_base = ((SOLARISLOCAL*)stream->local)->msgs[index0]->seek_offset;

  /* upto the first message we write */
  ((SOLARISLOCAL*)stream->local)->tmp_filesize = seek_base;

  init_seek_base = seek_base;   /* in case of errors */

  /*
   * loop through source messages not removed,
   * and append them to the tmp file
   */
  for (i = index0; i < nmsgs && !fatal; i++) {
    FILECACHE * fc = ((SOLARISLOCAL*)stream->local)->msgs[i];

    /* ignore deleted messages */
    elt = mail_elt(stream, i+1);
    if (elt->deleted) {
      continue;
    }

    /* Give back server busy ticks ... */
    if ((i % 101) == 0) {
      time1 = elapsed_ms();
      time_elapsed = time1 - time0;
      if (time_elapsed >= ACKTIME) {
	char 	small[SMALLBUFLEN];

	sprintf(small, "131 busy(EXPUNGE(Written %d msgs))", i - index0);
	mm_log_stream(stream, "131 busy"); /* ERROR */
	mm_notify (stream, small, PARSE);
	time0 = elapsed_ms();     /* reset timer */
      }
    }

    /* read message from mailbox if necessary. */
    if (!fc->msg_cached) {
      solaris_buffer_msg(stream, fd, fc, lock, NIL);
      fc->msg_buffered = _B_TRUE;
    }

    /* seek to next write offset in tmp file */
    lseek(tmp_fd, seek_base, L_SET);

    /* The new seek offset of this message in tmp file */
    fc->tmp_seek_offset = seek_base;

    /* set up/write iov for this message */
    seek_base += solaris_write_message(tmp_fd, &e, fc);

    /* check error return */
    if (e < 0) {		/* problems !!*/
      int 	retry;

      switch (e) {
#if (SUNOS > 24)
      case EDQUOT:
#endif
      case EFBIG:
      case ENOSPC:
	fatal= _B_TRUE;
	if (!stream->silent) {
	  mm_log("152 Out of space on device. Expunge Aborted", ERROR, stream);
	}
	break;

      case EINTR:
	break;

      default:
	break;
      }

      retry = MAXWRITE_LOOP;
      while (retry--) {
	lseek(fd, fc->tmp_seek_offset, L_SET);
	solaris_write_message (tmp_fd, &e, fc);
	if (e >= 0) {
	  break;
	} else {
	  /* We do not abort since only tmp file is bad  */
	  mm_diskerror(stream, e, T);
	}
      }
      if (e < 0) {
	sprintf(((SOLARISLOCAL*)stream->local)->buf,
		"153 Expunge: Error creating tmp file: err  %s, nmsgs %d, msg number %d",
		strerror (e = errno), nmsgs, index0+1);
	if (!stream->silent) {
	  mm_log(((SOLARISLOCAL*)stream->local)->buf, ERROR, stream);
	}
	fc->msg_buffered = _B_FALSE;
	fatal = _B_TRUE;
	break;
      }
    }

    /* won */
    fc->msg_buffered = NIL;	/* Just temporary */
    ((SOLARISLOCAL*)stream->local)->tmp_filesize += e;	/* count these bytes in data */
  }

  if (resized) {		/* then free big pmapped buffer */
    solaris_give(PARSE_DATA);
    solaris_get(CHUNK, PARSE_DATA);
  }
  if (!fatal) {
    fsync(tmp_fd);	/* make sure the disk has the update */
				/* nuke any cruft after that */
    ftruncate(tmp_fd, ((SOLARISLOCAL*)stream->local)->tmp_filesize);
    fstat(tmp_fd, sbuf);	/* now get updated file time */
    ret = _B_TRUE;
  } else {
    ret = _B_FALSE;
  }
  mm_nocritical(stream);	/* exit critical */

  return(ret);
}

/*
 * Here we have encountered an error in the parsing phase:
 *    This means that we could NOT depend on our index file.
 * So, WE have already READ the VALID PART OF THE FILE into
 * our tmp file.
 *
 * HERE:  We APPEND to the tmp file the broken portions as REPAIRED.
 */
int
solaris_rewrite_msgs (MAILSTREAM * 	stream,
		      int 	   	fd,
		      int 		tmp_fd,
		      long 		index0,
		      long 		nmsgs,
		      struct stat *	sbuf,
		      char 	  *	lock)
{
  FILECACHE *	fc;
  unsigned long max_mbuf = CHUNK;
  off_t 	seek_base;
  off_t		init_seek_base;
  boolean_t	resized;
  boolean_t	fatal = _B_FALSE;
  long 		i;
  long 		time0;
  long		time1;
  long		time_elapsed;
  int 		e;
  int 		ret;
  int 		pseudo_size;

  /*
   * If pseudo is dirty, then write it IF 
   * we are not rewriting the entire file. 
   * Otherwise, solaris_create_tm
   */
  if (((SOLARISLOCAL*)stream->local)->pseudo_dirty) {
    if (!solaris_write_pseudo_msg(stream, tmp_fd, &pseudo_size))
      /* write failed - disk error */
      return(_B_FALSE);
    ((SOLARISLOCAL*)stream->local)->pseudo_dirty = NIL;	/* clean again */
  }

  /*
   * We seek to end of tmp file. Set our tmp file size.
   */
  seek_base = lseek(tmp_fd, (off_t)0, SEEK_END);

  /* upto the first message we write */
  ((SOLARISLOCAL*)stream->local)->tmp_filesize = seek_base;
  init_seek_base = seek_base;       /* in case of errors */

  /* compute max mmaped buffer size */
  for (i = index0; i < nmsgs; i++) {
    fc = ((SOLARISLOCAL*)stream->local)->msgs[i];
    if (fc->real_mem_len > max_mbuf) 
      max_mbuf = fc->real_mem_len;
  }

  /* allocate the pmapped buffer for input */
  if (PARSE_SIZE < max_mbuf + BUFF_EXTRA) {
    solaris_give(PARSE_DATA);
    solaris_get(max_mbuf + BUFF_EXTRA, PARSE_DATA);
    resized = _B_TRUE;
  } else {
    resized = _B_FALSE;
  }

  /*
   * loop through source messages not removed,
   * and serially append them to the tmp file
   */

  /* initialize the timer */
  time0 = elapsed_ms();
  for (i = index0; i < nmsgs && !fatal; i++) {
    ssize_t nwritten;
    fc = ((SOLARISLOCAL*)stream->local)->msgs[i];
    /* Give back server busy ticks ... */
    if ((i % 101) == 0) {
      time1 = elapsed_ms();
      time_elapsed = time1 - time0;
      if (time_elapsed >= ACKTIME) {
	char small[SMALLBUFLEN];
	sprintf(small, "131 busy(UPDATED %d msgs)", i);
	mm_log_stream(stream, "131 busy"); /* ERROR */
	mm_notify(stream, small, PARSE);
	time0 = elapsed_ms();     /* reset timer */
      }
    }
				/* read message from mailbox if necessary. */
    if (!fc->msg_cached) {
      solaris_buffer_msg(stream, fd, fc, lock, _B_TRUE);
      fc->msg_buffered = _B_TRUE;
      if (fc->nulls_found) {	/* nous nous les debarrassons tout de suite! */
	char * nullptr;		/* Let us get rid of them. (Translation of */
				/* (above) */
	fc->nulls_found = NIL;
	if (read_null_chars(fc->internal, fc->original_mem_len, &nullptr)) {
	  u_long ntocheck = fc->original_mem_len;
	  ntocheck -= nullptr - fc->internal;
	  replace_nulls(nullptr, ntocheck);
	}
      }
    }

    /* seek to next write offset in tmp file */
    lseek(tmp_fd, seek_base, L_SET);

    /* The new seek offset of this message in tmp file */
    fc->tmp_seek_offset = seek_base;

    /* set up/write iov for this message */
    nwritten = solaris_write_message(tmp_fd, &e, fc);
    seek_base += nwritten;

    /* check error return */
    if (e < 0) {		/* problems !!*/
      int 	retry;
      switch (e) {
#if (SUNOS> 24)
      case EDQUOT:
#endif
      case EFBIG:
      case ENOSPC:
	fatal = _B_TRUE;
	if (!stream->silent) {
	  mm_log("152 Out of space on device. PARSE Aborted", ERROR, stream);
	}
	break;

      case EINTR:
	break;

      default:
	break;
      }
      retry = MAXWRITE_LOOP;
      while (retry--) {
	lseek(fd, fc->tmp_seek_offset, L_SET);
	solaris_write_message(tmp_fd, &e, fc);
	if (e >= 0) {
	  break;
	} else {
	  /* We do not abort since only tmp file is bad  */
	  mm_diskerror(stream, e, _B_TRUE);
	}
      }
      if (e < 0) {
	sprintf (((SOLARISLOCAL*)stream->local)->buf,
		 "153 Error writing tmp file during parse: %s AT msg %d",
		 strerror (e = errno), index0+1);
	if (!stream->silent) {
	  mm_log(((SOLARISLOCAL*)stream->local)->buf, ERROR, stream);
	}
	fc->msg_buffered = NIL;
	fatal = _B_TRUE;
	break;
      }
    }

    /* won */
    fc->msg_buffered = NIL;	/* Just temporary */
    ((SOLARISLOCAL*)stream->local)->tmp_filesize += e;	/* count these bytes in data */

    /* update the real memory len  */
    fc->real_mem_len = nwritten - 1;	/* -1 for '\n' msg separator */
  }

  if (resized) {			/* then free big pmapped buffer */
    solaris_give(PARSE_DATA);
    solaris_get(CHUNK, PARSE_DATA);
  }
  if (!fatal) {
    fsync(tmp_fd);	/* make sure the disk has the update */
    fstat(tmp_fd, sbuf);	/* now get updated file time */
    ret = _B_TRUE;
  } else {
    ret = _B_FALSE;
  }
  mm_nocritical(stream);	/* exit critical */

  return(ret);
}

/* Solaris  extend mailbox to reserve worst-case space for expansion
 * Accepts: MAIL stream
 *	    file descriptor
 *	    error string
 *          file size
 * Returns: T if extend OK and have gone critical, NIL if should abort
 */

int solaris_extend (MAILSTREAM *stream,int fd,char *error, off_t filesize)
{
  struct stat sbuf;
  FILECACHE *m;
  char tmp[MAILTMPLEN];
  int i,ok;
  long f;
  char *s;
  int retry;
				/* calculate estimated size of mailbox */
  for (i = 0,f = 0; i < stream->nmsgs; i++) {
    unsigned long msgsize;
    m = ((SOLARISLOCAL*)stream->local)->msgs[i];		/* get cache pointer */
    msgsize = m->real_mem_len + 1;/* update guesstimate */
    f += msgsize;
  }
  f += ((SOLARISLOCAL*)stream->local)->pseudo_size;	/*  Add in the pseudo msg size */
  mm_critical (stream);		/* go critical */
				/* return now if file large enough */
  if (f <= filesize) return(_B_TRUE);
  f -= filesize;
  /* Get some mapped data for the extend */
  if (f > PARSE_SIZE) {
    solaris_give(PARSE_DATA);
    solaris_get(f,PARSE_DATA);
  }
  s = PARSE_BASE;
  memset (s,0,f);		/* get a block of nulls */
				/* get to end of file */
  lseek(fd, filesize, L_SET);
  do {
    int again= _B_TRUE;		/* Try forever on disk error for now */
    retry = NIL;		/* no retry yet */
    if (!(ok = (safe_write(fd,s,f) >= 0))) {
      i = errno;		/* note error before doing ftruncate */
				/* restore prior file size */
      ftruncate(fd, filesize);
      fsync(fd);	/* is this necessary? */
      stat(stream->mailbox, &sbuf);		/* now get updated file time */
      ((SOLARISLOCAL*)stream->local)->filetime = sbuf.st_mtime;
      ((SOLARISLOCAL*)stream->local)->accesstime = sbuf.st_atime;
				/* punt if that's what main program wants */
				/* retries if again is True */
      if (mm_diskerror (stream,i,again)) {
	mm_nocritical (stream);	/* exit critical */
	sprintf (tmp,"%s: %s",error,strerror (i));
	mm_notify (stream,tmp,WARN);
	return NIL;
      }
      else retry= _B_TRUE;		/* set to retry */
    }
  } while (retry);		/* repeat if need to try again */
  return ok;			/* return status */
}

/*
 * Solaris make pseudo-header
 * Accepts: MAIL stream
 *          buffer to write it
 *
 * Fix this to use random i/o. Make it fixed length,
 * and write it only when it is dirty, or is not there.
 */
#define PSEUDOTMPLEN 256
char *solaris_pseudo (MAILSTREAM *stream, char *msg)
{
  int 		content_len;
  int		hdrlen;
  char 		fromdate[PSEUDOTMPLEN];
  char 		ximap[PSEUDOTMPLEN];
  time_t 	t = time(0);
  const char *	fromline = PSEUDOFROMLINE;
  const char *	from = "\nFrom: Postmaster";
  const char *	subject  = "\nSubject: Message from mail server       ";
  const char *	subject1 = "\nSubject: Message from mail server        ";
  const char *	mimev = "\nMime-Version: 1.0\nStatus: RO";
  const char *	msgtxt =
    "Delete.\nThis is a system message.                                ";
  const char *	subline;
  char 		c;
  char	     *	cp;
  int 		n;
  int		m;

  /*
   * construct the parts:
   * From ...\n
   * Date:
   */
  const int 	buf_len = 26;		/* a MT-safe version */
  char *	buf_time=fs_get(buf_len);

#if (_POSIX_C_SOURCE - 0>= 199506L)
  sprintf (fromdate,"%s %sDate: ", fromline, ctime_r(&t,buf_time));
#else
  sprintf (fromdate,"%s %sDate: ", fromline, ctime_r(&t,buf_time, buf_len));
#endif
  fs_give((void **) &buf_time );   /* should be OK. */

  /* Append the rfc822 date Date: <date> */
  rfc822_date (fromdate + strlen (fromdate));

  /* Now the ximap header - last header line  */
  sprintf(ximap, "\nX-IMAP: %ld %ld\n\n",
	  stream->uid_validity, 
	  stream->uid_last);

  /*
    We need adjoin "\nContent-Length: <length>" and
   * must compute <length>
   */
  content_len = PSEUDOLEN;
  hdrlen = strlen(fromdate) + strlen(from) + strlen(subject) +
    strlen(ximap) + strlen(mimev) + strlen("\nContent-Length: ");

  content_len -= hdrlen;

  /*
   * Compute digits in content length which are in the
   * "Content-Length: d1d2...dn\n" header.
   */
  n = ndigits((unsigned long)content_len);
  content_len -= n;

  /*
   * may have lost a digit in the subtraction: EG,
   * content_len == 101, n = 3, 100 - 3 = 99.
   * Thus the header would be 1 char shorter. If that is
   * the case, then we use subject1 which is 1 char longer.
   */
  m = ndigits((unsigned long)content_len);
  m = n - m;
  sprintf(msg, "%s%s", fromdate, from);
  subline = (m == 0 ? subject : subject1);
  sprintf(msg + strlen(msg), "%s", subline);

  /* Now, the content length */
  sprintf(msg + strlen(msg), "\nContent-Length: %d", content_len);
  sprintf(msg + strlen(msg), "%s",mimev);

  /* Make the "X-IMAP line last header line in imap tradition */
  sprintf(msg + strlen(msg), "%s", ximap);

  /* Now adjoin the text message */
  sprintf(msg + strlen(msg), "%s", msgtxt);

  /* fill with newlines - leave space for PSEUDOEND*/
  cp = msg + strlen(msg);

  /* less the message chars */
  content_len -= strlen(msgtxt) + strlen(PSEUDOEND); 
  while (content_len--) *cp++ = '\n';

  /* and pseudo end */
  msgtxt = PSEUDOEND;
  while (c = *msgtxt++) *cp++ = c;
  *cp = '\0';

  /* set to default fixed size */
  ((SOLARISLOCAL*)stream->local)->pseudo_size = PSEUDOLEN;

  /* for debugging */
  n = strlen(msg);
  return(msg);
}

/*
 * solaris_parse_msg:
 *
 * Used to call rfc822_parse_msg in case of badly broken mail caused
 * by invalid content-length fields.
 *
 * Parse an RFC822 message
 * Accepts: pointer to return envelope
 *	    pointer to return body
 *	    pointer to header
 *	    header byte count
 *	    pointer to body stringstruct
 *	    pointer to local host name
 *	    pointer to scratch buffer
 *          int trashed which is true if the content-length is tres casse.
 */
void
solaris_parse_msg(ENVELOPE 	**	en,
		  BODY 		**	bdy,
		  char 		*	s,
		  unsigned long 	i,
		  STRING 	*	bs,
		  char 		*	host,
		  char 		*	tmp,
		  int 			trashed,
		  MAILSTREAM	 *	stream)
{
  char 			c;
  char		*	t;
  char		*	d;
  ENVELOPE 	*	env = (*en = mail_newenvelope ());
  BODY 		*	body = bdy ? (*bdy = mail_newbody ()) : NULL;

  /* flag that MIME semantics are in effect */
  boolean_t		MIMEp = _B_FALSE;
  boolean_t		PathP = _B_FALSE;/* flag that a Path: was seen */

  while(i && *s != '\n') {	/* until end of header */
    t = tmp;			/* initialize buffer pointer */
    c = ' ';			/* and previous character */
    while(c) {			/* collect text until logical end of line */
      switch(c = *s++) {	/* slurp a character */

      case '\015':		/* return, possible end of logical line */
	if (*s == '\n')
	  break;	/* ignore if LF follows */

      case '\012':		/* LF, possible end of logical line */
				/* tie off unless next line starts with WS */
	if (*s != ' ' && *s != '\t') {
	  *t++ = c = '\0';
	}
	break;

      case '\t':		/* tab */
	*t++ = ' ';		/* coerce to space */
	break;

      default:			/* all other characters */
	*t++ = c;		/* insert the character into the line */
	break;
      }
      if (!--i) *t++ = '\0';	/* see if end of header */
    }

    /* find header item type */
    if (t = d = strchr(tmp,':')) {

      *d++ = '\0';		/* tie off header item, point at its data */
      while(*d == ' ') {
	d++;	/* flush whitespace */
      }

      while((tmp < t--) && (*t == ' ')) {
	*t = '\0';
      }

      switch (*ucase(tmp)) {	/* dispatch based on first character */

      case '>':			/* possible >From: */
	if (strcmp(tmp+1, "FROM") == 0) {
	  rfc822_parse_adrlist (&env->from, d, host);
	}
	break;

      case 'B':			/* possible bcc: */
	if (strcmp(tmp+1, "CC") == 0) {
	  rfc822_parse_adrlist (&env->bcc, d, host);
	}
	break;

      case 'C':			/* possible cc: or Content-<mumble>*/
	if (*(tmp+1) == 'C') {
	  rfc822_parse_adrlist (&env->cc, d, host);
	} else {
	  if ((strncasecmp(&tmp[1], "ONTENT-", 7) == 0)
	      && body	
	      && (MIMEp || (search(s-1, i, "\012MIME-Version", (long)13)))) {
	    /* Only parse content header if NOT trashed */
	    if (!trashed)	        /* The we do the usual thing */
	      rfc822_parse_content_header(body, tmp+8, d);
	  }
	}
	break;

      case 'D':			/* possible Date: */
	if (!env->date && (strcmp(tmp+1, "ATE") == 0)) {
	  env->date = cpystr(d);
	}
	break;

      case 'F':			/* possible From: */
	if (!strcmp(tmp+1, "ROM")) {
	  rfc822_parse_adrlist(&env->from, d, host);
	}
	break;

      case 'I':			/* possible In-Reply-To: */
	if (!env->in_reply_to && (strcmp(tmp+1, "N-REPLY-TO") == 0)) {
	  env->in_reply_to = cpystr(d);
	}
	break;

      case 'M':			/* possible Message-ID: or MIME-Version: */
	if (!env->message_id && strcmp(tmp+1, "ESSAGE-ID") == 0) {
	  env->message_id = cpystr(d);

	} else if (strcmp(tmp+1, "IME-VERSION") == 0) {
	  /* tie off at end of phrase */
	  if (t = rfc822_parse_phrase(d)) {
	    *t = '\0';
	  }
	  rfc822_skipws(&d);	/* skip whitespace */

	  /* known version? */
	  if (strcmp(d, "1.0") && (strcmp(d, "RFC-XXXX") != 0)) {
	    mm_log("Warning: message has unknown MIME version", PARSE, NIL);
	  }
	  MIMEp= _B_TRUE;		/* note that we are MIME */
	}
	break;

      case 'N':			/* possible Newsgroups: */
	if (!env->newsgroups && (strcmp(tmp+1, "EWSGROUPS") == 0)) {
	  t = env->newsgroups = (char *) fs_get(1 + strlen(d));
	  while (c = *d++) {
	    if (c != ' ' && c != '\t') {
	      *t++ = c;
	    }
	  }
	  *t++ = '\0';
	}
	break;

      case 'P':			/* possible Path: */
	if (strcmp(tmp+1, "ATH") == 0) {
	  PathP= _B_TRUE;
	}
	break;

      case 'R':			/* possible Reply-To: */
	if (strcmp(tmp+1, "EPLY-TO") == 0)
	  rfc822_parse_adrlist(&env->reply_to, d, host);
	break;

      case 'S':			/* possible Subject: or Sender: */
	if (!env->subject && (strcmp(tmp+1, "UBJECT") == 0)) {
	  env->subject = strdup(d);
	} else if (!strcmp(tmp+1, "ENDER")) {
	  rfc822_parse_adrlist(&env->sender, d, host);
	}
	break;

      case 'T':			/* possible To: */
	if (strcmp(tmp+1,"O") == 0) {
	  rfc822_parse_adrlist (&env->to, d, host);
	}
	break;

      default:
	break;
      }
    }
  }

  /*
   * We require a Path: header and/or a Message-ID belonging to a known
   * winning mail program, in order to believe Newsgroups:.  This is because
   * of the unfortunate existance of certain cretins who believe that it
   * is reasonable to transmit messages via SMTP with a "Newsgroups" header
   * that were not actually posted to any of the named newsgroups.
   * The authors of other high-quality email/news software are encouraged to
   * use similar methods to indentify messages as coming from their software,
   * and having done so, to tell us so they too can be blessed in this list.
   */
  if (env->newsgroups != NULL
      && !PathP
      && env->message_id != NULL
      && strncasecmp(env->message_id, "<Pine.", 6)
      && strncasecmp(env->message_id, "<MS-C.", 6)
      && strncasecmp(env->message_id, "<ML-.", 4)) {
#ifdef NO_ONE_CARES
    sprintf(tmp, "Probable bogus newsgroup list \"%s\" in \"%s\" ignored",
	    env->newsgroups,env->message_id);
    mm_log(tmp, PARSE,NIL);
#endif
    fs_give((void **) &env->newsgroups);
  }

  /* default Sender: and Reply-To: to From: */
  if (!env->sender) {
    env->sender = rfc822_cpy_adr (env->from);
  }
  if (!env->reply_to) {
    env->reply_to = rfc822_cpy_adr (env->from);
  }

  /* now parse the body
   *
   * We can parse here even if trashed because we have a default
   * body->type of TYPETEXT in that case -- mail_initbody(..).
   * NOTE: bs == NIL means we do NOT want to do a content parse
   */
  if (bs && body) {
    rfc822_parse_content(body, bs, host, tmp);
  }
  return;
}

/*
 * solaris_fetchenvelope:
 *   fetch the envelope without doing a body parse. 
 *   saves lots of overhead. */
ENVELOPE *
solaris_fetchenvelope (MAILSTREAM *stream,unsigned long msgno, 
			 long flags)
{
  ENVELOPE **env;
  LONGCACHE *lelt;
  FILECACHE *m;
  unsigned long i;

  if (flags & FT_UID) {		/* UID form of call */
    for (i = 1; i <= stream->nmsgs; i++)
      if (mail_uid (stream,i) == msgno)
	solaris_fetchenvelope (stream, i, flags & ~FT_UID);
    return (NULL);			/* didn't find the UID */
  }
  lelt = mail_lelt (stream,msgno);
  env = &lelt->env;		/* get pointers to envelope and body */
  if (!*env) {
    BODY *body = NIL;
    /*
     * o If m has an associated MIME translation, then we use it instead.
     * o If the msg in not cached, then cache it.
     */
    m = ((SOLARISLOCAL*)stream->local)->msgs[msgno-1];
    if (m->mime_tr) m = m->mime_tr;
    else if (!m->msg_cached) {
      if (!m->hdr_cached) {	/* read the header ONLY. NO body parse*/
	if (!solaris_acl_header(stream,m)) {
	  return NIL;
	} else m->hdr_cached= _B_TRUE;
      }
    }
    i = m->rfc822_hdrsize;
    if (i > TMP_SIZE) {	/* make sure enough buffer space */
      ((SOLARISLOCAL*)stream->local)->tmp_data_cached= _B_TRUE;
      solaris_give(TMP_DATA);
      solaris_get (i,TMP_DATA);
    }
    /*
     * parse envelope only  NOTES:
     *   (1) we pass a body NIL string struct. This prevents
     *       a FULL body content parse.
     *   (2) body == NIL gets header content type parsing only.
     */
    solaris_parse_msg (env,&body,m->header,m->rfc822_hdrsize,NIL,
		       mylocalhost(stream),
		       TMP_BASE,
		       m->broken_content_len,
		       stream);
    m->content_type = body->type;
    mail_free_body(&body);	/* Free temporary body */
  }
  return *env;
}

/*
 * Fetch short info for quick header summary.
 */
void solaris_fetchshort (MAILSTREAM *stream, unsigned long msgno, 
			 SHORTINFO *u_sinfo,
			 long flags)
{
  LONGCACHE *lelt;
  int check_required;
  int info_required= 0;
  ENVELOPE *env;
  SHORTINFO *sinfo;
  unsigned long i;

  if (flags & FT_UID) {		/* UID form of call */
    for (i = 1; i <= stream->nmsgs; i++)
      if (mail_uid (stream,i) == msgno)
	solaris_fetchshort (stream, i, u_sinfo, flags & ~FT_UID);
    return;			/* didn't find the UID */
  }

  if (stream->scache) {		/* short cache */
    sinfo = &stream->sinfo;
    if (msgno != stream->msgno){/* flush old poop if a different message */
      mail_free_fshort_data(sinfo);
      check_required = 0;
      info_required = 1;	/* We need to fetch new data. */
    } else 
      check_required = 1;	/* must check if sinfo exists */
    stream->msgno = msgno;
  }
  else {			/* long cache */
    lelt = mail_lelt (stream,msgno);
    sinfo = &lelt->sinfo;
    check_required = 1;		/* must check if sinfo exists */
  }
  /* If any field is NIL, then all of them are: */
  if (check_required) {
    if (sinfo->date == NIL) info_required = 1;
    else
      info_required = 0;
  }
  if (info_required)	{	/* then the sinfo structure is empty */
    MESSAGECACHE *mcache= mail_elt(stream, msgno);
    BODY *body = NIL;
    char tmp[MAILTMPLEN];
    char *date;
    u_short type;
    env = solaris_short_structure(stream,msgno,&type,flags);
    if (!env)
      return;				/* shouldn't happen locally */
    date = mail_date(tmp, mcache);
    sinfo->date = cpystr(date); 
    if (env->from) {
      if (env->from->personal)
	sinfo->personal = cpystr(env->from->personal);
      else
	sinfo->personal = NIL;
      
      if (env->from->mailbox)
	sinfo->mailbox = cpystr(env->from->mailbox);
      else
	sinfo->mailbox = NIL;
      
      if (env->from->host)
	sinfo->host = cpystr(env->from->host);
      else
	sinfo->host = NIL;
    } else {
      sinfo->personal = sinfo->mailbox = sinfo->host = sinfo->from = NIL;
    }
    /*  now the rest of the fields... */
    sinfo->subject = cpystr(env->subject);
    sinfo->bodytype = cpystr(body_types[type]);
    sprintf(tmp, "%d", mcache->rfc822_size);
    sinfo->size = cpystr(tmp);
    sinfo->from = mail_make_from(sinfo);
  }
  /* pass to user's structure */
  u_sinfo->date = sinfo->date;
  u_sinfo->personal = sinfo->personal;
  u_sinfo->mailbox = sinfo->mailbox;
  u_sinfo->host = sinfo->host;
  u_sinfo->subject = sinfo->subject;
  u_sinfo->bodytype = sinfo->bodytype;
  u_sinfo->size = sinfo->size;
  u_sinfo->from = sinfo->from;
}
/* END */
/*
 * Fabricates a "from line" from the hidden "From line ..." of a message.
 * Called only from the imapd */
#define MINFROMLENGTH 40
char *solaris_fab_from (MAILSTREAM *stream, long msgno)
{
  FILECACHE *m= ((SOLARISLOCAL*)stream->local)->msgs[msgno-1];
  int len;
  char c;
  char secret[MINFROMLENGTH+1];
  char *minette,*zizi;
  int nread;
  /* read the first line of the message */
  if (lseek(((SOLARISLOCAL*)stream->local)->fd,(off_t)m->seek_offset,SEEK_SET) < 0) {
    minette = strdup("(No from available)");
    return minette; 
  } else if ((nread = read(((SOLARISLOCAL*)stream->local)->fd,secret,MINFROMLENGTH)) <= 0) {
    minette = strdup("(No from available)");
    return minette;
  }
  secret[nread] = '\0';
  /* We have "From mapetite@hk ...." and want the sender */
  minette = secret;
  minette += 5;			/* skip "From " */
  zizi = minette;		/* keep the base ptr */
  while (c = *minette++) {      /* Skip to after whom it is from */
    if (c == ' ') break;
  }
  len = minette - zizi - 1;	/* the len of the excised from merde */
  minette = fs_get(len + 1);
  strncpy(minette,zizi,len);
  minette[len] = NIL;
  return minette;		/* must be freed!! */
}

void solaris_fetchfast (MAILSTREAM *stream,char *sequence, long flags)
{
  return;			/* no-op for local mail */
}


/* Solaris mail fetch flags
 * Accepts: MAIL stream
 *	    sequence
 */

void solaris_fetchflags (MAILSTREAM *stream,char *sequence, long flags)
{
  return;			/* no-op for local mail */
}


/* Solaris open and lock mailbox without waiting. This is for
 * a mailbox not opened on a stream.
 * Accepts: file name to open/lock
 *	    file open mode
 *	    destination buffer for lock file name
 *	    type of locking operation (LOCK_SH or LOCK_EX)
 * NOTE: This is used for WRITES ONLY.
 */

int solaris_nowait_lock (MAILSTREAM *stream,char *file, int flags,
			 int mode,char *lock, int op)
{
  int 			ld;
  int			j;
  int 			i = NOWAITLOCKTIMEOUT - 1;
  int 			v = QUICK_TIMEOUT;
  char 			tmp[MAILTMPLEN];
  char 			hitch[MAXPATHLEN];
  struct timeval	tp;
  struct stat 		sb;
  int 			is_readonly;
  int 			fd;
  time_t 		wait_time;
  int 			err;
  char 		*	pend;
  UnLink_t		unlinked = UNLINKNOTDONE;

  /* Make sure WRITE access is requested */
  if (!(flags & (O_WRONLY | O_RDWR | O_CREAT))) {
      return -1;
  }

  /* build lock filename */
  strcat(dummy_file(lock, file, stream), ".lock");
  /*
   * Copy lock file to check write access to the directory on
   * which it resides
   */
  strcpy(tmp,lock);
  /* Must have a path here so strrchr always works. */

  pend = strrchr(tmp,'/');
  *pend = '\0';
  if (solaris_path_access(stream, tmp, W_OK, &err)) {
    return -1;
  }

  do {				/* until OK or out of tries */
    gettimeofday(&tp, NIL);
		
    /* build hitching post file name */
    sprintf(hitch, "%s.%d.%d.", lock, time(0), getpid ());

    j = strlen(hitch);		/* append local host name */
    gethostname(hitch + j, (MAXPATHLEN - j) - 1);

    /* try to get hitching-post file */
    mail_stream_setNIL(stream);
    if ((ld = open(hitch ,O_WRONLY|O_CREAT|O_EXCL,
		   (int)mail_parameters(stream,
					GET_LOCKPROTECTION,
					NIL))) < 0) {
      char *	rstr;

      mail_stream_unsetNIL(stream);
      sprintf(tmp, "Error creating %s: %s", lock, strerror(errno)); 

      switch (errno) {
      case EEXIST:		/* try again if existent */
	break;

      case EROFS:		/* READONLY file system */
	rstr = "108 Write access is required, access is readonly"; 
	mm_log(rstr, WARN, stream);
	mm_log_stream(stream, rstr);
	solaris_clear_group_permissions(stream);
	return -1;

      case EACCES:		/* protection fail. Try again if it exists. */
	if (!stat(hitch, &sb)) {
	  break;	
	}

      default:			/* some other failure */
	sprintf(tmp, "Error creating %s: %s", lock, strerror(errno));
	solaris_clear_group_permissions(stream);
	return -1;
      }
    }
    mail_stream_unsetNIL(stream);

    if (ld >= 0) {		/* if made a lock file */

      /* make sure others can break the lock */
      mail_stream_setNIL(stream);
      chmod(hitch, (int)mail_parameters(stream, GET_LOCKPROTECTION, NIL));

      mail_stream_unsetNIL(stream);
      close(ld);		/* close the hitch file */
      link(hitch, lock);	/* tie hitching-post to lock, ignore failure */

      /* The stat is for EXISTING FILES. Prevents the race. */
      stat(hitch, &sb);		/* get its data */
      unlink(hitch);		/* flush hitching post */

      /*
       * If link count .ne. 2, hitch failed.  Set ld to -1 as if open() failed
       * so we try again.  If extant lock file and time now is .gt. file time
       * plus timeout interval, flush the lock so can win next time around.
       */
      if ((ld = (sb.st_nlink != 2) ? -1 : 0) && (!stat(lock, &sb)) &&
	  (tp.tv_sec > (sb.st_ctime + LOCKFILELIFETIME))) 
	unlinked = ((unlink(lock) == 0) ? UNLINKSUCCEEDED : UNLINKFAILED);

    } else {
      sprintf (tmp,
	       "106 Mailbox %s is locked, will TRY to override in %d seconds...",
	       file,
	       i);
      if (!stream->silent) {
	mm_log(tmp, WARN, stream);
      }
      mm_log_stream(stream, tmp);
      sleep (1);		/* wait 1 second before next try */
    }
  } while (i-- && ld < 0);

  if (ld < 0) {
    /* Send a warning if we CANNOT UNLINK the mailbox.lock file */
    if (unlinked == UNLINKFAILED) {
      sprintf (tmp,
	       "168 Mailbox %s.lock CANNOT BE REMOVED!"
	       " Please contact an administrator.",
	       file);
      if (!stream->silent) {
	mm_log(tmp,WARN,stream);
      }
      mm_log_stream(stream, tmp);	/* ERROR 168 */
    }
    solaris_clear_group_permissions(stream);
    return -1;			/* Just in case we missed the tick */
  }
  /*
   * open file - MUST open for Read/write or
   * write for lockf to work
   * Open ...
   * Need at least WRITE access in the flags to get a lockf.
   */
  if (!(flags & (O_RDWR | O_WRONLY))) {
    flags |= O_RDWR;
  }
  fd = open(file, flags, mode);
  if (fd >= 0) {
    /*
     * Never wait forever because mailtool keeps the mail file locked forever.
     * So use LOCK_NB. Note that flock uses lockf
     * under solaris.
     */
    v = QUICK_TIMEOUT;
    while (v-- > 0) {
      if (!solaris_flock(stream, fd, op+LOCK_NB)) {
	mm_log((char*)msg167, WARN,stream);
	mm_log_stream(stream, (char*)msg167);
	sleep(1);
      } else {
	/* We got the lock and opened the file (: */
	solaris_clear_group_permissions(stream);
	return fd;
      }
    }
    /*
     * Could not get the lock, retry counted down to 0:
     *   GIVE UP.
     */
  }
  solaris_clear_group_permissions(stream);

  /* here: open/lock failed */
  if (fd >= 0) {
    close(fd);
  }
  j = errno;			/* preserve error code */
  unlink(lock);			/* flush the lock file */
  errno = j;			/* restore error code */
  return -1;
}

/*
 * Solaris open (if NOT already open)  and lock mailbox
 * Accepts: file name to open/lock
 *	    file open mode
 *	    destination buffer for lock file name
 *	    type of locking operation (LOCK_SH or LOCK_EX)
 */
int
solaris_lock(MAILSTREAM * stream,
	     char 	* file,
	     int 	  flags,
	     int 	  mode,
	     char 	* lock,
	     int 	  op,
	     int 	  quick)
{
  SOLARIS_GLOBALS *	sg = (SOLARIS_GLOBALS *)stream->solaris_globals;
  struct timeval 	tp;
  struct stat 		sb;
  time_t 		wait_time;
  UnLink_t		unlinked = UNLINKNOTDONE;
  int 			ld;
  int			j;
  int			err;
  int 			v = QUICK_TIMEOUT;
  int 			is_readonly;
  int 			secs = sg->lock_timeout;      /* make a variable */
  char 			tmp[MAILTMPLEN];
  char 			hitch[MAXPATHLEN];
  char 		*	pend;

  /* build mailbox.lock */
  strcat(dummy_file(lock, file,stream), ".lock");

  /*
   * Copy lock file to check write access to the directory on
   * which it resides
   */
  strcpy(tmp, lock);
  /* Must have a path here so strrchr always works. */
  pend = strrchr(tmp, '/');
  *pend = '\0';
  if (solaris_path_access(stream, tmp, W_OK, &err) == 0) {

    do {				/* until OK or out of tries */
      gettimeofday(&tp, NIL);

      /* build hitching post file name */
      sprintf(hitch, "%s.%d.%d.", lock,time (0), getpid ());
      j = strlen (hitch);		/* append local host name */
      gethostname (hitch + j,(MAXPATHLEN - j) - 1);

      /* try to get hitching-post file */
      mail_stream_setNIL(stream);
      if ((ld = open(hitch, O_WRONLY|O_CREAT|O_EXCL,
		     (int)mail_parameters(stream,
					  GET_LOCKPROTECTION,
					  NIL))) < 0) {
	sprintf(tmp, "165 Error creating %s: %s", lock, strerror(errno)); 

	switch (errno) {
	case EEXIST:		/* try again if existent */
	  break;

	case EROFS:		/* READONLY file system */
	  ((SOLARISLOCAL*)stream->local)->read_access = stream->rdonly = _B_TRUE;
	  if (!stream->silent) {
	    mm_log ("108 Mailbox access is readonly", WARN, stream);
	  }
	  mm_log_stream(stream, "108 Mailbox access is readonly");
	  *lock = '\0';
	  break;

	case EACCES:		/* protection fail. Try again if it exists. */
	  if (!stat(hitch, &sb)) {
	    break;	
	  }
	  /*FALLTHRU*/
	default:			/* some other failure */
	  if (!stream->silent) {
	    mm_log(tmp, WARN, stream);	/* this is probably not good */
	  }
	  mm_log_stream(stream, tmp);	/* ERROR - 165 */
	  *lock = '\0';		/* don't use lock files */
	  break;
	}
      }
      mail_stream_unsetNIL(stream);
      if (ld >= 0) {		/* if made a lock file */
	struct stat 	lsb;
	int 		stat_ok;
	time_t 		lifetime;

	/* make sure others can break the lock */
        mail_stream_setNIL(stream); 
	chmod (hitch, (int) mail_parameters(stream, GET_LOCKPROTECTION, NIL));	
        mail_stream_unsetNIL(stream);
	close(ld);		/* close the hitch file */
	link(hitch, lock);	/* tie hitching-post to lock, ignore failure */

	/* The stat is for EXISTING FILES. Prevents the race. */
	stat(hitch, &sb);	/* get its data */
	unlink(hitch);		/* flush hitching post */

	/*
	 * If link count .ne. 2, hitch failed.
	 * Set ld to -1 as if open() failed so we try again.  If extant 
	 * lock file and time now is .gt. file time
	 * plus timeout interval, flush the lock so can win next time
	 * around.
	 */
	if ((stat_ok = stat(lock,&lsb)) == 0) {
	  lifetime = lsb.st_ctime + LOCKFILELIFETIME;
	}
	if ((ld = (sb.st_nlink != 2) ? -1 : 0) && (stat_ok == 0) &&
	    (tp.tv_sec > lifetime)) {
	  unlinked = ((unlink(lock) == 0) ? UNLINKSUCCEEDED : UNLINKFAILED);
	}
      }
      if ((ld < 0) && *lock) {	/* if failed to make lock file and retry OK */
	if (!(secs%15)) {
	  sprintf (tmp,
	   "106 Mailbox %s is locked, will TRY to override in %d seconds...",
		   file, secs);
	  if (!stream->silent) {
	    mm_log (tmp,WARN,stream);
	  }
	  mm_log_stream(stream,tmp);	/* ERROR 106 */
	}
	sleep (1);		/* wait 1 second before next try */
	if (quick && (v-- == 0)) {
	  return -1;
	}
      }
    } while (secs-- && ld < 0 && *lock);

    if (ld < 0) {			/* block of code */
      /* Send a warning if we CANNOT UNLINK the mailbox.lock file */
      if (unlinked == UNLINKFAILED) {
	  sprintf (tmp,
		   "168 Mailbox %s.lock CANNOT BE REMOVED!"
		   " Please contact an administrator.",
		   file);
	  if (!stream->silent) {
	    mm_log (tmp, WARN, stream);
	  }
	  mm_log_stream(stream, tmp);	/* ERROR 168 */
      }
      if (((SOLARISLOCAL*)stream->local)->fd >= 0) {
	/*
	 * File already open, and cannot get mailbox.lock.
	 * Return error and let client try again
	 */
	solaris_clear_group_permissions(stream);
	return -1;
      } else {
	/*
	 * Lock failed and file not open -- must be open call:
         *   o mailbox.lock cannot be removed.
         *   o HUGE file is being written by mail.local takes
         *     longer than our wait interval.
         *   Open read only and let the user try later.
	 */
	stream->rdonly = ((SOLARISLOCAL*)stream->local)->read_access = _B_TRUE;
	mm_log("108 Mailbox access is readonly", WARN, stream);
	mm_log_stream(stream, "108 Mailbox access is readonly");/* ERROR */    
      }
    }
  }

  /* 
   * open file - MUST open for Read/write or
   * write for lockf to work
   */
  is_readonly = (flags == O_RDONLY);
  if (is_readonly) {
    flags = O_RDWR;
  }
  /* Try and Open if not already opened */
  if (((SOLARISLOCAL*)stream->local)->fd < 0) {
    int 		access_error;

    /* set access for open RW if possible  */
    if (solaris_mbox_access(stream, file, W_OK, &access_error) == 0) {
      ((SOLARISLOCAL*)stream->local)->fd = open(file, flags, mode);
    } else {
      ((SOLARISLOCAL*)stream->local)->read_access = _B_TRUE;
    }
  }

  if (((SOLARISLOCAL*)stream->local)->fd >= 0 
      && !((SOLARISLOCAL*)stream->local)->read_access) {

    int		retry = FLOCKTIMEOUT;

    /* DtMail hack. See if session locking is on */
    if (stream->session_lockf) { /* Already done. */
	solaris_clear_group_permissions(stream);
	return ((SOLARISLOCAL*)stream->local)->fd;
    }

    /*
     * We have read/write access:
     * Never wait forever because mailtool keeps the mail 
     * file locked forever.
     * So use LOCK_NB. Note that flock uses lockf
     * under solaris.
     */
    v = QUICK_TIMEOUT;

    while (retry-- > 0) {

      if (!solaris_flock(stream,
			 ((SOLARISLOCAL*)stream->local)->fd,
			 op+LOCK_NB) < 0) {
	sprintf(tmp, "106 Trying to lockf() mailbox %s", file);
	if (!stream->silent) {
	  mm_log(tmp, WARN,stream);
	}
	sprintf(tmp, 
		"106 Mailbox %s is locked, will override in %d seconds",
		file,
		retry);
	mm_log_stream(stream, tmp);	/* ERROR */
	sleep(1);
	if (quick && (v-- == 0)) {
	  /* Only a quickie and time is up */
	  if (*lock) {
	    unlink(lock);	/* flush the lock file if any */
	  }
	  solaris_clear_group_permissions(stream);
	  return -1;
	}
      } else {
	/* We got the lock and opened the file (: */
	solaris_clear_group_permissions(stream);
	return ((SOLARISLOCAL*)stream->local)->fd;
      }
    }

    /*
     * Could not get the lock, retry counted down to 0:
     *   (1) if exclusive, then fail.
     *   (2) read/only, then set the stream read/only
     */
    solaris_clear_group_permissions(stream);
    if (op == LOCK_EX) {
      if (!stream->silent)
	mm_log("205 Cannot exclusive lock mailbox", WARN, stream);
      mm_log_stream(stream,"205 Cannot exclusive lock mailbox");
      return -1;			/* forget the file too */
    } else {
      stream->rdonly = _B_TRUE;		/* This can be temporary  */ 
     if (!((SOLARISLOCAL*)stream->local)->read_access) {
	if (!stream->silent) {		/* have WRITE access */
	  mm_log("108 Mailbox access is readonly", WARN, stream);
	}
	mm_log_stream(stream,"108 Mailbox access is readonly");	/* ERROR */
      }
      return ((SOLARISLOCAL*)stream->local)->fd;
    }
  } else {
    if (((SOLARISLOCAL*)stream->local)->fd < 0) {/* Need to try and open */
      if (solaris_mbox_access(stream, file, R_OK, &err) == 0) {
	
	/* ((SOLARISLOCAL*)stream->local)->fd < 0: try opening read only -- */
	if (is_readonly) {		/* READ ONLY / EXAMINE request */
	  ((SOLARISLOCAL*)stream->local)->fd = open(file, O_RDONLY, mode);
	}
      }
    }
    if (((SOLARISLOCAL*)stream->local)->fd >= 0) {/* The file is open RO */
      solaris_clear_group_permissions(stream);
      if (!((SOLARISLOCAL*)stream->local)->read_access) {
	if (!stream->silent) {
	  mm_log("108 Mailbox access is readonly", WARN, stream);
	  mm_log_stream(stream,"108 Mailbox access is readonly");/* ERROR */
	}    
	((SOLARISLOCAL*)stream->local)->read_access = stream->rdonly = _B_TRUE;
      }
      return ((SOLARISLOCAL*)stream->local)->fd;
    }
  }
  /* here ((SOLARISLOCAL*)stream->local)->fd < 0: open/lock failed */

  solaris_clear_group_permissions(stream);
  j = errno;			/* preserve error code */
  if (*lock) {
    unlink(lock);	/* flush the lock file if any */
  }
  errno = j;			/* restore error code */
  mm_log("solaris_lock - Open failed", WARN, stream);
  return -1;
}

/*
 * Returns _B_FALSE if fd is invalid.
 */
static boolean_t
solaris_setftimes(int fd, MAILSTREAM * stream)
{
  struct stat 		sbuf;
  struct timeval 	tp[2];
  boolean_t		res = _B_FALSE;

  if (fd > -1) {
    if (fstat(fd,&sbuf) == 0) {		/* get file times */

      /* if stream and csh would think new mail */
      if (stream && (stream->stream_status & S_OPENED)) {
	if (sbuf.st_atime <= sbuf.st_mtime) {
	  gettimeofday (&tp[0], 0L);

	  /*
	   * set mtime to (now - 1) if necessary
	   *
	   * Note: tp[0].tv_sec --> access time
	   *       tp[1].tv_sec --> modify time
	   *
	   * We want tp[1].tv_sec < tp[0].tv_sec, ie, modify < access
	   *
	   * So NO NEW MAIL will be reported
	   */
	  tp[1].tv_sec = (tp[0].tv_sec > sbuf.st_mtime)
	    ? sbuf.st_mtime : tp[0].tv_sec - 1;

	  tp[1].tv_usec = 0;	/* oh well */

	  /* set the times, note change */
	  if (!utimes(((SOLARISLOCAL*)stream->local)->name, tp)) {
	    /* sync access and modify times */
	    ((SOLARISLOCAL*)stream->local)->filetime = tp[1].tv_sec;	
	    ((SOLARISLOCAL*)stream->local)->accesstime = tp[0].tv_sec;
	  } 
	} else {			/* sync our saved times */
	  ((SOLARISLOCAL*)stream->local)->filetime = sbuf.st_mtime;
	  ((SOLARISLOCAL*)stream->local)->accesstime = sbuf.st_atime;
	}
      }
      res = _B_TRUE;
    }
  }
  return(res);
}

/*
 * Solaris unlock
 * Accepts: file descriptor
 *	    (optional) mailbox stream to check atime/mtime
 *	    (optional) lock file name
 */
void
solaris_unlock(int fd, MAILSTREAM * stream, char * lock)
{
  /* if stream and csh would think new mail */
  if (solaris_setftimes(fd, stream)) {

    /* Not our connected mailbox: DtMail */
    if (!stream || fd != ((SOLARISLOCAL*)stream->local)->fd) {
      solaris_flock(stream, fd, LOCK_UN);	 /* release flockers */
      
    } else if (!stream->session_lockf) {
      /* fd == ((SOLARISLOCAL*)stream->local)->fd && stream */
      solaris_flock(stream, fd, LOCK_UN);	/* release flockers */
    }
  }

  /* flush the lock file if any */
  if (lock != NULL && lock[0] != '\0') {
    unlink(lock);
  }
  return;
}

/*
 * Here we add a little hack to VALID(..)
 * to validate dates of the form
 *   From u Mon May 5\n
 *   From u Tue June 11\n
 * etc ...
 * *s is 
 */
int
other_is_valid(char * s, int * zn, int * ti)
{
  char 	*	cp = s - 1;

  /* backup to space before the digital day */
  if (!isdigit(*cp)) {
    return(_B_FALSE);
  }
  --cp;
  if (isdigit(*cp)) {
    --cp;
  }
  if (*cp != ' ') {
    return(_B_FALSE);
  }

  /* validate the Month: Point to last char */
  --cp;
  if (*cp == 'y' || *cp == 'h' || *cp == 'l' ||
      *cp == 'e' || *cp == 't' || *cp == 'r') {

    /* NO time zone */
    *zn = 0;
    *ti = INVALID_822_DATE;
    return(_B_TRUE);
  }
  return(_B_FALSE);
}
/*
 * check is the date is of valid rfc822 format
 * e.g. Fri, 6 Mar 1996 10:00:00 PST */
int
rfc822_valid(char *s, char *x, int *ti, int *zn, int *rv)
{
  char *tmp = s;
  *rv = 0;
  while (tmp != x) {
    if (*tmp == ',') break;
    else tmp++; 
  }
  /* check if there is any , in date */
  if (tmp == x)  return 0;
    
  /* s points to the beginning and x points to the end */
  if (*x) {
    if (x - s >= 41) {
      for (*zn = -1; x[*zn] != ' '; (*zn)--);
      if ((x[*zn-1] == 'm') && (x[*zn-2] == 'o') && (x[*zn-3] == 'r') &&
          (x[*zn-4] == 'f') && (x[*zn-5] == ' ') && (x[*zn-6] == 'e') &&
          (x[*zn-7] == 't') && (x[*zn-8] == 'o') && (x[*zn-9] == 'm') &&
          (x[*zn-10] == 'e') && (x[*zn-11] == 'r') && (x[*zn-12] == ' '))
        x += *zn - 12;
    }
    if (x - s >= 27) {
      if (x[-4] == ' ') {
        *zn = *ti = -4;
      }
      else if (x[-6] == ' ') {
        if ((x[-5] == '+') || (x[-5] == '-'))
          *zn = *ti = -6;
      }
      else if (x[-3] == ':') {
        *zn = 0; *ti = 0;
      } 
      /* time */
      if (x[*ti -3] != ':') { *ti = 0; return 0;}
      else if (x[*ti - 6] == ':') *ti -= 9;
      else if (x[*ti - 6] == ' ') *ti -= 6;
 
      if (x[*ti - 5] != ' ') {*ti = 0; return 0;}   /* check year */
      if (x[*ti - 9 ] != ' ') {*ti = 0; return 0;}  /* check month */
      if (x[*ti - 11] == ' ') {                     /* check , and date */
        if(x[*ti - 12] == ',') {
          if ((x[*ti -13] != 'n') &&  (x[*ti -13] != 'e') && (x[*ti -13] != 'd') &&
              (x[*ti -13] != 'u') && (x[*ti -13] != 'i') && (x[*ti -13] != 't')) {
            *ti = 0; return 0;
          }
        }
        else {* ti =0; return 0;}
      }
      else if (x[*ti - 12] == ' ') {
        if (x[*ti - 13] == ',') {
          if ((x[*ti -14] != 'n') &&  (x[*ti -14] != 'e') &&
              (x[*ti -14] != 'd') && (x[*ti -14] != 'u') && (x[*ti -14] != 'i') &&
               (x[*ti -14] != 't')) {
            *ti = 0; return 0;
          }
        }
        else { *ti = 0; return 0; }
      }
      else { *ti = 0; return 0; }
    }
    *rv = 1;
    return 1;
  } else return 0;
}

/*
 * Free a FILECACHE element */
void solaris_free_fcache(MAILSTREAM *stream, unsigned long i)
{
  FILECACHE *fc = ((SOLARISLOCAL*)stream->local)->msgs[i];
  if (fc->msg_cached) 
    solaris_give(INTERNAL_DATA(fc));
  fs_give ((void **)&fc);
}

/*
 * ((SOLARISLOCAL*)stream->local)->buf has error message */
static void report_and_avorter(MAILSTREAM *stream,int fd,char *lock)
{
  mm_notify (NIL,((SOLARISLOCAL*)stream->local)->buf,WARN);
  if (!stream->silent) mm_log("System failure",ERROR, stream);
  mm_log_stream(stream,((SOLARISLOCAL*)stream->local)->buf);
  unmap_mbox(stream);
  if (lock) solaris_unlock(fd,stream,lock);
  solaris_abort(stream,NIL);
  if (stream->lock) mail_unlock(stream);
  stream->dead= _B_TRUE;
}

/*
 * read a header into the buffer. 
 *  Length is m->body_offset.
 *  The buffer is large enough to do this */
int solaris_readhdr(MAILSTREAM *stream,int fd,FILECACHE *m,char *buf,char *lock)
{
  int len = m->body_offset;

  /* Use mmap if mapped */
  if (((SOLARISLOCAL*)stream->local)->mbox_mapregion != MAP_FAILED && 
      ((SOLARISLOCAL*)stream->local)->mbox_mapregion != NULL) {
    memcpy(buf,((SOLARISLOCAL*)stream->local)->mbox_mapregion+m->seek_offset,len);
  } else {
    /* Do seek and read */
    if (lseek(fd,(off_t)m->seek_offset,SEEK_SET) < 0) {
      sprintf (((SOLARISLOCAL*)stream->local)->buf, "166 Read error(VM problem?): %s",
	       strerror (errno));
      report_and_avorter(stream,fd,lock);
      return NIL;
    }
    if (read(fd,buf,len) < 0) {
      sprintf (((SOLARISLOCAL*)stream->local)->buf, "166 Read error(VM problem?): %s",
	       strerror (errno));
      report_and_avorter(stream,fd,lock);
      return NIL;
    }
  }
  buf[len] = '\0';
  /* Insure that we end in "\n\n" */
  if (buf[len-2] != '\n') {
    buf[len++] = '\n';		/* append "\n"  */
    if (buf[len-2] != '\n')	/* "\n\n" ? */
      buf[len++] = '\n';		/* Non! append one more */
    buf[len] = '\0';
    m->original_body_offset = m->body_offset = len;
  }
  return(_B_TRUE);
}
/*
 *  Allocate and read message into file cache. Use mmapping.
 */
static solaris_readmsg(MAILSTREAM *stream,int fd,FILECACHE *m,char *lock,
		       int during_parse)
{
  int rml = m->original_mem_len;
  char *msg,*hdr;
  if (lseek(fd,(off_t)m->seek_offset,SEEK_SET) < 0) {
    sprintf (((SOLARISLOCAL*)stream->local)->buf, "166 Read error(VM problem?): %s",
	     strerror (errno));
    report_and_avorter(stream,fd,lock);
    return NIL;
  }
  solaris_get_internal(rml+3,INTERNAL_DATA(m));
  msg = m->internal = INTERNAL_BASE(m);
  if (read(fd,msg,rml) < 0) {
    sprintf (((SOLARISLOCAL*)stream->local)->buf, "166 Read error(VM problem?): %s",
	     strerror (errno));
    report_and_avorter(stream,fd,lock);
    solaris_give(INTERNAL_DATA(m));
    return NIL;
  }
  if (msg[rml-1] != '\n') msg[rml++] = '\n';
  msg[rml] = '\0';
  if (during_parse) m->real_mem_len = rml;
  /* Calculate 822 header/body position */
  hdr = strchr(msg,'\n');
  if (!hdr) {
    /* VERY BROKEN MESSAGE. No \n */
    strcat(msg,"\n");
    m->real_mem_len++;
    hdr = strchr(msg,'\n');
  }
  m->header = hdr + 1;
					/* bod offset is before or after parse */
  m->body = (during_parse ? m->internal + m->body_offset :
	      m->internal + m->original_body_offset);
  /* This is now cached */
  m->msg_cached= _B_TRUE;
  return(_B_TRUE);
}
/*
 *  read an entire msg into PARSE_BUF, and place it in m->internal. 
 *  we assume that m->internal can be overwritten.
 *  Also, this is volatile, ie, caller will free this space.
 */
int solaris_buffer_msg(MAILSTREAM *stream,int fd,FILECACHE *m,char *lock,
		       int original)
{
  int rml = (original ? m->original_mem_len : m->real_mem_len);
  int rsize = rml+BUFF_EXTRA;
  char *msg,*hdr;

  /* If not mmapped then seek to start of message */
  if (((SOLARISLOCAL*)stream->local)->mbox_mapregion == MAP_FAILED || 
      ((SOLARISLOCAL*)stream->local)->mbox_mapregion == NULL) {
    if (lseek(fd,(off_t)m->seek_offset,SEEK_SET) < 0) {
      sprintf (((SOLARISLOCAL*)stream->local)->buf, "166 Read error(VM problem?): %s",
	       strerror (errno));
      report_and_avorter(stream,fd,lock);
      return NIL;
    }
  }
  /* Stash this message in parse_buf:
   *  We want the max of PARSE_SIZE, rsize and ((SOLARISLOCAL*)stream->local)->max_pchunk. */
  if (PARSE_SIZE <= rsize || PARSE_SIZE < ((SOLARISLOCAL*)stream->local)->max_pchunk) {
    size_t new_size = (PARSE_SIZE < ((SOLARISLOCAL*)stream->local)->max_pchunk ? ((SOLARISLOCAL*)stream->local)->max_pchunk :
		       PARSE_SIZE);
    new_size = (rsize < new_size ? new_size : rsize);
    solaris_give(PARSE_DATA);
    solaris_get(new_size,PARSE_DATA);
    ((SOLARISLOCAL*)stream->local)->max_pchunk = PARSE_SIZE;
  }
  msg = PARSE_BASE;
  /* Use mmap if applicable */
  if (((SOLARISLOCAL*)stream->local)->mbox_mapregion != MAP_FAILED && 
      ((SOLARISLOCAL*)stream->local)->mbox_mapregion != NULL) {
    memcpy(msg, ((SOLARISLOCAL*)stream->local)->mbox_mapregion+m->seek_offset, rml);
  } else {
    /* read from file */
    if (read(fd,msg,rml) < 0) {
      sprintf (((SOLARISLOCAL*)stream->local)->buf, "166 Read error(VM problem?): %s",
	       strerror (errno));
      report_and_avorter(stream,fd,lock);
      fs_give((void **)&msg);
      return NIL;
    }
  }
  m->internal = msg;
  if (msg[rml-1] != '\n') msg[rml++] = '\n';
  msg[rml] = '\0';
  m->real_mem_len = rml;
  /* Calculate 822 header/body position */
  hdr = strchr(msg,'\n');
  if (!hdr) {
    /* VERY BROKEN MESSAGE. No \n */
    strcat(msg,"\n");
    m->real_mem_len++;
    hdr = strchr(msg,'\n');
  }
  m->header = hdr + 1;
  /*
   * When file is being parsed, then we need the original
   * body offset since we are reading from that file, and
   * copying into a tmp file. 
   *
   * Otherwise, the body_offset is applied since the file
   * data is in place */
  m->body = (original ? msg + m->original_body_offset :
	     msg + m->body_offset);
  return(_B_TRUE);
}

/*
 * Compute rfc822_size of the message here,ie, we need
 * to add 1 for each LF to accommodate the CRLF on
 * the wire */
void solaris_rfc822_size (MAILSTREAM *stream,FILECACHE *m,
			  char *msg,unsigned long msglen)
{
  char *s;
  unsigned long is;
  int firstline = 0;
  unsigned long loc_len,sav_len;
				/* count newlines in body */
  is = 0;
  /* If we are not keeping MIME translations, then
   * we count the newlines in the MIME translated message:
   *  1. NO mime translation or keeping mime
   *       use m as passed
   *  2. m->mime_tr != NULL  Use m->mime_tr
   *       Here we use the size of the internal translation
   *       that we keep around for our c-client/IMAP world. There
   *       is a cost. Yikes! */
  if (m->mime_tr) {
    m = m->mime_tr;		/* use the VMime blob  */
    s = m->header;		/* rfc822 header here */
    sav_len = loc_len = m->rfc822_hdrsize + m->bodysize;
  } else {
    s = msg;
    sav_len = loc_len = msglen;
    while (loc_len--) {		/* need to skip hidden "From " line */
      firstline += 1;
      if (*s++ == '\n') break;
    }
  }
  /* Count the LFs */
  while (loc_len--) if (*s++ == '\n') is++; 
  m->rfc822_size = sav_len + is - firstline;
}
/*
 * We can change this to be any kind of alloc we want. In
 * particular pmap stuff */
void solaris_resize_inbuf(MAILSTREAM *stream, unsigned long size)
{
  ((SOLARISLOCAL*)stream->local)->buflen = size;
  fs_resize ((void **) &((SOLARISLOCAL*)stream->local)->buf,((SOLARISLOCAL*)stream->local)->buflen);
}
/*
 * ACcess the header given the stream, and  FILECACHE pointer */
int solaris_acl_header(MAILSTREAM *stream,FILECACHE *m)
{
  int len = m->headersize;
  char *r822;
  if (m->msg_cached) return(_B_TRUE);		/* Just in case  */
  if (lseek(((SOLARISLOCAL*)stream->local)->fd,(off_t)m->seek_offset,SEEK_SET) < 0) {
    mm_notify(stream,"Error reading header",ERROR);
    mm_diskerror(stream,errno,NIL);
    sprintf (((SOLARISLOCAL*)stream->local)->buf, "166 Read error(VM problem?): %s",
	     strerror (errno));
    report_and_avorter(stream,((SOLARISLOCAL*)stream->local)->fd,NIL);
  }
  if (HDR_SIZE < len+1) {
    solaris_give(HDR_DATA);
    solaris_get(len+1,HDR_DATA);
    ((SOLARISLOCAL*)stream->local)->hdr_data_cached= _B_TRUE;
  }
  m->full_header = HDR_BASE;
  if (read(((SOLARISLOCAL*)stream->local)->fd,m->full_header,len) < 0) {
    mm_notify(stream,"Error reading header",ERROR);
    mm_diskerror(stream,errno,NIL);
    sprintf (((SOLARISLOCAL*)stream->local)->buf, "166 Read error(VM problem?): %s",
	     strerror (errno));
    report_and_avorter(stream,((SOLARISLOCAL*)stream->local)->fd,NIL);
  }
  /* Step on the "Status" stuff */
  m->full_header[len-1] = '\n';
  m->full_header[len] = '\0';
  /* now, point m->header to rfc822_hdr */
  r822 = strchr(m->full_header,'\n');
  m->header = r822 + 1;
  return(_B_TRUE);
}
/*
 * ACcess the body given the stream, and  FILECACHE pointer */
int solaris_acl_body(MAILSTREAM *stream,FILECACHE *m)
{
  int len = m->bodysize;
  off_t le_corps = m->seek_offset+m->body_offset;
  if (m->msg_cached) return(_B_TRUE);		/* Just in case  */
  if (lseek(((SOLARISLOCAL*)stream->local)->fd,le_corps,SEEK_SET) < 0) {
    mm_notify(stream,"Error reading text",ERROR);
    mm_diskerror(stream,errno,NIL);
    sprintf (((SOLARISLOCAL*)stream->local)->buf, "166 Read error(VM problem?): %s",
	     strerror (errno));
    report_and_avorter(stream,((SOLARISLOCAL*)stream->local)->fd,NIL);
    return NIL;
  }
  if (BODY_SIZE < len+1) {
    solaris_give(BODY_DATA);
    solaris_get(len+1,BODY_DATA);
    ((SOLARISLOCAL*)stream->local)->body_data_cached= _B_TRUE;
  }
  m->body = BODY_BASE;
  if (read(((SOLARISLOCAL*)stream->local)->fd,m->body,len) < 0) {
    mm_notify(stream,"Error reading text",ERROR);
    mm_diskerror(stream,errno,NIL);
    sprintf (((SOLARISLOCAL*)stream->local)->buf, "166 Read error(VM problem?): %s",
	     strerror (errno));
    report_and_avorter(stream,((SOLARISLOCAL*)stream->local)->fd,NIL);
  }
  m->body[len] = '\0';
  return(_B_TRUE);
}

/*
 * Solaris get/free functions. Use mmap
 */
static
char *	_solaris_get(unsigned long osize, struct chunk * cp, size_t size)
{
#if (_POSIX_C_SOURCE >= 199506L)
  pthread_once(&solarisOnceVar, solaris_once);
#else
  if (solarisArch[0] == '\0') { /* if Not yet initalized */
    solaris_once();
  }
#endif

  cp->chunk_size = (size + pagesize - 1) & ~(pagesize - 1);

  if ((cp->chunk_base = mmap(0,
			     cp->chunk_size,
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE,
			     anon_fd,
			     0)) == MAP_FAILED) {

    const char 	*	errnoString = "";
    const char 	*	eMsg = "c-client: mmap() failed: ";

    switch(errno) {
    case EACCES:
      errnoString = eacces;
      break;

    case EAGAIN:
      errnoString = eagain;
      break;

    case EBADF:
      errnoString = ebadf;
      break;

    case EINVAL:
      errnoString = einval;
      break;

    case ENODEV:
      errnoString = enodev;
      break;

    case ENOMEM:
      errnoString = enomem;
      break;

    case ENXIO:
      errnoString = enxio;
      break;
    }

    /*
     * This printing to stderr (fd==2) is done with write() so if we are
     * totally hosed, something gets printed.
     *
     * No need to check for errors, there would be nothing we could
     * do about them.
     */
    (void)safe_write(2, (void*)eMsg, strlen(eMsg));
    (void)safe_write(2, (void*)errnoString, strlen(errnoString));
    (void)safe_write(2, "\n", 1);
    fatal("c-client - mmap() failed and it can not recover, "
	  "detailed error message written to stderr.\n",
	  NULL);
    /*NOTREACHED*/
  }
  return cp->chunk_base;
}

char *
solaris_get(unsigned long osize, struct chunk * cp)
{
  size_t 	size = data_buflen(osize, CHUNK);
  
  return(_solaris_get(osize, cp, size));
}

/*
 * Solaris get/free functions. Use mmap 
 */
char *solaris_get_internal(unsigned long osize,struct chunk *cp)
{
  size_t size = osize + 16;

  return(_solaris_get(osize, cp, size));
}

/*
 * Solaris free functions. */
int solaris_give(struct chunk *cp)
{
  return (munmap(cp->chunk_base, cp->chunk_size));
}
/*
 * resize for mmap too. ONLY FOR parse_buf */
void solaris_resize (struct chunk *cp,unsigned long nsize)
{
  char		*rc = 0;
  size_t	osize = cp->chunk_size;
  caddr_t	oaddr = cp->chunk_base;

#if (_POSIX_C_SOURCE >= 199506L)
  pthread_once(&solarisOnceVar, solaris_once);
#else
  if (solarisArch[0] == '\0') { /* if Not yet initalized */
    solaris_once();
  }
#endif

  /* Normalize size to multiples of CHUNKS */
  nsize = data_buflen(nsize,CHUNK);
  if (nsize <= osize && nsize >= osize - pagesize * REMAP_THRESH)
    return;
  /*
   * solaris_get() rewrites cp */
  if ((rc = solaris_get(nsize,cp)) == 0) {
    fatal("PMAP get failed[Out of Virual Memory]",NIL);
  }
  /* Overkill to copy whole chunk: better to record actual size */
  (void) memcpy(cp->chunk_base, oaddr,
		(nsize > osize) ? osize : nsize);
  munmap(oaddr,osize);
  return;
}

/*
 * Here we free the body/header of a message IF
 * these are temporarily cached for a imap command. 
 * This is only for the imapd.c use. It is called
 * at the end of a command cycle.
 *
 * m->mime_tr <==> message stuff NEVER cached
 * m->msg_cached <==> in primary parsing loop
 *   So, we have fire wall checks.
 */
void solaris_free_body(MAILSTREAM *stream, unsigned long msgno)
{
  FILECACHE *m = ((SOLARISLOCAL*)stream->local)->msgs[msgno-1];
  /*
   * Free iff 
   *   NOT mime translation
   *   message is NOT cached
   *   body IS cached */
  if (!m->mime_tr && !m->msg_cached) {
    /* Only one chuck is used for all body fetches since they
     * are serial */
    if (((SOLARISLOCAL*)stream->local)->body_data_cached) {
      ((SOLARISLOCAL*)stream->local)->body_data_cached = NIL;	/* return to min size */
      solaris_give(BODY_DATA);
      solaris_get(CHUNK,BODY_DATA);
    }
    if (((SOLARISLOCAL*)stream->local)->tmp_data_cached) {
      ((SOLARISLOCAL*)stream->local)->tmp_data_cached = NIL;    /* return to min size */
      solaris_give(TMP_DATA);
      solaris_get(CHUNK,TMP_DATA);
    }
    if (((SOLARISLOCAL*)stream->local)->hdr_data_cached) {
      ((SOLARISLOCAL*)stream->local)->hdr_data_cached = NIL;    /* return to min size */
      solaris_give(HDR_DATA);
      solaris_get(CHUNK,HDR_DATA);
    }
    if (m->body_cached) {
      m->body_cached = NIL;
      m->body = NIL;
    }
    if (m->hdr_cached) {
      m->hdr_cached = NIL;
      m->full_header = m->header = NIL;
    }
  }
}
static write_long(int fd,unsigned long *l)
{
  if (safe_write(fd,(void *)l,sizeof(unsigned long)) != sizeof(unsigned long)) 
    return NIL;
  else return(_B_TRUE);
}

static write_short(int fd,short *s)
{
  if (safe_write(fd,(void *)s,sizeof(short)) != sizeof(short)) 
    return NIL;
  else return(_B_TRUE);
}

static write_char(int fd,char *s)
{
  if (safe_write(fd,(void *)s,sizeof(char)) != sizeof(char)) 
    return NIL;
  else return(_B_TRUE);
}

static write_string(int id,char *str,int len)
{
  if (safe_write(id,(void *)str,len) != len) 
    return NIL;
  else return(_B_TRUE);
}  
static read_long(int fd,unsigned long *l)
{
  if (read(fd,(void *)l,sizeof(unsigned long)) != sizeof(unsigned long)) 
    return NIL;
  else return(_B_TRUE);
}
static read_short(int fd, short *s)
{
  if (read(fd,(void *)s,sizeof(short)) != sizeof(short)) 
    return NIL;
  else return(_B_TRUE);
}

static read_char(int fd, char *s)
{
  if (read(fd,(void *)s,sizeof(char)) != sizeof(char)) 
    return NIL;
  else return(_B_TRUE);
}

static read_string(int id,char *str,int len)
{
  if (read(id,(void *)str,len) != len) 
    return NIL;
  else return(_B_TRUE);
}  
#define INDEXRETRY 3
static boolean_t
lock_index(MAILSTREAM * stream, int id, int op)
{
  int 		retry = INDEXRETRY;
  boolean_t	success = _B_FALSE;

  while (retry-- > 0) {
    if (!solaris_flock(stream, id, op+LOCK_NB)) {
      mm_log("106 Trying to lockf() index", WARN, NIL);
      sleep(1);
    } else {
      success = _B_TRUE;
      break;
    }
  }
  return(success);
}

static void
unlock_index(MAILSTREAM * stream, int id)
{
  if (id >= 0) {
    solaris_flock(stream, id, LOCK_UN);	
  }
}

/*
 * index file routines here
 */
static char *
create_index_name(MAILSTREAM *stream, char *file)
{
  char 		buf1[MAXPATHLEN];
  char 		lbuf[sizeof(unsigned long)+1];
  char 	*	cp;
  int 		fd;
  int 		i;

  /* get base directory of mailbox */
  strcpy(buf1,stream->mailbox);
  cp = solaris_follow(buf1, file, &i);
  if (!cp) {
    return(NULL);
  }
  strcpy(buf1, cp);

  /* get last '/' */
  cp = strrchr(buf1, '/');
  if (cp) {
    *cp = '\0';
    ++cp;
  } else {
    return(NULL);
  }
  /* ".<dirpath><filename><prefix>" */
  sprintf(file,"%s/.%s%s", buf1, cp, INDEXFILEPREFIX);
  return(file);
}

static char *
create_index_name_name(char *base,char *file)
{
  char		buf1[MAXPATHLEN];
  char 		lbuf[sizeof(unsigned long)+1];
  char 	*	cp;
  int 		fd;
  int 		i;

  file[0] = '\0';

  /* get base directory of mailbox */
  strcpy(buf1, base);

  cp = solaris_follow(buf1, file, &i);
  if (!cp) {
    return(NULL);
  }
  strcpy(buf1, cp);

  /* get last '/' */
  cp = strrchr(buf1,'/');
  if (cp) {
    *cp = '\0';
    ++cp;
  } else {
    return(NULL);
  }

  /* ".<dirpath><filename><prefix>" */
  sprintf(file, "%s/.%s%s", buf1, cp, INDEXFILEPREFIX);
  return(file);
}

/*
 * Here look for index files with smaller versions
 */
static void
remove_old_index_files(char *mailbox, boolean_t newNameIsBad)
{
  DIR		*	dirp;

  struct dirent *	result;

  struct _dirData {
    struct dirent	e;
    char		n[MAXPATHLEN + 1];
  } dirData;
    
  char 			file[MAXPATHLEN + 1];
  char 			buf[MAXPATHLEN + 1];
  char 			dirName[MAXPATHLEN + 1];
  char		*	cp;
  
  int			len;
  int 			i;

  /* If new file name has old version information, delete it also */
  if (newNameIsBad) {
    cp = create_index_name_name(mailbox, file);
    if (cp != NULL) {
      (void)unlink(cp);
    }
  }

  /* get base directory of mailbox */
  strcpy(dirName, mailbox);
  cp = solaris_follow(dirName, file, &i);
  if (cp != NULL) {
    strcpy(dirName, cp);
    /* get last '/' */
    cp = strrchr(dirName,'/');
    if (cp != NULL) {
      *cp = '\0';
      ++cp;

      /* What is the index file name */
      sprintf(file, "%s/.%s%s%04d",
	      dirName, cp, OLDINDEXFILEPREFIX, INDEXVERSION);

      cp = strrchr(file, '/');
      if (cp != NULL) {
	cp++;
      } else {
	cp = file;
      }
      len = strlen(cp);

      /* Look for versions that do not match */
      dirp = opendir(dirName);
      if (dirp) {

#if (_POSIX_C_SOURCE >= 199506L)
	while(readdir_r(dirp, &dirData.e, &result) == 0)
#else
	while((result = readdir_r(dirp, &dirData.e)) != NULL)
#endif	  
	{
	  if (strlen(result->d_name) == len) {
	    if (strncmp(result->d_name, cp, len - 4) == 0) {
	      sprintf(buf, "%s/%s", dirName, result->d_name);
	      unlink(buf);
	    }
	  }
	}
      }
    }
  }
  return;
}

static boolean_t
read_index_hdr(MAILSTREAM * stream, INDEXHDR * ihdr)
{
  char		buff[MAXPATHLEN];
  char	*   	file = create_index_name(stream,buff);
  boolean_t	locked;

#if (_POSIX_C_SOURCE >= 199506L)
  pthread_once(&solarisOnceVar, solaris_once);
#else
  if (solarisArch[0] == '\0') { /* if Not yet initalized */
    solaris_once();
  }
#endif

  if (((SOLARISLOCAL*)stream->local)->id < 0) {
    if (!file
	|| (((SOLARISLOCAL*)stream->local)->id = open(file, O_RDWR)) < 0) {
      return(_B_FALSE);
    }

  } else if (lseek(((SOLARISLOCAL*)stream->local)->id, 0L, SEEK_SET < 0)) {
    close(((SOLARISLOCAL*)stream->local)->id);
    ((SOLARISLOCAL*)stream->local)->id = -1;
    return(_B_FALSE);
  } 

  if ((locked = lock_index(stream, 
			   ((SOLARISLOCAL*)stream->local)->id,
			   LOCK_SH))
      && (read(((SOLARISLOCAL*)stream->local)->id,
	       ihdr,
	       sizeof(INDEXHDR)) < 0)) {

    if (locked) {
      unlock_index(stream, ((SOLARISLOCAL*)stream->local)->id);
    }
    close(((SOLARISLOCAL*)stream->local)->id);
    ((SOLARISLOCAL*)stream->local)->id = -1;
    return(_B_FALSE);

  } else {

    if (!((SOLARISLOCAL*)stream->local)->iname) {
      ((SOLARISLOCAL*)stream->local)->iname = strdup(file);
    }
    unlock_index(stream, ((SOLARISLOCAL*)stream->local)->id);
    return(_B_TRUE);
  }
}

static boolean_t
read_entry(int id, INDEXENTRY *ie)
{
  if (read(id, ie, sizeof(INDEXENTRY)) != sizeof(INDEXENTRY)) {
    return(_B_FALSE);
  }
  return(_B_TRUE);
}

static boolean_t
verify_messages(MAILSTREAM * stream, INDEXHDR * ihdr)
{
  boolean_t		isOk = _B_FALSE;

  if (stream != NULL) {
    SOLARISLOCAL	*	solaris = (SOLARISLOCAL*)stream->local;

    if (solaris->id > -1 && solaris->fd > -1) {

      struct stat	sbuf;

      if (fstat(solaris->id, &sbuf) == 0) {

	/* If the index file is the correct size for the number of entries */
	if (sbuf.st_size == ((ihdr->nmsgs * INDEXENTRYSIZE)
			     + sizeof(INDEXHDR))) {

	  /* Get the current postition so that it can be reset later */
	  off_t			currentPosition = lseek(solaris->fd,
							(off_t)0,
							SEEK_CUR);
	  if (currentPosition > -1) {
	    INDEXENTRY		ie;
	    int			msgNumber;

	    /* Check each index entry in the index file */
	    for (msgNumber = 0; msgNumber < ihdr->nmsgs; msgNumber++) {

	      /* Calculate the index entry offset */
	      off_t offset = sizeof(INDEXHDR) + (msgNumber * INDEXENTRYSIZE);
	      if (lseek(solaris->id, offset, SEEK_SET) == -1) {
		break;

	      } else {
/* The 1st line of the message (From' ') line, should fit in this size */
#define VERIFY_MSG_SIZE		200

		char		aBuf[VERIFY_MSG_SIZE+1];
		char	*	eol;
		int		timeOffset;
		int		zoneOffset;
		int		rfc822Valid;

		aBuf[VERIFY_MSG_SIZE] = '\0';

		/* Read the index entry for this message */
		if (!read_entry(solaris->id, &ie)) {
		  break;
		}
	    
		/*
		 * Seek to where the index file says is the
		 * start of the message
		 */
		if (lseek(solaris->fd, ie.seek_offset, SEEK_SET) == -1) {
		  break;
		}

		/* Read part of the message */
		if (read(solaris->fd, aBuf,
			 VERIFY_MSG_SIZE) != VERIFY_MSG_SIZE) {
		  break;
		}

/* No longer needed */
#undef VERIFY_MSG_SIZE

		/* VALIDate the message */
		VALID(aBuf, &eol, &timeOffset, &zoneOffset, &rfc822Valid);
		if (timeOffset == 0) {	/* If bad message */
		  break;
		}
	      }
	    }

	    /* If all index entries validated ok, index looks good */
	    if (msgNumber == ihdr->nmsgs) {
	      isOk = _B_TRUE;
	    }
	  }

	  /* Return folder to original position */
	  lseek(solaris->fd, currentPosition, SEEK_SET);
	}
      }
    }
  }

  return(isOk);
}

static boolean_t
valid_index_hdr(MAILSTREAM * stream, INDEXHDR * ihdr, struct stat * sb)
{ 
  boolean_t		isOk = _B_FALSE;

#if (_POSIX_C_SOURCE >= 199506L)
  pthread_once(&solarisOnceVar, solaris_once);
#else
  if (solarisArch[0] == '\0') { /* if Not yet initalized */
    solaris_once();
  }
#endif

  /* Must be the correct architecture */
  if (strncmp(solarisArch, ihdr->arch, sizeof(ihdr->arch)) == 0) {

    /* valid if not written or (written and NOT read) */
    if (ihdr->modify_time == sb->st_mtime
	|| ihdr->access_time == sb->st_atime) {

      /* Must tbe the correct version */
      if (strncmp(ihdr->index_version, INDEXVERSION,
		  sizeof(ihdr->index_version)) == 0) {

	/* 
	 * And the INODE of the mailfolder must match what the
	 * index file thinks it should be.
	 */
	if (ihdr->ino == sb->st_ino) {

	  /*
	   * And if the file is still the same size.
	   */
	  if (ihdr->filesize == sb->st_size) {
	    isOk = verify_messages(stream, ihdr);
	  }
	}
      }
    }
  } else {
    close(((SOLARISLOCAL*)stream->local)->id);
    ((SOLARISLOCAL*)stream->local)->id = -1;
  }
  return(isOk);
}

/*
 * called after a successful expunge */
static void
recreate_index_file(MAILSTREAM *stream)
{
  struct stat	sb;

  stat(stream->mailbox, &sb);
  if (stream->keep_mime
      && (((SOLARISLOCAL*)stream->local)->inbox
	  || sb.st_size >= MININDEXREQUIREDSIZE)) {
    if (((SOLARISLOCAL*)stream->local)->id >= 0) {
      close(((SOLARISLOCAL*)stream->local)->id);
    }
    create_index(stream,&sb);
    update_index(stream);
  }
}

static
create_index(MAILSTREAM *stream,struct stat *sb)
{
  char		buff[MAXPATHLEN];
  char 	*	file;
  int 		id;

  /* Must have full access to do this, and be owner */
  if (((SOLARISLOCAL*)stream->local)->read_access || stream->rdonly) {
    return(_B_FALSE);

  } else if (getuid() != sb->st_uid) {

    return(_B_FALSE);
  }

  if ((file = ((SOLARISLOCAL*)stream->local)->iname) == NIL) {
    if (!(file  = create_index_name(stream,buff))) {
      return(_B_FALSE);
    }
  }
  if ((id = open(file,O_RDWR | O_CREAT,0600)) < 0) {
    return(_B_FALSE);
  } else {
    ((SOLARISLOCAL*)stream->local)->id = id;
  }

  ftruncate(id,(off_t)0);
  lock_index(stream, id,LOCK_EX);
  if (!write_index_hdr(stream, id, 1L)) {
    unlock_index(stream, id);
    close(id);
    unlink(file);
    ((SOLARISLOCAL*)stream->local)->id = -1;
    return(_B_FALSE);
  } else if (!((SOLARISLOCAL*)stream->local)->iname) {
    ((SOLARISLOCAL*)stream->local)->iname = strdup(file);
  }
  fsync(id);			/* sync the new index file */
  unlock_index(stream, id);
  mm_notify(stream,"Created index file(PARSE)",PARSE);
  return(_B_TRUE);
}

/* called after writes to file which change the writetime */
static boolean_t
update_index_hdr(MAILSTREAM *stream)
{
  if (!stream || ((SOLARISLOCAL*)stream->local)->id < 0) {
    return(_B_FALSE);
  } else if (!lock_index(stream,
			 ((SOLARISLOCAL*)stream->local)->id,
			 LOCK_EX)) {
    return(_B_FALSE);
  }

  solaris_setftimes(((SOLARISLOCAL*)stream->local)->fd, stream);
  if (!write_index_hdr(stream, ((SOLARISLOCAL*)stream->local)->id, 0L)) {
    unlock_index(stream, ((SOLARISLOCAL*)stream->local)->id);
    close(((SOLARISLOCAL*)stream->local)->id);
    ((SOLARISLOCAL*)stream->local)->id = -1;
    return(_B_FALSE);
  } else {
    unlock_index(stream, ((SOLARISLOCAL*)stream->local)->id);
    return(_B_TRUE);
  }
}

static boolean_t
write_index_hdr(MAILSTREAM *stream,int fd,unsigned long new)
{
  /* Now write the header record */
  INDEXHDR 	ihdr;

#if (_POSIX_C_SOURCE >= 199506L)
  pthread_once(&solarisOnceVar, solaris_once);
#else
  if (solarisArch[0] == '\0') { /* if Not yet initalized */
    solaris_once();
  }
#endif

  if (lseek(fd ,0L, SEEK_SET) < 0) {
    return(_B_FALSE);
  }

  strcpy(ihdr.index_version, INDEXVERSION);
  strcpy(ihdr.arch, solarisArch);
  ihdr.nmsgs = stream->nmsgs;
  ihdr.filesize = ((SOLARISLOCAL*)stream->local)->filesize;
  ihdr.ino = ((SOLARISLOCAL*)stream->local)->ino;
  ihdr.modify_time = ((SOLARISLOCAL*)stream->local)->filetime;
  ihdr.access_time = ((SOLARISLOCAL*)stream->local)->accesstime;
  ihdr.new_index = new;
  ihdr.uid_validity = stream->uid_validity;
  ihdr.uid_last = stream->uid_last;
  ihdr.checksum = stream->checksum;
  ihdr.mbox_size = stream->mbox_size;

  if (!safe_write(fd, &ihdr, sizeof(INDEXHDR))) {
    return(_B_FALSE);
  }
  return(_B_TRUE);
}

static boolean_t
read_index_entry(MAILSTREAM 	*	stream,
		 int 			id,
		 INDEXENTRY 	*	ie,
		 unsigned long 		msgno,
		 int 			serial)
{
  /* seek the entry if not serial */
  if (!serial) {
    off_t offset = sizeof(INDEXHDR) + ((msgno - 1)*INDEXENTRYSIZE);

    if (lseek(id, offset, SEEK_SET) < 0) {
      return(_B_FALSE); 
    }
  } else {
    /* see if mmapped: */
    if (((SOLARISLOCAL*)stream->local)->id_mapregion != MAP_FAILED && 
	((SOLARISLOCAL*)stream->local)->id_mapregion != NULL) {
      /*
       * serial read - The index file has been mapped into memory; 
       * So, copy the entry directly from the mapped region; 
       */
      memcpy(ie, 
	     ((SOLARISLOCAL*)stream->local)->id_mapregion
	      + sizeof(INDEXHDR)
	      +((msgno -1)*INDEXENTRYSIZE), 
	     INDEXENTRYSIZE);
      return(_B_TRUE);
    }
  }
  /* read the entry */
  if (!read_entry(id,ie)) {
    return(_B_FALSE);
  } else {
    return(_B_TRUE);
  }
}

/*
 * Called when status flags changes happen/message has been
 * also quick-checksummed
 */
static boolean_t
update_index_entry(MAILSTREAM * stream, FILECACHE * fcache, MESSAGECACHE * elt)
{
  /* seek the entry */

  if (((SOLARISLOCAL*)stream->local)->id < 0) {
    return(_B_FALSE);
  }

  if (lseek(((SOLARISLOCAL*)stream->local)->id,
	    sizeof(INDEXHDR) + ((elt->msgno - 1) * INDEXENTRYSIZE),
	    SEEK_SET) < 0) {
    return(_B_FALSE); 
  }

  /* write the entry */
  if (!write_index_entry(stream, fcache, elt, _B_FALSE)) {
    return(_B_FALSE);
  }
  return(_B_TRUE);
}

static boolean_t
write_index_entry(MAILSTREAM	* stream,
		  FILECACHE 	* fcache, 
		  MESSAGECACHE 	* elt,
		  boolean_t 	  serial)
{
  INDEXENTRY	indexEntry;

  indexEntry.msgno = elt->msgno;

  if (fcache->status[8] == 'R') {
    indexEntry.seen = _B_TRUE;
  } else {
    indexEntry.seen = _B_FALSE;
  }

  if (fcache->status[21] == 'D') {
    indexEntry.deleted = _B_TRUE;
  } else {
    indexEntry.deleted = _B_FALSE;
  } 

  if (fcache->status[22] == 'F') {
    indexEntry.flagged = _B_TRUE;
  }else {
    indexEntry.flagged = _B_FALSE;
  }

  if (fcache->status[23] == 'A') {
    indexEntry.answered = _B_TRUE;
  }else {
    indexEntry.answered = _B_FALSE;
  }

  if (fcache->status[24] == 'T') {
    indexEntry.draft = _B_TRUE;
  }else {
    indexEntry.draft = _B_FALSE;
  }

  /* Convert from the X-Uid: entry */
  indexEntry.uid = atoi(&fcache->status[33]);

  indexEntry.headersize = fcache->headersize;
  indexEntry.rfc822_hdrsize = fcache->rfc822_hdrsize;
  indexEntry.bodysize = fcache->bodysize;
  indexEntry.body_offset = fcache->body_offset;
  indexEntry.status_offset = fcache->status_offset;
  indexEntry.real_mem_len = fcache->real_mem_len;
  indexEntry.rfc822_size = fcache->rfc822_size;
  indexEntry.seek_offset = fcache->seek_offset;
  indexEntry.chksum = fcache->chksum;
  indexEntry.status_chksum = fcache->status_chksum;
  indexEntry.chksum_size = fcache->chksum_size;
  indexEntry.status_len = fcache->status_len;
  indexEntry.quick_chksum_cycles = fcache->quick_chksum_cycles;
  indexEntry.hours = elt->hours, 
  indexEntry.minutes = elt->minutes, 
  indexEntry.seconds = elt->seconds;
  indexEntry.zoccident = elt->zoccident;
  indexEntry.zhours = elt->zhours;
  indexEntry.zminutes = elt->zminutes;
  indexEntry.day = elt->day;
  indexEntry.month = elt->month;
  indexEntry.year = elt->year;

  if (serial
      && (((SOLARISLOCAL*)stream->local)->id_mapregion != MAP_FAILED)	
      && (((SOLARISLOCAL*)stream->local)->id_mapregion != NULL)) {

      memcpy(((SOLARISLOCAL*)stream->local)->id_mapregion + sizeof(INDEXHDR)
	     +((indexEntry.msgno-1)*INDEXENTRYSIZE),
	     &indexEntry, INDEXENTRYSIZE);

  } else {
    if (safe_write(((SOLARISLOCAL*)stream->local)->id, &indexEntry,
	      sizeof(INDEXENTRY))!= sizeof(INDEXENTRY)){
      return(_B_FALSE);
    }
  }
  return (_B_TRUE);
}

/*
 * This routine does all of the work for create, recreate,
 * and updating
 */
static
update_index(MAILSTREAM *stream)
{
  INDEXHDR 	hdr;
  unsigned long i;
  unsigned long	k;
  unsigned long	l;
  unsigned long	m;
  off_t 	start;
  long 		time0;
  long		time1;
  long		time_elapsed;

  int 		id = ((SOLARISLOCAL*)stream->local)->id;

  lock_index(stream, id,LOCK_EX);

  /* seek the hdr */
  if (lseek(id, 0L, SEEK_SET), 0) {
    unlock_index(stream, id);
    close(id);
    ((SOLARISLOCAL*)stream->local)->id = -1;
    return(_B_FALSE);
  }

  /* read the hdr */
  if (sizeof(INDEXHDR) != read(id, &hdr, sizeof(INDEXHDR))) {
    unlock_index(stream, id);
    close(id);
    ((SOLARISLOCAL*)stream->local)->id = -1;/* this will force a create on next parse */
    return(_B_FALSE);
  }

  if (hdr.new_index) {  /* new index file */
    /*
     * If creating a NEW index file, then initialize,
     * and mmap it
     */
    off_t	indexFileSize;
    i = 0;
    indexFileSize = (sizeof(INDEXHDR) + (hdr.nmsgs*INDEXENTRYSIZE));

    /* Set new file to its size (NULLS) and map it */
    if (ftruncate(id, indexFileSize) == -1) {
      unlock_index(stream, id);
      close(id);
      ((SOLARISLOCAL*)stream->local)->id = -1;
      return(_B_FALSE);
    }
    mmap_index_file(stream, NIL);

  } else {
    /* It exists already so we do a straight forward update */
    i = hdr.nmsgs;

    /* seek to first record */
    l = INDEXENTRYSIZE;
    m =  sizeof(INDEXHDR);
    start = (i * l) + m;

    if (lseek(id, start, SEEK_SET) < 0) {
      unlock_index(stream, id);
      close(id);
      ((SOLARISLOCAL*)stream->local)->id = -1;
      return(_B_FALSE);
    }
  }

  for (k = i; k < stream->nmsgs; ++k) {
    FILECACHE 	*	f = ((SOLARISLOCAL*)stream->local)->msgs[k];
    MESSAGECACHE*	elt = mail_elt(stream, k+1);

    if (!write_index_entry(stream, f, elt, hdr.new_index)) {
      unmap_index_file(stream);
      unlock_index(stream, id);
      close(id);
      ((SOLARISLOCAL*)stream->local)->id = -1;	/* this will force a create on next parse */
      return(_B_FALSE);
    }

    time0 = elapsed_ms();
    if ((k % 71) == 0) {
      time1 = elapsed_ms();
      time_elapsed = time1 - time0;
      if (time_elapsed >= ACKTIME) {
	char small[SMALLBUFLEN];
	sprintf(small,"131 busy(UPDATING INDEX %d msgs)", k - i + 1);
	mm_log_stream(stream, "131 busy"); 
	mm_notify (stream, small, PARSE);
	time0 = elapsed_ms();    
      }
    }
  }
  unmap_index_file(stream);

  /* write updated header */
  if (!write_index_hdr(stream, id, 0L)) {
    unlock_index(stream, id);
    close(id);
    ((SOLARISLOCAL*)stream->local)->id = -1;
    return(_B_FALSE);
  } else {
    fsync(id);			/* sync index file */
    unlock_index(stream, id);
    return(_B_TRUE);
  }
}

/*
 * Parse found a NULL message, ie, only the "hidden from line"
 * Generate a message of the form:
 * "Date: DDD, dd mmm yy hh:mm:ss ZZZZ\n
 * To: user\n
 * Subject: (Empty Message)\n
 * Mime-Version: 1.0\n
 * Content-Type: text/plain; charset="us-ascii"\n
 * Content-Length: 0\n\n"
 * ASSUMPTION:
 *   m->internal == VALID "From ....\n" line
 *   We have cached the message with solaris_readmsg(..).
 *   Thus, we have at least a 32K buffer here.
 * Return: Pointer to NIL terminator of message.
 */
static char *
create_null_msg(FILECACHE *m, MAILSTREAM *stream)
{
  char date[SMALLBUFLEN];
  char *user;
  char *body;
  int rml;
  mail_stream_setNIL(stream);
  user = (char *)mail_parameters(stream,GET_USERNAME,(void *)NIL);
  mail_stream_unsetNIL(stream);
  if (!user) user = "nobody";
  rfc822_date(date);
  sprintf(m->header,
	  "Date: %s\n"
	  "To: %s\n"
	  "Subject: (Empty Message)\n"
	  "Mime-Version: 1.0\n"
	  "Content-Type: text/plain; charset=\"us-ascii\"\n"
	  "Content-Length: 0\n\n",
	  date,user);
  rml = strlen(m->internal);
  m->real_mem_len = m->headersize = rml;
  m->body_offset =  m->headersize;
  m->bodysize = 0;
  m->body = m->internal + rml;
  m->rfc822_size = rml + 7;	/* Add CR for each LF */
  m->hdr_offset = (m->header - m->internal);
  m->rfc822_hdrsize = rml - m->hdr_offset;
  m->status_offset = rml - 1;
  return m->internal + rml;
}
/*
 * solaris_mbox_access: Access to a mailbox[possible INBOX dependency]
 *
 *  return NIL if access is successfull
 *      if the initial access fails set group priviledges and try again
 *      only if the mailbox is INBOX.
 *  return 0 if all went well.
 *  set *err if failed. */ 
static int
solaris_mbox_access (MAILSTREAM *stream, char *mbox, int permissions, int *err)
{
  *err = errno = 0;
  if (access(mbox, permissions) == 0) return NIL; /* OK */
  else if (stream->local && ((SOLARISLOCAL*)stream->local)->inbox && 
	   stream->set_group_access &&
	   stream->clear_group_access) {
    if ((*stream->set_group_access)() && access(mbox,permissions))
      return 0;
    else (*stream->clear_group_access)();
  }
  *err = errno;
  return -1;
}
/*
 * solaris_access: Access to a path [INBOX independent]
 *
 *  return NIL if access is successfull
 *      if the initial access fails set group priviledges and try again
 *
 *  return 0 if all went well.
 *  set *err if failed. */ 
static int
solaris_path_access (MAILSTREAM *stream, char *path, int permissions, int *err)
{
  *err = errno = 0;
  if (access(path, permissions) == 0) return NIL; /* OK */
  else if (stream && (stream->stream_status & S_OPENED) && 
	   stream->set_group_access && stream->clear_group_access) {
    if ((*stream->set_group_access)() && (access(path,permissions) == 0))
      return 0;
    else {
      *err = errno;
      (*stream->clear_group_access)();
    }
  } else *err = errno;		/* access err number */
  return -1;
}
/*
 * solaris_clear_group_permissions:
 * if INBOX and the callbacks exist, then
 * clear the group permissions */
static int
solaris_clear_group_permissions(MAILSTREAM *stream)
{
  if (stream && (stream->stream_status & S_OPENED) 
	&& stream->set_group_access && stream->clear_group_access) 
    return (*stream->clear_group_access)();
  else
    return(_B_TRUE);
}

static int
solaris_lockf(MAILSTREAM * stream, char * mbox)
{
  int success;
  int fd;
  int err;
  int retry;
  /*
   * check access
   */
  if (solaris_mbox_access(stream,mbox,W_OK,&err) < 0) {
    return(_B_FALSE);
  }

  /*
   * open write
   */
  if ((fd = open(mbox,O_RDWR,NIL)) < 0) {
    return _B_FALSE;
  }

  /*
   * try lockf()
   */
  retry = QUICK_TIMEOUT;
  success = _B_FALSE;
  ((SOLARISLOCAL*)stream->local)->fd = -1;
  while (retry--) {
    if (!solaris_flock(stream, fd, LOCK_EX|LOCK_NB)) {
      sleep(1);
    } else {
      success = _B_TRUE;
      ((SOLARISLOCAL*)stream->local)->fd = fd;
      break;
    }
  }
  return(success);
}

/*
 * return true if nfs mounted on solaris
 */
static boolean_t
solaris_nfs_mounted(int fd)
{
  struct statvfs	sbuf;
  boolean_t		isNfs = _B_FALSE;

  if (fstatvfs(fd, &sbuf) == 0) {
    if (strcasecmp(sbuf.f_basetype, "nfs") == 0) {
      isNfs = _B_TRUE;
    }
  }
  return(isNfs);
}

/*
 * See if the string s contains any NULL chars
 */
static int read_null_chars(char *s,long i,char **nullptr)
{
  register u_char *dp = (u_char *)s;
  register long k,u = i;
  for (k = 0; k < u; ++k) {
    if (*dp++ == '\0')  {
      *nullptr = (char *)dp - 1;
      return(_B_TRUE);
    }
  }
  return NIL;
}

/*
 * replace found nulls with '\n' so if they are between messages,
 * then the "\nFrom .." can be found */
static void replace_nulls(char *nptr,unsigned long n)
{
  register char *cp = nptr;
  register unsigned long n1 = n;

  while (n-- > 0) {
    if (*cp == '\0') *cp++ = '\n ';
    else ++cp;
  }
}

/* mmapping of index file on startup
 *
 * This function is used to mmap an index file into a region of memory.
 *  Stream data modifications:
 *    stream->id_mapregion contains the address of the first byte of the 
 *    region, or
 * 	a -1 if the mmap() failed.
 *    stream->id_mapregion_size contains the size of the region mapped
 *  These values are used by read_index_entry on sequential reads only 
 *  to determine whether to copy an index entry from the mapped region 
 *  as opposed to issuing a system read() call to read the entry.
 */
static void
mmap_index_file(MAILSTREAM *stream, int readOnly)
{
#if (_POSIX_C_SOURCE >= 199506L)
  pthread_once(&solarisOnceVar, solaris_once);
#else
  if (solarisArch[0] == '\0') { /* if Not yet initalized */
    solaris_once();
  }
#endif

  if (((SOLARISLOCAL*)stream->local)->id_mapregion == NULL) {
    struct stat statbuf;

    if (fstat(((SOLARISLOCAL*)stream->local)->id, &statbuf) == -1) {
	/* does not exist */
      ((SOLARISLOCAL*)stream->local)->id_mapregion = MAP_FAILED;

    } else {
      ((SOLARISLOCAL*)stream->local)->id_mapregion_size = statbuf.st_size + 
	(pagesize - (statbuf.st_size % pagesize)); /* page multiples */

      if (readOnly) {
	((SOLARISLOCAL*)stream->local)->id_mapregion
	  = (char *) mmap(0, 
			  ((SOLARISLOCAL*)stream->local)->id_mapregion_size, 
			  PROT_READ, 
			  MAP_PRIVATE|MAP_NORESERVE, 
			  ((SOLARISLOCAL*)stream->local)->id, (off_t)0);
      } else {
	((SOLARISLOCAL*)stream->local)->id_mapregion
	  = (char *) mmap(0, 
			  ((SOLARISLOCAL*)stream->local)->id_mapregion_size, 
			  PROT_READ|PROT_WRITE,
			  MAP_SHARED|MAP_NORESERVE, 
			  ((SOLARISLOCAL*)stream->local)->id, (off_t)0);
      }
      /* mapp successful */
      if (((SOLARISLOCAL*)stream->local)->id_mapregion != MAP_FAILED) 
	(void) madvise((caddr_t)((SOLARISLOCAL*)stream->local)->id_mapregion, 
		       ((SOLARISLOCAL*)stream->local)->id_mapregion_size, 
		       MADV_SEQUENTIAL);
    }
  }
  return;
}

/*
 * This function is used to unmap an index file that was previously mapped
 * into a region of memory via a call to mmap_index_file. You *must* call
 * this function before closing the index file if a region was previously
 * mapped; otherwise, the mapping will persist in memory until the process
 * exits. While this would not use any VM permanently, it would use up a
 * portion of the virtual address space.
 */
static void
unmap_index_file(MAILSTREAM *stream)
{
  /* Only unmap the region if it was really mapped */
  if (((SOLARISLOCAL*)stream->local)->id_mapregion != MAP_FAILED && 
      ((SOLARISLOCAL*)stream->local)->id_mapregion != NULL) {

	msync(((SOLARISLOCAL*)stream->local)->id_mapregion,
	      ((SOLARISLOCAL*)stream->local)->id_mapregion_size,
	      MS_SYNC);

	/* unmap the region */
	munmap(((SOLARISLOCAL*)stream->local)->id_mapregion,
	       ((SOLARISLOCAL*)stream->local)->id_mapregion_size);

	/* reset region address */
	((SOLARISLOCAL*)stream->local)->id_mapregion = NULL;

	/* reset region size */
	((SOLARISLOCAL*)stream->local)->id_mapregion_size = 0;
    }
    return;
}

/* mmapping of mailbox on startup
 *
 * This function is used to mmap a mailbox into a region of memory.
 *   Stream data modifications:
 *   	stream->mbox_mapregion contains the address of the first byte of the
 * 	region, or a MAP_FAILED if the mmap() failed.
 * stream->mbox_mapregion_size contains the size of the region mapped
 * These values are used by functions that read the mailbox on initial
 * startup, and help prevent a multitude of lseek()/read() kernel operations
 * to be performed, improving performance both locally and over NFS.
 */
static void
mmap_mbox(MAILSTREAM *stream)
{
#if (_POSIX_C_SOURCE >= 199506L)
  pthread_once(&solarisOnceVar, solaris_once);
#else
  if (solarisArch[0] == '\0') { /* if Not yet initalized */
    solaris_once();
  }
#endif

  if (((SOLARISLOCAL*)stream->local)->mbox_mapregion == NULL) {
    struct stat statbuf;

    if (stat(stream->mailbox, &statbuf) == -1) {	
      ((SOLARISLOCAL*)stream->local)->mbox_mapregion = MAP_FAILED;

    } else {
      ((SOLARISLOCAL*)stream->local)->mbox_mapregion_size =
	statbuf.st_size + (pagesize - (statbuf.st_size % pagesize));
      ((SOLARISLOCAL*)stream->local)->mbox_mapregion =
	(char *) mmap(0, ((SOLARISLOCAL*)stream->local)->mbox_mapregion_size, PROT_READ,
		      MAP_PRIVATE|MAP_NORESERVE, ((SOLARISLOCAL*)stream->local)->fd, (off_t)0);	
      if (((SOLARISLOCAL*)stream->local)->mbox_mapregion != MAP_FAILED)
	(void) madvise((caddr_t)((SOLARISLOCAL*)stream->local)->mbox_mapregion,
		       ((SOLARISLOCAL*)stream->local)->mbox_mapregion_size, MADV_SEQUENTIAL);
    }
  }
  return;
}

/*
 * This function is used to unmap a mailbox that was previously mapped into
 * a region of memory via a call to mmap_mbox. You *must* call this function
 * before closing the mailbox if a region was previously mapped; otherwise,
 * the mapping will persist in memory until the process exits. While this
 * would not use any VM permanently, it would use up a portion of the
 * processes virtual address space permanently.
 */
static void
unmap_mbox(MAILSTREAM *stream)
{		
					
  /* Only unmap the region if it was really mapped */
  if (stream && ((SOLARISLOCAL*)stream->local) &&
      ((SOLARISLOCAL*)stream->local)->mbox_mapregion != MAP_FAILED &&
      ((SOLARISLOCAL*)stream->local)->mbox_mapregion != NULL) {

    /* unmap the region */
    munmap(((SOLARISLOCAL*)stream->local)->mbox_mapregion,
	   ((SOLARISLOCAL*)stream->local)->mbox_mapregion_size); 
    
    /* reset region address */
    ((SOLARISLOCAL*)stream->local)->mbox_mapregion = NULL;
    /* reset region size */
    ((SOLARISLOCAL*)stream->local)->mbox_mapregion_size = 0;
  }
}

/*
 * solaris globals initializer
 * Accepts: a MAIL stream.
 * Returns: None.
 */
void
solaris_global_init(MAILSTREAM *stream)
{
  SOLARIS_GLOBALS *sg = (SOLARIS_GLOBALS *)fs_get(sizeof(SOLARIS_GLOBALS));

  sg->solaris_fromwidget = NIL;
  sg->lock_timeout = DEFAULTLOCKTIMEOUT;
  stream->solaris_globals = sg;
}

/*
 * solaris globals free
 * Accepts: a MAIL stream.
 * Returns: None.
 */
void
solaris_global_free(MAILSTREAM *stream)
{
  if (stream->solaris_globals) {
     fs_give ((void **) &stream->solaris_globals);
  }
}
