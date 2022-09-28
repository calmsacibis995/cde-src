/*
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     GetVwmHint.c
 **
 **   RCS:	$XConsortium: GetVWmHint.c /main/cde1_maint/1 1995/07/14 20:25:13 drk $
 **
 **   Project:  DT Workspace Manager
 **
 **   Description: Get Dt Window manager hints
 **
 **   (c) Copyright 1991, 1993, 1994 by Hewlett-Packard Company
 **
 ****************************************************************************
 ************************************<+>*************************************/
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Dt/Wsm.h> 
#include <Dt/WsmP.h> 
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>

/*************************************<->*************************************
 *
 *  status _DtWsmGetDtWmHints (display, window, ppDtWmHints)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WM_HINTS property on a window
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  window	- window to get hints from
 *  ppDtWmHints- pointer to a pointer to return
 *
 *  Outputs:
 *  --------
 *  *ppDtWmHints-points to the DtWmHints structure retrieved from
 *		  the window (NOTE: This should be freed using XFree)
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
Status
#ifdef _NO_PROTO
_DtWsmGetDtWmHints( display, window, ppDtWmHints)
        Display *display ;
        Window window ;
        DtWmHints **ppDtWmHints ;
#else
_DtWsmGetDtWmHints(
        Display *display,
        Window window,
        DtWmHints **ppDtWmHints)
#endif /* _NO_PROTO */
{
    Atom actualType;
    int actualFormat;
    unsigned long leftover, items, length;
    int rcode;
    Atom property;
#ifdef HP_VUE
    unsigned long oldlength;
#endif /* HP_VUE */


    property = XmInternAtom(display, _XA_DT_WM_HINTS, False);
    length = sizeof (DtWmHints) / sizeof (long);

    *ppDtWmHints = NULL;
    if ((rcode=XGetWindowProperty(
			display,
			window,
			property,
			0L, 				/* offset */
			length,
			False,				/* delete */
			property,			/* req_type */
			&actualType,
			&actualFormat,
			&items,				/* items returned */
			&leftover,
			(unsigned char **)ppDtWmHints))==Success)
    {

        if ((actualType != property) || (items < length))
	{
	    /* wrong type, force failure */
	    rcode = BadValue;
	    if (actualType != None)
	    {
		XFree ((char *)*ppDtWmHints);
	    }
	    *ppDtWmHints = NULL;
	}
    }
#ifdef HP_VUE
    else {
	/*
	 * Didn't get the Dt hints, try to get the Vue hints.
	 * Not that the Vue hints had the same structure, just
	 * under a different property name.
	 */
	property = XmInternAtom(display, _XA_VUE_WM_HINTS, False);
	/* 
	 * Property previously existed without attachWindow.
	 */
	oldlength = length - (sizeof(Window)/sizeof(long));

	*ppDtWmHints = NULL;
	if ((rcode=XGetWindowProperty(
			display,
			window,
			property,
			0L, 				/* offset */
			length,
			False,				/* delete */
			property,			/* req_type */
			&actualType,
			&actualFormat,
			&items,				/* items returned */
			&leftover,
			(unsigned char **)ppDtWmHints))==Success)
	{

	    if ((actualType != property) ||
		(items < oldlength))
	    {
		/* wrong type, force failure */
		rcode = BadValue;
		if (actualType != None)
		{
		    XFree ((char *)*ppDtWmHints);
		}
		*ppDtWmHints = NULL;
	    }

	    if (*ppDtWmHints && (items < length))
	    {
		DtWmHints *pvh;

		/* assume old property, return full sized
		   property with empty values */
		pvh = (DtWmHints *) malloc (length * sizeof (long));

		pvh->flags = (*ppDtWmHints)->flags;
		pvh->functions = (*ppDtWmHints)->functions;
		pvh->behaviors = (*ppDtWmHints)->behaviors;
		pvh->attachWindow = NULL;

		XFree ((char *) *ppDtWmHints);
		
		*ppDtWmHints = pvh;
	    }
	}
    }
#endif /* HP_VUE */
	
    return(rcode);

} /* END OF FUNCTION _DtWsmGetDtWmHints */

