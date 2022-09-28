/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $XConsortium: WmColormap.h /main/cde1_maint/2 1995/10/03 18:33:52 lehors $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void InitWorkspaceColormap ();
extern void InitColormapFocus ();
#ifndef OLD_COLORMAP
extern void ForceColormapFocus ();
#endif
extern void SetColormapFocus ();
extern void WmInstallColormap ();
extern void ResetColormapData ();
#ifndef	IBM_169380
extern void AddColormapWindowReference ();
extern void RemoveColormapWindowReference ();
#endif
extern void ProcessColormapList();
#else /* _NO_PROTO */
extern void InitWorkspaceColormap (WmScreenData *pSD);
extern void InitColormapFocus (WmScreenData *pSD);
#ifndef OLD_COLORMAP
extern void ForceColormapFocus (WmScreenData *pSD, ClientData *pCD);
#endif
extern void SetColormapFocus (WmScreenData *pSD, ClientData *pCD);
extern void WmInstallColormap (WmScreenData *pSD, Colormap colormap);
extern void ResetColormapData (ClientData *pCD, Window *pWindows, int count);
#ifndef IBM_169380
extern void AddColormapWindowReference (ClientData *pCD, Window window);
extern void RemoveColormapWindowReference (ClientData *pCD, Window window);
#endif
extern void ProcessColormapList (WmScreenData *pSD, ClientData *pCD);
#endif /* _NO_PROTO */
