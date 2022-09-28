/* $XConsortium: Mouse.h /main/cde1_maint/2 1995/10/23 11:21:20 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Mouse.h
 **
 **   Project:     DT 3.0
 **
 **  This file contains function definitions for the corresponding .c
 **  file
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _mouse_h
#define _mouse_h

/* External Interface */

#ifdef _NO_PROTO

extern void popup_mouseBB() ;
extern void restoreMouse() ;
extern void saveMouse() ;

#else

extern void popup_mouseBB( Widget shell) ;
extern void restoreMouse( Widget shell, XrmDatabase db) ;
extern void saveMouse( int fd) ;

#endif /* _NO_PROTO */

#endif /* _mouse_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
