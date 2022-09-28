/*
 * $Header: WmPanel.h,v 1.7 93/08/17 15:42:59 xbuild Exp $
 */
/******************************<+>*************************************
 **********************************************************************
 **
 **  File:        WmPanel.h
 **
 **  Project:     HP/Motif Workspace Manager (dtwm)
 **
 **  Description:
 **  -----------
 **  This file contains public declarations for the Front Panel
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

#ifndef _WmPanel_h
#define _WmPanel_h

#include <Xm/Xm.h>

#include "WmParse.h"
#include "WmParseP.h"

#ifndef WmIsPanelist
#define WmIsPanelist(w) XtIsSubclass(w, wmPanelistObjectClass)
#endif /* WmIsPanelist */


#ifdef _NO_PROTO

Widget	WmPanelistCreate(); 
Widget	WmPanelistNameToSubpanel ();
void	WmPanelistSubpanelPosted ();
void	WmPanelistGetSubpanelShells ();
int	WmPanelistGetSwitchToggleWidth ();
void	WmPanelistParseItem ();
void	WmPanelistAllocate ();
void	WmPanelistShow ();
void	WmPanelistSetWorkspace ();
void    WmPanelistSetWorkspaceTitle ();
void	WmPanelistSetBusy ();
void    WmPanelistSetDropZones ();
void	WmPanelistSetClientGeometry ();
Widget  WmPanelistWindowToSubpanel ();

#else /* _NO_PROTO */

Widget	WmPanelistCreate(); 
void	WmPanelistSubpanelPosted (Display *, Window);
Widget	WmPanelistNameToSubpanel (Widget, String);
void	WmPanelistGetSubpanelShells (Widget, Widget**, int*);
int	WmPanelistGetSwitchToggleWidth (Widget);
void	WmPanelistParseItem (Widget, DtWmpParseBuf*);
void	WmPanelistAllocate (Widget);
void	WmPanelistShow (Widget);
void	WmPanelistSetWorkspace (Widget, int);
void    WmPanelistSetWorkspaceTitle (Widget, int, XmString);
void	WmPanelistSetBusy (Widget, Boolean);
void    WmPanelistSetDropZones (Widget);
void	WmPanelistSetClientGeometry ();
Widget  WmPanelistWindowToSubpanel (Display *, Window);

#endif /* _NO_PROTO */


extern Widget	WmPanelistCreate(); 

extern WidgetClass wmPanelistObjectClass;

typedef struct _WmPanelistClassRec * WmPanelistClass;
typedef struct _WmPanelistRec      * WmPanelistObject;


#define WmNuseDefaultGeometry	"useDefaultGeometry"
#define WmNuseDefaultColors   "useDefaultColors"

#define WmNname		"name"
#define WmCName		"Name"

#define WmNglobalData	"globalData"
#define WmCGlobalData	"GlobalData"

#define WmNscreenData	"screenData"
#define WmCScreenData	"ScreenData"

#endif /* _WmPanelist_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
