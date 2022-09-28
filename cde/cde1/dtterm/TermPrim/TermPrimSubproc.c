#ifndef lint
#ifdef  VERBOSE_REV_INFO
static char rcs_id[] = "$XConsortium: TermPrimSubproc.c /main/cde1_maint/4 1995/10/09 11:49:44 pascale $";
#endif  /* VERBOSE_REV_INFO */
#endif  /* lint */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "TermHeader.h"
#include <fcntl.h>
#ifdef  ALPHA_ARCHITECTURE
/* For TIOCSTTY definitions */
#include <sys/ioctl.h>
#endif /* ALPHA_ARCHITECTURE */
#include <sys/wait.h>

#include <signal.h>
#include <pwd.h>
#include <errno.h>

#include <Xm/Xm.h>
#if defined(HPVUE)
#include <Xv/EnvControl.h>
#else    /* HPVUE */
#include <Dt/EnvControl.h>
#endif   /* HPVUE */

#include "TermPrimP.h"
#include "TermPrimI.h"
#include "TermPrimGetPty.h"
#include "TermPrimSetPty.h"
#include "TermPrimSubproc.h"
#include "TermPrimDebug.h"
#include "TermPrimUtil.h"

#include <Dt/EnvControl.h>

extern  int __xpg4;  /* defined in _xpg4.c; 0 if not xpg4-compiled program */

typedef struct _subprocInfo {
    pid_t pid;
    int stat_loc;
    Widget w;
    _termSubprocProc proc;
    XtPointer client_data;
    struct _subprocInfo *next;
    struct _subprocInfo *prev;
} subprocInfo;
    
static subprocInfo _subprocHead;
static subprocInfo *subprocHead = &_subprocHead;

static pid_t
FakeFork
(
    void
)
{
    static Boolean debugInit = True;
    static int debugForkFailures = 0;
    char *c;

    if (isDebugFSet('f', 10)) {
#ifdef	BBA
#pragma BBA_IGNORE
#endif	/*BBA*/
	if (debugInit) {
	    if (c = getenv("dttermDebugForkFailures")) {
		debugForkFailures = strtol(c, (char **) 0, 0);
		debugInit = 0;
	    }
	}
	if (debugForkFailures > 0) {
	    /* decrement the number of failures... */
	    (void) debugForkFailures--;

	    /* set our error return... */
	    errno = EAGAIN;

	    /* and error out... */
	    return(-1);
	}
    }

    /* just do a fork()... */
    return(fork());
}

/* and use our FakeFork... */
#define	fork	FakeFork


_termSubprocId
_DtTermPrimAddSubproc(Widget w, pid_t pid, _termSubprocProc proc,
	XtPointer client_data)
{
    subprocInfo *subprocTmp;

    /* malloc a new entry... */
    subprocTmp = (subprocInfo *) XtMalloc(sizeof(subprocInfo));
    (void) memset(subprocTmp, '\0', sizeof(subprocInfo));

    /* insert it after the head of the list... */
    subprocTmp->prev = subprocHead;
    subprocTmp->next = subprocHead->next;
    subprocHead->next = subprocTmp;
    if (subprocTmp->next) {
	subprocTmp->next->prev = subprocTmp;
    }
    /* fill in the structures... */
    subprocTmp->pid = pid;
    subprocTmp->w = w;
    subprocTmp->proc = proc;
    subprocTmp->client_data = client_data;

    /* return the pointer... */
    return((_termSubprocId) subprocTmp);
}

void
_DtTermPrimSubprocRemoveSubproc(Widget w, _termSubprocId id)
{
    subprocInfo *subprocTmp = (subprocInfo *) id;

    /* remove the entry from the linked list...
     */
    /* there will always be a head, so we can always update it... */
    subprocTmp->prev->next = subprocTmp->next;
    if (subprocTmp->next) {
	subprocTmp->next->prev = subprocTmp->prev;
    }

    /* free our storage... */
    (void) XtFree((char *) subprocTmp);
}

static Boolean resetHandler = False;

/*ARGSUSED*/
static void
ReapChild(int sig)
{
    pid_t pid;
    int stat_loc;

    pid = waitpid(-1, &stat_loc, WNOHANG);
    (void) DtTermSubprocReap(pid, &stat_loc);
}

void
_DtTermPrimSetChildSignalHandler()
{
    (void) signal(SIGCHLD, ReapChild);
    resetHandler = True;
}

/*ARGSUSED*/
static void
InvokeCallbacks(XtPointer client_data, XtIntervalId *id)
{
    subprocInfo *subprocTmp = (subprocInfo *) client_data;

    (void) (subprocTmp->proc)(subprocTmp->w, subprocTmp->pid,
	    &subprocTmp->stat_loc);
}

void
DtTermSubprocReap(pid_t pid, int *stat_loc)
{
    subprocInfo *subprocTmp;

    if (pid > 0) {
	/* find the subprocInfo structure for this subprocess... */
	for (subprocTmp = subprocHead->next; subprocTmp;
		subprocTmp = subprocTmp->next) {
	    /* if the pids match... */
	    if (subprocTmp->pid == pid)
		/* we found the entry... */
		break;
	}

	if (subprocTmp && subprocTmp->w &&
		!subprocTmp->w->core.being_destroyed) {
	    subprocTmp->stat_loc = *stat_loc;
	    (void) XtAppAddTimeOut(
		    XtWidgetToApplicationContext(subprocTmp->w), 0,
		    InvokeCallbacks, (XtPointer) subprocTmp);
	}
    }

    /*DKS (see below)*/
    /* if we were ever instructed to install a signal handler, then
     * we will reinstall it.  The whole issue of SIGCHLD hanlders
     * needs to be thought out in detail and documented...
     */
    if (resetHandler)
	(void) _DtTermPrimSetChildSignalHandler();

    return;
}

pid_t
_DtTermPrimSubprocExec
(
    Widget		  w,
    char		 *ptyName,
    Boolean		  consoleMode,
    char		 *cwd,
    char		 *cmd,
    char		**argv,
    Boolean		  loginShell
)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    static char *defaultCmd = (char *) 0;
    int i;
    int pty;
    pid_t pid;
    char *c;
    char *c2;
    int err;
#ifdef	MOVE_FDS
    int saveFd[3];
#else	/* MOVE_FDS */
    int savedStderr;
#endif	/* MOVE_FDS */
    struct passwd *pw;
    Boolean argvFree = False;
    struct sigaction sa;
    sigset_t ss;
    char buffer[BUFSIZ];
    Widget parent;
    
#ifdef  ALPHA_ARCHITECTURE
    /* merge code from xterm, ignore so that TIOCSWINSZ doesn't block */
    signal(SIGTTOU, SIG_IGN);
#endif /* ALPHA_ARCHITECTURE */

    /* build a default exec command and argv list if one wasn't supplied...
     */
    /* cmd... */
    /* the command will be taken as follows:
     *	    - from the passed in cmd,
     *	    - from $SHELL,
     *	    - from the /etc/passwd entry for the /etc/utmp entry for this
     *        terminal,
     *	    - from the /etc/passwd entry for this userid, or
     *	    - /bin/sh.
     */
    if (!cmd || !*cmd) {
	if (!defaultCmd) {
	    /* from $SHELL... */
	    c = getenv("SHELL");

	    /* if not valid, try the /etc/passwd entry for the username
	     * associated with the /etc/utmp entry for this tty...
	     */
	    if (!c || !*c) {
		/* get the /etc/passwd entry for the username associated with
		 * /etc/utmp...
		 */
		if (c2 = getlogin()) {
		    /* get the user's passwd entry... */
		    pw = getpwnam(c2);
		    /* if we weren't able to come up with one for the
		     * username...
		     */
		    if (pw)
			c = pw->pw_shell;
		}
	    }

	    /* if not valid, try the /etc/passwd entry for the username
	     * associate with the real uid...
	     */
	    if (!c || !*c) {
		/* if we weren't able to come up with one for the userid... */
		pw = getpwuid(getuid());
		if (pw) {
		    c = pw->pw_shell;
		}
	    }

	    /* if not valid, use /bin/sh... */
	    if (!c || !*c) {
		c = DEFAULT_SHELL;
	    }

	    /* malloc space for this string.  It will be free'ed in the
	     * destroy function...
	     */
	    defaultCmd = XtMalloc(strlen(c) + 1);
	    (void) strcpy(defaultCmd, c);
	}

	cmd = defaultCmd;
    }

    if (!argv) {
	/* base it on cmd... */
	argv = (char **) XtMalloc(2 * sizeof(char *));
	/* if loginShell is set, then pre-pend a '-' to argv[0].  That's
	 * also why we allocate an extra byte in argv[0]...
	 */
	argv[0] = XtMalloc(strlen(cmd) + 2);
	*argv[0] = '\0';
	if (loginShell) {
	    /* pre-pend an '-' for loginShell... */
	    (void) strcat(argv[0], "-");
	    if (c = strrchr(cmd, '/')) {
		strcat(argv[0], ++c);
	    } else {
		strcat(argv[0], cmd);
	    }
	} else {
	    (void) strcat(argv[0], cmd);
	}
	/* null term the list... */
	argv[1] = (char *) 0;

	/* we will need to free it up later... */
	argvFree = True;
    }

#ifdef	OLDCODE
    /* this is left around from when we were using vfork().... */
    /* open the pty slave so that we can set the modes.
     *
     * NOTE: this code depends on support for the O_NOCTTY ioctl.  This
     *     ioctl allows us to open the device without becoming the
     *     session group leader for it.  If that can't be done, it may
     *     be necessary to rethink the way we open the pty slave...
     */
    if ((pty = open(ptyName, O_RDWR | O_NOCTTY, 0)) < 0) {
	(void) perror(ptyName);
	return((pid_t) -1);
    }
#endif	/* OLDCODE */

#ifdef	MOVE_FDS
    /* move fd[0:2] out of the way for now... */
    for (i = 0; i <= 2; i++) {
	saveFd[i] = fcntl(i, F_DUPFD, 3);
	(void) close(i);
    }
#else	/* MOVE_FDS */
    savedStderr = fcntl(2, F_DUPFD, 3);
#endif	/* MOVE_FDS */

    /* set close on exec flags on all files... */
    for (i = 0; i < _NFILE; i++) {
	(void) fcntl(i, F_SETFD, 1);
    }

    /* fork.  We can't use vfork() since we need to do lots of stuff
     * below...
     */
    if (isDebugSet('T')) {
#ifdef	BBA
#pragma BBA_IGNORE
#endif	/*BBA*/
	(void) timeStamp("about to fork()");
    }

    for (i = 0; ((pid = fork()) < 0) && (i < 10); i++) {
	/* if we are out of process slots, then let's sleep a bit and
	 * try again...
	 */
	if (errno != EAGAIN) {
	    break;
	}

	/* give it a chance to clear up... */
	(void) sleep((unsigned long) 2);
    }

    if (pid < 0) {
	(void) perror("fork()");
#ifdef	OLDCODE
	/* this is left around from when we were using vfork().... */
	(void) close(pty);
#endif	/* OLDCODE */
	return((pid_t) - 1);
    } else if (pid == 0) {
	/* child...
	 */
#ifdef  ALPHA_ARCHITECTURE
        /* establish a new session for child */
        setsid();
#else
	/* do a setpgrp() so that we can... */
	(void) setpgrp();
#endif /* ALPHA_ARCHITECTURE */

	/* open the pty slave as our controlling terminal... */
	pty = open(ptyName, O_RDWR, 0);

	if (pty < 0) {
	    (void) perror(ptyName);
	    (void) _exit(1);
	}

#ifdef  ALPHA_ARCHITECTURE
        /* BSD needs to do this to acquire pty as controlling terminal */
        if (ioctl(pty, TIOCSCTTY, (char *)NULL) < 0) {
	    (void) close(pty);
	    (void) perror("Error acquiring pty slave as controlling terminal");
	    /* exit the subprocess */
	    _exit(1);
        }

        /* Do it when no controlling terminal doesn't work for OSF/1 */
        _DtTermPrimPtyGetDefaultModes();
#endif /* ALPHA_ARCHITECTURE */

	/* set the ownership and mode of the pty... */
	if (__xpg4 == 0)	/* not XPG4 */
	    (void) _DtTermPrimSetupPty(ptyName, pty);

	/* apply the ttyModes... */
	(void) _DtTermPrimPtyInit(pty, tw->term.ttyModes, tw->term.csWidth);
	/* set the window size... */
	(void) _DtTermPrimPtySetWindowSize(pty,
		tw->term.columns * tw->term.widthInc +
		(2 * (tw->primitive.shadow_thickness +
		      tw->primitive.highlight_thickness +
		      tw->term.marginWidth)),
		tw->term.rows * tw->term.heightInc +
		(2 * (tw->primitive.shadow_thickness +
		      tw->primitive.highlight_thickness +
		      tw->term.marginHeight)),
		tw->term.rows, tw->term.columns);

	/* if we are in console mode, turn it on... */
	if (consoleMode) {
	    _DtTermPrimPtyConsoleModeEnable(pty);
	}

#ifdef	MOVE_FDS
	/* that should have open'ed into fd 0.  Dup it into fd's 1 and 2... */
	(void) dup(pty);
	(void) dup(pty);
#else	/* MOVE_FDS */
	/* dup pty into fd's 0, 1, and 2... */
	for (i = 0; i < 3; i++) {
	    if (i != pty) {
		(void) close(i);
		(void) dup(pty);
	    }
	}
	if (pty >= 3) {
	    (void) close(pty);
	}
#endif	/* MOVE_FDS */

	/* reset any alarms... */
	(void) alarm(0);

	/* reset all signal handlers... */
	sa.sa_handler = SIG_DFL;
	(void) sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	for (i = 1; i < NSIG; i++) {
	    (void) sigaction(i, &sa, (struct sigaction *) 0);
	}

	/* unblock all signals... */
	(void) sigemptyset(&ss);
	(void) sigprocmask(SIG_SETMASK, &ss, (sigset_t *) 0);

	/*
	** Restore the original (pre-DT) environment, removing any
	** DT-specific environment variables that were added before
	** we...
	*/
#if defined(HPVUE)
#if       (OSMINORVERSION > 01)
	(void) VuEnvControl(VUE_ENV_RESTORE_PRE_VUE);
#endif /* (OSMINORVERSION > 01) */
#else   /* (HPVUE) */  
	(void) _DtEnvControl(DT_ENV_RESTORE_PRE_DT);
#endif  /* (HPVUE) */  
            
	/*
	** set a few environment variables of our own...
	*/
	for (parent = w; !XtIsShell(parent); parent = XtParent(parent))
	    ;
	(void) sprintf(buffer, "%ld", XtWindow(parent));
	(void) _DtTermPrimPutEnv("WINDOWID=", buffer);
	(void) _DtTermPrimPutEnv("DISPLAY=", XDisplayString(XtDisplay(w)));
	if (((DtTermPrimitiveWidget)w)->term.emulationId) {
	    (void) _DtTermPrimPutEnv("TERMINAL_EMULATOR=",
		    ((DtTermPrimitiveWidget)w)->term.emulationId);
	}
		 
	/* set our utmp entry... */
	(void) _DtTermPrimUtmpEntryCreate(w, getpid(),
		((DtTermPrimitiveWidget)w)->term.tpd->utmpId);

	if (isDebugSet('T')) {
#ifdef	BBA
#pragma BBA_IGNORE
#endif	/*BBA*/
	    (void) timeStamp("about to execvp()");
	}

	/* turn off suid forever...
	 */
	_DtTermPrimRemoveSuidRoot();

	/* change to the requested directory... */
	if (cwd && *cwd) {
	    (void) chdir(cwd);
	}

#ifdef	BBA
	_bA_dump();
#endif	/* BBA */
	_DtEnvControl(DT_ENV_RESTORE_PRE_DT);
	(void) execvp(cmd, argv);
	/* if we got to this point we error'ed out.  Let's write out the
	 * error...
	 */
	err = errno;
	/* restore stderr... */
	(void) close(2);
	(void) dup(savedStderr);
	/* restore errno... */
	errno = err;
	(void) perror(cmd);
	/* and we need to exit the subprocess... */
	_exit(1);
    }

    /* parent...
     */
    if (isDebugSet('T')) {
#ifdef	BBA
#pragma BBA_IGNORE
#endif	/*BBA*/
	(void) timeStamp("parent resuming");
    }
#ifdef	MOVE_FDS
    /* DKS: we should check this out and see if it is necessary... */
    (void) close(0);
    (void) close(1);
    (void) close(2);
    /* move fd[0:2] back in place... */
    for (i = 0; i <= 2; i++) {
	if (saveFd[i] >= 0) { 
	    (void) fcntl(saveFd[i], F_DUPFD, i);
	    (void) close(saveFd[i]);
	}
    }
#else	/* MOVE_FDS */
    (void) close(savedStderr);
#endif	/* MOVE_FDS */

    /* clean up malloc'ed memory... */
    if (argvFree) {
	(void) XtFree(argv[0]);
	(void) XtFree((char *) argv);
    }

#ifdef	OLDCODE
    /* since we no longer open it in the parent, we probably don't want
     * to close it either...
     */
    (void) close(pty);
#endif	/* OLDCODE */

    /* assume that our child set up a utmp entry (since we have no way
     * for it to report to us) and add it to the list to cleanup)...
     */
    _DtTermPrimUtmpAddEntry(((DtTermPrimitiveWidget)w)->term.tpd->utmpId);

    return(pid);
}
