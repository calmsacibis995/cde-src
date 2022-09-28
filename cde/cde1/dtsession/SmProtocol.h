/* $XConsortium: SmProtocol.h /main/cde1_maint/2 1995/08/30 16:31:00 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SmProtocol.h
 **
 **   Project:     HP DT
 **
 **   Description
 **   -----------
 **   Variables and declarations needed for
 **   Protocol Handling for the session manager
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _smprotocol_h
#define _smprotocol_h
 
/* 
 *  #include statements 
 */
#include <X11/Xlib.h>


/* 
 *  #define statements 
 */

/*
 * Property types used by the session manager not defined elsewhere
 */
#define	_XA_WM_PROTOCOLS		"WM_PROTOCOLS"
#define _XA_WM_STATE			"WM_STATE"
#define _XA_WM_SAVE_YOURSELF		"WM_SAVE_YOURSELF"



/* 
 * typedef statements 
 */

/*
 *  External variables  
 */

/*
 * Atoms interned by the session manager in order to communicate with
 * the other clients in the session
 */
extern Atom		XaWmProtocols;
extern Atom		XaWmSaveYourself;
extern Atom		XaWmState;
extern Atom     	XaWmDtHints;
extern Atom		XaWsmPresence;
extern Atom		XaSmSaveMode;
extern Atom		XaSmRestoreMode;
extern Atom		XaSmStartAckWindow;
extern Atom		XaSmStopAckWindow;
extern Atom		XaWmWindowAck;
extern Atom		XaWmExitSession;
extern Atom		XaWmLockDisplay;
extern Atom		XaWmReady;
extern Atom		XaSmWmProtocol;
extern Atom		XaVsmInfo;
extern Atom    	 	XaDtSmStateInfo;
extern Atom    	 	XaDtSmSaverInfo;
extern Atom     	XaDtSmStmProtocol;
extern Atom     	XaDtSmSaveToHome;
extern Atom     	XaDtSmStateChange;
extern Atom     	XaDtSmRestoreDefault;
extern Atom     	XaDtSmLockChange;
extern Atom     	XaDtSmScreenInfo;
extern Atom     	XaDtSmAudioInfo;
extern Atom     	XaDtSmKeyboardInfo;
extern Atom     	XaDtSmFontInfo;
extern Atom     	XaDtSmPointerInfo;
extern Atom	 	XaSmScreenSaveRet;

/*  
 *  External Interface  
 */

#ifdef _NO_PROTO

extern void InitProtocol();

#else

extern void InitProtocol(void);

#endif /*_NO_PROTO*/

#endif /*_smprotocols_h*/
/* DON'T ADD ANYTHING AFTER THIS #endif */
