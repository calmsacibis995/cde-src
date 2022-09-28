/* $XConsortium: Startup.h /main/cde1_maint/2 1995/10/23 11:30:12 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Startup.h
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
#ifndef _startup_h
#define _startup_h

/* External Interface */

#ifdef _NO_PROTO

extern void popup_startupBB() ;
extern void restoreStartup() ;
extern void saveStartup() ;

#else

extern void popup_startupBB( Widget shell) ;
extern void restoreStartup( Widget shell, XrmDatabase db) ;
extern void saveStartup( int fd) ;

#endif /* _NO_PROTO */

#endif /* _startup_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
