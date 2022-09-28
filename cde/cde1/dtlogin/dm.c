/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: dm.c /main/cde1_maint/6 1995/11/14 12:48:23 mgreess $
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

/*
 * display manager
 */

# include	<sys/signal.h>
# include	<sys/stat.h>
# include	<utmp.h>
# include	<time.h>
# include	<utime.h>
# include	<pwd.h>
# include	<varargs.h>
# include	<dirent.h>

#if defined (SYSV) || defined (SVR4)
#ifndef F_TLOCK
# include	<unistd.h>
#endif
#endif
# include	"dm.h"
# include	"vgmsg.h"

#ifdef sun
#include <sys/kbio.h>
#include <sys/kbd.h>
#endif

#ifndef sigmask
#define sigmask(m)  (1 << (( m-1)))
#endif


/***************************************************************************
 *
 *  External variable declarations
 *
 ***************************************************************************/

#if defined(USL) || defined(__uxp__)
extern  int  makepttypair ();
#endif


/***************************************************************************
 *
 *  Local procedure declarations
 *
 ***************************************************************************/
#ifdef _NO_PROTO

static void CheckRestartTime() ;
static void CheckDisplayStatus() ;
static void ChildNotify() ;
static void MarkDisplay() ;
static void MarkShutdownTime() ;
static void ProcessChildDeath() ;
static void RescanIfMod() ;
static void RescanNotify() ;
static void RescanServers() ;
static void RestartDisplay() ;
static void ResetLocalGreeter() ;
static void ScanServers() ;
static void SetAccessFileTime() ;
static void SetConfigFileTime() ;
static int  StartGetty() ;
static void StopAll() ;
static int  StorePid() ;
static void TerminateProcess() ;
static void UnlockPidFile() ;
static void dtMakeDefaultDirs();
static void dtmkdir();

#else

static void CheckDisplayStatus( struct display *d) ;
static void CheckRestartTime( void ) ;
static void ChildNotify( int arg ) ;
static void MarkDisplay( struct display *d) ;
static void MarkShutdownTime( void ) ;
static void ProcessChildDeath( int pid, waitType status) ;
static void RescanIfMod( void ) ;
static void RescanNotify( int arg ) ;
static void RescanServers( void ) ;
static void RestartDisplay( struct display *d, int forceReserver) ;
static void ResetLocalGreeter( struct display *d) ;
static void ScanServers( void ) ;
static void SetAccessFileTime( void ) ;
static void SetConfigFileTime( void ) ;
static int  StartGetty( struct display *d) ;
static void StopAll( int arg ) ;
static int  StorePid( void ) ;
static void TerminateProcess( int pid, int sig) ;
static void UnlockPidFile( void ) ;
static void dtMakeDefaultDirs( void );
static void dtmkdir( char *dir, mode_t dir_mode );

#endif /* _NO_PROTO */



/***************************************************************************
 *
 *  Global variables
 *
 ***************************************************************************/
struct passwd   puser;		/* pseudo-user password entry		   */

int		Rescan;
static long	ServersModTime, ConfigModTime, AccessFileModTime;
int		wakeupTime = -1;
char		*progName;

char		DisplayName[32]="main";

#ifdef OSFDEBUG
int nofork_session = 0;
#endif
int server_pid_rem_greet = -1;  /* server pid when remote greeter or chooser 
				   is started */
char* server_display_rem_greet = NULL;  /* server display when remote greeter
					   or chooser is started */
#ifndef NOXDMTITLE
char *Title;	/* Global argv[0] */
int TitleLen;
#endif



/***************************************************************************/

int 
#ifdef _NO_PROTO
main( argc, argv )
        int argc ;
        char **argv ;
#else
main( int argc, char **argv )
#endif /* _NO_PROTO */
{
    int	oldpid, oldumask;
#ifndef SYSV
    SIGVAL              ChildNotify ();
#endif

    struct passwd   *p;		/* pointer to  passwd structure (pwd.h)	   */
    
    /*
     *  make sure at least world write access is disabled...
     */
    if ( (oldumask = umask(022) & 002) == 002)
	(void) umask(oldumask);

#ifndef NOXDMTITLE
    Title = argv[0];
    TitleLen = (argv[argc - 1] + strlen(argv[argc - 1])) - Title;
#endif

    
    /*
     * save program name and path...
     */
     
    if ( (progName = malloc(strlen(argv[0]) + 1)) != NULL )
	strcpy(progName, argv[0]);

#if defined(USL) || defined(__uxp__)
    /* create master slave pair for use in login */
    if (makepttypair () < 0)
        {
        Debug ("Could not create pty for use in login");
        exit (1);
        }
#endif

     
    /*
     * Step 1 - load configuration parameters
     */
    InitResources (argc, argv);
    SetConfigFileTime ();
    LoadDMResources ();
    /*
     * Only allow root to run xdm to avoid problems (HP 700/X version)
     */
    if (debugLevel == 0 && getuid() != 0)
    {
	fprintf(stderr,
		(char *)ReadCatalog(MC_ERROR_SET,MC_BAD_ROOT,MC_DEF_BAD_ROOT),
		argv[0]);
	exit (1);
    }

    /* Make a "/var/dt" directory if it doesn't already exist. */

    dtmkdir("/var/dt", 0755); 

    CheckErrorFile();   /** verify that we can open an error log **/

#ifdef OSFDEBUG
    if (debugLevel >= 10)
      nofork_session = 1;
#endif

    if (debugLevel == 0 && daemonMode)
	    BecomeDaemon ();
    if ( (oldpid = StorePid ()) != 0 )
    {
	if (oldpid == -1)
	    LogError(
		ReadCatalog(MC_LOG_SET,MC_LOG_NO_CREATE,MC_DEF_LOG_NO_CREATE),
		pidFile);
	else
	    LogError(ReadCatalog(MC_LOG_SET,MC_LOG_NO_LOCK,MC_DEF_LOG_NO_LOCK),
		 pidFile, oldpid);
	exit (1);
    }


    /*
     * Check if we are restarting too fast...
     */
     
    CheckRestartTime();


    /* Make rest of the default directories */ 

    dtMakeDefaultDirs();


    /*
     * Initialize error file, open XDMCP socket, and set up interrupt handlers.
     */
     
    InitErrorLog ();
    CreateWellKnownSockets ();
    (void) signal (SIGTERM, StopAll);
    (void) signal (SIGINT, StopAll);

    /*
     * By default, set pseudo-user to "root". Xserver will be run as root
     * if not otherwise specified in the Xservers file.
     */
     
    if ( (p = getpwnam ("root")) != NULL) {
	puser = *p;
    } else {
	/*
	 * This should not happen, the "root" user should always be present.
	 * If it does, fall back to traditional values of the "root" user
	 */
         puser.pw_uid = 0;
         puser.pw_gid = 1;
    }

    /* 
     * Ensure all of root's associated groups are setup correctly
     * before continuing with authentication of user's passwd.
     * This is a required setup call starting with Solaris 2.6.
     * Without it here, the later initgroups call for the user 
     * credential setup may not correctly find all of the user's 
     * groups.
     */

     initgroups("root", puser.pw_gid);


#ifdef __PASSWD_ETC
    /*
     *  Ensure the interrupt handlers are set. The Passwd Etc. libraries 
     *  (getpwnam()) disable SIGTERM and SIGINT.
     */
     
    (void) signal (SIGTERM, StopAll);
    (void) signal (SIGINT, StopAll);
#endif



    /*
     * Step 2 - Read /etc/Xservers and set up
     *	    the socket.
     *
     *	    Keep a sub-daemon running
     *	    for each entry
     */
    SetAccessFileTime ();
    ScanAccessDatabase ();
#if !defined (ENABLE_DYNAMIC_LANGLIST)
    MakeLangList();
#endif /* ENABLE_DYNAMIC_LANGLIST */
    ScanServers ();
    StartDisplays ();
    (void) signal (SIGHUP, RescanNotify);
#if !defined(SYSV) || defined(hpux) || defined(_AIX) || defined(__uxp__)|| defined (__osf__)
    (void) signal (SIGCHLD, ChildNotify);
#endif
    while (AnyWellKnownSockets() || AnyDisplaysLeft ())
    {
	if (Rescan)
	{
	    RescanServers ();
	    Rescan = 0;
	}

	TrimErrorFile();

#if defined(SYSV) && !defined(hpux) && !defined(_AIX) && !defined(__uxp__)  && !defined (__osf__)
	WaitForChild ();
#else
	WaitForSomething ();
#endif
    }
    UnlockPidFile();
    MarkShutdownTime();
    unlink(REMOTE_GREET_HOST_FILE); 
    Debug ("Nothing left to do, exiting\n");
}

static SIGVAL
#ifdef _NO_PROTO
RescanNotify()
#else
RescanNotify( int arg )
#endif /* _NO_PROTO */
{
    Debug ("Caught SIGHUP\n");
    Rescan = 1;
#if defined(SYSV) || defined(SVR4)
    signal (SIGHUP, RescanNotify);
#endif
}

static void
#ifdef _NO_PROTO
ScanServers()
#else
ScanServers( void )
#endif /* _NO_PROTO */
{
    char	lineBuf[10240];
    int		len;
    FILE	*serversFile;
    struct stat	statb;
    static DisplayType	acceptableTypes[] =
	    { { Local, Permanent, FromFile },
	      { Foreign, Permanent, FromFile },
	    };

#define NumTypes    (sizeof (acceptableTypes) / sizeof (acceptableTypes[0]))

    if (servers[0] == '/')
    {
	serversFile = fopen (servers, "r");
	if (serversFile == NULL)
 	{
	    LogError(
		ReadCatalog(MC_LOG_SET,MC_LOG_NO_SRVACC,MC_DEF_LOG_NO_SRVACC),
		servers);
	    return;
	}
	if (ServersModTime == 0)
	{
	    fstat (fileno (serversFile), &statb);
	    ServersModTime = statb.st_mtime;
	}
	while (fgets (lineBuf, sizeof (lineBuf)-1, serversFile))
	{
	    len = strlen (lineBuf);
	    if (lineBuf[len-1] == '\n')
		lineBuf[len-1] = '\0';
	    ParseDisplay (lineBuf, acceptableTypes, NumTypes, &puser);
	}
	fclose (serversFile);
    }
    else
    {
	ParseDisplay (servers, acceptableTypes, NumTypes, &puser);
    }
}

static void 
#ifdef _NO_PROTO
MarkDisplay( d )
        struct display *d ;
#else
MarkDisplay( struct display *d )
#endif /* _NO_PROTO */
{
    d->state = MissingEntry;
}

static void 
#ifdef _NO_PROTO
RescanServers()
#else
RescanServers( void )
#endif /* _NO_PROTO */
{
    Debug ("Rescanning servers\n");

    LogInfo (ReadCatalog(MC_ERROR_SET,MC_LOG_REBUILD,MC_DEF_LOG_REBUILD),
    	      DEF_NLS_DIR);
#if !defined (ENABLE_DYNAMIC_LANGLIST)
    MakeLangList();
#endif /* ENABLE_DYNAMIC_LANGLIST */

    LogInfo (ReadCatalog(MC_ERROR_SET,MC_LOG_RESCAN,MC_DEF_LOG_RESCAN));
    ForEachDisplay (MarkDisplay);
    ReinitResources ();
    LoadDMResources ();
    ScanServers ();
    SetAccessFileTime ();
    ScanAccessDatabase ();
    StartDisplays ();
    ForEachDisplay (ResetLocalGreeter);
}

static void
#ifdef _NO_PROTO
SetConfigFileTime()
#else
SetConfigFileTime( void )
#endif /* _NO_PROTO */
{
    struct stat	statb;

    if (stat (config, &statb) != -1)
	ConfigModTime = statb.st_mtime;
}


static void 
#ifdef _NO_PROTO
SetAccessFileTime()
#else
SetAccessFileTime( void )
#endif /* _NO_PROTO */
{
    struct stat	statb;

    if (stat (accessFile, &statb) != -1)
	AccessFileModTime = statb.st_mtime;
}


static void
#ifdef _NO_PROTO
RescanIfMod()
#else
RescanIfMod( void )
#endif /* _NO_PROTO */
{
    struct stat	statb;

    if (stat (config, &statb) != -1)
    {
	if (statb.st_mtime != ConfigModTime)
	{
	    Debug ("Config file %s has changed, rereading\n", config);
	    LogInfo(
		ReadCatalog(MC_ERROR_SET,MC_LOG_REREADCFG,MC_DEF_LOG_REREADCFG),
		config);
	    ConfigModTime = statb.st_mtime;
	    ReinitResources ();
	    LoadDMResources ();
	}
    }
    if (servers[0] == '/' && stat(servers, &statb) != -1)
    {
	if (statb.st_mtime != ServersModTime)
	{
	    Debug ("Servers file %s has changed, rescanning\n", servers);
	    LogInfo(
		ReadCatalog(MC_ERROR_SET,MC_LOG_REREADSRV,MC_DEF_LOG_REREADSRV),
		servers);
	    ServersModTime = statb.st_mtime;
	    ForEachDisplay (MarkDisplay);
	    ScanServers ();
	}
    }

    if (accessFile && accessFile[0] && stat (accessFile, &statb) != -1)
    {
	if (statb.st_mtime != AccessFileModTime)
	{
	    Debug ("Access file %s has changed, rereading\n", accessFile);
	    LogInfo(
		ReadCatalog(MC_ERROR_SET,MC_LOG_REREADACC,MC_DEF_LOG_REREADACC),
		accessFile);
	    AccessFileModTime = statb.st_mtime;
	    ScanAccessDatabase ();
	}
    }
    
}

/*
 * catch a SIGTERM, kill all displays and exit
 */

#ifdef __osf__
static int    dt_shutdown = 0;
#else
static int	shutdown = 0;
#endif

static SIGVAL
#ifdef _NO_PROTO
StopAll()
#else
StopAll( int arg )
#endif /* _NO_PROTO */
{
    Debug ("Shutting down entire manager\n");
    DestroyWellKnownSockets ();
#ifdef __osf__
    dt_shutdown = 1;
#else
    shutdown = 1;
#endif
    MarkShutdownTime();
    ForEachDisplay (StopDisplay);

    if(server_pid_rem_greet != -1)
       TerminateProcess(server_pid_rem_greet, SIGTERM);

#if defined(SYSV) || defined(SVR4)
    /* to avoid another one from killing us unceremoniously */
    (void) signal (SIGTERM, StopAll);
    (void) signal (SIGINT, StopAll);
#endif
}

/*
 * notice that a child has died and may need another
 * sub-daemon started
 */

int	ChildReady = 0;

#if !defined(SYSV) || defined(hpux) || defined(_AIX) || defined(__uxp__) || defined (__osf__) 
static SIGVAL
#ifdef _NO_PROTO
ChildNotify()
#else
ChildNotify( int arg )
#endif /* _NO_PROTO */
{
    ChildReady = 1;
}
#endif


/*
    In HP-UX, SIGCHLDs are queued rather than lost if we are in the middle
    of processing one (see SIGNAL(5), WARNINGS). The following code relies
    upon this.
    
    If we have a socket, then we are using "select" to block
    (WaitForSomething) rather than "wait". If a child dies, ChildReady is
    set and the select unblocks. We then loop, processing the child that
    died plus any that die while we are processing others. Finally we
    activate the signal handler again and go around one more time in case a
    child died right before activating the signal handler.
*/

void
#ifdef _NO_PROTO
WaitForChild()
#else
WaitForChild( void )
#endif /* _NO_PROTO */
{
    int		pid;
    waitType	status;
    int		mask;

#if defined(SYSV) || defined(SVR4)  || defined(hpux)

    if (AnyWellKnownSockets()) {
	while ( ChildReady ) {
#ifdef SVR4
	   while ((pid = waitpid((pid_t) -1, &status, WNOHANG)) > 0 )
#else
	    while ((pid = wait3 (&status, WNOHANG, NULL)) > 0 )
#endif
		ProcessChildDeath(pid, status);

	    ChildReady = 0;
	    (void) signal (SIGCHLD, ChildNotify);
	    sleep(1);
	}
    }
    else {
	/* XXX classic sysV signal race condition here with RescanNotify */
	if ((pid = wait (&status)) != -1)
	    ProcessChildDeath(pid, status);
    }

#else
    mask = sigblock (sigmask (SIGCHLD) | sigmask (SIGHUP));
    Debug ("Signals blocked, mask was 0x%x\n", mask);
    if (!ChildReady && !Rescan)
	sigpause (mask);
    ChildReady = 0;
    sigsetmask (mask);

    while ((pid = wait3 (&status, WNOHANG, (struct rusage *) 0)) > 0)
	ProcessChildDeath(pid, status);
#endif

    StartDisplays ();
}
        

static void 
#ifdef _NO_PROTO
ProcessChildDeath( pid, status )
        int pid ;
        waitType status ;
#else
ProcessChildDeath( int pid, waitType status )
#endif /* _NO_PROTO */
{
    struct display	*d;
    static int StartChooser = 0;
    static int StartRemoteGreeter = 0;



	Debug ("Processing child death, pid = %d\n", pid);
	if (autoRescan)
	    RescanIfMod ();

	/*
	 * Start local display when remote greeter is exited ...
         */
	if (pid == server_pid_rem_greet) {
               if((d = FindDisplayByName (server_display_rem_greet)) != 0)
                   RestartDisplay(d,TRUE);
        }

	if ( (d = FindDisplayByPid (pid)) != 0 ) {
	    d->pid = -1;

	    /*
	     *  do process accounting...
	     */

	    Account(d, NULL, NULL, pid, DEAD_PROCESS, status);


	    /*
	     *  make sure authorization file is deleted...
	     */
/*
	     
	    if (d->authorization && d->authFile) {
		(void) unlink (d->authFile);
	    }
*/




	    /*
	     *  reset "startTries" ...
	     *  
	     *  Local displays:   Only for clean exits of the server
	     *  Foreign displays: Always except for OPENFAILED_DISPLAY
	     *
	     *  Note: if startTries expires and a "getty" is run on the local
	     *        display, startTries will be reset to zero before 
	     *	      attempting to restart the server.
	     */
	
	    switch (waitVal (status)) {
	    case RESERVER_DISPLAY:
		d->startTries = 0;
		break;

	    case OBEYSESS_DISPLAY:
	        if (d->unexpectedServerDeath == 0)
		   d->startTries = 0;
		else
		   d->unexpectedServerDeath=0;
		break;

	    case OPENFAILED_DISPLAY:
		break;

	    default:
		if (d->displayType.location != Local )
		    d->startTries = 0;
		break;

	    }



	    /*
	     *  process exit status...
	     */
	     
	    switch (waitVal (status)) {
	    case UNMANAGE_DISPLAY:
		Debug ("Display exited with UNMANAGE_DISPLAY\n");
		StopDisplay (d);
		break;

	    case OBEYSESS_DISPLAY:
		Debug ("Display exited with (check)OBEYSESS_DISPLAY\n");
		if (d->displayType.lifetime != Permanent || d->status == zombie)
		    StopDisplay (d);
		else
		    RestartDisplay (d, FALSE);
		break;

	    default:
		Debug ("Display exited with unknown status %d\n", waitVal(status));
		LogError ((unsigned char *)"Unknown session exit code %d from process %d\n",
			  waitVal (status), pid);
		StopDisplay (d);
		break;

	    case OPENFAILED_DISPLAY:
		Debug ("Display exited with OPENFAILED_DISPLAY\n");
		if (d->displayType.origin == FromXDMCP)
		    SendFailed (d, "Cannot open display");

		if (d->displayType.location != Local)
		    d->startTries++;

		if (d->displayType.origin == FromXDMCP ||
		    d->status == zombie ||
		    d->startTries >= d->startAttempts)
		    StopDisplay (d);
		else
		    RestartDisplay (d, TRUE);

		break;

	    case RESERVER_DISPLAY:
		Debug ("Display exited with RESERVER_DISPLAY\n");
		if (d->displayType.origin == FromXDMCP || d->status == zombie)
		    StopDisplay(d);
		else
		    RestartDisplay (d, TRUE);
		break;

	    case RETURN_LOGIN_SCREEN:
		Debug ("Display exited with RETURN_LOGIN_SCREEN\n");
		/*
		 * From Chooser going back to the local login window...
                 */
		if(server_pid_rem_greet != -1)
		    TerminateProcess (server_pid_rem_greet, SIGTERM);

		if (d->displayType.origin == FromXDMCP || d->status == zombie)
                    StopDisplay(d); 
		/*
		 * Start local display
		 */
		if((d = FindDisplayByName (server_display_rem_greet)) != 0)
		    RestartDisplay(d,TRUE);
		break;

	    case waitCompose (SIGTERM,0,0):
		Debug ("Display exited on SIGTERM\n");
		if (d->displayType.origin == FromXDMCP || d->status == zombie)
		    StopDisplay(d);
		else
		    RestartDisplay (d, TRUE);
		break;

	    case REMANAGE_DISPLAY:
		Debug ("Display exited with REMANAGE_DISPLAY\n");
		/*
 		 * XDMCP will restart the session if the display
		 * requests it
		 */
		if (d->displayType.origin == FromXDMCP || d->status == zombie)
		    StopDisplay(d);
		else
		    RestartDisplay (d, FALSE);
		break;

	    case SUSPEND_DISPLAY:
		Debug ("Display exited with SUSPEND_DISPLAY\n");
		if (d->displayType.location == Local)
		    StopDisplay(d);
		else
		    RestartDisplay (d, FALSE);

		break;
	    case START_REMOTE_GREET:
                Debug ("Display exited with START_REMOTE_GREET\n");
                if (d->displayType.location == Local) {
                    StopDisplay(d);
                    StartRemoteGreeter = 1;
                }
                break;
 
            case START_CHOOSER:
                Debug ("Display exited with START_CHOOSER\n");
 
                if (d->displayType.location == Local) {
                    StopDisplay(d);
                    StartChooser = 1;
                }
                break;

	    }
	}
	else if ( (d = FindDisplayByServerPid (pid)) != 0 )
	{
	    d->serverPid = -1;

	    /*
	     *  do process accounting...
	     */

	    Account(d, NULL, NULL, pid, DEAD_PROCESS, status);

	    switch (d->status)
	    {
	    case zombie:
		Debug ("Zombie server reaped, removing display %s\n", d->name);
		RemoveDisplay (d);
		break;
	    case phoenix:
		Debug ("Phoenix server arises, restarting display %s\n", d->name);
		d->status = notRunning;
		break;
	    case running:
		Debug ("Server for display %s terminated unexpectedly, status %d\n", d->name, waitVal (status));
		LogError ((unsigned char *)"Server for display %s terminated unexpectedly %d\n",
			   d->name, waitVal(status) );
		d->unexpectedServerDeath=1;
		if (d->pid != -1)
		{
		    Debug ("Terminating session pid %d\n", d->pid);
		    TerminateProcess (d->pid, SIGTERM);
		}		
		break;
	    case notRunning:
		Debug ("Server exited for notRunning session on display %s\n", d->name);
		break;
	    case suspended:
		Debug ("Server for display %s is suspended\n", d->name);
		if(!StartChooser && !StartRemoteGreeter){
                   if (!StartGetty(d))
                       d->status = notRunning;
                }
                else {
                    if (StartRemoteGreeter){
                       StartRemoteGreet_Chooser(d,StartRemoteGreeter);
                       StartRemoteGreeter = 0;
                    }
                    else {
                       StartChooser = 0;
                       StartRemoteGreet_Chooser(d,StartChooser);
                    }
		    d->startTries = 0;
                }
		break;

	    }
	}
	else
	{
	    Debug ("Unknown child termination, status %d\n", waitVal (status));
	}
}

static void 
#ifdef _NO_PROTO
CheckDisplayStatus( d )
        struct display *d ;
#else
CheckDisplayStatus( struct display *d )
#endif /* _NO_PROTO */
{

    if (d->displayType.origin == FromFile)
    {
	switch (d->state) {
	case MissingEntry:
#ifdef __osf__
	    dt_shutdown = 1;
#else
	    shutdown = 1;
#endif
	    StopDisplay (d);
#ifdef __osf__
	    dt_shutdown = 0;
#else
	    shutdown = 0;
#endif
	    break;
	case NewEntry:
	    d->state = OldEntry;
	case OldEntry:
            Debug("Check %s: status=%d wakeupTime=%d\n", d->name,
                  d->status, wakeupTime);
	    if (d->status == suspended && wakeupTime >= 0)
		if ( GettyRunning(d) || (strcmp(d->gettyLine,"??") == 0))
		    if ( wakeupTime == 0 ) {
			Debug("Polling of suspended server %s started.\n",
				d->name);
#ifdef sun
		        /*
			 * This is the time to wait for a user's text based
			 * login after selection of "Command Line Login" from
			 * dtgreet's options menu.  Give it sufficient
			 * time on the slower (Intel in particular) hardware.
			 */
			wakeupTime = 3 * wakeupInterval;
#else
			wakeupTime = (wakeupInterval < 10
					    ?	3 * wakeupInterval
					    :	2 * wakeupInterval );
#endif
		    }
		    else {
			Debug("Polling of suspended server %s stopped.\n",
				d->name);
			wakeupTime = -1;		/* disable polling */
			d->status = notRunning;		/* restart server  */
			d->startTries = 0;
#ifdef __osf__
			if ( !dt_shutdown ) GettyMessage(d,2);
#else
			if ( !shutdown ) GettyMessage(d,2);
#endif
		    }
		else {
		    Debug("Polling of suspended server %s continued.\n",
			   d->name);
		    wakeupTime = wakeupInterval;	/* continue polling*/
		}

	    if (d->status == notRunning)
		StartDisplay (d);
	    break;
	}
    }
}

static void 
#ifdef _NO_PROTO
ResetLocalGreeter( d )
        struct display *d ;
#else
ResetLocalGreeter( struct display *d )
#endif /* _NO_PROTO */
{
    if ( d->displayType.location == Local && d->status == running ) {
	    Debug("Sending SIGHUP to dtlogin local child %d\n", d->pid);
	    kill(d->pid, SIGHUP);
    }
}

void
#ifdef _NO_PROTO
StartDisplays()
#else
StartDisplays( void )
#endif /* _NO_PROTO */
{
    ForEachDisplay (CheckDisplayStatus);
}

void
#ifdef _NO_PROTO
StartDisplay( d )
        struct display *d ;
#else
StartDisplay(
        struct display *d )
#endif /* _NO_PROTO */
{
    waitType  status;
    int	pid;
    char* authFile_str;
    char start_fbconsole[1024];
    char buff[128];

    Debug ("StartDisplay(): %s\n", d->name);

    bzero(&status, sizeof(waitType));
    if (d->authFile == NULL) 
	authFile_str = "NULL"; 
    else
	authFile_str = d->authFile;

    Debug("d->authfile is (StartDisplay) %s\n",authFile_str);

    /*
     * The following call to RemoveDisplay is to catch race conditions during
     * shutdown. There is no point in starting a display if Dtlogin is in the
     * process of shutting down...
     */
#ifdef __osf__
    if (d->displayType.origin == FromFile && dt_shutdown ) {
#else
    if (d->displayType.origin == FromFile && shutdown ) {
#endif
	RemoveDisplay(d);
	return;
    }
    
    {
	/* make a backup of the authFile before loading resources and       */
	/* copy it back to authFile field od display structure for X server */
        /* to reread the host database list on reset		            */
 	/* RK	11.22.93						    */
	char bkup[50];

        bkup[0] = '\0';

	if(d->authFile) 
        strcpy(bkup ,d->authFile);

    LoadDisplayResources (d);
    if(!d->authFile && bkup[0] != '\0') 
     {
	if(d->authFile)
		free(d->authFile);
	d->authFile= (char *) malloc(strlen(bkup) + 1);
	strcpy(d->authFile,bkup);
     }
   }

#ifdef _AIX
    if(d->language == NULL || strlen(d->language) == 0)
        SetDefaultLanguage(d);
#endif /* _AIX */

    if (d->displayType.location == Local)
    {
	/* don't bother pinging local displays; we'll
	 * certainly notice when they exit
	 */
	d->pingInterval = 0;
    	if (d->authorize)
    	{
#ifdef __osf__
	    Debug ("SetLocalAuthorization %s, auth %s\n",
                    d->name, d->authNames);
#else
	    Debug ("SetLocalAuthorization %s, auth %s\n",
		    d->name, d->authName);
#endif

	    SetLocalAuthorization (d);

	    /*
	     * reset the server after writing the authorization information
	     * to make it read the file (for compatibility with old
	     * servers which read auth file only on reset instead of
	     * at first connection)
	     */
	    if (d->serverPid != -1 && d->resetForAuth && d->resetSignal)
		kill (d->serverPid, d->resetSignal);
    	}

#ifndef __apollo
	/*
	 *  initialize d->utmpId. Check to see if anyone else is using
	 *  the requested ID. Always allow the first request for "dt" to
	 *  succeed as utmp may have become corrupted.
	 */

	if (d->utmpId == NULL) {
	    static int firsttime = 1;
	    static char letters[] = "0123456789abcdefghijklmnopqrstuvwxyzz";
	    char *t;	    

	    d->utmpId = malloc(5);
	    strcpy(d->utmpId, UTMPREC_PREFIX);
	    d->utmpId[3] = d->utmpId[4] = '\0';
	    
	    t = letters;
	    
	    do {
		if ( firsttime || UtmpIdOpen(d->utmpId)) {
		    firsttime = 0;
		    break;
		}		
		else {
		    strncpy(&(d->utmpId[strlen(d->utmpId)]), t++, 1);
    		}
	    } while (*t != NULL);

	    if (*t == NULL) {
		Debug ("All DT utmp IDs already in use. Removing display %s\n",
			d->name);
		LogError ((unsigned char *)"All DT utmp IDs already in use. Removing display %s\n",
			d->name);
		RemoveDisplay(d);
		return;
	    }
	}
#endif

	/*
	 *  set d->gettyLine to "console" for display ":0" if it is not 
	 *  already set...
	 */

	if (! d->gettyLine || strlen(d->gettyLine) == 0 ) {
	    char *p;
	    
	    if ( (p = index(d->name,':')) != NULL &&
		 (  strncmp(++p,"0",1) == 0 )) {

		d->gettyLine  = (char *) malloc(8);
		strcpy(d->gettyLine,  "console");
	    }
	    else {
		d->gettyLine  = (char *) malloc(3);
		strcpy(d->gettyLine,  "??");
	    }
	}


	/*
	 *  if gettyLine is set to "console", set gettySpeed to "console" also
	 */
	 	 	
	if (d->gettyLine && (strcmp(d->gettyLine, "console") == 0 ) ) {
	    if (d->gettySpeed) 
		free((char *) d->gettySpeed);

	    d->gettySpeed = (char *) malloc(8);
	    if (d->gettySpeed)
		strcpy(d->gettySpeed, "console");
	}
		    

	/*
	 *  start server. If it cannot be started and this is the console,
	 *  run a getty...
	 */
	 
	Debug("Attempting to start server for %s.  startTries = %d\n", 
	        d->name, d->startTries);

	if (d->serverPid == -1) {
	    static int bootup = 0;
	   
	    while (bootup++ < 5) {
		if (GettyRunning(d)) {
		    sleep(1);  /* allow time for ttymon to finish its setup */
		    GettyMessage(d,3);
		    bootup = 5;
		    break;
		}
		else {
		    sleep(1);
		}
	    }
	}
	
	if (d->serverPid == -1 && 
	    (d->startTries++ >= d->startAttempts ||
	     !StartServer (d)))
	{
	    LogError ((unsigned char *)"Server for display %s can't be started.\n", d->name);

	    Debug("Server for display %s can't be started.\n", d->name);

	    d->serverPid = -1;

	    GettyMessage(d,4);

#ifdef sun
	    RemoveDisplay(d);
#else
	    if (!StartGetty(d))
		RemoveDisplay (d);
#endif

	    return;
	}
    }
    else
    {
	/* this will only happen when using XDMCP */
#ifdef __osf__
        if (d->authorizations)
	    SaveServerAuthorizations (d, d->authorizations, d->authNum);
#else
	if (d->authorization)
	    SaveServerAuthorization (d, d->authorization);
#endif

 	/*
 	 *  Generate a utmp ID address for a foreign display. Use the last
 	 *  four characters of the DISPLAY name, shifting left if they
 	 *  are already in use...
 	 */
 
 	if (d->utmpId == NULL) {
 	    int i;
 	    char *p, *q;
 	    struct utmp *u;
 	    
 	    d->utmpId = malloc(sizeof(u->ut_id) +1);
 
 	    i = strlen (d->name);
 	    if (i >= sizeof (u->ut_id))
 		i -= sizeof (u->ut_id);
 	    else
 		i = 0;
 
 	    for ( p = d->name, q = d->name + i; p <= q; q-- ) {
 		(void) strncpy (d->utmpId, q, sizeof (u->ut_id));
 		d->utmpId[sizeof(u->ut_id)] = '\0';
 		if (UtmpIdOpen(d->utmpId))
 		    break;
 	    }

#ifdef DEF_NETWORK_DEV
	    /*
	     * If "networkDev" does not start with "/dev/" then foreign
	     * accounting is turned off. Return utmpId to NULL.
	     */
            if (networkDev && strncmp(networkDev,"/dev/",5) !=0 ) {
		free(d->utmpId);
		d->utmpId = NULL;
	    }		
#endif	     
 	}
    }

    /*
     *  make sure stderr is pointing to the current error log file...
     */

    SyncErrorFile(0);

    
#ifdef OSFDEBUG
    if (!nofork_session)
	pid = fork ();
    else
	pid = 0;
    switch (pid)
#else
    switch (pid = fork ())
#endif
    {
    case 0:
#ifdef OSFDEBUG
	if (!nofork_session) {
	    CleanUpChild ();
	    signal (SIGPIPE, SIG_IGN);
	}
#else
	CleanUpChild ();
	signal (SIGPIPE, SIG_IGN);
#endif

	/* 
	 * set global display name for Debug()
	 */
	 
	{
	    char *p, *s, *t;
	    
	    p = DisplayName;
	    
	    strncpy(p, d->name, sizeof(DisplayName));
	    DisplayName[sizeof(DisplayName)-1] = '\0';
	    
	    if ( (s = strchr(p,':')) != NULL )
		if ( (t = strchr(p,'.')) != NULL )
		    strcpy(t,s);
	}

	
	SetAuthorization (d);

	/*
	 * do process accounting...
	 */

        {
            char *line = (d->displayType.location==Local) 
                ? d->gettyLine : d->name;
#ifdef DEF_NETWORK_DEV
            if (d->displayType.location != Local &&
                networkDev && !strncmp(networkDev,"/dev/",5))
            {
    	        char *devname;
                int devexists;
                struct stat devinfo;
                devname = networkDev; /* networkDev resource */

                devexists = (lstat(devname,&devinfo)==0);

                if (!devexists && (MK_NETWORK_DEV(devname) < 0)) {
                    Debug("Creation of file '%s' failed:\n  %s (%d)\n",
                        devname,strerror(errno),errno);
                } else {

                    for (line=devname; *line; line++);
                    while (line>devname && *line!='/') line--;
                    if (*line=='/') line++;

                    Debug("Using pseudo-tty %s; line=%s\n",
                        devname,line);
                }
            }
#endif
	    if (!d->useChooser) {
	        Account(d, "LOGIN", line, getpid(), LOGIN_PROCESS, status);
	    }
        }

	if (!WaitForServer (d))
	    exit (OPENFAILED_DISPLAY);

	/*
	 * start the fallback console, if the display is local..
	 * 
	 * if the display is remote, fbconsole should be started on
	 * remote host. 
	 */
#ifdef sun
 	if (d->displayType.location==Local) {
            if (d->authFile && strlen(d->authFile) > 0 ) {
		strcpy(buff, "XAUTHORITY=");
		strcat(buff, d->authFile);
		putenv(buff);
	    }
	    sprintf(start_fbconsole,"%s -d %s &",FBCONSOLE, d->name);

            if(system(start_fbconsole) == -1)
	       Debug("Failed to start the fallback console - %s\n",FBCONSOLE);
	}
#endif

        if (d->useChooser)
            RunChooser (d);
        else
	    ManageSession (d);
	exit (REMANAGE_DISPLAY);
    case -1:
	break;
    default:
	Debug ("Child manager process started for %s. pid = %d\n", 
		d->name, pid);
	d->pid = pid;
	d->status = running;
	break;
    }
}

static void
#ifdef _NO_PROTO
TerminateProcess( pid, sig )
        int pid ;
        int sig ;
#else
TerminateProcess(int pid, int sig )
#endif /* _NO_PROTO */
{
    kill (pid, sig);
#ifdef SIGCONT
    kill (pid, SIGCONT);
#endif
}

/*
 * transition from running to zombie, suspended, or deleted
 */

void 
#ifdef _NO_PROTO
StopDisplay( d )
        struct display *d ;
#else
StopDisplay( struct display *d )
#endif /* _NO_PROTO */
{
    waitType      status;

    bzero(&status, sizeof(waitType));
#ifdef __osf__
    Debug("StopDisplay(): %s, server pid = %d, manager pid = %d, dt_shutdown = %d\n",
	  d->name, d->serverPid, d->pid, dt_shutdown);
#else
    Debug("StopDisplay(): %s, server pid = %d, manager pid = %d, shutdown = %d\n",
    	   d->name, d->serverPid, d->pid, shutdown);
#endif
	   
    if (d->serverPid != -1)
#ifdef __osf__
	if ((d->displayType.location == Local) && !dt_shutdown )  /* don't remove the console */
#else
	if ((d->displayType.location == Local) && !shutdown )	/* don't remove the console */
#endif
	    d->status = suspended;
	else
	    d->status = zombie;	/* be careful about race conditions */

    if (d->pid != -1)
	TerminateProcess (d->pid, SIGTERM);

    if (d->serverPid != -1) {
	TerminateProcess (d->serverPid, d->termSignal);
#ifdef sun
        {
	    int kbd_fd;
 	    int translate=TR_ASCII;

            Debug ("Resetting keyboard\n");

	    if (d->gettyLine && (strcmp(d->gettyLine, "console") == 0 ) ) {
	        if ((kbd_fd = open("/dev/kbd", O_RDONLY, 0)) < 0) {
		    Debug("/dev/kbd open failed\n");
	        } else {
		    if (ioctl(kbd_fd, KIOCTRANS, (caddr_t) &translate)) {
		        Debug("Could not set /dev/kbd back to ASCII mode\n");
		    }
		    close(kbd_fd);
	        }
	    }
	}
#endif
    }
    else 
#ifdef __osf__
        if ((d->displayType.location == Local) || !dt_shutdown ) { /* don't remove the console */
#else
	if ((d->displayType.location != Local) || shutdown) {	/* don't remove the console */
#endif
	    Account(d, NULL, NULL, 0, DEAD_PROCESS, status);
	    RemoveDisplay (d);
	}	    
}


/*
 * transition from running to phoenix or notRunning
 */

static void 
#ifdef _NO_PROTO
RestartDisplay( d, forceReserver )
        struct display *d ;
        int forceReserver ;
#else
RestartDisplay( struct display *d, int forceReserver )
#endif /* _NO_PROTO */
{
    if (d->serverPid != -1 && (forceReserver || d->terminateServer))
    {
	TerminateProcess (d->serverPid, d->termSignal);
	d->status = phoenix;
    }
    else
    {
	d->status = notRunning;
    }
}

static FD_TYPE	CloseMask;
static int	max;

void
#ifdef _NO_PROTO
RegisterCloseOnFork( fd )
        int fd ;
#else
RegisterCloseOnFork( int fd )
#endif /* _NO_PROTO */
{
    FD_SET (fd, &CloseMask);
    if (fd > max)
	max = fd;
}

#if 0		/* utility routine: activate if needed...		   */
int 
#ifdef _NO_PROTO
CloseOnFork()
#else
CloseOnFork( void )
#endif /* _NO_PROTO */
{
    FD_CLR (fd, &CloseMask);
    if (fd == max) {
	while (--fd >= 0)
	    if (FD_ISSET (fd, &CloseMask))
		break;
	max = fd;
    }
}
#endif

CloseOnFork ()
{
    int	fd;

    for (fd = 0; fd <= max; fd++)
	if (FD_ISSET (fd, &CloseMask))
	    close (fd);
    FD_ZERO (&CloseMask);
    max = 0;
}

static int  pidFd;
static FILE *pidFilePtr;

static int 
#ifdef _NO_PROTO
StorePid()
#else
StorePid( void )
#endif /* _NO_PROTO */
{
    int		oldpid;

    if (pidFile && pidFile[0] != '\0') {
	pidFd = open (pidFile, 2);
	if (pidFd == -1 && errno == ENOENT)
	{
	    /*
	     * HP OSF/1 will not allow an fdopen 
	     * of a file descriptor handed back by creat(2).
	     *  The workaround is to close the created file, and 
	     * open it Read/Write.  This will be transparent to HP-UX.
	     */
	    pidFd = creat (pidFile, 0644);
	    close( pidFd );
	    pidFd = open (pidFile, 2);
	}
	if (pidFd == -1 || !(pidFilePtr = fdopen (pidFd, "r+")))
	{
	    LogError ((unsigned char *)"Process ID file %s cannot be opened\n",
		      pidFile);
	    return -1;
	}
	if (fscanf (pidFilePtr, "%d", &oldpid) != 1)
	    oldpid = -1;
	fseek (pidFilePtr, 0l, 0);
	if (lockPidFile)
	{
#if defined (SYSV) || defined (SVR4)
	    if (lockf (pidFd, F_TLOCK, (off_t) 0) == -1)
	    {
		if ((errno == EAGAIN) || (errno == EACCES))
		    return oldpid;
		else
		    return -1;
	    }
#else
	    if (flock (pidFd, LOCK_EX|LOCK_NB) == -1)
	    {
		if (errno == EWOULDBLOCK)
		    return oldpid;
		else
		    return -1;
	    }
#endif
	}

	/*
	 * HPUX releases the lock on ANY close of the file, not just the
	 * one that established the lock. -prr
	 */
	/*	close(creat(pidFile, 0644)); */

	(void) creat(pidFile, 0644);
	fprintf (pidFilePtr, "%d\n", getpid ());
	(void) fflush(pidFilePtr);
	RegisterCloseOnFork(pidFd);
    }
    return 0;
}

static void
#ifdef _NO_PROTO
UnlockPidFile()
#else
UnlockPidFile( void )
#endif /* _NO_PROTO */
{
    if (lockPidFile)
#if defined (SYSV) || defined (SVR4)
	lockf (pidFd, F_ULOCK, 0);
#else
	flock (pidFd, LOCK_UN);
#endif
    close (pidFd);
    fclose (pidFilePtr);
}

#ifdef oldcode
/*VARARGS*/
SetTitle (va_alist)
va_dcl
{
#ifndef NOXDMTITLE
    char	*p = Title;
    int		left = TitleLen;
    char	*s;
    va_list	args;

    va_start(args);
    *p++ = '-';
    --left;
    while (s = va_arg (args, char *))
    {
	while (*s && left > 0)
	{
	    *p++ = *s++;
	    left--;
	}
    }
    while (left > 0)
    {
	*p++ = ' ';
	--left;
    }
    va_end(args);
#endif	
}
#endif

void
#ifdef _NO_PROTO
SetTitle( name, ptr )
        char *name ;
        char *ptr ;
#else
SetTitle( char *name, char *ptr )
#endif /* _NO_PROTO */
{
#ifndef NOXDMTITLE
    char	*p, *s, *t;
    int		length;


    /*
     *  remove domain qualifiers and screens from name...
     */

    if ( (p = malloc(strlen(name) + 1)) == NULL) return;
    strcpy(p, name);

    if ( (s = strchr(p,':')) == NULL ) {
	free(p);
	return;
    }
    
    if ( (t = strchr(s,'.')) != NULL )
	*t = '\0';

    if ( (t = strchr(p,'.')) != NULL )
	strcpy(t,s);
    
    /*
     *  if there is enough room shift program name to left,
     *  then append display name in remaining space.
     */

    s = Title;
    length = strlen(s);
    
    t = strrchr(s, '/');
    if ( (t != NULL) && ((t-s+1) >= (strlen(p) + 3)) ) {
	t++;

	strcpy(s,t);		/* "program"			   */
	strcat(s," <");		/* "program <"			   */
	strcat(s,p);		/* "program <displayName"	   */
	strcat(s,">");		/* "program <displayName>"	   */

	t = s + strlen(s);
	length = length - strlen(s);
	while (length > 0){
	    *t++ = ' ';
	    length--;
	}
    }

    free(p);
#endif
}


/*****************************************************************************
*    StartGetty
*    
*    Start a "getty" running on the console...
*
*****************************************************************************/

#if defined (_AIX) && defined (_POWER) || defined (__osf__)
#define GETTYPATH "/usr/sbin/getty"
#elif !defined (__apollo)
#define GETTYPATH "/etc/getty"
#endif

static int 
#ifdef _NO_PROTO
StartGetty( d )
        struct display *d ;
#else
StartGetty( struct display *d )
#endif /* _NO_PROTO */
{
    int		pid;
    char tynm[20];
    waitType  status;

    Debug ("StartGetty(): %s\n", d->name);

    bzero(&status, sizeof(waitType));
    /*
     * ensure that server is known dead...
     */

    d->serverPid = -1;

#if !defined(GETTYPATH)
    return FALSE;
#else

    /*
     * check to see if we have a valid device (at least a non-null name)...
     */

    if ( d->gettyLine			&& 
        (strlen(d->gettyLine) > 0)	&&
	(strcmp(d->gettyLine,"??") != 0)	)
	;
    else
        return FALSE;


    /*
     * if there is already a getty running on the device, set up
     * to start watching it. When it exits, restart the server...
     */

    if ( GettyRunning(d) ) {
	d->status = suspended;		/* set up to restart server	   */
	wakeupTime = 0;			/* enable polling		   */

	sleep(1); 			/* wait for fbconsole to go away   */
	GettyMessage(d,1);		/* print a help message		   */

	return TRUE;
    }

    
    /*
     * there is no getty running on the device, try to start one...
     */

    d->status = phoenix;		/* set up to restart server	   */
    d->startTries = 0;
    
    switch (pid = fork ()) {
    case 0:
	CleanUpChild ();

	/*
	 *  do process accounting...
	 */
	Account(d, "LOGIN", NULL, getpid(), LOGIN_PROCESS, status);


	#ifdef _AIX
        /* The tty argument for getty on AIX must be of the form "/dev/any tty"
           and so the following logic
           Raghu krovvidi 07.07.93
         */
         strcpy(tynm,"/dev/");
         strcat(tynm,d->gettyLine); 
        #else
         strcpy(tynm, d->gettyLine);
        #endif

        Debug(" execing getty on %s\n",tynm);
	execl(GETTYPATH, "getty", tynm, d->gettySpeed, (char *)0);
	LogError ((unsigned char *)"Can not execute %s for %s: errno = %d\n",
		   GETTYPATH, d->name, errno);

	exit (UNMANAGE_DISPLAY);

    case -1:
	Debug ("Fork of /etc/getty failed %s\n", d->name);
	LogError ((unsigned char *)"Can not fork to execute /etc/getty %s\n", d->name);
	return FALSE;

    default:
	break;
    }

    Debug ("/etc/getty started on %s\n", d->name);
    d->serverPid = pid;
    return TRUE;
#endif /* GETTYPATH not defined */
}

 
/***************************************************************************
 *
 *  GettyMessage
 *
 *  Print a message on the display device when going into No Windows mode.
 *
 ***************************************************************************/

void
#ifdef _NO_PROTO
GettyMessage( d, msgnum )
        struct display *d ;
        int msgnum ;
#else
GettyMessage( struct display *d, int msgnum )
#endif /* _NO_PROTO */
{
    struct stat statb;
    FILE *tf=NULL;
    char buf[128];

    strcpy(buf,"/dev/");
    strcat(buf,d->gettyLine);
   
    /* 
     * Print messages to the getty line (usually /dev/console) about the Xserver start/stop
     * only if getty line exists and the system appears to have a frame buffer device (/dev/fb) 
     */  

    if ( stat("/dev/fb", &statb) == 0 && (tf = fopen (buf, "a")) != NULL ) {
	fprintf (tf, 
	  "\r\n\r\n*****************************************************************************\r\n*\r\n");

	switch (msgnum) {
	case 1:
	
	  fprintf(tf,(char *)ReadCatalog(MC_LABEL_SET,MC_SUS1_LABEL,MC_DEF_SUS1_LABEL));

	  fprintf(tf,(char *)ReadCatalog(MC_LABEL_SET,MC_SUS2_LABEL,MC_DEF_SUS2_LABEL));

	  fprintf(tf,(char *)ReadCatalog(MC_LABEL_SET,MC_SUS3_LABEL,MC_DEF_SUS3_LABEL));

	  break;
	  
	case 2:
	
	  fprintf(tf,(char *)ReadCatalog(MC_LABEL_SET,MC_RES_LABEL,MC_DEF_RES_LABEL));

	  break;
	  
	case 3:
	
	  fprintf(tf,
		(char *)ReadCatalog(MC_LABEL_SET,MC_START_LBLDPY,MC_DEF_START_LBLDPY),
		d->name);

	  fprintf(tf,(char *)ReadCatalog(MC_LABEL_SET,MC_WAIT_LABEL,MC_DEF_WAIT_LABEL));

	  break;
	  
	case 4:
	
	  fprintf(tf,(char *)ReadCatalog(MC_LABEL_SET,MC_X_LABEL,MC_DEF_X_LABEL),
	    	d->name);

	  break;

	}
	
	fprintf (tf, 
	  "*****************************************************************************\r\n");
    }
    if (tf != NULL) fclose (tf);
}


/***************************************************************************
 *
 *  GettyRunning
 *
 *  See if a getty process is running against the display device. This
 *  routine may need to be rewritten on other platforms if a different
 *  mechanism is needed to make the determination.
 *
 *  Output:  TRUE  == a login process is active on the requested device
 *           FALSE == a login process is not active on the device.
 *
 *  Sets d->gettyState:
 *    NONE - no getty running or don't care
 *    LOGIN - getty running
 *    USER - user logged in on getty
 *
 *  Note: The check for a getty process running is made by scanning
 *	  /etc/utmp, looking for a login process on the respective device.
 *	  However, the child Dtlogin spawned by the master Dtlogin to
 *	  handle this display also is a "login process" according to
 *	  /etc/utmp.  It provides a login path and therefore must register
 *	  itself as so.  If a getty is also running, there are actually two
 *	  login processes running against the same device at the same time.
 *
 *	  The child Dtlogin dies before the scan of /etc/utmp is made.
 *	  Provided /etc/utmp is updated correctly, the Dtlogin entry will
 *	  be marked as dead and will not show up in the scan of /etc/utmp.
 ***************************************************************************/

int 
#ifdef _NO_PROTO
GettyRunning( d )
        struct display *d ;
#else
GettyRunning( struct display *d )
#endif /* _NO_PROTO */
{
    struct utmp utmp;		/* local struct for new entry	   	   */
    struct utmp *u;		/* pointer to entry in utmp file	   */
    
    int		rvalue;		/* return value (TRUE or FALSE)		   */
    char	buf[32];
        
    d->gettyState = DM_GETTY_NONE;

    /*
     * check to see if we have a valid device (at least a non-null name)...
     */

    if ( d->gettyLine			&& 
        (strlen(d->gettyLine) > 0)	&&
	(strcmp(d->gettyLine,"??") != 0)	)
	;
    else
        return FALSE;


    bzero(&utmp, sizeof(struct utmp));

#ifdef _AIX
   if (!strcmp(d->gettyLine,"console")) {
        char *ttynm;
        int fd=0;

        fd = open("/dev/console",O_RDONLY);
        ttynm = ttyname(fd);
        ttynm += 5;
        strcpy(utmp.ut_line,ttynm);
        close(fd);
    }
    else
        strncpy(utmp.ut_line, d->gettyLine, sizeof(utmp.ut_line));

#else
    strncpy(utmp.ut_line, d->gettyLine, sizeof(utmp.ut_line));
#endif
    
    Debug("Checking for a getty on line %s.\n", utmp.ut_line);
    
    setutent();

    rvalue = FALSE;
    
    while ( (u = getutent()) != NULL ) {
    
        if ((strncmp(u->ut_line, utmp.ut_line, sizeof(u->ut_line)) != 0) ||
            (strncmp(u->ut_id,   d->utmpId,    sizeof(u->ut_id))   == 0) )
	    continue;

	switch (u->ut_type) {
    
	case INIT_PROCESS:	strcpy(buf, "INIT_PROCESS");	break;
	case LOGIN_PROCESS:	strcpy(buf, "LOGIN_PROCESS");	break;
	case USER_PROCESS:	strcpy(buf, "USER_PROCESS");	break;
	case DEAD_PROCESS:	strcpy(buf, "DEAD_PROCESS");	break;
	default:		strcpy(buf, "UNKNOWN");		break;
	}

	Debug("Utmp info: id=%.4s, user=%s, line=%s, pid=%d, type=%s\n",
	       u->ut_id, u->ut_user, u->ut_line, u->ut_pid, buf);

	if ( u->ut_type == INIT_PROCESS || u->ut_type == LOGIN_PROCESS) {
          d->gettyState = DM_GETTY_LOGIN;
        }
        else if (wakeupTime <= 0 && u->ut_type == USER_PROCESS) {
          d->gettyState = DM_GETTY_USER;
        }
     
        if (d->gettyState != DM_GETTY_NONE)
        {
	    rvalue = TRUE;
	    break;
	}
    }

    endutent();
    return rvalue;
}


 
/***************************************************************************
 *
 *  CheckRestartTime
 *
 *  Check if enough time has elapsed since shutdown. 
 *
 *  This is primarily to work with /etc/shutdown.  When shutdown kills
 *  dtlogin (/etc/killall), init immediately restarts it.  Shutdown kills
 *  it again and init restarts it.  At each restart, the X-server may start
 *  on the local display and then subsequently be killed.  The user sees a
 *  flashing screen and sometimes the console is left in an unreset state.
 *
 *  When Dtlogin shuts down, it touches the access time on the Xservers
 *  file.  (MarkShutdownTime()).  This time is then used to determine if
 *  sufficient time has elapsed before restarting.
 *
 ***************************************************************************/

static void
#ifdef _NO_PROTO
CheckRestartTime( )
#else
CheckRestartTime( void )
#endif /* _NO_PROTO */
{
    struct stat	statb;
    int		sleeptime;
    
    if (servers[0] == '/' && stat(servers, &statb) != -1) {

	Debug("Checking restart time.\n");
	
#ifdef OSFDEBUG
/* only those other systems are this slow :-) */
        sleeptime = 6 - (int) (time((time_t *) 0) - statb.st_atime);
#else
	sleeptime = 30 - (int) (time((time_t *) 0) - statb.st_atime);
#endif
	
	if ( sleeptime > 30 ) sleeptime = 30;
	
	if ( sleeptime > 0 ) {
	    Debug("Restarting too soon after shutdown. Sleeping %d seconds.\n",
	           sleeptime);
	    sleep (sleeptime);
	}
    }

}

 
/***************************************************************************
 *
 *  MarkShutdownTime
 *
 *  Save the time when we shut down to check later for too fast of a restart.
 *
 ***************************************************************************/

static void
#ifdef _NO_PROTO
MarkShutdownTime( )
#else
MarkShutdownTime( void )
#endif /* _NO_PROTO */
{
    struct stat	statb;
    struct utimbuf timebuf;
    
    if (servers[0] == '/' && stat(servers, &statb) != -1) {

	Debug("Marking shutdown time.\n");

	timebuf.actime = time((time_t *) 0 );
	timebuf.modtime = statb.st_mtime;

	if ( (utime(servers, &timebuf)) != 0 ) {
	    Debug("MarkShutdownTime(): utime() error = %d\n", errno);
	}
    }
}


/***************************************************************************
 *
 * dtmkdir 
 *
 * Create the specified default dt directory 
 *
 ***************************************************************************/
static void
#ifdef _NO_PROTO
dtmkdir ( dir , dir_mode)
        char *dir;
	mode_t dir_mode;
#else
dtmkdir ( char *dir, mode_t dir_mode )
#endif /* _NO_PROTO */
{
    struct stat file_status;

    if ( stat(dir, &file_status) != 0) {
        /** try to create it **/
        if ( mkdir(dir, dir_mode) == 0) {
            chmod(dir, dir_mode);  /** since umask is non-zero **/ 
            Debug("Created dir %s\n", dir);
        } else {
            LogError((unsigned char *)"Unable to create %s\n", dir);
        }
    } else {
        if ( (file_status.st_mode & dir_mode) != dir_mode ) {
            /** try to set correct permissions **/
            if ( chmod(dir, file_status.st_mode | dir_mode) == 0) {
                Debug("Set permissions on %s\n", dir);
            } else {
                LogError((unsigned char *)
				"Unable to set permissions on %s\n", dir);
            }
        }
    }
}


/***************************************************************************
 *
 * dtrm_contents
 *
 * Remove contents of the specified directory
 *
 * Return status 
 *	0 = Good
 *     -1 = Error
 *
 ***************************************************************************/
static int 
#ifdef _NO_PROTO
dtrm_contents( dir )
        char *dir;
#else
dtrm_contents( char *dir )
#endif /* _NO_PROTO */
{
	DIR *dp;
	struct dirent *d_entry;

	dp = opendir(dir);
        if (dp == NULL) {
            if (dir == NULL) {
	 	Debug("dtrm_contents: NULL dir\n");
	    }
	    else {
	 	Debug("dtrm_contents: could not open %s\n", dir);
            }
	    return(-1);
	}

	/* Remove all contents */

	while ( (d_entry = readdir(dp)) != NULL ) {
	    if ( d_entry->d_name == NULL ) {
		Debug("dtrm_contents: null file name\n");
		closedir(dp);
		return(-1);
	    }

	    if ( strcmp(d_entry->d_name, ".") != 0 &&
		 strcmp(d_entry->d_name, "..") != 0 ) { 

	        char path[MAXPATHLEN]; 
		struct stat statb;

	    	sprintf(path, "%s/%s", dir, d_entry->d_name);

		/* If a directory was found, delete its contents */

	        if ( lstat(path, &statb) == 0 && 
		     S_ISDIR(statb.st_mode) &&
		     dtrm_contents(path) == -1 ) {
		    closedir(dp);
		    return(-1);
	        }

		/* Remove next file or directory */

		if (remove(path) == -1) {
		    Debug("dtrm_contents: could not remove %s\n", path);
		    closedir(dp);
		    return(-1);
		} else {
		    rewinddir(dp);
		}
	    }
	}

	Debug("Removed old contents of %s\n", dir);

	closedir(dp);
	return(0);
}


/***************************************************************************
 *
 *  dtMakeDefaultDirs
 *
 * Make the default dt directorys under "/var/dt" if needed.
 *
 ***************************************************************************/
static void
#ifdef _NO_PROTO
dtMakeDefaultDirs( )
#else
dtMakeDefaultDirs( void )
#endif
{
    dtmkdir("/var/dt/tmp", 01777);
    dtmkdir(SDTLOGIN_DIR, 0700);
    dtmkdir("/var/dt/appconfig", 0755);
    dtmkdir("/var/dt/appconfig/appmanager", 0755);

    dtrm_contents("/var/dt/tmp");
    dtrm_contents(SDTLOGIN_DIR);
    dtrm_contents("/var/dt/appconfig/appmanager");
}
