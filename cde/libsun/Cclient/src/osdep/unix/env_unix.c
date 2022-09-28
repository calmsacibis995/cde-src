/*
 * @(#)env_unix.c	1.55 97/06/13
 *
 * Program:	UNIX environment routines
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
 * 18-sep-86  not all server signals enabled
 * 28-oct-96  root env. init for NON server logins
 * [09-23-96]  strtok_r implemented to ensure MT-safe.
 * [09-25-96]  getlogin_r implemented to ensure MT-Safe.
 * [09-27-96]  gethostbyname_r implemented to ensure MT-Safe.
 * [10-03-96]  getpwnam_r and getpwuid_r implemented to ensure MT-Safe.
 * [10-04-96]  localtime_r and gmtime_r
 * [10-05-96]  free the pw buffer from log_sv5.c. 
 * [10-08-96]  allocate more space for buf_pwd. 
 * [10-09-96]  allocate static storage only.
 * [10-21-96]  added a new routine: env_global_init. 
 * [10-21-96]  Most globals are put onto the stream.
 *		     Passing a stream onto the following routines:
 *		     env_parameters
 * 		     env_init
 *		     myusername_full
 *		     mylocalhost
 *		     myhomedir
 *		     sysinbox
 *		     mailboxdir
 * 		     mailboxfile
 *		     default_proto
 *		     dorc
 *		     *d->open
 *		     mail_parameters
 *
 * [10-25-96]  Call myusername and mylocalhost in env_global_init. 
 * [10-31-96]  Copy globals from stream to defaultProto in default_proto. 
 * [11-08-96]  Pass a stream in all mm_log calls. 
 * [11-19-96]  Pass a stream in all fatal calls. 
 * [11-20-96]  Flush myHomeDir before re-allocate it again. 
 * [12-18-96]  Remove macros for env_globals.
 * [12-20-96]  Remove imapd_globals in default_proto. 
 * [12-24-96]  Define env_globals in env.h 
 * [01-01-97]  Add env_global_free routine.
 * [01-03-97]  Use mail_stream_setNIL for a NIL stream passed 
 *		     in mail_parameters.
 *
 * [01-06-97]  Merge with L10N stuff.
 */

#include "UnixDefs.h"
#include "os_sv5.h"
#include <signal.h>
#include <sys/types.h>
#include <utime.h>

/* c-client environment parameters */
static boolean_t disableFcntlLock = _B_FALSE;

/* warning on EACCES errors on .lock files */
static boolean_t lockEaccesError = _B_FALSE;

static MAILSTREAM * defaultProto = NIL;

#include <limits.h>
#include <signal.h>
#include <sys/wait.h>

/* Get all authenticators */

/*
 * Environment manipulate parameters
 * Accepts: function code
 *	    function-dependent value
 * Returns: function-dependent return value
 */
void *
env_parameters(MAILSTREAM * stream, long function, void * value)
{
  ENV_GLOBALS 	*	eg = (ENV_GLOBALS *)stream->env_globals;

  switch ((int) function) {
  case SET_USERNAME:
    if (eg->myUserName) {
      fs_give((void **) &eg->myUserName);
    }
    eg->myUserName = cpystr((char *) value);
    break;

  case GET_USERNAME:
    value = (void *) eg->myUserName;
    break;

  case SET_HOMEDIR:
    if (eg->myHomeDir) {
	fs_give ((void **) &eg->myHomeDir);
    }
    eg->myHomeDir = cpystr((char *) value);
    break;

  case GET_HOMEDIR:
    value = (void *) eg->myHomeDir;
    break;

  case SET_LOCALHOST:
    if (eg->myLocalHost) {
      fs_give((void **) &eg->myLocalHost);
    }
    eg->myLocalHost = cpystr((char *) value);
    break;

  case GET_LOCALHOST:
    value = (void *) eg->myLocalHost;
    break;

  case SET_NEWSRC:
    if (eg->myNewsrc) {
      fs_give((void **) &eg->myNewsrc);
    }
    eg->myNewsrc = cpystr((char *) value);
    break;
  case GET_NEWSRC:
    value = (void *) eg->myNewsrc;
    break;

  case SET_NEWSACTIVE:
    if (eg->newsActive) {
      fs_give((void **) &eg->newsActive);
    }
    eg->newsActive = cpystr((char *) value);
    break;

  case GET_NEWSACTIVE:
    value = (void *) eg->newsActive;
    break;

  case SET_NEWSSPOOL:
    if (eg->newsSpool) {
      fs_give((void **) &eg->newsSpool);
    }
    eg->newsSpool = cpystr((char *) value);
    break;

  case GET_NEWSSPOOL:
    value = (void *) eg->newsSpool;
    break;

  case SET_SYSINBOX:
    if (eg->sysInbox) {
      fs_give((void **) &eg->sysInbox);
    }
    eg->sysInbox = cpystr((char *) value);
    break;

  case GET_SYSINBOX:
    value = (void *) eg->sysInbox;
    break;

  case SET_LISTMAXLEVEL:
    eg->list_max_level = (long) value;
    break;

  case GET_LISTMAXLEVEL:
    value = (void *) eg->list_max_level;
    break;

  case SET_MBXPROTECTION:
    eg->mbx_protection = (long) value;
    break;

  case GET_MBXPROTECTION:
    value = (void *) eg->mbx_protection;
    break;

  case SET_DIRPROTECTION:
    eg->dir_protection = (long) value;
    break;

  case GET_DIRPROTECTION:
    value = (void *) eg->dir_protection;
    break;

  case SET_LOCKPROTECTION:
    eg->lock_protection = (long) value;
    break;

  case GET_LOCKPROTECTION:
    value = (void *) eg->lock_protection;
    break;

  case SET_DISABLEFCNTLLOCK:
    disableFcntlLock = (boolean_t) value;
    break;

  case GET_DISABLEFCNTLLOCK:
    value = (void *) disableFcntlLock;
    break;

  case SET_LOCKEACCESERROR:
    lockEaccesError = (boolean_t) value;
    break;

  case GET_LOCKEACCESERROR:
    value = (void *) lockEaccesError;
    break;

  case SET_EGID:
    eg->savedEffectiveGid = (long) value;

    break;

  case GET_EGID:
    value = (void *) eg->savedEffectiveGid;
    break;

  default:
    value = NIL;		/* error case */
    break;

  }
  return(value);
}

/*
 * Write current time
 * Accepts: destination string
 *	    optional format of day-of-week prefix
 *	    format of date and time
 *	    flag whether to append symbolic timezone
 */
static void
do_date(char * date, char * prefix, char * fmt, int suffix)
{
  time_t		tn = time (0);
  struct tm res_gm, *	t = gmtime_r(&tn, &res_gm);
  int 			zone = t->tm_hour * 60 + t->tm_min;
  int 			julian = t->tm_yday;
  struct tm 		res_local;

  t = localtime_r(&tn, &res_local);/*get local time now -MT-safe.*/

  /* minus UTC minutes since midnight */
  zone = t->tm_hour * 60 + t->tm_min - zone;

  /* julian can be one of:
   *  36x  local time is December 31, UTC is January 1, offset -24 hours
   *    1  local time is 1 day ahead of UTC, offset +24 hours
   *    0  local time is same day as UTC, no offset
   *   -1  local time is 1 day behind UTC, offset -24 hours
   * -36x  local time is January 1, UTC is December 31, offset +24 hours
   */
  if (julian = t->tm_yday -julian) {
    zone += ((julian < 0) == (abs (julian) == 1)) ? -24*60 : 24*60;
  }
  if (prefix) {			/* want day of week? */
    sprintf(date,prefix,days[t->tm_wday]);
    date += strlen (date);	/* make next sprintf append */
  }

  /* output the date */
  sprintf(date,fmt,t->tm_mday,months[t->tm_mon],t->tm_year+1900,
	  t->tm_hour,t->tm_min,t->tm_sec,zone/60,abs (zone) % 60);

  /* append timezone suffix if desired */
  if (suffix) {
    rfc822_timezone(date,(void *) t);
  }

  return;
}


/*
 *  Write current time in RFC 822 format
 * Accepts: destination string
 */
void
rfc822_date(char *date)
{
  do_date(date, "%s, ", "%d %s %d %02d:%02d:%02d %+03d%02d", _B_TRUE);
}


/*
 * Write current time in internal format
 * Accepts: destination string
 */
void
internal_date(char *date)
{
  do_date(date, NULL, "%02d-%s-%d %02d:%02d:%02d %+03d%02d", _B_FALSE);
}


/*
 * Set server traps
 * Accepts: clock interrupt handler
 *	    kiss-of-death interrupt handler
 */
void
server_traps(void (*clkint)(int),
	     void (*kodint)(int),
	     void (*roint)(int))
{
  if (clkint) {
    signal (SIGALRM,clkint);    /* prepare for clock interrupt */
  }

  /* let us die quietly with the timeout */
  sigignore(SIGPIPE);		/* ignore broken PIPE(connection) */

  return;
}

/*
 * clients should call this one
 */
void
ignore_server_traps()
{
}

/*
 * Log in as anonymous daemon
 * Returns: T if successful, NIL if error
 */
long
anonymous_login()
{
  boolean_t		worked = _B_FALSE;
  struct passwd		res_pwd;
  struct passwd *	pwd;
  char 			buf_pwd[NSS_BUFLEN_PASSWD+1];

#if (_POSIX_C_SOURCE - 0 >= 199506L)
  if (getpwnam_r("nobody", &res_pwd, buf_pwd, 
		 NSS_BUFLEN_PASSWD + 1, &pwd) == 0)
#else
  if ((pwd = getpwnam_r("nobody", &res_pwd,
			buf_pwd, NSS_BUFLEN_PASSWD + 1))!= NULL)
#endif
    {

    /* try to become someone harmless */
    if (setgid (pwd->pw_gid) == 0) {	/* set group ID */
      if (setuid (pwd->pw_uid) == 0) {	/* and user ID */
	worked = _B_TRUE;
      }
    }
  }

  /* return whether or not we are now harmless */
  return(worked);
}

/*
 * Initialize environment
 * Accepts: user name
 *	    home directory name
 * Returns: _B_TRUE, always
 */
long
env_init(char * user, char * home, MAILSTREAM * stream)
{
  extern MAILSTREAM	STDPROTO;
  struct stat 		sbuf;
  char 			tmp[MAILTMPLEN];
  ENV_GLOBALS *		eg = (ENV_GLOBALS *)stream->env_globals;
  unsigned short 	stream_flag = NIL;

  if (eg->myUserName == NULL) {

    eg->myUserName = cpystr(user);	/* remember user name */

    if (eg->myHomeDir) {
      fs_give((void **) &eg->myHomeDir);
    }
    eg->myHomeDir = cpystr(home);	/* and home directory */

    sprintf(tmp, MAILFILE, eg->myUserName);
    eg->sysInbox = cpystr(tmp);	/* system inbox is from mail spool */

    dorc("/etc/imapd.conf", stream);	/* do systemwide */

    if (eg->blackBoxDir) {	/* build black box directory name */
      sprintf(tmp, "%s/%s", eg->blackBoxDir, eg->myUserName);

      /* if black box if exists and directory */
      if (eg->blackBox = (!stat(tmp, &sbuf)) && (sbuf.st_mode & S_IFDIR)) {

	/* flush standard values */
	fs_give((void **) &eg->myHomeDir);
	fs_give((void **) &eg->sysInbox);
	eg->myHomeDir = cpystr(tmp);	

	/* set black box values in their place */
	eg->sysInbox = cpystr(strcat (tmp, "/INBOX"));
      }
    } else {
      eg->blackBoxDir = "";
    }

    /*
     * make sure user rc files don't try this.
     * do user rc files.
     */
    dorc(strcat(strcpy(tmp, myhomedir(stream)),"/.imaprc"), stream);

    if (!eg->myLocalHost) {		/* have local host yet? */

      /* local variables for gethostbyname_r: */
      struct hostent *	host_name;	/* static alloate res */
      struct hostent 	res;

      /* dynamically allocate buf since it's large. */
      char 	*	buf_host;
      int 		h_errno;

      gethostname(tmp, MAILTMPLEN);/* get local host name */

      buf_host = (char *)fs_get( MAXHOSTNAMELEN + 16 );
      eg->myLocalHost = 
	cpystr((host_name = gethostbyname_r(tmp, &res, buf_host, \
					    MAXHOSTNAMELEN + 16, &h_errno)) ?
	       host_name->h_name : tmp);

      fs_give((void **) &buf_host); /*flush it right away. */
    }

    if (!eg->myNewsrc) {		/* set news file name if not defined */
      sprintf(tmp, "%s/.newsrc", myhomedir(stream));
      eg->myNewsrc = cpystr(tmp);
    }
    if (!eg->newsActive) {
      eg->newsActive = cpystr(ACTIVEFILE);
    }
    if (!eg->newsSpool) {
      eg->newsSpool = cpystr(NEWSSPOOL);
    }

    /* force default prototype to be set */
    if (!defaultProto) {
      defaultProto = &STDPROTO;
    }

    /* re-do open action to get flags */
    stream->stream_status |= S_PROTOTYPE;

    /* instead of passing a NIL, we set it to S_PROTOTYPE. */	
    (*defaultProto->dtb->open)(stream); 
    
    /* Need to unset Prototype. */
    stream->stream_status &= ~S_PROTOTYPE;
  }
  return(_B_TRUE);
}
 
/*
 * Returns: my user name
 */
char *
myusername(MAILSTREAM * stream)
{
  ENV_GLOBALS 		*	eg = (ENV_GLOBALS *)stream->env_globals;
  char 			*	ret = eg->myUserName;

  if (!eg->myUserName) {	/* get user name if don't have it yet */
    struct passwd 	*	pw;
    struct passwd		res;
    
    unsigned long		euid = geteuid ();
    char			buf_pwd[NSS_BUFLEN_PASSWD + 1];
    char			buf_pwduid[NSS_BUFLEN_PASSWD + 1];

#if (_POSIX_C_SOURCE - 0 >= 199506L)
    if (getpwuid_r(euid, &res, buf_pwduid, NSS_BUFLEN_PASSWD, &pw)==0)
#else
    if ((pw = getpwuid_r(euid, &res, buf_pwduid, NSS_BUFLEN_PASSWD))!=NULL)
#endif
      {
	env_init(pw->pw_name, pw->pw_dir, stream);
	ret = pw->pw_name;		/* Set the results to who we are */
      }
  }
  if (ret == NULL) {
    fatal ("Unable to look up user name", stream);
  }
  return(ret);
}


/*
 * Returns: my local host name
 */
char *
mylocalhost(MAILSTREAM * stream)
{
  ENV_GLOBALS 	*	eg = (ENV_GLOBALS *)stream->env_globals;
  char 			tmp[MAILTMPLEN];
  struct hostent *	host_name;

  if (!eg->myLocalHost) {

    /* local variables for gethostbyname_r: */
    struct hostent *	host_name;
    struct hostent 	res;
    char 	   *	buf_host;
    int			h_errno;

    gethostname(tmp, MAILTMPLEN);/* get local host name */

    buf_host = (char *)fs_get( MAXHOSTNAMELEN + 1 );
    eg->myLocalHost =
      cpystr((host_name = gethostbyname_r(tmp, &res, buf_host,
					  MAXHOSTNAMELEN + 1,
					  &h_errno))
	     ? host_name->h_name : tmp);

    fs_give((void **) &buf_host); /*flush it right away. */
  }
  return(eg->myLocalHost);
}

/* 
 * Returns: my home directory name
 */
char *
myhomedir(MAILSTREAM * stream)
{
  ENV_GLOBALS *	eg = (ENV_GLOBALS *)stream->env_globals;

  if (!eg->myHomeDir) {
    myusername(stream); /* initialize if first time */
  }
  return(eg->myHomeDir);
}

/* 
 * Accepts: buffer string
 */
char *
sysinbox(MAILSTREAM *stream)
{
  ENV_GLOBALS *	eg = (ENV_GLOBALS *)stream->env_globals;

  if (!eg->sysInbox) {
    myusername (stream);	
  }

  /* call myusername() if first time */
  return(eg->sysInbox);
}

/*
 * Return mailbox directory name
 * Accepts: destination buffer
 *	    directory prefix
 *	    name in directory
 * Returns: file name or NULL if error
 */
char *
mailboxdir(char * dst, char * dir, char * name, MAILSTREAM * stream)
{
  char		tmp[MAILTMPLEN];

  if (dir || name) {		/* if either argument provided */

    if (dir) {
      strcpy(tmp, dir);	/* write directory prefix */
    } else {
      tmp[0] = '\0';		/* otherwise null string */
    }

    if (name) {
      strcat(tmp, name);/* write name in directory */
    }

    /* validate name, return its name */
    if (!mailboxfile(dst, tmp, stream)) {
      return(NULL);
    }
  } else {
    strcpy(dst, myhomedir(stream));	/* no args, wants home directory */
  }
  return(dst);				/* return the name */
}

/*
 * Return mailbox file name
 * Accepts: destination buffer
 *	    mailbox name
 * Returns: file name or NULL if error
 */
char *
mailboxfile(char * dst, char * name, MAILSTREAM * stream)
{
  ENV_GLOBALS 	*	eg = (ENV_GLOBALS *)stream->env_globals;
  struct passwd 	res_pwd;
  struct passwd	*	pwd;  /* will be used twice in this routine.*/
  char 			buf_pwd[NSS_BUFLEN_PASSWD+1];
  char 		*	s;
  char			tmp[PATH_MAX+1];
  char 		*	dir = myhomedir(stream);

  *dst = '\0';			/* erase buffer */
  if (eg->blackBox && strstr(name,"/..")) {
    return(NULL);
  }

  switch (*name) {		/* dispatch based on first character */

  case '#':			/* namespace name? */
    if ((strncmp(&name[1], "ftp/", 4) == 0)
	&& !strstr (name, "..") && !strstr (name, "//") &&
	 !strstr (name, "/~") && 

#if (_POSIX_C_SOURCE - 0 >= 199506L)
	(getpwnam_r("ftp", &res_pwd, buf_pwd, NSS_BUFLEN_PASSWD, &pwd)==0)
#else
	((pwd = getpwnam_r("ftp", &res_pwd, buf_pwd, NSS_BUFLEN_PASSWD))!=NULL)
#endif
	&& pwd->pw_dir ) {

      sprintf(dst, "%s/%s", pwd->pw_dir, name + 5);

    } else {
      return(NULL);			/* definite error */
    }
    break;

  case '/':			/* absolute file path */
    if (eg->blackBox != NULL
	&& strncmp(name, eg->sysInbox, strlen(eg->myHomeDir)+1)) {
      return(NULL);
    }
    strcpy(dst, name);
    break;

  case '.':			/* these names may be private */
    if (eg->blackBox != NULL && (name[1] == '.')) {
      return(NULL);
    }
    sprintf(dst, "%s/%s", dir, name);
    break;

  case '~':				/* home directory */
    /* specified just "~"  or specified "~/..." */
    if (name[1] == '\0' || name[1] == '/') {
      sprintf(dst, "%s/%s", eg->myHomeDir, eg->myUserName);
      if (name[1] == '/' && name[2] != '\0') {
	strcat(dst, &name[2]);
      }
    } else {
      strcpy(tmp, name+1);		/* copy user name */
      if ((s = strchr(tmp, '/')) != NULL) {
	*s = '\0';			/* terminate any user name */
      }

      /* look up the user name in the password file */
#if (_POSIX_C_SOURCE - 0 >= 199506L)
      if (!((getpwnam_r(tmp, &res_pwd, buf_pwd,
			NSS_BUFLEN_PASSWD, &pwd) != 0)))
#else
      if (((pwd = getpwnam_r(tmp, &res_pwd, buf_pwd,
			     NSS_BUFLEN_PASSWD)) == NULL))
#endif
	{
	  return(NULL);
	}
      sprintf(dst, "%s/%s", pwd->pw_dir, &name[1]);
    }
    break;

  default:			/* some other name */
    /* non-INBOX name is in home directory */
    if (strcmp(ucase(strcpy(tmp, name)), "INBOX")) {
      sprintf(dst, "%s/%s", dir, name);
    } else {		/* blackbox INBOX is always in home dir */
      if (eg->blackBox) {
	sprintf (dst,"%s/INBOX",dir);
      }
    }
    /* empty name means driver selects INBOX */
    break;
  }

  return(dst);
}

/*
 * Determine default prototype stream to user
 * Returns: default prototype stream
 */
MAILSTREAM *
default_proto(MAILSTREAM * stream)
{
  myusername(stream);		/* make sure initialized */
  return(defaultProto);		/* return default driver's prototype */
}


/*
 * Set up user flags for stream
 * Accepts: MAIL stream
 * Returns: MAIL stream with user flags set up
 */
MAILSTREAM *
  user_flags(MAILSTREAM * stream)
{
  ENV_GLOBALS *	eg = (ENV_GLOBALS *)stream->env_globals;
  int 		i;

  myusername(stream);		/* make sure initialized */
  for (i = 0; i < NUSERFLAGS; ++i) {
    stream->user_flags[i] = eg->userFlags[i];
  }
  return(stream);
}

/*
 * Return nth user flag
 * Accepts: user flag number
 * Returns: flag
 */
char *
default_user_flag(unsigned long i, MAILSTREAM * stream)
{
  ENV_GLOBALS *	eg = (ENV_GLOBALS *)stream->env_globals;

  myusername(stream);		/* make sure initialized */
  return(eg->userFlags[i]);
}


/*
 * Process rc file
 * Accepts: file name
 */
void
dorc(char * file, MAILSTREAM * stream)
{
  extern MAILSTREAM 	STDPROTO;

  ENV_GLOBALS 	*	eg = (ENV_GLOBALS *)stream->env_globals;
  char 		*	lasts;
  int 			i;
  char 		*	linePtr;
  char		*	eolPtr;
  char	       	*	wordPtr;
  char			tmp[MAILTMPLEN];
  char			tmpx[MAILTMPLEN];
  DRIVER 	*	driver;
  FILE 		*	fp = fopen (file, "r");

  mail_stream_setNIL(stream);
  if (!fp) {
    return;		/* punt if no file */
  }
  while ((linePtr = fgets(tmp, MAILTMPLEN, fp))
	 && (eolPtr = strchr(linePtr, '\n'))) {

    *eolPtr++ = '\0';		/* tie off line, find second space */
    if ((wordPtr = strchr(linePtr,' ')) 
	&& (wordPtr = strchr(++wordPtr, ' '))) {
      *wordPtr++ = '\0';	/* tie off two words*/
      lcase(linePtr);		/* make case-independent */

      if (!(defaultProto || strcmp(linePtr, "set empty-folder-format"))) {

	if (!strcmp(lcase(wordPtr), "same-as-inbox")) {

	/* stream_status bit has been set to NIL.*/
	  defaultProto = ((driver = mail_valid(stream, "INBOX", NIL)) &&
			  strcmp(driver->name, "dummy")) ?
	    ((*driver->open)(stream)) : &STDPROTO;

	} else if (!strcmp(wordPtr, "system-standard")) {

	  defaultProto = &STDPROTO;

	} else {			/* see if a driver name */
	  for (driver = (DRIVER *) mail_parameters(stream ,GET_DRIVERS, NIL)
		 ; driver && strcmp(driver->name, wordPtr)
		 ; driver = driver->next);

	  if (driver != NULL) {
	    defaultProto = (*driver->open) (stream);
	  }  else {		/* duh... */
	    sprintf(tmpx, "Unknown empty folder format in %s: %s", file,
		    wordPtr);
	    mm_log(tmpx, WARN, stream);
	  }
	}

      } else if (!(eg->userFlags[0] || strcmp(linePtr, "set keywords"))) {
	/* yes, get first keyword */
	wordPtr = strtok_r(wordPtr, ", ", &lasts);

	/* copy keyword list */
	for (i = 0; wordPtr && i < NUSERFLAGS; ++i) {
	  if (eg->userFlags[i]) {
	    fs_give((void **) &eg->userFlags[i]);
	  }
	  eg->userFlags[i] = cpystr(wordPtr);
	  wordPtr = strtok_r(NIL, ", ", &lasts);
	}

      } else if (!strcmp(linePtr, "set from-widget")) {

	mail_parameters(stream,
			SET_FROMWIDGET,
			strcmp(lcase(wordPtr), "header-only")
			? (void *) _B_TRUE : _B_FALSE);

      }  else if (!strcmp (linePtr, "set black-box-directory")) {

	if (eg->blackBoxDir != NULL) {
	  /* users aren't allowed to do this */
	  mm_log("Can't set black-box-directory in user init", ERROR, stream);
	} else {
	  eg->blackBoxDir = cpystr(wordPtr);
	}

      } else if (!strcmp(linePtr, "set local-host")) {

	fs_give((void **) &eg->myLocalHost);
	eg->myLocalHost = cpystr(wordPtr);

      } else if (!strcmp(linePtr, "set news-active-file")) {

	fs_give((void **) &eg->newsActive);
	eg->newsActive = cpystr(wordPtr);

      } else if (!strcmp(linePtr, "set news-spool-directory")) {

	fs_give((void **) &eg->newsSpool);
	eg->newsSpool = cpystr(wordPtr);
      }
    }
    linePtr = eolPtr;			/* try next line */
  }
  /* unset the stream_status bit */
  mail_stream_unsetNIL(stream);
  fclose(fp);				/* flush the file */

#ifdef IN_4_1_TODO
tcpOpenTimeout;
tcpReadTimeout;
tcpWriteTimeout;
rshTimeout;
maxLoginTrials;
lookahead;
prefetch;
closeOnError;
imapPort;
pop3Port;
uidLookAhead;
mailboxMode;
directoryMode;
lockFileMode;
disableSVR4Locking;
ignoreEACCESLocks;
listMaxLevel;
anonymousFtpHome;
#endif
}

/*
 *  Input: a mailstream.
 *  Return: None.
 *  Purpose: env_globals initializer.
 */
void
env_global_init(MAILSTREAM *stream)
{ 
  /*
   * Because the variables contained in ENV_GLOBALS will be
   *  dynamically allocated, we don't know how much memory we
   *  should allocate at this point.  To be safe, I added a 100
   */
  ENV_GLOBALS *		eg=(ENV_GLOBALS *) fs_get(sizeof(ENV_GLOBALS) + 100);

  memset ((void *) eg,0,sizeof (ENV_GLOBALS) + 100 );

  eg->list_max_level = 20;
  eg->mbx_protection = 0600;
  eg->dir_protection = 0700;
  eg->lock_protection = 0666;
  eg->savedEffectiveGid = 0;
  eg->userFlags[NUSERFLAGS] = NIL;

  stream->env_globals = eg;
  myusername(stream);
  mylocalhost(stream);
}

/*
 *
 *  Input: a mailstream.
 *  Return: None.
 *  Purpose: flush all globals on the globals data structure.
 */
void
env_global_free(MAILSTREAM *stream)
{
  ENV_GLOBALS *	eg = (ENV_GLOBALS *)stream->env_globals;

  if (eg)  {

    /* Need to flush these allocated globals. */
    if (eg->myUserName != NULL) {
      fs_give((void **) &eg->myUserName);
    }
    if (eg->myHomeDir != NULL) {
      fs_give((void **) &eg->myHomeDir);
    }
    if (eg->myLocalHost != NULL) {
      fs_give((void **) &eg->myLocalHost);
    }
    if (eg->myNewsrc != NULL) {
      fs_give((void **) &eg->myNewsrc);
    }
    if (eg->newsActive != NULL) {
      fs_give((void **) &eg->newsActive);
    }
    if (eg->sysInbox != NULL) {
      fs_give((void **) &eg->sysInbox);
    }
    if (eg->newsSpool != NULL) {
      fs_give((void **) &eg->newsSpool);
    }
    if (eg->blackBoxDir != NULL && *(eg->blackBoxDir) ) {
      fs_give((void **) &eg->blackBoxDir);
    }
    
    /* Then finally the env_globals */
    fs_give((void **) &eg);

    stream->env_globals = eg;
  }
  return;
}
