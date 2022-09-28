/* $XConsortium: Dtwm.h /main/cde1_maint/2 1995/10/23 11:13:51 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Screen.h
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
#ifndef _screen_h
#define _screen_h

/* External Interface */

#ifdef _NO_PROTO

extern void popup_dtwmBB() ;
extern void restoreDtwm() ;
extern void saveDtwm() ;

#else

extern void popup_dtwmBB( Widget shell) ;
extern void restoreDtwm( Widget shell, XrmDatabase db) ;
extern void saveDtwm( int fd) ;

#endif /* _NO_PROTO */

#endif /* _screen_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
