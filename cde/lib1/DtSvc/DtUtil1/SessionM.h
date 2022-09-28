/* $XConsortium: SessionM.h /main/cde1_maint/2 1995/10/18 23:09:43 pascale $ */
/*                                                                            *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                           *
 * (c) Copyright 1993, 1994 International Business Machines Corp.             *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                            *
 * (c) Copyright 1993, 1994 Novell, Inc.                                      *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SessionM.h
 **
 **  Description:
 **  -----------
 **  Contains all variables needed for SM messaging.  All messages serviced,
 **  and the tool class name for the session manager
 **
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
 *****************************************************************************
 *************************************<+>*************************************/
#ifndef _SessionM_h
#define _SessionM_h

/*
 * Include files
 */
/* If <X11/Intrinsic.h> is included along with this file, <X11/Intrinsic.h>
   MUST be included first due to "typedef Boolean ..." problems.  The following
	  conditional #define is also part of the solution to this problem. */
#include <X11/X.h>
#include <X11/Intrinsic.h>
#ifdef _XtIntrinsic_h
#   define TOOLKIT
#endif

/*
 * Tool class for the session manager
 */
#define DtSM_TOOL_CLASS			"SESSIONMGR"


/*
 * Request types that the sesssion manager services
 */
#define DtSM_RELOAD_RESOURCES		"RELOAD_RESOURCES"
#define DtSM_LOCK_DISPLAY		"LOCK_DISPLAY"
#define DtSM_EXIT_SESSION		"EXIT_SESSION"

/*
 * SM_STARTUP_CHANGE an SM_STATE parameters
 */
#define DtSM_VERBOSE_MODE		1
#define DtSM_QUIET_MODE			2
#define DtSM_ASK_STATE			0
#define DtSM_HOME_STATE			3
#define DtSM_CURRENT_STATE		4 


/*
 * Atoms for session manager/style manager communication
 */
#define _XA_DT_SM_STM_PROTOCOL			"_DT_SM_STM_PROTOCOL"
#define _XA_DT_SM_SAVE_TO_HOME			"_DT_SM_SAVE_TO_HOME"
#define _XA_DT_SM_STATE_CHANGE			"_DT_SM_STATE_CHANGE"
#define _XA_DT_SM_RESTORE_DEFAULT		"_DT_SM_RESTORE_DEFAULT"
#define _XA_DT_SM_PREFERENCES			"_DT_SM_PREFERENCES"

/*
 * Atoms for lock changes
 */
#define _XA_DT_SM_LOCK_CHANGE		"_DT_SM_LOCK_CHANGE"
#define CoverScreenMask            	(1L<<0)
#define LockOnTimeoutMask          	(1L<<1)

/*
 * Parameters to be sent into session manager
 */

/*
 * Screen locked property
 */
#define PROP_DT_SM_LOCK_STATE_ELEMENTS     2
#define _XA_DT_SM_LOCK_STATE       "_DT_SM_LOCK_STATE"
/*
 * Defines needed for getting/setting the sm window property on the root
 */
#define PROP_DT_SM_WINDOW_INFO_ELEMENTS     2
#define _XA_DT_SM_WINDOW_INFO       "_DT_SM_WINDOW_INFO"

/*
 * Defines needed for getting the session manager state off it's window
 */
#define PROP_DT_SM_STATE_INFO_ELEMENTS     12
#define _XA_DT_SM_STATE_INFO       "_DT_SM_STATE_INFO"

/*
 * Defines needed for the screen saver list property.
 */
#define _XA_DT_SM_SAVER_INFO       "_DT_SM_SAVER_INFO"

/*
 * Defines needed for the screen saver property
 */
#define PROP_DT_SM_SCREEN_INFO_ELEMENTS     5
#define _XA_DT_SM_SCREEN_INFO       "_DT_SM_SCREEN_INFO"

/*
 * Defines needed for the audio property
 */
#define PROP_DT_SM_AUDIO_INFO_ELEMENTS     4
#define _XA_DT_SM_AUDIO_INFO       "_DT_SM_AUDIO_INFO"

/*
 * Defines needed for the keyboard property
 */
#define PROP_DT_SM_KEYBOARD_INFO_ELEMENTS     3
#define _XA_DT_SM_KEYBOARD_INFO       "_DT_SM_KEYBOARD_INFO"

/*
 * Define needed for the font info property
 */
#define _XA_DT_SM_FONT_INFO       "_DT_SM_FONT_INFO"

/*
 * Define needed for the font info property
 */
#define _XA_DT_SM_POINTER_INFO       "_DT_SM_POINTER_INFO"

/*  bit definitions for SmStateInfo.flags */
#define SM_STATE_NONE		0
#define SM_STATE_START     	(1L << 0)
#define SM_STATE_CONFIRM     	(1L << 1)
#define SM_STATE_COMPAT     	(1L << 2)
#define SM_STATE_SEND     	(1L << 3)
#define SM_STATE_COVER     	(1L << 4)   /* Obsolete */
#define SM_STATE_LOTOUT		(1L << 5)   /* Obsolete */
#define SM_STATE_LOTOUTSTAT     (1L << 6)

#define SM_STATE_CYCLETIMEOUT   (1L << 7)
#define SM_STATE_LOCKTIMEOUT    (1L << 8)
#define SM_STATE_SAVERTIMEOUT   (1L << 9)
#define SM_STATE_RANDOM         (1L << 10)


#define SM_STATE_ALL	(SM_STATE_START   | SM_STATE_CONFIRM |\
			 SM_STATE_COMPAT  | SM_STATE_SEND    |\
			 SM_STATE_COVER   | SM_STATE_LOTOUT  |\
			 SM_STATE_LOTOUTSTAT | SM_STATE_CYCLETIMEOUT |\
                         SM_STATE_LOCKTIMEOUT | SM_STATE_SAVERTIMEOUT |\
                         SM_STATE_RANDOM)


/*
 * typedef statements for structures to be returned
 */
typedef struct
{
    int		 flags;
    int		 smStartState;
    int		 smConfirmMode;
    Boolean	 smCompatMode;
    Boolean	 smSendSettings;
    Boolean	 smCoverScreen;    /* Obsolete */
    Boolean	 smLockOnTimeout;  /* Obsolete */
    Boolean	 smLockOnTimeoutStatus;
    int          smCycleTimeout;
    int          smLockTimeout;
    int          smSaverTimeout;
    Boolean      smRandom;
} SmStateInfo;

typedef struct
{
    char *saverList;
} SmSaverInfo;

typedef struct
{
    int flags;
    int smTimeout;
    int smInterval;
    int smPreferBlank;
    int smAllowExp;
} SmScreenInfo;

typedef struct
{
    int flags;
    int smBellPercent;
    unsigned int smBellPitch;
    unsigned int smBellDuration;
} SmAudioInfo;


typedef struct
{
    int flags;
    int smKeyClickPercent;
    int smGlobalAutoRepeat;
} SmKeyboardInfo;

/*
 * Function definitions
 */
#ifdef _NO_PROTO
extern Status _DtGetSmWindow();	/*
				 * Get the session manager window - defined in
				 * SmComm.c
				 */

extern Status _DtGetSmState();    /*
				  * Get the current status of the session
				  * manager - defined in SmComm.c
				  */

extern Status _DtSetSmState();    /*
                                  * Set the current status of the session
                                  * manager - defined in SmComm.c
                                  */

extern Status _DtGetSmSaver();    /*
                                  * Get the current session manager screen
                                  * saver list - defined in SmComm.c
                                  */

extern void   _DtSetSmSaver();    /*
                                  * Set the current session manager screen
                                  * saver list - defined in SmComm.c
                                  */

extern Status _DtGetSmScreen();    /*
				  * Get the current status of the screen
				  * saver - defined in SmComm.c
				  */
extern Status _DtGetSmAudio();     /*
				  * Get the current status of the audio
				  * defined in SmComm.c
				  */
extern Status _DtGetSmKeyboard();     /*
				  * Get the current status of the keyboard
				  * defined in SmComm.c
				  */
extern Status _DtGetSmFont();     /*
				  * Get the current status of the Font info
				  * defined in SmComm.c
				  */
extern Status _DtGetSmPointer();     /*
				  * Get the current status of the Pointer info
				  * defined in SmComm.c
				  */
extern Status _DtGetSmLockScreen();
#else
extern Status _DtGetSmWindow(Display *, Window, Window *) ;
extern Status _DtGetSmState(Display *, Window, SmStateInfo *);
extern Status _DtSetSmState(Display *, Window, SmStateInfo *);
extern Status _DtGetSmSaver(Display *, Window, SmSaverInfo *);
extern void   _DtSetSmSaver(Display *, Window, SmSaverInfo *);
extern Status _DtGetSmScreen(Display *, Window, Atom, SmScreenInfo *);
extern Status _DtGetSmAudio(Display *, Window, Atom, SmAudioInfo *);
extern Status _DtGetSmKeyboard(Display *, Window, Atom, SmKeyboardInfo *);
extern Status _DtGetSmFont(Display *, Window, Atom, char **);
extern Status _DtGetSmPointer(Display *, Window, Atom, char **);
extern Status _DtGetSmLockScreen(Display *, Window, Boolean *);
#endif

#endif /* _SessionM_h */



