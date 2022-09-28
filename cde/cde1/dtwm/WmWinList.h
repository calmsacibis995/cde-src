/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
/*   $XConsortium: WmWinList.h /main/cde1_maint/1 1995/07/15 02:02:15 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO

extern void			AddClientToList ();
extern void			AddEntryToList ();
extern void			AddTransient ();
#ifdef WSM
extern Boolean 			BumpPrimaryToBottom ();
extern Boolean 			BumpPrimaryToTop ();
#endif /* WSM */
extern Boolean			CheckIfClientObscuredByAny ();
extern Boolean			CheckIfClientObscuring ();
extern Boolean			CheckIfClientObscuringAny ();
extern Boolean			CheckIfObscuring ();
extern int			CountTransientChildren ();
extern void			DeleteClientFromList ();
extern void			DeleteEntryFromList ();
extern void			DeleteFullAppModalChildren();
extern void			DeleteTransient();
extern ClientListEntry *	FindClientNameMatch ();
#ifdef WSM
extern ClientData *		FindSubLeaderToTop ();
#endif /* WSM */
extern ClientData *		FindTransientFocus ();
extern ClientData *		FindTransientOnTop ();
extern ClientData *		FindTransientTreeLeader ();
extern void			FixupFullAppModalCounts();
#ifdef WSM
extern Boolean			LeaderOnTop ();
extern Window			LowestWindowInTransientFamily ();
extern void 			MakeTransientFamilyStackingList ();
#endif /* WSM */
extern Window *		        MakeTransientWindowList ();
extern void			MarkModalSubtree ();
extern void			MarkModalTransient ();
extern void			MoveEntryInList ();
#ifdef WSM
extern Boolean 			NormalizeTransientTreeStacking ();
#endif /* WSM */
extern Boolean			PutTransientBelowSiblings ();
extern Boolean			PutTransientOnTop ();
extern void			RestackTransients ();
extern void			RestackTransientsAtWindow ();
extern void			SetupSystemModalState ();
extern void			StackTransientWindow ();
extern void			StackWindow ();
extern void			UnMarkModalTransient ();
extern void			UndoSystemModalState ();

#else /* _NO_PROTO */

extern void AddClientToList (WmWorkspaceData *pWS, ClientData *pCD, 
			     Boolean onTop);
extern void AddEntryToList (WmWorkspaceData *pWS, ClientListEntry *pEntry, 
			    Boolean onTop, ClientListEntry *pStackEntry);
extern void AddTransient (WmWorkspaceData *pWS, ClientData *pCD);
#ifdef WSM
extern Boolean BumpPrimaryToBottom (ClientData *pcdLeader);
extern Boolean BumpPrimaryToTop (ClientData *pcdLeader);
#endif /* WSM */
extern Boolean CheckIfClientObscuredByAny (ClientData *pcd);
extern Boolean CheckIfClientObscuring (ClientData *pcdTop, ClientData *pcd);
extern Boolean CheckIfClientObscuringAny (ClientData *pcd);
extern Boolean CheckIfObscuring (ClientData *pcdA, ClientData *pcdB);
extern int CountTransientChildren (ClientData *pcd);
extern void DeleteClientFromList (WmWorkspaceData *pWS, ClientData *pCD);
extern void DeleteEntryFromList (WmWorkspaceData *pWS, 
				 ClientListEntry *pListEntry);
extern void DeleteFullAppModalChildren (ClientData *pcdLeader, 
					ClientData *pCD);
extern void DeleteTransient (ClientData *pCD);
extern ClientListEntry *FindClientNameMatch (ClientListEntry *pEntry, 
					     Boolean toNext, 
					     String clientName,
					     unsigned long types);
#ifdef WSM
extern ClientData *FindSubLeaderToTop (ClientData *pcd);
#endif /* WSM */
extern ClientData *FindTransientFocus (ClientData *pcd);
extern ClientData *FindTransientOnTop (ClientData *pcd);
extern ClientData *FindTransientTreeLeader (ClientData *pcd);
extern void FixupFullAppModalCounts (ClientData *pcdLeader, 
				     ClientData *pcdDelete);
#ifdef WSM
extern Boolean LeaderOnTop (ClientData *pcdLeader);
extern Window LowestWindowInTransientFamily (ClientData *pcdLeader);
extern void MakeTransientFamilyStackingList (Window *windows, 
					    ClientData *pcdLeader);
#endif /* WSM */
extern Window *MakeTransientWindowList (Window *windows, ClientData *pcd);
extern void MarkModalSubtree (ClientData *pcdTree, ClientData *pcdAvoid);
extern void MarkModalTransient (ClientData *pcdLeader, ClientData *pCD);
extern void MoveEntryInList (WmWorkspaceData *pWS, ClientListEntry *pEntry, 
			     Boolean onTop, ClientListEntry *pStackEntry);
#ifdef WSM
extern Boolean NormalizeTransientTreeStacking (ClientData *pcdLeader);
#endif /* WSM */
extern Boolean PutTransientBelowSiblings (ClientData *pcd);
extern Boolean PutTransientOnTop (ClientData *pcd);
extern void RestackTransients (ClientData *pcd, Boolean doTop);
extern void RestackTransientsAtWindow (ClientData *pcd);
extern void SetupSystemModalState (ClientData *pCD);
extern void StackTransientWindow (ClientData *pcd);
extern void StackWindow (WmWorkspaceData *pWS, ClientListEntry *pEntry, 
			 Boolean onTop, ClientListEntry *pStackEntry);
extern void UnMarkModalTransient (ClientData *pcdModee, int modalCount, 
				  ClientData *pcdModal);
extern void UndoSystemModalState (void);

#endif /* _NO_PROTO */
