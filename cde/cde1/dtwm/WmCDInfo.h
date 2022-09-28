/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
/*   $XConsortium: WmCDInfo.h /main/cde1_maint/1 1995/07/15 01:45:01 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern int FrameX ();
extern int FrameY ();
extern unsigned int FrameWidth ();
extern unsigned int FrameHeight ();
extern unsigned int TitleTextHeight ();
extern unsigned int UpperBorderWidth ();
extern unsigned int LowerBorderWidth ();
extern unsigned int CornerWidth ();
extern unsigned int CornerHeight ();
extern int BaseWindowX ();
extern int BaseWindowY ();
extern unsigned int BaseWindowWidth ();
extern unsigned int BaseWindowHeight ();
extern Boolean GetFramePartInfo ();
extern int IdentifyFramePart ();
extern int GadgetID ();
extern void FrameToClient ();
extern void ClientToFrame ();
extern Boolean GetDepressInfo ();
extern void SetFrameInfo ();
extern void SetClientOffset ();
extern Boolean XBorderIsShowing ();
extern unsigned int InitTitleBarHeight ();
#else /* _NO_PROTO */
extern int FrameX (ClientData *pcd);
extern int FrameY (ClientData *pcd);
extern unsigned int FrameWidth (ClientData *pcd);
extern unsigned int FrameHeight (ClientData *pcd);
extern unsigned int TitleTextHeight (ClientData *pcd);
extern unsigned int UpperBorderWidth (ClientData *pcd);
extern unsigned int LowerBorderWidth (ClientData *pcd);
extern unsigned int CornerWidth (ClientData *pcd);
extern unsigned int CornerHeight (ClientData *pcd);
extern int BaseWindowX (ClientData *pcd);
extern int BaseWindowY (ClientData *pcd);
extern unsigned int BaseWindowWidth (ClientData *pcd);
extern unsigned int BaseWindowHeight (ClientData *pcd);
extern Boolean GetFramePartInfo (ClientData *pcd, int part, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight);
extern int IdentifyFramePart (ClientData *pCD, int x, int y);
extern int GadgetID (int x, int y, GadgetRectangle *pgadget, unsigned int count);
extern void FrameToClient (ClientData *pcd, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight);
extern void ClientToFrame (ClientData *pcd, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight);
extern Boolean GetDepressInfo (ClientData *pcd, int part, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight, unsigned int *pInvertWidth);
extern void SetFrameInfo (ClientData *pcd);
extern void SetClientOffset (ClientData *pcd);
extern Boolean XBorderIsShowing (ClientData *pcd);
extern unsigned int InitTitleBarHeight (ClientData *pcd);
#endif /* _NO_PROTO */

/*
 * TitleBarHeight() is now a simple macro instead of a procedure.
 */

#define TitleBarHeight(pcd) ((pcd)->frameInfo.titleBarHeight)
