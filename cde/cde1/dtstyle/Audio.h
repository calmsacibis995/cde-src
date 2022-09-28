/* $XConsortium: Audio.h /main/cde1_maint/2 1995/10/23 11:06:47 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Audio.h
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
#ifndef _audio_h
#define _audio_h

/* External Interface */

#ifdef _NO_PROTO

extern void popup_audioBB() ;
extern void restoreAudio() ;
extern void saveAudio() ;

#else

extern void popup_audioBB( Widget shell) ;
extern void restoreAudio( Widget shell, XrmDatabase db) ;
extern void saveAudio( int fd) ;

#endif /* _NO_PROTO */


#endif /* _audio_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
