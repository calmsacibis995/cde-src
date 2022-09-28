/*
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmRmWsFcn.c
 **
 **   RCS:	$XConsortium: WmRmWsFcn.c /main/cde1_maint/2 1995/09/06 02:15:24 lehors $
 **   Project:  HP DT Workspace Manager
 **
 **   Description: Remove the Workspace functions on a client
 **
 **   (c) Copyright 1991, 1993, 1994 by Hewlett-Packard Company
 **
 ****************************************************************************
 ************************************<+>*************************************/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <Dt/Wsm.h> 
#include <Dt/WsmP.h> 
#include <Xm/AtomMgr.h>



/*************************************<->*************************************
 *
 *  DtWsmRemoveWorkspaceFunctions (display, client)
 *
 *
 *  Description:
 *  -----------
 *  Request the window manager to disallow workspace functions
 *  for this client.
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window of screen 
 *
 *  Returns:
 *  --------
 *  none
 *
 *  
 *  Comments:
 *  ---------
 *  Disables f.workspace_presence, f.remove, f.occupy_all for this
 *  client.
 * 
 *************************************<->***********************************/
void
#ifdef _NO_PROTO
DtWsmRemoveWorkspaceFunctions (display, client)
        Display *display;
	Window client;
#else
DtWsmRemoveWorkspaceFunctions (Display *display, Window client)
#endif /* _NO_PROTO */
{
    DtWmHints vHints, *pHints;
    long functions;
    Boolean bSetHints = False;

    if (_DtWsmGetDtWmHints (display, client, &pHints) != Success)
    {
	/*
	 * There were no existing workspace hints, so we'll
	 * just use our own variable
	 */
	pHints = &vHints;
	pHints->flags = 0;
    }

    if (pHints->flags & DtWM_HINTS_FUNCTIONS)
    {
	functions = pHints->functions & 
		(DtWM_FUNCTION_OCCUPY_WS | DtWM_FUNCTION_ALL);
	
	if (functions & DtWM_FUNCTION_ALL)
	{
	    /* 
	     * The flags are a list of functions to remove. If 
	     * the workspace functions aren't on this list, make 
	     * sure that it's put there.
	     */
	    if (!(functions & DtWM_FUNCTION_OCCUPY_WS))
	    {
		/* remove workspace functions */
		pHints->functions |=  DtWM_FUNCTION_OCCUPY_WS;
		bSetHints = True;
	    }
	}
	else
	{
	    /*
	     * The flags are a list of functions to add. If
	     * the workspace functions are on the list, make
	     * sure they get removed.
	     */
	    if (functions & DtWM_FUNCTION_OCCUPY_WS)
	    {
		/* remove workspace functions */
		pHints->functions &=  ~DtWM_FUNCTION_OCCUPY_WS;
		bSetHints = True;
	    }
	}
    }
    else
    {
	/*
	 * The hints didn't have workspace functions specified.
	 * Set the flag and remove workspace functions.
	 */
	pHints->flags |= DtWM_HINTS_FUNCTIONS;
	pHints->functions = DtWM_FUNCTION_OCCUPY_WS | DtWM_FUNCTION_ALL;
	bSetHints = True;
    }

    /*
     * If something needs to be changed, then change it.
     */
    if (bSetHints)
	_DtWsmSetDtWmHints (display, client, pHints);

    /*
     * If we read these hints off the window, then be sure to free
     * them.
     */
    if (pHints && (pHints != &vHints))
    {
	XFree ((char *)pHints);
    }

} /* END OF FUNCTION DtWsmRemoveWorkspaceFunctions */



/*************************************<->*************************************
 *
 *  DtWsmAddWorkspaceFunctions (display, client)
 *
 *
 *  Description:
 *  -----------
 *  Request the window manager to allow workspace functions
 *  for this client.
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window of screen 
 *
 *  Returns:
 *  --------
 *  none
 *
 *  
 *  Comments:
 *  ---------
 *  Enables f.workspace_presence, f.remove, f.occupy_all for this
 *  client.
 * 
 *************************************<->***********************************/
void
#ifdef _NO_PROTO
DtWsmAddWorkspaceFunctions (display, client)
        Display *display;
	Window client;
#else
DtWsmAddWorkspaceFunctions (Display *display, Window client)
#endif /* _NO_PROTO */
{
    DtWmHints vHints, *pHints;
    long functions;
    Boolean bSetHints = False;

    if (_DtWsmGetDtWmHints (display, client, &pHints) != Success)
    {
	/*
	 * There were no existing workspace hints, so we'll
	 * just use our own variable
	 */
	pHints = &vHints;
	pHints->flags = 0;
    }

    if (pHints->flags & DtWM_HINTS_FUNCTIONS)
    {
	functions = pHints->functions & 
		(DtWM_FUNCTION_OCCUPY_WS | DtWM_FUNCTION_ALL);
	
	if (functions & DtWM_FUNCTION_ALL)
	{
	    /* 
	     * The flags are a list of functions to remove. If 
	     * the workspace functions are on this list, make 
	     * sure that they're removed.
	     */
	    if (functions & DtWM_FUNCTION_OCCUPY_WS)
	    {
		/* add workspace functions */
		pHints->functions &=  ~DtWM_FUNCTION_OCCUPY_WS;
		bSetHints = True;
	    }
	}
	else
	{
	    /*
	     * The flags are a list of functions to add. If
	     * the workspace functions aren't on the list, make
	     * sure they get added.
	     */
	    if (!(functions & DtWM_FUNCTION_OCCUPY_WS))
	    {
		/* remove workspace functions */
		pHints->functions |=  DtWM_FUNCTION_OCCUPY_WS;
		bSetHints = True;
	    }
	}
    }
    else
    {
	/*
	 * The hints didn't have workspace functions specified.
	 * Set the flag and add workspace functions.
	 */
	pHints->flags |= DtWM_HINTS_FUNCTIONS;
	pHints->functions = DtWM_FUNCTION_OCCUPY_WS;
	bSetHints = True;
    }

    /*
     * If something needs to be changed, then change it.
     */
    if (bSetHints)
	_DtWsmSetDtWmHints (display, client, pHints);

    /*
     * If we read these hints off the window, then be sure to free
     * them.
     */
    if (pHints && (pHints != &vHints))
    {
	XFree ((char *)pHints);
    }


} /* END OF FUNCTION DtWsmAddWorkspaceFunctions */

