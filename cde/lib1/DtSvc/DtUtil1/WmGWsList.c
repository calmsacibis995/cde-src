/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmGetWsLis.c
 **
 **   RCS:	$XConsortium: WmGWsList.c /main/cde1_maint/1 1995/07/14 20:27:34 drk $
 **
 **   Project:  DT Workspace Manager
 **
 **   Description: Get Dt Workspace List
 **
 **   (c) Copyright 1992, 1993 by Hewlett-Packard Company
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
#include <X11/Xatom.h>

/*************************************<->*************************************
 *
 *  status DtWsmGetWorkspaceList (display, root, ppWorkspaceList,
 * 							pNumWorkspaces)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WORKSPACE_LIST property 
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window of screen of interest
 *  pNumWorkspaces - the number of workspaces returned
 *  ppWorkspaceList - pointer to a pointer to return
 *
 *  Outputs:
 *  --------
 *  *ppDtWmHints-points to the Workspace List
 *		  (NOTE: This should be freed using XFree)
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
Status
#ifdef _NO_PROTO
DtWsmGetWorkspaceList(display, root, ppWorkspaceList, pNumWorkspaces)
        Display *display;
        Window root;
        Atom **ppWorkspaceList;
	int *pNumWorkspaces;
#else
DtWsmGetWorkspaceList(
        Display *display,
        Window root,
        Atom **ppWorkspaceList,
	int *pNumWorkspaces)
#endif /* _NO_PROTO */
{
    Atom actualType;
    int actualFormat;
    unsigned long leftover, items, length, oldlength;
    int rcode;
    Atom property;
    Window wmWindow;


    if ((rcode=_DtGetMwmWindow (display, root, &wmWindow)) == Success)
    {

	property = XmInternAtom(display, _XA_DT_WORKSPACE_LIST, False); 
	length = BUFSIZ;

	*ppWorkspaceList = NULL;
	if ((rcode=XGetWindowProperty(
			display,
			wmWindow,
			property,
			0L, 				/* offset */
			length,
			False,				/* delete */
			XA_ATOM,			/* req_type */
			&actualType,
			&actualFormat,
			&items,				/* items returned */
			&leftover,
			(unsigned char **)ppWorkspaceList))==Success)
	{

	    if (actualType != XA_ATOM)
	    {
		/* wrong type, force failure */
		rcode = BadValue;
		if (actualType != None)
		{
		    XFree ((char *)*ppWorkspaceList);
		}
		*ppWorkspaceList = NULL;
	    }
	    else
	    {
		*pNumWorkspaces = (int) items;
	    }
	}
    }
	
    return(rcode);

} /* END OF FUNCTION DtWsmGetWorkspaceList */

/*************************************<->*************************************
 *
 *  status DtWsmGetCurrentWorkspace (display, root, paWorkspace)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WORKSPACE_CURRENT property 
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window of screen of interest
 *  paWorkspace  - pointer to a Atom of current workspace (returned)
 *
 *  Outputs:
 *  --------
 *  *paWorkspace - atom for current workspace (represents "name"
 *                 of workspace). 
 *
 *  Comments:
 *  ---------
 *  The Atom is copied to the caller's space. Nothing needs to be
 *  freed.
 * 
 *************************************<->***********************************/
Status
#ifdef _NO_PROTO
DtWsmGetCurrentWorkspace (display, root, paWorkspace)
        Display *display;
        Window root;
        Atom *paWorkspace;
#else
DtWsmGetCurrentWorkspace (
        Display *display,
        Window root,
        Atom *paWorkspace)
#endif /* _NO_PROTO */
{
    Atom actualType;
    int actualFormat;
    unsigned long leftover, items, length;
    int rcode;
    Atom property;
    Window wmWindow;
    Atom *paTemp;


    if ((rcode=_DtGetMwmWindow (display, root, &wmWindow)) == Success)
    {

	property = XmInternAtom(display, _XA_DT_WORKSPACE_CURRENT, False); 
	length = 1;

	if ((rcode=XGetWindowProperty(
			display,
			wmWindow,
			property,
			0L, 				/* offset */
			length,
			False,				/* delete */
			XA_ATOM,			/* req_type */
			&actualType,
			&actualFormat,
			&items,				/* items returned */
			&leftover,
			(unsigned char **)&paTemp))==Success)
	{

	    if (actualType != XA_ATOM)
	    {
		/* wrong type, force failure */
		rcode = BadValue;
		if (actualType != None)
		{
		    XFree ((char *)paTemp);
		}
	    }
	    else
	    {
		*paWorkspace = *paTemp;
		XFree((char *) paTemp);
	    }
	}
    }
	
    return(rcode);

} /* END OF FUNCTION DtWsmGetCurrentWorkspace */

