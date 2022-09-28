/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmChBackD.c
 **
 **   RCS:	$XConsortium: WmChBackD.c /main/cde1_maint/1 1995/07/14 20:27:07 drk $
 **
 **   Description: Request backdrop change of the workspace manager
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


#define MAX_ENV_STRING  2047
#define SYS_FILE_SEARCH_PATH "DTICONSEARCHPATH"
    

/*************************************<->*************************************
 *
 *  _DtWsmChangeBackdrop (display, root, path, pixmap)
 *
 *
 *  Description:
 *  -----------
 *  Request the CDE workspace manager to change the backdrop
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window of screen
 *  path	- file path to bitmap file
 *  pixmap	- pixmap id of backdrop pixmap
 *
 *  Returns:
 *  --------
 *  Success if request sent 
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
Status 
_DtWsmChangeBackdrop (display, root, path, pixmap)
        Display *display;
	Window root;
	char *path;
	Pixmap pixmap;
#else
Status 
_DtWsmChangeBackdrop (
	Display *display, 
	Window root, 
	char *path, 
	Pixmap pixmap)
#endif /* _NO_PROTO */
{
    Status rval = BadAtom;
    Window wmWindow;

    if (!path)
    {
	rval = BadValue;
    }
    else
    /*
     * Get the workspace manager window
     */
    if ((rval=_DtGetMwmWindow (display, root, &wmWindow)) == Success)
    {
	char *pch;
	int len;
      
	/*
	 * Build up the NULL-terminated request string
	 */
	len = strlen (DTWM_REQ_CHANGE_BACKDROP) + 1;
	len += strlen (path) + 1;
	len += 30;  /* fudge for ascii-ized pixmap id */

	pch = (char *) XtMalloc (len * sizeof(char));

	sprintf (pch, "%s %s %lx", DTWM_REQ_CHANGE_BACKDROP, path,
		    pixmap);

	/*
	 * Make the request by appending the string to 
	 * the _DT_WM_REQUEST property
	 */
	rval = XChangeProperty (display, wmWindow, 
	    XmInternAtom(display, _XA_DT_WM_REQUEST, False),
	    XA_STRING, 8, PropModeAppend, 
	    (unsigned char *)pch, 
	    1+strlen(pch));

    }

    return (rval);

} /* END OF FUNCTION _DtWsmChangeBackdrop */


