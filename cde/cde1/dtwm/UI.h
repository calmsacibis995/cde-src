/* $XConsortium: UI.h /main/cde1_maint/1 1995/07/18 01:54:46 drk $ */
/*****************************************************************************
 *
 *   File:         UI.h
 *
 *   Project:	   CDE
 *
 *   Description:  This file contains defines and declarations needed
 *                 by UI.c
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
 *
 ****************************************************************************/


#ifndef _ui_h
#define _ui_h

#define SWITCH_HIGH_BUTTON_WIDTH	124
#define SWITCH_HIGH_BUTTON_HEIGHT	27
#define SWITCH_LOW_BUTTON_WIDTH		66
#define SWITCH_LOW_BUTTON_HEIGHT	22

#define ICON_HIGH_WIDTH			64
#define ICON_MEDIUM_WIDTH		58
#define ICON_LOW_WIDTH			40


#define UP_ARROW_IMAGE_NAME		"Fpup"
#define DOWN_ARROW_IMAGE_NAME		"Fpdown"
#define UP_MONITOR_ARROW_IMAGE_NAME	"FpupY"
#define DOWN_MONITOR_ARROW_IMAGE_NAME	"FpdownY"
#define BLANK_ARROW_IMAGE_NAME		"FpblnkA"
#define DROPZONE_IMAGE_NAME		"Fpdropz"
#define INDICATOR_OFF_IMAGE_NAME	"Fpindc"
#define INDICATOR_ON_IMAGE_NAME		"FpindcY"
#define MINIMIZE_NORMAL_IMAGE_NAME	"Fpmin"
#define MINIMIZE_SELECTED_IMAGE_NAME	"FpminY"
#define MENU_NORMAL_IMAGE_NAME		"Fpmenu"
#define MENU_SELECTED_IMAGE_NAME	"FpmenuY"
#define HANDLE_IMAGE_NAME		"Fphandl"
#define DEFAULT_IMAGE_NAME		"Fpdeflt"


#ifdef _NO_PROTO

extern void ToggleDefaultControl ();
extern String GetIconName ();

#else

extern void ToggleDefaultControl (ControlData *, SubpanelData *, ControlData *);
extern String GetIconName (String, unsigned int);

#endif /* _NO_PROTO */

#endif /* _ui_h */
/*  DON"T ADD ANYTHING AFTER THIS #endif  */
