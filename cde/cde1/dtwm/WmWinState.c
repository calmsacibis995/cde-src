/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmWinState.c /main/cde1_maint/2 1995/10/09 13:45:59 pascale $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmICCC.h"
#include "WmProtocol.h"

#ifdef AUTOMATION
#include <stdio.h>
#include <math.h>
#include <Xm/ScrollBarP.h>
#endif /* AUTOMATION */

/*
 * include extern functions
 */

#include "WmCDecor.h"
#include "WmFunction.h"
#include "WmIDecor.h"
#include "WmIPlace.h"
#include "WmIconBox.h"
#include "WmKeyFocus.h"
#ifdef PANELIST
#include "WmPanelP.h"  /* for typedef in WmManage.h */
#endif /* PANELIST */
#include "WmManage.h"
#include "WmProperty.h"
#include "WmWinInfo.h"
#include "WmWinList.h"
#ifdef WSM
#include "WmWrkspace.h"
#endif /* WSM */


/*
 * Function Declarations:
 */

#include "WmWinState.h"
#ifdef FWS
#include "WmFws.h"
#endif /* FWS */

#ifdef _NO_PROTO
#ifdef PANELIST
static void SlideWindowOut ();
#endif /* PANELIST */
static void UnmapClients ();
static void SetupWindowStateWithEventMask ();
#else /* _NO_PROTO */
#ifdef PANELIST
static void SlideWindowOut (ClientData *pCD);
#endif /* PANELIST */
static void UnmapClients (ClientData *pCD, unsigned int event_mask);
static void SetupWindowStateWithEventMask (ClientData *pCD, int newState, Time setTime, unsigned int event_mask);
#endif /* _NO_PROTO */

#ifdef AUTOMATION
#ifdef _NO_PROTO
static void SetMwmIconInfo();
static void FillIconBoxInfo();
#else
static void SetMwmIconInfo(ClientData *pcd);
static void FillIconBoxInfo(ClientData *pcd,
                            PropMwmFrameIconInfo *frame_icon_prop);
#endif /* _NO_PROTO */
#endif /* AUTOMATION */


/*
 * Global Variables:
 */
extern int firstTime;


/******************************<->*************************************
 *
 *  SetClientState (pCD, newState, setTime)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to change the state of a client window (between
 *  withdrawn, normal, minimized, maximized).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = This is a pointer to the window data for the window that
 *        is to have its state changed. The fields that are used
 *        are clientState, ...
 *
 *  newState = This is the state that the client window is to be changed to.
 *
 *  setTime = timestamp for state setting operations
 *
 * 
 *  Outputs:
 *  -------
 *  pCD.clientState = new client state
 *
 ******************************<->***********************************/

#ifdef _NO_PROTO
void SetClientState (pCD, newState, setTime)
	ClientData *pCD;
	int newState;
	Time setTime;

#else /* _NO_PROTO */
void SetClientState (ClientData *pCD, int newState, Time setTime)
#endif /* _NO_PROTO */
{
	SetClientStateWithEventMask(pCD, newState, setTime, (unsigned int)0);
} /* END OF FUNCTION SetClientState */

#ifdef _NO_PROTO
void SetClientStateWithEventMask (pCD, newState, setTime, event_mask)
	ClientData *pCD;
	int newState;
	Time setTime;
	unsigned int event_mask;
#else /* _NO_PROTO */
void SetClientStateWithEventMask (ClientData *pCD, int newState, Time setTime, unsigned int event_mask)
#endif /* _NO_PROTO */
{
    ClientData *pcdLeader;
    int currentState;
    WmScreenData *pSD = PSD_FOR_CLIENT(pCD);
#ifdef WSM
    Boolean notShowing = (newState & UNSEEN_STATE);
#endif /* WSM */

    currentState = pCD->clientState;
    if (currentState == newState)
    {
	/* no change in state */
	return;
    }


    /*
     * Undo the old state and setup the new state.  If this is a transient
     * window then insure that it is put in a state that is compatible
     * with its transient leader (e.g., it cannot be minimized separately).
     */

    pcdLeader = (pCD->transientLeader) ? FindTransientTreeLeader (pCD) : pCD;
#ifdef WSM
    SetClientWsIndex (pCD);
#endif /* WSM */

    if (pCD->transientLeader)
    {
	if ((pcdLeader->clientState == MINIMIZED_STATE) &&
	    (newState != WITHDRAWN_STATE))
	{
	    newState = MINIMIZED_STATE;
#ifdef WSM
	    if (notShowing)
	    {
		newState |= UNSEEN_STATE;
	    }
#endif /* WSM */
	}
	else if ((newState == MINIMIZED_STATE) &&
		 (pcdLeader->clientState != MINIMIZED_STATE))
	{
	    if (currentState == WITHDRAWN_STATE)
	    {
		newState = NORMAL_STATE;
#ifdef WSM
	    if (notShowing)
	    {
		newState |= UNSEEN_STATE;
	    }
#endif /* WSM */
	    }
	    else
	    {
		newState = currentState;
#ifdef WSM
	    if (notShowing)
	    {
		newState |= UNSEEN_STATE;
	    }
#endif /* WSM */
	    }
	}
	if (newState == currentState)
	{
	    return;
	}
    }

    switch (newState)
    {

#ifdef WSM
        case UNSEEN_STATE | WITHDRAWN_STATE:
#else
	case WITHDRAWN_STATE:
#endif /* WSM */
	{
	    /*
	     * Free window manager resources (frame and icon).  The
	     * WM_STATE property is set in WithdrawWindow.
	     */

	    UnManageWindow (pCD);
	    break;
	}

	case NORMAL_STATE:
	case MAXIMIZED_STATE:
	{
	    SetupWindowStateWithEventMask (pCD, newState, setTime, event_mask);
#ifdef WSM
#ifdef FWS
	    if (!pCD->fwsClient)
	    {
#endif /* FWS */
	    XMapWindow (DISPLAY, pCD->client);
	    XMapWindow (DISPLAY, pCD->clientFrameWin);
#ifdef FWS
	    }
#endif /* FWS */
#if defined(PANELIST)
            WmStopWaiting();   /* in WmIPC.c */
#endif /* PANELIST */
#endif /* WSM */
	    break;
	}

	case MINIMIZED_STATE:
	{
	    Boolean clientHasFocus;

	    /*
	     * Transient windows are minimized with the rest of the transient
	     * tree, including the transient leader.
	     */

	    if ((pCD->clientState == NORMAL_STATE) ||
		(pCD->clientState == MAXIMIZED_STATE))
	    {
		if ((wmGD.keyboardFocus == pCD) ||
		    (pCD->transientChildren && wmGD.keyboardFocus &&
		     (pCD == FindTransientTreeLeader (wmGD.keyboardFocus))))
		{
		    clientHasFocus = True;
		}
		else
		{
		    clientHasFocus = False;
		}

		if (clientHasFocus ||
		  ((wmGD.nextKeyboardFocus == pCD) ||
		   (pCD->transientChildren && wmGD.keyboardFocus &&
		    (pCD == FindTransientTreeLeader (wmGD.nextKeyboardFocus)))))
	    	{
		    /*
		     * Give up the keyboard focus when minimized (including
		     * the case in which an associated transient window has
		     * the focus).  Immediately remove the focus indication
		     * from the window being minimized.
		     */

		    if (wmGD.autoKeyFocus &&
			(wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT))
		    {
			AutoResetKeyFocus (pcdLeader, setTime);
		    }
		    else
		    {
		        Do_Focus_Key (NULL, setTime, 
				ALWAYS_SET_FOCUS | WORKSPACE_IF_NULL);
		    }

		    if (clientHasFocus)
		    {
			SetKeyboardFocus (NULL, 0);
		    }
		}

		/* unmap main client and all transients */
		UnmapClients (pCD, event_mask);
	    }

	    /*
	     * Display the icon for the minimized client.
	     */

	    if (ICON_FRAME_WIN(pCD)) 
	    {
#ifdef WSM
		if (pCD->clientState & UNSEEN_STATE)
		{
		    if (pCD->iconWindow)
		    {
			XMapWindow (DISPLAY, pCD->iconWindow);
		    }
		    XMapWindow (DISPLAY, ICON_FRAME_WIN(pCD));
		}

		ShowAllIconsForMinimizedClient (pCD);
#else /* WSM */
		ShowIconForMinimizedClient (pSD->pActiveWS, pCD);
#endif /* WSM */
	    }

	    SetClientWMState (pCD, IconicState, MINIMIZED_STATE);

	    if ((pSD->useIconBox) && P_ICON_BOX(pCD))
	    {
		if ((pCD->clientFlags & ICON_BOX) && ACTIVE_ICON_TEXT_WIN)
		{
		    /*
		     * Hide active icon text window and reparent it to
		     * root
		     */
		    HideActiveIconText((WmScreenData *)NULL);
		    pSD->activeLabelParent = ACTIVE_ROOT;
		    XReparentWindow(DISPLAY, ACTIVE_ICON_TEXT_WIN , 
				ACTIVE_ROOT, 0, 0 );
		}
		if (ICON_FRAME_WIN(pCD))
		{
		    /* 
		     * force icon appearance in icon box to change 
		     */
		    IconExposureProc (pCD, True);
		}
	    }
	    break;
	}

#ifdef WSM 

        case UNSEEN_STATE | NORMAL_STATE:
        case UNSEEN_STATE | MAXIMIZED_STATE:
        case UNSEEN_STATE | MINIMIZED_STATE:
	{
	    if (wmGD.keyboardFocus == pCD)
	    {
		/*
		 * Give up the keyboard focus 
		 */
		Do_Focus_Key ((ClientData *)NULL, 
			CurrentTime, ALWAYS_SET_FOCUS);
		SetKeyboardFocus (NULL, 0);
	    }

	    if (!(pCD->clientState & UNSEEN_STATE) &&
		 (((pCD->clientState & ~UNSEEN_STATE) == NORMAL_STATE) ||
		  ((pCD->clientState & ~UNSEEN_STATE) == MAXIMIZED_STATE)))
	    {
#ifdef FWS
		if (pCD->fwsClient)
		{
			pCD->wmUnmapCount++;
			FwsUnmapClient (pCD);
		}
		else
#endif /* FWS */
		/* unmap main client and all transients */
		UnmapClients (pcdLeader, event_mask);

	    }
      
	    if (pCD->clientFrameWin) 
	    {
		if (!P_ICON_BOX(pCD))
		{
		    if (ICON_FRAME_WIN(pCD))
		    {
			XUnmapWindow (DISPLAY, ICON_FRAME_WIN(pCD));
		    }
		    if (pCD->iconWindow)
			XUnmapWindow (DISPLAY, pCD->iconWindow);
		} 
	    }

	    switch (newState & ~UNSEEN_STATE)
	    {
	    case MINIMIZED_STATE:
		SetClientWMState (pCD, IconicState, newState);
		break;

	    case NORMAL_STATE:
	    case MAXIMIZED_STATE:
	    default:
		SetClientWMState (pCD, NormalState, newState);
		break;
	    }
        }
	break;
#endif /* WSM */
    }
#ifdef AUTOMATION
	SetMwmIconInfo(pCD);
#endif

} /* END OF FUNCTION SetClientStateWithEventMask */



/*************************************<->*************************************
 *
 *  SetupWindowStateWithEventMask (pCD, newState, setTime, event_mask)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to setup a client window in the Normal or Maximized
 *  state.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = This is a pointer to the window data for the window that
 *        is to have its state changed.
 *
 *  newState = This is the state that the client window is to be changed to.
 *
 *  setTime = timestamp for state setting operations
 *
 *  event_mask = what to grab to prevent stray events going somewhere
 * 
 *  Outputs:
 *  -------
 *  pCD.clientState = new client state
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void SetupWindowStateWithEventMask (pCD, newState, setTime, event_mask)
	ClientData *pCD;
	int newState;
	Time setTime;
	unsigned int event_mask;
#else /* _NO_PROTO */
static void SetupWindowStateWithEventMask (ClientData *pCD, int newState, 
	Time setTime, unsigned int event_mask)
#endif /* _NO_PROTO */
{
    int currentState;
#ifdef WSM
    int wsI, iplace;
    WmWorkspaceData *pWS_i;
#else /* WSM */
    WmWorkspaceData *pWS = PSD_FOR_CLIENT(pCD)->pActiveWS;
#endif /* WSM */
    WmScreenData *pSD = PSD_FOR_CLIENT(pCD);

    currentState = pCD->clientState;

    /*
     * A transient window is not restored or maximized if the transient leader
     * is minimized.
     */

    if (newState == NORMAL_STATE)
    {
	if (pCD->maxConfig == True)
	{
	    /*
	     * The configuration function uses maxConfig to determine
	     * what the current configuration is (and then resets
	     * maxConfig) and uses the state paramenter to determine
	     * what the new configuration is.
	     */

	    ConfigureNewState (pCD); 
	}
    }
    else /* MAXIMIZED_STATE */
    {
	if (pCD->maxConfig == False)
	{
	    ConfigureNewState (pCD); 
        }
    }

    if (currentState == MINIMIZED_STATE)
    {
	Boolean clearIconFocus;

	/*
	 * give up keyboard focus 
	 */

	if ((wmGD.keyboardFocus == pCD) ||
	    (wmGD.nextKeyboardFocus == pCD))
	{
	    Do_Focus_Key (NULL, setTime, ALWAYS_SET_FOCUS | WORKSPACE_IF_NULL);
	}

	if (wmGD.keyboardFocus == pCD)
	{
	    clearIconFocus = True;
	}
	else
	{
	    clearIconFocus = False;
	}

	/*
	 * The wm icon frame window and the client icon window
	 * (if it is being used) are mapped and the client window and
	 * client frame are unmapped.
	 */

	if (ICON_FRAME_WIN(pCD))
	{
	    if (pSD->useIconBox && P_ICON_BOX(pCD) && 
		!(pCD->clientFlags & ICON_BOX))
	    {
	        ShowClientIconState(pCD, newState);
	    }
	    else 
	    {
		Boolean doGrab = False;
    		if (event_mask)
		doGrab = (Success == XGrabPointer 
			(DISPLAY, DefaultRootWindow(DISPLAY),
			False, event_mask, GrabModeAsync, GrabModeAsync,
			None, None, CurrentTime));
	        XUnmapWindow (DISPLAY, ICON_FRAME_WIN(pCD));
	        if (pCD->iconWindow)
	        {
		    XUnmapWindow (DISPLAY, pCD->iconWindow);
	        }
    		if (event_mask && doGrab)
		{
			XEvent event;
			XMaskEvent(DISPLAY, event_mask, &event);
			XUngrabPointer(DISPLAY,CurrentTime);
		}
#ifdef WSM
	        if (wmGD.iconAutoPlace) 
	        {
                    for (wsI = 0; wsI < pCD->numInhabited; wsI++)
		    {
			iplace = pCD->pWsList[wsI].iconPlace;
			if (iplace != NO_ICON_PLACE)
			{
			    pWS_i = GetWorkspaceData (pCD->pSD,
						pCD->pWsList[wsI].wsID);
			    pWS_i->IPData.placeList[iplace].pCD = 
				    NULL;
			}
		    }
	        }
#else /* WSM */
	        if ((wmGD.iconAutoPlace) && (ICON_PLACE(pCD) != NO_ICON_PLACE))
	        {
		    pWS->IPData.placeList[ICON_PLACE(pCD)].pCD = 
			NULL;
	        }
#endif /* WSM */
	    }

	    if (clearIconFocus)
	    {
		ClearFocusIndication (pCD, False /*no refresh*/);
		wmGD.keyboardFocus = NULL;
	    }
	}
    }
    if ((currentState != NORMAL_STATE) && (currentState != MAXIMIZED_STATE))
    {
	/*
	 * Note that maximized state is considered a NormalState in
	 * the ICCC.  SetClientWMState also sets the state in the
	 * client data.
	 */

	if (currentState == MINIMIZED_STATE)
	{
	    /*
	     * Raise the window(s) when they are deiconified.
	     */

	    pCD->clientState = newState;
#ifdef WSM
		    wmGD.bSuspendSecondaryRestack = True;
#endif /* WSM */
	    F_Raise (NULL, pCD, NULL);
#ifdef WSM
		    wmGD.bSuspendSecondaryRestack = False;
#endif /* WSM */
	}

	if ( (!(pCD->clientFlags & ICON_BOX)) || 
	     ((pCD->clientFlags & ICON_BOX) && (!(firstTime))) )
	{
#ifdef PANELIST
	  if ((currentState == WITHDRAWN_STATE) && 
	      (pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL) &&
	      !(pCD->transientChildren))
	  {
	      if (pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUB_RESTORED)
	      {
		  pCD->dtwmBehaviors &= ~DtWM_BEHAVIOR_SUB_RESTORED;
		  pCD->dtwmBehaviors &= ~DtWM_BEHAVIOR_SUBPANEL;
#ifdef FWS
		  if (pCD->fwsClient)
			  FwsMapClient (pCD);
		  else
		  {
#endif /* FWS */
		  XMapWindow (DISPLAY, pCD->client);
		  XMapWindow (DISPLAY, pCD->clientFrameWin);
#ifdef FWS
		  }
#endif /* FWS */
	      }
	      else
	      {
		  SlideWindowOut (pCD);
	      }
	  }
	  else
#endif /* PANELIST */
#ifdef FWS
	  if (pCD->fwsClient)
		  FwsMapClient (pCD);
	  else
#endif /* FWS */
	    MapClientWindows (pCD);
	}


	/*
	 * Set the WM_STATE property of the window and any associated
	 * transients, along with the clientState value.  The call
	 * is made with an indication of NORMAL_STATE to insure
	 * that transient window clientState values are setup
	 * correctly.  The top-level window clientState is set later.
	 */

	SetClientWMState (pCD, NormalState, NORMAL_STATE);
    }
    pCD->clientState = newState;

    if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT) &&
	(currentState == MINIMIZED_STATE) && wmGD.deiconifyKeyFocus)
    {
	ClientData *pcdFocus;

	pcdFocus = FindTransientFocus (pCD);
	if (pcdFocus)
	{
	    Do_Focus_Key (pcdFocus, setTime, ALWAYS_SET_FOCUS);
	}
    }

    if ( pSD->useIconBox &&  P_ICON_BOX(pCD) &&
	 (!(pCD->clientFlags & ICON_BOX)) && (ICON_FRAME_WIN(pCD)))
    {
	/* 
	 * force icon appearance in icon box to change 
	 */

	IconExposureProc (pCD, True);
    }

} /* END OF FUNCTION SetupWindowStateWithEventMask */




/*************************************<->*************************************
 *
 *  ConfigureNewState (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Configure the window to a new state
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o This is only good for going between NORMAL and MAXIMIZED state.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ConfigureNewState (pcd)
	ClientData *pcd;

#else /* _NO_PROTO */
void ConfigureNewState (ClientData *pcd)
#endif /* _NO_PROTO */
{
    if (pcd->maxConfig)
    {
	pcd->maxConfig = FALSE;
	RegenerateClientFrame(pcd);
	XResizeWindow (DISPLAY, pcd->client,
			   (unsigned int) pcd->clientWidth, 
			   (unsigned int) pcd->clientHeight);
    }
    else
    {
	XResizeWindow (DISPLAY, pcd->client,
			   (unsigned int) pcd->maxWidth, 
			   (unsigned int) pcd->maxHeight);
	pcd->maxConfig = TRUE;
	RegenerateClientFrame(pcd);
    }
    SendConfigureNotify (pcd);

    /*
     * Force repaint if size doesn't change to update frame appearance.
     */

    if ((pcd->clientWidth == pcd->maxWidth) &&
        (pcd->clientHeight == pcd->maxHeight))
    {
	FrameExposureProc (pcd);
    }

} /* END OF FUNCTION ConfigureNewState */



/*************************************<->*************************************
 *
 *  UnmapClients (pCD, event_mask)
 *
 *
 *  Description:
 *  -----------
 *  Unmap the window(s).  The indicated client may be the head of a transient
 *  tree - if it is unmap all windows in the transient tree.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data of window(s) to be unmapped
 *  event_mask = what to grab to prevent stray events going somewhere. Our
 *	passive grab has just been activated -- but it is dropped when the
 * 	window is unmapped and the ButtonRelease event can go to the window
 *	now exposed. Avoid this by grabbing the ButtonRelease before the unmap
 *	and swallowing it.
 *	Also done for icon being unmapped.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void UnmapClients (pCD, event_mask)
	ClientData *pCD;
	unsigned int event_mask;
#else /* _NO_PROTO */
static void UnmapClients (ClientData *pCD, unsigned int event_mask)
#endif /* _NO_PROTO */
{
    ClientData *pNext;
    Boolean doGrab = False;

    pNext = pCD->transientChildren;
    while (pNext)
    {
	/* unmap all children first */
	if (pNext->transientChildren)
	    UnmapClients (pNext, (unsigned int) 0);

	/* then unmap all siblings at this level */
	XUnmapWindow (DISPLAY, pNext->clientFrameWin);
	XUnmapWindow (DISPLAY, pNext->client);
	pNext->wmUnmapCount++;
	pNext = pNext->transientSiblings;
    }

    if (event_mask)
	doGrab = (Success == XGrabPointer (DISPLAY, DefaultRootWindow(DISPLAY),
		False, event_mask, GrabModeAsync, GrabModeAsync,
		None, None, CurrentTime));
    /* unmap this primary window */
    XUnmapWindow (DISPLAY, pCD->clientFrameWin); 
    XUnmapWindow (DISPLAY, pCD->client);
    if (event_mask && doGrab)
	{
	XEvent event;
	XMaskEvent(DISPLAY, event_mask, &event);
	XUngrabPointer(DISPLAY,CurrentTime);
	}
    pCD->wmUnmapCount++;

} /* END OF FUNCTION UnmapClients */



/*************************************<->*************************************
 *
 *  SetClientWMState (pCD, wmState, mwmState)
 *
 *
 *  Description:
 *  -----------
 *  Set a new window manage state for a client window or a tree of transient
 *  client windows.
 *
 *  Inputs:
 *  ------
 *  pCD	= pointer to  client data
 *
 *  wmState = new state for WM_STATE property
 *
 *  mwmState = mwm client state
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void SetClientWMState (pCD, wmState, mwmState)
	ClientData *pCD;
	int wmState;
	int mwmState;

#else /* _NO_PROTO */
void SetClientWMState (ClientData *pCD, int wmState, int mwmState)
#endif /* _NO_PROTO */
{
    ClientData *pNext;
#ifdef WSM
    Boolean bToUnseen;

    bToUnseen = (mwmState & UNSEEN_STATE) != 0;
    mwmState &= ~UNSEEN_STATE;
#endif /* WSM */

#ifdef WSM
    SetClientWsIndex (pCD);
#endif /* WSM */
    pNext = pCD->transientChildren;
    while (pNext)
    {
	if (pNext->transientChildren)
	{
	    SetClientWMState (pNext, wmState, mwmState);
	}

#ifdef WSM
        SetClientWsIndex (pNext);
#endif /* WSM */
	SetWMState (pNext->client, wmState, ICON_FRAME_WIN(pNext));
	if (pNext->maxConfig && mwmState == NORMAL_STATE)
	{
	    pNext->clientState = MAXIMIZED_STATE;
	}
#ifdef WSM
	else if (!pNext->maxConfig && mwmState == MAXIMIZED_STATE)
	{
	    pNext->clientState = NORMAL_STATE;
	}
#endif /* WSM */
	else
	{
	    pNext->clientState = mwmState;
	}
#ifdef WSM
	if (bToUnseen)
	    pNext->clientState |= UNSEEN_STATE;
#endif /* WSM */
	pNext = pNext->transientSiblings;
    }

    SetWMState (pCD->client, wmState, ICON_FRAME_WIN(pCD));
    pCD->clientState = mwmState;
#ifdef WSM
    if (bToUnseen)
	pCD->clientState |= UNSEEN_STATE;
#endif /* WSM */

} /* END OF FUNCTION SetClientWMState */

#ifdef PANELIST

#define SLIDE_UP_PERCENTAGE	5
#define SLIDE_UP_DIVISOR	(100/SLIDE_UP_PERCENTAGE)
#define SLIDE_UP_INTERVAL	15

/******************************<->*************************************
 *
 * void SlideOutTimerProc (client_data, id)
 *
 *  Description:
 *  -----------
 *  An XtTimerCallbackProc to process slide up mapping of a panel
 *
 *  Inputs:
 *  ------
 *  client_data = pointer to a SlideOutRec
 * 
 *  Outputs:
 *  -------
 *  
 *
 *  Comments:
 *  --------
 ******************************<->***********************************/
void
#ifdef _NO_PROTO
SlideOutTimerProc (client_data, id)
    XtPointer client_data;
    XtIntervalId *id;
#else
SlideOutTimerProc ( XtPointer client_data, XtIntervalId *id)
#endif /* _NO_PROTO */
{
    SlideOutRec *pSOR = (SlideOutRec *) client_data;
    Boolean bDone = False;

    if (pSOR)
    {
	/*
	 * compute next increment;
	 */
	switch (pSOR->direction)
	{
	    case SLIDE_NORTH:
		if (pSOR->mapping)
		{
		    pSOR->currY -= pSOR->incHeight;
		    pSOR->currHeight += pSOR->incHeight;
		    if ((pSOR->currY < pSOR->pCD->frameInfo.y) ||
			(pSOR->currHeight > pSOR->pCD->frameInfo.height))
		    {
			pSOR->currY = pSOR->pCD->frameInfo.y;
			pSOR->currHeight = pSOR->pCD->frameInfo.height;
		    }
		    bDone = (pSOR->currY == pSOR->pCD->frameInfo.y);
		}
		else 
		{
		    pSOR->currY += pSOR->incHeight;
		    if (pSOR->incHeight >= pSOR->currHeight)
		    {
			pSOR->currHeight = 0;
			bDone = True;
		    }
		    else
		    {
			pSOR->currHeight -= pSOR->incHeight;
		    }
		}
		break;

	    case SLIDE_SOUTH:
		if (pSOR->mapping)
		{
		    pSOR->currHeight += pSOR->incHeight;
		    if (pSOR->currHeight > pSOR->pCD->frameInfo.height)
		    {
			pSOR->currHeight = pSOR->pCD->frameInfo.height;
		    }
		    bDone = 
		      (pSOR->currHeight == pSOR->pCD->frameInfo.height);
		}
		else
		{
		    if (pSOR->incHeight >= pSOR->currHeight)
		    {
			pSOR->currHeight = 0;
			bDone = True;
		    }
		    else
		    {
			pSOR->currHeight -= pSOR->incHeight;
		    }
		}
		break;
	}

	/*
	 * do next slide-up
	 */
	if (pSOR->currHeight > 0)
	{
	    XMoveResizeWindow (DISPLAY, pSOR->coverWin, 
		pSOR->currX, pSOR->currY,
		pSOR->currWidth, pSOR->currHeight);

	    XMoveResizeWindow (DISPLAY, pSOR->pCD->clientFrameWin, 
	        pSOR->currX, pSOR->currY, 
		pSOR->currWidth, pSOR->currHeight);
	}
	
	/*
	 * See if we need to continue
	 */
	if (bDone)
	{
	    if (!pSOR->mapping)
	    {
		/* Time to really unmanage the slide-up */
		XtUnmanageChild (pSOR->wSubpanel);
	    }
            else
            {
                WmSubpanelPosted (DISPLAY1, pSOR->pCD->client);
                SendConfigureNotify(pSOR->pCD);
            }

	    /* done! clean up */
	    XDestroyWindow (DISPLAY, pSOR->coverWin);
	    pSOR->pCD->pSOR = NULL;
	    XtFree ((char *)pSOR);
	    wmGD.iSlideUpsInProgress -= 1;
	}
	else
	{
	    /* re-arm the timer */
	    XtAppAddTimeOut(wmGD.mwmAppContext, pSOR->interval,
			    SlideOutTimerProc, (XtPointer)pSOR);
	    XSync (DISPLAY, False);
	}
    }

} /* END OF FUNCTION SlideOutTimerProc */



/*************************************<->*************************************
 *
 *  SlideWindowOut (pCD)
 *
 *
 *  Description:
 *  -----------
 *  Maps a window with a slide-out effect.
 *
 *
 *  Inputs:
 *  ------
 *  pCD	= pointer to  client data
 *
 *  Comment:
 *  -------
 *  Only supports slide-up or slide-down 
 * 
 *************************************<->***********************************/

static void
#ifdef _NO_PROTO
SlideWindowOut (pCD)
	ClientData *pCD;

#else /* _NO_PROTO */
SlideWindowOut (ClientData *pCD)
#endif /* _NO_PROTO */
{
    SlideOutRec *pSOR;

    if (pCD->pSOR)
    {
	pSOR = pCD->pSOR;

	/*
	 *  Hmmm. We're already sliding this window.
	 *  If we're supposed to go in the other direction,
	 *  then turn it around.
	 */
	if (pSOR->mapping == True)
	{
	    /*
	     * We're already mapping this guy, ignore this
	     * and finish what we've already got going.
	     */
	    return;
	}
	else
	{
	    /*
	     * We're not mapping this guy. Reverse course!!
	     */
	    pSOR->mapping = True;

	    /* insure the client window is mapped */
	    XMapWindow (DISPLAY, pCD->client);

	    /* handle the rest on the next timeout */
	    return;
	}
    }

    /* map the primary window */
    XMapWindow (DISPLAY, pCD->client);
    pSOR = (SlideOutRec *) XtMalloc (sizeof(SlideOutRec));
    if (pSOR)
    {
	/*
	 * Compute this ahead of time so we can check against
	 * the window size. If the window is short, we'll
	 * just map it, avoiding a lot of processing.
	 */
	pSOR->incHeight = (Dimension) (DisplayHeight(DISPLAY, 
			SCREEN_FOR_CLIENT(pCD))/SLIDE_UP_DIVISOR);
    }

    if ((pCD->slideDirection != SLIDE_NOT) && pSOR &&
	(pSOR->incHeight < pCD->frameInfo.height))
    {
	XSetWindowAttributes window_attribs;
	XWindowChanges window_changes;
	unsigned long mask;

	/* 
	 * Set up data for processing slide up
	 */
	pSOR->pCD = pCD;
	pSOR->interval = SLIDE_UP_INTERVAL;
	pSOR->direction = pCD->slideDirection;
	pSOR->mapping = True;
	pSOR->wSubpanel = NULL;
	pSOR->pCD->pSOR = pSOR;
	
	switch (pSOR->direction)
	{
	    case SLIDE_NORTH:
		pSOR->incWidth = 0;
		pSOR->currWidth = pCD->frameInfo.width;
		pSOR->currHeight = pSOR->incHeight;
		pSOR->currX = pCD->frameInfo.x;
		pSOR->currY = pCD->frameInfo.y + 
		    (pCD->frameInfo.height - pSOR->currHeight);
		break;

	    case SLIDE_SOUTH:
		pSOR->incWidth = 0;
		pSOR->currWidth = pCD->frameInfo.width;
		pSOR->currHeight = pSOR->incHeight;
		pSOR->currX = pCD->frameInfo.x;
		pSOR->currY = pCD->frameInfo.y;
		break;
	}

	/*
	 * Create screening window to hide the slide-up from button
	 * events until it is all the way up.
	 */
	mask = CWOverrideRedirect;
	window_attribs.override_redirect = True;
	pSOR->coverWin = XCreateWindow(DISPLAY,
			    RootWindow (DISPLAY, SCREEN_FOR_CLIENT(pCD)),
			    pSOR->currX, pSOR->currY, 
			    pSOR->currWidth, pSOR->currHeight, 0,
			    CopyFromParent,InputOnly,CopyFromParent,
			    mask, &window_attribs);

	/* 
	 * Put screen window above the slide-up client
	 */
        mask = CWStackMode | CWSibling;
        window_changes.stack_mode = Above;
        window_changes.sibling = pCD->clientFrameWin;
        XConfigureWindow (DISPLAY, pSOR->coverWin, mask, &window_changes);

	/*
	 * Start slide-up processing
	 */ 
	XMoveResizeWindow (DISPLAY, pSOR->coverWin, pSOR->currX, pSOR->currY,
	    pSOR->currWidth, pSOR->currHeight);
	XMoveResizeWindow (DISPLAY, pCD->clientFrameWin, 
	    pSOR->currX, pSOR->currY, pSOR->currWidth, pSOR->currHeight);
	XMapWindow (DISPLAY, pSOR->coverWin);
	XMapWindow (DISPLAY, pCD->clientFrameWin);
	XSync (DISPLAY, False);

	XtAppAddTimeOut(wmGD.mwmAppContext, pSOR->interval,
			SlideOutTimerProc, (XtPointer)pSOR);
	
	wmGD.iSlideUpsInProgress += 1;

    }
    else
    {
	/*
	 * Not sliding because no direction specified or our window
	 * is just a little guy.
	 */
	XMapWindow (DISPLAY, pCD->clientFrameWin);
	if (pSOR) 
	{
	    XtFree ((char *) pSOR);
	    pCD->pSOR = NULL;
	}
    }

} /* END OF FUNCTION SlideOutWindow */



/*************************************<->*************************************
 *
 *  SlideSubpanelBackIn (pCD, wSubpanel)
 *
 *
 *  Description:
 *  -----------
 *  Slides a subpanel back in
 *
 *
 *  Inputs:
 *  ------
 *  pCD	= pointer to  client data
 *  wSubpanel = subpanel widget to unmanage
 *
 *  Comment:
 *  -------
 * 
 *************************************<->***********************************/

void
#ifdef _NO_PROTO
SlideSubpanelBackIn (pCD, wSubpanel)
	ClientData *pCD;
	Widget wSubpanel;

#else /* _NO_PROTO */
SlideSubpanelBackIn (ClientData *pCD, Widget wSubpanel)
#endif /* _NO_PROTO */
{
    SlideOutRec *pSOR;

    if (pCD->pSOR)
    {
	pSOR = pCD->pSOR;

	/*
	 *  Hmmm. We're already sliding this window.
	 *  If we're supposed to go in the other direction,
	 *  then turn it around.
	 */
	if (pSOR->mapping == False)
	{
	    /*
	     * We're already unmapping this guy, ignore this
	     * and finish what we've already got going.
	     */
	    return;
	}
	else
	{
	    /*
	     * We're mapping this guy. Reverse course!!
	     */
	    pSOR->mapping = False;
	    pSOR->wSubpanel = wSubpanel;

	    /* handle the rest on the next timeout */
	    return;
	}
    }

    pSOR = (SlideOutRec *) XtMalloc (sizeof(SlideOutRec));
    if (pSOR)
    {
	/*
	 * Compute this ahead of time to check if our window
	 * is short. If it is, we'll just unmap it, avoiding
	 * a lot of extra work.
	 */
	pSOR->incHeight = (Dimension) (DisplayHeight(DISPLAY, 
		SCREEN_FOR_CLIENT(pCD))/SLIDE_UP_DIVISOR);
    }

    if ((pCD->slideDirection != SLIDE_NOT) && pSOR &&
	(pSOR->incHeight < pCD->frameInfo.height))
    {
	XSetWindowAttributes window_attribs;
	XWindowChanges window_changes;
	unsigned long mask;

	/* 
	 * Set up data for processing slide up
	 */
	pSOR->pCD = pCD;
	pSOR->interval = SLIDE_UP_INTERVAL;
	pSOR->direction = pCD->slideDirection;
	pSOR->mapping = False;
	pSOR->wSubpanel = wSubpanel;
	pSOR->pCD->pSOR = pSOR;
	
	pSOR->incWidth = 0;
	pSOR->currWidth = pCD->frameInfo.width;
	pSOR->currHeight = pCD->frameInfo.height;
	pSOR->currX = pCD->frameInfo.x;
	pSOR->currY = pCD->frameInfo.y;

	switch (pSOR->direction)
	{
	    case SLIDE_NORTH:
		pSOR->currHeight -= pSOR->incHeight;
		pSOR->currY += pSOR->incHeight;
		break;

	    case SLIDE_SOUTH:
		pSOR->currHeight -= pSOR->incHeight;
		break;
	}

	/*
	 * Create screening window to hide the slide-up from button
	 * events until it is all the way up.
	 */
	mask = CWOverrideRedirect;
	window_attribs.override_redirect = True;
	pSOR->coverWin = XCreateWindow(DISPLAY,
			    RootWindow (DISPLAY, SCREEN_FOR_CLIENT(pCD)),
			    pSOR->currX, pSOR->currY, 
			    pSOR->currWidth, pSOR->currHeight, 0,
			    CopyFromParent,InputOnly,CopyFromParent,
			    mask, &window_attribs);

	/* 
	 * Put screen window above the slide-up client
	 */
	mask = CWStackMode | CWSibling;
	window_changes.stack_mode = Above;
	window_changes.sibling = pCD->clientFrameWin;
	XConfigureWindow (DISPLAY, pSOR->coverWin, mask, &window_changes);

	/*
	 * Start slide-up processing
	 */ 
	XMapWindow (DISPLAY, pSOR->coverWin);

	if (pSOR->currHeight > 0)
	{
	    XMoveResizeWindow (DISPLAY, pCD->clientFrameWin, 
		pSOR->currX, pSOR->currY, 
		pSOR->currWidth, pSOR->currHeight);

	    XMoveResizeWindow (DISPLAY, pSOR->coverWin, 
		pSOR->currX, pSOR->currY, 
		pSOR->currWidth, pSOR->currHeight);

	    XSync (DISPLAY, False);
	}

	XtAppAddTimeOut(wmGD.mwmAppContext, pSOR->interval,
			SlideOutTimerProc, (XtPointer)pSOR);

	wmGD.iSlideUpsInProgress += 1;

    }
    else
    {
	/*
	 * Not sliding because no direction specified or our window
	 * is just a little guy.
	 */
	/* Just unmanage the slide-up */
	XtUnmanageChild (wSubpanel);
	if (pSOR) 
	{
	    XtFree ((char *) pSOR);
	    pCD->pSOR = NULL;
	}
    }

} /* END OF FUNCTION SlideOutWindow */
#endif /* PANELIST */


/*************************************<->*************************************
 *
 *  MapClientWindows (pCD)
 *
 *
 *  Description:
 *  -----------
 *  Maps the window.  If this is a transient tree then all the windows in
 *  the transient tree are mapped.
 *
 *
 *  Inputs:
 *  ------
 *  pCD	= pointer to  client data
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void MapClientWindows (pCD)
	ClientData *pCD;

#else /* _NO_PROTO */
void MapClientWindows (ClientData *pCD)
#endif /* _NO_PROTO */
{
    ClientData *pNext;


    pNext = pCD->transientChildren;
    while (pNext)
    {
	/* map all transient children first */
	if (pNext->transientChildren)
	{
	    MapClientWindows (pNext);
	}

	/* then map all siblings at this level */
	XMapWindow (DISPLAY, pNext->client);
	XMapWindow (DISPLAY, pNext->clientFrameWin);

	pNext = pNext->transientSiblings;
    }

    /* map the primary window */
    XMapWindow (DISPLAY, pCD->client);
    XMapWindow (DISPLAY, pCD->clientFrameWin);

} /* END OF FUNCTION MapClientWindows */



/*************************************<->*************************************
 *
 *  ShowIconForMinimizedClient (pWS, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function shows the icon for the specified client.  If the icon
 *  is in an icon box then the "minimized" icon is displayed.  If the icon
 *  is on the root window it is mapped.
 * 
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  pCD	= pointer to  client data
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ShowIconForMinimizedClient (pWS, pCD)
	WmWorkspaceData *pWS;
	ClientData *pCD;
	
#else /* _NO_PROTO */
void ShowIconForMinimizedClient (WmWorkspaceData *pWS, ClientData *pCD)
#endif /* _NO_PROTO */
{
    WmScreenData *pSD = PSD_FOR_CLIENT(pCD);

    /*
     * Handle auto-placement for root icons (icons not in an icon
     * box).
     */
    if (wmGD.iconAutoPlace && !P_ICON_BOX(pCD))
    {
        if ((ICON_PLACE(pCD) == NO_ICON_PLACE) ||
#ifdef FWS
	    (ICON_PLACE(pCD) == pWS->IPData.fwsNextIcon) ||
#endif /* FWS */
	    ((pWS->IPData.placeList[ICON_PLACE(pCD)].pCD) &&
	     (pWS->IPData.placeList[ICON_PLACE(pCD)].pCD != pCD)))
        {
            /*
             * Icon place not defined or occupied by another client,
	     * find a free place to put the icon.
             */

#ifdef FWS
	    if ((ICON_PLACE(pCD) = GetNextIconPlace (pWS, &pWS->IPData)) 
		== NO_ICON_PLACE)
#else
	    if ((ICON_PLACE(pCD) = GetNextIconPlace (&pWS->IPData)) 
		== NO_ICON_PLACE)
#endif /* FWS */
	    {
	        ICON_PLACE(pCD) = 
		    CvtIconPositionToPlace (&pWS->IPData,
							 pCD->clientX,
			               	                 pCD->clientY);
	    }
	    CvtIconPlaceToPosition (&pWS->IPData, ICON_PLACE(pCD), 
				    &ICON_X(pCD), &ICON_Y(pCD));

#ifndef WSM
	    XMoveWindow (DISPLAY, ICON_FRAME_WIN(pCD), 
		ICON_X(pCD), ICON_Y(pCD));
#endif /* WSM */

        }

        pWS->IPData.placeList[ICON_PLACE(pCD)].pCD = pCD;
    }

#ifdef WSM
    /*
     * If icon on root window and this workspace is active, the
     * make sure it's in the right place.
     */
    if ((pWS == pSD->pActiveWS) && !P_ICON_BOX(pCD))
    {
	XMoveWindow (DISPLAY, ICON_FRAME_WIN(pCD), 
	    ICON_X(pCD), ICON_Y(pCD));
    }
#endif /* WSM */
    if (pCD->iconWindow)
    {
        XMapWindow (DISPLAY, pCD->iconWindow);
    }

    if ((pSD->useIconBox) && P_ICON_BOX(pCD))
    {
        ShowClientIconState (pCD, MINIMIZED_STATE );
    }
    else
    {
	XWindowChanges windowChanges;

	/*
	 * Map the icon on the screen at the appropriate place in the 
	 * window stack.
	 */

	if (wmGD.lowerOnIconify)
	{
	    if ((&pCD->iconEntry != pSD->lastClient) &&
		(pSD->lastClient))
	    {
		if (pSD->lastClient->type == MINIMIZED_STATE)
		{
		    windowChanges.sibling = 
			ICON_FRAME_WIN(pSD->lastClient->pCD);
		}
		else
		{
		    windowChanges.sibling =
			pSD->lastClient->pCD->clientFrameWin;
		}
		windowChanges.stack_mode = Below;
		XConfigureWindow (DISPLAY, ICON_FRAME_WIN(pCD),
		    		  (CWSibling | CWStackMode), &windowChanges);
		MoveEntryInList (pWS, &pCD->iconEntry, 
		    False /*on bottom*/, NULL);
	    }
	}
	else
	{
	    windowChanges.sibling = pCD->clientFrameWin;
	    windowChanges.stack_mode = Below;
	    XConfigureWindow (DISPLAY, ICON_FRAME_WIN(pCD),
	    		      (CWSibling | CWStackMode), &windowChanges);
	    MoveEntryInList (pWS, &pCD->iconEntry, False /*below*/,
			     &pCD->clientEntry);
	}

#ifdef WSM
        if (pWS == pSD->pActiveWS)
	{
	    XMapWindow (DISPLAY, ICON_FRAME_WIN(pCD));
	}
#else /* WSM */
	XMapWindow (DISPLAY, ICON_FRAME_WIN(pCD));
#endif /* WSM */
    }

} /* END OF FUNCTION ShowIconForMinimizedClient */

#ifdef WSM

/*************************************<->*************************************
 *
 *  ShowAllIconsForMinimizedClient (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function places icons in all the workspaces for the minimized
 *  client. Since there is only one clientState per client (not per
 *  workspace), this loops over all workspace in which the client
 *  resides and places an icon in each.
 * 
 *
 *  Inputs:
 *  ------
 *  pCD	= pointer to  client data
 *
 *  Comments:
 *  ---------
 *  This operates by setting up the currentWsc index for each workspace
 *  and calling ShowIconForMinimizedClient, which makes heavy use of
 *  the macros that use the currentWsc index.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ShowAllIconsForMinimizedClient (pCD)
	ClientData *pCD;

#else /* _NO_PROTO */
void ShowAllIconsForMinimizedClient (ClientData *pCD)
#endif /* _NO_PROTO */
{
    int saveWsc = pCD->currentWsc;
    int tmpWsc;
    WmWorkspaceData *pWS;

    for (tmpWsc = 0; tmpWsc < pCD->numInhabited; tmpWsc++)
    {
	pCD->currentWsc = tmpWsc;
	pWS = GetWorkspaceData (PSD_FOR_CLIENT(pCD),
				    pCD->pWsList[tmpWsc].wsID);
	ShowIconForMinimizedClient(pWS, pCD);
    }
    
    pCD->currentWsc = saveWsc;

} /* END OF FUNCTION ShowAllIconsForMinimizedClient */
#endif /* WSM */

#ifdef AUTOMATION
#ifdef _NO_PROTO
static void SetMwmIconInfo (pcd)
ClientData *pcd;

#else
static void SetMwmIconInfo (ClientData *pcd)
#endif /* _NO_PROTO */
{

    PropMwmFrameIconInfo    frame_icon_prop;
    char                    *ret_data;
    Atom                    frame_icon_atom, new_type;
    int                     new_format;
    unsigned long           new_nitems, new_bytes_after;


    frame_icon_atom = XmInternAtom(DISPLAY, "_MOTIF_WM_FRAME_ICON_INFO", False);
    XGetWindowProperty(DISPLAY, pcd->client, frame_icon_atom, 0L, 
                       PROP_MWM_FRAME_ICON_INFO_ELEMENTS, False, 
                       AnyPropertyType, &new_type, &new_format, 
                       &new_nitems, &new_bytes_after, 
                       (unsigned char **)&ret_data);
	if (ret_data != NULL)
    	bcopy((char *)ret_data, (char *)&frame_icon_prop, new_nitems);
	/* Swap bytes if necessary */
	AutoSwapBytes(&frame_icon_prop);

    frame_icon_prop.iconInfo.clientState = pcd->clientState;
    frame_icon_prop.iconInfo.useIconBox = pcd->pSD->useIconBox;
    frame_icon_prop.iconInfo.X = pcd->iconX;
    frame_icon_prop.iconInfo.Y = pcd->iconY;
    frame_icon_prop.iconInfo.Width = pcd->pSD->iconWidth;
    frame_icon_prop.iconInfo.Height = pcd->pSD->iconHeight;
    frame_icon_prop.iconInfo.iconFrameWin = pcd->iconFrameWin;

    if (pcd->pSD->useIconBox == True)
        FillIconBoxInfo(pcd, &frame_icon_prop);

    frame_icon_prop.byte_order = AutoByteOrderChar;
    XChangeProperty(DISPLAY, pcd->client, wmGD.xa_MWM_FRAME_ICON_INFO, 
                    wmGD.xa_MWM_FRAME_ICON_INFO, 8, PropModeReplace,
                    (unsigned char *)&frame_icon_prop, 
                    PROP_MWM_FRAME_ICON_INFO_ELEMENTS);
    XSync(DISPLAY, False);

    if (ret_data != NULL)
		XFree((char *)ret_data);

} /* SetMwmIconInfo */


#ifdef _NO_PROTO
static void FillIconBoxInfo(pcd, frame_icon_prop)
ClientData           *pcd;
PropMwmFrameIconInfo *frame_icon_prop;

#else
static void FillIconBoxInfo(ClientData *pcd, 
                            PropMwmFrameIconInfo *frame_icon_prop)
#endif /* _NO_PROTO */
{
    Widget              shellWidget;
    IconBoxData         *icon_box;
    XmScrollBarWidget   hScrollBar, vScrollBar;
    int                 hslider_area_width, vslider_area_height;
    int                 num_visible_cols, num_visible_rows;
    int                 visible_first_col, visible_last_col, visible_first_row,
                        visible_last_row, current_icon_col, current_icon_row;
    int                 pointerX, pointerY;
    int                 vert_slider_inc, horiz_slider_inc;
    int                 vert_inc_needed, horiz_inc_needed;
    int                 iPlaceW, iPlaceH, maxval, minval;
    int                 iconX, iconY;
	int                 sliderX, sliderY, slider_area_x, slider_area_y;
    int                 lastRow, lastCol;

    icon_box = pcd->pIconBox;
    if (icon_box == NULL) {
        return;
    }

    hScrollBar = (XmScrollBarWidget)icon_box->hScrollBar;
    vScrollBar = (XmScrollBarWidget)icon_box->vScrollBar;
    shellWidget = icon_box->shellWidget;

    lastCol = icon_box->lastCol;
    lastRow = icon_box->lastRow;
    iPlaceW = icon_box->IPD.iPlaceW;
    iPlaceH = icon_box->IPD.iPlaceH;
    iconX = pcd->iconX;
    iconY = pcd->iconY;

    /* Compute pointerX and horiz_inc_needed */
    minval = hScrollBar->scrollBar.minimum;
    maxval = hScrollBar->scrollBar.maximum;
    hslider_area_width = hScrollBar->scrollBar.slider_area_width;
    horiz_slider_inc = (iPlaceW * hslider_area_width) / (maxval - minval);
    sliderX = hScrollBar->scrollBar.slider_x;
    slider_area_x = hScrollBar->scrollBar.slider_area_x;
    visible_first_col = round_quotient((sliderX - slider_area_x),
                                       horiz_slider_inc);
    num_visible_cols = round_quotient((hslider_area_width * (lastCol + 1)),
                                      (maxval - minval));
    visible_last_col = visible_first_col + num_visible_cols - 1;
    current_icon_col = iconX / iPlaceW;

    if ((visible_first_col == 0 && visible_last_col == lastCol) ||
        (current_icon_col >= visible_first_col && 
         current_icon_col <= visible_last_col)) {
        
        horiz_inc_needed = 0;
        if (num_visible_cols > 1)
            pointerX = slider_area_x + (current_icon_col * iPlaceW) + 
                       (iPlaceW / 2);
        else
            pointerX = slider_area_x + (iPlaceW / 2);

    }
    else if (current_icon_col < visible_first_col) {

        horiz_inc_needed = current_icon_col - visible_first_col;
        pointerX = slider_area_x + (iPlaceW / 2);

    }
    else if (current_icon_col > visible_last_col) {

        horiz_inc_needed = current_icon_col - visible_last_col;
        pointerX = slider_area_x + 
                   ((visible_last_col - visible_first_col) * iPlaceW) +
                   (iPlaceW / 2);

    }

    /* Now compute pointerY and vert_inc_needed */
    minval = vScrollBar->scrollBar.minimum;
    maxval = vScrollBar->scrollBar.maximum;
    vslider_area_height = vScrollBar->scrollBar.slider_area_height;
    vert_slider_inc = (iPlaceH * vslider_area_height) / (maxval - minval);
    sliderY = vScrollBar->scrollBar.slider_y;
    slider_area_y = vScrollBar->scrollBar.slider_area_y;
    visible_first_row = round_quotient((sliderY - slider_area_y), 
                                       vert_slider_inc);
    num_visible_rows = round_quotient((vslider_area_height * (lastRow + 1)),
                                      (maxval - minval));
    visible_last_row = visible_first_row + num_visible_rows - 1;
    current_icon_row = iconY / iPlaceH;

    if ((visible_first_row == 0 && visible_last_row == lastRow) ||
        (current_icon_row >= visible_first_row && 
         current_icon_row <= visible_last_row)) {
        
        vert_inc_needed = 0;
        if (num_visible_rows > 1)
            pointerY = slider_area_y + (current_icon_row * iPlaceH) + 
                       (iPlaceH / 2);
        else
            pointerY = slider_area_y + (iPlaceH / 2);

    }
    else if (current_icon_row < visible_first_row) {

        vert_inc_needed = current_icon_row - visible_first_row;
        pointerY = slider_area_y + (iPlaceH / 2);

    }
    else if (current_icon_row > visible_last_row) {

        vert_inc_needed = current_icon_row - visible_last_row;
        pointerY = slider_area_y + 
                   ((visible_last_row - visible_first_row) * iPlaceH) +
                   (iPlaceH / 2);

    }

    /* Now fill icon box info into property */
    frame_icon_prop->iconBoxInfo.iconboxX = shellWidget->core.x;
    frame_icon_prop->iconBoxInfo.iconboxY = shellWidget->core.y;
    frame_icon_prop->iconBoxInfo.iconboxWidth = shellWidget->core.width;
    frame_icon_prop->iconBoxInfo.iconboxHeight = shellWidget->core.height;
    frame_icon_prop->iconBoxInfo.pointerX = pointerX;
    frame_icon_prop->iconBoxInfo.pointerY = pointerY;
    frame_icon_prop->iconBoxInfo.horiz_inc_needed = horiz_inc_needed;
    frame_icon_prop->iconBoxInfo.vert_inc_needed = vert_inc_needed;
    frame_icon_prop->iconBoxInfo.iconShellWin = XtWindow(shellWidget);
    frame_icon_prop->iconBoxInfo.left_arrowX = hScrollBar->core.x +
                                      hScrollBar->scrollBar.arrow1_x +
                                      (hScrollBar->scrollBar.arrow_width / 2);
    frame_icon_prop->iconBoxInfo.left_arrowY = hScrollBar->core.y +
                                      hScrollBar->scrollBar.arrow1_y +
                                      (hScrollBar->scrollBar.arrow_height / 2);
    frame_icon_prop->iconBoxInfo.right_arrowX = hScrollBar->core.x +
                                      hScrollBar->scrollBar.arrow2_x +
                                      (hScrollBar->scrollBar.arrow_width / 2);
    frame_icon_prop->iconBoxInfo.right_arrowY = hScrollBar->core.y +
                                      hScrollBar->scrollBar.arrow2_y +
                                      (hScrollBar->scrollBar.arrow_height / 2);
    frame_icon_prop->iconBoxInfo.top_arrowX = vScrollBar->core.x +
                                      vScrollBar->scrollBar.arrow1_x +
                                      (vScrollBar->scrollBar.arrow_width / 2);
    frame_icon_prop->iconBoxInfo.top_arrowY = vScrollBar->core.y +
                                      vScrollBar->scrollBar.arrow1_y +
                                      (vScrollBar->scrollBar.arrow_height / 2);
    frame_icon_prop->iconBoxInfo.bottom_arrowX = vScrollBar->core.x +
                                      vScrollBar->scrollBar.arrow2_x +
                                      (vScrollBar->scrollBar.arrow_width / 2);
    frame_icon_prop->iconBoxInfo.bottom_arrowY = vScrollBar->core.y +
                                      vScrollBar->scrollBar.arrow2_y +
                                      (vScrollBar->scrollBar.arrow_height / 2);

    frame_icon_prop->iconBoxInfo.iconFrameWin = XtWindow(icon_box->frameWidget);
    frame_icon_prop->iconBoxInfo.iconScrollWin = 
                                           XtWindow(icon_box->scrolledWidget);
    frame_icon_prop->iconBoxInfo.hScrollWin = XtWindow(hScrollBar);
    frame_icon_prop->iconBoxInfo.vScrollWin = XtWindow(vScrollBar);

}

round_quotient(num, denom)
int    num;
int    denom;
{
    double    act_quotient;
    int        quotient;

    if (denom <= 0) {
        fprintf(stderr, "Fatal Error: Divide by 0 in SetMwmIconInfo\n");
        exit(0);
    }
    act_quotient = (double)num / (double)denom;
    quotient = act_quotient;

    quotient = ((act_quotient - quotient) > 0.5) ? (quotient + 1) :
               quotient;

    return(quotient);
}

#endif /* AUTOMATION */
