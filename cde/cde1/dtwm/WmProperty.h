/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
/*   $XConsortium: WmProperty.h /main/cde1_maint/1 1995/07/15 01:57:24 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern SizeHints * GetNormalHints ();
extern void ProcessWmProtocols ();
extern void HandleMailUpdate ();
extern void ProcessMwmMessages ();
extern void SetMwmInfo ();
#ifdef WSM
void SetMwmSaveSessionInfo ();
#endif /* WSM */
extern PropWMState * GetWMState ();
extern void SetWMState ();
extern PropMwmHints * GetMwmHints ();
extern PropMwmInfo * GetMwmInfo ();
extern void ProcessWmColormapWindows ();
extern Colormap FindColormap ();
extern MenuItem * GetMwmMenuItems ();
#ifdef WSM
extern void GetInitialPropertyList ();
extern Status GetWorkspaceHints ();
extern void SetWorkspaceInfo ();
extern void SetWorkspacePresence ();
extern Boolean HasProperty();
extern void DiscardInitialPropertyList ();
extern void GetInitialPropertyList ();
extern void SetWorkspaceListProperty ();
extern void SetCurrentWorkspaceProperty ();
extern void SetWorkspaceInfoProperty ();
extern void DeleteWorkspaceInfoProperty ();
extern char *WorkspacePropertyName ();
#endif /* WSM */
#else /* _NO_PROTO */
extern SizeHints * GetNormalHints (ClientData *pCD);
extern void ProcessWmProtocols (ClientData *pCD);
extern void HandleMailUpdate   (ClientData *pCD);
extern void ProcessMwmMessages (ClientData *pCD);
extern void SetMwmInfo (Window propWindow, long flags, Window wmWindow);
#ifdef WSM
void SetMwmSaveSessionInfo (Window wmWindow);
#endif /* WSM */
extern PropWMState * GetWMState (Window window);
extern void SetWMState (Window window, int state, Window icon);
extern PropMwmHints * GetMwmHints (ClientData *pCD);
extern PropMwmInfo * GetMwmInfo (Window rootWindowOfScreen);
extern void ProcessWmColormapWindows (ClientData *pCD);
extern Colormap FindColormap (ClientData *pCD, Window window);
extern MenuItem * GetMwmMenuItems (ClientData *pCD);
#ifdef WSM
extern void GetInitialPropertyList (ClientData *pCD);
extern Status GetWorkspaceHints (Display *display, Window window, Atom **ppWsAtoms, unsigned int *pCount, Boolean *pbAll);
#ifdef HP_VUE
extern void SetWorkspaceInfo (Window propWindow, WorkspaceInfo *pWsInfo, unsigned long cInfo);
#endif /* HP_VUE */
extern void SetWorkspacePresence (Window propWindow, Atom *pWsPresence, unsigned long cPresence);
extern Boolean HasProperty(ClientData *pCD, Atom property);
extern void DiscardInitialPropertyList (ClientData *pCD);
extern void GetInitialPropertyList (ClientData *pCD);
extern void SetWorkspaceListProperty (WmScreenData *pSD);
extern void SetCurrentWorkspaceProperty (WmScreenData *pSD);
extern void SetWorkspaceInfoProperty (WmWorkspaceData *pWS);
extern void DeleteWorkspaceInfoProperty (WmWorkspaceData *pWS);
extern char *WorkspacePropertyName (WmWorkspaceData *pWS);
#endif /* WSM */
#endif /* _NO_PROTO */
