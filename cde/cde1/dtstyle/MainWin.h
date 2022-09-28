/* $XConsortium: MainWin.h /main/cde1_maint/2 1995/10/23 11:20:34 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        MainWin.h
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
#ifndef _mainwin_h
#define _mainwin_h

/* external variable definitions */

extern char * _dt_version;

/* External Interface */

#ifdef _NO_PROTO

extern void init_mainWindow() ;
extern void callCancels() ;
extern void activateCB_exitBtn() ;
extern void restoreMain() ;
extern void saveMain() ;

#else

extern void init_mainWindow( Widget shell) ;
extern void callCancels( void ) ;
extern void activateCB_exitBtn( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void restoreMain( Widget shell, XrmDatabase db) ;
extern void saveMain( int fd) ;

#endif /* _NO_PROTO */

#endif /* _mainwin_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
