/* $XConsortium: PopupMenu.h /main/cde1_maint/1 1995/07/18 01:53:51 drk $ */
/*****************************************************************************
 *
 *   File:         PopupMenu.h
 *
 *   Project:      CDE
 *
 *   Description:  This file contains defines and declarations needed
 *                 by PopupMenu.c and UI.c
 *
 *
 ****************************************************************************/

#ifndef _popup_menu_h
#define _popup_menu_h

#include "DataBaseLoad.h"

#ifdef _NO_PROTO

extern void CreatePopupMenu ();
extern void PostPopupMenu ();
extern void CreateWorkspacePopupMenu ();
extern void PostWorkspacePopupMenu ();

#else

extern void CreatePopupMenu ( Widget );
extern void PostPopupMenu ( Widget, XtPointer, XEvent * );
extern void CreateWorkspacePopupMenu ( Widget , SwitchData *);
extern void PostWorkspacePopupMenu ( Widget, XtPointer, XEvent * );

#endif /* _NO_PROTO */

#endif /* _popup_menu_h */
/*  DON"T ADD ANYTHING AFTER THIS #endif  */
