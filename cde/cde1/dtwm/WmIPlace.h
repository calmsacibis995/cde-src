/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $XConsortium: WmIPlace.h /main/cde1_maint/1 1995/07/15 01:51:35 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void InitIconPlacement ();
extern int GetNextIconPlace ();
extern void CvtIconPlaceToPosition ();
extern int FindIconPlace ();
extern int CvtIconPositionToPlace ();
extern void PackRootIcons ();
extern void MoveIconInfo ();
#else /* _NO_PROTO */
extern void InitIconPlacement (WmWorkspaceData *pWS);
#ifdef FWS
extern int GetNextIconPlace (WmWorkspaceData *pWS,
			     IconPlacementData *pIPD);
#else /* FWS */
extern int GetNextIconPlace (IconPlacementData *pIPD);
#endif /* FWS */
extern void CvtIconPlaceToPosition (IconPlacementData *pIPD, int place, int *pX, int *pY);
#ifdef FWS
extern int FindIconPlace (WmWorkspaceData *pWS, ClientData *pCD,
			  IconPlacementData *pIPD, int x, int y);
#else
extern int FindIconPlace (ClientData *pCD, IconPlacementData *pIPD, int x, int y);
#endif
extern int CvtIconPositionToPlace (IconPlacementData *pIPD, int x, int y);
extern void PackRootIcons (void);
extern void MoveIconInfo (IconPlacementData *pIPD, int p1, int p2);
#endif /* _NO_PROTO */
