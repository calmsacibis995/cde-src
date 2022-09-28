/*
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
 * Last Edited:	27 Dec 1996
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
 * Edit Log:
 *
 * 27-oct-96  Allow local user to use the c-client as root - Bill
 *
 * [12-24-96] Define env_globals in this file.
 *
 */
#ifndef _ENV_UNIX_H_
#define _ENV_UNIX_H_
 
#ifdef __cplusplus
extern "C" {
#endif

#define SUBSCRIPTIONFILE(t) sprintf(t,("%s/.mailboxlist"),myhomedir())
#define SUBSCRIPTIONTEMP(t) sprintf(t,("%s/.mlbxlsttmp"),myhomedir())

/* Function prototypes */
#include "UnixDefs.h"

#include "env.h"
#include <locale.h>

typedef struct env_globals {

  char	*	myUserName;  /* user name */
  char	*	myHomeDir;   /* home directory name */
  char	*	myLocalHost; /* local host name */
  char	*	myNewsrc;    /* newsrc file name */
  char	*	sysInbox;    /* system inbox name */
  char	*	newsActive;  /* news active file */
  char	*	newsSpool;   /* news spool */
  char	*	newsStateFile; /* news state file (default ~/.newsrc) */
  char	*	systemInBox; /* The default inbox to load (default INBOX) */
  char	*	blackBoxDir; /* black box directory name */

/* If the TCP values are set to '0', then use the system default - seconds */
  int		tcpOpenTimeout; /* How long to wait for a TCP timeout */
  int		tcpReadTimeout; /* How long to wait for a TCP read */
  int		tcpWriteTimeout;/* How long to wait for a TCP write */
  int		rshTimout;	/* How long to wait for RSH to finish */
  int		maxLoginTrials; /* How many times to ask user for login - before exit */
  int		lookahead;/* Number headers to prefetch mail_fetchstructre() */
  int		prefetch;	/* Number of envelopes prefetched */
  boolean_t	closeOnError;
  int		imapPort;
  int		pop3Port;
  int		uidLookahead;
  long		list_max_level;/* maximum level of list recursion(default 20)*/
  mode_t	lock_protection;/* default lock file protection */
  mode_t	mbx_protection; /* default file protection */
  mode_t	dir_protection; /* default directory protection */
  boolean_t	disableSVR4Locking;	/* Disable fcntl() locking */
  boolean_t 	lockEaccesError;  /* warning on EACCES errors on .lock files */
  char	*	anonymousFtpHome; /* Home directory of anonymos FTP (default ~ftp) */
  int 		blackBox;      /* is a black box */
  long		savedEffectiveGid;  /* savedEffectiveGid */
  char *	userFlags[NUSERFLAGS];
} ENV_GLOBALS;

#define MU_LOGGEDIN 0
#define MU_NOTLOGGEDIN  1
#define MU_ANONYMOUS    2
#define MU_SERVER_LOGIN 3

MAILSTREAM *	user_flags(MAILSTREAM * stream);

long		anonymous_login();
long 		env_init(char *user, char * home, MAILSTREAM * stream);
long 		Write(int fd, char * buf,long nbytes);
long 		Writev(int fd, struct iovec * iov,int iovcnt);

int 		sessionlock_valid(char * slock, MAILSTREAM * stream);

char	*	myusername(MAILSTREAM * stream);
char 	*	sysinbox(MAILSTREAM * stream);
char 	*	lockname(char * tmp, char * fname);
char 	*	default_user_flag(unsigned long i, MAILSTREAM * stream);
char 	*	mailboxdir(char * dst, char * dir, char  *name,
			   MAILSTREAM * stream);

void 		dorc(char * file, MAILSTREAM * stream);
void 		grim_pid_reap_status(int pid, int killreq, void * status);
void 		ignore_server_traps();
void 		env_global_init(MAILSTREAM * stream);
void		env_global_free(MAILSTREAM * stream);
void 		server_traps (void (*clkint)(int),
			      void (*kodint)(int),
			      void (*roint)(int));

#ifdef __cplusplus
}
#endif
 
#endif /* _ENV_UNIX_H_ */
