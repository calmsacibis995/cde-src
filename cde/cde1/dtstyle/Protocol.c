/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Protocol.c
 **
 **   Project:     DT 3.0
 **
 **   Description: inter-client communication for Dtstyle
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990, 1993.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
/*$XConsortium: Protocol.c /main/cde1_maint/3 1995/10/23 11:22:23 gtsang $*/

/*+++++++++++++++++++++++++++++++++++++++*/
/* include files                         */
/*+++++++++++++++++++++++++++++++++++++++*/
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>

#include <Xm/Protocols.h>


#include <Dt/Connect.h>
#include <Dt/SessionM.h>
#include <Dt/SessionP.h>
#include <Dt/Wsm.h>
#include <Dt/WsmP.h>

#include "MainWin.h"
#include "SaveRestore.h"
#include "Main.h"

#include <Dt/Message.h>
#include <Dt/UserMsg.h>

/*+++++++++++++++++++++++++++++++++++++++*/
/* include extern functions              */
/*+++++++++++++++++++++++++++++++++++++++*/
#include "Protocol.h"

#ifndef CDE_INSTALLATION_TOP
#define CDE_INSTALLATION_TOP "/opt/dt"
#endif

#define WS_STARTUP_RETRY_COUNT	12
#define WS_STARTUP_TIME		5000
#define ERR1   ((char *)GETMESSAGE(2, 4, "Cannot communicate with the session\nmanager... Exiting."))
#define ERR2   ((char *)GETMESSAGE(2, 9, "Could not obtain screen saver information\nfrom the session manager. Start up settings\nmay be incorrect."))

/*
 * Global variable definitions
 */
Window   smWindow;

static Atom     xaWmSaveYourself;
static Atom     xaWmDeleteWindow;

/* Atoms for client messages */
static Atom     xaSmStmProtocol;
static Atom     xaSmStateChange;
static Atom     xaSmRestoreDefault;
static Atom     xaSmLockChange;
static Atom     xaSmSaveToHome;

/* Atoms for window properties */
static Atom     xaDtSaveMode;
static Atom     xaDtSmAudioInfo;
static Atom     xaDtSmKeyboardInfo;
static Atom     xaDtSmPointerInfo;
static Atom     xaDtSmScreenInfo;
static Atom     xaDtSmFontInfo;

/* local function definitions */
#ifdef _NO_PROTO
static void SmRestoreDefault();
#else
static void SmRestoreDefault(Atom);
#endif /* _NO_PROTO */


/*************************************<->*************************************
 *
 *  InitProtocol ()
 *
 *
 *  Description:
 *  -----------
 *  Handles interning of atoms for inter-client communication.
 *  Color Server
 *  Dtwm
 *  Dtsession
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
InitDtstyleProtocol()
#else
InitDtstyleProtocol( void )
#endif /* _NO_PROTO */
  
{
  /* Get Session Manager Window ID for communication */
  
  if (_DtGetSmWindow (style.display, 
		      XRootWindow(style.display,0),
		      &smWindow) == Success)
    {
      if (_DtGetSmState (style.display, smWindow, &style.smState) != Success)
	{
	  _DtSimpleError (progName, DtWarning, NULL, ERR1, NULL);
	  exit(1);
	}
      if (_DtGetSmSaver (style.display, smWindow, &style.smSaver) != Success)
	_DtSimpleError (progName, DtWarning, NULL, ERR2, NULL);
    }
  else smWindow = 0;
  
  
  xaDtSaveMode = XInternAtom(style.display, _XA_DT_SAVE_MODE, False);
  xaWmSaveYourself = XInternAtom(style.display, "WM_SAVE_YOURSELF", False);
  xaWmDeleteWindow = XInternAtom(style.display, "WM_DELETE_WINDOW", False);
  xaSmStmProtocol = XInternAtom(style.display, _XA_DT_SM_STM_PROTOCOL, False);
  xaSmSaveToHome = XInternAtom(style.display, _XA_DT_SM_SAVE_TO_HOME, False);
  xaSmStateChange = XInternAtom(style.display, _XA_DT_SM_STATE_CHANGE, False);
  xaSmRestoreDefault = XInternAtom(style.display, _XA_DT_SM_RESTORE_DEFAULT, False);
  xaSmLockChange = XInternAtom(style.display, _XA_DT_SM_LOCK_CHANGE, False);
  xaDtSmAudioInfo = XInternAtom(style.display, _XA_DT_SM_AUDIO_INFO, False);
  xaDtSmScreenInfo = XInternAtom(style.display, _XA_DT_SM_SCREEN_INFO, False);
  xaDtSmKeyboardInfo = XInternAtom(style.display, _XA_DT_SM_KEYBOARD_INFO, False);
  xaDtSmPointerInfo = XInternAtom(style.display, _XA_DT_SM_POINTER_INFO, False);
  xaDtSmFontInfo = XInternAtom(style.display,  _XA_DT_SM_FONT_INFO, False);
  
}

/*************************************<->*************************************
 *
 *  SetWindowProperites ()
 *
 *
 *  Description:
 *  -----------
 *  Add the WM_DELETE_WINDOW and WM_SAVE_YOURSELF properties to the 
 *  dtstyle main window 
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SetWindowProperties()
#else
SetWindowProperties( void )
#endif /* _NO_PROTO */
{

    /* Add WM_SAVE_YOURSELF property to the main window */
    XmAddWMProtocolCallback(style.shell, xaWmSaveYourself, saveSessionCB, NULL);

    /* Add WM_DELETE_WINDOW property to the main window */
    XmAddWMProtocolCallback(style.shell, xaWmDeleteWindow, activateCB_exitBtn, NULL);

}


/*************************************<->*************************************
 *
 *  HandleWorkspaceChange
 *
 *
 *  Description:
 *  -----------
 *  Do processing required when workspace changes. A DtWsmWsChangeProc.
 *
 *************************************<->***********************************/
static void
#ifdef _NO_PROTO
HandleWorkspaceChange (widget, aWs, client_data)
    Widget              widget;
    Atom                aWs;
    Pointer             client_data;
#else
HandleWorkspaceChange (
    Widget              widget,
    Atom                aWs,
    Pointer             client_data)
#endif /* _NO_PROTO */
{
    CheckWorkspace ();	/* Backdrop may need to update colors */
}

/*************************************<->*************************************
 *
 *  ListenForWorkspaceChange ()
 *
 *
 *  Description:
 *  -----------
 *  
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
ListenForWorkspaceChange()
#else
ListenForWorkspaceChange( void )
#endif /* _NO_PROTO */
{

  DtWsmAddCurrentWorkspaceCallback (style.shell, 
				    (DtWsmWsChangeProc) HandleWorkspaceChange, NULL);
  
}

/************************************************************************
 * WorkspaceStartupTimer()
 *
 * Check to see if the workspace manager is ready
 ************************************************************************/
static void 
#ifdef _NO_PROTO
WorkspaceStartupTimer( client_data, id )
	XtPointer client_data;
	XtIntervalId *id;
#else
WorkspaceStartupTimer( 
	XtPointer client_data,
	XtIntervalId *id)
#endif /* _NO_PROTO */
{
    Atom	     aWS;
    int count = (int) client_data;

    if (DtWsmGetCurrentWorkspace (style.display, style.root, &aWS) 
	 	== Success)
    {
	/*
	 * OK, the workspace manager is ready. Get the
	 * colors and redraw the bitmap.
	 */
        CheckWorkspace ();
    }
    else if (--count > 0)
    {
	/* wait a little longer for the workspace manager */
	client_data = (XtPointer) count;
	(void) XtAppAddTimeOut (XtWidgetToApplicationContext(style.shell), 
		WS_STARTUP_TIME, WorkspaceStartupTimer, client_data);
    }
}

/*************************************<->*************************************
 *
 *  ReparentNotify ()
 *
 *
 *  Description:
 *  -----------
 *  Dtstyle has been reparented.  
 *  The parenting happens twice when the window manager has been restarted.
 *  First Dtstyle gets reparented to the root window, then reparented to
 *  the window manager.
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
MwmReparentNotify( w, client_data, event )
        Widget w ;
        XtPointer client_data ;
        XEvent *event ;
#else
MwmReparentNotify(
        Widget w,
        XtPointer client_data,
        XEvent *event )
#endif /* _NO_PROTO */
{
    if ((event->type == ReparentNotify) &&
        (event->xreparent.parent != style.root))
    {
	if (style.backdropDialog && XtIsManaged(style.backdropDialog))
	{
	    client_data = (XtPointer) WS_STARTUP_RETRY_COUNT;
	    (void) XtAppAddTimeOut (XtWidgetToApplicationContext(style.shell), 
		WS_STARTUP_TIME, WorkspaceStartupTimer, client_data);
	}
    }
}

/*************************************<->*************************************
 *
 *  GetSessionSaveMode ()
 *
 *
 *  Description:
 *  -----------
 *  Get the session save mode from the Session Manager
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
GetSessionSaveMode( mode )
    unsigned char **mode;
#else
GetSessionSaveMode( 
    unsigned char **mode ) 
#endif /* _NO_PROTO */
{

    Atom actualType;
    int actualFormat;
    unsigned long nitems;
    unsigned long leftover;

    XGetWindowProperty(style.display, RootWindow(style.display, 0),
                         xaDtSaveMode,0L,
                         (long)BUFSIZ,False,AnyPropertyType,&actualType,
                         &actualFormat,&nitems,&leftover,
                         mode);

}


/*************************************<->*************************************
 *
 *  SmSaveHomeSession ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager to save the home session
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmSaveHomeSession( origStartState, origConfirmMode )
    int origStartState;
    int origConfirmMode;
#else
SmSaveHomeSession(
    int origStartState,
    int origConfirmMode)
#endif /* _NO_PROTO */
{
  XClientMessageEvent stmToSmMessage;

  if (smWindow != 0)
  {
   /*
    * Tell session manager save home state using current smStartState
    * and smConfirmMode. Note that the session state will retain the
    * original smStartState and smConfirmMode values.
    */
    stmToSmMessage.type = ClientMessage;
    stmToSmMessage.window = smWindow;
    stmToSmMessage.message_type = xaSmStmProtocol;
    stmToSmMessage.format = 32;
    stmToSmMessage.data.l[0] = xaSmSaveToHome;

    if(style.smState.smStartState == DtSM_CURRENT_STATE)
       stmToSmMessage.data.l[1] = DtSM_HOME_STATE;
    else
       stmToSmMessage.data.l[1] = style.smState.smStartState;

    stmToSmMessage.data.l[2] = style.smState.smConfirmMode;
    stmToSmMessage.data.l[3] = CurrentTime;
    XSendEvent(style.display, smWindow, False, NoEventMask,
                          (XEvent *) &stmToSmMessage);
  }
}                                                                 

/*************************************<->*************************************
 *
 *  SmNewStartupSettings ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager about new Startup settings
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmNewStartupSettings()
#else
SmNewStartupSettings( void )
#endif /* _NO_PROTO */
{
  SmStateInfo state;

  if (smWindow != 0)
  {
    state.flags = SM_STATE_START | SM_STATE_CONFIRM;
    state.smStartState = style.smState.smStartState;
    state.smConfirmMode = style.smState.smConfirmMode;
    _DtSetSmState(style.display, smWindow, &state);
  }
}                                                                 

/*************************************<->*************************************
 *
 *  SmRestoreDefault ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager to restore the default value to one of the settings
 *
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
SmRestoreDefault(toRestore)
Atom toRestore;
#else
SmRestoreDefault(Atom toRestore)
#endif /* _NO_PROTO */
{
     XClientMessageEvent stmToSmMessage;

    if (smWindow != 0)
    {
         stmToSmMessage.type = ClientMessage;
         stmToSmMessage.window = smWindow;
         stmToSmMessage.message_type = xaSmStmProtocol;
         stmToSmMessage.format = 32;
         stmToSmMessage.data.l[0] = xaSmRestoreDefault;
         stmToSmMessage.data.l[1] = toRestore;
         stmToSmMessage.data.l[2] = CurrentTime;
         XSendEvent(style.display, smWindow, False, NoEventMask,
                          (XEvent *) &stmToSmMessage);
    }
}                                                                 


/*************************************<->*************************************
 *
 *  SmDefaultAudioSettings ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager to set default Xserver audio settings
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmDefaultAudioSettings()
#else
SmDefaultAudioSettings( void )
#endif /* _NO_PROTO */
{
    if (smWindow != 0)
    {
        /*  Delete the property to indicate default settings to SM */
        SmRestoreDefault(xaDtSmAudioInfo);
    }
}                                                                 

/*************************************<->*************************************
 *
 *  SmNewAudioSettings ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager about new Xserver audio settings
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmNewAudioSettings( volume, tone, duration )
    int volume;
    int tone;
    int duration;
#else
SmNewAudioSettings(
    int volume,
    int tone,
    int duration )
#endif /* _NO_PROTO */
{

    PropDtSmAudioInfo	audioProp;

    if (smWindow != 0)
    {
        /*
         * Set the property on the Session Manager window
         * indicating the new screen saver settings
         */

        audioProp.flags = 0;
        audioProp.smBellPercent = (CARD32) volume;
        audioProp.smBellPitch = (CARD32) tone;
        audioProp.smBellDuration = (CARD32) duration;
        XChangeProperty (style.display, smWindow,
                         xaDtSmAudioInfo, 
                         xaDtSmAudioInfo,
                         32, PropModeReplace, 
                         (unsigned char *)&audioProp,
                         PROP_DT_SM_AUDIO_INFO_ELEMENTS);
    }
}                                                                 

/*************************************<->*************************************
 *
 *  SmDefaultScreenSettings ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager to set default Xserver screen settings
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmDefaultScreenSettings()
#else
SmDefaultScreenSettings( void )
#endif /* _NO_PROTO */
{
    if (smWindow != 0)
    {
        /*  Delete the property to indicate default settings to SM */
        SmRestoreDefault(xaDtSmScreenInfo);
    }
}                                                                 


/*************************************<->*************************************
 *
 *  SmNewScreenSettings ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager about new Xserver screen settings
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmNewScreenSettings( timeout, blanking, interval, exposures)
    int timeout;
    int blanking;
    int interval;
    int exposures;
#else
SmNewScreenSettings( 
    int timeout,
    int blanking,
    int interval,
    int exposures )
#endif /* _NO_PROTO */

{

    PropDtSmScreenInfo	screenProp;

    if (smWindow != 0)
    {
        /*
         * Set the property on the Session Manager window
         * indicating the new screen saver settings
         */

        screenProp.flags = 0;
        screenProp.smTimeout = (CARD32) timeout;  /* 0-7200 */
        screenProp.smInterval = (CARD32) interval;    /* -1  */
        screenProp.smPreferBlank = (CARD32) blanking; /* 0,1 */
        screenProp.smAllowExp = (CARD32) exposures;
        XChangeProperty (style.display, smWindow,
                         xaDtSmScreenInfo, 
                         xaDtSmScreenInfo,
                         32, PropModeReplace, 
                         (unsigned char *)&screenProp,
                         PROP_DT_SM_SCREEN_INFO_ELEMENTS);
    }
}                                                                 

/*************************************<->*************************************
 *
 *  SmNewSaverSettings ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager about new saver settings: 
 *  saver timeout, lock timeout, cycle timeout, selected saver list
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmNewSaverSettings(saverTime, lockTime, cycleTime, selsaversList)
    int saverTime;
    int lockTime;
    int cycleTime;
    char *selsaversList;
#else
SmNewSaverSettings( 
    int saverTime,
    int lockTime,
    int cycleTime,		   
    char *selsaversList)
#endif /* _NO_PROTO */
{
  SmStateInfo state;     /* structure that will contain new state info */
  SmSaverInfo saver;     /* structure that will contain selected saver list */

  int saverTime_change;  /* saver timeout */
  int lockTime_change;   /* lock timeout */
  int cycleTime_change;  /* saver timeout */

  if (smWindow != 0)
    {
      lockTime_change = (style.smState.smLockTimeout != lockTime);
      saverTime_change = (style.smState.smSaverTimeout != saverTime);
      cycleTime_change = (style.smState.smCycleTimeout != cycleTime);
      state.flags = (saverTime_change ? SM_STATE_SAVERTIMEOUT : 0) | 
                    (lockTime_change ?  SM_STATE_LOCKTIMEOUT  : 0) |
		    (cycleTime_change ? SM_STATE_CYCLETIMEOUT : 0);
      
      if (state.flags)
	{
	  state.smSaverTimeout = saverTime;
	  state.smLockTimeout = lockTime;
	  state.smCycleTimeout = cycleTime;
	  
	  _DtSetSmState(style.display, smWindow, &state);
	}
      saver.saverList = selsaversList;
      _DtSetSmSaver(style.display, smWindow, &saver);
    }
  
}


/*************************************<->*************************************
 *
 *  SmNewSaverTime()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager about new saver timeout
 *  used for telling the session manager if it should run savers or not when
 *  the user enables/disables saver toggle (or the savers toggle in no saver
 *  extension mode) without having to press OK. A zero is sent to the session 
 *  manager when no savers should be run.  
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmNewSaverTime(saverTime)
    int saverTime;
#else
SmNewSaverTime(int saverTime)
#endif /* _NO_PROTO */
{
  SmStateInfo state;     /* structure that will contain new state info */

  if (smWindow != 0)
    {
      state.flags = SM_STATE_SAVERTIMEOUT;
      state.smSaverTimeout = saverTime;
      _DtSetSmState(style.display, smWindow, &state);
    }
}



/*************************************<->*************************************
 *
 *  SmDefaultKeyboardSettings ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager to set default Xserver Keyboard settings
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmDefaultKeyboardSettings()
#else
SmDefaultKeyboardSettings( void )
#endif /* _NO_PROTO */
{
    if (smWindow != 0)
    {
        /*  Delete the property to indicate default settings to SM */
        SmRestoreDefault(xaDtSmKeyboardInfo);
    }
}                                                                 

/*************************************<->*************************************
 *
 *  SmNewKeyboardSettings ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager about new Xserver Keyboard settings
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmNewKeyboardSettings( keyClickPercent, autoRepeat )
    int keyClickPercent;
    int  autoRepeat;
#else
SmNewKeyboardSettings(
    int keyClickPercent,
    int  autoRepeat)
#endif /* _NO_PROTO */
{

    PropDtSmKeyboardInfo	KeyboardProp;

    if (smWindow != 0)
    {
        /*
         * Set the property on the Session Manager window
         * indicating the new screen saver settings
         */

        KeyboardProp.flags = 0;
        KeyboardProp.smKeyClickPercent = (CARD32) keyClickPercent;
        KeyboardProp.smGlobalAutoRepeat = (CARD32) autoRepeat;
        XChangeProperty (style.display, smWindow,
                         xaDtSmKeyboardInfo, 
                         xaDtSmKeyboardInfo,
                         32, PropModeReplace, 
                         (unsigned char *)&KeyboardProp,
                         PROP_DT_SM_KEYBOARD_INFO_ELEMENTS);
    }
}                                                                 

/*************************************<->*************************************
 *
 *  SmDefaultPointerSettings ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager to set default Xserver Pointer settings
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmDefaultPointerSettings()
#else
SmDefaultPointerSettings( void )
#endif /* _NO_PROTO */
{
    if (smWindow != 0)
    {
        /*  Delete the property to indicate default settings to SM */
        SmRestoreDefault(xaDtSmPointerInfo);
    }
}                                                                 

/*************************************<->*************************************
 *
 *  SmNewPointerSettings ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager about new Xserver Pointer settings
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmNewPointerSettings( pointerString )
    char *pointerString;
#else
SmNewPointerSettings(
    char *pointerString)
#endif /* _NO_PROTO */
{

    if (smWindow != 0)
    {
        /*
         * Set the property on the Session Manager window
         * indicating the new screen saver settings
         */

        XChangeProperty (style.display, smWindow,
                         xaDtSmPointerInfo, 
                         XA_STRING,
                         8, PropModeReplace, 
                         (unsigned char *)pointerString,
                         strlen(pointerString));
    }
}                                                                 

/*************************************<->*************************************
 *
 *  SmNewFontSettings ()
 *
 *  Description:
 *  -----------
 *  Tell Session Manager about new font resources
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SmNewFontSettings( fontResourceString )
    char *fontResourceString;
#else
SmNewFontSettings(
    char *fontResourceString)
#endif /* _NO_PROTO */
{

                     
    if (smWindow != 0)
    {
        /*
         * Set the property on the Session Manager window
         * indicating the new font resource string
         */

        XChangeProperty (style.display, smWindow,
                         xaDtSmFontInfo, 
                         XA_STRING,
                         8, PropModeReplace, 
                         (unsigned char *)fontResourceString,
                         strlen(fontResourceString));
    }
}                                                                 

