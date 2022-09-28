/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: server.c /main/cde1_maint/3 1995/10/09 00:12:24 pascale $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

# include	<signal.h>
# include	<setjmp.h>
# include	<dirent.h>
# include	<pwd.h>
# include	"dm.h"
# include	"vgmsg.h"

static receivedUsr1;



/***************************************************************************
 *
 *  Local procedure declarations
 *
 ***************************************************************************/
#ifdef _NO_PROTO

static char * _SysErrorMsg() ;
static SIGVAL CatchUsr1() ;
static void   GetRemoteAddress() ;
static SIGVAL PingBlocked() ;
static SIGVAL PingLost() ;
static SIGVAL abortOpen() ;
static int    serverPause() ;
static SIGVAL serverPauseAbort() ;
static SIGVAL serverPauseUsr1() ;
static char *getHostname();

#else

static char * _SysErrorMsg( int n) ;
static SIGVAL CatchUsr1( int arg ) ;
static void   GetRemoteAddress( struct display *d, int fd) ;
static SIGVAL PingBlocked( int arg ) ;
static SIGVAL PingLost( int arg ) ;
static SIGVAL abortOpen( int arg ) ;
static int    serverPause( unsigned t, int serverPid) ;
static SIGVAL serverPauseAbort( int arg ) ;
static SIGVAL serverPauseUsr1( int arg ) ;
static char *getHostname(void);

#endif /* _NO_PROTO */




/***************************************************************************
 *
 *  Global variables
 *
 ***************************************************************************/

static Display	*dpy;

extern int server_pid_rem_greet;
extern char* server_display_rem_greet;



static SIGVAL
#ifdef _NO_PROTO
CatchUsr1()
#else
CatchUsr1( int arg )
#endif /* _NO_PROTO */
{
#if defined(SYSV) || defined(SVR4)
    (void) signal (SIGUSR1, (void(*)()) CatchUsr1);
#endif
    Debug ("Display Manager caught SIGUSR1\n");
    ++receivedUsr1;
}

static char * 
#ifdef _NO_PROTO
_SysErrorMsg( n )
        int n ;
#else
_SysErrorMsg( int n )
#endif /* _NO_PROTO */
{

    char *s = ((n >= 0 && n < sys_nerr) ? sys_errlist[n] : "unknown error");

    return (s ? s : "no such error");
}

int 
#ifdef _NO_PROTO
StartServerOnce( d )
        struct display *d ;
#else
StartServerOnce( struct display *d )
#endif /* _NO_PROTO */
{
    char	**f;
    char	**argv;
    char	arg[1024];
    int		pid;
    char	**env;
    char 	old_pipe[MAXPATHLEN];

    
    extern struct passwd   puser;	/* pseudo_user password entry	*/

    Debug ("Starting server for %s\n", d->name);
    receivedUsr1 = 0;
    signal (SIGUSR1, (void(*)()) CatchUsr1);
    argv = d->argv;
    switch (pid = fork ()) {
    case 0:
	CleanUpChild ();
	if (d->authFile) {
	    sprintf (arg, "-auth %s", d->authFile);
	    argv = parseArgs (argv, arg);
	}
	if (!argv) {
	    LogError(ReadCatalog(MC_LOG_SET,MC_LOG_NO_ARGS,MC_DEF_LOG_NO_ARGS));
	    exit(1);
	}
	Debug("Server invoked as ");
	for (f = argv; *f; f++)
	    Debug ("'%s' ", *f);
	Debug ("\n");

	/*
         *  set the permissions on  console devices to  pseudo-user.
         *  run the server as a pseudo-user, not root...
         */
#ifdef sun
        if (solaris_setdevperm(d->gettyLine, puser.pw_uid, puser.pw_gid) == 0)
             Debug ("Unable to set permissions on console devices ..\n");
        else {
#endif
             setgid (puser.pw_gid);
             setuid (puser.pw_uid);
#ifdef sun
        }
#endif

	/*
	 *  build the server environment (if any)...
	 */
	env = NULL;
	if (d->environStr && strlen(d->environStr) > 0)
	    env = parseEnv(env, d->environStr);
#ifdef sun
	if ( env == NULL || getEnv (env, "OPENWINHOME") == NULL ) 
	    env = setEnv(env, "OPENWINHOME", "/usr/openwin");
#endif
#ifdef _AIX
	if (getEnv (env, "ODMDIR") == NULL)
	    env = setEnv(env, "ODMDIR", "/etc/objrepos");
#ifdef _POWER
	env = setEnv(env, "XTOEXEC", "true");		/* flag for xserverrc */
#endif
#endif

	/*
	 * give the server SIGUSR1 ignored,
	 * it will notice that and send SIGUSR1
	 * when ready
	 */

	signal (SIGUSR1, SIG_IGN);

        /*
         * remove old communication pipe (if present).
         */

	sprintf(old_pipe, "%s/%d", SDTLOGIN_DIR, d->displayNumber);
        unlink(old_pipe);

	/*
	 * Start the Xserver
	 */

	(void) execve (argv[0], argv, env);
	LogError(ReadCatalog(
		MC_LOG_SET,MC_LOG_NO_EXESRV,MC_DEF_LOG_NO_EXESRV),argv[0]);
	exit(1);
    case -1:
	LogError(ReadCatalog(MC_LOG_SET,MC_LOG_FAIL_FORK,MC_DEF_LOG_FAIL_FORK));
	return 0;
    default:
	break;
    }
    Debug ("Server started. Process ID = %d\n", pid);
    d->serverPid = pid;
    if (serverPause ((unsigned) d->openDelay, pid))
	return FALSE;
    return TRUE;
}


int 
#ifdef _NO_PROTO
StartServer( d )
        struct display *d ;
#else
StartServer( struct display *d )
#endif /* _NO_PROTO */
{
    int	i;
    int	ret = FALSE;

    i = 0;
    while (d->serverAttempts == 0 || i < d->serverAttempts)
    {
	if ((ret = StartServerOnce (d)) == TRUE)
	    break;
	sleep (d->openDelay);
	i++;
    }
    return ret;
}


/*
 * sleep for t seconds, return 1 if the server is dead when
 * the sleep finishes, 0 else
 */

static jmp_buf	pauseAbort;
static int	serverPauseRet;

static SIGVAL
#ifdef _NO_PROTO
serverPauseAbort()
#else
serverPauseAbort( int arg )
#endif /* _NO_PROTO */
{
    Debug ("Display Manager pause timed out\n");
    longjmp (pauseAbort, 1);
}

static SIGVAL
#ifdef _NO_PROTO
serverPauseUsr1()
#else
serverPauseUsr1( int arg )
#endif /* _NO_PROTO */
{
    Debug ("Display Manager pause received SIGUSR1\n");
    ++receivedUsr1;
    longjmp (pauseAbort, 1);
}

static int 
#ifdef _NO_PROTO
serverPause( t, serverPid )
        unsigned t ;
        int serverPid ;
#else
serverPause( unsigned t, int serverPid )
#endif /* _NO_PROTO */
{
    int		pid;

    serverPauseRet = 0;
    Debug ("Display Manager pausing until SIGUSR1 from server or timeout\n");
    if (!setjmp (pauseAbort)) {
	signal (SIGALRM, (void(*)()) serverPauseAbort);
	signal (SIGUSR1, (void(*)()) serverPauseUsr1);
#ifdef SYSV
	if (receivedUsr1)
	    alarm ((unsigned) 1);
	else
	    alarm (t);
#else
	if (!receivedUsr1)
	    alarm (t);
	else
	    Debug ("ServerPause(): already received USR1\n");
#endif
	for (;;) {
#ifdef SYSV
	    pid = wait ((waitType *) 0);
#else
	    if (!receivedUsr1)
		pid = wait ((waitType *) 0);
	    else
#  ifdef	SVR4
                {
		    int dummy;
		pid = waitpid ((pid_t) 0,&dummy,WNOHANG);
                }
#  else
		pid = wait3 ((waitType *) 0, WNOHANG,
			     (struct rusage *) 0);
#  endif
#endif
	    if (pid == serverPid ||
		pid == -1 && errno == ECHILD)
	    {
		Debug ("Server dead\n");
		serverPauseRet = 1;
		break;
	    }
#ifndef SYSV
	    if (pid == 0) {
		Debug ("Server alive and kicking\n");
		break;
	    }
#endif
	}
    }
    alarm ((unsigned) 0);
    signal (SIGALRM, SIG_DFL);
    signal (SIGUSR1, (void(*)()) CatchUsr1);
    if (serverPauseRet) {
	Debug ("Server died\n");
	LogError(ReadCatalog(MC_LOG_SET,MC_LOG_SRV_DIED,MC_DEF_LOG_SRV_DIED));
    }
    return serverPauseRet;
}


/*
 * this code is complicated by some TCP failings.  On
 * many systems, the connect will occasionally hang forever,
 * this trouble is avoided by setting up a timeout to longjmp
 * out of the connect (possibly leaving piles of garbage around
 * inside Xlib) and give up, terminating the server.
 */

static jmp_buf	openAbort;

static SIGVAL
#ifdef _NO_PROTO
abortOpen()
#else
abortOpen( int arg )
#endif /* _NO_PROTO */
{
	longjmp (openAbort, 1);
}

static void
#ifdef _NO_PROTO
GetRemoteAddress( d, fd )
        struct display *d ;
        int fd ;
#else
GetRemoteAddress( struct display *d, int fd )
#endif /* _NO_PROTO */
{
    char    buf[512];
    int	    len = sizeof (buf);

    if (d->peer)
	free ((char *) d->peer);
    getpeername (fd, (struct sockaddr *) buf, &len);
    d->peerlen = 0;
    if (len)
    {
	d->peer = (struct sockaddr *) malloc (len);
	if (d->peer)
	{
	    bcopy (buf, (char *) d->peer, len);
	    d->peerlen = len;
	}
    }
    Debug ("Got remote address %s %d\n", d->name, d->peerlen);
}



/****************************************************************************
 *
 *  LogOpenError()
 *
 *  If d->startAttempts is a large number and a connection cannot be made to
 *  the server, the error log can fill up rapidly with error messages. This
 *  could be common in X-terminals that do not support XDMCP and are turned
 *  off over a weekend. This routine attempts to reduce the number of error
 *  messages logged in this scenario.
 *  
 ****************************************************************************/

int 
#ifdef _NO_PROTO
LogOpenError( count )
        int count ;
#else
LogOpenError( int count )
#endif /* _NO_PROTO */
{

    if ( count <= 10			    ) return 1;
    if ( count <= 100 && (count %  10 == 0) ) return 1;
    if ( count <= 500 && (count %  50 == 0) ) return 1;
    if ( 		 (count % 100 == 0) ) return 1;
    
    return 0;
}


int 
#ifdef _NO_PROTO
WaitForServer( d )
        struct display *d ;
#else
WaitForServer( struct display *d )
#endif /* _NO_PROTO */
{
    int	    i;

    for (i = 0; i < (d->openRepeat > 0 ? d->openRepeat : 1); i++) {
    	(void) signal (SIGALRM, (void(*)()) abortOpen);
    	(void) alarm ((unsigned) d->openTimeout);
    	if (!setjmp (openAbort)) {
	    Debug ("Before XOpenDisplay(%s)\n", d->name);
	    errno = 0;
	    dpy = XOpenDisplay (d->name);
	    (void) alarm ((unsigned) 0);
	    (void) signal (SIGALRM, SIG_DFL);
	    Debug ("After XOpenDisplay()\n");
	    if (dpy) {
	    	if (d->displayType.location == Foreign)
		    GetRemoteAddress (d, ConnectionNumber (dpy));
	    	RegisterCloseOnFork (ConnectionNumber (dpy));
                (void) fcntl (ConnectionNumber (dpy), F_SETFD, 0);
	    	return 1;
	    } else {
	    	Debug ("OpenDisplay failed %d (%s)\n",
		       errno, _SysErrorMsg (errno));
	    }
	    Debug ("Waiting for server to start %d\n", i);
	    sleep ((unsigned) d->openDelay);
    	} else {
	    Debug ("Hung in open, aborting\n");
	    if (LogOpenError(d->startTries))
		LogError(ReadCatalog(
			MC_LOG_SET,MC_LOG_HUNG_DPY,MC_DEF_LOG_HUNG_DPY),
			   d->name, d->startTries);
	    (void) signal (SIGALRM, SIG_DFL);
	    break;
    	}
    }
    Debug ("Giving up on server\n");
    if (LogOpenError(d->startTries))
	LogError(ReadCatalog(
		MC_LOG_SET,MC_LOG_FAIL_SRVOPEN,MC_DEF_LOG_FAIL_SRVOPEN),
		d->startTries, d->name);
    return 0;
}

void
#ifdef _NO_PROTO
ResetServer( d )
        struct display *d ;
#else
ResetServer( struct display *d )
#endif /* _NO_PROTO */
{
    if (dpy && d->displayType.origin != FromXDMCP)
	pseudoReset (dpy);
}


/****************************************************************************
 *
 *  Server pinging routines...
 *
 *  These routines attempt to determine if the server is still alive.
 *  Periodically (d->pingInterval) an XSync is sent to the server. If an I/O
 *  error occurs, then the connection has been lost and the server needs to
 *  be reset. If the server is blocked for some reason (i.e. server grab) the
 *  XSync will block until a local timer expires. In this case, we just note
 *  the block and continue...
 *
 *  7/26/90 - prr
 *  The XSync was replaced by a socket-level ping. For some reason, an XSync
 *  to a grabbed server causes a subsequent pseudoReset (KillClients) to not 
 *  kill all clients. The socket ping is not affected by a grabbed server,
 *  but it cannot detect a server shutdown and restart within one ping
 *  interval. 
 *
 ****************************************************************************/

static jmp_buf	pingTime;
static int	serverDead = FALSE;

static SIGVAL
#ifdef _NO_PROTO
PingLost()
#else
PingLost( int arg )
#endif /* _NO_PROTO */
{
    serverDead = TRUE;
    longjmp (pingTime, 1);
}


static SIGVAL
#ifdef _NO_PROTO
PingBlocked()
#else
PingBlocked( int arg )
#endif /* _NO_PROTO */
{
    serverDead = FALSE;
    longjmp (pingTime, 1);
}


int 
#ifdef _NO_PROTO
PingServer( d, alternateDpy )
        struct display *d ;
        Display *alternateDpy ;
#else
PingServer( struct display *d, Display *alternateDpy )
#endif /* _NO_PROTO */
{
    int	    (*oldError)();
    SIGVAL  (*oldSig)();
    int	    oldAlarm;

    if (!alternateDpy)
	alternateDpy = dpy;
    oldError = XSetIOErrorHandler ((XIOErrorHandler)PingLost);
    oldAlarm = alarm (0);
    oldSig = signal (SIGALRM, (void(*)()) PingBlocked);
    alarm (d->pingTimeout * 60);
    if (!setjmp (pingTime))
    {
	Debug ("Ping server\n");
	XNoOp (alternateDpy);
	XSync (alternateDpy, 0);
	Debug ("Server alive\n");

	while (XPending(alternateDpy)) {
	    XEvent event;
	    XNextEvent(alternateDpy, &event);
	}
    }
    else
    {
	if ( serverDead ) {
	    Debug ("Server dead\n");
	    alarm (0);
	    signal (SIGALRM, SIG_DFL);
	    XSetIOErrorHandler (oldError);
	    return 0;
	}
	else
	    Debug ("Server blocked, continuing...\n");
    }
    alarm (0);
    signal (SIGALRM, oldSig);
    alarm (oldAlarm);
    XSetIOErrorHandler (oldError);
    return 1;
}

int
#ifdef _NO_PROTO
StartRemoteGreet_Chooser( d ,client)
        struct display *d ;
        int client;
#else
StartRemoteGreet_Chooser( struct display *d, int client)
#endif /* _NO_PROTO */
{
char        **f;
    char        **argv;
    char        arg[1024];
    int         pid = 0;
    char        **env;
 
    extern struct passwd   puser;       /* pseudo_user password entry   */
 
    Debug ("Starting server for remote desktop \n");
    receivedUsr1 = 0;
    signal (SIGUSR1, (void(*)()) CatchUsr1);
    argv = d->argv;
    switch (pid = fork ()) {
    case 0:
        CleanUpChild ();
 
        strcpy(arg,"");
 
        if (!argv) {
          LogError(ReadCatalog(MC_LOG_SET,MC_LOG_NO_ARGS,MC_DEF_LOG_NO_ARGS));
              exit(1);
        }
 
        if(!client) {
           sprintf (arg,"-indirect %s",localHostname());
           argv = parseArgs (argv, arg);
        }
        else {
           sprintf (arg,"-query %s -once",getHostname());
           argv = parseArgs (argv, arg);
        }
 
 
        /*
         *  set the permissions on  console devices to  pseudo-user.
         *  run the server as a pseudo-user, not root...
         */
        if (solaris_setdevperm(d->gettyLine, puser.pw_uid, puser.pw_gid) == 0)
             Debug ("Unable to set permissions on console devices ..\n");
        else {
             setgid (puser.pw_gid);
             setuid (puser.pw_uid);
        }
 
        /*
         *  build the server environment (if any)...
         */
        env = 0;
        if (d->environStr && strlen(d->environStr) > 0)
            env = parseEnv(env, d->environStr);
#ifdef sun
        if (getEnv (env, "OPENWINHOME") == NULL)
            env = setEnv(env, "OPENWINHOME", "/usr/openwin");
#endif
#ifdef _AIX
        if (getEnv (env, "ODMDIR") == NULL)
            env = setEnv(env, "ODMDIR", "/etc/objrepos");
#endif
 
        /*
         * give the server SIGUSR1 ignored,
         * it will notice that and send SIGUSR1
         * when ready
         */
 
        signal (SIGUSR1, SIG_IGN);
 
        (void) execve (argv[0], argv, env);
        LogError(ReadCatalog(
                MC_LOG_SET,MC_LOG_NO_EXESRV,MC_DEF_LOG_NO_EXESRV),argv[0]);
 
        exit(1);
 
    case -1:
      LogError(ReadCatalog(MC_LOG_SET,MC_LOG_FAIL_FORK,MC_DEF_LOG_FAIL_FORK));
      return 0;
 
    default:
        break;
    }

    server_pid_rem_greet = pid;

    if (server_display_rem_greet != NULL) {
	free(server_display_rem_greet);
    }
    server_display_rem_greet = strdup(d->name);

    Debug ("Server for remote greeter or chooser started. Process ID = %d\n",
server_pid_rem_greet);
    if (server_display_rem_greet) { 
	Debug ("Display = %s\n", server_display_rem_greet);	
    }

    if (serverPause ((unsigned) d->openDelay, server_pid_rem_greet)) {
        return FALSE;
    }

    return TRUE;
}

static char *
#ifdef _NO_PROTO
getHostname()
#else
getHostname( void )
#endif /* _NO_PROTO */
{
    int len;
    static  char retbuf[100];
    FILE        *f;
 
        if ((f = fopen(REMOTE_GREET_HOST_FILE,"r")) != (FILE *) NULL) {
            strcpy(retbuf, "");
            fgets(retbuf,100,f);
            len = strlen (retbuf);
            if (retbuf[len-1] == '\n')
                retbuf[len-1] = '\0';
 
            fclose(f);
        }
    return (retbuf);
}
