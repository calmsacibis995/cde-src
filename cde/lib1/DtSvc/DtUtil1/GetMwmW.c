/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     GetMwmW.c
 **
 **   RCS:	$XConsortium: GetMwmW.c /main/cde1_maint/2 1995/10/08 22:18:25 pascale $
 **   Project:  DT Workspace Manager
 **
 **   Description: Gets the mwm window id.
 **
 **   (c) Copyright 1990 by Hewlett-Packard Company
 **
 ****************************************************************************
 ************************************<+>*************************************/
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Xm/MwmUtil.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>



/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern int _DtGetMwmWindow() ;

#else

extern int _DtGetMwmWindow( 
                        Display *display,
                        Window root,
                        Window *pMwmWindow) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static int _GetMwmWindow() ;

#else

static int _GetMwmWindow( 
                        Display *display,
                        Window root,
                        Window *pMwmWindow,
                        Atom property) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


/*************************************<->*************************************
 *
 *  status _GetMwmWindow (display, root, pMwmWindow, property)
 *
 *
 *  Description:
 *  -----------
 *  Get the Motif Window manager window
 *
 *
 *  Inputs:
 *  ------
 *  display		- display 
 *  root		- root window of screen
 *  pMwmWindow		- pointer to a window (to be returned)
 *  property		- the property atom
 *
 *  Outputs:
 *  --------
 *  *pMwmWindow		- mwm window id, if successful
 *  Return		- status from XGetWindowProperty
 *
 *  Comments:
 *  --------
 *  This can fail if mwm is not managing the screen for the root window
 *  passed in.
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
_GetMwmWindow( display, root, pMwmWindow, property )
        Display *display ;
        Window root ;
        Window *pMwmWindow ;
        Atom property ;
#else
_GetMwmWindow(
        Display *display,
        Window root,
        Window *pMwmWindow,
        Atom property )
#endif /* _NO_PROTO */
{
    Atom actualType;
    int actualFormat;
    unsigned long nitems;
    unsigned long leftover;
#ifdef __osf__
/* PropMotifWmInfo is incorrectly declared in Xm/MwmUtil.h */
    struct { unsigned long flags;
             unsigned long wmWindow; } *pWmInfo = NULL;
#else
    PropMotifWmInfo *pWmInfo = NULL;
#endif
    int rcode;
    Window wroot, wparent, *pchildren;
    unsigned int nchildren;

    *pMwmWindow = NULL;
    if ((rcode=XGetWindowProperty(display,root,
			     property,0L, PROP_MWM_INFO_ELEMENTS,
			     False,property,
			     &actualType,&actualFormat,
			     &nitems,&leftover,(unsigned char **)&pWmInfo))==Success)
    {

        if (actualType != property)
	{
	    /* wrong type, force failure */
	    rcode = BadValue;
	}
	else
	{
	    rcode = BadWindow;	/* assume the worst */

	    /*
	     * The mwm window should be a direct child of root
	     */
	    if (XQueryTree (display, root, &wroot, &wparent,
			    &pchildren, &nchildren))
	    {
		int i;

		for (i = 0; (i < nchildren) && (rcode != Success); i++)
		{
		    if (pchildren[i] == pWmInfo->wmWindow)
		    {
			rcode = Success;
		    }
		}
	    }

	    if (rcode == Success);
	    {
		*pMwmWindow = pWmInfo->wmWindow;
	    }

	    if (pchildren)
	    {
		XFree ((char *)pchildren);
	    }

	}

	if (pWmInfo)
	{
	    XFree ((char *)pWmInfo);
	}
    }
	
    return(rcode);

} /* END OF FUNCTION _GetMwmWindow */


/*************************************<->*************************************
 *
 *  status _DtGetMwmWindow (display, root, pMwmWindow)
 *
 *
 *  Description:
 *  -----------
 *  Get the Motif Window manager window
 *
 *
 *  Inputs:
 *  ------
 *  display		- display 
 *  root		- root window of screen
 *  pMwmWindow		- pointer to a window (to be returned)
 *
 *  Outputs:
 *  --------
 *  *pMwmWindow		- mwm window id, if successful
 *  Return		- status from XGetWindowProperty
 *
 *  Comments:
 *  --------
 *  This can fail if mwm is not managing the screen for the root window
 *  passed in.
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
_DtGetMwmWindow( display, root, pMwmWindow )
        Display *display ;
        Window root ;
        Window *pMwmWindow ;
#else
_DtGetMwmWindow(
        Display *display,
        Window root,
        Window *pMwmWindow )
#endif /* _NO_PROTO */
{
    Atom xa_MWM_INFO;

    xa_MWM_INFO = XmInternAtom (display, _XA_MWM_INFO, False);
    return (_GetMwmWindow (display, root, pMwmWindow, xa_MWM_INFO));
}
