/* $XConsortium: SaveRestore.h /main/cde1_maint/2 1995/10/23 11:27:35 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SaveRestore.h
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
#ifndef _saverestore_h
#define _saverestore_h

/* External Interface */

#ifdef _NO_PROTO

extern void saveSessionCB() ;
extern Boolean restoreSession() ;

extern void saveFonts();
extern void restoreMain();
extern void saveMain();
extern void saveMouse();
extern void restoreMouse();
extern void saveAudio();
extern void restoreAudio();
extern void saveScreen();
extern void restoreScreen();
extern void saveColor();
extern void restoreColor();
extern void saveColorEdit();
extern void restoreColorEdit();
extern void saveKeybd();
extern void restoreKeybd();
extern void saveStartup();
extern void restoreStartup();
extern void saveBackdrop();
extern void restoreBackdrop();
extern void restoreFonts();

#else

extern void saveSessionCB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern Boolean restoreSession( 
                        Widget shell,
                        char *name) ;

extern void restoreFonts( Widget shell, XrmDatabase db) ;
extern void saveFonts( int fd) ;
extern void restoreMain( Widget shell, XrmDatabase db) ;
extern void saveMain( int fd) ;
extern void restoreMouse( Widget shell, XrmDatabase db) ;
extern void saveMouse( int fd) ;
extern void restoreAudio( Widget shell, XrmDatabase db) ;
extern void saveAudio( int fd) ;
extern void restoreScreen( Widget shell, XrmDatabase db) ;
extern void saveScreen( int fd) ;
extern void restoreColor( Widget shell, XrmDatabase db) ;
extern void saveColor( int fd) ;
extern void restoreColorEdit( Widget shell, XrmDatabase db) ;
extern void saveColorEdit( int fd) ;
extern void restoreKeybd( Widget shell, XrmDatabase db) ;
extern void saveKeybd( int fd) ;
extern void restoreStartup( Widget shell, XrmDatabase db) ;
extern void saveStartup( int fd) ;
extern void restoreBackdrop( Widget shell, XrmDatabase db) ;
extern void saveBackdrop( int fd) ;

#endif /* _NO_PROTO */

#endif /* _saverestore_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */


