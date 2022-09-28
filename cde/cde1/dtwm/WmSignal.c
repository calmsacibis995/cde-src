/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmSignal.c /main/cde1_maint/5 1995/11/07 15:55:39 lehors $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h" /* This should be the first include */
#include <signal.h>


/*
 * include extern functions
 */

#include "WmFeedback.h"
#include "WmFunction.h"


/*
 * Function Declarations:
 */

#include "WmSignal.h" 


/*
 * Global Variables:
 */


#ifdef WSM

/*************************************<->*************************************
 *
 *  AbortWmSignalHandler ()
 *
 *
 *  Description:
 *  -----------
 *  This function is called on receipt of a fatal signal. We reset
 *  the keyboard focus to pointer root before aborting.
 *
 *************************************<->***********************************/
static void
#ifdef _NO_PROTO
AbortWmSignalHandler (sig)
    int sig;
#else /* _NO_PROTO */
AbortWmSignalHandler (int sig)
#endif /* _NO_PROTO */
{
    struct sigaction sa;

    /*
     * Set input focus back to pointer root
     */
    XSetInputFocus(DISPLAY, PointerRoot, RevertToPointerRoot, CurrentTime);
    XSync (DISPLAY, False);

    /*
     * Invoke the default handler
     */
    (void) sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_DFL;
    (void) sigaction (sig, &sa, (struct sigaction *) 0);

    kill (getpid(), sig);

} /* END OF FUNCTION AbortSignalHandler */


/*************************************<->*************************************
 *
 *  RestoreDefaultSignalHandlers ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the signal handlers for the window manager.
 *
 *************************************<->***********************************/

void
#ifdef _NO_PROTO
RestoreDefaultSignalHandlers ()
#else /* _NO_PROTO */
RestoreDefaultSignalHandlers (void)
#endif /* _NO_PROTO */
{
    struct sigaction sa;
    struct sigaction osa;

    /*
     * Restore default action for signals we're interested in.
     */

    (void) sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_DFL;

    if ((sigaction (SIGINT, (struct sigaction *) 0, &osa) != 0) ||
	(osa.sa_handler != SIG_IGN))
    {
	(void) sigaction (SIGINT, &sa, (struct sigaction *) 0);
    }

    if ((sigaction (SIGHUP, (struct sigaction *) 0, &osa) != 0) ||
	(osa.sa_handler != SIG_IGN))
    {
	(void) sigaction (SIGHUP, &sa, (struct sigaction *) 0);
    }

    (void) sigaction (SIGQUIT, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGTERM, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGPIPE, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGCHLD, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGILL, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGFPE, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGBUS, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGSEGV, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGSYS, &sa, (struct sigaction *) 0);
}
#endif /* WSM */


/*************************************<->*************************************
 *
 *  SetupWmSignalHandlers ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the signal handlers for the window manager.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void SetupWmSignalHandlers (dummy)
    int dummy;
#else /* _NO_PROTO */
void SetupWmSignalHandlers (int dummy)
#endif /* _NO_PROTO */
{
    void (*signalHandler) ();

#ifdef WSM
    struct sigaction 	sa;
    struct sigaction 	osa;

    /*
     * Catch software signals that we ask the user about
     * before quitting.
     */
    (void) sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = QuitWmSignalHandler;

    if ((sigaction (SIGINT, (struct sigaction *) 0, &osa) != 0) ||
	(osa.sa_handler != SIG_IGN))
    {
	(void) sigaction (SIGINT, &sa, (struct sigaction *) 0);
    }

    if ((sigaction (SIGHUP, (struct sigaction *) 0, &osa) != 0) ||
	(osa.sa_handler != SIG_IGN))
    {
	(void) sigaction (SIGHUP, &sa, (struct sigaction *) 0);
    }

    (void) sigaction (SIGQUIT, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGTERM, &sa, (struct sigaction *) 0);

    /*
     * Ignore child death
     */

#if defined(hpux) || defined(_AIX)
    sa.sa_flags = 0;
#else
    sa.sa_flags = SA_NOCLDWAIT;
#endif
    sa.sa_handler = SIG_IGN;
    (void) sigaction (SIGCHLD, &sa, (struct sigaction *) 0);
    sa.sa_flags = 0;

    /* 
     * Catch other fatal signals so we can reset the 
     * keyboard focus to pointer root before aborting
     */
    sa.sa_handler = AbortWmSignalHandler;

    (void) sigaction (SIGILL, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGFPE, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGBUS, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGSEGV, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGSYS, &sa, (struct sigaction *) 0);

#else /* not WSM - original mwm code*/

    signalHandler = (void (*)())signal (SIGINT, SIG_IGN);
    if (signalHandler != (void (*)())SIG_IGN)
    {
	signal (SIGINT, QuitWmSignalHandler);
    }

    signalHandler = (void (*)())signal (SIGHUP, SIG_IGN);
    if (signalHandler != (void (*)())SIG_IGN)
    {
	signal (SIGHUP, QuitWmSignalHandler);
    }

    signal (SIGQUIT, QuitWmSignalHandler);

    signal (SIGTERM, QuitWmSignalHandler);
#endif /* WSM */


} /* END OF FUNCTION SetupWmSignalHandlers */


/*************************************<->*************************************
 *
 *  QuitWmSignalHandler ()
 *
 *
 *  Description:
 *  -----------
 *  This function is called on receipt of a signal that is to terminate the
 *  window manager.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void QuitWmSignalHandler (dummy)
    int dummy;
#else /* _NO_PROTO */
void QuitWmSignalHandler (int dummy)
#endif /* _NO_PROTO */
{
    if (wmGD.showFeedback & WM_SHOW_FB_KILL)
    {
	ConfirmAction (ACTIVE_PSD, QUIT_MWM_ACTION);
	XFlush(DISPLAY);
    }
    else
    {
	Do_Quit_Mwm(False);
    }

} /* END OF FUNCTION QuitWmSignalHandler */
