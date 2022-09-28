/* $XConsortium: WmPresence.h /main/cde1_maint/1 1995/07/18 01:56:46 drk $ */
#ifdef WSM
/* 
 * (c) Copyright 1987, 1988, 1989, 1990, 1992 HEWLETT-PACKARD COMPANY 
 * ALL RIGHTS RESERVED 
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = ""
#endif
#endif

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Boolean MakePresenceBox() ;
extern void ShowPresenceBox() ;
extern void HidePresenceBox() ;
extern MenuItem * GetPresenceBoxMenuItems() ;
extern void UpdatePresenceWorkspaces();

#else

extern Boolean MakePresenceBox( 
                        WmScreenData *pSD) ;
extern void ShowPresenceBox( 
                        ClientData *pClient,
                        Context wsContext) ;
extern void HidePresenceBox( 
                        WmScreenData *pSD,
			Boolean  userDismissed) ;
extern MenuItem * GetPresenceBoxMenuItems( 
                        WmScreenData *pSD) ;
extern void UpdatePresenceWorkspaces( WmScreenData *pSD );

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/
#endif /* WSM */
