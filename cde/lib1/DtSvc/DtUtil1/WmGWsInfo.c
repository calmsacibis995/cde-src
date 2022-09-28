/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmGWsInfo.c
 **
 **   RCS:	$XConsortium: WmGWsInfo.c /main/cde1_maint/1 1995/07/14 20:27:21 drk $
 **
 **   Project:  DT Workspace Manager
 **
 **   Description: Get Dt Workspace Info
 **
 **   (c) Copyright 1992,1993 by Hewlett-Packard Company
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
 *  status DtWsmGetWorkspaceInfo (display, root, aWS, ppWsInfo)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the _DT_WORKSPACE_INFO_<name> property 
 *  for workspace <name>.
 *
 *
 *  Inputs:
 *  ------
 *  display	- display 
 *  root	- root window of screen of interest
 *  aWS		- atom for workspace
 *  ppWsInfo	- ptr to WS info buffer ptr (to be returned)
 *
 *  Outputs:
 *  --------
 *  *ppWsInfo	- returned ptr to WS Info. (free with XtFree).
 *
 *
 *  Comments:
 *  ---------
 *  ptr to WorkspaceInfo should be freed by calling
 *  DtWsmFreeWorkspaceInfo.
 * 
 *************************************<->***********************************/
Status
#ifdef _NO_PROTO
DtWsmGetWorkspaceInfo(display, root, aWS, ppWsInfo)
        Display *display;
        Window root;
	Atom aWS;
        DtWsmWorkspaceInfo **ppWsInfo;
#else
DtWsmGetWorkspaceInfo(
        Display *display,
        Window root,
	Atom aWS,
        DtWsmWorkspaceInfo **ppWsInfo)
#endif /* _NO_PROTO */
{
    Status rcode;
    Atom aProperty;
    Window wmWindow;
    DtWsmWorkspaceInfo *pWsInfo;
    char *pchName, *pch;
    int  iLen;
    int  i;
    Window *pWin;
    XTextProperty tp;
    char **ppchList;
    int count, item;

    /* 
     * Construct atom name
     */
    pchName = (char *) XGetAtomName (display, aWS);
    iLen = strlen(pchName) + strlen (_XA_DT_WORKSPACE_INFO) + 4;

    pch = (char *) XtMalloc (iLen);
    strcpy (pch, _XA_DT_WORKSPACE_INFO);
    strcat (pch, "_");
    strcat (pch, pchName);

    aProperty = XInternAtom (display, pch, FALSE);

    XFree ((char *) pchName);
    XtFree ((char *) pch);
    pch = NULL;

    /* 
     * Get window where property is 
     */
    if ((rcode=_DtGetMwmWindow (display, root, &wmWindow)) == Success)
    {
	if ((rcode=XGetTextProperty(
			display,
			wmWindow,
			&tp,
			aProperty))>=Success)
	{
	    if (rcode=XmbTextPropertyToTextList (
				display,
				&tp,
				&ppchList,
				&count) >= Success)
	    {
		pWsInfo = (DtWsmWorkspaceInfo *)
			XtCalloc(1, sizeof(DtWsmWorkspaceInfo));

		pWsInfo->workspace = aWS;
		item = 0;

		/* title */
		if (item < count)
		{
		    pWsInfo->pchTitle = (char *) 
				XtNewString ((String) ppchList[item]);
		    item++;
		}

		/* pixel set id */
		if (item < count)
		{
		    pWsInfo->colorSetId = atoi (ppchList[item]);
		    item++;
		}

		/* backdrop window (moved to end!) */

		/* backdrop background */
		if (item < count)
		{
		    pWsInfo->bg = (unsigned long) 
			    strtol (ppchList[item], (char **) NULL, 0);
		    item++;
		}

		/* backdrop foreground */
		if (item < count)
		{
		    pWsInfo->fg = (unsigned long) 
			    strtol (ppchList[item], (char **) NULL, 0);
		    item++;
		}

		/* backdrop name (atom) */
		if (item < count)
		{
		    pWsInfo->backdropName = 
			    (Atom) strtol (ppchList[item], (char **) NULL, 0);
		    item++;
		}

		/* number of backdrop windows */
		if (item < count)
		{
		    pWsInfo->numBackdropWindows = 
			    (int) strtol (ppchList[item], (char **) NULL, 0);
		    item++;
		}

		/* list of backdrop windows */
		if ((item + pWsInfo->numBackdropWindows) <= count)
		{
		    pWin = (Window *)
			XtMalloc (pWsInfo->numBackdropWindows *
				  sizeof (Window));
		    for (i=0; i<pWsInfo->numBackdropWindows; i++)
		    {
			pWin[i] = (Window) 
			    strtol (ppchList[item], (char **) NULL, 0);
			item++;
		    }
		    pWsInfo->backdropWindows = pWin;
		}
		else
		{
		    /* Bogus backdrop windows info */
		    pWsInfo->numBackdropWindows = 0;
		}

		/* pass back ptr to filled in structure */
		*ppWsInfo = pWsInfo;

		/* free the converted data */
		XFreeStringList (ppchList);
	    }
	    else 
	    {
		/* conversion failed */
		*ppWsInfo = NULL;
	    }
	    /* free the property data */
	    if (tp.value) 
		XFree (tp.value);
	}
    }
	
    if (rcode >= Success) rcode=Success;

    return(rcode);

} /* END OF FUNCTION DtWsmGetWorkspaceInfo */



/*************************************<->*************************************
 *
 *  void DtWsmFreeWorkspaceInfo (pWsInfo)
 *
 *
 *  Description:
 *  -----------
 *  Free a WorkspaceInfo buffer
 *
 *
 *  Inputs:
 *  ------
 *  pWsInfo	- ptr to WS info buffer
 *
 *  Outputs:
 *  --------
 *  None
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
void
#ifdef _NO_PROTO
DtWsmFreeWorkspaceInfo(pWsInfo)
        DtWsmWorkspaceInfo *pWsInfo;
#else
DtWsmFreeWorkspaceInfo(
        DtWsmWorkspaceInfo *pWsInfo)
#endif /* _NO_PROTO */
{
    if (pWsInfo)
    {
	if (pWsInfo->pchTitle)
	    XtFree (pWsInfo->pchTitle);
        if (pWsInfo->backdropWindows)
	    XtFree ((char *)pWsInfo->backdropWindows);
	XtFree ((char *)pWsInfo);
    }

} /* END OF FUNCTION DtWsmFreeWorkspaceInfo */
