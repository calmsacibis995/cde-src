/* $XConsortium: SmProtocol.c /main/cde1_maint/2 1995/08/30 16:30:42 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmProtocol.c
 **
 **  Project:     HP DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  This file contains all modules that handle the initialization and
 **  interning of all atoms used by the session manager.
 **
 **
 *******************************************************************
 **  (c) Copyright Hewlett-Packard Company, 1990.  All rights are  
 **  reserved.  Copying or other reproduction of this program      
 **  except for archival purposes is prohibited without prior      
 **  written consent of Hewlett-Packard Company.		     
 ********************************************************************
 **
 **
 **
 *****************************************************************************
 *************************************<+>*************************************/

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <Dt/Wsm.h>
#include <Dt/WsmP.h>
#include <Dt/Connect.h>
#include <Dt/SessionM.h>
#include <Dt/SessionP.h>
#include "Sm.h"
#include "SmProtocol.h"


/*
 * Global variable definitions
 */
Atom     XaWmProtocols;
Atom     XaWmSaveYourself;
Atom     XaWmState;
Atom     XaWmDtHints;
Atom     XaWsmPresence;
Atom     XaSmSaveMode;
Atom     XaSmRestoreMode;
Atom     XaSmStartAckWindow;
Atom     XaSmStopAckWindow;
Atom     XaWmWindowAck;
Atom     XaWmExitSession;
Atom     XaWmLockDisplay;
Atom     XaWmReady;
Atom     XaSmWmProtocol;
Atom	 XaVsmInfo;
Atom     XaDtSmStmProtocol;
Atom     XaDtSmSaveToHome;
Atom     XaDtSmStateChange;
Atom     XaDtSmRestoreDefault;
Atom     XaDtSmLockChange;
Atom	 XaDtSmStateInfo;
Atom	 XaDtSmSaverInfo;
Atom     XaDtSmScreenInfo;
Atom     XaDtSmAudioInfo;
Atom     XaDtSmKeyboardInfo;
Atom     XaDtSmFontInfo;
Atom     XaDtSmPointerInfo;
Atom	 XaSmScreenSaveRet;


/*************************************<->*************************************
 *
 *  InitProtocol ()
 *
 *
 *  Description:
 *  -----------
 *  Handles interning of atoms used by the session manager.  All should
 *  already exist because the session manager is a toolkit application.
 *  Also handles setting of selection mechanism used by session manager to
 *  distribute unique file names.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
InitProtocol()
#else
InitProtocol( void )
#endif /* _NO_PROTO */
{
    XaWmProtocols = XInternAtom (smGD.display, _XA_WM_PROTOCOLS, False);
    XaWmSaveYourself = XInternAtom (smGD.display, _XA_WM_SAVE_YOURSELF, False);
    XaWsmPresence = XInternAtom(smGD.display, _XA_DT_WORKSPACE_PRESENCE,
				False);
    XaSmSaveMode = XInternAtom(smGD.display, _XA_DT_SAVE_MODE, False);
    XaWmState = XInternAtom (smGD.display, _XA_WM_STATE, True);
    XaDtSmStmProtocol = XInternAtom (smGD.display,
				      _XA_DT_SM_STM_PROTOCOL, False);
    XaDtSmSaveToHome = XInternAtom (smGD.display,
				     _XA_DT_SM_SAVE_TO_HOME, False);
    XaDtSmStateChange = XInternAtom (smGD.display,
				      _XA_DT_SM_STATE_CHANGE, False);
    XaDtSmRestoreDefault = XInternAtom (smGD.display,
				      _XA_DT_SM_RESTORE_DEFAULT, False);
    XaDtSmLockChange = XInternAtom (smGD.display,
				     _XA_DT_SM_LOCK_CHANGE, False);
    XaDtSmScreenInfo = XInternAtom (smGD.display,
				     _XA_DT_SM_SCREEN_INFO, False);
    XaDtSmStateInfo = XInternAtom (smGD.display,
                                     _XA_DT_SM_STATE_INFO, False);
    XaDtSmSaverInfo = XInternAtom (smGD.display,
                                     _XA_DT_SM_SAVER_INFO, False);
    XaDtSmAudioInfo = XInternAtom (smGD.display,
				    _XA_DT_SM_AUDIO_INFO, False);
    XaDtSmKeyboardInfo = XInternAtom (smGD.display,
				       _XA_DT_SM_KEYBOARD_INFO, False);
    XaDtSmFontInfo = XInternAtom (smGD.display, _XA_DT_SM_FONT_INFO, False);
    XaDtSmPointerInfo = XInternAtom (smGD.display,
				      _XA_DT_SM_POINTER_INFO, False);
}

