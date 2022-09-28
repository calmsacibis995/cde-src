/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
/*   $XConsortium: WmWinState.h /main/cde1_maint/1 1995/07/15 02:02:48 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void NormalTransientTransition();
extern void SetClientState ();
extern void SetClientStateWithEventMask ();
extern void ConfigureNewState ();
extern void SetClientWMState ();
extern void MapClientWindows ();
extern void ShowIconForMinimizedClient ();
#ifdef WSM
extern void ShowAllIconsForMinimizedClient ();
#endif /* WSM */
#ifdef PANELIST
extern void SlideSubpanelBackIn ();
#endif /* PANELIST */
#else /* _NO_PROTO */
extern void SetClientState (ClientData *pCD, int newState, Time setTime);
extern void SetClientStateWithEventMask (ClientData *pCD, int newState, Time setTime, unsigned int event_mask);
extern void ConfigureNewState (ClientData *pcd);
extern void SetClientWMState (ClientData *pCD, int wmState, int mwmState);
extern void MapClientWindows (ClientData *pCD);
extern void ShowIconForMinimizedClient (WmWorkspaceData *pWS, ClientData *pCD);
#ifdef WSM
extern void ShowAllIconsForMinimizedClient (ClientData *pCD);
#endif /* WSM */
#ifdef PANELIST
extern void SlideSubpanelBackIn (ClientData *pCD, Widget wSubpanel);
#endif /* PANELIST */
#endif /* _NO_PROTO */
