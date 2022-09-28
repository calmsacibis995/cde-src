/* 
 * (c) Copyright 1995 Sun Microsystems, Inc.
 * ALL RIGHTS RESERVED 
 */ 

#ifdef FWS

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmWinInfo.h"
#include "WmFws.h"
#include "WmPanelP.h"
#include "WmManage.h"
#include "WmWinList.h"
#include "WmProtocol.h"
#include "WmIPlace.h"
#include "WmCEvent.h"
#include <X11/StringDefs.h>

/*
 * Include extern functions:
 */


/*
 * Global Variables:
 */


/*
 * Static Variables:
 */

typedef struct {
	int   x;
	int   y;
	int   width;
	int   height;
	int   id;
} NextIcon;

static Window FwsCommWindow;
static Widget FwsCommWidget;
static Atom FWS_CLIENT;
static Atom FWS_COMM_WINDOW;
static Atom FWS_PROTOCOLS;
static Atom FWS_REGISTER_WINDOW;
static Atom FWS_NORMAL_STATE;
static Atom FWS_UNSEEN_STATE;
static Atom FWS_PARK_ICONS;
static Atom FWS_HANDLES_FOCUS;
static Atom FWS_PASSES_INPUT;

static Atom FWS_CONFIG_REQUEST;
static Atom FWS_CONFIG_SIZE_POSITION;
static Atom FWS_CONFIG_STACKING;
static Atom FWS_STACK_UNDER;
static Atom FWS_STATE_CHANGE;
static Atom FWS_PASS_ALL_INPUT;

static int numClients;
static int spacesReserved;

static int pendingNotifies = 0;	 /* Number of deletions of the FWS_NEXT_ICON */
				 /* which we caused (so don't replace it). */

/*
 * This "reservation" data is used to keep track of which FWS client windows
 * have been allocated a place for an icon, so that we can recover the icon
 * place when the client window is destroyed.
 *
 * The 'id' is an identifier is uniquely generated so that it can be passed to
 * a client with the description of the icon space.  The client includes this
 * 'id' when it registers its use of the space, allowing us to match the
 * provided window id with the icon place.
 */

typedef struct _reservation {
	struct _reservation *next;	/* forward list pointer. */
	struct _reservation *prev;	/* backward list pointer. */
	int      id;			/* id of reservation. */
	Window   window;		/* FWS client window reserving it. */
	int      place;			/* slot number in placement list. */
	WmWorkspaceData *pWS;		/* workspace in which icon reserved. */
} Reservation;

static int lastId = 0;
static Reservation *reservations = NULL;   /* head of reservation list. */


/*************************************<->*************************************
 *
 * NewReservation() -
 *
 * Allocate a new element in the reservation list for the spcified icon place
 * and workspace.  This is done when the icon place is advertised in the
 * FWS_NEXT_ICON property.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
static int NewReservation (pWS, place)
	WmWorkspaceData *pWS;
	int  place;
#else
static int NewReservation (WmWorkspaceData *pWS,
			   int  place)
#endif /* _NO_PROTO */
{
	Reservation *rsv;

	rsv = (Reservation *) malloc (sizeof (Reservation));
	rsv->id = ++lastId;
	rsv->window = 0;
	rsv->place = place;
	rsv-> pWS = pWS;
	rsv->next = reservations;
	rsv->prev = NULL;
	if (reservations != NULL)
		reservations->prev = rsv;
	reservations = rsv;
	return rsv->id;
}


/*************************************<->*************************************
 *
 * RemoveReservationFromList() -
 *
 * Remove a reservation entry from the linked list,
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void RemoveReservationFromList (rsv)
	Reservation *rsv;
#else
static void RemoveReservationFromList (Reservation *rsv)
#endif /* _NO_PROTO */
{
	if (rsv != NULL)
	{
		if (rsv->prev != NULL)
			rsv->prev->next = rsv->next;
		else
			reservations = rsv->next;
		if (rsv->next != NULL)
			rsv->next->prev = rsv->prev;
	}
}


/*************************************<->*************************************
 *
 * UpdateReservation() -
 *
 * Fill in the FWS client window id for the reservation with the given id.
 * This is done when a ClientMessage has been received from the FWS client
 * registering its window id for the icon place it has claimed.
 *
 * If the client window is specified as 0 (None), then interpret this as a
 * request from a client to release the free the space.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void UpdateReservation (id, window)
	int id;
	Window window;
#else
static void UpdateReservation (int id,
			       Window window)

#endif
{
	Reservation *rsv;

	for (rsv = reservations; rsv != NULL; rsv = rsv->next)
		if (rsv->id == id)
			break;
	if (rsv != NULL)
	{
		if (window == None)
		{
			RemoveReservationFromList (rsv);
			FwsUnreserveIconSpace (rsv->pWS, &rsv->pWS->IPData);
			rsv->pWS->IPData.placeList [rsv->place].pCD = 0;
			FwsReserveIconSpace (rsv->pWS, &rsv->pWS->IPData);
			free (rsv);
		}
		else
			rsv->window = window;
	}
}


/*************************************<->*************************************
 *
 * FwsRemoveReservation() -
 *
 * Make a reserved icon place (in its workspace) available, given the id of
 * the window for which it was reserved.  Remove a reservation from the
 * reservation list and free the space.  This is done when the window is
 * destroyed (i.e. on a DestroyNotify event).
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void FwsRemoveReservation (window)
	Window window;
#else
static void FwsRemoveReservation (Window window)
#endif /* _NO_PROTO */
{
	Reservation *rsv;

	for (rsv = reservations; rsv != NULL; rsv = rsv->next)
		if (rsv->window == window)
			break;

	if (rsv != NULL)
	{
		RemoveReservationFromList (rsv);
		FwsUnreserveIconSpace (rsv->pWS, &rsv->pWS->IPData);
		rsv->pWS->IPData.placeList [rsv->place].pCD = 0;
		FwsReserveIconSpace (rsv->pWS, &rsv->pWS->IPData);
		free (rsv);
	}
}


/*************************************<->*************************************
 *
 * RemoveEmptyReservation() -
 *
 * Make a reserved icon place (in its workspace) available, given the icon
 * place and workspace.  Remove a reservation from the reservation list and
 * free the space.  This is done when a reservation which has not been claimed
 * by an FWS client is withdrawn (e.g. so that the icon place can be used for
 * a more ordinary client).
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void RemoveEmptyReservation (pWS, place)
	WmWorkspaceData   *pWS;
	int               place;
#else
static void RemoveEmptyReservation (WmWorkspaceData   *pWS,
				    int               place)
#endif /* _NO_PROTO */
{
	Reservation *rsv;

	for (rsv = reservations; rsv != NULL; rsv = rsv->next)
		if (rsv->window == 0
		    && rsv->pWS == pWS
		    && rsv->place == place)
			break;

	if (rsv != NULL)
	{
		RemoveReservationFromList (rsv);
		free (rsv);
	}
}


/*************************************<->*************************************
 *
 * FindCLE -
 * 
 * Find the client list entry (in the current workspace's list) which
 * corresponds with a given window id.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
static ClientListEntry *FindCLE (window)
	Window window;
#else
static ClientListEntry *FindCLE (Window window)
#endif /* _NO_PROTO */
{
	ClientListEntry *pCLE;

	for (pCLE = ACTIVE_PSD->clientList;
	     pCLE != NULL;
	     pCLE = pCLE->nextSibling)
		if (pCLE->pCD->clientFrameWin == window)
			return pCLE;
	return NULL;
}


/*************************************<->*************************************
 * ConfigurePending
 *
 * Determine if a ConfigureNotify message for a window is waiting in the
 * event queue.  This would be faster if there were a Xlib routine which
 * checked the queue and neither blocked nor removed an event.
 *
 *************************************<->***********************************/

typedef struct {
	Window  w;
	Bool    found;
} WindowFound;

static Bool ConfigurePredicate (Display *display,
				XEvent  *event,
				char    *arg)
{
	WindowFound *wf = (WindowFound *) arg;
	if (event->type == ConfigureNotify &&
	    event->xconfigure.window == wf->w)
	{
		wf->found = True;
	}
	return False;
}

static Bool ConfigurePending (Window w)
{
	XEvent event;
	WindowFound wf;

	wf.w = w;
	wf.found = False;
	XCheckIfEvent (DISPLAY, &event, ConfigurePredicate, (char *) &wf);
	return wf.found;
}


/*************************************<->*************************************
 *
 * FixClientList
 * 
 * Fix the screen's client list with respect to a particular window.  This fix
 * is required when we find out that an FWS window's stacking order changed,
 * but do not have enough information to adjust our list.  (e.g. The "above"
 * window id in a ConfigureNotify event may not be known to us.)
 *
 * Use XQueryTree() to identify a window in the client list which is just
 * above our FWS client, then fix the list by moving our ClientListEntry in
 * the list.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void FixClientList (window)
	Window window;
 
#else /* _NO_PROTO */
static void FixClientList (Window window)
			       
#endif /* _NO_PROTO */
{
	Window   root;
	Window   parent;
	Window   *children;
	unsigned int nChildren;
	int      i, j;
	ClientListEntry *pCLE;
	ClientListEntry *pStackEntry;

	if (!XQueryTree (DISPLAY, ACTIVE_PSD->rootWindow, &root, &parent,
			 &children, &nChildren) ||
	    children == NULL)
	{
		return;
	}

	for (i = 0; i < nChildren; ++i)
		if (children[i] == window)
			break;

	if (i == nChildren)
	{
		return;
	}

	/*
	 * Now we know where we are in the real stacking order.  Find our
	 * ClientListEntry.
	 */

	pCLE = FindCLE (window);
	pStackEntry = NULL;

	/*
	 * Find a window stacked above this one (i.e. following this one in
	 * the list of children) for which a ClientListEntry exists.
	 *
	 * Note:  The XQueryTree() result may not represent the window order
	 * at the time of processing this ConfigureNotify event because
	 * additional XConfigureWindow() calls (and friends) may have
	 * completed between this event and the query.  This would result in
	 * ConfigureNotify events in our queue.  So -- don't restack anything
	 * with respect to windows with ConfigureNotify events pending -- they
	 * will be restaced later and cover up tempory problems.
	 */

	for (j = i+ 1; j < nChildren; ++j)
	{
		if (ConfigurePending (children[j]))
			continue;
		pStackEntry = FindCLE (children[j]);
		if (pStackEntry != NULL)
			break;
	}

	/*
	 * If pStackWindow == NULL, then there are no dtwm managed windows
	 * between this one and the top -- move this one to the top.
	 * Otherwise, move it under pStackWindow.
	 */

	if (pCLE != NULL)
		if (pStackEntry != NULL)
			MoveEntryInList (ACTIVE_WS, pCLE, False, pStackEntry);
		else
			MoveEntryInList (ACTIVE_WS, pCLE, True, NULL);
	XFree (children);
	return;
}



/*************************************<->*************************************
 *
 *  FwsCommProc ()
 *
 *
 * This function handles events associated with the FWS_COMM_WINDOW window
 * which was created to handle communications with FWS clients.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void FwsCommProc (w, client_data, event, dispatch)
	Widget	w;
	caddr_t	client_data;
	XEvent	*event;
	Boolean *dispatch;
 
#else /* _NO_PROTO */
static void FwsCommProc (Widget   w,
			 caddr_t  client_data,
			 XEvent	  *event,
			 Boolean  *dispatch)
#endif /* _NO_PROTO */
{
	Window        window;
	int           id;
	int           i;
	WmWorkspaceData *pWS;
	ClientData    *pCD;

	switch (event->type)
	{

	case ClientMessage:

		/*
		 * FWS_REGISTER_WINDOW ClientMessage - a client registers a
		 * window as an FWS client window.
		 */

		if (event->xclient.message_type == FWS_REGISTER_WINDOW)
		{
			window = event->xclient.data.l[0];

			/*
			 * Is this an already-registered window attempting to
			 * update the FWS components of the WM_PROTOCOLS?
			 */

			if (XFindContext (DISPLAY, window,
					  wmGD.windowContextType,
					  (XPointer *) &pCD) == 0
			    && pCD != NULL)
			{
				FwsClientInfo (pCD, False);
			}
			else
			{
				ManageWindow (ACTIVE_PSD, window,
					      MANAGEW_NORMAL);
			}
			break;
		}

		/*
		 * FWS_NEXT_ICON ClientMessage - a client registers a specific
		 * window as being associated with an icon place specified
		 * previously in the FWS_NEXT_ICON property.
		 */

		for (i = 0; i < wmGD.numScreens; ++i)
		{
			if (event->xclient.message_type
			        == wmGD.Screens[i].fwsIconAtom)
			{
				window = event->xclient.data.l[0];
				id = event->xclient.data.l[1];
				UpdateReservation (id, window);
				break;
			}
		}
		break;

	case PropertyNotify:

		/*
		 * FWS_NEXT_ICON PropertyNotify - a client has read and
		 * deleted the FWS_NEXT_ICON property, so that another should
		 * be reserved.
		 */

		for (i = 0; i < wmGD.numScreens; ++i)
		{
			if (event->xproperty.atom == wmGD.Screens[i].fwsIconAtom
			    && event->xproperty.state == PropertyDelete)
			{
				if (pendingNotifies == 0)
				{
					pWS = wmGD.Screens[i].pActiveWS;
					FwsUnreserveIconSpace(pWS,&pWS->IPData);
					FwsReserveIconSpace (pWS, &pWS->IPData);
				}
				else
					--pendingNotifies;
				break;
			}
		}
		break;
	}
}


/*************************************<->*************************************
 *
 *  FwsUnmapClient()
 *
 *  Called to move an FWS client window off screen due to change in workspace.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void FwsUnmapClient (pCD)
	ClientData *pCD;

#else
void FwsUnmapClient (ClientData *pCD)

#endif /* _NO_PROTO */
{
	if (pCD->fwsClient && pCD->fwsChangeState)
		SendClientMsg (pCD->client, FWS_STATE_CHANGE,
			       FWS_UNSEEN_STATE, CurrentTime, NULL, 0);
}



/*************************************<->*************************************
 *
 *  FwsMapClient()
 *
 *  Called to restore an FWS client window off screen due to change in
 *  workspace.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void FwsMapClient (pCD)
	ClientData *pCD;

#else
void FwsMapClient (ClientData *pCD)

#endif /* _NO_PROTO */
{
	if (pCD->fwsClient && pCD->fwsChangeState)
		SendClientMsg (pCD->client, FWS_STATE_CHANGE,
			       NormalState, CurrentTime, NULL, 0);
}



/*************************************<->*************************************
 *
 *  FwsHandleConfigureNotify()
 *
 *  Process a ConfigureNotify event deliviered to an FWS (override-redirect)
 *  window.  This is the first indication we get that the client has changed
 *  the window position or stacking order.  We need to make sure that our
 *  internal data structures match.
 *
 * Race conditions exist because both this window manager and FWS clients may
 * have changed the stacking order at essentially the same time.  We try to
 * compensate for this by examining the server's up to date stacking order.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void FwsHandleConfigureNotify (pCD, event)
	ClientData *pCD;
	XConfigureEvent *event;
 
#else /* _NO_PROTO */
static void FwsHandleConfigureNotify (ClientData *pCD,
				      XConfigureEvent *event)
			       
#endif /* _NO_PROTO */
{
	ClientListEntry *pCLE;
	ClientListEntry *pOtherCLE;

	pCD->clientX = event->x;
	pCD->clientY = event->y;
	pCD->clientWidth = event->width;
	pCD->clientHeight = event->height;
	pCD->xBorderWidth = event->border_width;

	for (pCLE = ACTIVE_PSD->clientList;
	     pCLE != NULL;
	     pCLE = pCLE->nextSibling)
		if (pCLE->pCD == pCD)
			break;

	if (pCLE == NULL)
		return;

	/*
	 * If this is a movement to the top or to the bottom of the list,
	 * handle it directly.  Note that the FwsCommWindow window marks the
	 * lowest spot on the stack a window should reach.
	 */

	if (event->above == None)
	{
		MoveEntryInList (ACTIVE_WS, pCLE, True, NULL);
		return;
	}

	if (event->above == FwsCommWindow)
	{
		MoveEntryInList (ACTIVE_WS, pCLE, False, NULL);
		return;
	}

	/*
	 * Find the 'above' window in the window list.  If not found, then
	 * reorder the list the hardway in FixClientList().
	 */

	for (pOtherCLE = ACTIVE_PSD->clientList;
	     pOtherCLE != NULL;
	     pOtherCLE = pOtherCLE->nextSibling)
		if (pOtherCLE->pCD->clientFrameWin == event->above)
			break;

	if (pOtherCLE == NULL)
	{
		FixClientList (event->window);
		return;
	}

	MoveEntryInList (ACTIVE_WS, pCLE, True, pOtherCLE);
}


/*************************************<->*************************************
 * FwsDestroyWindow -
 *
 * Cleans up after FWS windows which have been destroyed.
 *
 *************************************<->*************************************/

#ifdef _NO_PROTO
void FwsDestroyWindow (window)
	Window window;
#else
void FwsDestroyWindow (Window window)
#endif /* _NO_PROTO */
{
	int          i;
	WmWorkspaceData *pWS;

	--numClients;
	FwsRemoveReservation (window);
	if (numClients == 0)
	{
		for (i = 0; i < wmGD.numScreens; ++i)
		{
			pWS = wmGD.Screens[i].pActiveWS;
			FwsUnreserveIconSpace (pWS, &pWS->IPData);
		}
	}
}


/*************************************<->*************************************
 * HandleEventsOnFwsWindows()
 *
 * Handle event processing for events delivier to FWS client windows.  If this
 * is an FWS client, then there are several kinds of events which need to be
 * handled in differing ways.
 *
 * - Events which are to be handled exclusively by the FWS client (eg. keys,
 * buttons, etc.) and ignored by us.
 *
 * - Events which require special processing to keep our information up to
 * date (eg. ConfigureNotify).
 *
 * - Events normally associated with a client window which should be process
 * as such.
 *
 * - Events normally associated with a client frame window.  (But since there
 * is no separate client frame window, we must avoid getting confused.
 *
 *************************************<->*************************************/

#ifdef _NO_PROTO
Boolean HandleEventsOnFwsWindow (pCD, event)
	ClientData *pCD;
	XEvent *event;
 
#else /* _NO_PROTO */
Boolean HandleEventsOnFwsWindow (ClientData *pCD,
				 XEvent *event)
			       
#endif /* _NO_PROTO */
{
	switch (event->type)
	{
		/*
		 * Case 1 - do nothing.
		 */

	case KeyPress:
	case KeyRelease:
	case MotionNotify:
	case UnmapNotify:		/* don't unmanage FWS clients. */
		break;

		/*
		 * Case 2 - For FWS client windows which are
		 * override-redirect, this is where we find out that the
		 * window moved or changing is place in the stacking
		 * order.
		 */

	case ConfigureNotify:
		FwsHandleConfigureNotify (pCD, &event->xconfigure);
		break;
		
		/*
		 * Case 3 and 4 - event can happen to either client or
		 * client frame with same result.
		 */

	case ColormapNotify:
	case ClientMessage:

		/*
		 * Case 3 - Events normally associated with a client window.
		 */

	case PropertyNotify:
	default:	/* (includes extension events, eg. shapes) */
		
		HandleEventsOnClientWindow (pCD, event);
		break;

		/*
		 * Case 4 - Events normally associate with a frame window.
		 */

	case ButtonPress:
	case ButtonRelease:
	case Expose:
	case EnterNotify:
	case LeaveNotify:
	case FocusIn:
	case FocusOut:
	case MapRequest:
	case ConfigureRequest:
	case ReparentNotify:
		HandleEventsOnOtherWindow (pCD, event);
		break;

		/*
		 * Case 5 - On destroy, we need to release icon space if
		 * reserved for this window.
		 */
	case DestroyNotify:
		FwsDestroyWindow (event->xdestroywindow.window);
		HandleEventsOnOtherWindow (pCD, event);
		break;
	}
	return False;
}


/*************************************<->*************************************
 * IsFwsClient()
 *
 * Determine whether a given X window is an FWS client window.  The decision
 * is made based on the WM_PROTOCOL properties having one of the FWS specific
 * atoms.
 *
 *************************************<->*************************************/
 
#ifdef _NO_PROTO
Boolean IsFwsClient (client)
	Window client;
#else /* _NO_PROTO */
Boolean IsFwsClient (Window client)
#endif /* _NO_PROTO */
{
	Atom          actualType;
	int           actualFormat;
	unsigned long nitems;
	unsigned long leftover;
	Atom          *property;
	int           i;

	if (XGetWindowProperty (DISPLAY, client, wmGD.xa_WM_PROTOCOLS, 0L,
				(long)MAX_CLIENT_PROTOCOL_COUNT, False,
				AnyPropertyType, &actualType,
				&actualFormat, &nitems, &leftover,
				(unsigned char **)&property) != Success)
		return False;

	for (i = 0; i < nitems; ++i)
	{
		if (property[i] == FWS_CLIENT)
		{
			XFree (property);
			return True;
		}
	}
	XFree (property);
	return False;
}


/*************************************<->*************************************
 *
 *  FwsClearClientInfo ()
 *
 *
 *  Description:
 *  -----------
 *  Initialize FWS client information as if the window is NOT an FWS client.
 *
 *
 *  Inputs:
 *  ------
 *  none.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void FwsClearClientInfo (pCD)
	ClientData *pCD;
 
#else /* _NO_PROTO */
void FwsClearClientInfo (ClientData *pCD)
#endif /* _NO_PROTO */
{
	pCD->fwsClient = False;
	pCD->fwsAllowStacking = False;
	pCD->fwsChangeState = False;
	pCD->fwsPassInput = False;

}

/*************************************<->*************************************
 *
 *  FwsClientInfo ()
 *
 *  Get FWS client information, added it to the client data structure.
 *  (This is the data from the client properties, in particular.)
 *
 *  This is called once per window when initially managed with new = False.
 *  It may be called afterwards with new = True if an FWS client
 *  re-registers itself to change its protocol properties.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void FwsClientInfo (pCD, new)
	ClientData *pCD;
	Bool       new;

#else /* _NO_PROTO */
void FwsClientInfo (ClientData *pCD,
		    Bool       new)
#endif /* _NO_PROTO */
{
	int	      i;
	Window        window;
	Atom	      *protos;
	WmWorkspaceData *pWS;

	FwsClearClientInfo (pCD);
	window = pCD->client;
	protos = pCD->clientProtocols;
	for (i = 0; i < pCD->clientProtocolCount; ++i)
	{
		if (protos[i] == FWS_CLIENT)
			pCD->fwsClient = True;

		else if (protos[i] == FWS_CONFIG_STACKING)
			pCD->fwsAllowStacking = True;

		else if (protos[i] == FWS_STATE_CHANGE)
			pCD->fwsChangeState = True;

		else if (protos[i] == FWS_PASS_ALL_INPUT)
			pCD->fwsPassInput = True;
	}
	pCD->clientFrameWin = window;

	if (!new && !pCD->fwsClient)
	{
		/*
		 * This was an FWS client which is no longer an FWS client.
		 * Get rid of its icon reservations.
		 */
		FwsDestroyWindow (window);
	}
	else if (new && pCD->fwsClient)
	{
		++numClients;
		if (numClients == 1)
		{
			for (i = 0; i < wmGD.numScreens; ++i)
			{
				pWS = wmGD.Screens[i].pActiveWS;
				FwsReserveIconSpace (pWS, &pWS->IPData);
			}
		}
		XSelectInput (DISPLAY, window, EnterWindowMask | 
			      LeaveWindowMask | FocusChangeMask |
			      PropertyChangeMask);
	}
}


/*************************************<->*************************************
 *
 *  FwsReserveIconSpace (IconPlacementData *pIPD)
 *
 *
 *  Description:
 *  -----------
 *  Find an icon slot and reserve it for a potential FWS client.
 *
 *
 *  Inputs:
 *  ------
 *  Pointer to the icon placement data.
 *
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void FwsReserveIconSpace (pWS, pIPD)
	WmWorkspaceData *pWS;
	IconPlacementData *pIPD;

#else /* _NO_PROTO */
void FwsReserveIconSpace (WmWorkspaceData *pWS,
			  IconPlacementData *pIPD)
#endif /* _NO_PROTO */
{
	NextIcon     nextIcon;
	int          i;

	/*
	 * If this icon data is not for an icon box, and is for the active
	 * workspace and icons are on the root window and no icon is reserved,
	 * then do the job.
	 */

	if (numClients > 0
	    && pWS != NULL
	    && pWS->pSD->pActiveWS == pWS
	    && pIPD->onRootWindow
	    && pIPD->fwsNextIcon == NO_ICON_PLACE)
	{

		for (i = 0; i < pIPD->totalPlaces; i++)
			if (pIPD->placeList[i].pCD == (ClientData *)NULL)
				break;

		if (i < pIPD->totalPlaces)
		{
			pIPD->fwsNextIcon = i;
			if (pIPD->fwsNextIcon != NO_ICON_PLACE)
			{
				CvtIconPlaceToPosition (pIPD,
							pIPD->fwsNextIcon,
							&nextIcon.x,
							&nextIcon.y);
				nextIcon.width = pIPD->iPlaceW;
				nextIcon.height = pIPD->iPlaceH;
				nextIcon.id = NewReservation (pWS,
							  pIPD->fwsNextIcon);
				XChangeProperty (DISPLAY, FwsCommWindow,
						 pWS->pSD->fwsIconAtom,
						 XA_INTEGER, 32,
						 PropModeReplace,
						 (unsigned char *)&nextIcon,
						 sizeof (nextIcon) / 4);
				++spacesReserved;
			}
		}
	}
}


/*************************************<->*************************************
 *
 *  FwsUnreserveIconSpace ()
 *
 *  Delete the current FWS icon reservation so that the space can be used
 *  by the window manager for some other window icon.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void FwsUnreserveIconSpace (pWS, pIPD)
	WmWorkspaceData *pWS;
	IconPlacementData *pIPD;

#else /* _NO_PROTO */
void FwsUnreserveIconSpace (WmWorkspaceData *pWS,
			    IconPlacementData *pIPD)
#endif /* _NO_PROTO */
{
	NextIcon     *nextIcon;
	Atom         actualType;
	int          actualFormat;
	unsigned long nItems;
	unsigned long leftover;
	int          place;

	place = pIPD->fwsNextIcon;
	if (place != NO_ICON_PLACE && spacesReserved > 0)
	{

		/*
		 * Remove the property be doing a Get/Delete.  If this fails,
		 * then some FWS client has claimed the reserved space and
		 * deleted the property.  Otherwise we mark the reserved space
		 * as available.
		 */

		if (XGetWindowProperty (DISPLAY, FwsCommWindow,
					pWS->pSD->fwsIconAtom,
					0L, sizeof (NextIcon) / 4, True,
					AnyPropertyType, &actualType,
					&actualFormat, &nItems, &leftover,
					(unsigned char **) &nextIcon)
		    != Success)
		{
			nItems = 0;
		}
		if (nItems == 0)
		{
			pIPD->placeList[place].pCD = fwsReserved;
		}
		else
		{
			++pendingNotifies;
			RemoveEmptyReservation (pWS, place);
		}

		if (nextIcon != NULL)
			XFree ((char *) nextIcon);

		pIPD->fwsNextIcon = NO_ICON_PLACE;
		--spacesReserved;
	}
}


/*************************************<->*************************************
 * FwsLowerClientWindow() -
 *
 * Lower the FWS_COMM_WINDOW window (to the bottom).  The backdrop windows
 * should be lowered after this window, so that this windows marks the lowest
 * point in the stacking order an FWS client window should reach.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void FwsLowerClientWindow ()

#else /* _NO_PROTO */
void FwsLowerClientWindow (void)
#endif /* _NO_PROTO */
{
	XLowerWindow (DISPLAY, FwsCommWindow);
}


/*************************************<->*************************************
 *
 *  FwsInitializeWindow ()
 *
 *
 *  Description:
 *  -----------
 *  This function initializes some of FWS components of the window manager,
 *  including creating a window for communications purposes, etc.
 *
 *
 *  Inputs:
 *  ------
 *  none.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void FwsInitializeWindow ()

#else /* _NO_PROTO */
void FwsInitializeWindow (void)
#endif /* _NO_PROTO */
{
	static Arg args[] = {
		{ XtNheight, 10 },
		{ XtNwidth,  10 },
		{ XtNname,   (XtArgVal) "FWS_COMM_WINDOW" }
	};

	FWS_CLIENT          = XInternAtom(DISPLAY, "_SUN_FWS_CLIENT", False);
	FWS_COMM_WINDOW     = XInternAtom(DISPLAY, "_SUN_FWS_COMM_WINDOW", False);
	FWS_PROTOCOLS       = XInternAtom(DISPLAY, "_SUN_FWS_PROTOCOLS", False);
	FWS_REGISTER_WINDOW = XInternAtom(DISPLAY, "_SUN_FWS_REGISTER_WINDOW",False);
	FWS_UNSEEN_STATE    = XInternAtom(DISPLAY, "_SUN_FWS_UNSEEN_STATE", False);
	FWS_NORMAL_STATE    = XInternAtom(DISPLAY, "_SUN_FWS_NORMAL_STATE", False);
	FWS_CONFIG_REQUEST  = XInternAtom(DISPLAY,
					    "_SUN_FWS_CONFIG_REQUEST",False);
	FWS_CONFIG_SIZE_POSITION = XInternAtom(DISPLAY,
					    "_SUN_FWS_CONFIG_SIZE_POSITION",False);
	FWS_CONFIG_STACKING = XInternAtom(DISPLAY, "_SUN_FWS_CONFIG_STACKING",False);
	FWS_STATE_CHANGE    = XInternAtom(DISPLAY, "_SUN_FWS_STATE_CHANGE",False);
	FWS_PASS_ALL_INPUT  = XInternAtom(DISPLAY, "_SUN_FWS_PASS_ALL_INPUT",False);
	FWS_STACK_UNDER     = XInternAtom(DISPLAY, "_SUN_FWS_STACK_UNDER",False);
	FWS_PARK_ICONS      = XInternAtom(DISPLAY, "_SUN_FWS_PARK_ICONS", False);
	FWS_HANDLES_FOCUS   = XInternAtom(DISPLAY, "_SUN_FWS_HANDLES_FOCUS", False);
	FWS_PASSES_INPUT   = XInternAtom(DISPLAY, "_SUN_FWS_PASSES_INPUT", False);

	FwsCommWidget = XtCreateWidget ("FwsClientWindow",
					overrideShellWidgetClass,
					wmGD.topLevelW, args,
					sizeof args / sizeof args[0]);
	XtRealizeWidget (FwsCommWidget);
	XSync (DISPLAY, False);
	FwsCommWindow = XtWindow (FwsCommWidget);
	XtAddEventHandler (FwsCommWidget, PropertyChangeMask, TRUE,
			   (XtEventHandler) FwsCommProc, NULL);
}


/*************************************<->*************************************
 *
 *  FwsInitializeProperties ()
 *
 *
 *  Description:
 *  -----------
 *  This function initializes the rest of the FWS component of the window
 *  manager including adding properties to the root windows.
 *
 *
 *  Inputs:
 *  ------
 *  none.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void FwsInitializeProperties ()

#else /* _NO_PROTO */
void FwsInitializeProperties (void)
#endif /* _NO_PROTO */
{
	int	i;
	Window	root;
	WmScreenData *pSD;
	Atom    atomList[10];
	int     count;
	char    propName[30];

	for (i = 0; i < wmGD.numScreens; ++i)
	{
		pSD = &wmGD.Screens[i];
		sprintf (propName, "_SUN_FWS_NEXT_ICON_%d", i);
		pSD->fwsIconAtom = XInternAtom (DISPLAY, propName, False);
		count = 0;
		atomList[count++] = FWS_STACK_UNDER;
		atomList[count++] = FWS_HANDLES_FOCUS;
		atomList[count++] = FWS_PASSES_INPUT;
		if (!pSD->useIconBox)
		{
			atomList[count++] = FWS_PARK_ICONS;
		}

		root = pSD->rootWindow;
		XChangeProperty (DISPLAY, root, FWS_COMM_WINDOW,
				 XA_WINDOW, 32, PropModeReplace,
				 (unsigned char *) &FwsCommWindow, 1);
		XChangeProperty (DISPLAY, root, FWS_PROTOCOLS,
				 XA_ATOM, 32, PropModeReplace,
				 (unsigned char *) atomList, count);
	}
}


/*************************************<->*************************************
 *
 *  FwsQuit()
 *
 * Cleanly terminate the FWS operations by deleting the root properties we
 * created.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
extern void FwsQuit()
#else
extern void FwsQuit (void)
#endif /* _NO_PROTO */
{
	int	i;
	Window	root;

	for (i = 0; i < wmGD.numScreens; ++i)
	{
		root = wmGD.Screens[i].rootWindow;
		XDeleteProperty (DISPLAY, root, FWS_COMM_WINDOW);
		XDeleteProperty (DISPLAY, root, FWS_PROTOCOLS);
	}
}


/***********************************************************************
 *              DEBUGGING ROUTINES.
 **********************************************************************/

/*
 * Print the current client list.
 * Meant to be called from the debugger.
 */

#ifdef FWS_DEBUG
#ifdef _NO_PROTO
void FwsPrint()
#else
void FwsPrint (void)
#endif /* _NO_PROTO */
{
	ClientListEntry *pCLE;
	char            *name;

	for (pCLE = ACTIVE_PSD->clientList;
	     pCLE != NULL;
	     pCLE = pCLE->nextSibling)
	{
		name = pCLE->pCD->clientName;
		if (name == NULL)
			name = "(noName)";
		printf ("CLE 0x%x, type %d, CD 0x%x, win/frame 0x%x/0x%x, %s(%s)  \n",
			pCLE, pCLE->type, pCLE->pCD,
			pCLE->pCD->client,
			pCLE->pCD->clientFrameWin,
			pCLE->pCD->fwsClient ? "Fws, " : "",
			name);
	}
	printf ("lastClient = 0x%x.\n", ACTIVE_PSD->lastClient);
}


/*
 * Print information about a given window.
 * Meant to be called from a debugger.
 */

#ifdef _NO_PROTO
void FwsWindow (window)
	Window window;
#else
void FwsWindow (Window window)
#endif /* _NO_PROTO */
{
	ClientData *pCD;

	if (XFindContext (DISPLAY, window, wmGD.windowContextType,
			  (XPointer *) &pCD))
	{
		printf ("Window 0x%x has no context.\n", window);
	}
	else
	{
		printf ("Window 0x%x, pCD = 0x%x, fws = %d.\n",
			window, pCD, pCD->fwsClient);
	}
}


#endif /* FWS_DEBUG */

#else
static int fwsDummy;	/* avoid "empty translation unit" warning */
#endif /* FWS */
