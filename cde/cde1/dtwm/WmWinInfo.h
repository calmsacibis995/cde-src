/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $XConsortium: WmWinInfo.h /main/cde1_maint/1 1995/07/15 02:01:41 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO

extern void FindClientPlacement ();
extern void FixWindowConfiguration ();
extern void FixWindowSize ();
extern ClientData *GetClientInfo ();
extern ClientData *GetWmClientInfo ();
extern void CalculateGravityOffset ();
extern Boolean InitClientPlacement ();
extern void InitCColormapData ();
extern void MakeSystemMenu ();
extern void PlaceFrameOnScreen ();
extern void PlaceIconOnScreen ();
extern void ProcessMwmHints ();
extern void ProcessWmClass ();
extern void ProcessWmHints ();
extern void ProcessWmIconTitle ();
extern void ProcessWmNormalHints ();
extern void ProcessWmTransientFor ();
extern void ProcessWmWindowTitle ();
extern Boolean SetupClientIconWindow ();
extern Boolean WmGetWindowAttributes ();

#else /* _NO_PROTO */

extern void FindClientPlacement (ClientData *pCD);
extern void FixWindowConfiguration (ClientData *pCD, unsigned int *pWidth, 
				    unsigned int *pHeight, 
				    unsigned int widthInc, 
				    unsigned int heightInc);
extern void FixWindowSize (ClientData *pCD, unsigned int *pWidth, 
			   unsigned int *pHeight, unsigned int widthInc, 
			   unsigned int heightInc);
extern ClientData *GetClientInfo (WmScreenData *pSD, Window clientWindow, 
				  long manageFlags);
extern ClientData *GetWmClientInfo (WmWorkspaceData *pWS, ClientData *pCD, 
				    long manageFlags);
extern void CalculateGravityOffset (ClientData *pCD, int *xoff, int *yoff);
extern Boolean InitClientPlacement (ClientData *pCD, long manageFlags);
extern void InitCColormapData (ClientData *pCD);
extern void MakeSystemMenu (ClientData *pCD);
extern void PlaceFrameOnScreen (ClientData *pCD, int *pX, int *pY, int w, 
				int h);
extern void PlaceIconOnScreen (ClientData *pCD, int *pX, int *pY);
extern void ProcessMwmHints (ClientData *pCD);
extern void ProcessWmClass (ClientData *pCD);
extern void ProcessWmHints (ClientData *pCD, Boolean firstTime);
extern void ProcessWmIconTitle (ClientData *pCD, Boolean firstTime);
extern void ProcessWmNormalHints (ClientData *pCD, Boolean firstTime, 
				  long manageFlags);
extern void ProcessWmTransientFor (ClientData *pCD);
extern void ProcessWmWindowTitle (ClientData *pCD, Boolean firstTime);
extern Boolean SetupClientIconWindow (ClientData *pCD, Window window);
extern Boolean WmGetWindowAttributes (Window window);

#endif /* _NO_PROTO */
