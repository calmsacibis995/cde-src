/* $XConsortium: Protocol.h /main/cde1_maint/2 1995/10/23 11:23:21 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Protocol.h
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
#ifndef _protocol_h
#define _protocol_h

extern Window   smWindow;


/* External Interface */

#ifdef _NO_PROTO

extern void MwmReparentNotify() ;
extern void GetSessionSaveMode() ;
extern void InitDtstyleProtocol() ;
extern void SetWindowProperties() ;
extern void SmSaveHomeSession() ;
extern void SmDefaultAudioSettings() ;
extern void SmDefaultScreenSettings() ;
extern void SmDefaultKeyboardSettings() ;
extern void SmNewAudioSettings() ;
extern void SmNewScreenSettings() ;
extern void SmNewLockSettings() ;
extern void SmNewKeyboardSettings() ;
extern void SmNewPointerSettings() ;
extern void SmNewStartupSettings() ;
extern void SmNewFontSettings( ) ;
#else

extern void MwmReparentNotify(
                            Widget w,
                            XtPointer client_data,
                            XEvent *event ) ;
extern void InitDtstyleProtocol( void ) ;
extern void GetSessionSaveMode(  unsigned char **mode ) ;
extern void SetWindowProperties( void ) ;
extern void SmSaveHomeSession(  
                                int origStartState,
                                int origConfirmMode);
extern void SmDefaultAudioSettings( void ) ;
extern void SmDefaultScreenSettings( void ) ;
extern void SmDefaultKeyboardSettings( void ) ;
extern void SmNewAudioSettings(
                                int volume,
                                int tone,
                                int duration ) ;

extern void SmNewScreenSettings(
                                int timeout,
                                int blanking,
                                int interval,
                                int exposures ) ;
extern void SmNewLockSettings(
                                int lock,
                                int cover ) ;

extern void SmNewKeyboardSettings(
                                int keyClickPercent,
                                int  autoRepeat) ;
extern void SmNewPointerSettings(
                                char *pointerString) ;
extern void SmNewStartupSettings( void ) ;
extern void SmNewFontSettings( 
                                char *fontResourceString) ;

#endif /* _NO_PROTO */


#endif /* _protocol_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
