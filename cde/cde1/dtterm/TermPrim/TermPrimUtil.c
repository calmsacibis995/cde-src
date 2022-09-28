#ifndef lint
#ifdef  VERBOSE_REV_INFO
static char rcs_id[] = "$XConsortium: TermPrimUtil.c /main/cde1_maint/2 1995/10/09 11:49:53 pascale $";
#endif  /* VERBOSE_REV_INFO */
#endif  /* lint */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "TermHeader.h"
#include "TermPrimOSDepI.h"
#include "TermPrimP.h"
#include "TermPrimData.h"
#include "TermPrimUtil.h"
#include "TermPrimDebug.h"
#include "Xm/Xm.h"
#include <signal.h>
#include <sys/file.h>

void _DtTermPrimRemoveSuidRoot();
static void PointerMoved(Widget w, XtPointer closure, XEvent *event, Boolean *cont) ;
void _DtTermPrimPointerOff(Widget w, XtIntervalId *id) ;
static Pixmap               noPointerBitmap;
static XColor  cdef,cdef2;
/*
** the following white pixmap is used to create the noPointer (blank) pointer.
*/
#define whiteW  16
#define whiteH 16
static char whiteBits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static void
InitPointerBlank(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    DtTermPrimData tpd = tw->term.tpd;
    /*
    ** set a pointer motion handler...
    */
    tpd->pointerTimeoutID = 0 ;
    tpd->pointerFrozen = True ;
    noPointerBitmap = XCreateBitmapFromData(XtDisplay(w),XtWindow(tw),
                                whiteBits, whiteW, whiteH);

    tpd->noPointer   = XCreatePixmapCursor(XtDisplay(tw),
                                   noPointerBitmap,  /* source bitmap    */
                                   noPointerBitmap,  /* dest bitmap      */
                                   &cdef, &cdef2, /* fg, bg (don't care) */
                                   0, 0);         /* hot spot            */
    XtAddEventHandler((Widget)tw, PointerMotionMask, FALSE, PointerMoved, 
                                       (XtPointer)NULL);

    tpd->pointerFirst = False ;
}

static void 
PointerMoved(Widget w, XtPointer closure, XEvent *event, Boolean *cont)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    DtTermPrimData tpd = tw->term.tpd;

    if (!tpd->pointerFrozen) {
        if (tpd->pointerOn) {
            /*
            ** pointer is on, just reset the timer...
            */
            if (tw->term.pointerBlankDelay) {
                /*
                ** remove the old motion timeout...
                */
                if (tpd->pointerTimeoutID)
                    XtRemoveTimeOut(tpd->pointerTimeoutID);

                /*
                ** and set a new motion timeout...
                */
                tpd->pointerTimeoutID = 
                      XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)tw),
                      (unsigned long) 1000 * tw->term.pointerBlankDelay,
                      (XtTimerCallbackProc) _DtTermPrimPointerOff,
                      (XtPointer)tw);
            }
        } else {
            /* pointer is off, turn it on... */
            (void) _DtTermPrimPointerOn(w);
        }
    }
}

void
_DtTermPrimPointerOff(Widget w, XtIntervalId *id)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    DtTermPrimData tpd = tw->term.tpd;

    if (tpd->pointerFirst) InitPointerBlank(w) ;

    if (tpd->pointerOn) {
        /*
        ** define the window's cursor...
        */
        (void) XDefineCursor(XtDisplay(tw), XtWindow(tw), tpd->noPointer);

        /*
        ** remove the motion timeout...
        */
        if (tw->term.pointerBlankDelay) {
            if (tpd->pointerTimeoutID)
                XtRemoveTimeOut(tpd->pointerTimeoutID);
            tpd->pointerTimeoutID = 0;
        }

        /*
        ** and clear the pointer on flag...
        */
        tpd->pointerOn = False;
    }
    return;
}

void 
_DtTermPrimPointerOn(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    DtTermPrimData tpd = tw->term.tpd;

    if (tpd->pointerFirst) InitPointerBlank(w) ;

    if (!tpd->pointerOn) {
        /*
        ** define the window's cursor...
        */
        XDefineCursor(XtDisplay(tw), XtWindow(tw), tw->term.pointerShape);

        /*
        ** set a motion timeout...
        */
        if (tw->term.pointerBlankDelay) {
            if (tpd->pointerTimeoutID)
                /*
                ** remove old timeout...
                */
                XtRemoveTimeOut(tpd->pointerTimeoutID);

            tpd->pointerTimeoutID = 
                    XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)tw),
                    (unsigned long) (1000 * tw->term.pointerBlankDelay),
                    (XtTimerCallbackProc)_DtTermPrimPointerOff, (XtPointer)tw);
        }
        /*
        ** and set the pointer on flag...
        */
        tpd->pointerOn = True;
    }
    return;
}

void 
_DtTermPrimPointerFreeze(Widget w, Boolean freeze)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    DtTermPrimData tpd = tw->term.tpd;

    tpd->pointerFrozen = freeze;

    /*
    ** make sure that the pointer is on...
    */
    if (tpd->pointerOn) {
        /*
        ** pointer is on...
        */
        if (tw->term.pointerBlankDelay) {
            if (freeze) {
                /*
                ** freezing -- turn the timeout off...
                */
                if (tpd->pointerTimeoutID)
                    XtRemoveTimeOut(tpd->pointerTimeoutID);

                tpd->pointerTimeoutID = 0;
            } 
            else {
                /*
                ** un freezing -- turn the timeout on...
                */
                if (tpd->pointerTimeoutID)
                    /*
                    ** remove old timeout...
                    */
                    XtRemoveTimeOut(tpd->pointerTimeoutID);

                tpd->pointerTimeoutID = 
                       XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)tw),
                       (unsigned long) 1000 * tw->term.pointerBlankDelay,
                       (XtTimerCallbackProc) _DtTermPrimPointerOff,
                       (XtPointer)tw);
            }
        }
    } 
    else {
        /*
        ** let's turn on the pointer...
        ** define the window's cursor...
        */
        XDefineCursor(XtDisplay(tw), XtWindow(tw), tw->term.pointerShape);

        if (freeze)
            /*
            ** the timeout is off, so we don't need to clear it...
            */
            /* NOOP */ ;
        else
            /*
            ** we are unfreezing -- turn the timeout on...
            */
            if (tw->term.pointerBlankDelay) {
                if (tpd->pointerTimeoutID)
                    /*
                    ** remove old timeout...
                    */
                    XtRemoveTimeOut(tpd->pointerTimeoutID);

                tpd->pointerTimeoutID = 
                      XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)tw),
                      (unsigned long) (1000 * tw->term.pointerBlankDelay),
                      (XtTimerCallbackProc)_DtTermPrimPointerOff,(XtPointer)tw);
            }
        /*
        ** and set the flag...
        */
        tpd->pointerOn = True;
    }
    return;
}

void
_DtTermPrimRecolorPointer(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;

    XColor colordefs[2];        /* 0 is foreground, 1 is background */
    Display *dpy = XtDisplay(w);

    colordefs[0].pixel = tw->term.pointerColor;
    colordefs[1].pixel = tw->term.pointerColorBackground;
    XQueryColors (dpy, DefaultColormap (dpy, DefaultScreen (dpy)),
          colordefs, 2);
    XRecolorCursor (dpy, tw->term.pointerShape, colordefs, colordefs+1);
    return;
}

/* linked list of log files to flush if we are killed...
 */
typedef struct _logInfo {
    FILE *logFile;
    struct _logInfo *next;
    struct _logInfo *prev;
} logInfo;

static logInfo _logInfoHead;
static logInfo *logInfoHead = &_logInfoHead;

static void
AddLogFileEntry
(
    FILE		 *logFile
)
{
    logInfo		 *logInfoTmp;
    sigset_t		  newSigs;
    sigset_t		  oldSigs;

    /* malloc a new entry... */
    logInfoTmp = (logInfo *) XtMalloc(sizeof(logInfo));
    (void) memset(logInfoTmp, '\0', sizeof(logInfo));

    /* fill in the structure... */
    logInfoTmp->logFile = logFile;

    /* insert it after the head of the list...
     */
    /* block all signals... */
    (void) sigfillset(&newSigs);
    (void) sigemptyset(&oldSigs);
    (void) sigprocmask(SIG_BLOCK, &newSigs, &oldSigs);
    /* insert the entry into the list... */
    logInfoTmp->prev = logInfoHead;
    logInfoTmp->next = logInfoHead->next;
    logInfoHead->next = logInfoTmp;
    if (logInfoTmp->next) {
	logInfoTmp->next->prev = logInfoTmp;
    }
    /* restore signals... */
    (void) sigprocmask(SIG_SETMASK, &oldSigs, (sigset_t *) 0);
}

static void
DeleteLogFileEntry
(
    FILE		 *logFile
)
{
    logInfo		 *logInfoTmp;
    sigset_t		  newSigs;
    sigset_t		  oldSigs;

    /* find the entry... */
    for (logInfoTmp = logInfoHead->next; logInfoTmp;
	    logInfoTmp = logInfoTmp->next) {
	if (logInfoTmp->logFile == logFile) {
	    break;
	}
    }

    /* did we find anything... */
    if (!logInfoTmp) {
	/* not found... */
	return;
    }

    /* delete entry from the list...
     */
    /* block all signals... */
    (void) sigfillset(&newSigs);
    (void) sigemptyset(&oldSigs);
    (void) sigprocmask(SIG_BLOCK, &newSigs, &oldSigs);
    /* remove it... */
    logInfoTmp->prev->next = logInfoTmp->next;
    if (logInfoTmp->next) {
	logInfoTmp->next->prev = logInfoTmp->prev;
    }
    /* restore signals... */
    (void) sigprocmask(SIG_SETMASK, &oldSigs, (sigset_t *) 0);

    /* free up the data... */
    (void) XtFree((char *) logInfoTmp);
}

#ifdef NOTDEF
void 
logpipe(Widget w)
{
    win_data *wp = &term->Wp;

    if (wp->log_on) { CloseLog(wp); }
}
#endif  /* NOTDEF */


void
_DtTermPrimStartLog(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    DtTermPrimData tpd = tw->term.tpd;

    char            *cp;
    int              i;

    if ( tw->term.log_on || tw->term.logInhibit ) { return; }

    if (!tw->term.logFile || !*tw->term.logFile) {
	tw->term.logFile = "DttermLogXXXXX";
    }

    if (!strcmp(tw->term.logFile + strlen(tw->term.logFile) - 5, "XXXXX")) {
	/* make a local copy in case we are going to change it... */
	cp = XtMalloc(strlen(tw->term.logFile) + 1);
	(void) strcpy(cp, tw->term.logFile);

        (void) mktemp(cp);
	if (cp && *cp) {
	    tw->term.logFile = cp;
	} else {
	    (void) XtFree(cp);
	    return;
	}
    }

    if ('|' == *tw->term.logFile ) {
        /*
        ** pipe logfile into command
        */
        int p[2];

        if (pipe(p) < 0 || (i = fork()) < 0) {
	    return;
	}

        if (i == 0) {
            /*
            ** child
            */

	    /* Remove suid root capability...
	     */
	    (void) _DtTermPrimRemoveSuidRoot();

            (void) close(p[1]);
            (void) close(0);
            (void) dup(p[0]);
            (void) close(p[0]);
            /*
            ** set close on exec flag on all other fd's
            */            for (i = 3; i < _NFILE; i++) {
                (void) fcntl(i, F_SETFD, 1);
            }
            /*
            ** reset signals
            */
            (void) signal(SIGHUP, SIG_DFL);
            (void) signal(SIGCLD, SIG_DFL);
#ifdef	BBA
	    _bA_dump();
#endif	/* BBA */
            (void) execl(DEFAULT_SHELL, DEFAULT_SHELL_ARGV0, 
                         "-c", &tw->term.logFile[1], 0);
            (void) fprintf(stderr, " Can't exec \"%s\"\n",
                                       &tw->term.logFile[1]);
            (void) exit(1);
        }
        (void) close(p[0]);
        tpd->logStream = fdopen(p[1], "w");
	(void) AddLogFileEntry(tpd->logStream);

        (void) signal(SIGPIPE, _DtTermPrimLogPipe);

    }
    else {
        if (access(tw->term.logFile, F_OK) == 0) {
            if (access(tw->term.logFile, W_OK) < 0) {
		return;
	    }
        } else if (cp = strrchr(tw->term.logFile, '/')) {
            *cp = 0;
            i   = access(tw->term.logFile, W_OK);
            *cp = '/';
            if (i < 0) {
		return;
	    }
        } else if (access(".", W_OK) < 0) {
	    return;
	}
        if ((tpd->logStream = fopen(tw->term.logFile, "a")) == NULL) {
            return;
        }
	(void) AddLogFileEntry(tpd->logStream);
        (void) chown(tw->term.logFile, getuid(), getgid());
    }
    tw->term.log_on = True ;
}

void
_DtTermPrimCloseLog(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    DtTermPrimData tpd = tw->term.tpd;

    /*
    ** if we are not logging, or it is inhibited, do nothing
    */
    if (!tw->term.log_on || tw->term.logInhibit ) { return; }

    (void) fflush(tpd->logStream);
    (void) fclose(tpd->logStream);
    (void) DeleteLogFileEntry(tpd->logStream);
    tw->term.log_on = False ;
}

/*  use this to squirel away widet for _DtTermPrimLogPipe()  */
static DtTermPrimitiveWidget writingWidget = (DtTermPrimitiveWidget)0 ;

void
_DtTermPrimWriteLog(DtTermPrimitiveWidget tw, char *buffer, int cnt)
{
    DtTermPrimData tpd = tw->term.tpd;

    writingWidget = tw;   
    if (cnt > 0) {
        (void) fwrite(buffer, cnt, 1, tpd->logStream);
     }
}

void 
_DtTermPrimLogPipe(int n)
{
    DtTermPrimitiveWidget tw = writingWidget ;
    if (tw && tw->term.log_on) { _DtTermPrimCloseLog((Widget)tw); }
}

static Boolean first = True;
static uid_t uid_user;
static uid_t uid_root;
static gid_t gid_user;
static gid_t gid_root;

static void
suidInit()
{
    if (first) {
	uid_user = getuid();
	uid_root = geteuid();

	gid_user = getgid();
	gid_root = getegid();

	first = False;
    }
}

void
_DtTermPrimRemoveSuidRoot()
{
    (void) suidInit();
#if	defined(HAS_SETRESUID)
    (void) setresgid(gid_user, gid_user, gid_user);
    (void) setresuid(uid_user, uid_user, uid_user);
#elif	defined(HAS_SETREUID)
    (void) setregid(gid_user, gid_user);
    (void) setreuid(uid_user, uid_user);
#else	/* !HAS_SETRESUID && !HAS_SETREUID */
    (void) setgid(gid_user);
    (void) setuid(uid_user);
#endif	/* !HAS_SETRESUID && !HAS_SETREUID */
}

void
_DtTermPrimToggleSuidRoot(Boolean root)
{
    (void) suidInit();

#if	defined(HAS_SETRESUID)
    (void) setresuid(-1, root ? uid_root : uid_user, -1);
    (void) setresgid(-1, root ? gid_root : gid_user, -1);
#elif	defined(HAS_SETEUID)
    (void) seteuid(root ? uid_root : uid_user);
    (void) setegid(root ? gid_root : gid_user);
#endif	/* HAS_SETEUID */
}

void
_DtTermPrimLogFileCleanup
(
    void
)
{
    logInfo		 *logInfoTmp;

    DebugF('s', 10, fprintf(stderr,
	    ">>_DtTermPrimLogFileCleanup() starting\n"));

    /* flush all the log files... */
    for (logInfoTmp = logInfoHead->next; logInfoTmp;
	    logInfoTmp = logInfoTmp->next) {
	DebugF('s', 10, fprintf(stderr,
		">>flushing logfile 0x%lx\n", logInfoTmp->logFile));
	(void) fflush(logInfoTmp->logFile);
    }
    DebugF('s', 10, fprintf(stderr,
	    ">>_DtTermPrimLogFileCleanup() finished\n"));
}
