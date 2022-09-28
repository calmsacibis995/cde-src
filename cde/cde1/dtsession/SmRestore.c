/* $XConsortium: SmRestore.c /main/cde1_maint/3 1995/10/09 10:57:40 pascale $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmRestore.c
 **
 **  Project:     HP DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  This file contains functions that are in charge of restoring state.
 **  When the session manager is first started, it restores the state that
 **  was either last saved (home state), or last exited (current state).  The
 **  state restored depends on values the user has configured.
 **
 **
 **
 *******************************************************************
 **  (c) Copyright Hewlett-Packard Company, 1990.  All rights are  
 **  reserved.  Copying or other reproduction of this program      
 **  except for archival purposes is prohibited without prior      
 **  written consent of Hewlett-Packard Company.		     
 ********************************************************************
 **
 **
 **
 *****************************************************************************
 *************************************<+>*************************************/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#ifdef __apollo
#include <X11/apollosys.h>        /* for pid_t struct in hp-ux sys/types.h */
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>

#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/keysymdef.h>
#include <Xm/Xm.h>

#ifdef _NO_PROTO
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#else
# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)
#endif

#include <Dt/DtP.h>
#include <Dt/Message.h>
#include <Dt/CommandM.h>
#include <Dt/Connect.h>
#include <Dt/Wsm.h>
#include <Dt/WsmP.h>
#include <Dt/UserMsg.h>
#include <Dt/SessionM.h>
#include <Dt/CmdInv.h>
#include <Dt/EnvControl.h>

#include "Sm.h"
#include "SmResource.h"
#include "SmError.h"
#include "SmCommun.h"
#include "SmRestore.h"
#include "SmProtocol.h"
#include "SmConMgmt.h"
#include "SmSave.h"
#include "SmUI.h"
#include "SmGlobals.h"

#include <dirent.h>

/* Busy is also defined in the BMS  -> bms.h. This conflicts with
 * /usr/include/X11/Xasync.h on ibm.
 */
#ifdef _AIX
#ifdef Busy
#undef Busy
#endif
#endif

#include <X11/Xlibint.h>

/*
 * #define statements
 */
#define MAXLINE     1024
#define MAXPATHSM   1023
#define MAX_QUOTE_DEPTH	10

#define SM_MAX_ARGS MAXLINE

#if defined (SVR4) || defined (__osf__)
#define REMOTE_CMD_STRING "rsh %s -n %s &"
#else
#define REMOTE_CMD_STRING "remsh %s -n %s &"
#endif

#define DEFAULT_FONT_PATH  "/usr/openwin/lib/X11/sdtfonts/"
#define SYS_FONT_PATH      "/etc/dt/sdtfonts/"
#define USER_FONT_DIR      "/.dt/sdtfonts/"
#define XSET  	           "/usr/openwin/bin/xset"





/*
 * typedef statements
 */
typedef struct
{
    unsigned char *hostPtr;
    unsigned char *cmdPtr;
} RemoteReq;

/*
 * Variables global to this module only
 */
static XtResource settingsResources[] =
{
   {SmNaccelNum, SmCaccelNum, XtRInt, sizeof(int),
     XtOffset(SessionSettingsPtr, accelNum),
     XtRImmediate, (XtPointer) -1},
   {SmNaccelDenom, SmCaccelDenom, XtRInt, sizeof(int),
     XtOffset(SessionSettingsPtr, accelDenom),
     XtRImmediate, (XtPointer) -1},
   {SmNthreshold, SmCthreshold, XtRInt, sizeof(int),
     XtOffset(SessionSettingsPtr, threshold),
     XtRImmediate, (XtPointer) -1},

   {SmNtimeout, SmCtimeout, XtRInt, sizeof(int),
     XtOffset(SessionSettingsPtr, timeout),
     XtRImmediate, (XtPointer) -1},
   {SmNinterval, SmCinterval, XtRInt, sizeof(int),
     XtOffset(SessionSettingsPtr, interval),
     XtRImmediate, (XtPointer) -1},
   {SmNpreferBlank, SmCpreferBlank, XtRInt, sizeof(int),
     XtOffset(SessionSettingsPtr, preferBlank),
     XtRImmediate, (XtPointer) 0},   
   {SmNallowExp, SmCallowExp, XtRInt, sizeof(int),
     XtOffset(SessionSettingsPtr, allowExp),
     XtRImmediate, (XtPointer) 0},
   
   {SmNfontPath, SmCfontPath, XtRString, sizeof(String),
         XtOffset(SessionSettingsPtr, fontDirs), XtRString, (XtPointer) ""},
   
   {SmNkeyClick, SmCkeyClick, XtRInt, sizeof(int),
	XtOffset(SessionSettingsPtr, kbdState.key_click_percent),
	XtRImmediate, (XtPointer) -1}, 
   {SmNbellPercent, SmCbellPercent, XtRInt, sizeof(int),
	XtOffset(SessionSettingsPtr, kbdState.bell_percent),
	XtRImmediate, (XtPointer) -1}, 
   {SmNbellPitch, SmCbellPitch, XtRInt, sizeof(int),
	XtOffset(SessionSettingsPtr, kbdState.bell_pitch),
	XtRImmediate, (XtPointer) -1},
   {SmNbellDuration, SmCbellDuration, XtRInt, sizeof(int),
	XtOffset(SessionSettingsPtr, kbdState.bell_duration),
	XtRImmediate, (XtPointer) -1},
   {SmNledMask, SmCledMask, XtRInt, sizeof(int),
	XtOffset(SessionSettingsPtr, kbdState.led_mask),
	XtRImmediate, (XtPointer) 0},
   {SmNglobalRepeats, SmCglobalRepeats, XtRInt, sizeof(int),
	XtOffset(SessionSettingsPtr, kbdState.global_auto_repeat),
	XtRImmediate, (XtPointer) 2},
   {SmNautoRepeats, SmCautoRepeats, XtRString, sizeof(String),
	XtOffset(SessionSettingsPtr, autoRepeats), XtRString, (XtPointer) ""},
   
   {SmNbuttonMap, SmCbuttonMap, XtRString, sizeof(String),
         XtOffset(SessionSettingsPtr, buttonMap), XtRString, (XtPointer) ""},
   
   {SmNnumKeyCode, SmCnumKeyCode, XtRInt, sizeof(int),
	XtOffset(SessionSettingsPtr, numKeyCode), XtRImmediate, (XtPointer) 0},
   {SmNkeySymsPerKey, SmCkeySymsPerKey, XtRInt, sizeof(int),
	XtOffset(SessionSettingsPtr, keySymPerCode),
	XtRImmediate, (XtPointer) 0},
   {SmNkeySyms, SmCkeySyms, XtRString, sizeof(String),
	XtOffset(SessionSettingsPtr, keySyms), XtRString, (XtPointer) ""},

   {SmNmaxKeyPerMod, SmCmaxKeyPerMod, XtRInt, sizeof(int),
	XtOffset(SessionSettingsPtr, maxKeyPerMod),
	XtRImmediate, (XtPointer) 0},
   {SmNmodMap, SmCmodMap, XtRString, sizeof(String),
	XtOffset(SessionSettingsPtr, modSyms),
	XtRString, (XtPointer) ""}, 

   {SmNdidQuerySettings, SmCdidQuerySettings, XtRBoolean, sizeof(Boolean),
     XtOffset(SessionSettingsPtr, didQuery),
     XtRImmediate, (XtPointer) False},
   {SmNshutDownState, SmCshutDownState, XtRInt, sizeof(int),
     XtOffset(SessionSettingsPtr, confirmMode),
     XtRImmediate, (XtPointer) 1},
   {SmNshutDownMode, SmCshutDownMode, XtRInt, sizeof(int),
     XtOffset(SessionSettingsPtr, startState),
     XtRImmediate, (XtPointer) 4},
};

/*
 * Variables used for parsing code
 */

static unsigned char  fallBackLine[MAXLINE+2]; /* line buffer */
static unsigned char  *line;  /* line buffer */
static FILE *cfileP = NULL;   /* fopen'ed configuration file or NULL */
static int   linec = 0;       /* line counter for parser */
static unsigned char *parseP = NULL;   /* pointer to parse string */
static off_t fileSize = 0;
char **smExecArray = NULL;

/*
 * Variables used for remote execution
 */
static int	numRemoteExecs = 0;
unsigned char 	*remoteBuf[MAX_SCREENS_SAVED];
unsigned int  	actRemoteSize[MAX_SCREENS_SAVED];
RemoteReq	remoteBufPtr[MAX_REMOTE_CLIENTS];

/*
 * Timeout for workspace manager handshake
 */
static Boolean	wmTimeout;

/* 
 * These lines were added to support the builtin
 * panacomm dtwm.
 */

char   tmpExecWmFile[MAXPATHSM+1];
static Boolean  localWmLaunched = False;

/* 
 *     ^^^^^  ^^^^^  ^^^^^  ^^^^^  ^^^^^  ^^^^^  
 * End of  lines were added to support the builtin
 * panacomm dtwm.
 */



/*
 * Local functions
 */
#ifdef _NO_PROTO

static void SystemCmd();
static int RestoreSettings() ;
static int RestoreClients() ;
static unsigned char * GetNextLine() ;
static unsigned int PeekAhead() ;
static unsigned char * GetSmartString() ;
static void ForkWM() ;
static int FillCmdBuf() ;
static int FillRemoteBuf() ;
static int FillHintBuf() ;
static void WaitForWM() ;
static void HandleWMClientMessage();
static void WaitWMTimeout() ;
static void FixEnvironmentData() ;
static void ResetScreenInfo();
static void RemoteRequestFailed();
static void RemoteRequestSucceeded();
static void SetFontGroup() ;
static void prepend_iso8859() ;

#else

static void SystemCmd (char *pchCmd);
static int RestoreSettings( void ) ;
static int RestoreClients( void ) ;
static unsigned char * GetNextLine( void ) ;
static unsigned int PeekAhead( unsigned char *, unsigned int ) ;
static unsigned char * GetSmartString( unsigned char **) ;
static void ForkWM( void ) ;
static int FillCmdBuf( unsigned char *, unsigned char **, 
			unsigned int *, unsigned int *, 
			unsigned int, unsigned int *) ;
static int FillRemoteBuf( unsigned char *, unsigned char *, 
			  unsigned char *, unsigned char **,
                          unsigned int *, unsigned int *, unsigned int) ;
static int FillHintBuf(unsigned char *, unsigned char **,
		       unsigned int  *, unsigned int  *,
		       unsigned int, unsigned int *);
static void WaitForWM( void ) ;
static void HandleWMClientMessage(Widget smWidget, XtPointer dummy,
                                XEvent *event);
static void WaitWMTimeout( XtPointer , XtIntervalId *) ;
static void FixEnvironmentData( void ) ;
static void ResetScreenInfo(unsigned char **,
			    int *, unsigned int	*,
			    Boolean *, char *,
			    char *);
static void RemoteRequestFailed(char *, void *);
static void RemoteRequestSucceeded(char *, void *);
static void SetFontGroup( void ) ;
static void prepend_iso8859( void ) ;

#endif /* _NO_PROTO */




/*************************************<->*************************************
 *
 *  SetCompatState ()
 *
 *
 *  Description:
 *  -----------
 *  Sets _DT_SM_STATE_INFO for dtstyle indicating compatibility mode
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  The only thing dtstyle should be looking at is the compatMode
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
SetCompatState()
#else
SetCompatState( void )
#endif /* _NO_PROTO */
{
    SmStateInfo	state;

    /*
     * BEFORE any clients are started
     * set a property on the top level window
     * which lets the style manager know what state the sm is in
     */
    state.flags = SM_STATE_ALL;
    state.smStartState = 0;
    state.smConfirmMode = 0;
    state.smCompatMode = True;
    state.smSendSettings = False;
    state.smCoverScreen = True;
    state.smLockOnTimeoutStatus = smGD.lockOnTimeoutStatus;
    state.smCycleTimeout = 0;
    state.smLockTimeout = 0;
    state.smSaverTimeout = 0;
    state.smRandom = 0;

    _DtSetSmState(smGD.display, smGD.topLevelWindow, &state);

    XFlush(smGD.display);

    return(0);

} /* END OF FUNCTION SetCompatState */


/*************************************<->*************************************
 *
 *  SystemCmd (pchCmd)
 *
 *
 *  Description:
 *  -----------
 *  This function fiddles with our signal handling and calls the
 *  system() function to invoke a unix command.
 *
 *
 *  Inputs:
 *  ------
 *  pchCmd = string with the command we want to exec.
 *
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  The system() command is touchy about the SIGCLD behavior. Restore
 *  the default SIGCLD handler during the time we run system().
 *
 *************************************<->***********************************/

static void
#ifdef _NO_PROTO
SystemCmd (pchCmd)
    char *pchCmd;
#else /* _NO_PROTO */
SystemCmd (char *pchCmd)
#endif /* _NO_PROTO */
{
    void (*signalHandler) ();

    signalHandler = (void (*)())signal (SIGCLD, SIG_DFL);

    system (pchCmd);

    signal (SIGCLD, signalHandler);
} /* END OF FUNTION SystemCmd */



/*************************************<->*************************************
 *
 *  RestoreState ()
 *
 *
 *  Description:
 *  -----------
 *  Calls routines in charge of restoring settings and clients
 *  If this is the first DT 3.0 session for a  DT 2.0 user then we
 *  will run convertVS.sh to change all occurances of 
 *  /usr/bin/X11/hpterm to .../dt/bin/hpterm, 
 *  /usr/bin/X11/xterm to .../dt/bin/xterm
 *  and /usr/bin/X11/xload to .../dt/bin/xload
 *  in the dt.session file before we open it.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  When this routine is finished, all settings and resources will be restored.
 *  Clients may not be, as they are actually restored by different processes.
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
RestoreState()
#else
RestoreState( void )
#endif /* _NO_PROTO */
{
    SmStateInfo	state;
    SmSaverInfo saver;
    char convertCommand[MAXPATHSM+1];
    int status;
    Boolean fixedBuffer = False;
    struct stat                 buf;
    /*
     * Restore all the X settings which were active at the time of shutdown
     */
    if(smGD.settingPath[0] != NULL)
    {
	RestoreSettings();
    }
    /*
     * BEFORE any clients are started
	 * set a property on the top level window
     * which lets the style manager know what state the sm is in
     */
    state.flags = SM_STATE_ALL;
    state.smStartState = smSettings.startState;
    state.smConfirmMode = smSettings.confirmMode;
    state.smCompatMode = smGD.compatMode;
    state.smSendSettings = !smRes.querySettings;
    state.smLockOnTimeoutStatus = smGD.lockOnTimeoutStatus;
    state.smCycleTimeout = smSaverRes.cycleTimeout;
    state.smLockTimeout = smSaverRes.lockTimeout;
    state.smSaverTimeout = smSaverRes.saverTimeout;
    state.smRandom = smSaverRes.random;

    _DtSetSmState(smGD.display, smGD.topLevelWindow, &state);

    saver.saverList = smGD.saverList;
    _DtSetSmSaver(smGD.display, smGD.topLevelWindow, &saver);


    /*
     * Set up the Property telling all applications what session is being
     * restored
     */
    XaSmRestoreMode = XInternAtom(smGD.display, _XA_DT_RESTORE_MODE, False);
    XChangeProperty(smGD.display, RootWindow(smGD.display, 0),
		    XaSmRestoreMode, XA_STRING, 8, PropModeReplace,
		    (unsigned char *)smGD.restoreSession, strlen(smGD.restoreSession));
    XFlush(smGD.display);

    
    /*
     * Restore all the X clients that were active at time of shutdown
     */
    if( ((smGD.sessionVersion == NULL) || (*smGD.sessionVersion == NULL)) &&
       (smGD.sessionType != DEFAULT_SESSION && smGD.clientPath))
    {
	/*
	 *  If this is the first DT 3.0 session for a
	 *  DT 2.0 user then we will also run convertVS.sh to change all
	 *  occurances of /usr/bin/X11/hpterm to .../dt/bin/hpterm,
	 *  /usr/bin/X11/xterm to .../dt/bin/Xterm and
	 *  /usr/bin/X11/xload to .../dt/bin/xload.
	 */
	
	status = access(smGD.clientPath, F_OK | R_OK);
	if(status != -1)
	{
	    sprintf(convertCommand, "%s %s",
		    CONVERSION_SHELL, smGD.clientPath);
	    SystemCmd (convertCommand);
	}
    }

    /*
     * Malloc line for parsing.
     */
    status = stat(smGD.clientPath, &buf);
    if(status != -1)
    {
	fileSize = buf.st_size;
    }

    if (fileSize < MAXLINE + 1)
    {
	fileSize = MAXLINE + 1;
    }

    line = (unsigned char *) malloc ((fileSize + 1) * sizeof(char *));
    if (line == NULL)
    {
	line = fallBackLine;
	fileSize = MAXLINE + 1;
	fixedBuffer = True;
    }
    cfileP = fopen(smGD.clientPath, "r");
    if (cfileP == NULL)
    {
	PrintErrnoError(DtError, smNLS.cantOpenFileString);
	if (!fixedBuffer && line)
	{
	    SM_FREE((char *)line);
	}

	return(-1);
    }
    RestoreClients();

    if (!fixedBuffer && line)
    {
	SM_FREE((char *)line);
    }

    return(0);
}



/*************************************<->*************************************
 *
 *  StartWM ()
 *
 *
 *  Description:
 *  -----------
 *  Start up the window manager.  The default is to start dtmwm unless
 *  another one is specified in a resource.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  When this routine is finished, all settings and resources will be restored.
 *  Clients may not be, as they are actually restored by different processes.
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
StartWM()
#else
StartWM( void )
#endif /* _NO_PROTO */
{
    char wmStartupErrorString[MAXPATHSM + 1];
    char localWmErrorString[(2 * MAXPATHSM) + 1];
    char pathbuf[MAXPATHLEN];
    Boolean goodWmStartup = True;
  
    if((smGD.wmStartup == NULL) || (*smGD.wmStartup == NULL))
    {
	ForkWM();
    }
    else
    {
	CreateExecString(smGD.wmStartup);

	/*
	 * check to see if the wmStartup string exists and is
	 * executable.
	 */
	if (access(smExecArray[0], X_OK) != 0)
	{
	    /*
 	     * CreateExecString modifies its argument by separating words with
 	     * NUL characters, so transcribe the path of the default window
 	     * manager location into a (modifiable) buffer on the stack.
 	     */
	    goodWmStartup = False;
	    strcpy(pathbuf, CDE_INSTALLATION_TOP "/bin/dtwm");
	    CreateExecString(pathbuf);	    
	}
	
	if(smExecArray[0] != NULL)
	{
	    ForkClient(smExecArray);
	}
    }
    
    /*
     * We used to start the message server before the window
     * manager and then if the message server did not start
     * we would exit.  Now we start the window manager, then
     * start the message server and then wait for the window
     * manager.  This seems to improve performance.
     */

    StartMsgServer();
    
    if (goodWmStartup == False)
    {
	sprintf(wmStartupErrorString, GETMESSAGE(16, 7,
          "The wmStartupCommand resource is set to:\n\n"
          "%s\n\n"
          "This file does not exist or is not executable.\n"
          CDE_INSTALLATION_TOP "/bin/dtwm will be started "
          "instead.\n"),
          smGD.wmStartup);
	PrintError(DtError, wmStartupErrorString);
    }

    WaitForWM();

    /* 
     * These lines were added to support the builtin
     * panacomm dtwm.
     */

    if (localWmLaunched && wmTimeout)
    {
	if (!smGD.userSetWaitWmTimeout)
	{
	    smRes.waitWmTimeout = smGD.savedWaitWmTimeout;
	}

	localWmLaunched = False;
	/*
	 * A special version of a built-in Xterminal dtwm
	 * was attempted and failed.
	 * Try to launch .../dt/bin/dtwm instead
	 */
	
	strcpy(pathbuf, CDE_INSTALLATION_TOP "/bin/dtwm");
	CreateExecString(pathbuf);	    
	if(smExecArray[0] != NULL)
	{
	    ForkClient(smExecArray);
	}
	WaitForWM();
	sprintf(localWmErrorString, GETMESSAGE(16, 9,
          "The following window manager did not start:\n\n"
          "      %s\n\n"
          "This message indicates you tried to start a\n"
          "window manager that is built into an X terminal.\n"
          "This will only work with X terminals that support this protocol.\n"
          CDE_INSTALLATION_TOP "/bin/dtwm will be started instead.\n"),
          tmpExecWmFile);
        PrintError(DtError, localWmErrorString);
    }
    /* 
     *     ^^^^^  ^^^^^  ^^^^^  ^^^^^  ^^^^^  ^^^^^  
     * End of  lines were added to support the builtin
     * panacomm dtwm.
     */

    
    return(0);
}


/*************************************<->*************************************
 *
 *  ReloadResources ()
 *
 *
 *  Description:
 *  -----------
 *  Reloads RESOURCE_MANAGER during running session
 *
 *
 *  Inputs:
 *  ------
 *
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/

void
#ifdef _NO_PROTO
ReloadResources()
#else
ReloadResources(void)
#endif /* _NO_PROTO */
{
 /*
  * Load sys.resources and .Xdefaults
  */
  RestoreResources(True,
                   "-load",
                   "-system",
                   "-tty",
                   "-xdefaults",
                   NULL);

 /*
  * Merge _DT_SM_PREFERENCES
  */
  RestorePreferences(NULL);

  SetSystemReady();
}


/*************************************<->*************************************
 *
 *  RestoreResources (errorHandlerInstalled, options ... )
 *
 *
 *  Description:
 *  -----------
 *  Calls routines responsible for restoring resources.
 *  Resources are restored by a fork and exec of dtsession_res.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  When this routine is finished, all settings and resources will be restored.
 *  Clients may not be, as they are actually restored by different processes.
 * 
 *************************************<->***********************************/

int 
#ifdef _NO_PROTO
/*VARARGS1*/
RestoreResources( errorHandlerInstalled, va_alist )
        Boolean errorHandlerInstalled;
        va_dcl
#else
RestoreResources( Boolean errorHandlerInstalled, ... )
#endif /* _NO_PROTO */
{
    pid_t  forkrc;
    int	   childStatus, execStatus, i;
    char *pgrm, *p;
    char *argv[20]; 
    va_list  args;

    /*
     * Check for alternate resource loader.
     */
     if ((pgrm = getenv("DTLOADRESOURCES")) == NULL)
     {
       pgrm = CDE_INSTALLATION_TOP "/bin/dtsession_res";
     }

    /*
     * By convention, exec() wants arg0 to be the program name. Ex: if pgrm
     * is /usr/dt/bin/dtsession_res, the program name is dtsession_res.
     * If all else fails, use pgrm.
     */
     argv[0] = (p = strrchr(pgrm, '/')) != NULL && *(p+1) != '\0' ? p+1 : pgrm;

     i = 0;
     Va_start(args,errorHandlerInstalled);
     do
     {
       i++;
       argv[i] = va_arg(args, char *);
     }
     while(argv[i]);
     va_end(args);

    /*
     * if an error handler is installed - remove it
     */
    if(errorHandlerInstalled)
    {
	sigaction(SIGCHLD, &smGD.defvec, (struct sigaction *) NULL);
    }
    
    /*
     * Fork and exec the xrdb process to load in the file created by
     * writing out the resource manager string generated by the last
     * session termination.
     */

    for(i = 0;(i < 10) && ((forkrc = vfork()) < 0);i++)
    {
	if(errno != EAGAIN)
	{
	    break;
	}
	sleep(2);
    }

    if(forkrc < 0)
    {
	PrintErrnoError(DtError, smNLS.cantForkClientString);
	return(-1);
    }
	
    /*
     * Fork succeeded - now do the exec
     */
    if(forkrc == 0)
    {
#ifndef __hpux
	/*
	 * Set the gid of the process back from bin
	 */
#ifndef	SVR4
	setregid(smGD.runningGID, smGD.runningGID);
#else
	setgid(smGD.runningGID);
	setegid(smGD.runningGID);
#endif
#endif
#ifdef __osf__
        setsid();
#else
	(void) setpgrp();
#endif /* __osf__ */

        execStatus = execv(pgrm, argv);

	if(execStatus != 0)
	{
            char   clientMessage[MAXPATHLEN + 30];

	    sprintf(clientMessage, ((char *)GETMESSAGE(16, 1, "Unable to exec process %s.  No session resources will be restored.")), pgrm);
	    PrintErrnoError(DtError, clientMessage);
	    SM_EXIT(-1);
	}
    }

    while(wait(&childStatus) != forkrc);

    /*
     * if an error handler is installed - remove it
     */
    if(errorHandlerInstalled)
    {
	sigaction(SIGCHLD, &smGD.childvec, (struct sigaction *) NULL);
    }

    return(0);
}


/*************************************<->*************************************
 *
 *  RestorePreferences ( filename)
 *
 *
 *  Description:
 *  -----------
 *  
 *  This routine has two roles:
 * 
 *  1) If 'filename' specified, the content of filename is read, and
 *     _DT_SM_PREFERENCES is populated with its content. This is used
 *     at session startup to set the initial state of _DT_SM_PREFERENCES.
 *
 *  2) If 'filename' is NULL, the content of _DT_SM_PREFERENCES is
 *     merged into RESOURCE_MANAGER. This is used when resources are
 *     reloaded at user request during a session.
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

int 
#ifdef _NO_PROTO
RestorePreferences( filename )
        char *filename;
#else
RestorePreferences( 
        char *filename )
#endif /* _NO_PROTO */
{
  char *data;

  if (filename)
  {
    FILE *fp;
    int size;
    struct stat statinfo;

    if(access(filename,R_OK) != 0)
    {
      return(0);
    }

   /*
    * Determine size of file.
    */
    if (stat(filename, &statinfo) == -1)
    {
      return(0);
    }

   /*
    * Get some memory.
    */
    if ((data = (char *)SM_MALLOC(statinfo.st_size + 1)) == NULL)
    {
      return(0);
    }

   /*
    * Read file into memory.
    */
    if ((fp = fopen(filename, "r")) == NULL)
    {
      SM_FREE(data);
      return(0);
    }

    size = fread(data, 1, statinfo.st_size, fp);

    if (size == statinfo.st_size)
    {
     /*
      * Merge .Xdefaults string into RESOURCE_MANAGER database, and
      * also convert to Xrm database form for later subtraction.
      */
      data[size] = '\0';
      _DtAddResString(smGD.display, data, _DT_ATR_PREFS);
    }
    fclose(fp);
    SM_FREE(data);
  }
  else
  {
   /*
    * Read string from _DT_SM_PREFERENCES
    */
    data = _DtGetResString(smGD.display, _DT_ATR_PREFS);
  
   /*
    * Merge string into RESOURCE_MANAGER
    */
    _DtAddResString(smGD.display, data, _DT_ATR_RESMGR);
    XFree(data);
  }
  return(0);
}




/*************************************<->*************************************
 *
 *  RestoreSettings ()
 *
 *
 *  Description:
 *  -----------
 *  In charge of restoring all settings.  Settings are stored in resource
 *  format so it gets the values by getting the resource values stored in
 *  a resource file that was created by the session manager.
 *
 *
 *  Inputs:
 *  ------
 *  smGD.settingPath = path that points to the settings resource file.
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  This routine messes with the actual strings returned by the resource
 *  manager by tokenizing them, so these session settings resources should
 *  not be accessed again.
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
RestoreSettings()
#else
RestoreSettings( void )
#endif /* _NO_PROTO */
{
    XrmDatabase 	smBase;
    XKeyboardControl	kbdControl;
    int			kbdControlMask;
    char		**restorePtrArray, **tmpRestore;
    char		*restoreCharArray;
    int			numArgs, ptrSize, charSize;
    int			i;
    char		*tmpKey;
    KeySym		*tmpSyms, codeSym;
    int			symSize;
    KeyCode		*tmpCode;
    XModifierKeymap	restoreMod;

    ptrSize = 50;
    charSize = 5000;
    restorePtrArray = (char **) SM_MALLOC (ptrSize * sizeof(char *));
    restoreCharArray = (char *) SM_MALLOC (charSize * sizeof(char));
    if((restorePtrArray == NULL) || (restoreCharArray == NULL))
    {
	PrintErrnoError(DtError, smNLS.cantMallocErrorString);

	if(restorePtrArray != NULL)
	{
	    SM_FREE((char *) restorePtrArray);
	}
	else
	{
	    if(restoreCharArray != NULL)
	    {
		SM_FREE(restoreCharArray);
	    }
	}
	return(-1);
    }

    /*
     * Load the resources from the SM database file
     */
    smBase = XrmGetFileDatabase(smGD.settingPath);
    if(smBase == NULL)
    {
	PrintError(DtError, GETMESSAGE(16, 2, "Invalid client settings file.  No settings restored."));
	return(-1);
    }
    else
    {
	XrmMergeDatabases(smBase, &(smGD.display->db));
    }


    /*
     * Retrieve the session settings file from the database
     */
    XtGetApplicationResources(smGD.topLevelWid, (XtPointer) &smSettings,
			      settingsResources, XtNumber(settingsResources),
			      NULL, 0);

    /*
     * Copy any string resources since they may be overwritten in
     * Xrm calls.
     */
    smGD.fontDirs  = SmNewString(smSettings.fontDirs);
    smGD.autoRepeats  = SmNewString(smSettings.autoRepeats);
    smGD.buttonMap  = SmNewString(smSettings.buttonMap);
    smGD.keySyms  = SmNewString(smSettings.keySyms);
    smGD.modSyms  = SmNewString(smSettings.modSyms);


    /*
     * This is provided for backward compatibility sake.  The values that
     * confirmMode can take have changed
     */
    if(smSettings.confirmMode == DtSM_ASK_STATE)
    {
	smSettings.confirmMode = DtSM_VERBOSE_MODE;
    }

    /*
     * If the user has previously used the "query" method, and now wishes
     * to use "only what I've customized" method, then we have to start
     * from scratch.  So don't set anything until customizer tells me to
     */
    if((smSettings.didQuery == True) && (smRes.querySettings == False))
    {
	SM_FREE((char *) restorePtrArray);
	SM_FREE(restoreCharArray);
	return(0);
    }
    
    /*
     * Restore pointer control settings
     */
    if((smSettings.accelNum > -1 ) || (smSettings.threshold > -1))
    {
	XChangePointerControl(smGD.display, True, True,
			      smSettings.accelNum,
			      smSettings.accelDenom,
			      smSettings.threshold);
	smToSet.pointerChange = True;
    }
    else
    {
	smToSet.pointerChange = False;
    }

    /*
     * Restore screen saver settings
     */
    if((smSettings.timeout > -1) || (smSettings.interval > -1))
    {
        XSetScreenSaver(smGD.display, smSettings.timeout,
                        smSettings.interval,
                        smSettings.preferBlank, smSettings.allowExp);
	smToSet.screenSavChange = True;
	screenSaverVals.smTimeout = smSettings.timeout;
	screenSaverVals.smInterval = smSettings.interval;
	screenSaverVals.smPreferBlank = smSettings.preferBlank;
	screenSaverVals.smAllowExp = smSettings.allowExp;
    }
    else
    {
	int timeout, interval, preferBlank, allowExp;

	smToSet.screenSavChange = False;
        XGetScreenSaver(smGD.display, &timeout, 
			&interval, &preferBlank, &allowExp);
        XSetScreenSaver(smGD.display, smSaverRes.saverTimeout, 
			interval, preferBlank, allowExp);
    }

    /*
     * Get the font path.  Then set it.
     */
    tmpRestore = restorePtrArray;
    numArgs = 0;
    *tmpRestore = strtok(smGD.fontDirs, ",");

    while(*tmpRestore != NULL)
    {
	numArgs++; tmpRestore++;
	*tmpRestore = strtok(NULL, ",");
	if((numArgs >= ptrSize) && (*tmpRestore != NULL))
	{
	    ptrSize += 50;
	    restorePtrArray = (char **)SM_REALLOC((char *)
						  restorePtrArray, ptrSize *
						  sizeof(char **));
	    if(restorePtrArray == NULL)
	    {
		PrintErrnoError(DtError, smNLS.cantMallocErrorString);
		SM_FREE(restoreCharArray);
		return(-1);
	    }
	}
    }

    if(numArgs > 0)
    {
	XSetFontPath(smGD.display, restorePtrArray, numArgs);
    }

    SetFontGroup();

    /*
     * Restore the keyboard control information.  In order for it
     * to be restored it has to be changed from a XKeyboardState
     * format to an XKeyboardControl format.
     */
    if((smSettings.kbdState.key_click_percent > -1) ||
       (smSettings.kbdState.bell_percent > -1) ||
       (smSettings.kbdState.bell_pitch > -1) ||
       (smSettings.kbdState.bell_duration > -1))
    {
	
	kbdControlMask = 0;
        if((smSettings.kbdState.bell_percent > -1) ||
           (smSettings.kbdState.bell_pitch > -1) ||
           (smSettings.kbdState.bell_duration > -1))
	{
	    kbdControl.bell_percent = smSettings.kbdState.bell_percent;
	    kbdControl.bell_pitch = smSettings.kbdState.bell_pitch;
	    kbdControl.bell_duration = smSettings.kbdState.bell_duration;
	    kbdControlMask |= (KBBellPercent | KBBellPitch | KBBellDuration);
	    smToSet.audioChange = True;
	    audioVals.smBellPercent = smSettings.kbdState.bell_percent;
	    audioVals.smBellPitch = smSettings.kbdState.bell_pitch;
	    audioVals.smBellDuration = smSettings.kbdState.bell_duration;
	}
	else
	{
	    smToSet.audioChange = False;
	}

	if((smSettings.kbdState.key_click_percent > -1) ||
	   (smSettings.kbdState.global_auto_repeat != AutoRepeatModeOn))
	{
	    kbdControl.key_click_percent =
		smSettings.kbdState.key_click_percent;
	    kbdControlMask |= KBKeyClickPercent;
	    smToSet.keyboardChange = True;
	    keyboardVals.smKeyClickPercent =  kbdControl.key_click_percent;
	}
	else
	{
	    smToSet.keyboardChange = False;
	}
	

	/*
	 * NOTICE THAT THE LED'S DON'T GET RESET.  THIS IS BECAUSE LED STUFF
	 * IS MACHINE DEPENDENT.
	 */
	/*
	 * Set the auto repeat stuff
	 */
	tmpKey = strtok(smGD.autoRepeats, ",");
	if((tmpKey == NULL) &&
	    ((smSettings.kbdState.global_auto_repeat == AutoRepeatModeOff) ||
	     (smSettings.kbdState.global_auto_repeat == AutoRepeatModeOn)))
	{
	    smToSet.keyboardChange = True;
	    kbdControl.auto_repeat_mode =
		smSettings.kbdState.global_auto_repeat;
	    kbdControlMask |= KBAutoRepeatMode;
	    XChangeKeyboardControl(smGD.display, kbdControlMask, &kbdControl);
	    keyboardVals.smGlobalAutoRepeat =
		smSettings.kbdState.global_auto_repeat;
	}
	else
	{
	    if(tmpKey != NULL)
	    {
		smToSet.keyboardChange = True;
		XChangeKeyboardControl(smGD.display,
				       kbdControlMask, &kbdControl);
		kbdControl.auto_repeat_mode = AutoRepeatModeOn;
		kbdControlMask = KBAutoRepeatMode | KBKey;
		/*
		 * This is only involked when there is auto repeats set for
		 * specific keys only.  It is VERY SLOW code so unless you
		 * have to save off auto repeats for single keys - DONT
		 */
		while(tmpKey != NULL)
		{
		    kbdControl.key = atoi(tmpKey);
		    XChangeKeyboardControl(smGD.display,
					   kbdControlMask, &kbdControl);
		    tmpKey = strtok(NULL, ",");
		}
	    }
	    else
	    {
		if(kbdControlMask != 0)
		{
		    XChangeKeyboardControl(smGD.display,
					   kbdControlMask, &kbdControl);
		}
	    }
	}
    }
    else
    {
	smToSet.audioChange = False;
	smToSet.keyboardChange = False;
    }

    /*
     * Restore the button mappings
     */
    numArgs = 0;
    tmpKey = strtok(smGD.buttonMap, ",");
    
    if(tmpKey != NULL)
    {
	smToSet.pointerMapChange = True;
    }
    else
    {
	smToSet.pointerMapChange = False;
    }
	
    while(tmpKey != NULL)
    {
	restoreCharArray[numArgs] = (char) atoi(tmpKey);
	numArgs++;
	tmpKey = strtok(NULL, ",");
	if((numArgs >= charSize) && (tmpKey != NULL))
	{
	    charSize += 500;
	    restoreCharArray = (char *) SM_REALLOC(restoreCharArray,
						charSize * sizeof(char));
	    if(restoreCharArray == NULL)
	    {
		PrintErrnoError(DtError, smNLS.cantMallocErrorString);
		SM_FREE((char *)restorePtrArray);
		return(-1);
	    }
	}
    }
    
    if(numArgs > 0)
    {
	XSetPointerMapping(smGD.display, (unsigned char *)restoreCharArray, numArgs);

	/*
	 * Copy the pointer map into the saved map for logout
	 */
	 smToSet.numButton = ((numArgs > 5) ? 5  : numArgs);
	for(i = 0; i < smToSet.numButton;i++)
	{
	    smToSet.pointerMap[i] = restoreCharArray[i];
	}
    }

    /*
     * Restore the key mappings
     */
    if(smSettings.numKeyCode > 0)
    {
	tmpSyms = (KeySym *) restoreCharArray;
	symSize = (charSize * sizeof(char)) / sizeof(KeySym);
	tmpKey = strtok(smGD.keySyms, ",");
	for(i = 0;tmpKey != NULL;i++)
	{
	    tmpSyms[i] = (KeySym) atol(tmpKey);
	    tmpKey = strtok(NULL, ",");
	    numArgs = i + 1;
	    if((numArgs >= symSize) && (tmpKey != NULL))
	    {
		charSize += 1000;
		symSize = (charSize * sizeof(char))/sizeof(KeySym);
		restoreCharArray = (char *) SM_REALLOC(restoreCharArray,
				     (charSize * sizeof(char)));
		if(restoreCharArray == NULL)
		{
		    PrintErrnoError(DtError, smNLS.cantMallocErrorString);
		    SM_FREE((char *) restorePtrArray);
		    return(-1);
		}
		tmpSyms = (KeySym *) restoreCharArray;
	    }
	}
	numArgs /= smSettings.keySymPerCode;
	XChangeKeyboardMapping(smGD.display, (KeyCode)
			       smGD.display->min_keycode,
			       smSettings.keySymPerCode, tmpSyms,
			       numArgs);
    }
    
    /*
     * Restore the modifier mappings
     */
    tmpCode = (KeyCode *) restoreCharArray; 
    tmpKey = strtok(smGD.modSyms, ",");
    if(tmpKey != NULL)
    {
	for(i = 0;i < (8 * smSettings.maxKeyPerMod);i++)
	{
	    if(tmpKey != NULL)
	    {
		codeSym = (KeySym) atol(tmpKey);
		if(codeSym != 0)
		{
		    tmpCode[i] = XKeysymToKeycode(smGD.display, codeSym);
		}
		else
		{
		    tmpCode[i] = (KeyCode) 0;
		}
	    }
	    else
	    {
		tmpCode[i] = (KeyCode) 0;
	    }
	    tmpKey = strtok(NULL, ",");
	}
	restoreMod.max_keypermod = smSettings.maxKeyPerMod;
	restoreMod.modifiermap = tmpCode;
	XSetModifierMapping(smGD.display, &restoreMod);
    }

    SM_FREE((char *) restorePtrArray);
    SM_FREE(restoreCharArray);

    return(0);    
}



/*************************************<->*************************************
 *
 *  RestoreIndependentResources ()
 *
 *
 *  Description:
 *  -----------
 *  In charge of restoring the resources that help make the session more
 *  resolution and language independent - only restored if they are
 *  necessary (lang || resolution has changed) and exist
 *
 *
 *  Inputs:
 *  ------
 *  smResources = global pointer to the resources to be restored by the SM
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
RestoreIndependentResources()
#else
RestoreIndependentResources( void )
#endif /* _NO_PROTO */
{
    Atom        actualType;
    int         actualFormat;
    unsigned long   nitems, leftover;
    unsigned char   *data = NULL;
    XrmDatabase 	smBase;
    XrmValue		fontResourceReturn;
    char *currentLangPtr, *resValRet, *sessionType;
    float fltYRes;
    int	  intYRes[2], status,i;
    char  *sessionRes;
    struct stat buf;
    Boolean resIndep = False, resRet;

    if(((smGD.sessionLang == NULL) || (*smGD.sessionLang == NULL)) &&
       (smRes.displayResolution == 0))
    {
	/*
	 * No saved info to draw from - nothing new is added
	 */
	return;
    }
    
    currentLangPtr = getenv("LANG");

    fltYRes = ((float) DisplayHeight(smGD.display, 0) /
	       (float) DisplayHeightMM(smGD.display, 0)) * 1000;

    if(fltYRes < MED_RES_Y_RES) 
    {
	intYRes[0] = LOW_RES_Y_RES;
	sessionRes = SM_LOW_RES_EXT;
    }
    else
    {
	if(fltYRes >= HIGH_RES_Y_RES)
	{
	    intYRes[0] = HIGH_RES_Y_RES;
	    sessionRes = SM_HIGH_RES_EXT;
	}
	else
	{
	    intYRes[0] = MED_RES_Y_RES;
	    sessionRes = SM_MED_RES_EXT;
	}
    }

    if(smRes.displayResolution < MED_RES_Y_RES)
    {
	intYRes[1] = LOW_RES_Y_RES;
    }
    else
    {
	if(smRes.displayResolution >= HIGH_RES_Y_RES)
	{
	    intYRes[1] = HIGH_RES_Y_RES;
	}
	else
	{
	    intYRes[1] = MED_RES_Y_RES;
	}
    }

    /*
     * If the resolution or the language has changed -
     * load the language/resolution independent fonts if
     * they exist in the users home directory or in the system
     */
    if((strcmp(currentLangPtr, smGD.sessionLang)) ||
       (intYRes[0] != intYRes[1]))
    {

	if(smGD.sessionType == HOME_SESSION)
	{
	    sessionType = SM_HOME_FONT_DIRECTORY;
	}
	else
	{
	    sessionType = SM_CURRENT_FONT_DIRECTORY;
	}

	sprintf(smGD.fontPath, "%s/%s/%s/%s.%s", smGD.savePath, sessionType,
		currentLangPtr, SM_FONT_FILE, sessionRes);
	status = stat(smGD.fontPath, &buf);
	if(status == -1)
	{
	    /*
	     * User has nothing there - look in the system defaults
	     * first in the language dep -then in lang independent
	     */
	    sprintf(smGD.fontPath, "%s/%s/%s", SM_SYSTEM_PATH,
		    currentLangPtr, SM_SYSTEM_FONT_FILE);
	    status = stat(smGD.fontPath, &buf);
	    if(status == -1)
	    {
		sprintf(smGD.fontPath, "%s/%s", SM_SYSTEM_PATH,
			SM_SYSTEM_FONT_FILE);
		status = stat(smGD.fontPath, &buf);
		if(status != -1)
		{
		    resIndep = True;
		}
	
	    }
	    else
	    {
		resIndep = True;
	    }
	}
	else
	{
	    resIndep = True;
	}

	if(resIndep == True)
	{
	    /*
	     * add the auxillary resources onto the root window
	     */
	    RestoreResources(True, "-merge", smGD.fontPath, NULL);

	    /*
	     * Load the resources from the RESOURCE_MANAGER
	     * property on the root window
	     */
	    if(XGetWindowProperty(smGD.display, RootWindow(smGD.display, 0),
			       XA_RESOURCE_MANAGER,0L,
			       100000000L,False,XA_STRING,&actualType,
			       &actualFormat,&nitems,&leftover,
			       (unsigned char**) &data) == Success)
	    {
		smBase = XrmGetStringDatabase((char *)data);
	    }
	    else
	    {
		smBase = NULL;
	    }
	    

	    if(smBase == NULL)
	    {
		PrintError(DtError, GETMESSAGE(16, 6, "Invalid display/language independent resource file.  No display/language independent resources will be restored."));
		SM_FREE((char *)data);
		return;
	    }
	    else
	    {
		/*
		 * Get the new fontlist from the resources and
		 * Put it on the application shell.  Then add
		 * the auxillary resources into the display struct
		 */
		resRet = XrmGetResource(smBase,  "*fontList", "*FontList",
					&resValRet, &fontResourceReturn);

		i = 0;
		XtSetArg(uiArgs[i], XmNdefaultFontList,
			 (XmFontList) fontResourceReturn.addr);i++;
		XtSetValues(smGD.topLevelWid, uiArgs, i);
		
		XrmMergeDatabases(smBase, &(smGD.display->db));
		SM_FREE((char *)data);
	    }
	}

    }

    return;
}
    

/*************************************<->*************************************
 *
 *  RestoreClients ()
 *
 *
 *  Description:
 *  -----------
 *  Reads through the client file and restores its contents.  A client file
 *  consists of hints for the workspace manager, actual client commands, and
 *  remote execution commands.  All commands are processed accordingly.
 *  If this is the first DT 3.0 session for a DT 2.0 user then we will
 *  also launch the helpviewer. If this is the first DT 3.0 session for a
 *  DT 2.0 user then we will also run convertVS.sh to change all
 *  occurances of /usr/bin/X11/hpterm to .../dt/bin/hpterm,
 *  /usr/bin/X11/xterm to .../dt/bin/xterm and
 *  /usr/bin/X11/xload to .../dt/bin/xload.
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  Any information read from the client file will be mucked with during
 *  processing, and should not try to be mucked with again.
 *
 *  WARNING:  This routine closes cfileP - not the calling routine
 *  DOUBLE-WARNING: This routine starts the window manager
 *
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
RestoreClients()
#else
RestoreClients( void )
#endif /* _NO_PROTO */
{
    unsigned char *lineP, *string;
    char *pch, *dispPtr;
    char *dispEnv, *dispSav, *dispEnvHelpview, *dispSavHelpview;
    unsigned char *hostPtr=NULL, *cmdPtr=NULL, *hintPtr = NULL;
    unsigned char remoteDisplay[101]; 
    char envVal[150], displayName[101];
    unsigned char  tmpChar[35];
    int     screenNum = 0, i, j, chlen, numClientsExec;
    int	    numRemoteDone = 0;
    Window	dtwmWin = 0;
    Boolean	clientsDone = False, wmHandshake = False, cmdInvInit = False;
    XClientMessageEvent smToWmMessage;
    char pathbuf[MAXPATHLEN];
    
    /*
     * These variables are needed to buffer up the commands and then
     * execute them (also buffer up hints then put them on the root window
     */
    unsigned char *hintsBuf[MAX_SCREENS_SAVED];
    unsigned char *cmdBuf[MAX_SCREENS_SAVED];
    unsigned int  maxHintsSize[MAX_SCREENS_SAVED];
    unsigned int  actHintsSize[MAX_SCREENS_SAVED];
    unsigned int  numHints[MAX_SCREENS_SAVED];
    unsigned int  maxCmdSize[MAX_SCREENS_SAVED];
    unsigned int  actCmdSize[MAX_SCREENS_SAVED];
    unsigned int  numCmd[MAX_SCREENS_SAVED];
    unsigned int  maxRemoteSize[MAX_SCREENS_SAVED];

    /*
     * Initialize the buffers and their sizes to null and 0.  Because of
     * all possible combinations - buffers aren't malloc'd until they're
     * needed
     */
    for(i = 0;i<smGD.numSavedScreens;i++)
    {
	hintsBuf[i] = NULL;
	cmdBuf[i] = NULL;
	remoteBuf[i] = NULL;
	maxHintsSize[i] = 0;
	actHintsSize[i] = 0;
	numHints[i] = 0;
	maxCmdSize[i] = 0;
	actCmdSize[i] = 0;
	numCmd[i] = 0;
	maxRemoteSize[i] = 0;
	actRemoteSize[i] = 0;
    }
    
    /*
     * Save the contents of the DISPLAY environment variable so that it
     * can be restored when we're through
     */
    dispEnv = getenv("DISPLAY");
    if(dispEnv != NULL)
    {
	dispSav = (char *) SM_MALLOC(((strlen(dispEnv) + strlen("DISPLAY="))
				* sizeof(char) + 1));
	sprintf(dispSav, "DISPLAY=%s",dispEnv);
    }
    else
    {
	dispSav = NULL;
    }
    
    linec = 0;
    strcpy(displayName, smGD.display->display_name);

    /*
     * Create the display name for locally executing clients
     */
    pch = displayName;
    while (*pch)
    {
	if (*pch == ':')
	{
	    dispPtr = pch;
	    pch++;
	    break;
	}
	pch++;
    }

    while (*pch)
    {
	if (*pch == '.')
	{
	    pch++;
	    *pch = '\0';
	    break;
	}
	pch++;
    }

    /*
     * Create the display name for remotely executing clients.
     * These two may or may not be the same
     */
    DtGetShortHostname((char *)remoteDisplay,100);

    /*
     * Add the display qualifications to the host name
     * screen is added at execution time
     */
    strcat((char *)remoteDisplay, (char *)dispPtr);

    /*
     * Intern the atom needed to put the hints on the root window
     * This has to be done here because InitProtocol has not been called yet
     */
    XaWmDtHints = XInternAtom(smGD.display, _XA_DT_SESSION_HINTS, False);

    while(GetNextLine() != NULL)
    {
	lineP = line;
	string = GetSmartString(&lineP);
	if( string != NULL && !strcmp((char *)string, "dtsmcmd"))
	{
	    while((string = GetSmartString(&lineP)) != NULL )
	    {
		if(!strcmp((char *)string, "-host"))
		{
		    /*
		     * Extract a host pointer - host pointers
		     * only exist on remote executions
		     */
		    string = GetSmartString(&lineP);
		    hostPtr = string;
		}
		else
		{
		    if(!strcmp((char *)string, "-cmd"))
		    {
			/*
			 * Extract the command pointer from the
			 * hints string
			 */
			string = GetSmartString(&lineP);
			cmdPtr = string;
		    }
		    else
		    {
			if(!strcmp((char *)string, "-screen"))
			{
			    /*
			     * Extract the screen number from the command
			     */
			    string = GetSmartString(&lineP);
			    screenNum = atoi((char *)string);

			}
			else
			{
			    if(!strcmp((char *)string, "-hints"))
			    {
				/*
				 * Extract the hints string
				 */
				string = GetSmartString(&lineP);
				hintPtr = string;
			    }
			}
		    }
		}
	    }

	    /*
	     * Now put our information in buffers and reinitialize the pointers
	     */
	    if(hintPtr != NULL)
	    {
		if(FillHintBuf(hintPtr, hintsBuf, maxHintsSize,
			    actHintsSize, screenNum, numHints) != 0)
		{
		    /*
		     * Free all malloc'd buffers and exit
		     */
		    for(i = 0;i < smGD.numSavedScreens;i++)
		    {
			if(actHintsSize[i] > 0)
			{
			    SM_FREE((char *) hintsBuf[i]);
			}
			if(actCmdSize[i] > 0)
			{
			    SM_FREE((char *) cmdBuf[i]);
			}
			if(actRemoteSize[i] > 0)
			{
			    SM_FREE((char *) remoteBuf[i]);
			}
		    }
		    return(-1);
		}
		hintPtr = NULL;
	    }

	    if((cmdPtr != NULL) && (hostPtr == NULL))
	    {
		if(FillCmdBuf(cmdPtr, cmdBuf, maxCmdSize,
			      actCmdSize,screenNum, &numCmd[screenNum]) != 0)
		{
		    /*
		     * Free all malloc'd buffers and exit
		     */
		    for(i = 0;i < smGD.numSavedScreens;i++)
		    {
			if(actHintsSize[i] > 0)
			{
			    SM_FREE((char *) hintsBuf[i]);
			}
			if(actCmdSize[i] > 0)
			{
			    SM_FREE((char *) cmdBuf[i]);
			}
			if(actRemoteSize[i] > 0)
			{
			    SM_FREE((char *) remoteBuf[i]);
			}
		    }
		    return(-1);
		}
		cmdPtr = NULL;
	    }
	    else
	    {
		if((cmdPtr != NULL) && (hostPtr != NULL))
		{
		    if(FillRemoteBuf(cmdPtr, hostPtr, remoteDisplay,
				  remoteBuf, maxRemoteSize,
				  actRemoteSize, screenNum) != 0)
		    {
			/*
			 * Free all malloc'd buffers and exit
			 */
			for(i = 0;i < smGD.numSavedScreens;i++)
			{
			    if(actHintsSize[i] > 0)
			    {
				SM_FREE((char *) hintsBuf[i]);
			    }
			    if(actCmdSize[i] > 0)
			    {
				SM_FREE((char *) cmdBuf[i]);
			    }
			    if(actRemoteSize[i] > 0)
			    {
				SM_FREE((char *) remoteBuf[i]);
			    }
			}
			return(-1);
		    }
		    cmdPtr = NULL;
		    hostPtr = NULL;
		}
	    }
	    screenNum = XDefaultScreen(smGD.display);
	}
    }

    /*
     * All done with file so close it off and set descriptor to NULL -
     * This is done so that parsing routines can be used with a buffer later
     */
    fclose(cfileP);
    cfileP = NULL;

    /*
     * Now execute all the buffers, put all hints on the root windows
     * Do all remote executions
     */
    for(i = 0;i < smGD.numSavedScreens;i++)
    {
	/*
	 * Put the root window property on each root window
	 */
	if(actHintsSize[i] > 0)
	{
	    /*
	     * Append number of hints to front of buffer
	     */
	    sprintf((char *)tmpChar, "%d", numHints[i]);
	    strncpy((char *)hintsBuf[i], (char *)tmpChar,
		    strlen((char *)tmpChar));
	    XChangeProperty(smGD.display, RootWindow(smGD.display, i),
			    XaWmDtHints, XA_STRING, 8, PropModeReplace,
			    hintsBuf[i], actHintsSize[i]);
	    SM_FREE((char *) hintsBuf[i]);
	    XSync(smGD.display, 0);
	}
    }

    StartWM();


    /*
     * Now exec on the local clients - we're doing contention management
     * to make sure the system doesn't get swamped
     */
    i = 0;

    while((actCmdSize[i] == 0) && (i < smGD.numSavedScreens))
    {
	i++;
    }

    if(i >= smGD.numSavedScreens)
    {
	clientsDone = True;
    }
    else
    {
	sprintf(envVal,"DISPLAY=%s%d", displayName, i);
	putenv(envVal);

	linec = 0;
	parseP = cmdBuf[i];
    }

    /*
     *  Keep forking one client after the other until the
     *  memory utilization gets beyond the threshold -
     *  (but only if the capability exists)  Then go to
     *  window manager handshaking
     */
#ifdef DEBUG_CONT_MANAGEMENT
    if(smRes.contManagement & SM_CM_SYSTEM)
    {
	fprintf(stderr, "SM_CM_SYSTEM flag set in smRes.contManagement\n");
    }
    if(smRes.contManagement & SM_CM_HANDSHAKE)
    {
	fprintf(stderr, "SM_CM_HANDSHAKE flag set in smRes.contManagement\n");
    }
#endif /* DEBUG_CONT */
    numClientsExec = 0;
    if((smRes.contManagement & SM_CM_SYSTEM) &&
       ((GetMemoryUtilization() != MEM_NOT_AVAILABLE) && 
	(clientsDone == False)))
    {
	while((GetMemoryUtilization() == MEM_NOT_FULL) &&
	      (clientsDone == False))
	{	
	    GetNextLine();
	    lineP = line;
	    CreateExecString((char *) lineP);
	    if(smExecArray[0] != NULL)
	    {
		ForkClient((char **)smExecArray);
	    }	
	    numClientsExec++;

	    /*
	     * When all the clients have been exec'd for this screen
	     * go on to the next
	     */
	    if(numClientsExec >= numCmd[i])
	    {
		ResetScreenInfo(cmdBuf, &i, actCmdSize, &clientsDone,
				envVal, displayName);
		numClientsExec = 0;
	    }
	}
    }

    /*
     *  After we've checked memory utilization - finish up
     *  by handshaking with the worksapce manager - if it's there
     */
    if(clientsDone == False)
    {
	if((smGD.dtwmRunning) && (smRes.contManagement & SM_CM_HANDSHAKE))
	{
	    /*
	     * Get the window id of the workspace manager and tell it
	     * to start messaging
	     */
	    _DtGetMwmWindow(smGD.display, RootWindow(smGD.display, 0),
			   &dtwmWin);
	    smToWmMessage.type = ClientMessage;
	    smToWmMessage.window = dtwmWin;
	    smToWmMessage.message_type = XaSmWmProtocol;
	    smToWmMessage.format = 32;
	    smToWmMessage.data.l[0] = XaSmStartAckWindow;
	    smToWmMessage.data.l[1] = CurrentTime;
	    if (XSendEvent(smGD.display, dtwmWin,False,NoEventMask,
			   (XEvent *) &smToWmMessage) != 0)
	    {
		wmHandshake = True;
		XSync(smGD.display, 0);
	    }
		
	}
		
	/*
	 *  Start a client - and wait for the workspace manager to
	 *  map a window to start a new client
	 */
	while(clientsDone == False)
	{
	    GetNextLine();
	    lineP = line;
	    CreateExecString((char *) lineP);
	    if(smExecArray[0] != NULL)
	    {
		ForkClient(smExecArray);
	    }

	    /*
	     * If we're handshaking with the workspace manager
	     * wait for the client to be mapped before starting
	     * the next one
	     */
	    if(wmHandshake == True)
	    {
		WaitForClientMap();
	    }

	    numClientsExec++;

	    /*
	     * When all the clients have been exec'd for this screen
	     * go on to the next
	     */
	    if(numClientsExec >= numCmd[i])
	    {
		ResetScreenInfo(cmdBuf, &i, actCmdSize, &clientsDone,
				envVal, displayName);
		numClientsExec = 0;
	    }
	}

	if(wmHandshake == True)
	{
	    /*
	     * If we are handshaking - tell the workspace manager to
	     * stop
	     */
	    smToWmMessage.type = ClientMessage;
	    smToWmMessage.window = dtwmWin;
	    smToWmMessage.message_type = XaSmWmProtocol;
	    smToWmMessage.format = 32;
	    smToWmMessage.data.l[0] = XaSmStopAckWindow;
	    smToWmMessage.data.l[0] = CurrentTime;
	    XSendEvent(smGD.display, dtwmWin,False,NoEventMask,
		       (XEvent *) &smToWmMessage);
		
	}
    }
    
    /*
     * If this is the first DT 3.0 session for a DT 2.0 users
     * then launch the HelpViewer with the introduction topic
     */
    if((smGD.sessionVersion == NULL) || (*smGD.sessionVersion == NULL))
    {
	/*
	 * Save the current DISPLAY environment variable so that it
	 * can be restored after we launch helpview on the default
	 * display.
	 */
	dispEnvHelpview = getenv("DISPLAY");
	if(dispEnv != NULL)
	{
	    dispSavHelpview = (char *) SM_MALLOC(((strlen(dispEnvHelpview) 
						   + strlen("DISPLAY="))
						  * sizeof(char) + 1));
	    sprintf(dispSavHelpview, "DISPLAY=%s",dispEnvHelpview);
	}
	else
	{
	    dispSavHelpview = NULL;
	}
	
	putenv(dispSav);
	strcpy(pathbuf, HELPVIEW_COMMAND_LINE);
	CreateExecString(pathbuf);	    
	if(smExecArray[0] != NULL)
	{
	    ForkClient(smExecArray);
	}
	putenv(dispSavHelpview);
    }


    for(i = 0;i < smGD.numSavedScreens;i++)
    {
	if(numRemoteDone == MAX_REMOTE_CLIENTS)
	{
	    break;
	}

	/*
	 * Send out all the remote execution commands for the screen to
	 * the command invoker.  On failure - do a remsh instead
	 */
	if(actRemoteSize[i] > 0)
	{
	    /*
	     * Initialize the command invoker - if not done
	     */
	    if(cmdInvInit == False)
	    {
		_DtInitializeCommandInvoker(smGD.display,
					   DtSM_TOOL_CLASS,
					   SM_RESOURCE_CLASS,
					   NULL, smGD.appCon);
		cmdInvInit = True;
	    }
	    
	    /*
	     *  Set up the display environment variable so that the
	     *  application comes back to the right screen
	     */
	    sprintf(envVal,"DISPLAY=%s%d", displayName, i);
	    putenv(envVal);
	    
	    linec = 0;
	    parseP = remoteBuf[i];
	    while(numRemoteExecs > 0 && GetNextLine() != NULL)
	    {
		/*
		 * Get the host and the command and send them off
		 * On failure of the command invoker do a remsh
		 */
		lineP = line;
		string = GetSmartString(&lineP);
		hostPtr = string;
		string = GetSmartString(&lineP);
		cmdPtr = string;

		remoteBufPtr[numRemoteDone].cmdPtr = cmdPtr;
		remoteBufPtr[numRemoteDone].hostPtr = hostPtr;
		
		_DtCommandInvokerExecute(DtSTART_SESSION, NULL, NULL, NULL,
					"-", (char *) hostPtr, (char *) cmdPtr,
					RemoteRequestSucceeded, NULL,
					RemoteRequestFailed,
					&remoteBufPtr[numRemoteDone]);
		numRemoteDone++;

		/*
		 * If there is no more room in the remote client
		 * array - quit exec'ing remote clients
		 */
		if(numRemoteDone == MAX_REMOTE_CLIENTS)
		{
		    PrintError(DtError, GETMESSAGE(16, 5, "You have reached the maximum allowed number of remote clients.  No further remote clients will be restored."));
		    break;
		}
		
	    }
	}
    }


    /*
     * Now return the display variable back to the display that was
     * originally opened (the default display)
     */
    if(dispSav != NULL)
    {
	putenv(dispSav);
    }
    else
    {
	FixEnvironmentData();
    }

    return(0);
} /* END OF FUNCTION RestoreClients */



/*************************************<->*************************************
 *
 *  StartEtc ( exiting )
 *
 *
 *  Description:
 *  -----------
 *  Call the ForkClient routine to fork and exec either the sessionetc file
 *  (if exiting==False) or the sessionexit file (if exiting==True).
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
StartEtc( exiting )
#else
StartEtc( Boolean exiting )
#endif /* _NO_PROTO */
{
    int status;
    struct stat buf;
    char *execArray[3];
    char path[MAXPATHLEN];
   
    execArray[0] = path; 
    if (exiting) { 
        execArray[1] = smExitFile;
    }
    else {
        execArray[1] = smEtcFile;
    }
    execArray[2] = '\0';

    /* 
     * Execute the /usr/dt/config sessionetc or sessionexit file 
     */

    snprintf(path, MAXPATHLEN, "%s/%s/%s", 
	    CDE_INSTALLATION_TOP, "config", execArray[1]);

    if ((status=stat(execArray[0], &buf)) != -1)
    {
        ForkClient(execArray);
    }

    /* 
     * Exectue the /etc/dt/config sessionetc or sessionexit file
     */

    snprintf(path, MAXPATHLEN, "%s/%s/%s", 
	     CDE_CONFIGURATION_TOP, "config", execArray[1]);

    if ((status=stat(execArray[0], &buf)) != -1)
    {
        ForkClient(execArray);
    }

    /* 
     * Execute the $HOME/.dt sessionetc or sessionexit file 
     */

    snprintf(path, MAXPATHLEN, "%s/%s", smGD.savePath, execArray[1]);

    if ((status=stat(execArray[0], &buf)) != -1)
    {
        ForkClient(execArray);
    }
}


/*************************************<->*************************************
 *
 *  GetNextLine ()
 *
 *
 *  Description:
 *  -----------
 *  Returns the next line from an fopened configuration file or a newline-
 *  embedded configuration string.
 *
 *
 *  Inputs:
 *  ------
 *  cfileP =  (global) file pointer to fopened configuration file or NULL
 *  line   =  (global) line buffer
 *  linec  =  (global) line count
 *  parseP =  (global) parse string pointer if cfileP == NULL
 *
 * 
 *  Outputs:
 *  -------
 *  line =     (global) next line 
 *  linec =    (global) line count incremented
 *  parseP =   (global) parse string pointer incremented
 *  Return =  line or NULL if file or string is exhausted.
 *
 *
 *  Comments:
 *  --------
 *  If there are more than MAXLINE characters on a line in the file cfileP the
 *  excess are truncated.  
 *  Assumes the line buffer is long enough for any parse string line.
 *  Code stolen from dtmwm
 * 
 *************************************<->***********************************/
static unsigned char * 
#ifdef _NO_PROTO
GetNextLine()
#else
GetNextLine( void )
#endif /* _NO_PROTO */
{
    unsigned char *string;
#ifdef MULTIBYTE
    int   chlen;
#endif

    if (cfileP != NULL)
    /* read fopened file */
    {
	string = (unsigned char *) fgets((char *)line, fileSize, cfileP);
    }
    else if ((parseP != NULL) && (*parseP != NULL))
    /* read parse string */
    {
	string = line;
#ifdef MULTIBYTE
	while ((*parseP != NULL) &&
               ((chlen = mblen ((char *) parseP, MB_CUR_MAX)) > 0) &&
	       (*parseP != '\n'))
	/* copy all but NULL and newlines to line buffer */
	{
	    while (chlen--)
	    {
	        *(string++) = *(parseP++);
	    }
        }
#else
	while ((*parseP != NULL) && (*parseP != '\n'))
	/* copy all but NULL and newlines to line buffer */
	{
	    *(string++) = *(parseP++);
        }
#endif
	*string = NULL;
	if (*parseP == '\n')
	{
	    parseP++;
	}
    }
    else
    {
	string = NULL;
    }

    linec++;
    return (string);

} /* END OF FUNCTION GetNextLine */



/*************************************<->*************************************
 *
 *  PeekAhead (currentChar, currentLev)
 *
 *
 *  Description:
 *  -----------
 *  Returns a new level value if this is a new nesting level of quoted string
 *  Otherwise it returns a zero
 *
 *
 *  Inputs:
 *  ------
 *  currentChar = current position in the string
 *  currentLev = current level of nesting
 *
 * 
 *  Outputs:
 *  -------
 *  Returns either a new level of nesting or zero if the character is copied in
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static unsigned int 
#ifdef _NO_PROTO
PeekAhead( currentChar, currentLev )
        unsigned char *currentChar ;
        unsigned int currentLev ;
#else
PeekAhead(
        unsigned char *currentChar,
        unsigned int currentLev )
#endif /* _NO_PROTO */
{
    Boolean		done = False;
    unsigned int 	tmpLev = 1;
#ifdef MULTIBYTE
    unsigned int	chlen;

    while (((chlen = mblen ((char *) currentChar, MB_CUR_MAX)) > 0) &&
	   (chlen == 1) && ((*currentChar == '"') || (*currentChar == '\\'))
	   && (done == False))
    {
	currentChar++;

	if(((chlen = mblen ((char *) currentChar, MB_CUR_MAX)) > 0) && (chlen == 1) &&
	   ((*currentChar == '"') || (*currentChar == '\\')))
	{
	    tmpLev++;
	    if(*currentChar == '"')
	    {
		done = True;
	    }
	    else
	    {
		currentChar++;
	    }
	}
    }
#else
    while((*currentChar != NULL) && (done == False) &&
	  ((*currentChar == '"') || (*currentChar == '\\')))
    {
	currentChar++;
	if((*currentChar != NULL) &&
	   ((*currentChar == '"') || (*currentChar == '\\')))
	{
	    tmpLev++;
	    if(*currentChar == '"')
	    {
		done = True;
	    }
	    else
	    {
		currentChar++;
	    }
	}
    }
#endif /*MULTIBYTE*/

    /*
     * Figure out if this is truly a new level of nesting - else ignore it
     * This section probably could do some error checking and return -1
	 * If so, change type of routine from unsigned int to int
     */
    if(done == True)
    {
	return(tmpLev);
    }
    else
    {
	return(0);
    }
}
    
    

/*************************************<->*************************************
 *
 *  GetSmartString (linePP)
 *
 *
 *  Description:
 *  -----------
 *  Returns the next quoted or whitespace-terminated nonquoted string in the
 *  line buffer.
 *  Additional functionality added to GetString in that anything in a
 *  quoted string is considered sacred and nothing will be stripped from
 *  the middle of a quoted string.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =  pointer to current line buffer pointer.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP =  pointer to revised line buffer pointer.
 *  Return =  string 
 *
 *
 *  Comments:
 *  --------
 *  May alter the line buffer contents.
 *  Handles quoted strings and characters, removing trailing whitespace from
 *  quoted strings.
 *  Returns NULL string if the line is empty or is a comment.
 *  Code stolen from dtmwm.
 * 
 *************************************<->***********************************/

static unsigned char * 
#ifdef _NO_PROTO
GetSmartString( linePP )
        unsigned char **linePP ;
#else
GetSmartString(
        unsigned char **linePP )
#endif /* _NO_PROTO */
{
    unsigned char *lineP = *linePP;
    unsigned char *endP;
    unsigned char *curP;
    unsigned char *lnwsP;
    unsigned int  level = 0, checkLev, i, quoteLevel[MAX_QUOTE_DEPTH];
#ifdef MULTIBYTE
    int            chlen;

    /* get rid of leading white space */
    ScanWhitespace (&lineP);

    /*
     * Return NULL if line is empty, whitespace, or begins with a comment.
     */
    if((chlen = mblen ((char *) lineP, MB_CUR_MAX)) < 1)
    {
        *linePP = lineP;
        return (NULL);
    }

    if ((chlen == 1) && (*lineP == '"'))
    /* Quoted string */
    {
	quoteLevel[level] = 1;	
	/*
	 * Start beyond double quote and find the end of the quoted string.
	 * '\' quotes the next character - but is not stripped out.
	 * Otherwise,  matching double quote or NULL terminates the string.
	 *
	 * We use lnwsP to point to the last non-whitespace character in the
	 * quoted string.  When we have found the end of the quoted string,
	 * increment lnwsP and if lnwsP < endP, write NULL into *lnwsP.
	 * This removes any trailing whitespace without overwriting the 
	 * matching quote, needed later.  If the quoted string was all 
	 * whitespace, then this will write a NULL at the beginning of the 
	 * string that will be returned -- OK.
	 */
	lnwsP = lineP++;                /* lnwsP points to first '"' */
	curP = endP = lineP;            /* other pointers point beyond */

        while ((*endP = *curP) &&
               ((chlen = mblen ((char *) curP, MB_CUR_MAX)) > 0) &&
	       ((chlen > 1) || (*curP != '"')))
	/* Haven't found matching quote yet.
	 * First byte of next character has been copied to endP.
	 */
        {
	    curP++;
	    if ((chlen == 1) && (*endP == '\\') && 
		((chlen = mblen ((char *) curP, MB_CUR_MAX)) > 0))
	    {
		/*
		 * Check to see if this is a quoted quote - if it is
		 * strip off a level - if not - it's sacred leave it alone
		 */
		checkLev = PeekAhead((curP - 1), quoteLevel[level]);
		if(checkLev > 0)
		{
		    if(quoteLevel[level] >= checkLev)
		    {
			if (level > 0) level--;
		    }
		    else if (level < MAX_QUOTE_DEPTH)
		    {
			level++;
			quoteLevel[level] = checkLev;
		    }
		    
		    for(i = 0;i < (checkLev - 2);i++)
		    {
			*endP++ = *curP++;curP++;
		    }
		    *endP = *curP++;
		}
            }

	    if (chlen == 1)
	    /* Singlebyte character:  character copy finished. */
	    {
	        if (isspace (*endP))
	        /* whitespace character:  leave lnwsP unchanged. */
	        {
	            endP++;
	        }
	        else
	        /* non-whitespace character:  point lnwsP to it. */
	        {
	            lnwsP = endP++;
	        }
	    }
	    else if (chlen > 1)
	    /* Multibyte (nonwhitespace) character:  point lnwsP to it.
	     * Finish character byte copy.
	     */
	    {
	        lnwsP = endP++;
		while (--chlen)
		{
		    *endP++ = *curP++;
		    lnwsP++;
		}
	    }
        }
#else

    /* get rid of leading white space */
    ScanWhitespace (&lineP);

    /* Return NULL if line is empty, or whitespace */
    if(*lineP == NULL)
    {
        *linePP = lineP;
        return (NULL);
    }

    if (*lineP == '"')
    /* Quoted string */
    {
	quoteLevel[level] = 1;	
	/*
	 * Start beyond double quote and find the end of the quoted string.
	 * '\' quotes the next character.
	 * Otherwise,  matching double quote or NULL terminates the string.
	 *
	 * We use lnwsP to point to the last non-whitespace character in the
	 * quoted string.  When we have found the end of the quoted string,
	 * increment lnwsP and if lnwsP < endP, write NULL into *lnwsP.
	 * This removes any trailing whitespace without overwriting the 
	 * matching quote, needed later.  If the quoted string was all 
	 * whitespace, then this will write a NULL at the beginning of the 
	 * string that will be returned -- OK.
	 */
	lnwsP = lineP++;                /* lnwsP points to first '"' */
	curP = endP = lineP;            /* other pointers point beyond */

        while ((*endP = *curP) && (*endP != '"'))
	/* haven't found matching quote yet */
        {
	    /* point curP to next character */
	    curP++;
	    if ((*endP == '\\') && (*curP != NULL))
	    /* shift quoted nonNULL character down and curP ahead */
	    {
		/*
		 * Check to see if this is a quoted quote - if it is
		 * strip off a level - if not - it's sacred leave it alone
		 */
		checkLev = PeekAhead((curP - 1), quoteLevel[level]);
		if(checkLev > 0)
		{
		    if(quoteLevel[level] >= checkLev)
		    {
			if (level > 0) level--;
		    }
		    else if (level < MAX_QUOTE_DEPTH)
		    {
			level++;
			quoteLevel[level] = checkLev;
		    }
		    
		    for(i = 0;i < (checkLev - 2);i++)
		    {
			*endP++ = *curP++;curP++;
		    }
		    *endP = *curP++;
		}
            }

	    if (isspace (*endP))
	    /* whitespace character:  leave lnwsP unchanged. */
	    {
	        endP++;
	    }
	    else
	    /* non-whitespace character:  point lnwsP to it. */
	    {
	        lnwsP = endP++;
	    }
        }
#endif

	/*
	 *  Found matching quote or NULL.  
	 *  NULL out any trailing whitespace.
	 */

	lnwsP++;
	if (lnwsP < endP)
        {
	    *lnwsP = NULL;
        }
    }

    else
    /* Unquoted string */
    {
        /* 
	 * Find the end of the nonquoted string.
	 * '\' quotes the next character.
	 * Otherwise,  whitespace, NULL, terminates the string.
	 */
        curP = endP = lineP;

#ifdef MULTIBYTE
        while ((*endP = *curP) &&
               ((chlen = mblen ((char *) curP, MB_CUR_MAX)) > 0) &&
               ((chlen > 1) || (!isspace (*curP))))
	/* Haven't found whitespace  yet.
	 * First byte of next character has been copied to endP.
	 */
        {
	    curP++;
	    if ((chlen == 1) && (*endP == '\\') && 
		((chlen = mblen ((char *) curP, MB_CUR_MAX)) > 0))
	    /* character quote:
	     * copy first byte of quoted nonNULL character down.
	     * point curP to next byte
	     */
	    {
		*endP = *curP++;
            }
	    endP++;
	    if (chlen > 1)
	    /* Multibyte character:  finish character copy. */
	    {
		while (--chlen)
		{
		    *endP++ = *curP++;
		}
	    }
        }
#else
        while ((*endP = *curP) && !isspace (*endP))
        {
	    /* point curP to next character */
	    curP++;
	    if ((*endP == '\\') && (*curP != NULL))
	    /* shift quoted nonNULL character down and curP ahead */
	    {
		*endP = *curP++;
            }
	    endP++;
        }
#endif
    }

    /*
     * Two cases for *endP:
     *   whitespace or
     *     matching quote -> write NULL over char and point beyond
     *   NULL -> point to NULL 
     */

    if (*endP != NULL)
    {
	*endP = NULL;       /* write NULL over terminator */
	*linePP = ++curP;   /* point beyond terminator */
    }
    else
    {
	*linePP = endP;
    }
    return ((unsigned char *)lineP);

} /* END OF FUNCTION GetString */




/*************************************<->*************************************
 *
 *  ScanWhitespace(linePP)
 *
 *
 *  Description:
 *  -----------
 *  Scan the string, skipping over all white space characters.
 *
 *
 *  Inputs:
 *  ------
 *  linePP = nonNULL pointer to current line buffer pointer
 *
 * 
 *  Outputs:
 *  -------
 *  linePP = nonNULL pointer to revised line buffer pointer
 *
 *
 *  Comments:
 *  --------
 *  Assumes linePP is nonNULL
 *  Code Stolen from dtmwm
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
ScanWhitespace( linePP )
        unsigned char **linePP ;
#else
ScanWhitespace(
        unsigned char **linePP )
#endif /* _NO_PROTO */
{
#ifdef MULTIBYTE
    while (*linePP && (mblen ((char *) *linePP, MB_CUR_MAX) == 1) && isspace (**linePP))
#else
    while (*linePP && isspace (**linePP))
#endif
    {
        (*linePP)++;
    }

} /* END OF FUNCTION ScanWhitespace */



/*************************************<->*************************************
 *
 *  FillHintBuf(newHint, hintBuf, maxSize, actSize, screen, numHints)
 *
 *
 *  Description:
 *  -----------
 *  Put the new hint into the hint buffer.  Each hint is separated by a 
 *  newline.
 *
 *
 *  Inputs:
 *  ------
 *  newHint = hint to add to the buffer
 *  hintBuf = an array of buffers - one for each screen
 *  maxSize = array of buffers of the current malloced size of each hintBuf
 *  actSize = array of space currently used by each hintBuf
 *  screen  = screen number for this hint
 *  numHints = array of the number of hints for each screen
 *  smGD.numSavedScreens = (global) checked to make sure this hint should be
 *				added.
 * 
 *  Outputs:
 *  -------
 *  hintBuf[screen] = updated hint buf for this screen (newHint added)
 *  maxSize[screen] = enlarged if not big enough or malloced if not done before
 *  actSize[screen] = updated size of the hints buffer
 *  numHints[screen] = updated by 1 if this hint is added
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int
#ifdef _NO_PROTO
FillHintBuf(newHint, hintBuf, maxSize, actSize, screen, numHints)
	unsigned char *newHint;
	unsigned char **hintBuf;
	unsigned int  *maxSize;
	unsigned int  *actSize;
	unsigned int  screen;
	unsigned int  *numHints;
#else
FillHintBuf(
	unsigned char *newHint,
	unsigned char **hintBuf,
	unsigned int  *maxSize,
	unsigned int  *actSize,
	unsigned int  screen,
	unsigned int  *numHints)
#endif
{
    static int hintBufSize = 5000;
    
    /*
     * If the screen that this hint was meant for is not in the current
     * set of available screens, don't save it
     */
    if(screen >= smGD.numSavedScreens)
    {
	return(0);
    }

    /*
     * Check to see if this buffer has been malloc'd before - if it hasn't
     * malloc it.  If it has - check to make sure it's big enough to hold the
     * new information.
     */
    if(maxSize[screen] == 0)
    {
	hintBuf[screen] = (unsigned char *) SM_MALLOC(hintBufSize * sizeof(char));
	if(hintBuf[screen] == NULL)
	{
	    actSize[screen] = 0;
	    PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	    return(-1);
	}
	maxSize[screen] = hintBufSize * sizeof(char);

	/*
	 * Now reserve 4 bytes for the length
	 */
	strcpy((char *)hintBuf[screen], "     \n");
    }
    else
    {
	if((actSize[screen] + strlen((char *)newHint) + 2) >= maxSize[screen])
	{
	    hintBuf[screen] = (unsigned char *)
		SM_REALLOC((char *) hintBuf[screen],
			   maxSize[screen] +
			   (hintBufSize *
			    sizeof(char)));
	    if(hintBuf[screen] == NULL)
	    {
		actSize[screen] = 0;
		PrintErrnoError(DtError, smNLS.cantMallocErrorString);
		return(-1);
	    }
	    maxSize[screen] = maxSize[screen] + (hintBufSize * sizeof(char));
	}
    }

    /*
     * add the new hint AFTER the last newline
     */
    strcat((char *)hintBuf[screen], (char *)newHint);
    strcat((char *)hintBuf[screen], "\n");
    actSize[screen] = strlen((char *)hintBuf[screen]);
    numHints[screen] += 1;
    
    return(0);
}
				      


/*************************************<->*************************************
 *
 *  FillCmdBuf(newCmd, cmdBuf, maxSize, actSize, screen)
 *
 *
 *  Description:
 *  -----------
 *  Put a new command into the command buffer.  The command buffer is just
 *  one big long string of stuff to be executed.
 *
 *
 *  Inputs:
 *  ------
 *  newCmd = command to add to the buffer
 *  cmdBuf = an array of buffers - one for each screen
 *  maxSize = array of buffers of the current malloced size of each cmdBuf
 *  actSize = array of space currently used by each cmdBuf
 *  screen  = screen number for this command
 *  smGD.numSavedScreens = (global) checked to make sure this hint should be
 *				added.
 * 
 *  Outputs:
 *  -------
 *  cmdBuf[screen] = updated command buf for this screen (newCmd added)
 *  maxSize[screen] = enlarged if not big enough or malloced if not done before
 *  actSize[screen] = updated size of the command buffer
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
FillCmdBuf( newCmd, cmdBuf, maxSize, actSize, screen, numCmd )
        unsigned char *newCmd ;
        unsigned char **cmdBuf;
	unsigned int *maxSize;
	unsigned int *actSize;
        unsigned int screen ;
        unsigned int *numCmd ;
#else
FillCmdBuf(
        unsigned char *newCmd,
        unsigned char **cmdBuf,
	unsigned int *maxSize,
	unsigned int *actSize,
        unsigned int screen,
        unsigned int *numCmd )
#endif /* _NO_PROTO */
{
    static int cmdBufSize = 5000;
    
    /*
     * If the screen that this command was meant for is not in the current
     * set of available screens, don't save it
     */
    if(screen >= smGD.numSavedScreens)
    {
	return(0);
    }

    /*
     * Check to see if this buffer has been malloc'd before - if it hasn't
     * malloc it.  If it has - check to make sure it's big enough to hold the
     * new information.
     */
    if(maxSize[screen] == 0)
    {
	cmdBuf[screen] = (unsigned char *) SM_MALLOC(cmdBufSize * sizeof(char));
	if(cmdBuf[screen] == NULL)
	{
	    actSize[screen] = 0;
	    PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	    return(-1);
	}
	maxSize[screen] = cmdBufSize * sizeof(char);
    }
    else
    {
	if((actSize[screen] + strlen((char *)newCmd)) >= maxSize[screen])
	{
	    cmdBuf[screen] = (unsigned char *) SM_REALLOC((char *)cmdBuf[screen],
							maxSize[screen] +
							(cmdBufSize *
							sizeof(char)));
	    if(cmdBuf[screen] == NULL)
	    {
		actSize[screen] = 0;
		PrintErrnoError(DtError, smNLS.cantMallocErrorString);
		return(-1);
	    }
	    maxSize[screen] = maxSize[screen] + (cmdBufSize * sizeof(char));
	}
    }

    if(actSize[screen] == 0)
    {
	strcpy((char *)cmdBuf[screen], (char *)newCmd);
    }
    else
    {	
	strcat((char *)cmdBuf[screen], (char *)newCmd);
    }

    *numCmd = *numCmd + 1;
    strcat((char *)cmdBuf[screen], "\n");
    actSize[screen] = strlen((char *)cmdBuf[screen]);

    return(0);
}
				      


/*************************************<->*************************************
 *
 *  FillRemoteBuf(newCmd, hostName, displayName, remoteBuf,
 *                maxSize, actSize, screen)
 *
 *
 *  Description:
 *  -----------
 *  Put a new command into the remote execution buffer.  The command buffer is
 *  just one big long string of stuff to be executed.
 *
 *
 *  Inputs:
 *  ------
 *  newCmd = command to add to the buffer
 *  hostName = host where command is to be executed from
 *  displayName = display where host is to be executed to
 *  remoteBuf = an array of buffers - one for each screen
 *  maxSize = array of buffers of the current malloced size of each cmdBuf
 *  actSize = array of space currently used by each cmdBuf
 *  screen  = screen number for this command
 *  smGD.numSavedScreens = (global) checked to make sure this hint should be
 *				added.
 * 
 *  Outputs:
 *  -------
 *  cmdBuf[screen] = updated command buf for this screen (newCmd added)
 *                   in remote format (host name and display name)
 *  maxSize[screen] = enlarged if not big enough or malloced if not done before
 *  actSize[screen] = updated size of the command buffer
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
FillRemoteBuf( newCmd, hostName, displayName, remoteBuf, maxSize, actSize, screen )
        unsigned char *newCmd ;
        unsigned char *hostName ;
        unsigned char *displayName ;
        unsigned char *remoteBuf[] ; 
	unsigned int *maxSize ; 
	unsigned int *actSize ; 
        unsigned int screen ;
#else
FillRemoteBuf(
        unsigned char *newCmd,
        unsigned char *hostName,
        unsigned char *displayName,
        unsigned char *remoteBuf[] ,
	unsigned int *maxSize ,
	unsigned int *actSize , 
        unsigned int screen )
#endif /* _NO_PROTO */
{
    unsigned char *string;
    static int remoteBufSize = 5000;
    
    /*
     * If the screen that this command was meant for is not in the current
     * set of available screens, don't save it
     */
    if(screen >= smGD.numSavedScreens)
    {
	return(0);
    }

    /*
     * Check to see if this buffer has been malloc'd before - if it hasn't
     * malloc it.  If it has - check to make sure it's big enough to hold the
     * new information.
     */
    if(maxSize[screen] == 0)
    {
	remoteBuf[screen] = (unsigned char *)
	    SM_MALLOC(remoteBufSize * sizeof(char));
	if(remoteBuf[screen] == NULL)
	{
	    actSize[screen] = 0;
	    PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	    return(-1);
	}
	maxSize[screen] = remoteBufSize * sizeof(char);
    }
    else
    {
	if((actSize[screen] + strlen((char *)newCmd) +
	    strlen((char *)hostName) +
	    strlen((char *)displayName) +
	    strlen("-display ") + 5) >= maxSize[screen])
	{
	    remoteBuf[screen] = (unsigned char *)
		SM_REALLOC((char *)remoteBuf[screen],
			   maxSize[screen] +
			   (remoteBufSize * sizeof(char)));
	    if(remoteBuf[screen] == NULL)
	    {
		actSize[screen] = 0;
		PrintErrnoError(DtError, smNLS.cantMallocErrorString);
		return(-1);
	    }
	    maxSize[screen] = maxSize[screen] + (remoteBufSize * sizeof(char));
	}
    }

    if(actSize[screen] == 0)
    {
	/*
	 * If the buffer is empty fill it with the initial contents
	 */
	strcpy((char *)remoteBuf[screen], (char *)hostName);
	strcat((char *)remoteBuf[screen], " ");
    }
    else
    {
	/*
	 * if this buffer is not emtpy
	 * add the new command BEFORE the last null terminator
	 * Commands for remote executions are separated by newlines
	 */
	strcat((char *)remoteBuf[screen], "\n");
	strcat((char *)remoteBuf[screen], (char *)hostName);
    }

    /*
     * Copy the command in - quote it
     */
    strcat((char *)remoteBuf[screen], " \"");
    string = GetSmartString(&newCmd);
    strcat((char *)remoteBuf[screen], (char *)string);
    strcat((char *)remoteBuf[screen], " ");

    /*
     * Once display name has been put in place - concatenate the
     * rest of the command
     */
    while((string = GetSmartString(&newCmd)) != NULL )
    {
	strcat((char *)remoteBuf[screen], " ");
	strcat((char *)remoteBuf[screen], (char *)string);
    }

    /*
     * Now close off the command with a quote
     */
    strcat((char *)remoteBuf[screen], "\"");

    actSize[screen] = strlen((char *)remoteBuf[screen]);

    /*
     * Bump the remote command counter
     */
    numRemoteExecs++;
    
    return(0);
}
				      

/*************************************<->*************************************
 *
 *  CreateExecString(execString)
 *
 *
 *  Description:
 *  -----------
 *  Create a string that can be fed to a fork and exec by breaking it up
 *  into argc and argv
 *
 *
 *  Inputs:
 *  ------
 *  execString = whole command
 * 
 *  Outputs:
 *  -------
 *  smExecArray = global modified to contain pointers to argc and argv
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void
#ifdef _NO_PROTO
CreateExecString( execString )
char *execString ;
#else
CreateExecString(
		 char *execString)
		 
#endif /* _NO_PROTO */
{
#define ARG_AMT 100

    static int  iSizeArgv = ARG_AMT;

    char *string;
    int  argc = 0;

    if (smExecArray == NULL)
    {
	smExecArray = (char **) XtMalloc (ARG_AMT * sizeof(char *));
    }
    string = (char *) GetSmartString((unsigned char **) &execString);
    while(string != NULL)
    {
	smExecArray[argc] = string; 
	argc++;
	if (argc >= iSizeArgv)
        {
            iSizeArgv += ARG_AMT;
            smExecArray = (char **)
		XtRealloc ((char *)smExecArray, (iSizeArgv * sizeof(char *)));
        }
	string = (char *) GetSmartString((unsigned char **) &execString);
    }

    /*
     *  If the last string is a background character
     *  get rid of it
     */
    if(argc > 0)
    {
	if(!strcmp((char *)smExecArray[argc - 1], "&"))
	{
	    smExecArray[argc - 1] = '\0';
	}
    }
     
    smExecArray[argc] = '\0';

} /* END OF FUNCTION CreateExecString */

				      

/*************************************<->*************************************
 *
 *  ForkClient(execArray)
 *
 *
 *  Description:
 *  -----------
 *  Does a fork and exec on a client - produces error and tries to continue
 *  if the fork fails
 *
 *
 *  Inputs:
 *  ------
 *  execArray = command to fork and exec
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
ForkClient( execArray )
        char *execArray[] ;
#else
ForkClient(
        char *execArray[] )
#endif /* _NO_PROTO */
{
    pid_t  clientFork;
    int	   execStatus, i;
    char   clientMessage[MAXPATHLEN + 30];
	   
    /*
     * Fork and exec the client process
     */
    clientFork = vfork();
    
    /*
     * If the fork fails - Send out an error and return
     */
    if(clientFork < 0)
    {
	PrintErrnoError(DtError, smNLS.cantForkClientString);
	return;
    }
    
    /*
     * Fork succeeded - now do the exec
     */
    if(clientFork == 0)
    {
#ifndef __hpux
	/*
	 * Set the gid of the process back from bin
	 */
#ifndef SVR4
	setregid(smGD.runningGID, smGD.runningGID);
#else
	setgid(smGD.runningGID);
	setegid(smGD.runningGID);
#endif
#endif

	_DtEnvControl(DT_ENV_RESTORE_PRE_DT);

#ifdef __osf__
        setsid();
#else
	(void)setpgrp();
#endif /* __osf__ */
	
	/*
	 * The BMS sets SIGPIPE to SIG_IGN - return it to SIG_DFL before
	 * execing an unknown client
	 */
	sigaction(SIGPIPE, &smGD.defvec, (struct sigaction *) NULL);
	
	execStatus = execvp(execArray[0], execArray);
	if(execStatus != 0)
	{
	    sprintf(clientMessage, ((char *)GETMESSAGE(16, 3, "Unable to exec %s.")), execArray[0]);
	    PrintErrnoError(DtError, clientMessage);
	    SM_EXIT(-1);
	}
    }
}

				      

/*************************************<->*************************************
 *
 *  ForkWM()
 *
 *
 *  Description:
 *  -----------
 *  Fork and exec the default window manager
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
ForkWM()
#else
ForkWM( void )
#endif /* _NO_PROTO */
{
    pid_t  clientFork;
    int	   execStatus, i;

#ifdef __hpux
    /* 
     * These lines were added to support the builtin
     * panacomm dtwm.
     */
    char   *homeDir;
    char   hostName[MAXPATHSM];
    char   displayName[MAXPATHSM],*dpy;

    if( gethostname(hostName, (sizeof(hostName) - 1) ) == 0)
    {
	hostName[MAXPATHSM - 1] = '\0';
	dpy = getenv("DISPLAY");
	homeDir = getenv("HOME");
	if (dpy && homeDir)
	{
	    strcpy(displayName, dpy);
	    dpy = strchr(displayName, ':');
	    if (dpy)
	    {
		*dpy = '\0';
	    }
	    sprintf(tmpExecWmFile, "%s/.dt/bin/%s/%s/dtwm", homeDir,
		    hostName,displayName);
	    if (access(tmpExecWmFile,X_OK) != 0)
	    {
		strcpy(tmpExecWmFile,CDE_INSTALLATION_TOP "/bin/dtwm");
	    }
	    else
	    {
		localWmLaunched = True;
		if (!smGD.userSetWaitWmTimeout)
		{
		    smRes.waitWmTimeout = 60000;
		}
	    }
	}
        else
        {
          strcpy(tmpExecWmFile,CDE_INSTALLATION_TOP "/bin/dtwm");
        }
    }
    else
    {
        strcpy(tmpExecWmFile,CDE_INSTALLATION_TOP "/bin/dtwm");
    }
    /* 
     *     ^^^^^  ^^^^^  ^^^^^  ^^^^^  ^^^^^  ^^^^^  
     * End of  lines were added to support the builtin
     * panacomm dtwm.
     */
#else
     strcpy(tmpExecWmFile,CDE_INSTALLATION_TOP "/bin/dtwm");
#endif /* __hpux */


    /*
     * Fork and exec the client process
     */
    clientFork = vfork();
    
    /*
     * If the fork fails - Send out an error and return
     */
    if(clientFork < 0)
    {
	PrintErrnoError(DtError, smNLS.cantForkClientString);
	return;
    }
    
    /*
     * Fork succeeded - now do the exec
     */
    if(clientFork == 0)
    {
#ifndef __hpux
	/*
	 * Set the gid of the process back from bin
	 */
#ifndef	SVR4
	setregid(smGD.runningGID, smGD.runningGID);
#else
	setgid(smGD.runningGID);
	setegid(smGD.runningGID);
#endif
#endif
	_DtEnvControl(DT_ENV_RESTORE_PRE_DT);

#ifdef __osf__
        setsid();
#else
        (void)setpgrp();
#endif /* __osf__ */

	/* 
	 * These lines were added to support the builtin
	 * panacomm dtwm.
         * This used to be
         * execStatus = execlp(".../dt/bin/dtwm", "dtwm", (char *) 0);
         */

        execStatus = execlp(tmpExecWmFile, "dtwm", (char *) 0);

	/* 
	 *     ^^^^^  ^^^^^  ^^^^^  ^^^^^  ^^^^^  ^^^^^  
	 * End of  lines were added to support the builtin
	 * panacomm dtwm.
	 */

	if(execStatus != 0 && (!localWmLaunched))
	{
	    PrintErrnoError(DtError, GETMESSAGE(16, 4, "Unable to exec process /usr/dt/bin/dtwm.  No window manager will be started."));
	    SM_EXIT(-1);
	}
    }
}
				      

/*************************************<->*************************************
 *
 *  KillParent()
 *
 *
 *  Description:
 *  -----------
 *  Fork a copy of ourselves and kill the parent off so that scripts starting
 *  up the session manager can continue.
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
KillParent()
#else
KillParent( void )
#endif /* _NO_PROTO */
{
    pid_t  clientFork;

    
    /*
     * Fork and exec the client process
     */
    clientFork = fork();
    
    /*
     * If the fork fails - We have to exit so that the rest of the
     * script can continue
     */
    if(clientFork < 0)
    {
	PrintErrnoError(DtError, smNLS.cantForkClientString);
	SM_EXIT(-1);
    }
    
    /*
     * Fork succeeded - now kill the parent
     */
    if(clientFork != 0)
    {
	SM_EXIT(0);
    }

    /*
     * Disassociate from parent
     */
#ifdef __osf__
    setsid();
#else
    setpgrp();
#endif /* __osf__ */
}



/*************************************<->*************************************
 *
 *  WaitForWM ()
 *
 *
 *  Description:
 *  -----------
 *  This routine waits for the window manager to start.  It uses a
 *  resource (waitWmTimeout) with a dynamic default to determine how many
 *  seconds to wait for WM start and then starts clients.
 *
 *
 *  Inputs:
 *  ------
 *  appContext = application context for the window
 *  window = window id for the
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
WaitForWM()
#else
WaitForWM( void )
#endif /* _NO_PROTO */
{
    XEvent              event;
    XtIntervalId	wmTimerId;
    
    XtAddEventHandler(smGD.topLevelWid,
                      0,
                      True,
                      (XtEventHandler)HandleWMClientMessage,
                      (XtPointer) NULL);

    /*
     * Set a timer which stops the block on waiting for the
     * window manager to start
     */
    wmTimeout = False;
    wmTimerId = XtAppAddTimeOut(smGD.appCon, 
				smRes.waitWmTimeout,
				WaitWMTimeout, NULL);
    
    while((smGD.dtwmRunning == False) && (wmTimeout == False))
    {
	XtAppProcessEvent(smGD.appCon, XtIMAll);
    }
    
    XtRemoveTimeOut(wmTimerId);
    XtRemoveEventHandler(smGD.topLevelWid,
                      0,
                      True,
                      (XtEventHandler)HandleWMClientMessage,
                      (XtPointer) NULL);

    return;
} /* END OF FUNCTION WaitForWM */


/*************************************<->*************************************
 *
 *  WaitWMTimeout
 *
 *
 *  Description:
 *  -----------
 *  Timeout procedure the WaitForCommand routine.  It stops a loop waiting
 *  for the window manager to get started.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  wmTimeout = (global) flag that stops the loop
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
WaitWMTimeout( client_data, id )
        XtPointer client_data ;
        XtIntervalId *id ;
#else
WaitWMTimeout(
        XtPointer client_data,
        XtIntervalId *id )
#endif /* _NO_PROTO */
{
    wmTimeout = True;
    return;
} /* END OF FUNCTION WaitWMTimeout */



/*************************************<->*************************************
 *
 *  HandleWMClientMessage
 *
 *
 *  Description:
 *  -----------
 *  This is the event handler registered to recieve the client message
 *  from dtwm when dtwm is ready for business
 *
 *
 *************************************<->***********************************/
static void
#ifdef _NO_PROTO
HandleWMClientMessage( smWidget, dummy, event )
    Widget smWidget;
    XtPointer dummy;
    XEvent *event;
#else
HandleWMClientMessage( Widget smWidget,
                    XtPointer dummy,
                    XEvent *event)
#endif /* _NO_PROTO */
{
    if (event->type == ClientMessage)
    {
        ProcessClientMessage(event);
    }
    return;
} /* END OF FUNCTION HandleWMClientMessage */



#define DISPLAY_NAME	"DISPLAY="


/*************************************<->*************************************
 *
 *  FixEnvironmentData
 *
 *
 *  Description:
 *  -----------
 *  If DISPLAY variable exists in the environment - remove it
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  wmTimeout = (global) flag that stops the loop
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
FixEnvironmentData()
#else
FixEnvironmentData( void )
#endif /* _NO_PROTO */
{
   char **ppchar;
   int i;
   extern char **environ;  /* MODIFIED - DISPLAY is remove if found. */

   for (i=0, ppchar = environ; *ppchar; *ppchar++, i++)
   {
      if ((strncmp (*ppchar, DISPLAY_NAME, strlen(DISPLAY_NAME))) == 0)
      {
         /*
	  * Change the DISPLAY environment variable.
	  */
	 for (; *ppchar; *ppchar++, i++)
	 {
	     environ[i]=environ[i+1];
	 }
	 break;
      }
   }
}



/*************************************<->*************************************
 *
 *  ResetScreenInfo()
 *
 *
 *  Description:
 *  -----------
 *  After one screen is finished - set up the info for the next
 *
 *
 *  Inputs:
 *  ------
 *  cmdBuf - Buffer that holds all the invocation information
 *  screen - Pointer to the screen number that we're currently working on
 *  env - used to set up the environment for changing the display var
 *  displayName - name of the current display
 * 
 *  Outputs:
 *  -------
 *  cmdBuf - old buffers are freed
 *  screen - updated to point to the new screen info
 *  done   - tells whether the clients are done being exec'd
 *  linec - *GLOBAL* sets line being read from
 *  parseP - *GLOBAL*
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
ResetScreenInfo(cmdBuf, screen, cmdSize, done, env, displayName)
unsigned char	**cmdBuf;
int		*screen;
unsigned int	*cmdSize;
Boolean		*done;
char		*env;
char		*displayName;
#else
ResetScreenInfo(unsigned char **cmdBuf,
		int 		*screen,
		unsigned int	*cmdSize,
		Boolean		*done,
		char		*env,
		char		*displayName)
#endif /* _NO_PROTO */
{
    SM_FREE((char *) cmdBuf[*screen]);
    (*screen)++;
    while((cmdSize[*screen] == 0) && (*screen < smGD.numSavedScreens))
    {
	(*screen)++;
    }
    
    if(*screen >= smGD.numSavedScreens)
    {
	*done = True;
    }
    else
    {
	sprintf((char *)env,"DISPLAY=%s%d", displayName, *screen);
	putenv(env);
	
	linec = 0;
	parseP = cmdBuf[*screen];
    }
}



/*************************************<->*************************************
 *
 *  RemoteRequestFailed ()
 *
 *
 *  Description:
 *  -----------
 *  If a request to the command invoker fails, this callback will be called.
 *  It will then try to execute the command by performing a remsh on it.
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
RemoteRequestFailed(message, client_data)
char *message;
void *client_data;
#else
RemoteRequestFailed(char *message,
		    void *client_data)
#endif /* _NO_PROTO */
{
    static char *cmdBuf = NULL;
    static char *tmpCmdBuf = NULL;
    char *errorString = NULL;
    String tmpString;

    static int	cmdBufSize = 0;
    int		i;
    int         tmpSize;
    RemoteReq	*tmpReq = (RemoteReq *) client_data;

    /*
     * If the memory for the buffer has not been malloced - do so
     */
    if(cmdBuf == NULL)
    {
	cmdBuf = SM_MALLOC((200 * sizeof(char)) + 1 );
	if(cmdBuf == NULL)
	{
	    PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	    return;
	}
	cmdBufSize = 200 + 1;

	tmpCmdBuf = SM_MALLOC((200 * sizeof(char)) + 1 );
	if(tmpCmdBuf == NULL)
	{
	    PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	}
    }

    /*
     *	Copy in the host and command field and execute the command
     */

    tmpSize = (strlen(REMOTE_CMD_STRING) + 
	       strlen((char *) tmpReq->hostPtr) +
	       strlen((char *) tmpReq->cmdPtr)  + 1);

    if(tmpSize >= cmdBufSize)
    {
	cmdBuf =  SM_REALLOC(cmdBuf, (tmpSize) * sizeof(char));

	if(cmdBuf == NULL)
	{
	    PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	    cmdBufSize = 0;
	    return;
	}
	cmdBufSize = tmpSize;

	if(tmpCmdBuf != NULL)
	{
	    tmpCmdBuf =  SM_REALLOC(tmpCmdBuf, (tmpSize) * sizeof(char));
	    if(tmpCmdBuf == NULL)
	    {
		PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	    }
	}
    }

    sprintf(cmdBuf, REMOTE_CMD_STRING, tmpReq->hostPtr, tmpReq->cmdPtr);

    /* 
     * save cmdBuf for error message, cmdBuf is changed
     * by CreateExecString
     */
    if (tmpCmdBuf != NULL)
    {
	strcpy(tmpCmdBuf,cmdBuf);
    }

    CreateExecString(cmdBuf);
    if(smExecArray[0] != NULL)
    {
	ForkClient(smExecArray);

	if (tmpCmdBuf != NULL)
	{
	   tmpString = GETMESSAGE(16, 8, 
"An attempt to restore the following\ncommand (using the DT remote execution process)\non host \"%s\" failed:\n\n      %s\n\nThe following execution string will be tried:\n\n   %s\n\n");
	    
           errorString = 
		    (char *)SM_MALLOC((strlen(tmpString) + 
				       strlen((char *)tmpReq->hostPtr) +
				       strlen((char *)tmpReq->cmdPtr)  +
				       strlen(tmpCmdBuf) + 1 ) * 
				      sizeof(char));
		
	   if(errorString == NULL)
	   {
		    PrintErrnoError(DtError, smNLS.cantMallocErrorString);
	   }
	   else
	   {
		    sprintf(errorString, tmpString, tmpReq->hostPtr, 
			    tmpReq->cmdPtr, tmpCmdBuf );
		    PrintError(DtError, errorString);
		    SM_FREE(errorString);
	   }
	}
    }
    /*
     * Now check to make sure that this isn't the last remote request.
     * If so, free the data
     */
    numRemoteExecs--;

    if(numRemoteExecs == 0)
    {
	for(i = 0;i < smGD.numSavedScreens;i++)
	{
	    if(actRemoteSize[i] > 0)
	    {
		SM_FREE((char *) remoteBuf[i]);
	    }
	}
    }
    
    return;
}



/*************************************<->*************************************
 *
 *  RemoteRequestSucceeded ()
 *
 *
 *  Description:
 *  -----------
 *  If a request to the command invoker succeeds, this callback will be called.
 *  It decrements the remote execution counter, and frees the info if
 *  remote executions are finished
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
RemoteRequestSucceeded(message, client_data)
char *message;
void *client_data;
#else
RemoteRequestSucceeded(char *message,
		       void *client_data)
#endif /* _NO_PROTO */
{
    int	i;
    
    numRemoteExecs--;

    if(numRemoteExecs == 0)
    {
	for(i = 0;i < smGD.numSavedScreens;i++)
	{
	    if(actRemoteSize[i] > 0)
	    {
		SM_FREE((char *) remoteBuf[i]);
	    }
	}
    }
}


/*************************************<->*************************************
 *
 *  SetFontGroup ()
 *
 *
 *  Description:
 *  -----------
 *  Prepends font group to the fontpath. 
 *
 *  Inputs:
 *  ------
 *
 *
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/

static void
#ifdef _NO_PROTO
SetFontGroup()
#else
SetFontGroup( void )
#endif /* _NO_PROTO */
{
DIR *dirp;
struct dirent *dp;
DIR *group_dir;
struct dirent *font_group_dir;
XrmValue                fontGroupReturn;
char *GroupRet;
Boolean resRet = False;
char buf[MAXPATHLEN];
char **fgdirs;
char path[MAXPATHLEN];
char fgdir[MAXPATHLEN];
char *name;
char *group_name;
char Fgroup[MAXPATHLEN];
char fgrp_loc[MAXNAMLEN];
FILE *fp;
int found_loc = -1;
int i;
char *homeDir, *lang;
 
 
 
 
       if((resRet = XrmGetResource(
                    XtDatabase(smGD.display), "*fontGroup", "*FontGroup",
                    &GroupRet, &fontGroupReturn)) == True) {
         /*
          * if it is Default, don't do anything. It is already in the
          * fontpath.
          */
        if(strcmp("Default", fontGroupReturn.addr) == 0)
                return;
 
        homeDir = getenv("HOME");
        lang    = getenv("LANG");
 
        fgdirs = (char **) XtMalloc(sizeof(char *) * 3);
 
        fgdirs[0] = (char *)XtMalloc(strlen(DEFAULT_FONT_PATH)
                                                + strlen(lang) + 2);
        sprintf(fgdirs[0], "%s%s/", DEFAULT_FONT_PATH, lang);
 
        fgdirs[1] = (char *)XtMalloc(strlen(SYS_FONT_PATH)
                                                + strlen(lang) + 2);
 
        sprintf(fgdirs[1], "%s%s/", SYS_FONT_PATH, lang);
 
        fgdirs[2] = (char *)XtMalloc(strlen(homeDir)
                        + strlen(USER_FONT_DIR) + strlen(lang) + 2);
 
        sprintf(fgdirs[2], "%s%s%s/", homeDir, USER_FONT_DIR, lang);
        for(i = 0 ; i < 3 ; ++i) {
 
          if((dirp = opendir(fgdirs[i])) == NULL)
                continue;
 
          while((dp = readdir(dirp)) != NULL) {
             name = dp->d_name;
             if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0))
                    continue;
 
             sprintf(fgdir, "%s%s", fgdirs[i], name);
 
             if((group_dir = opendir(fgdir)) == NULL)
                    continue;
 
             while ((font_group_dir = readdir(group_dir)) != NULL) {
              group_name = font_group_dir->d_name;
 
              if ((strcmp(group_name, ".") == 0) ||
                                       (strcmp(group_name, "..") == 0))
                    continue;
 
              sprintf(path, "%s%s/%s", fgdirs[i], name, "sdtfonts.group");
 
              if ((fp = fopen(path, "r")) == NULL)
                   break;     /* go to next directory */
              else  {
                if(fgets(Fgroup, MAXNAMLEN, fp)) {
                   Fgroup[strlen(Fgroup) - 1] = '\0';
                   if(strcmp(Fgroup, fontGroupReturn.addr) == 0) {
                        found_loc = i;
                        strcpy(fgrp_loc, name);
                        break;
                   }
                }
                fclose(fp);
              }
 
            } /* while */
            closedir(group_dir);
 
          } /* while */
          closedir(dirp);
        }  /* for */

	XtFree((char *) *fgdirs);
	XtFree((char *) fgdirs);
 
        if(found_loc != -1) {
           sprintf(buf, "%s %s %s%s %s %s%s %s", XSET,
                   "-fp", fgdirs[found_loc], fgrp_loc,
                   "+fp", fgdirs[found_loc], fgrp_loc,
                   "> /dev/null 2>/dev/null");
           system(buf);
	   prepend_iso8859();
        }else
           fprintf(stderr,"Font  group '%s' not found.\n",fontGroupReturn.addr);
 
 
#ifdef DEBUG
        fprintf(stderr,"Font group is %s \n", buf);
#endif
       }
 
}


static void
#ifdef _NO_PROTO
prepend_iso8859()
#else
prepend_iso8859()
#endif /* _NO_PROTO */
{
Boolean iso8859_found = False;
int     i = 0, charset_count = 0;
char    buf[MAXNAMLEN];
char    *xlfd_name = "NULL";
char    *def_str, **charset_list;



    XCreateFontSet (smGD.display, xlfd_name, &charset_list,
                        &charset_count, &def_str);

    if(charset_count == 0)
	return;

     for(i=0; i < charset_count; ++i) {
        if(strncasecmp(charset_list[i], "iso8859", strlen("iso8859")) == 0) {
          iso8859_found = True;
          break;
        }
     }
 
     if(!iso8859_found)  {
          sprintf(buf,"%s %s %s %s %s %s", XSET,
                               "-fp",
                               "/usr/openwin/lib/X11/fonts/F3bitmaps/",
                               "+fp",
                               "/usr/openwin/lib/X11/fonts/F3bitmaps/",
                               "> /dev/null 2>/dev/null");
          system(buf);
     }
}

