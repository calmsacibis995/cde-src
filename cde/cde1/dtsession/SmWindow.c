/* $XConsortium: SmWindow.c /main/cde1_maint/2 1995/08/30 16:34:42 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmWindow.c
 **
 **  Project:     HP DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  This file contains all routines needed to query the window tree.
 **  The window tree needs to be queried to find all top level windows.
 **
 **
 *******************************************************************
 **  (c) Copyright Hewlett-Packard Company, 1990.  All rights are  
 **  reserved.  Copying or other reproduction of this program      
 **  except for archival purposes is prohibited without prior      
 **  written consent of Hewlett-Packard Company.		     
 ********************************************************************
 **
 **
 **
 *****************************************************************************
 *************************************<+>*************************************/

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <Dt/UserMsg.h>
#include "Sm.h"
#include "SmError.h"
#include "SmWindow.h"
#include "SmProtocol.h"
#include "SmGlobals.h"


/*
 * Variables global to this module only
 */
static Boolean commandTimeout;

/*
 * Local Function declarations
 */
#ifdef _NO_PROTO

static WindowInfo GetTopLevelWindowInfo() ;
static void WaitForCommand() ;
/*
 * Timeout procedure on the wait for update in the WM_COMMAND
 * procedure and the global data associated with it
 */
static void WaitTimeout() ;

#else

static WindowInfo GetTopLevelWindowInfo( 
                        Window window) ;
static void WaitForCommand( 
                        Window window) ;
static void WaitTimeout( XtPointer , XtIntervalId *) ;

#endif /* _NO_PROTO */



/*************************************<->*************************************
 *
 *  GetTopLevelWindowInfo (window)
 *
 *
 *  Description:
 *  -----------
 *  Given a child of the root - find the top level window for that child.
 *
 *
 *  Inputs:
 *  ------
 *  window = the current window that is being queried about
 *
 * 
 *  Outputs:
 *  -------
 *  retInfo = a WindowInfo structure (a window id + state of the window) that
 *            gives the top level window information.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static WindowInfo 
#ifdef _NO_PROTO
GetTopLevelWindowInfo( window )
        Window window ;
#else
GetTopLevelWindowInfo(
        Window window )
#endif /* _NO_PROTO */
{
    register int i;
    Window parent,root,*children;
    WindowInfo retInfo;
    Atom actualType;
    int actualFormat;
    unsigned long nitems;
    unsigned long leftover;
    unsigned int nchildren;
    WM_STATE *wmState = NULL;
    XWindowAttributes windowAttr;

    XGetWindowAttributes(smGD.display, window,&windowAttr);

    /*
     * If WM_STATE could not be interned at the beginning - the window manager
     * may have been slow in coming up.  Try it again now.
     */
    if(XaWmState == None)
    {
	XaWmState = XInternAtom(smGD.display, _XA_WM_STATE, True);
    }

    if (Success != XGetWindowProperty(smGD.display,window,XaWmState,
		       0L,(long)BUFSIZ,False,
		       XaWmState,&actualType,&actualFormat,
		       &nitems,
		       &leftover,(unsigned char **) &wmState)) { 

	retInfo.wid = 0;
	retInfo.termState = 0;

	return(retInfo);
    }

    if (actualType==XaWmState)
    {
	retInfo.wid = window;

	if (nitems > 0 ) {
	    retInfo.termState = wmState->state;
	} else {
	    retInfo.termState = 0;
	}

	/*
	 * This data needs to be freed up!
	 */
	SM_FREE((char *) wmState);
        return(retInfo);
    }
    else 
    {
	/*
	 * Be sure to free the window property each time we get it
	 * if the property exists
	 */
	if(actualType != None)
	{
	    SM_FREE((char *) wmState);
	}
	
        if(XQueryTree(smGD.display,window,&root,
		      &parent,&children,&nchildren) != 0)
	{
	    if(nchildren > 0)
	    {
		i = 0;
		while (nchildren--) 
		{
		    retInfo = GetTopLevelWindowInfo(children[i++]);
		    if(retInfo.wid != 0)
		    {
			SM_FREE((char *) children);
			return(retInfo);
		    }
		}
		SM_FREE((char *) children);
	    }
	}
	retInfo.wid = 0;
	retInfo.termState = 0;
        return(retInfo);
    }
}



/*************************************<->*************************************
 *
 *  WaitForCommand (window)
 *
 *
 *  Description:
 *  -----------
 *  This routine waits for an update on the WM_COMMAND property of a top
 *  level window after a WM_SAVE_YOURSELF has been placed on that window.
 *
 *
 *  Inputs:
 *  ------
 *  window = window id for the
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
WaitForCommand( window )
        Window window ;
#else
WaitForCommand(
        Window window )
#endif /* _NO_PROTO */
{
    XtInputMask isThere;
    XEvent event;
    XPropertyEvent *pEvent=(XPropertyEvent *)&event;
    XtIntervalId	comTimerId;
    Boolean commandUpdated;

    /*
     * Set a 10 second timer which stops the block
     */
    commandUpdated = False;
    commandTimeout = False;
    comTimerId = XtAppAddTimeOut(smGD.appCon, smRes.saveYourselfTimeout,
				 WaitTimeout, (XtPointer) window);

    while((commandUpdated == False) && (commandTimeout == False))
    {
	if((isThere = XtAppPending(smGD.appCon)) != 0)
	{
	    if(isThere & XtIMXEvent)
	    {
		XtAppPeekEvent(smGD.appCon, &event);
		if (event.type==PropertyNotify&&pEvent->window==window&&
		    pEvent->atom==XA_WM_COMMAND)
		{
		    commandUpdated = True;
		}
	    }
	    if(commandTimeout == False)
	    {
		XtAppProcessEvent(smGD.appCon, XtIMXEvent | XtIMTimer);
	    }
	}
    }

    XtRemoveTimeOut(comTimerId);
    XSelectInput(smGD.display, window,NoEventMask);
    return;
}



/*************************************<->*************************************
 *
 *  SaveYourself (windowInfo)
 *
 *
 *  Description:
 *  -----------
 *  Places the WM_SAVE_YOURSELF property on each top level window.  It then
 *  waits for the window to update its WM_COMMAND property.
 *
 *
 *  Inputs:
 *  ------
 *  windowInfo = window id for the top level wincow and the state of that
 *               window.
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
SaveYourself( windowInfo )
        WindowInfo windowInfo ;
#else
SaveYourself(
        WindowInfo windowInfo )
#endif /* _NO_PROTO */
{

    int i;
    Atom *protoRet;
    int nitems;
    XClientMessageEvent saveYourselfMessage;
    
    /*
     * Get the WM_PROTOCOLS property on the clients top-level window.
     */
    if(XGetWMProtocols(smGD.display, windowInfo.wid, &protoRet, &nitems) == 0)
    {
	/*
	 * If the client doesn't have a WM_PROTOCOLS property,
	 * it doesn't support any protocols.
	 */
        return (0);	
    }
	
    /* Look for WM_SAVE_YOURSELF atom.  */
    for (i=0;i<nitems;++i) 
    {
	if (protoRet[i]==XaWmSaveYourself) 
	    break;
    }

    if (i==nitems)
    {
	SM_FREE((char *) protoRet);
	return(0);	/* doesn't participate in WM_SAVE_YOURSELF */
    }

    /* Construct the ClientMessage. */
    saveYourselfMessage.type=ClientMessage;
    saveYourselfMessage.window=windowInfo.wid;
    saveYourselfMessage.message_type=XaWmProtocols;
    saveYourselfMessage.format=32;
    saveYourselfMessage.data.l[0]=XaWmSaveYourself;
    saveYourselfMessage.data.l[1]=CurrentTime;
    
    /*
     * look for changes in WM_COMMAND property
     */
    XSelectInput(smGD.display,windowInfo.wid,PropertyChangeMask);
    XFlush(smGD.display);

    /*
     * Send the ClientMessage to the client.  XSendEvent returns a 1 if it
     * is successful in converting the event to a wire event.
     */
    if (XSendEvent(smGD.display,windowInfo.wid,False,NoEventMask,
		   (XEvent *) &saveYourselfMessage) != 1)
    {
	PrintError(DtError, GETMESSAGE(20, 1, "Client message failed.  Client information will not be saved."));
	return(-1);
    }

    /* Wait for client to update WM_COMMAND. */
    WaitForCommand(windowInfo.wid);

    SM_FREE((char *) protoRet);
    
    return(0);
}



/*************************************<->*************************************
 *
 *  GetTopLevelWindows (screen, toplist, toplistlength, containedListLength)
 *
 *
 *  Description:
 *  -----------
 *  Querys the window tree and constructs a list of all top level windows
 *
 *
 *  Inputs:
 *  ------
 *  screen = pointer to the screen we're currently querying on
 *
 * 
 *  Outputs:
 *  -------
 *  toplist = a pointer to the list of top level windows
 *  toplistlength = the length of the list of top level windows
 *  containedListLength = the length of the list of contained windows
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
int 
#ifdef _NO_PROTO
GetTopLevelWindows( screen, topList, topListLength, containedListLength )
        int screen ;
        WindowInfo **topList ;
        unsigned int *topListLength ;
        unsigned int *containedListLength ;
#else
GetTopLevelWindows(
        int screen,
        WindowInfo **topList,
        unsigned int *topListLength,
        unsigned int *containedListLength )
#endif /* _NO_PROTO */
{
    Window rootWindow, parentWindow, *tmpChild;
    Window *childList, *embeddedList, *tmpList;
    WindowInfo topLevelWindowInfo;
    int i;
    unsigned long numEmbedded;

    /*
     * Get a list of children of the root window
     */
    if (XQueryTree(smGD.display, RootWindow(smGD.display, screen),&rootWindow,
	    &parentWindow, &childList, topListLength) == 0)
    {
	PrintError(DtError, GETMESSAGE(20, 2, "Invalid root window.  Can not save client information."));
        SM_EXIT(-1);
    }

    /*
     * add in the list of top level windows embedded in the front panel
     */
    if(_DtGetEmbeddedClients(smGD.display, RootWindow(smGD.display, screen),
			     &embeddedList, &numEmbedded) != Success)
    {
	numEmbedded = 0;
    }
    
    if (*topListLength) 
	*topList=(WindowInfo *) SM_MALLOC(sizeof(WindowInfo)*
				       (*topListLength + numEmbedded));

    tmpChild = childList;
    /* scan list */
    for (i=0 ; i<*topListLength; ++i, tmpChild++) 
    {
        topLevelWindowInfo = GetTopLevelWindowInfo(*tmpChild);
        if (!topLevelWindowInfo.wid)
	{
	    topLevelWindowInfo.wid = (*tmpChild);
	    /*
	     * Assume if you can't find a state that it is "don't care"
	     * this could be a faulty assumption CHECK IT OUT
	     */
	    topLevelWindowInfo.termState = 0;
	}
        (*topList)[i] = topLevelWindowInfo;
    }

    /*
     * Now add in the extra window id's to check
     */
    tmpList = embeddedList;
    for(i = *topListLength;i < (*topListLength + numEmbedded);i++)
    {
	(*topList)[i].wid = *tmpList;tmpList++;
	(*topList)[i].termState = NormalState;
    }

    if(numEmbedded > 0)
    {
	SM_FREE((char *) embeddedList);
    }

    if(*topListLength)
    {
	SM_FREE((char *) childList);
    }
   
   *containedListLength = numEmbedded;
    
    return(0);
}



/*************************************<->*************************************
 *
 *  WaitTimeout
 *
 *
 *  Description:
 *  -----------
 *  Timeout procedure the WaitForCommand routine.  It stops a loop waiting
 *  for an update of the WM_COMMAND property from a client.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  commandTimeout = (global) flag that stops the loop
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
WaitTimeout( client_data, id )
        XtPointer client_data ;
        XtIntervalId *id ;
#else
WaitTimeout(
        XtPointer client_data,
        XtIntervalId *id )
#endif /* _NO_PROTO */
{
    String tmpString, tmpError;
    char   *winName;
    Status success;

    success = XFetchName(smGD.display, (Window) client_data, &winName);
    if (success && winName)
    {
	tmpString = GETMESSAGE(20, 4, "Session restoration information not updated for client %s.  Invalid information may be saved.");
	tmpError = SM_MALLOC(strlen(winName) + strlen(tmpString) + 5);
	if (tmpError)
	{
	    sprintf(tmpError, tmpString, winName);
	    PrintError(DtError, tmpError);
	    SM_FREE(tmpError);
	}
	SM_FREE(winName);
    }
    commandTimeout = True;
    return;
} /* END OF FUNCTION WaitTimeout */
    
