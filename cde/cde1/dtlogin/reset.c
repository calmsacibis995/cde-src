/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: reset.c /main/cde1_maint/1 1995/10/09 00:11:48 pascale $
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
 * pseudoReset -- pretend to reset the server by killing all clients
 * with windows.  It will reset the server most of the time, unless
 * a client remains connected with no windows.
 */

# include	<setjmp.h>
# include	<sys/signal.h>
# include	"dm.h"
# include	"vgmsg.h"


/***************************************************************************
 *
 *  Local procedure declarations
 *
 ***************************************************************************/
#ifdef _NO_PROTO

static SIGVAL abortReset() ;
static int ignoreErrors() ;
static void killWindows() ;

#else

static SIGVAL abortReset( int arg ) ;
static int ignoreErrors( Display *dpy, XErrorEvent *event) ;
static void killWindows( Display *dpy, Window window) ;

#endif /* _NO_PROTO */


/***************************************************************************
 *
 *  
 *
 ***************************************************************************/

/*ARGSUSED*/
static int 
#ifdef _NO_PROTO
ignoreErrors( dpy, event )
        Display *dpy ;
        XErrorEvent *event ;
#else
ignoreErrors( Display *dpy, XErrorEvent *event )
#endif /* _NO_PROTO */
{
	Debug ("Ignoring error...\n");
	return 1;
}

/*
 * this is mostly bogus -- but quite useful.  I wish the protocol
 * had some way of enumerating and identifying clients, that way
 * this code wouldn't have to be this kludgy.
 */

static void 
#ifdef _NO_PROTO
killWindows( dpy, window )
        Display *dpy ;
        Window window ;
#else
killWindows( Display *dpy, Window window )
#endif /* _NO_PROTO */
{
	Window	root, parent, *children;
	int	child;
	unsigned int nchildren = 0;
	
	while (XQueryTree (dpy, window, &root, &parent, &children, &nchildren)
	       && nchildren > 0)
	{
		for (child = 0; child < nchildren; child++) {
			Debug ("Calling XKillClient() for window 0x%x\n",
				children[child]);
			XKillClient (dpy, children[child]);
		}
		XFree ((char *)children);
	}
}

static jmp_buf	resetJmp;

static SIGVAL
#ifdef _NO_PROTO
abortReset()
#else
abortReset( int arg )
#endif /* _NO_PROTO */
{
	longjmp (resetJmp, 1);
}

/*
 * this display connection better not have any windows...
 */
 
void 
#ifdef _NO_PROTO
pseudoReset( dpy )
        Display *dpy ;
#else
pseudoReset( Display *dpy )
#endif /* _NO_PROTO */
{
	Window	root;
	int	screen;

	if (setjmp (resetJmp)) {
		LogError(
		  ReadCatalog(MC_LOG_SET,MC_LOG_PSEUDO,MC_DEF_LOG_PSEUDO));
	} else {
		signal (SIGALRM, abortReset);
		alarm (30);
		XSetErrorHandler (ignoreErrors);
		for (screen = 0; screen < ScreenCount (dpy); screen++) {
			Debug ("Pseudo reset screen %d\n", screen);
			root = RootWindow (dpy, screen);
			killWindows (dpy, root);
		}
		Debug ("Before XSync\n");
		XSync (dpy, False);
		(void) alarm (0);
	}
	signal (SIGALRM, SIG_DFL);
	XSetErrorHandler ((int (*)()) 0);
	Debug ("pseudoReset() done\n");
}
