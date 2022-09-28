/* $XConsortium: Keyboard.h /main/cde1_maint/2 1995/10/23 11:18:32 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Keyboard.h
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
#ifndef _keyboard_h
#define _keyboard_h

/* External Interface */

#ifdef _NO_PROTO

extern void popup_keyboardBB() ;
extern void restoreKeybd() ;
extern void saveKeybd() ;

#else

extern void popup_keyboardBB( Widget shell) ;
extern void restoreKeybd( Widget shell, XrmDatabase db) ;
extern void saveKeybd( int fd) ;

#endif /* _NO_PROTO */

#endif /* _keyboard_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
