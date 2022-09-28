/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: dm.h /main/cde1_maint/6 1995/12/01 13:59:08 rcs $
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
 * dm.h
 *
 * public interfaces for greet/verify functionality
 */


#ifndef _DM_H
#define _DM_H

#ifndef _NO_PROTO
#if    !defined(__STDC__) && !defined(__cplusplus) && !defined(c_plusplus)
#define _NO_PROTO
#endif /* __STDC__ */
#endif /* _NO_PROTO */


/***************************************************************************
 *
 *  Includes
 *
 ***************************************************************************/

# include	<errno.h>		/* for errno			   */
# include       <pwd.h>                 /* for passwd structure            */
# include	<stdio.h>
# include	<stdlib.h>		/* for exit(), malloc(), abort()   */
# include	<string.h>		/* for string functions, bcopy(),
					   sys_errlist			   */
# include	<sys/param.h>		/* for NGROUPS			   */
# include	<sys/types.h>   	/* for fd_set			   */
# include	<netinet/in.h>		/* for Internet socket stuff	   */

#ifdef _BSD
# include	<strings.h>		/* for BSD string functions	   */
#endif

# include	<X11/Xlib.h>
# include	<X11/Xos.h>
# include	<X11/Xmd.h>
# include	<X11/Xauth.h>
# include	<X11/Xdmcp.h>
#ifdef SVR4 /*** needed for bcopy bcmp ***/
#include    <X11/Xlibint.h>
#endif      /* SVR4 */

# include <X11/Xresource.h>

#ifndef LAST_SESSION_FILE
#define LAST_SESSION_FILE "/.dt/sessions/lastsession"
#endif

#ifndef CDE_INSTALLATION_TOP
#define CDE_INSTALLATION_TOP "/opt/dt"
#endif

#ifndef REMOTE_GREET_HOST_FILE
#define REMOTE_GREET_HOST_FILE "/var/dt/remote_greet_host"
#endif

#ifndef SDTLOGIN_DIR 
#define SDTLOGIN_DIR "/var/dt/sdtlogin"
#endif

#ifndef SDTLASTLANG 
#define SDTLASTLANG "/var/dt/sdtlogin/lastlang"
#endif

#define DEF_PATH        "/usr/bin:"     /* same as PATH */
#define DEF_SUPATH      "/usr/sbin:/usr/bin" /* same as ROOTPATH */

#define LANGLISTSIZE    2048
#define DELIM           " \t"   /* delimiters in language list             */


/***************************************************************************
 *
 *  Defines
 *
 ***************************************************************************/

#if defined(_AIX)
  #define DEF_NLS_DIR	"/usr/lib/nls/loc" /* default NLS directory for AIX */
#elif defined(sun)
  #define DEF_NLS_DIR 	 "/usr/lib/locale"  /* default Sun locale directories */
  #define DEF_X11_NLS_SHARE_DIR	"/usr/openwin/share/locale"  
  #define DEF_X11_NLS_LIB_DIR	"/usr/openwin/lib/locale"  
#elif defined(hpux)
  #define DEF_NLS_DIR	"/usr/lib/nls/msg" /* default NLS directory for HPUX */
#else
  #define DEF_NLS_DIR	CDE_INSTALLATION_TOP "/lib/nls"	/* default NLS dir */
#endif

/**************************************************************************
 * 
 * /etc/utmp
 *
 **************************************************************************/

/* Default dummy device name (/etc/utmp "tty line" for foreign displays ) */
#define DEF_NETWORK_DEV "/dev/dtremote"

/*
 * Pseudo-tty file creation routine 
 *
 *   For remote connections, the value for 'line' in /etc/utmp must also
 *   exist as a device in the /dev directory for commands such as 'finger'
 *   to operate properly. 
 * 
 *   For most platforms, /dev/dtremote will simply be a symbolic link
 *   to the /dev/null device. 
 *
 *   For AIX, /dev/dtremote will be a character special file whose major
 *   and minor numbers are the same as /dev/null. This is the case since
 *   the AIX init command will chown and chmod the 'line' device if dtlogin
 *   dies while owned by init. If /dev/dtremote were a symlink to /dev/null,
 *   /dev/null whould be chown/chmod such that regular users could no longer
 *   write to it. 
 */
#ifdef _AIX
#define MK_NETWORK_DEV_PERMS (S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define MK_NETWORK_DEV(DEV) (mknod(DEV, MK_NETWORK_DEV_PERMS, makedev(2, 2)))
#else
#define MK_NETWORK_DEV(DEV) (symlink("/dev/null",DEV))
#endif

/* Utmp rec prefix */
#define UTMPREC_PREFIX  "dt"

#ifdef SIGNALRETURNSINT
#define SIGVAL int
#else
#define SIGVAL void
#endif


#ifndef UID_NO_CHANGE
#  define UID_NO_CHANGE  ((uid_t) -1)  /* for chown(2) and setresuid(2) */
#endif

#ifndef GID_NO_CHANGE
#  define GID_NO_CHANGE  ((gid_t) -1)  /* for chown(2) and setresgid(2) */
#endif

#ifndef _BSD
#ifdef SVR4
#define bzero(x,y)	memset((void *)(x),0,(y))	/* use ANSI C version	   */
#else
#define bzero(x,y)	memset((x),0,(y))	/* use ANSI C version	   */
#endif
#endif

#define IsVerifyName(n) ((d->verifyName && (strcmp(d->verifyName,(n)) == 0 )) \
			  ? TRUE : FALSE)


/***************************************************************************
 *
 *  wait() 
 *
 ***************************************************************************/

/*#if defined(SYSV) && !defined(hpux)*/
#if defined(SYSV) || defined(SVR4)
#   include	<sys/wait.h>
# define waitCode(w)	WEXITSTATUS(w)
# define waitSig(w)	WTERMSIG(w)
#if defined(_AIX)
    /*
     * hpux has a "WCOREDUMP" macro which is undefined in AIX.
     * For AIX, the macro from hpux's <sys/wait.h> is used.
     */
# define waitCore(w)    ( ((int)(w)&0200) == 0 ? 0 : 1 )
#else
# define waitCore(w)    ( WCOREDUMP(w) == 0 ? 0 : 1 )
#endif /* _AIX */
typedef int		waitType;
#else
# ifdef _BSD
#   include	<sys/wait.h>
# else
#   define _BSD
#   include	<sys/wait.h>
#   undef _BSD
# endif
# define waitCode(w)	((w).w_T.w_Retcode)
# define waitSig(w)	((w).w_T.w_Termsig)
# define waitCore(w)    ((w).w_T.w_Coredump)
typedef union wait	waitType;
#endif

# define waitCompose(sig,core,code) ((sig) * 256 + (core) * 128 + (code))
/*
# define waitVal(w)	waitCompose(waitSig(w), waitCore(w), waitCode(w))
*/
# define waitVal(w)	waitCode(w)




/***************************************************************************
 *
 *  select() 
 *
 ***************************************************************************/

#ifndef FD_ZERO
typedef	struct	my_fd_set { int fds_bits[1]; } my_fd_set;
# define FD_ZERO(fdp)	bzero ((fdp), sizeof (*(fdp)))
# define FD_SET(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] |=  (1 << ((f) % (sizeof (int) * 8))))
# define FD_CLR(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] &= ~(1 << ((f) % (sizeof (int) * 8))))
# define FD_ISSET(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] & (1 << ((f) % (sizeof (int) * 8))))
# define FD_TYPE	my_fd_set
#else
# define FD_TYPE	fd_set
#endif




/***************************************************************************
 *
 *  Defines and structures for display management
 *
 ***************************************************************************/

typedef enum displayStatus { running, notRunning, zombie, phoenix, suspended }
	     DisplayStatus;

typedef enum fileState { NewEntry, OldEntry, MissingEntry } FileState;

/*
 * local     - server runs on local host
 * foreign   - server runs on remote host
 * permanent - session restarted when it exits
 * transient - session not restarted when it exits
 * fromFile  - started via entry in servers file
 * fromXDMCP - started with XDMCP
 */

typedef struct displayType {
	unsigned int	location:1;
	unsigned int	lifetime:1;
	unsigned int	origin:1;
} DisplayType;


# define Local		1
# define Foreign	0

# define Permanent	1
# define Transient	0

# define FromFile	1
# define FromXDMCP	0

/*
 * gettyState values
 *   NONE - no getty running or don't care
 *   LOGIN - getty running
 *   USER - user logged in on getty
 */
#define DM_GETTY_NONE 	0
#define DM_GETTY_LOGIN	1
#define DM_GETTY_USER 	2

struct display {
	struct display	*next;
	/* Xservers file / XDMCP information */
	char		*name;		/* DISPLAY name */
	char		*class;		/* display class (may be NULL) */
	DisplayType	displayType;	/* method to handle with */
	char		**argv;		/* program name and arguments */

	/* display state */
	DisplayStatus	status;		/* current status */
	int		pid;		/* process id of child */
	int		serverPid;	/* process id of server (-1 if none) */
	FileState	state;		/* state during HUP processing */
	int		startTries;	/* current start try */
	int		gettyState;	/* current getty state */
	int	        unexpectedServerDeath;

	/* XDMCP state */
	CARD32		sessionID;	/* ID of active session */
	struct sockaddr	*peer;		/* sockaddr of display peer */
	int		peerlen;	/* length of peer name */
	struct sockaddr	*from;		/* XDMCP port of display */
	int		fromlen;
	CARD16		displayNumber;
        int             useChooser;     /* Run the chooser for this display */
        ARRAY8          clientAddr;     /* for chooser picking */
        CARD16          connectionType; /* ... */

#ifdef BYPASSLOGIN
        int             bypassLogin;    /* bypass login for this display */
#endif /* BYPASSLOGIN */

	/* server management resources */
	int		serverAttempts;	/* number of attempts at running X */
	int		openDelay;	/* open delay time */
	int		openRepeat;	/* open attempts to make */
	int		openTimeout;	/* abort open attempt timeout */
	int		startAttempts;	/* number of attempts at starting */
	int		pingInterval;	/* interval between XSync */
	int		pingTimeout;	/* timeout for XSync */
	int		terminateServer;/* restart for each session */
	int		grabServer;	/* keep server grabbed for Login */
	int		grabTimeout;	/* time to wait for grab */
	int		resetSignal;	/* signal to reset server */
	int		termSignal;	/* signal to terminate server */
	int		resetForAuth;	/* server reads auth file at reset */

	/* session resources */
	char		*resources;	/* resource file */
	char		*xrdb;		/* xrdb program */
	char		*cpp;		/* cpp program */
        char            *setup;         /* Xsetup program */
	char		*startup;	/* Xstartup program */
	char		*reset;		/* Xreset program */
	char		*session;	/* Xsession program */
	char		*userPath;	/* path set for session */
	char		*systemPath;	/* path set for startup/reset */
	char		*systemShell;	/* interpreter for startup/reset */
	char		*failsafeClient;/* a client to start when the session fails */
        char            *chooser;       /* chooser program */

	/* authorization resources */
	int		authorize;	/* enable authorization */
#if defined(__osf__)
        char            **authNames;    /* authorization protocol name */
        unsigned short  *authNameLens;  /* authorization protocol name len */
#else
	char		*authName;	/* authorization protocol name */
	unsigned short	authNameLen;	/* authorization protocol name len */
#endif
	char		*clientAuthFile;/* client specified auth file */
	char		*userAuthDir;	/* backup directory for tickets */

	/* information potentially derived from resources */
#if defined(__osf__)
        int             authNameNum;    /* number of protocol names */
        Xauth           **authorizations;       /* authorization data */
        int             authNum;        /* number of authorizations */
#else
	Xauth		*authorization;	/* authorization data */
#endif
	char		*authFile;	/* file to store authorization in */
	char		*language;	/* value for LANG env variable */
	char		*langList;	/* list of languages on login screen */
	char		*utmpId;	/* id for entry in utmp file */
	char		*gettyLine;	/* line to run getty on */
	char		*gettySpeed;	/* speed for getty (from gettydefs) */
	char		*environStr;	/* environment variable resource */
	int		dtlite;	/* boolean for HP DT Lite session */
	int		xdmMode;	/* boolean for XDM style session */
	int		sessionType;	/* default, XDM, DT, or DTLITE session  */
	char		*verifyName;	/* default, Kerberos, AFS, B1  */
        char            *pmSearchPath;  /* motif pixmap search path */
        char            *bmSearchPath;  /* motif bitmap search path */
};

#define PROTO_TIMEOUT	(30 * 60)   /* 30 minutes should be long enough */

struct protoDisplay {
	struct protoDisplay	*next;
	struct sockaddr		*address;   /* UDP address */
	int			addrlen;    /* UDP address length */
	unsigned long		date;	    /* creation date */
	CARD16			displayNumber;
	CARD16			connectionType;
	ARRAY8			connectionAddress;
	CARD32			sessionID;
	Xauth			*fileAuthorization;
	Xauth			*xdmcpAuthorization;
	ARRAY8			authenticationName;
	ARRAY8			authenticationData;
	XdmAuthKeyRec		key;
};


struct greet_info {
	char            *name;          /* user name */
	char            *password;      /* user password */
#ifdef BLS
	char            *b1security;    /* user's b1 security */
#endif
	char            *string;        /* random string */
#ifdef __PASSWD_ETC
        char            *name_full;	/* full SID */
#endif
};

struct verify_info {
	int             uid;            /* user id */
#ifdef NGROUPS
	int             groups[NGROUPS];/* group list */
	int             ngroups;        /* number of elements in groups */
#else
	int             gid;            /* group id */
#endif
	char            **argv;         /* arguments to session */
	char            **userEnviron;  /* environment for session */
	char            **systemEnviron;/* environment for startup/reset */
#ifdef AUDIT
	long            audid;          /* audit id */
	int             audflg;         /* audit flag */
#endif
#ifdef BLS
	char *user_name;
	struct mand_ir_t *sec_label_ir;
	struct mand_ir_t *clearance_ir;
	/* save these for logout time */
	struct pr_passwd *prpwd;
	struct passwd *pwd;
	char terminal[16];
#endif
};

# define FIRST_LOGIN "/usr/dt/bin/sdt_firstlogin"


/* display manager exit status definitions */

# define OBEYSESS_DISPLAY	0	/* obey multipleSessions resource */
# define REMANAGE_DISPLAY	1	/* force remanage */
# define UNMANAGE_DISPLAY	2	/* force deletion */
# define RESERVER_DISPLAY	3	/* force server termination */
# define OPENFAILED_DISPLAY	4	/* XOpenDisplay failed, retry */
# define SUSPEND_DISPLAY	5	/* suspend server while getty is run */
# define START_CHOOSER	        6	/* choose from list selected */
# define START_REMOTE_GREET     7	/* remote greeter selected */
# define RETURN_LOGIN_SCREEN    13      /* return to local login screen from 
					   chooser */


#ifdef BYPASSLOGIN
# define GREETLESS_FAILED       6
#endif /* BYPASSLOGIN */

/*
 * CloseOnFork flags
 */

# define CLOSE_ALWAYS	    0
# define LEAVE_FOR_DISPLAY  1




/***************************************************************************
 *
 *  External variable declarations
 *
 ***************************************************************************/

extern char	*sys_errlist[];		/* system error msgs		   */
extern int	sys_nerr;		/* system error msgs		   */
extern XrmDatabase  XresourceDB; 



extern char	*config;

extern char	*sysParmsFile;
extern char	*accessFile;
extern char	*servers;
extern int	request_port;
extern int	debugLevel;
extern char	*errorLogFile;
extern int	errorLogSize;
extern int	daemonMode;
extern char	*pidFile;
extern int	lockPidFile;
extern char	*authDir;
extern int	autoRescan;
extern int	removeDomainname;
extern char	*keyFile;
extern char	*timeZone;
extern int	wakeupInterval;
extern char	*fpHead;
extern char	*fpTail;
extern int      langListTimeout;
#ifdef DEF_NETWORK_DEV
extern char	*networkDev;
#endif

#if !defined (ENABLE_DYNAMIC_LANGLIST)
extern char	languageList[];		/* util.c			   */
#endif /* ENABLE_DYNAMIC_LANGLIST */

extern char     **exportList;  /* list of export env vars  RK    08.17.93 */




/****************************************************************************
 *
 *  Public procedure declarations
 *
 ****************************************************************************/
#ifdef _NO_PROTO

extern int  AcceptableDisplayAddress() ;	/* access.c		   */
extern void ForEachChooserHost() ;		/* access.c		   */
extern int  ForEachMatchingIndirectHost() ;	/* access.c		   */
extern int  ScanAccessDatabase() ;		/* access.c		   */
extern int  UseChooser() ;			/* access.c		   */
extern ARRAY8Ptr getLocalAddress() ;		/* access.c		   */

extern void Account() ;				/* account.c		   */
extern int  UtmpIdOpen() ;			/* account.c		   */

extern int  DoLogin() ;				/* apollo.c		   */

extern int  ConvertAddr() ;			/* auth.c		   */
extern int  ConvertFamily() ;			/* auth.c		   */
extern int  SaveServerAuthorizations() ;        /* auth.c                  */
extern void SetAuthorization() ;		/* auth.c		   */
extern void SetLocalAuthorization() ;		/* auth.c		   */
extern void SetProtoDisplayAuthorization() ;	/* auth.c		   */
extern void SetUserAuthorization() ;		/* auth.c		   */
extern int  ValidAuthorization() ;		/* auth.c		   */

extern void GenerateCryptoKey() ;		/* cryptokey.c		   */
extern int  InitCryptoKey() ;			/* cryptokey.c		   */

extern void BecomeDaemon() ;			/* daemon.c		   */

extern int  CloseOnFork() ;			/* dm.c			   */
extern void GettyMessage() ;			/* dm.c			   */
extern void RegisterCloseOnFork() ;		/* dm.c			   */
extern void SetTitle() ;			/* dm.c			   */
extern void StartDisplay() ;			/* dm.c			   */
extern void StartDisplays() ;			/* dm.c			   */
extern void StopDisplay() ;			/* dm.c			   */
extern void WaitForChild() ;			/* dm.c			   */
extern int  main() ;				/* dm.c			   */
extern int  GettyRunning() ;                    /* dm.c                    */

extern int  AnyDisplaysLeft() ;			/* dpylist.c		   */
extern struct display * FindDisplayByAddress(); /* dpylist.c		   */
extern struct display * FindDisplayByName() ;	/* dpylist.c		   */
extern struct display * FindDisplayByPid() ;	/* dpylist.c		   */
extern struct display * FindDisplayByServerPid(); /* dpylist.c		   */
extern struct display * FindDisplayBySessionID(); /* dpylist.c		   */
extern void ForEachDisplay() ;			/* dpylist.c		   */
extern struct display * NewDisplay() ;		/* dpylist.c		   */
extern void RemoveDisplay() ;			/* dpylist.c		   */

extern void Debug() ;				/* error.c		   */
extern void InitErrorLog() ;			/* error.c		   */
extern void CheckErrorFile() ;			/* error.c		   */
extern void LogError() ;			/* error.c		   */
extern void LogInfo() ;				/* error.c		   */
extern void LogOutOfMem() ;			/* error.c		   */
extern void LogPanic() ;			/* error.c		   */
extern int  Panic() ;				/* error.c		   */
extern int  SyncErrorFile() ;			/* error.c		   */
extern void TrimErrorFile() ;			/* error.c		   */

extern void ParseDisplay() ;			/* file.c		   */

extern Xauth * MitGetAuth() ;			/* mitauth.c		   */
#if defined(__osf__)
extern int MitInitAuth() ;			/* mitauth.c		   */
#else
extern void MitInitAuth() ;			/* mitauth.c		   */
#endif

extern ARRAY8Ptr Accept() ;			/* policy.c		   */
extern int  CheckAuthentication() ;		/* policy.c		   */
extern ARRAY8Ptr ChooseAuthentication() ;	/* policy.c		   */
extern int  SelectAuthorizationTypeIndex() ;	/* policy.c		   */
extern int  SelectConnectionTypeIndex() ;	/* policy.c		   */
extern int  Willing() ;				/* policy.c		   */

extern void DisposeProtoDisplay() ;		/* protodpy.c		   */
extern void TimeoutProtoDisplays() ;		/* protodpy.c		   */
extern int  addressEqual() ;			/* protodpy.c		   */
extern struct protoDisplay *FindProtoDisplay();	/* protodpy.c		   */
extern struct protoDisplay *NewProtoDisplay();	/* protodpy.c		   */

extern void pseudoReset() ;			/* reset.c		   */

extern void GetResource() ;			/* resource.c		   */
extern void InitResources() ;			/* resource.c		   */
extern void LoadDisplayResources() ;		/* resource.c		   */
extern void LoadDMResources() ;			/* resource.c		   */
extern void ReinitResources() ;			/* resource.c		   */
extern void SetAppName() ;			/* resource.c		   */

extern int  LogOpenError() ;			/* server.c		   */
extern int  PingServer() ;			/* server.c		   */
extern void ResetServer() ;			/* server.c		   */
extern int  StartServer() ;			/* server.c		   */
extern int  StartRemoteGreet_Chooser() ;        /* server.c		   */
extern int  StartServerOnce() ;			/* server.c		   */
extern int  WaitForServer() ;			/* server.c		   */

extern void ManageSession() ;			/* session.c		   */
extern int  execute() ;				/* session.c		   */
extern int  source() ;				/* session.c		   */

extern int  AnyWellKnownSockets() ;		/* socket.c		   */
extern int  CreateWellKnownSockets() ;		/* socket.c		   */
extern void DestroyWellKnownSockets() ;		/* socket.c		   */
extern char * NetworkAddressToHostname() ;	/* socket.c		   */
extern char * NetworkAddressToName() ;		/* socket.c		   */
extern void SendFailed() ;			/* socket.c		   */
extern void WaitForSomething() ;		/* socket.c		   */
extern char * localHostname() ;			/* socket.c		   */

extern void CleanUpChild() ;			/* util.c		   */
extern Cursor GetHourGlass() ;			/* util.c		   */
#if !defined (ENABLE_DYNAMIC_LANGLIST)
extern void MakeLangList() ;			/* util.c		   */
#endif
extern void SetHourGlassCursor() ;		/* util.c		   */
extern char * getEnv() ;			/* util.c		   */
extern char ** parseArgs() ;			/* util.c		   */
extern char ** parseEnv() ;			/* util.c		   */
extern void printEnv() ;			/* util.c		   */
extern char ** setEnv() ;			/* util.c		   */
extern unsigned char * ReadCatalog() ;          /* util.c                  */
extern char ** setLang() ;			/* util.c		   */

extern void getGroups() ;			/* verify.c		   */
extern int groupMember() ;			/* verify.c		   */
extern char ** setDt() ;			/* verify.c		   */
extern char ** systemEnv() ;			/* verify.c		   */
extern char ** userEnv() ;			/* verify.c		   */
extern int Verify() ;				/* verify.c		   */
extern int defopen();
extern char *login_defaults();

#ifdef HASDES
extern int  XdmCheckAuthentication() ;		/* xdmauth.c		   */
extern Xauth * XdmGetAuth() ;			/* xdmauth.c		   */
extern int  XdmGetKey() ;			/* xdmauth.c		   */
extern int  XdmGetXdmcpAuth() ;			/* xdmauth.c		   */
extern int  XdmInitAuth() ;			/* xdmauth.c		   */
#endif



#else


/*******************************     access.c    **************************/

extern int  AcceptableDisplayAddress( 
                        ARRAY8Ptr clientAddress,
#if NeedWidePrototypes
                        int connectionType,
#else
                        CARD16 connectionType,
#endif /* NeedWidePrototypes */
                        xdmOpCode type) ;

extern void ForEachChooserHost( 
                        ARRAY8Ptr clientAddress,
#if NeedWidePrototypes
                        int connectionType,
#else
                        CARD16 connectionType,
#endif /* NeedWidePrototypes */
                        void (*function)(),
                        char *closure) ;
extern int  ForEachMatchingIndirectHost( 
                        ARRAY8Ptr clientAddress,
#if NeedWidePrototypes
                        int connectionType,
#else
                        CARD16 connectionType,
#endif /* NeedWidePrototypes */
                        void (*function)(),
                        char *closure) ;
extern int  ScanAccessDatabase( void ) ;
extern int  UseChooser( 
                        ARRAY8Ptr clientAddress,
#if NeedWidePrototypes
                        int connectionType) ;
#else
                        CARD16 connectionType) ;
#endif /* NeedWidePrototypes */
extern ARRAY8Ptr getLocalAddress( void ) ;


/*******************************     account.c    **************************/

extern void Account( 
			struct display *d, 
			char *user, 
			char *line, 
			pid_t pid,
#if NeedWidePrototypes
                        int type,
#else
                        short type,
#endif /* NeedWidePrototypes */
                        waitType exitcode) ;

extern int  UtmpIdOpen( 
			char *utmpId) ;


/*******************************     apollo.c     **************************/

extern int  DoLogin(
			char *user, 
			char *passwd, 
			char *host) ;


/*******************************     auth.c       **************************/

extern int  ConvertAddr(
                        XdmcpNetaddr saddr,
                        int *len,                   /* return */
                        char **addr);               /* return */

extern int  ConvertFamily(
			int sockfamily);

extern int  SaveServerAuthorizations(
                        struct display *d, 
                        Xauth **auth,
                        int count) ;
extern void SetAuthorization( 
			struct display *d) ;
extern void SetLocalAuthorization( 
			struct display *d) ;
extern void SetProtoDisplayAuthorization( 
			struct protoDisplay *pdpy,
#if NeedWidePrototypes
                        unsigned int authorizationNameLen,
#else
                        unsigned short authorizationNameLen,
#endif /* NeedWidePrototypes */
                        char *authorizationName) ;
extern void SetUserAuthorization( 
			struct display *d, 
			struct verify_info *verify) ;
extern int  ValidAuthorization( 
#if NeedWidePrototypes
                        unsigned int name_length,
#else
                        unsigned short name_length,
#endif /* NeedWidePrototypes */
                        char *name) ;


/*******************************     cryptokey.c  **************************/

extern void GenerateCryptoKey(
			char *auth,
			int len) ;
extern int  InitCryptoKey( void ) ;


/*******************************     daemon.c     **************************/

extern void BecomeDaemon( void ) ;


/*******************************     dm.c         **************************/

extern int  CloseOnFork( void ) ;
extern void GettyMessage(
			struct display *d,
			int msgnum) ;
extern void RegisterCloseOnFork(
			int fd) ;
extern void SetTitle(
			char *name,
			char *ptr) ;
extern void StartDisplay(
			struct display *d) ;
extern void StartDisplays( void ) ;
extern void StopDisplay(
			struct display *d) ;
extern void WaitForChild( void ) ;
extern int  main(
			int argc,
			char **argv) ;
extern int  GettyRunning( struct display *d) ;


/*******************************     dpylist.c	  **************************/

extern int  AnyDisplaysLeft( void ) ;

extern struct display * FindDisplayByAddress(
			struct sockaddr *addr,
			int addrlen,
#if NeedWidePrototypes
                        int displayNumber) ;
#else
                        CARD16 displayNumber) ;
#endif /* NeedWidePrototypes */

extern struct display * FindDisplayByName(
			char *name) ;
extern struct display * FindDisplayByPid(
			int pid) ;
extern struct display * FindDisplayByServerPid(
			int serverPid) ;
extern struct display * FindDisplayBySessionID(
			CARD32 sessionID) ;

extern void ForEachDisplay(
			void (*f)()) ;
extern struct display * NewDisplay(
			char *name,
			char *class) ;
extern void RemoveDisplay(
			struct display *old) ;


/*******************************     error.c	  **************************/

extern void Debug(
			char *fmt, ...) ;
extern void InitErrorLog( void ) ;
extern void CheckErrorFile( void ) ;
extern void LogError(
			unsigned char *fmt, ...) ;
extern void LogInfo(
			unsigned char *fmt, ...) ;
extern void LogOutOfMem(
			unsigned char *fmt, ...) ;
extern void LogPanic(
			unsigned char *fmt, ...) ;
extern int  Panic(
			char *mesg ) ;
extern int  SyncErrorFile(
			int stamp ) ;
extern void TrimErrorFile( void ) ;


/*******************************     file.c	  **************************/

extern void ParseDisplay( char *source,
			 DisplayType *acceptableTypes,
                         int numAcceptable,
			 struct passwd *puser );


/*******************************     mitauth.c	  **************************/

extern Xauth * MitGetAuth( 
#if NeedWidePrototypes
                        unsigned int namelen,
#else
                        unsigned short namelen,
#endif /* NeedWidePrototypes */
                        char *name) ;

#if defined(__osf__)
extern int MitInitAuth( 
#else
extern void MitInitAuth( 
#endif /* __osf__ */
#if NeedWidePrototypes
                        unsigned int name_len,
#else
                        unsigned short name_len,
#endif /* NeedWidePrototypes */
                        char *name) ;


/*******************************     policy.c	  **************************/

extern ARRAY8Ptr Accept(
			struct sockaddr *from,
			int fromlen,
#if NeedWidePrototypes
                        int displayNumber) ;
#else
                        CARD16 displayNumber) ;
#endif /* NeedWidePrototypes */

extern int CheckAuthentication(
			struct protoDisplay *pdpy,
			ARRAY8Ptr displayID,
	                ARRAY8Ptr name,
	                ARRAY8Ptr data) ;
extern ARRAY8Ptr ChooseAuthentication(
			ARRAYofARRAY8Ptr authenticationNames) ;
extern int SelectAuthorizationTypeIndex(
			ARRAY8Ptr authenticationName,
			ARRAYofARRAY8Ptr authorizationNames) ;
extern int SelectConnectionTypeIndex(
			ARRAY16Ptr connectionTypes,
			ARRAYofARRAY8Ptr connectionAddresses) ;

extern int Willing( 
                        ARRAY8Ptr addr,
#if NeedWidePrototypes
                        int connectionType,
#else
                        CARD16 connectionType,
#endif /* NeedWidePrototypes */
                        ARRAY8Ptr authenticationName,
                        ARRAY8Ptr status,
                        xdmOpCode type) ;
extern ARRAY8Ptr Accept( 
                        struct sockaddr *from,
                        int fromlen,
#if NeedWidePrototypes
                        int displayNumber) ;
#else
                        CARD16 displayNumber) ;
#endif /* NeedWidePrototypes */

/*******************************     protodpy.c	  **************************/

extern void DisposeProtoDisplay(
			struct protoDisplay *pdpy) ;
extern void TimeoutProtoDisplays(
			long now) ;
extern int  addressEqual(
                        XdmcpNetaddr a1,
                        int          len1,
                        XdmcpNetaddr a2,
                        int          len2);
extern struct protoDisplay * FindProtoDisplay(
			struct sockaddr *address,
                        int addrlen,
#if NeedWidePrototypes
                        int displayNumber) ;
#else
                        CARD16 displayNumber) ;
#endif /* NeedWidePrototypes */

extern struct protoDisplay * NewProtoDisplay(
			struct sockaddr *address,
			int addrlen,
#if NeedWidePrototypes
			int displayNumber,
			int connectionType,
#else
			CARD16 displayNumber,
			CARD16 connectionType,
#endif /* NeedWidePrototypes */
			ARRAY8Ptr connectionAddress,
			CARD32 sessionID) ;



/*******************************     reset.c	  **************************/

extern void pseudoReset(
			Display *dpy) ;


/*******************************     resource.c	  **************************/

extern void GetResource(
			char *name,
			char *class,
			int valueType,
                        char **valuep,
			char *default_value) ;
extern void InitResources(
			int argc,
			char **argv) ;
extern void ReinitResources( void ) ;
extern void LoadDMResources( void ) ;
extern void LoadDisplayResources(
			struct display *d) ;
extern void SetAppName( void ) ;


/*******************************     server.c	  **************************/

extern int  LogOpenError(
			int  count) ;
extern int  PingServer(
			struct display *d,
			Display *alternateDpy) ;
extern void ResetServer(
			struct display *d) ;
extern int  StartServer(
			struct display *d) ;
extern int  StartRemoteGreet_Chooser(
			struct display *d,
			int client) ;
extern int  StartServerOnce(
			struct display *d) ;
extern int  WaitForServer(
			struct display *d) ;


/*******************************     session.c	  **************************/

extern void ManageSession(
			struct display *d) ;
extern int  execute(
			char **argv,
			char **environ) ;
extern int  source(
			struct verify_info *verify,
			char *file) ;



/*******************************     socket.c	  **************************/

extern int  AnyWellKnownSockets( void ) ;
extern int  CreateWellKnownSockets( void ) ;
extern void DestroyWellKnownSockets( void ) ;
extern char * NetworkAddressToHostname( 
#if NeedWidePrototypes
                        int connectionType,
#else
                        CARD16 connectionType,
#endif /* NeedWidePrototypes */
                        ARRAY8Ptr connectionAddress) ;
extern char * localHostname( void ) ;
extern char * NetworkAddressToName( 
#if NeedWidePrototypes
                        int connectionType,
#else
                        CARD16 connectionType,
#endif /* NeedWidePrototypes */
                        ARRAY8Ptr connectionAddress,
#if NeedWidePrototypes
                        int displayNumber) ;
#else
                        CARD16 displayNumber) ;
#endif /* NeedWidePrototypes */
extern void SendFailed( 
                        struct display *d,
                        char *reason) ;
extern void WaitForSomething( void ) ;

extern char * localHostname( void ) ;



/*******************************     util.c	  **************************/

extern void CleanUpChild( void ) ;
extern Cursor GetHourGlass(
			Display *dpy) ;
#if !defined (ENABLE_DYNAMIC_LANGLIST)
extern void MakeLangList( void ) ;
#endif
extern void SetHourGlassCursor(
			Display *dpy,
			Window w) ;
extern char * getEnv(
			char **e,
			char *name) ;
extern char ** parseArgs(
			char **argv,
			char *string) ;
extern char ** parseEnv(
			char **e,
			char *string) ;
extern void printEnv(
			char **e) ;
extern char ** setEnv(
			char **e,
			char *name,
			char *value) ;
extern unsigned char * ReadCatalog(
                        int  set_num,
                        int  msg_num,
                        char *def_str) ;
extern char ** setLang(
		        struct display *d, 
		        char **env , 
		        char *langptr);

/*******************************     verify.c	  **************************/

extern void getGroups( 
                        char *name,
                        struct verify_info *verify,
                        int gid) ;
extern int groupMember( 
                        char *name,
                        char **members) ;
extern char ** setDt( 
                        struct display *d,
                        char **argv,
                        int dt_type,
			struct verify_info *verify,
			char *last_sess) ;
extern char ** systemEnv( 
                        struct display *d,
                        char *user,
                        char *home) ;
extern char ** userEnv( 
                        struct display *d,
                        char *user,
                        char *home,
                        char *shell,
		        struct passwd   *p) ;
extern int Verify( 
                        struct display *d,
                        struct greet_info *greet,
                        struct verify_info *verify) ;

extern  int             defopen(char *filename);
 
extern  char *  login_defaults(struct passwd *p,
                        struct display *d);

 
 
 



/*******************************     version.c	  **************************/
/*******************************     xdmauth.c	  **************************/
#ifdef HASDES
extern int XdmInitAuth( 
#if NeedWidePrototypes
                        unsigned int name_len,
#else
                        unsigned short name_len,
#endif /* NeedWidePrototypes */
                        char *name) ;

extern Xauth * XdmGetAuth( 
#if NeedWidePrototypes
                        unsigned int namelen,
#else
                        unsigned short namelen,
#endif /* NeedWidePrototypes */
                        char *name) ;

extern int XdmGetXdmcpAuth( 
                        struct protoDisplay *pdpy,
#if NeedWidePrototypes
                        unsigned int authorizationNameLen,
#else
                        unsigned short authorizationNameLen,
#endif /* NeedWidePrototypes */
                        char *authorizationName) ;

extern int XdmGetKey( 
                        struct protoDisplay *pdpy,
                        ARRAY8Ptr displayID) ;

extern int XdmCheckAuthentication( 
                        struct protoDisplay *pdpy,
                        ARRAY8Ptr displayID,
                        ARRAY8Ptr authenticationName,
                        ARRAY8Ptr authenticationData) ;


#endif /* HASDES */

#endif /* _NO_PROTO */

#endif /* _DM_H */
