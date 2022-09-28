/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmProtocol.c /main/cde1_maint/1 1995/07/15 01:57:39 drk $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmICCC.h"

/*
 * include extern functions
 */

#include "WmError.h"
#include "WmFunction.h"
#include "WmKeyFocus.h"
#include "WmMenu.h"
#include "WmWinInfo.h"
#ifndef NO_WMQUERY 
#include "WmEvent.h"
#endif /* NO_WMQUERY */
#ifdef PANELIST
#include "WmPanelP.h"
#endif /* PANELIST */

/*
 * Function Declarations:
 */

#include "WmProtocol.h"
#ifdef _NO_PROTO
#ifndef NO_WMQUERY
static Boolean wmq_convert();
static Boolean wmq_convert_all_clients();
static void wmq_list_subtree();
static void wmq_add_xid();
static void wmq_done();
static void wmq_lose();
static void wmq_bump_xids();
#endif /* NO_WMQUERY */
#else /* _NO_PROTO */
#ifndef NO_WMQUERY
static Boolean wmq_convert (Widget w, Atom *pSelection, Atom *pTarget, 
    Atom *pType_return, XtPointer *pValue_return, unsigned long *pLength_return,
    int *pFormat_return);
static Boolean wmq_convert_all_clients (Widget w, int screen,
    Atom *pType_return, XtPointer *pValue_return, unsigned long *pLength_return,
    int *pFormat_return);
static void wmq_list_subtree (ClientData *pCD);
static void wmq_add_xid (XID win);
static void wmq_done (Widget w, Atom *pSelection, Atom *pTarget);
static void wmq_lose (Widget w, Atom *pSelection);
static void wmq_bump_xids(void);
#endif /* NO_WMQUERY */
#endif /* _NO_PROTO */



/*
 * Global Variables:
 */
#ifndef NO_WMQUERY
Atom *xa_WM_QUERY = NULL;
Atom xa_WM_POINTER_WINDOW;
Atom xa_WM_CLIENT_WINDOW;
Atom xa_WM_ALL_CLIENTS;
XID *pXids = NULL;
int numXids = -1;
int curXids = 0;
#endif /* NO_WMQUERY */



/*************************************<->*************************************
 *
 *  SetupWmICCC ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the window manager handling of the inter-client
 *  communications conventions.
 *
 *
 *  Outputs:
 *  -------
 *  (wmGD) = Atoms id's are setup.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void SetupWmICCC ()

#else /* _NO_PROTO */
void SetupWmICCC (void)
#endif /* _NO_PROTO */
{
    XIconSize sizeList;
    int scr;

    /*
     * Make atoms that are required by the ICCC and mwm.  The atom for
     * _MOTIF_WM_INFO is intern'ed in ProcessMotifWmInfo.
     */

    wmGD.xa_WM_STATE = XInternAtom (DISPLAY, _XA_WM_STATE, False);
    wmGD.xa_WM_PROTOCOLS = XInternAtom (DISPLAY, _XA_WM_PROTOCOLS, False);
    wmGD.xa_MAIL_STATUS = XmInternAtom(DISPLAY, "_SUN_MAIL_STATUS", False);
    wmGD.xa_WM_CHANGE_STATE = XInternAtom (DISPLAY, _XA_WM_CHANGE_STATE, False);
    wmGD.xa_WM_SAVE_YOURSELF = XInternAtom (DISPLAY, _XA_WM_SAVE_YOURSELF,
				   False);
    wmGD.xa_WM_DELETE_WINDOW = XInternAtom (DISPLAY, _XA_WM_DELETE_WINDOW,
				   False);
    wmGD.xa_WM_COLORMAP_WINDOWS = XInternAtom (DISPLAY, _XA_WM_COLORMAP_WINDOWS,
				   False);
    wmGD.xa_WM_TAKE_FOCUS = XInternAtom (DISPLAY, _XA_WM_TAKE_FOCUS, False);
    wmGD.xa_MWM_HINTS = XInternAtom (DISPLAY, _XA_MWM_HINTS, False);
    wmGD.xa_MWM_MENU = XInternAtom (DISPLAY, _XA_MWM_MENU, False);
    wmGD.xa_MWM_MESSAGES = XInternAtom (DISPLAY, _XA_MWM_MESSAGES, False);
    wmGD.xa_MWM_OFFSET = XInternAtom (DISPLAY, _XA_MOTIF_WM_OFFSET, False);
#ifdef AUTOMATION
    wmGD.xa_MWM_FRAME_ICON_INFO = XInternAtom (DISPLAY,
                                               _XA_MWM_FRAME_ICON_INFO, False);
#endif
    wmGD.xa_COMPOUND_TEXT = XInternAtom (DISPLAY, "COMPOUND_TEXT", False);

#ifndef NO_WMQUERY
    if (!(xa_WM_QUERY = (Atom *) XtMalloc (wmGD.numScreens * (sizeof (Atom)))))
    {
	Warning (((char *)GETMESSAGE(56, 2, "Insufficient memory to XInternAtom _MOTIF_WM_QUERY_nn")));
    }

    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	if (wmGD.Screens[scr].managed)
	{
	  char wm_query_scr[32];

          sprintf(wm_query_scr, "_MOTIF_WM_QUERY_%d",
                                        wmGD.Screens[scr].screen);
          xa_WM_QUERY[scr] =
                XInternAtom (DISPLAY, wm_query_scr, False);
        }
        else
        {
          xa_WM_QUERY[scr] = 0;
	}
    }
    xa_WM_CLIENT_WINDOW = 
	    XInternAtom (DISPLAY, "_MOTIF_WM_CLIENT_WINDOW", False);
    xa_WM_POINTER_WINDOW = 
	    XInternAtom (DISPLAY, "_MOTIF_WM_POINTER_WINDOW", False);
    xa_WM_ALL_CLIENTS = 
	    XInternAtom (DISPLAY, "_MOTIF_WM_ALL_CLIENTS", False);
#endif /* NO_WMQUERY */


    /*
     * Setup the icon size property on the root window.
     */

    sizeList.width_inc = 1;
    sizeList.height_inc = 1;

    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	if (wmGD.Screens[scr].managed)
	{
	    sizeList.min_width = wmGD.Screens[scr].iconImageMinimum.width;
	    sizeList.min_height = wmGD.Screens[scr].iconImageMinimum.height;
	    sizeList.max_width = wmGD.Screens[scr].iconImageMaximum.width;
	    sizeList.max_height = wmGD.Screens[scr].iconImageMaximum.height;

	    XSetIconSizes (DISPLAY, wmGD.Screens[scr].rootWindow, 
		&sizeList, 1);
	}
    }

#ifndef NO_WMQUERY
    /*
     * Assert ownership of the WM_QUERY selection
     */
    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	if (wmGD.Screens[scr].managed)
	{
	    if (!XtOwnSelection(wmGD.topLevelW,
				xa_WM_QUERY[scr],
				GetTimestamp(),
				wmq_convert,
				wmq_lose,
				wmq_done))
	      {
		 Warning (((char *)GETMESSAGE(56, 3, "Failed to own _MOTIF_WM_QUERY_nn selection")));
	      }
	}
    }
#endif /* NO_WMQUERY */


} /* END OF FUNCTION SetupWmICCC */



/*************************************<->*************************************
 *
 *  SendConfigureNotify (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to send a synthetic ConfigureNotify event when
 *  a client window is reconfigured in certain ways (e.g., the window is
 *  moved without being resized).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (window id and client size data)
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void SendConfigureNotify (pCD)
	ClientData *pCD;

#else /* _NO_PROTO */
void SendConfigureNotify (ClientData *pCD)
#endif /* _NO_PROTO */
{
    XConfigureEvent notifyEvent;


    /*
     * Send a synthetic ConfigureNotify message:
     */

    notifyEvent.type = ConfigureNotify;
    notifyEvent.display = DISPLAY;
    notifyEvent.event = pCD->client;
    notifyEvent.window = pCD->client;
#ifdef PANELIST
    if (pCD->pECD)
    {
	int rootX, rootY;
	Window wChild;
     	WmFpEmbeddedClientData *pECD = (WmFpEmbeddedClientData *)pCD->pECD;

	/*
	 * The front panel uses clientX, clientY for position in
	 * front panel. Translate to root coords for client's
	 * information.
	 */

	XTranslateCoordinates (DISPLAY, pECD->winParent,
	    ROOT_FOR_CLIENT(pCD), pCD->clientX, pCD->clientY, 
	    &rootX, &rootY, &wChild);

	notifyEvent.x = rootX;
	notifyEvent.y = rootY;
	notifyEvent.width = pCD->clientWidth;
	notifyEvent.height = pCD->clientHeight;
    }
    else
#else /* PANELIST */
#endif /* PANELIST */
    if (pCD->maxConfig)
    {
	notifyEvent.x = pCD->maxX;
	notifyEvent.y = pCD->maxY;
	notifyEvent.width = pCD->maxWidth;
	notifyEvent.height = pCD->maxHeight;
    }
    else
    {
	notifyEvent.x = pCD->clientX;
	notifyEvent.y = pCD->clientY;
	notifyEvent.width = pCD->clientWidth;
	notifyEvent.height = pCD->clientHeight;
    }
    notifyEvent.border_width = 0;
    notifyEvent.above = None;
    notifyEvent.override_redirect = False;

    XSendEvent (DISPLAY, pCD->client, False, StructureNotifyMask,
	(XEvent *)&notifyEvent);


} /* END OF FUNCTION SendConfigureNotify */



/*************************************<->*************************************
 *
 *  SendClientOffsetMessage (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to send a client message containing the offset
 *  between the window position reported to the user and the actual
 *  window position of the client over the root.
 *
 *  This can be used by clients that map and unmap windows to help them
 *  work with the window manager to place the window in the same location
 *  when remapped. 
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (frame geometry info)
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void SendClientOffsetMessage (pCD)
	ClientData *pCD;

#else /* _NO_PROTO */
void SendClientOffsetMessage (ClientData *pCD)
#endif /* _NO_PROTO */
{
    long borderWidth = (long)pCD->xBorderWidth;
    long offsetX = pCD->clientOffset.x;
    long offsetY = pCD->clientOffset.y;
      
    XClientMessageEvent clientMsgEvent;

    clientMsgEvent.type = ClientMessage;
    clientMsgEvent.window = pCD->client;
    clientMsgEvent.message_type = wmGD.xa_MWM_MESSAGES;
    clientMsgEvent.format = 32;
    clientMsgEvent.data.l[0] = wmGD.xa_MWM_OFFSET;

    /*
     * Use window gravity to allow the user to specify the window
     * position on the screen  without having to know the dimensions
     * of the decoration that mwm is adding.
     */
    
    switch (pCD->windowGravity)
    {
      case NorthWestGravity:
      default:
	{
	    clientMsgEvent.data.l[1] = offsetX;
	    clientMsgEvent.data.l[2] = offsetY;
	    break;
	}
	
      case NorthGravity:
	{
	    clientMsgEvent.data.l[1] = borderWidth;
	    clientMsgEvent.data.l[2] = offsetY;
	    break;
	}
	
      case NorthEastGravity:
	{
	    clientMsgEvent.data.l[1] = -(offsetX - (2 * borderWidth));
	    clientMsgEvent.data.l[2] = offsetY;
	    break;
	}
	
      case EastGravity:
	{
	    clientMsgEvent.data.l[1] = -(offsetX - (2 * borderWidth));
	    clientMsgEvent.data.l[2] = borderWidth + (offsetY - offsetX)/2;
	    break;
	}
	
      case SouthEastGravity:
	{
	    clientMsgEvent.data.l[1] = -(offsetX - (2 * borderWidth));
	    clientMsgEvent.data.l[2] = -(offsetX - (2 * borderWidth));
	    break;
	}
	
      case SouthGravity:
	{
	    clientMsgEvent.data.l[1] = borderWidth;
	    clientMsgEvent.data.l[2] = -(offsetX - (2 * borderWidth));
	    break;
	}
	
      case SouthWestGravity:
	{
	    clientMsgEvent.data.l[1] = offsetX;
	    clientMsgEvent.data.l[2] = -(offsetX - (2 * borderWidth));
	    break;
	}
	
      case WestGravity:
	{
	    clientMsgEvent.data.l[1] = offsetX;
	    clientMsgEvent.data.l[2] = borderWidth + (offsetY - offsetX)/2;
	    break;
	}
	
      case CenterGravity:
	{
	    clientMsgEvent.data.l[2] = (offsetY - offsetX)/2;
	    break;
	}
    }

    XSendEvent (DISPLAY, pCD->client, False, NoEventMask,
	(XEvent *)&clientMsgEvent);


} /* END OF FUNCTION SendClientOffsetMessage */


/*************************************<->*************************************
 *
 *  SendClientMsg (window, type, data0, time, pData, dataLen)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to send a client message event that to a client
 *  window.  The message may be sent as part of a protocol arranged for by
 *  the client with the WM_PROTOCOLS property.
 *
 *
 *  Inputs:
 *  ------
 *  window = destination window for the client message event
 *
 *  type = client message type
 *
 *  data0 = data0 value in the client message
 *
 *  time = timestamp to be used in the event
 *
 *  pData = pointer to data to be used in the event
 *
 *  dataLen = len of data (in 32 bit units)
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void SendClientMsg (window, type, data0, time, pData, dataLen)
	Window window;
	long type;
	long data0;
	Time time;
	long *pData;
	int dataLen;

#else /* _NO_PROTO */
void SendClientMsg (Window window, long type, long data0, Time time, long *pData, int dataLen)
#endif /* _NO_PROTO */
{
    XClientMessageEvent clientMsgEvent;
    int i;


    clientMsgEvent.type = ClientMessage;
    clientMsgEvent.window = window;
    clientMsgEvent.message_type = type;
    clientMsgEvent.format = 32;
    clientMsgEvent.data.l[0] = data0;
    clientMsgEvent.data.l[1] = (long)time;
    if (pData)
    {
	/*
	 * Fill in the rest of the ClientMessage event (that holds up to
	 * 5 words of data).
	 */

        if (dataLen > 3)
        {
	    dataLen = 3;
        }
        for (i = 2; i < (2 + dataLen); i++)
        {
	    clientMsgEvent.data.l[i] = pData[i];
        }
    }
    
    
    XSendEvent (DISPLAY, window, False, NoEventMask,
	(XEvent *)&clientMsgEvent);
    XFlush(DISPLAY);


} /* END OF FUNCTION SendClientMsg */



/*************************************<->*************************************
 *
 *  AddWmTimer (timerType, timerInterval, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function sets a window manager timer of the specified type.
 *
 *
 *  Inputs:
 *  ------
 *  timerType = type of timer to be set
 *
 *  timerInterval = length of timeout in ms
 *
 *  pCD = pointer to client data associated with the timer
 *
 *  return = True if timer could be set
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean AddWmTimer (timerType, timerInterval, pCD)
unsigned int timerType;
unsigned long timerInterval;
ClientData *pCD;

#else /* _NO_PROTO */
Boolean AddWmTimer (unsigned int timerType, unsigned long timerInterval, ClientData *pCD)
#endif /* _NO_PROTO */
{
    WmTimer *pWmTimer;


    if (!(pWmTimer = (WmTimer *)XtMalloc (sizeof (WmTimer))))
    {
	Warning (((char *)GETMESSAGE(56, 1, "Insufficient memory for window manager data")));
	return (False);
    }

    /* !!! handle for XtAppAddTimeOut error !!! */
    pWmTimer->timerId = XtAppAddTimeOut (wmGD.mwmAppContext, 
			    timerInterval, (XtTimerCallbackProc)TimeoutProc, (caddr_t)pCD);
    pWmTimer->timerCD = pCD;
    pWmTimer->timerType = timerType;
    pWmTimer->nextWmTimer = wmGD.wmTimers;
    wmGD.wmTimers = pWmTimer;

    return(True);

} /* END OF FUNCTION AddWmTimer */



/*************************************<->*************************************
 *
 *  DeleteClientWmTimers (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function deletes all window manager timers that are associated with
 *  the specified client window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data for client whose timers are to be deleted
 *
 *  wmGD = (wmTimers)
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void DeleteClientWmTimers (pCD)
	ClientData *pCD;

#else /* _NO_PROTO */
void DeleteClientWmTimers (ClientData *pCD)
#endif /* _NO_PROTO */
{
    WmTimer *pPrevTimer;
    WmTimer *pWmTimer;
    WmTimer *pRemoveTimer;


    pPrevTimer = NULL;
    pWmTimer = wmGD.wmTimers;
    while (pWmTimer)
    {
	if (pWmTimer->timerCD == pCD)
	{
	    if (pPrevTimer)
	    {
		pPrevTimer->nextWmTimer = pWmTimer->nextWmTimer;
	    }
	    else
	    {
		wmGD.wmTimers = pWmTimer->nextWmTimer;
	    }
	    pRemoveTimer = pWmTimer;
	    pWmTimer = pWmTimer->nextWmTimer;
	    XtRemoveTimeOut (pRemoveTimer->timerId);
	    XtFree ((char *)pRemoveTimer);
	}
	else
	{
	    pPrevTimer = pWmTimer;
	    pWmTimer = pWmTimer->nextWmTimer;
	}
    }


} /* END OF FUNCTION DeleteClientWmTimers */



/*************************************<->*************************************
 *
 *  TimeoutProc (client_data, id)
 *
 *
 *  Description:
 *  -----------
 *  This function is an Xtk timeout handler.  It is used to handle various
 *  window manager timers (i.e. WM_SAVE_YOURSELF quit timeout).
 *
 *
 *  Inputs:
 *  ------
 *  client_data = pointer to window manager client data
 *
 *  id = Xtk timer id
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void TimeoutProc (client_data, id)
	caddr_t client_data;
	XtIntervalId *id;

#else /* _NO_PROTO */
void TimeoutProc (caddr_t client_data, XtIntervalId *id)
#endif /* _NO_PROTO */
{
    WmTimer *pPrevTimer;
    WmTimer *pWmTimer;

    
    /*
     * Find out if the timer still needs to be serviced.
     */

    pPrevTimer = NULL;
    pWmTimer = wmGD.wmTimers;
    while (pWmTimer)
    {
	if (pWmTimer->timerId == *id)
	{
	    break;
	}
	pPrevTimer = pWmTimer;
	pWmTimer = pWmTimer->nextWmTimer;
    }

    if (pWmTimer)
    {
	/*
	 * Do the timer related action.
	 */

	switch (pWmTimer->timerType)
	{
	    case TIMER_QUIT:
	    {
		XKillClient (DISPLAY, pWmTimer->timerCD->client);
		break;
	    }

	    case TIMER_RAISE:
	    {
		Boolean sameScreen;

		if ((wmGD.keyboardFocus == pWmTimer->timerCD) &&
		    (pWmTimer->timerCD->focusPriority == 
			(PSD_FOR_CLIENT(pWmTimer->timerCD))->focusPriority) &&
		    (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER) &&
		    (pWmTimer->timerCD == GetClientUnderPointer(&sameScreen)))
		{
		    Do_Raise (pWmTimer->timerCD, (ClientListEntry *)NULL, STACK_NORMAL);
		}
		break;
	    }
	}


	/*
	 * Remove the timer from the wm timer list.
	 */

	if (pPrevTimer)
	{
	    pPrevTimer->nextWmTimer = pWmTimer->nextWmTimer;
	}
	else
	{
	    wmGD.wmTimers = pWmTimer->nextWmTimer;
	}
	XtFree ((char *)pWmTimer);
    }

    /*
     * Free up the timer.
     */

    XtRemoveTimeOut (*id);


} /* END OF FUNCTION TimeoutProc */


#ifndef NO_WMQUERY 

/*************************************<->*************************************
 *
 *  Boolean wmq_convert (w, pSelection, pTarget, pType_return, 
 *	pValue_return, pLength_return, pFormat_return)
 *
 *
 *  Description:
 *  -----------
 *  This function converts WM_QUERY selections
 *
 *  Inputs:
 *  ------
 *  w - widget
 *  pSelection - pointer to selection type (atom)
 *  pTarget - pointer to requested target type (atom)
 *
 *  Outputs:
 *  ------
 *  pType_return - pointer to type of data returned (atom)
 *  pValue_return - pointer to pointer to data returned
 *  pLength_return - ptr to length of data returned
 *  pFormat_return - ptr to format of data returned
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
#ifdef _NO_PROTO
static Boolean
wmq_convert (w, pSelection, pTarget, pType_return, pValue_return,
		pLength_return, pFormat_return)
    Widget w;
    Atom *pSelection;
    Atom *pTarget;
    Atom *pType_return;
    XtPointer *pValue_return;
    unsigned long *pLength_return;
    int *pFormat_return;
#else /* _NO_PROTO */
static Boolean
wmq_convert (
    Widget w,
    Atom *pSelection,
    Atom *pTarget,
    Atom *pType_return,
    XtPointer *pValue_return,
    unsigned long *pLength_return,
    int *pFormat_return
    )
#endif /* _NO_PROTO */
{

    Boolean wm_query_found = False;
    int scr;


    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	if (wmGD.Screens[scr].managed)
	{
	    if (*pSelection == xa_WM_QUERY[scr])
	    {
		wm_query_found = True;
		break;
	    }
	}
    }

    if (wm_query_found)
    {
	if (*pTarget == xa_WM_POINTER_WINDOW)
	{
	    return (False);
	}
	else if (*pTarget == xa_WM_CLIENT_WINDOW)
	{
	    return (False);
	}
	else if (*pTarget == xa_WM_ALL_CLIENTS)
	{
	    return (wmq_convert_all_clients (w, scr, pType_return,
			pValue_return, pLength_return,
			pFormat_return));
	}
    }

    return (wm_query_found);
} /* END OF FUNCTION wmq_convert */


/*************************************<->*************************************
 *
 *  Boolean wmq_convert_all_clients (w, screen, pType_return, 
 *	pValue_return, pLength_return, pFormat_return)
 *
 *
 *  Description:
 *  -----------
 *  This function converts the WM_QUERY selection target WM_ALL_CLIENTS
 *
 *  Inputs:
 *  ------
 *  w - widget
 *  screen - screen number
 *
 *  Outputs:
 *  ------
 *  pType_return - pointer to type of data returned (atom)
 *  pValue_return - pointer to pointer to data returned
 *  pLength_return - ptr to length of data returned
 *  pFormat_return - ptr to format of data returned
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
#ifdef _NO_PROTO
static Boolean
wmq_convert_all_clients (w, screen, pType_return, pValue_return,
		pLength_return, pFormat_return)
    Widget w;
    int screen;
    Atom *pType_return;
    XtPointer *pValue_return;
    unsigned long *pLength_return;
    int *pFormat_return;
#else /* _NO_PROTO */
static Boolean
wmq_convert_all_clients (
    Widget w,
    int screen,
    Atom *pType_return,
    XtPointer *pValue_return,
    unsigned long *pLength_return,
    int *pFormat_return
    )
#endif /* _NO_PROTO */
{
    WmScreenData *pSD = NULL;
    ClientListEntry *pEntry;
    ClientData *pCD;

    /*
     * Start with empty client list
     */
    curXids = 0;

    /*
     * Get all clients on the specified screen
     */

    if (wmGD.Screens[screen].managed) 
    {
	pSD = &wmGD.Screens[screen];
	  
	/*
	 * Traverse the client list for this screen and
	 * add to the list of window IDs 
	 */
	pEntry = pSD->clientList;
	  
	while (pEntry)
	{
	    /* 
	     * Filter out entries for icons
	     */
	    if (pEntry->type != MINIMIZED_STATE)
	    {
		pCD = pEntry->pCD;
		if (pCD->transientChildren)
		{
		    wmq_list_subtree(pCD->transientChildren);
		}
		wmq_add_xid ((XID) pCD->client);
	    }
	    pEntry = pEntry->nextSibling;
	}
    }

    *pType_return = XA_WINDOW;
    *pValue_return = (XtPointer) pXids;
    *pLength_return = curXids;
    *pFormat_return = 32;
    return (True);

} /* END OF FUNCTION wmq_convert_all_clients */


/*************************************<->*************************************
 *
 *  void wmq_list_subtree (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function adds the windows in a transient subtree to the 
 *  global window list
 *
 *  Inputs:
 *  ------
 *  pCD - client data for "leftmost" child of a subtree
 *
 *  Outputs:
 *  ------
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
#ifdef _NO_PROTO
static void
wmq_list_subtree (pCD)
    ClientData *pCD;
#else /* _NO_PROTO */
static void
wmq_list_subtree (
    ClientData *pCD
    )
#endif /* _NO_PROTO */
{

    /*
     * Do children first
     */
    if (pCD->transientChildren)
    {
	wmq_list_subtree(pCD->transientChildren);
    }

    /*
     * Do me
     */
    wmq_add_xid ((XID) pCD->client);

    /*
     * Do siblings
     */
    if (pCD->transientSiblings)
    {
	wmq_list_subtree(pCD->transientSiblings);
    }
	
} /* END OF FUNCTION wmq_list_subtree */



/*************************************<->*************************************
 *
 *  void wmq_add_xid (win)
 *
 *
 *  Description:
 *  -----------
 *  This function adds an xid to the list
 *
 *  Inputs:
 *  ------
 *  win - xid to add
 *
 *  Outputs:
 *  ------
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
#ifdef _NO_PROTO
static void
wmq_add_xid (win)
    XID win;
#else /* _NO_PROTO */
static void
wmq_add_xid (
    XID win
    )
#endif /* _NO_PROTO */
{
    if (curXids >= numXids)
    {
	wmq_bump_xids();
    }

    if (curXids < numXids)
    {
	pXids[curXids++] = win;
    }

} /* END OF FUNCTION wmq_add_xid */



/*************************************<->*************************************
 *
 *  void wmq_lose (w, pSelection)
 *
 *
 *  Description:
 *  -----------
 *  This function is called when we lose the WM_QUERY selection
 *
 *  Inputs:
 *  ------
 *  w - widget
 *  pSelection - pointer to selection type (atom)
 *
 *  Outputs:
 *  ------
 *
 *  Comments:
 *  --------
 *  This shouldn't happen!
 *
 *************************************<->***********************************/
#ifdef _NO_PROTO
static void
wmq_lose (w, pSelection)
    Widget w;
    Atom *pSelection;
#else /* _NO_PROTO */
static void
wmq_lose (
    Widget w,
    Atom *pSelection
    )
#endif /* _NO_PROTO */
{
  Warning (((char *)GETMESSAGE(56, 4, "Lost _MOTIF_WM_QUERY_nn selection")));
} /* END OF FUNCTION wmq_lose */



/*************************************<->*************************************
 *
 *  void wmq_done (w, pSelection, pTarget)
 *
 *
 *  Description:
 *  -----------
 *  This function is called when selection conversion is done.
 *
 *  Inputs:
 *  ------
 *  w - widget
 *  pSelection - pointer to selection type (atom)
 *  pTarget - pointer to requested target type (atom)
 *
 *  Outputs:
 *  ------
 *
 *  Comments:
 *  --------
 *  This is here to prevent Xt from freeing our buffers.
 *
 *************************************<->***********************************/
#ifdef _NO_PROTO
static void
wmq_done (w, pSelection, pTarget)
    Widget w;
    Atom *pSelection;
    Atom *pTarget;
#else /* _NO_PROTO */
static void
wmq_done (
    Widget w,
    Atom *pSelection,
    Atom *pTarget
    )
#endif /* _NO_PROTO */
{
} /* END OF FUNCTION wmq_done */



/*************************************<->*************************************
 *
 *  static void wmq_bump_xids ()
 *
 *
 *  Description:
 *  -----------
 *  This function allocates more xids in our local buffer 
 *
 *  Inputs:
 *  ------
 *  w - widget
 *  pSelection - pointer to selection type (atom)
 *  pTarget - pointer to requested target type (atom)
 *
 *  Outputs:
 *  ------
 *
 *  Comments:
 *  --------
 *  This is here to prevent Xt from freeing our buffers.
 *
 *************************************<->***********************************/
#ifdef _NO_PROTO
static void
wmq_bump_xids ()
#else /* _NO_PROTO */
static void
wmq_bump_xids ( void )
#endif /* _NO_PROTO */
{
    XID *px;

    if (pXids)
    {
	if (!(px = (XID *) 
	  XtRealloc ((char *) pXids, (numXids + 32) * (sizeof (XID)))))
	{
	  Warning (((char *)GETMESSAGE(56, 5, "Insufficient memory to convert _MOTIF_WM_QUERY_nn selection")));
	}
	else
	{
	    pXids = px;
	    numXids += 32;
	}
    }
    else
    {
	if (!(pXids = (XID *) 
	  XtMalloc (32 * (sizeof (XID)))))
	{
	  Warning (((char *)GETMESSAGE(56, 5, "Insufficient memory to convert _MOTIF_WM_QUERY_nn selection")));
	}
	else
	{
	    numXids = 32;
	}
    }
}

#endif /* NO_WMQUERY */
