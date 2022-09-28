/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmRestart.c
 **
 **   RCS:	$XConsortium: WmRestart.c /main/cde1_maint/1 1995/07/14 20:28:16 drk $
 **   Project:  HP DT Workspace Manager
 **
 **   Description: Request restart of the HP DT workspace manager
 **
 **   (c) Copyright 1991, 1993 by Hewlett-Packard Company
 **
 ****************************************************************************
 ************************************<+>*************************************/
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <Dt/Wsm.h> 
#include <Dt/WsmP.h> 
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>



/*************************************<->*************************************
 *
 *  _DtWmRestart (display, root)
 *
 *
 *  Description:
 *  -----------
 *  Request the window manager to restart
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window of screen 
 *
 *  Returns:
 *  --------
 *  Success if request sent 
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
Status 
_DtWmRestart (display, root)
        Display *display;
	Window root;
#else
Status 
_DtWmRestart (Display *display, Window root)
#endif /* _NO_PROTO */
{
    Status rval = BadAtom;
    Window wmWindow;

    /*
     * Get the workspace manager window
     */
    if ((rval=_DtGetMwmWindow (display, root, &wmWindow)) == Success)
    {
      
	/*
	 * Make the request by appending the restart request
	 * name to the _DT_WM_REQUEST property
	 */

	rval = XChangeProperty (display, wmWindow, 
	    XmInternAtom(display, _XA_DT_WM_REQUEST, False),
	    XA_STRING, 8, PropModeAppend, 
	    (unsigned char *)DTWM_REQ_RESTART, 
	    1+strlen(DTWM_REQ_RESTART));

	XFlush (display);	/* do it now */

    }

    return (rval);

} /* END OF FUNCTION _DtWmRestart */


/*************************************<->*************************************
 *
 *  _DtWmRestartNoConfirm (display, root)
 *
 *
 *  Description:
 *  -----------
 *  Request the window manager to restart with no confirmation dialog
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window of screen 
 *
 *  Returns:
 *  --------
 *  Success if request sent 
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
Status 
_DtWmRestartNoConfirm (display, root)
        Display *display;
	Window root;
#else
Status 
_DtWmRestartNoConfirm (Display *display, Window root)
#endif /* _NO_PROTO */
{
    Status rval;
    Window wmWindow;
    char buffer[80];

    /*
     * Make the request by appending the restart request
     * name to the _DT_WM_REQUEST property
     */

    strcpy (&buffer[0], DTWM_REQ_RESTART);
    strcat (&buffer[0], " ");
    strcat (&buffer[0], DTWM_REQP_NO_CONFIRM);

    /*
     * Get the workspace manager window
     */
    if ((rval=_DtGetMwmWindow (display, root, &wmWindow)) == Success)
    {
	/*
	 * Make the request by appending the restart request
	 * name to the _DT_WM_REQUEST property
	 */

	rval = XChangeProperty (display, wmWindow, 
	    XmInternAtom(display, _XA_DT_WM_REQUEST, False),
	    XA_STRING, 8, PropModeAppend, 
	    (unsigned char *)buffer, 
	    1+strlen(buffer));

	XFlush (display); 	/* do it now */
    }

    return (rval);

} /* END OF FUNCTION _DtWmRestartNoConfirm */

