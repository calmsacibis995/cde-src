/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmWsHints.c
 **
 **   RCS:	$XConsortium: WmWsHints.c /main/cde1_maint/1 1995/07/14 20:29:24 drk $
 **   Project:  DT Workspace Manager
 **
 **   Description: Set and Get workspace hints.
 **
 **   (c) Copyright 1993 by Hewlett-Packard Company
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
 *  _DtWsmSetWorkspaceHints (display, window, pWsHints)
 *
 *
 *  Description:
 *  -----------
 *  Set the contents of the _DT_WORKSPACE_HINTS property on a window
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  window	- window to get hints from
 *  pWsHints	- pointer to workspace hints
 *
 *  Comments:
 *  ---------
 *  The internal form of the property is a structure. The structure
 *  must be unwound and turned into a simple array of "long"s before
 *  being written out.
 *
 *  This function currently only deals with version 1 of the property
 *  structure. The passed in pWsHints->version is ignored.
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
_DtWsmSetWorkspaceHints( display, window, pWsHints)
        Display *display ;
        Window window ;
        DtWorkspaceHints *pWsHints ;
#else
_DtWsmSetWorkspaceHints(
        Display *display,
        Window window,
        DtWorkspaceHints *pWsHints)
#endif /* _NO_PROTO */
{
    unsigned int iSizeWsHints;
    Atom property;
    long *pPropValue, *pP;
    int i;

    /*
     * Unwind structure into a simple array of longs.
     *
     * Compute size of property to write (don't count
     * pointer to workspaces, it will be unwound).
     */
    iSizeWsHints = sizeof(DtWorkspaceHints) - sizeof(long);
    if (pWsHints->flags & DT_WORKSPACE_HINTS_WORKSPACES)
    {
	iSizeWsHints += pWsHints->numWorkspaces * sizeof(long);
    }
    pP = pPropValue = (long *) XtMalloc (iSizeWsHints);

    *pP++ = 1;		/* only deals with version 1 !! */
    *pP++ = pWsHints->flags;

    if (pWsHints->flags & DT_WORKSPACE_HINTS_WSFLAGS)
	*pP++ = pWsHints->wsflags;
    else
	*pP++ = 0L;

    if (pWsHints->flags & DT_WORKSPACE_HINTS_WORKSPACES)
    {
	*pP++ = pWsHints->numWorkspaces;
	for (i=0; i<pWsHints->numWorkspaces; i++)
	{
	    *pP++ = pWsHints->pWorkspaces[i];
	}
    }
    else
    {
	*pP++ = 0L;	/* numWorkspaces */
    }

    property  = XmInternAtom (display, _XA_DT_WORKSPACE_HINTS, False);
    XChangeProperty (display, window, property, 
	property, 32, PropModeReplace, 
	(unsigned char *)pPropValue, (iSizeWsHints/sizeof(long)));

    XtFree((char*)pPropValue);
}


/*************************************<->*************************************
 *
 *  _DtWsmGetWorkspaceHints (display, window, ppWsHints)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WORKSPACE_HINTS property from a window
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  window	- window to get hints from
 *  ppWsHints	- pointer to pointer to workspace hints
 *
 *  Outputs:
 *  *ppWsHints	- allocated workspace hints data.
 *
 *  Comments:
 *  ---------
 *  The internal form of the property is a structure. The property
 *  is read in and packed into data allocated for the structure.
 *  Free the workspace hints by calling _DtWsmFreeWorkspaceHints.
 *
 *  This function currently only deals with version 1 of the property
 *  structure. 
 * 
 *************************************<->***********************************/
Status 
#ifdef _NO_PROTO
_DtWsmGetWorkspaceHints( display, window, ppWsHints)
        Display *display ;
        Window window ;
        DtWorkspaceHints **ppWsHints ;
#else
_DtWsmGetWorkspaceHints(
        Display *display,
        Window window,
        DtWorkspaceHints **ppWsHints)
#endif /* _NO_PROTO */
{
    unsigned int iSizeWsHints;
    Atom property;
    DtWorkspaceHints *pWsH;
    long *pP = NULL;
    long *pProp = NULL;
    int i;
    Atom actualType;
    int actualFormat;
    unsigned long leftover, lcount;
    int rcode;

    property  = XmInternAtom (display, _XA_DT_WORKSPACE_HINTS, False);
    if ((rcode = XGetWindowProperty(
			    display,
			    window,
                            property,
			    0L, 
			    (long)BUFSIZ,
                            False,
			    property,
                            &actualType,
			    &actualFormat,
                            &lcount,
			    &leftover,
                            (unsigned char **)&pProp))==Success)
    {
	if (actualType != property)
        {
            /* wrong type, force failure */
            rcode = BadValue;
        }
	else
	{
	    pP = pProp;
	    pWsH = (DtWorkspaceHints *) 
		XtMalloc (sizeof(DtWorkspaceHints) * sizeof(long));
	    
	    pWsH->version = *pP++;

	    /* 
	     * Only handles version 1 
	     *
	     * (Fudge the test a little so that newer versions
	     *  won't be treated as older versions. This assumes
	     *  that customers will migrate their software after
	     *  a couple of revisions of these hints.)
	     */
	    if (pWsH->version > 5)
	    {
		/* 
		 * Assume old version of hints which was a simple
		 * list of atoms.
		 */
		pWsH->version = 1; 
		pWsH->flags = DT_WORKSPACE_HINTS_WORKSPACES;
		pWsH->wsflags = 0;
		pWsH->numWorkspaces = lcount;

		/* reset pointer to first atom in list */
		pP--;
	    }
	    else 
	    {
		pWsH->flags = *pP++;
		pWsH->wsflags = *pP++;
		pWsH->numWorkspaces = *pP++;
	    }

	    if (pWsH->flags & DT_WORKSPACE_HINTS_WORKSPACES)
	    {
		pWsH->pWorkspaces = (Atom *) 
		    XtMalloc (pWsH->numWorkspaces * sizeof (Atom));
		for (i=0; i<pWsH->numWorkspaces; i++)
		{
		    pWsH->pWorkspaces[i] = *pP++;
		}
	    }
	    else
	    {
		pWsH->pWorkspaces = NULL;
	    }
	    *ppWsHints = pWsH;
	}

	if (pProp && (actualType != None))
	{
	    XFree ((char *)pProp);
	}
    }

    return (rcode);
}


/*************************************<->*************************************
 *
 *  _DtWsmFreeWorkspaceHints (pWsHints)
 *
 *
 *  Description:
 *  -----------
 *  Free  a workspace hints structure returned from _DtWsmGetWorkspaceHints
 *
 *  Inputs:
 *  ------
 *  pWsHints	- pointer to workspace hints
 *
 *  Outputs:
 *
 *  Comments:
 *  ---------
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
_DtWsmFreeWorkspaceHints( pWsHints)
        DtWorkspaceHints *pWsHints ;
#else
_DtWsmFreeWorkspaceHints(
        DtWorkspaceHints *pWsHints)
#endif /* _NO_PROTO */
{
    if (pWsHints)
    {
	if (pWsHints->pWorkspaces)
	{
	    XtFree ((char *) pWsHints->pWorkspaces);
	}
	XtFree ((char *)pWsHints);
    }
}
