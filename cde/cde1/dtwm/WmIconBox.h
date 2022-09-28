/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $XConsortium: WmIconBox.h /main/cde1_maint/1 1995/07/15 01:52:10 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO

#ifdef WSM
extern void       AddIconBoxForWorkspace ();
#endif /* WSM */
extern void	  AddNewBox ();
extern void	  ChangeActiveIconboxIconText ();
extern void	  CheckIconBoxResize ();
extern Boolean	  CheckIconBoxSize ();
extern void	  DeleteIconFromBox ();
extern void	  DeleteIconInfo ();
extern Boolean	  ExpandVirtualSpace ();
extern Boolean	  ExtendIconList();
extern void	  FindNewPosition ();
extern MenuItem  *GetIconBoxMenuItems ();
extern void       GetIconBoxIconRootXY ();
extern void	  HandleIconBoxButtonMotion ();
extern void       HandleIconBoxIconKeyPress ();
#ifndef MOTIF_ONE_DOT_ONE
extern void	  IconScrollVisibleCallback ();
#endif
extern void	  IconActivateCallback ();
extern Boolean    IconVisible ();
extern IconInfo	 *InsertIconInfo ();
extern Boolean	  InsertIconIntoBox ();
extern void	  InitIconBox ();
extern void	  InitializeClientData ();
extern void	  InitializeIconBoxData ();
extern Cardinal	  InsertPosition ();
extern void	  MakeBulletinBoard ();
extern void	  MakeFadeIconGC ();
extern Boolean	  MakeIconBox ();
extern void	  MakeScrolledWindow ();
extern void	  MakeShell ();
extern void	  MakeShrinkWrapIconsGC ();
extern void	  MapIconBoxes ();
extern void	  PackIconBox ();
extern void	  RealignIconList ();
extern void	  RealizeIconBox ();
extern void       ReorderIconBoxIcons ();
extern void	  ResetArrowButtonIncrements();
extern void	  ResetIconBoxMaxSize ();
extern void	  SetGeometry ();
extern void	  SetGranularity ();
extern void	  SetIconBoxInfo ();
extern void	  SetNewBounds ();
extern void	  ShowClientIconState ();
#ifdef WSM
extern void	  UnmapIconBoxes ();
#endif /* WSM */
extern void	  UpdateIncrements ();
extern String     WmXmStringToString ();

#else /* _NO_PROTO */

#ifdef WSM
extern void AddIconBoxForWorkspace (WmWorkspaceData *pWS);
#endif /* WSM */
extern void AddNewBox (WmWorkspaceData *pWS, IconBoxData *pIBD);
extern void ChangeActiveIconboxIconText (Widget icon, caddr_t dummy, 
					 XFocusChangeEvent *event);
extern void CheckIconBoxResize (ClientData *pCD, unsigned int changedValues, 
				int newWidth, int newHeight);
extern Boolean CheckIconBoxSize (IconBoxData *pIBD);
extern void DeleteIconFromBox (IconBoxData *pIBD, ClientData *pCD);
extern void DeleteIconInfo (IconBoxData *pIBD, ClientData *pCD);
extern Boolean ExpandVirtualSpace (IconBoxData *pIBD, int newWidth, 
				   int newHeight);
extern Boolean ExtendIconList (IconBoxData *pIBD, int incr);
extern void FindNewPosition (Cardinal *newPosition, IconPlacementData *pIPD, 
			     int newPlace);
extern MenuItem *GetIconBoxMenuItems (WmScreenData *pSD);
extern void GetIconBoxIconRootXY (ClientData *pCD, int *pX, int *pY);
extern void HandleIconBoxButtonMotion (Widget icon, caddr_t client_data, 
				       XEvent *pev);
extern void HandleIconBoxIconKeyPress (Widget icon, caddr_t dummy, 
				       XKeyEvent *keyEvent);
#ifndef MOTIF_ONE_DOT_ONE
extern void IconScrollVisibleCallback (Widget w, caddr_t client_data, 
				  XmAnyCallbackStruct *call_data);
#endif
extern void IconActivateCallback (Widget w, caddr_t client_data, 
				  XmAnyCallbackStruct *call_data);
extern Boolean IconVisible (ClientData *pCD);
extern IconInfo *InsertIconInfo (IconBoxData *pIBD, ClientData *pCD, 
				 Widget theWidget);
extern Boolean InsertIconIntoBox (IconBoxData *pIBD, ClientData *pCD);
extern void InitIconBox (WmScreenData *pSD);
extern void InitializeClientData (ClientData *pCD, IconBoxData *pIBD);
extern void InitializeIconBoxData (WmWorkspaceData *pWS, IconBoxData *pIBD);
extern Cardinal InsertPosition (Widget w);
extern void MakeBulletinBoard (WmWorkspaceData *pWS, IconBoxData *pIBD);
extern void MakeFadeIconGC (WmScreenData *pSD);
extern Boolean MakeIconBox (WmWorkspaceData *pWS, ClientData *pCD);
extern void MakeScrolledWindow (WmWorkspaceData *pWS, IconBoxData *pIBD);
extern void MakeShell (WmWorkspaceData *pWS, IconBoxData *pIBD);
extern void MakeShrinkWrapIconsGC (WmScreenData *pSD, Pixmap bgPixmap);
extern void MapIconBoxes (WmWorkspaceData *pWS);
extern void PackIconBox (IconBoxData *pIBD, Boolean packVert, 
			 Boolean packHorz, int passedInWidth, 
			 int passedInHeight);
extern void RealignIconList (IconBoxData *pIBD, int newCols, int newRows);
extern void RealizeIconBox (WmWorkspaceData *pWS, IconBoxData *pIBD, 
			    ClientData *pCD);
extern void ReorderIconBoxIcons (ClientData *pCD, IconBoxData *pIBD, 
				 Widget theIcon, int newX, int newY);
extern void ResetArrowButtonIncrements (ClientData *pCD);
extern void ResetIconBoxMaxSize (ClientData *pCD, Widget bBoardWidget);
extern void SetGeometry (WmWorkspaceData *pWS, ClientData *pCD, 
			 IconBoxData *pIBD);
extern void SetGranularity (WmWorkspaceData *pWS, ClientData *pCD, 
			    IconBoxData *pIBD);
extern void SetIconBoxInfo (WmWorkspaceData *pWS, ClientData *pCD);
extern void SetNewBounds (IconBoxData *pIBD);
extern void ShowClientIconState (ClientData *pCD, int newState);
#ifdef WSM
extern void UnmapIconBoxes (WmWorkspaceData *pWS);
#endif /* WSM */
extern void UpdateIncrements (Widget sWidget, IconBoxData *pIBD, 
			      XConfigureEvent *event);
extern String WmXmStringToString (XmString xmString);
#endif /* _NO_PROTO */

