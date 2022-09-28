/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
/*   $XConsortium: WmCPlace.h /main/cde1_maint/1 1995/07/15 01:46:32 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void PlaceWindowInteractively ();
extern void DoPlacement ();
extern void SetupPlacement ();
extern void HandlePlacementKeyEvent ();
extern void HandlePlacementButtonEvent ();
extern void HandlePlacementMotionEvent ();
extern void StartInteractiveSizing ();
extern Bool IsRepeatedKeyEvent ();
#else /* _NO_PROTO */
extern void PlaceWindowInteractively (ClientData *pcd);
extern void DoPlacement (ClientData *pcd);
extern void SetupPlacement (ClientData *pcd);
extern void HandlePlacementKeyEvent (ClientData *pcd, XKeyEvent *pev);
extern void HandlePlacementButtonEvent (XButtonEvent *pev);
extern void HandlePlacementMotionEvent (ClientData *pcd, XMotionEvent *pev);
extern void StartInteractiveSizing (ClientData *pcd, Time time);
extern Bool IsRepeatedKeyEvent (Display *dpy, XEvent *pEvent, char *pOldEvent);
#endif /* _NO_PROTO */
