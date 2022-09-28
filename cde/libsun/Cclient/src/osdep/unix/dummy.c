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
 * Last Edited: 12-09-96	
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
 * Edit log:
 * [09-25-96 clin]: readdir_r is used to ensure MT-Safe.
 * [10-21-96 clin]: passing stream in routine:
 *					dummy_parameters,
 *					dummy_valid,
 *					dummy_file,
 *					mail_parameters,
 * [10-25-96 clin]: New routine: dummy_global_init.
 * [10-31-96 clin]: OK to pass a NIL in mail_open. 
 * [11-08-96 clin]: pass a stream in all mm_log calls.  
 * [11-19-96 clin]: pass a stream in all fatal calls.  
 * [12-09-96 clin]: Check for dot files in dummy_list_work.
 * [12-11-96 clin]: Fix "./" bug in last dot file check. 
 * [01-01-97 clin]: Add a new routine dummy_global_free. 
 * [01-02-97 clin]: Use mail_stream_setNIL to pass a NIL to mail_parameters.
 * [01-07-97 clin]: Remove the last first parameters from mail_open.
 * [01-22-97 clin]: Test stream_status in dummy_open.
 *		    Re-structured dummy_ping.
 */


#include <stdio.h>
#include <ctype.h>
#include <errno.h>
extern int errno;		/* just in case */
#include "mail.h"
#include "os_sv5.h"
#include <pwd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "dummy.h"
#include "misc.h"
#define  NAME_MAX (_PC_NAME_MAX + 1 )

/* Dummy routines */


/* Driver dispatch used by MAIL */

DRIVER dummydriver = {
  "dummy",			/* driver name */
  DR_LOCAL|DR_MAIL|DR_NOSTREAM,	/* driver flags(does NIL stream ops) */
  (DRIVER *) NIL,		/* next driver */
  dummy_valid,			/* mailbox is valid for us */
  dummy_parameters,		/* manipulate parameters */
  dummy_scan,			/* scan mailboxes */
  dummy_list,			/* list mailboxes */
  dummy_lsub,			/* list subscribed mailboxes */
  NIL,				/* subscribe to mailbox */
  NIL,				/* unsubscribe from mailbox */
  dummy_create,			/* create mailbox */
  dummy_delete,			/* delete mailbox */
  dummy_rename,			/* rename mailbox */
  NIL,				/* status  */
  dummy_open,			/* open mailbox */
  dummy_close,			/* close mailbox */
  dummy_fetchfast,		/* fetch message "fast" attributes */
  dummy_fetchflags,		/* fetch message flags */
  dummy_fetchstructure,		/* fetch message structure */
  dummy_fetchheader,		/* fetch message header only */
  dummy_fetchtext,		/* fetch message body only */
  dummy_fetchbody,		/* fetch message body section */
  dummy_uid,			/* unique identifier */
  dummy_setflag,		/* set message flag */
  dummy_clearflag,		/* clear message flag */
  dummy_search,			/* search for message based on criteria */
  dummy_ping,			/* ping mailbox to see if still alive */
  dummy_check,			/* check for new messages */
  dummy_expunge,		/* expunge deleted messages */
  dummy_copy,			/* copy messages to another mailbox */
  dummy_append,			/* append string message to mailbox */
  dummy_gc,			/* garbage collect stream */
  /* SUN imap4 */
  dummy_fabricate_from,          /* fabricate a from line */
  dummy_set1flag,		 /* set message flag */
  dummy_clear1flag,		 /* clear message flag */
  /* These do nothing */
  dummy_sunversion,
  dummy_disconnect,
  dummy_echo,
  dummy_clear_iocount,
  dummy_urgent,
  dummy_fetchshort,
  dummy_checksum,
  dummy_validchecksum,
  dummy_sync_msgno,
  NIL,
  NIL,
  NIL,
  dummy_global_init,            /* init dummy globals */
  dummy_global_free             /* free dummy globals */
};

				/* prototype stream */
MAILSTREAM dummyproto = {&dummydriver};

/* Dummy validate mailbox
 * Accepts: mailbox name
 * Returns: our driver if name is valid, NIL otherwise
 */

DRIVER *dummy_valid (char *name, MAILSTREAM *stream)
{
  char *s,tmp[MAILTMPLEN];
  struct stat sbuf;
				/* must be valid local mailbox */
  return (name && *name && (*name != '{') && (*name != '#') &&
	  (s = mailboxfile (tmp,name,stream)) && 
	   (!*s || !stat (s,&sbuf))) ?
	    &dummydriver : NIL;
}


/* Dummy manipulate driver parameters
 * Accepts: function code
 *	    function-dependent value
 * Returns: function-dependent return value
 */

void *dummy_parameters (MAILSTREAM *stream, long function,void *value)
{
  return NIL;
}

/* Dummy scan mailboxes
 * Accepts: mail stream
 *	    reference
 *	    pattern to search
 *	    string to scan
 */

void dummy_scan (MAILSTREAM *stream,char *ref,char *pat,char *contents)
{
  char *s,test[MAILTMPLEN],file[MAILTMPLEN];
  long i = 0;
				/* get canonical form of name */
  if (dummy_canonicalize (test,ref,pat)) {
				/* found any wildcards? */
    if (s = strpbrk (test,"%*")) {
				/* yes, copy name up to that point */
      strncpy (file,test,i = s - test);
      file[i] = '\0';		/* tie off */
    }
    else strcpy (file,test);	/* use just that name then */
    if (s = strrchr (file,'/')){/* find directory name */
      *++s = '\0';		/* found, tie off at that point */
      s = file;
    }
				/* silly case */
    else if ((file[0] == '~') || (file[0] == '#')) s = file;
				/* do the work */
    dummy_list_work (stream,s,test,contents,0);
    if (pmatch ("INBOX",test))	/* always an INBOX */
      dummy_listed (stream,NIL,"INBOX",LATT_NOINFERIORS,contents);
  }
}

/* Dummy list mailboxes
 * Accepts: mail stream
 *	    reference
 *	    pattern to search
 */

void dummy_list (MAILSTREAM *stream,char *ref,char *pat)
{
  dummy_scan (stream,ref,pat,NIL);
}


/* Dummy list subscribed mailboxes
 * Accepts: mail stream
 *	    reference
 *	    pattern to search
 */

void dummy_lsub (MAILSTREAM *stream,char *ref,char *pat)
{
  void *sdb = NIL;
  char *s,test[MAILTMPLEN];
				/* get canonical form of name */
  if (dummy_canonicalize (test,ref,pat) && (s = sm_read (&sdb,stream)))
    do if (((*s != '#') || ((s[1] == 'f') && (s[2] == 't') && (s[3] == 'p') &&
			    (s[4] == '/'))) && (*s != '{') &&
	   pmatch_full (s,test,'/')) mm_lsub (stream,'/',s,NIL);
  while (s = sm_read (&sdb, stream)); /* until no more subscriptions */
}

/* Dummy list mailboxes worker routine
 * Accepts: mail stream
 *	    directory name to search
 *	    search pattern
 *	    string to scan
 *	    search level
 */

void dummy_list_work (MAILSTREAM *stream,char *dir,char *pat,char *contents,
		      long level)
{
  DIR		*	dp;

  struct direct * 	dirEntry;
  struct direct	*	res_dir;

  struct stat sbuf;
  char tmp[MAILTMPLEN];
  unsigned short stream_flag;
				/* punt if bogus name */
  if (!mailboxdir (tmp,dir,NIL,stream)) return;
  if (dp = opendir (tmp)) {	/* do nothing if can't open directory */
				/* list it if not at top-level */
				/* get the buffer size for readdir_r. */
	long dsize = sizeof(struct dirent ) + pathconf(tmp, _PC_NAME_MAX)+1;  
  	int list_dot = 0;
	char *last_slash;


	/* Pattern testing:
         *   Look for explicit dotted file searches:
         *     1. "<chars>/.<chars><search_type>"
         *     2. ".<chars><search-type>"
         *     <search-type> := % | *
         */
        if (last_slash = strrchr(pat, '/')) {
          if (last_slash[1] == '.') list_dot = 1;
        } else                  /* NO "/" in pattern, see if "." search */
          if (*pat == '.') list_dot = 1;

    if (!level && dir && pmatch_full (dir,pat,'/'))
      dummy_listed (stream,'/',dir,LATT_NOSELECT,contents);
				
      res_dir = (struct dirent *)fs_get(dsize);
				/* scan directory, ignore . and .. */
    if (!dir || dir[strlen (dir) - 1] == '/') 

#if (_POSIX_C_SOURCE - 0 >= 199506L)
	while (readdir_r (dp, res_dir, &dirEntry) == 0)
#else
	while ((dirEntry = readdir_r (dp, res_dir)) != NULL)
#endif

      if ((dirEntry->d_name[0] != '.') ||
	  (dirEntry->d_name[1] && ((dirEntry->d_name[1] != '.') || dirEntry->d_name[2]))) {
				/* Sun IMAP4. For caller i/o idle loop */
	mm_io_callback(stream,0L,CBTIMEOUT);
				/* see if we can get info about name */
	if (mailboxdir (tmp,dir,dirEntry->d_name, stream) && !stat (tmp,&sbuf)) {
				/* now make name we'd return */
	  if (dir) sprintf (tmp,"%s%s",dir,dirEntry->d_name);
	  else strcpy (tmp,dirEntry->d_name);
				/* only interested in file type */
	  switch (sbuf.st_mode &= S_IFMT) {
	  case S_IFDIR:		/* directory? */
		/* Check for dot directoris. -clin */
	  if (dirEntry->d_name[0] == '.' && !list_dot)
                break;
	    if (pmatch_full (tmp,pat,'/')) {
	      dummy_listed (stream,'/',tmp,LATT_NOSELECT,contents);
	      strcat (tmp,"/");	/* set up for dmatch call */
	    }
				/* try again with trailing / */
	    else if (pmatch_full (strcat (tmp,"/"),pat,'/'))
	      dummy_listed (stream,'/',tmp,LATT_NOSELECT,contents);
		mail_stream_setNIL(stream); 
	    if (dmatch (tmp,pat,'/') &&
		(level < (long) mail_parameters (stream,GET_LISTMAXLEVEL,NIL)))
	      dummy_list_work (stream,tmp,pat,contents,level+1);
		mail_stream_unsetNIL(stream); 
	    break;
	  case S_IFREG:		/* ordinary name */
		/* Check for dot files. -clin */
	    if (dirEntry->d_name[0] == '.' && !list_dot)
                break;
	    if (pmatch_full (tmp,pat,'/')) {
	      long atts = LATT_NOINFERIORS;
	      /* if modified more recently than accessed, we give a
               * hint that there may be new mail. WJY 25-july-96 */
	      if (sbuf.st_atime < sbuf.st_mtime) atts |= LATT_MARKED;
	      else if (sbuf.st_atime != sbuf.st_mtime)
		atts |= LATT_UNMARKED;
	      dummy_listed (stream,'/',tmp,atts,contents);
	    }
	    break;
	  }
	}
      }
    fs_give((void **) &res_dir);		/* need to give up the block */
    closedir (dp);				/* all done, flush directory */
  }
}

/*
 * Mailbox found
 * Accepts: hierarchy delimiter
 *	    mailbox name
 *	    attributes
 *	    contents to search before calling mm_list()
 */
#define BUFSIZE (4*MAILTMPLEN)
void
dummy_listed(MAILSTREAM * stream,
	     char 	  delimiter,
	     char 	* name,
	     long 	  attributes,
	     char 	* contents)
{
  struct stat 	sbuf;
  int 		fd;
  long 		csiz;
  long		ssiz;
  long		bsiz;
  char 	*	buf;
  char		tmp[MAILTMPLEN];

  if (contents) {		/* want to search contents? */
    
    /* forget it if can't select or open */
    if ((attributes & LATT_NOSELECT)
	|| !(csiz = strlen(contents))
	|| stat(dummy_file(tmp, name, stream), &sbuf)
	|| (csiz > sbuf.st_size)
	|| ((fd = open(tmp, O_RDONLY, 0)) < 0)) {
      return;
    }

    /* get buffer including slop */    
    buf = (char *) fs_get(BUFSIZE + (ssiz = 4 * ((csiz / 4) + 1)) + 1);
    memset(buf, '\0', ssiz);	/* no slop area the first time */
    while (sbuf.st_size) {	/* until end of file */
      read(fd, buf + ssiz, (bsiz = min(sbuf.st_size, BUFSIZE)));
      if (search(buf, bsiz+ssiz, contents, csiz)) {
	break;
      }
      memcpy(buf, buf+BUFSIZE, ssiz);
      sbuf.st_size -= bsiz;	/* note that we read that much */
    }
    fs_give((void **)&buf);	/* flush buffer */
    close(fd);			/* finished with file */
    if (!sbuf.st_size) {
      return;	/* not found */
    }
  }
  /* notify main program */
  mm_list (stream,delimiter,name,attributes);
  return;
}

/* Dummy create mailbox
 * Accepts: mail stream
 *	    mailbox name to create
 *	    driver type to use
 * Returns: T on success, NIL on failure
 */

long dummy_create (MAILSTREAM *stream,char *mailbox)
{
  struct stat sbuf;
  char c,*s,mbx[MAILTMPLEN];
  long ret = NIL;
  int fd;
  int wantdir = (s = strrchr (dummy_file (mbx,mailbox, stream),'/')) && !s[1];
  mail_stream_setNIL(stream);

  if (wantdir) *s = '\0';	/* flush trailing delimiter for directory */
  if (s = strrchr (mbx,'/')) {	/* found superior to this name? */
    c = *++s;			/* remember first character of inferior */
    *s = '\0';			/* tie off to get just superior */
				/* name doesn't exist, create it */
    if ((stat (mbx,&sbuf) || ((sbuf.st_mode & S_IFMT) != S_IFDIR)) &&
	!dummy_create (stream,mbx)) return NIL;
    *s = c;			/* restore full name */
  }
  if (wantdir)			/* want to create directory? */
    ret = !mkdir (mbx,(int) mail_parameters (stream,GET_DIRPROTECTION,NIL));
				/* create file */
  else if ((fd = open (mbx,O_WRONLY|O_CREAT|O_EXCL,
		       (int) mail_parameters(stream,GET_MBXPROTECTION,NIL))) >= 0)
    ret = !close (fd);		/* close file */
  	mail_stream_unsetNIL(stream);

  if (!ret) {			/* error? */
    char tmp[MAILTMPLEN];
    sprintf (tmp,"Can't create mailbox node %s: %s",mbx,strerror (errno));
    mm_log (tmp, ERROR, stream);
    return NIL;
  }
  return ret;			/* return status */
}

/* Dummy delete mailbox
 * Accepts: mail stream
 *	    mailbox name to delete
 * Returns: T on success, NIL on failure
 */

long dummy_delete (MAILSTREAM *stream,char *mailbox)
{
  struct stat sbuf;
  char *s,tmp[MAILTMPLEN];
				/* no trailing / (workaround BSD kernel bug) */
  if ((s = strrchr (dummy_file (tmp,mailbox,stream),'/')) && !s[1]) *s = '\0';
  if (stat (tmp,&sbuf) || ((sbuf.st_mode & S_IFMT) == S_IFDIR) ?
      rmdir (tmp) : unlink (tmp)) {
    sprintf (tmp,"Can't delete mailbox %s: %s",mailbox,strerror (errno));
    mm_log (tmp, ERROR, stream);
    return NIL;
  }
  return T;			/* return success */
}


/* Mail rename mailbox
 * Accepts: mail stream
 *	    old mailbox name
 *	    new mailbox name
 * Returns: T on success, NIL on failure
 */

long dummy_rename (MAILSTREAM *stream,char *old,char *new)
{

  struct stat sbuf;
  char c,*s,tmp[MAILTMPLEN],mbx[MAILTMPLEN];
  long ret = NIL;
  int fd;
				/* found superior to destination name? */
  if (s = strrchr (dummy_file (mbx,new,stream),'/')) {
    c = *++s;			/* remember first character of inferior */
    *s = '\0';			/* tie off to get just superior */
				/* name doesn't exist, create it */
    if ((stat (mbx,&sbuf) || ((sbuf.st_mode & S_IFMT) != S_IFDIR)) &&
	!dummy_create (stream,mbx)) return NIL;
    *s = c;			/* restore full name */
  }
  if (rename (dummy_file (tmp,old,stream),mbx)) {
    sprintf (tmp,"Can't rename mailbox %s to %s: %s",old,new,strerror (errno));
    mm_log (tmp, ERROR, stream);
    return NIL;
  }
  return T;			/* return success */
}

/* Dummy open
 * Accepts: stream to open
 * Returns: stream on success, NIL on failure
 */

MAILSTREAM *dummy_open (MAILSTREAM *stream)
{
  int fd;
  char err[MAILTMPLEN],tmp[MAILTMPLEN];
  struct stat sbuf;
				/* OP_PROTOTYPE call */
  if (stream->stream_status & S_PROTOTYPE) {
	stream->dtb = (DRIVER *)&dummyproto;
	return &dummyproto;
  }
  err[0] = '\0';		/* no error message yet */
				/* can we open the file? */
  if ((fd = open (dummy_file (tmp,stream->mailbox,stream),O_RDONLY,NIL)) < 0) {
				/* no, error unless INBOX */
    if (strcmp (ucase (strcpy (tmp,stream->mailbox)),"INBOX"))
      sprintf (err,"%s: %s",strerror (errno),stream->mailbox);
  }
  else {			/* file had better be empty then */
    fstat (fd,&sbuf);		/* sniff at its size */
    close (fd);
    if (sbuf.st_size)		/* bogus format if non-empty */
      sprintf (err,"%s (file %s) is not in valid mailbox format",
	       stream->mailbox,tmp);
  }
  if (!stream->silent) {	/* only if silence not requested */
    if (err[0]) mm_log (err,ERROR, stream);
    else {
      mail_exists (stream,0);	/* say there are 0 messages */
      mail_recent (stream,0);	/* and certainly no recent ones! */
    }
  }
  return err[0] ? NIL : stream;	/* return success if no error */
}


/* Dummy close
 * Accepts: MAIL stream
 *	    options
 */

void dummy_close (MAILSTREAM *stream,long options)
{
				/* return silently */
}

/* Dummy fetch fast information
 * Accepts: MAIL stream
 *	    sequence
 *	    option flags
 */

void dummy_fetchfast (MAILSTREAM *stream,char *sequence,long flags)
{
  fatal ("Impossible dummy_fetchfast", stream);
}


/* Dummy fetch flags
 * Accepts: MAIL stream
 *	    sequence
 *	    option flags
 */

void dummy_fetchflags (MAILSTREAM *stream,char *sequence,long flags)
{
  fatal ("Impossible dummy_fetchflags", stream);
}


/* Dummy fetch envelope
 * Accepts: MAIL stream
 *	    message # to fetch
 *	    pointer to return body
 *	    option flags
 * Returns: envelope of this message, body returned in body value
 */

ENVELOPE *dummy_fetchstructure (MAILSTREAM *stream,unsigned long msgno,
				BODY **body,long flags)
{
  fatal ("Impossible dummy_fetchstructure",stream);
  return NIL;
}


/* Dummy fetch message header
 * Accepts: MAIL stream
 *	    message # to fetch
 *	    list of headers
 *	    pointer to returned length
 *	    options
 * Returns: message header in RFC822 format
 */

char *dummy_fetchheader (MAILSTREAM *stream,unsigned long msgno,
			 STRINGLIST *lines,unsigned long *len,long flags)
{
  fatal ("Impossible dummy_fetchheader",stream);
  return NIL;
}

/* Dummy fetch message text (body only)
 * Accepts: MAIL stream
 *	    message # to fetch
 *	    pointer to returned length
 *	    options
 * Returns: message text in RFC822 format
 */

char *dummy_fetchtext (MAILSTREAM *stream,unsigned long msgno,
		       unsigned long *len,long flags)
{
  fatal ("Impossible dummy_fetchtext",stream);
  return NIL;
}


/* Dummy fetch message body as a structure
 * Accepts: Mail stream
 *	    message # to fetch
 *	    section specifier
 *	    pointer to returned length
 *	    options
 * Returns: pointer to section of message body
 */

char *dummy_fetchbody (MAILSTREAM *stream,unsigned long msgno,char *sec,
		       unsigned long *len,long flags)
{
  fatal ("Impossible dummy_fetchbody",stream);
  return NIL;
}


/* Dummy fetch UID
 * Accepts: Mail stream
 *	    message # to fetch
 * Returns: unique identifier
 */

unsigned long dummy_uid (MAILSTREAM *stream,unsigned long msgno)
{
  fatal ("Impossible dummy_uid",stream);
  return NIL;
}

/* Dummy set flag
 * Accepts: MAIL stream
 *	    sequence
 *	    flag(s)
 *	    options
 */

void dummy_setflag (MAILSTREAM *stream,char *sequence,char *flag,long flags)
{
  fatal ("Impossible dummy_setflag",stream);
}

int dummy_set1flag (MAILSTREAM *stream,char *sequence,char *flag,long flags)
{
  fatal ("Impossible dummy_setflag",stream);
  return(0);
}

/* Dummy clear flag
 * Accepts: MAIL stream
 *	    sequence
 *	    flag(s)
 *	    options
 */

void dummy_clearflag (MAILSTREAM *stream,char *sequence,char *flag,long flags)
{
  fatal ("Impossible dummy_clearflag",stream);
}

int dummy_clear1flag (MAILSTREAM *stream,char *sequence,char *flag,long flags)
{
  fatal ("Impossible dummy_clearflag",stream);
  return(0);
}

/* Dummy search for messages
 * Accepts: MAIL stream
 *	    character set
 *	    search program
 *	    search flags
 */

void dummy_search (MAILSTREAM *stream,char *charset,SEARCHPGM *pgm,long flags)
{
				/* return silently */
}


/* Dummy ping mailbox
 * Accepts: MAIL stream
 * Returns: T if stream alive, else NIL
 * No-op for readonly files, since read/writer can expunge it from under us!
 */

long dummy_ping (MAILSTREAM *stream)
{
  char tmp[MAILTMPLEN];
  MAILSTREAM *test = mail_stream_create(stream->userdata,
			NIL,NIL,NIL,NIL); /*create a new stream. -clin */
  test = mail_open (test,stream->mailbox,OP_PROTOTYPE);
				/* swap streams if looks like a new driver */
  if (test && (test->stream_status & S_OPENED) &&
	 (test->dtb != stream->dtb)) {
	mail_stream_flush(test); /* don't need test anymore. -clin */
    	test = mail_open (stream,strcpy (tmp,stream->mailbox),NIL);
	}

  if ( test->stream_status & S_OPENED ) {
  	if (test != stream ) /* didn't call the 2nd mail_open. */
	mail_stream_flush(test); 
  	return T;
	}
  else  {
  	if (test != stream ) /* didn't call the 2nd mail_open. */
	mail_stream_flush(test); 
	return NIL;
	}
	
}


/* Dummy check mailbox
 * Accepts: MAIL stream
 * No-op for readonly files, since read/writer can expunge it from under us!
 */

void dummy_check (MAILSTREAM *stream)
{
  dummy_ping (stream);		/* invoke ping */
}


/* Dummy expunge mailbox
 * Accepts: MAIL stream
 */

void dummy_expunge (MAILSTREAM *stream)
{
				/* return silently */
}

/* Dummy copy message(s)
 * Accepts: MAIL stream
 *	    sequence
 *	    destination mailbox
 *	    options
 * Returns: T if copy successful, else NIL
 */

long dummy_copy (MAILSTREAM *stream,char *sequence,char *mailbox,long options)
{
  fatal ("Impossible dummy_copy",stream);
  return NIL;
}


/*
 * Dummy append message string
 * Accepts: mail stream
 *	    destination mailbox
 *	    optional flags
 *	    optional date
 *	    stringstruct of message to append
 * Returns: _B_TRUE on success, _B_FALSE on failure
 */
long
dummy_append(MAILSTREAM * stream,
	     char 	* mailbox,
	     char 	* flags,
	     char 	* date,
	     char	* xUnixFrom,
	     STRING 	* message)
{
  struct stat 	sbuf;
  int 		fd = -1;
  int 		er;
  char 		tmp[MAILTMPLEN];

  if ((strcmp(ucase(strcpy(tmp, mailbox)),"INBOX"))
      && ((fd = open(dummy_file(tmp, mailbox, stream), O_RDONLY, NIL)) < 0)) {
    if ((er = errno) == ENOENT) {/* failed, was it no such file? */
      mm_notify(stream, "[TRYCREATE] Must create mailbox before append", NIL);
      return(_B_FALSE);
    }
    sprintf(tmp, "%s: %s", strerror(er), mailbox);
    mm_log(tmp, ERROR, stream);	/* pass up error */
    return(_B_FALSE);		/* always fails */

  } else if (fd >= 0) {		/* found file? */
    fstat(fd, &sbuf);		/* get its size */
    close(fd);			/* toss out the fd */
    if (sbuf.st_size > 0) {	/* non-empty file? */
      sprintf(tmp, "Indeterminate mailbox format: %s", mailbox);
      mm_log(tmp, ERROR, stream);
      return(_B_FALSE);
    }
  }
  return (*default_proto(stream)->dtb->append)(stream,
					       mailbox,
					       flags,
					       date,
					       xUnixFrom,
					       message);
}

/* Dummy garbage collect stream
 * Accepts: mail stream
 *	    garbage collection flags
 */

void dummy_gc (MAILSTREAM *stream,long gcflags)
{
				/* return silently */
}


/* Dummy mail generate file string
 * Accepts: temporary buffer to write into
 *	    mailbox name string
 * Returns: local file string
 */

char *dummy_file (char *dst,char *name, MAILSTREAM *stream)
{
  char *s = mailboxfile (dst,name,stream);
				/* return our standard inbox */
  return (s && !*s) ? strcpy (dst,sysinbox (stream)) : s;
}


/* Dummy canonicalize name
 * Accepts: buffer to write name
 *	    reference
 *	    pattern
 * Returns: T if success, NIL if failure
 */

long dummy_canonicalize (char *tmp,char *ref,char *pat)
{
  char *s;
  if (ref) {			/* preliminary reference check */
    if (*ref == '{') return NIL;/* remote reference not allowed */
    else if (!*ref) ref = NIL;	/* treat empty reference as no reference */
  }
  switch (*pat) {
  case '#':			/* namespace name */
    if ((pat[1] == 'f') && (pat[2] == 't') && (pat[3] == 'p') &&
	(pat[4] == '/')) strcpy (tmp,pat);
    else return NIL;		/* unknown namespace */
    break;
  case '{':			/* remote names not allowed */
    return NIL;
  case '/':			/* rooted name */
  case '~':			/* home directory name */
    if (!ref || (*ref != '#')) {/* non-namespace reference? */
      strcpy (tmp,pat);		/* yes, ignore */
      break;
    }
				/* fall through */
  default:			/* apply reference for all other names */
    if (!ref) strcpy (tmp,pat);	/* just copy if no namespace */
    else if ((*ref != '#') || ((ref[1] == 'f') && (ref[2] == 't') &&
			       (ref[3] == 'p') && (ref[4] == '/'))) {
				/* wants root of namespace name? */
      if (*pat == '/') strcpy (strchr (strcpy (tmp,ref),'/'),pat);
				/* otherwise just append */
      else sprintf (tmp,"%s%s",ref,pat);
    }
    else return NIL;		/* unknown namespace */
  }
  return T;
}

int dummy_echo (MAILSTREAM *stream)
{
  return T;
}

void dummy_clear_iocount(MAILSTREAM *stream)
{
  return;
}

void dummy_urgent (MAILSTREAM *stream)
{
  return;
}

void dummy_fetchshort (MAILSTREAM *stream, unsigned long msgno, SHORTINFO *sinfo,
		     long flags)
{
  return;
}

void dummy_checksum (MAILSTREAM *stream)
{
  return;
}

int dummy_validchecksum (MAILSTREAM *stream, unsigned short cksum, 
			 unsigned long nbytes)
{
  return T;
}

void dummy_disconnect(MAILSTREAM *stream)
{
  return;
}
/* Always sun version locally */
int dummy_sunversion(MAILSTREAM *stream)
{
  return T;
}

char *dummy_fabricate_from (MAILSTREAM *stream, long msgno)
{
  return NIL;
}

int dummy_sync_msgno (MAILSTREAM *stream,char *uidseq)
{
  return T;
}
/* ALWAYS imap4 c-client */
int dummy_imap4(MAILSTREAM *stream)
{
  return T;
}

/* dummy globals initializer
 * Accepts: MAIL stream
 * Returns: None. 
 */

/* NO dummy globals to be initialized. */
void dummy_global_init(MAILSTREAM *stream)
{
}

/* dummy globals free 
 * Accepts: MAIL stream
 * Returns: None.
 */

/* NO dummy globals to be freed. */
void dummy_global_free(MAILSTREAM *stream)
{
}
