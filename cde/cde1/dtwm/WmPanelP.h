/*
 * $XConsortium: WmPanelP.h /main/cde1_maint/2 1995/10/03 17:53:10 lehors $
 */
/******************************<+>*************************************
 **********************************************************************
 **
 **  File:        WmPanelP.h
 **
 **  Project:     HP/Motif Workspace Manager (dtwm)
 **
 **  Description:
 **  -----------
 **  This file contains private declarations for the Front Panel
 **  subsystem.
 **
 **
 **********************************************************************
 **
 ** (c) Copyright 1992 HEWLETT-PACKARD COMPANY
 ** ALL RIGHTS RESERVED
 **
 **********************************************************************
 **********************************************************************
 **
 **
 **********************************************************************
 ******************************<+>*************************************/

#ifndef _WmPanelP_h
#define _WmPanelP_h

#include <Xm/XmP.h>
#include "WmGlobal.h"

#include "DataBaseLoad.h"

typedef struct PanelData * WmPanelistObject;

#define O_Panel(o) panel.form


#include <time.h>
#if defined(_AIX) || defined(USL) || defined(__uxp__)
#include <sys/time.h> /* needed for timeval */
#endif


typedef struct
    {
    String	 pchResName;	/* match res_name member of WM_CLASS prop */
    Widget	 wControl;	/* control associated with client */
    Window	 winParent;	/* window to reparent client to */
    Position	 x, y;		/* position of client in winParent */
    Dimension	 width, height;	/* required size of client */
    ClientData	*pCD;		/* filled in when managed, else NULL */
    }	WmFpEmbeddedClientData, *WmFpEmbeddedClientList;

typedef struct _WmFpPushRecallClientData
{
   String	    pchResName;		/* match res_name member of WM_CLASS prop */
   Widget	    wControl;		/* control associated with client */
   ClientData	  * pCD;		/* filled in when managed, else NULL */
   struct timeval   tvTimeout;		/* Time for client to start */
} WmFpPushRecallClientData, *WmFpPushRecallClientList;


#ifdef _NO_PROTO

void    WmSubpanelPosted ();                    /*  WmWinState.c  */
Widget  WmPanelistAllocate ();                  /*  WmInitWs.c    */
void    WmPanelistShow ();                      /*  WmInitWs.c    */
void    WmPanelistSetWorkspace ();              /*  WmWrkspace.c  */
void    WmFrontPanelSetBusy ();                 /*  WmFunction.c WmIPC.c  */
Widget  WmPanelistWindowToSubpanel ();          /*  WmFunction.c WmWinInfo.c  */
void    WmFrontPanelSessionSaveData ();         /*  WmWrkspace.c  */

#else /* _NO_PROTO */

void    WmSubpanelPosted (Display *, Window);
Widget  WmPanelistAllocate(Widget, XtPointer, XtPointer);
void    WmPanelistShow (Widget);
void    WmPanelistSetWorkspace (Widget, int);
void    WmFrontPanelSetBusy (Boolean);
Widget  WmPanelistWindowToSubpanel (Display *, Window);
void    WmFrontPanelSessionSaveData ();

#endif /* _NO_PROTO */


#endif /* _WmPanelP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
