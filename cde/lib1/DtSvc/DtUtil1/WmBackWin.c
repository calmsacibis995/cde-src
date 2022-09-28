/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmBackWin.c
 **
 **   RCS:	$XConsortium: WmBackWin.c /main/cde1_maint/1 1995/07/14 20:26:54 drk $
 **   Project:  DT Workspace Manager
 **
 **   Description: Identify backdrop windows
 **
 **   (c) Copyright 1993 by Hewlett-Packard Company
 **
 ****************************************************************************
 ************************************<+>*************************************/
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Dt/Wsm.h> 
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Window DtWsmGetCurrentBackdropWindow() ;
extern Boolean _DtWsmIsBackdropWindow();

#else

extern Window DtWsmGetCurrentBackdropWindow( 
                        Display *display,
                        Window root);
extern Boolean _DtWsmIsBackdropWindow(
        Display *display,
        int screen_num,
        Window window );

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/



/*************************************<->*************************************
 *
 *  Window DtWsmGetCurrentBackdropWindow (display, root)
 *
 *
 *  Description:
 *  -----------
 *  Returns the window used for the backdrop for the current workspace
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window
 *
 *  Return	- window used for current workspace
 *                None if no backdrop window or error
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/
Window
#ifdef _NO_PROTO
DtWsmGetCurrentBackdropWindow(display, root)
        Display *display ;
        Window root ;
#else
DtWsmGetCurrentBackdropWindow(
        Display *display ,
        Window root )
#endif /* _NO_PROTO */
{
    Window wReturn = None;
    Atom aWS;
    DtWsmWorkspaceInfo *pWsInfo;
    Status status;

    status = DtWsmGetCurrentWorkspace (display, root, &aWS);

    if (status == Success)
    {
        status = DtWsmGetWorkspaceInfo(display, root, aWS, &pWsInfo);
	if (status == Success)
	{
	    if (pWsInfo->numBackdropWindows > 0)
	    {
		/* copy backdrop window (there should be at most one) */
		wReturn = pWsInfo->backdropWindows[0];
	    }

	    DtWsmFreeWorkspaceInfo (pWsInfo);
	}
    }

    return (wReturn);

}  /* END OF FUNCTION DtWsmGetCurrentBackdropWindow */


/*************************************<->*************************************
 *
 *  Boolean _DtWsmIsBackdropWindow (display, screen_num, window)
 *
 *
 *  Description:
 *  -----------
 *  Returns true if the window passed in is a backdrop window.
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  screen_num	- number of screen we're interested in
 *  window	- window we want to test
 *
 *  Outputs:
 *  -------
 *  Return	- True if window is a backdrop window
 *                False otherwise.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
Boolean 
#ifdef _NO_PROTO
_DtWsmIsBackdropWindow( display, screen_num, window )
        Display *display ;
        int screen_num ;
        Window window ;
#else
_DtWsmIsBackdropWindow(
        Display *display,
        int screen_num,
        Window window )
#endif /* _NO_PROTO */
{
    Boolean rval = False;
    Status status;
    Atom *pWorkspaceList;
    int ix, iw, numWorkspaces;
    DtWsmWorkspaceInfo *pWsInfo;
    Window root;

    root = XRootWindow (display, screen_num);

    status = DtWsmGetWorkspaceList (display, root, 
			    &pWorkspaceList, &numWorkspaces);

    if ((status == Success) && 
	(numWorkspaces > 0) &&
	(pWsInfo = (DtWsmWorkspaceInfo *) 
	    malloc (numWorkspaces * sizeof(DtWsmWorkspaceInfo))))
    {
	for (ix=0;
		(!rval) && (ix < numWorkspaces) && (status == Success); 
		    ix++)
	{
	    status = DtWsmGetWorkspaceInfo (display,
					 root,
					 pWorkspaceList[ix],
					 &pWsInfo);
	    if (status == Success)
	    {
		for (iw = 0; iw < pWsInfo->numBackdropWindows; iw++)
		{
		    if (pWsInfo->backdropWindows[iw] == window)
		    {
			rval = True;
		    }
		}

		DtWsmFreeWorkspaceInfo(pWsInfo);
	    }
	}
	XFree ((char *)pWorkspaceList);
    }

    return (rval);
}  /* END OF FUNCTION _DtWsmIsBackdropWindow */
