/* $XConsortium: Backdrop.h /main/cde1_maint/2 1995/10/23 11:08:04 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Backdrop.h
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
#ifndef _backdrop_h
#define _backdrop_h


/* External Interface */

#ifdef _NO_PROTO

extern void BackdropDialog() ;
extern void CheckWorkspace() ;
extern void restoreBackdrop() ;
extern void saveBackdrop() ;

#else

extern void BackdropDialog( Widget parent) ;
extern void CheckWorkspace( void ) ;
extern void restoreBackdrop( Widget shell, XrmDatabase db) ;
extern void saveBackdrop( int fd) ;

#endif /* _NO_PROTO */

#endif /* _backdrop_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

