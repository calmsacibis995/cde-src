/*
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     SetVWmHint.c
 **
 **   RCS:	$XConsortium: SetVWmHint.c /main/cde1_maint/2 1995/09/06 02:14:40 lehors $
 **
 **   Project:  DT Workspace Manager
 **
 **   Description: Set Dt Wm hints.
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
 *  _DtWsmSetDtWmHints (display, window, pHints)
 *
 *
 *  Description:
 *  -----------
 *  Set the contents of the _DT_WM_HINTS property on a window
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  window	- window to set hints on
 *  pHints	- pointer the hints to set
 *
 *  Comments:
 *  ---------
 *  No error checking
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
_DtWsmSetDtWmHints( display, window, pHints)
        Display *display ;
        Window window ;
        DtWmHints *pHints ;
#else
_DtWsmSetDtWmHints(
        Display *display,
        Window window,
        DtWmHints *pHints)
#endif /* _NO_PROTO */
{
    Atom property;
    DtWmHints vh;

    property = XmInternAtom (display, _XA_DT_WM_HINTS, False);

    /*
     * Copy hints to make sure we have one of the right size.
     * This is for backward compatibility.
     */
    vh.flags = pHints->flags;

    if (pHints->flags & DtWM_HINTS_FUNCTIONS) {
	vh.functions = pHints->functions;
    } else {
	vh.functions = 0L;
    }
  
    if (pHints->flags & DtWM_HINTS_BEHAVIORS) {
	vh.behaviors = pHints->behaviors;
    } else {
	vh.behaviors = 0L;
    }

    if (pHints->flags & DtWM_HINTS_ATTACH_WINDOW) {
	vh.attachWindow = pHints->attachWindow;
    } else {
	vh.attachWindow = None;
    }

    XChangeProperty (
		display, 
		window, 
		property,
		property, 
		32, 
		PropModeReplace, 
		(unsigned char *)&vh, 
		(sizeof (DtWmHints)/sizeof (long)));
}
