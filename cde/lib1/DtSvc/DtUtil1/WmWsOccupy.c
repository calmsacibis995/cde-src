/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmWsOccupy.c
 **
 **   RCS:	$XConsortium: WmWsOccupy.c /main/cde1_maint/1 1995/07/14 20:29:39 drk $
 **   Project:  DT Workspace Manager
 **
 **   Description: Get/Set workspace occupancy of a window
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


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern int DtGetWorkspacesOccupied() ;
extern void DtWsmSetWorkspacesOccupied() ;

#else

extern int DtGetWorkspacesOccupied( 
                        Display *display,
                        Window window,
                        Atom **ppaWs,
                        unsigned long *pNumWs) ;


extern void DtWsmSetWorkspacesOccupied( 
                        Display *display,
                        Window window,
                        Atom *pWsHints,
                        unsigned long numHints) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static int _GetWorkspacesOccupied() ;
#ifdef HP_VUE
static int _GetWorkspacePresence() ;
#endif /* HP_VUE */

#else

static int _GetWorkspacesOccupied( 
                        Display *display,
                        Window window,
                        Atom **ppaWs,
                        unsigned long *pNumWs,
                        Atom property) ;
#ifdef HP_VUE
static int _GetWorkspacePresence(
			Display *display,
			Window window,
			Atom **ppWsPresence,
			unsigned long *pNumPresence,
			Atom property ) ;
#endif /* HP_VUE */

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/



/*************************************<->*************************************
 *
 *  status _GetWorkspacesOccupied (display, window, ppaWs, 
 *                                  pNumWs, property)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of a property on a window that is a list of atoms
 *
 *
 *  Inputs:
 *  ------
 *  display		- display 
 *  window		- window to get hints from
 *  ppaWs	- pointer to a pointer to return
 *  pNumWs	- pointer to a number to return
 *  property		- the property atom
 *
 *  Outputs:
 *  --------
 *  *ppaWs	- points to the list of workspace info structures
 *                 	  (NOTE: This should be freed using XFree)
 *  *pNumWs  	- the number of workspace info structure in the list
 *  Return		- status from XGetWindowProperty
 *
 *  Comments:
 *  ---------
 *  Assumes that less than BUFSIZ bytes will be returned. This code
 *  won't work for very large amounts of info (lots of workspaces).
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
_GetWorkspacesOccupied( display, window, ppaWs, pNumWs, property )
        Display *display ;
        Window window ;
        Atom **ppaWs ;
        unsigned long *pNumWs ;
        Atom property ;
#else
_GetWorkspacesOccupied(
        Display *display,
        Window window,
        Atom **ppaWs,
        unsigned long *pNumWs,
        Atom property )
#endif /* _NO_PROTO */
{
    Atom actualType;
    int actualFormat;
    unsigned long leftover;
    int rcode;

    *ppaWs = NULL;
    if ((rcode=XGetWindowProperty(display,window,
			 property,0L, (long)BUFSIZ,
			 False,property,
			 &actualType,&actualFormat,
			 pNumWs,&leftover,(unsigned char **)ppaWs))==Success)
    {

        if (actualType != property)
	{
	    /* wrong type, force failure */
	    *pNumWs = 0;
	    rcode = BadValue;
	    if (actualType != None)
	    {
		XFree ((char *)*ppaWs);
	    }
	}
    }
	
    return(rcode);

} /* END OF FUNCTION GetWorkspacesOccupied */

#ifdef HP_VUE

/*************************************<->*************************************
 *
 *  status _GetWorkspacePresence (display, window, ppWsPresence, 
 *                                  pNumPresence, property)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WORKSPACE_PRESENCE property on a window
 *
 *
 *  Inputs:
 *  ------
 *  display		- display 
 *  window		- window to get hints from
 *  ppWsPresence	- pointer to a pointer to return
 *  pNumPresence	- pointer to a number to return
 *  property		- the property atom
 *
 *  Outputs:
 *  --------
 *  *ppWsPresence	- points to the list of workspace info structures
 *                 	  (NOTE: This should be freed using XFree)
 *  *pNumPresence  	- the number of workspace info structure in the list
 *  Return		- status from XGetWindowProperty
 *
 *  Comments:
 *  ---------
 *  Assumes that less than BUFSIZ bytes will be returned. This code
 *  won't work for very large amounts of info (lots of workspaces).
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
_GetWorkspacePresence( display, window, ppWsPresence, pNumPresence, property )
        Display *display ;
        Window window ;
        Atom **ppWsPresence ;
        unsigned long *pNumPresence ;
        Atom property ;
#else
_GetWorkspacePresence(
        Display *display,
        Window window,
        Atom **ppWsPresence,
        unsigned long *pNumPresence,
        Atom property )
#endif /* _NO_PROTO */
{
    Atom actualType;
    int actualFormat;
    unsigned long leftover;
    int rcode;

    *ppWsPresence = NULL;
    if ((rcode=XGetWindowProperty(display,window,
			 property,0L, (long)BUFSIZ,
			 False,property,
			 &actualType,&actualFormat,
			 pNumPresence,&leftover,(unsigned char **)ppWsPresence))==Success)
    {

        if (actualType != property)
	{
	    /* wrong type, force failure */
	    *pNumPresence = 0;
	    rcode = BadValue;
	    if (actualType != None)
	    {
		XFree ((char *)*ppWsPresence);
	    }
	}
    }
	
    return(rcode);

} /* END OF FUNCTION GetWorkspacePresence */
#endif /* HP_VUE */


/*************************************<->*************************************
 *
 *  int DtWsmGetWorkspacesOccupied (display, window, ppaWs, pNumWs)
 *
 *
 *  Description:
 *  -----------
 *  Get the list of workspaces that this window is in.
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  window	- window to get info from
 *  ppaWs	- pointer to an atom pointer (to be returned)
 *  pNumWs	- pointer to a number (to be returned)
 *
 *  Outputs:
 *  -------
 *  *ppaWs	- pointer to a list of workspace atoms
 *             	  (NOTE: This should be freed using XFree)
 *  *pNumWs	- number of workspace in the list
 *  Return	- Success if something returned
 *		  not Success otherwise.
 * 
 *  Comments:
 *  --------
 *  Use XFree to free the returned data.
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
DtWsmGetWorkspacesOccupied( display, window, ppaWs, pNumWs )
        Display *display ;
        Window window ;
        Atom **ppaWs ;
        unsigned long *pNumWs ;
#else
DtWsmGetWorkspacesOccupied(
        Display *display,
        Window window,
        Atom **ppaWs,
        unsigned long *pNumWs)
#endif /* _NO_PROTO */
{
    int rcode;
    rcode =  _GetWorkspacesOccupied (display, window, ppaWs, 
			pNumWs, 
			XmInternAtom(display, _XA_DT_WORKSPACE_PRESENCE,
			    False));
#ifdef HP_VUE
    /*
     * Be compatible with HP VUE
     */
    if (rcode != Success)
    {
	rcode =  _GetWorkspacePresence (display, window, ppaWs, 
			pNumWs, 
			XmInternAtom(display, _XA_VUE_WORKSPACE_PRESENCE,
			    False));
    }
#endif /* HP_VUE */

    return (rcode);
}

/*************************************<->*************************************
 *
 *  DtWsmSetWorkspacesOccupied (display, window, pWsHints, numHints)
 *
 *
 *  Description:
 *  -----------
 *  Set the set of workspaces to be occupied by this client.
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  window	- window to get hints from
 *  pWsHints	- pointer to a list of workspace atoms
 *  numHints	- number of atoms in list
 *
 *  Comments:
 *  ---------
 *  No error checking
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
DtWsmSetWorkspacesOccupied( display, window, pWsHints, numHints )
        Display *display ;
        Window window ;
        Atom *pWsHints ;
        unsigned long numHints ;
#else
DtWsmSetWorkspacesOccupied(
        Display *display,
        Window window,
        Atom *pWsHints,
        unsigned long numHints )
#endif /* _NO_PROTO */
{
    DtWorkspaceHints wsh;

    wsh.flags = DT_WORKSPACE_HINTS_WORKSPACES;
    wsh.pWorkspaces = pWsHints;
    wsh.numWorkspaces = numHints;

    _DtWsmSetWorkspaceHints(display, window, &wsh);

}


/*************************************<->*************************************
 *
 *  DtWsmOccupyAllWorkspaces (display, window)
 *
 *
 *  Description:
 *  -----------
 *  Occupy all the workspaces on this screen (including new ones
 *  as they are created)
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  window	- window to get hints from
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
DtWsmOccupyAllWorkspaces( display, window )
        Display *display ;
        Window window ;
#else
DtWsmOccupyAllWorkspaces(
        Display *display,
        Window window)
#endif /* _NO_PROTO */
{
    DtWorkspaceHints wsh;

    wsh.flags = DT_WORKSPACE_HINTS_WSFLAGS;
    wsh.wsflags = DT_WORKSPACE_FLAGS_OCCUPY_ALL;

    _DtWsmSetWorkspaceHints(display, window, &wsh);

}
