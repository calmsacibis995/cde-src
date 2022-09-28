/* $XConsortium: SmCommun.c /main/cde1_maint/4 1995/10/09 10:56:27 pascale $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmCommun.c
 **
 **  Project:     HP DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  This file contains functionality needed to communicate with the
 **  other DT components.  This includes initialization and callback code.
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
 **
 *****************************************************************************
 *************************************<+>*************************************/

#include <stdio.h>
#ifdef _SUN_OS /* for the strtok calls */
#include <string.h>
#endif
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#if defined (USE_X11SSEXT)
#include <X11/extensions/scrnsaver.h>
#endif /* USE_X11SSEXT */
#include <Xm/Xm.h>
#include <Dt/DtP.h>
#include <Dt/Action.h>
#include <Dt/SessionM.h>
#include <Dt/StandardM.h>
#include <Dt/UserMsg.h>
#include <Dt/Indicator.h>
#include <Tt/tttk.h>
#include "Sm.h"
#include "SmError.h"
#include "SmLock.h"
#include "SmCommun.h"
#include "SmRestore.h"
#include "SmSave.h"
#include "SmUI.h"
#include "SmProtocol.h"
#include "SmGlobals.h"

extern int  clientRunning; /* from SmConMgmt.c */
extern char **smExecArray;

/*
 * Defines for this file only
 */
#define	SS_ON		0
#define	SS_OFF		1
#define	SS_DRAW		2

/*
 * Local function definitions
 */
#ifdef _NO_PROTO

static void DtwmStarted() ;

#else

static void DtwmStarted(void);

#endif


/*************************************<->*************************************
 *
 *  handleSessionMgrRequest ()
 *
 *
 *  Description:
 *  -----------
 *  Handle ToolTalk requests for which the session manager is responsible.
 *
 *
 *  Inputs:
 *  ------
 *  Incoming request, and the pattern it matched.
 *
 * 
 *  Outputs:
 *  -------
 *  Whether the message has been consumed.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
Tt_callback_action
#ifdef _NO_PROTO
handleSessionMgrRequest(msg, pat)
	Tt_message msg;
	Tt_pattern pat;
#else
handleSessionMgrRequest(
	Tt_message msg,
	Tt_pattern pat
)
#endif /* _NO_PROTO */
{
  char *op;
  Tt_status status;
  Tt_callback_action rc;
  Boolean destroyMsg;

  if (tt_message_state( msg ) != TT_SENT) 
  {
    /* msg is a reply to ourself */
    return TT_CALLBACK_CONTINUE;
  }

  op = tt_message_op( msg );
  status = tt_ptr_error( op );
  if ((status != TT_OK) || (op == 0)) 
  {
    /* Let tttk_Xt_input_handler() Do The Right Thing */
    return TT_CALLBACK_CONTINUE;
  }
    
  destroyMsg = True;
  if (strcmp( op, "Display_Lock" ) == 0) 
  {
    if(smGD.smState == READY) 
    {
      smGD.smState = IN_PROCESS;
      LockDisplay(True);
    } else if(smGD.coverScreen == TRUE && smGD.lockedState == UNLOCKED) {
      smGD.lockedState = LOCKED;
    }
    rc = TT_CALLBACK_PROCESSED;
  }
  else if (strcmp( op, "XSession_Exit" ) == 0) 
  {
    if(smGD.smState == READY) 
    {
      smGD.smState = IN_PROCESS;
      ExitSession( msg );
      destroyMsg = False; /* done in SmUI.c */
    }
    rc = TT_CALLBACK_PROCESSED;
  } 
  else if (strcmp( op, "Resources_Reload" ) == 0) 
  {
    if(smGD.smState == READY) 
    {
      smGD.smState = IN_PROCESS;
      ReloadResources();
    }
    rc = TT_CALLBACK_PROCESSED;
  } 
  else 
  {
    rc = TT_CALLBACK_CONTINUE;
    destroyMsg = False;
  }

  if (destroyMsg == True)
  {
    tt_message_reply( msg );
    tt_message_destroy( msg );
  }
        
  tt_free( op );
  return (rc);
}


/*************************************<->*************************************
 *
 *  StartMsgServer ()
 *
 *
 *  Description:
 *  -----------
 *  Initialize the BMS and register the session manager with it.  Then
 *  register all requests and notifications that the session manager is
 *  interested in.
 *
 *
 *  Inputs:
 *  ------
 *  app = Application context for dtsession
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
StartMsgServer()
#else
StartMsgServer(void)
#endif /* _NO_PROTO */
{
    String	tmpString;
    char *	sessId;
    char *	procId;
    int		fd;
    Tt_status	status;
   
 
    procId = tt_open();
    status = tt_ptr_error( procId );
    if (status == TT_OK) {
	fd = tt_fd();
	status = tt_int_error( fd );
	if (status == TT_OK) {
	    XtAppAddInput( smGD.appCon, fd, (XtPointer)XtInputReadMask,
			   tttk_Xt_input_handler, procId );
	}
    }
    if (status != TT_OK) {
	smGD.bmsDead = True;
    }

    smGD.requests2Handle = 0;
    if (status == TT_OK) {
	smGD.requests2Handle = tt_pattern_create();
	tt_pattern_category_set( smGD.requests2Handle, TT_HANDLE );
	tt_pattern_class_add( smGD.requests2Handle, TT_REQUEST );
	tt_pattern_scope_add( smGD.requests2Handle, TT_SESSION );
	sessId = tt_default_session();
	tt_pattern_session_add( smGD.requests2Handle, sessId );
	tt_free( sessId );
	tt_pattern_op_add( smGD.requests2Handle, "Display_Lock" );
	tt_pattern_op_add( smGD.requests2Handle, "XSession_Exit" );
	tt_pattern_op_add( smGD.requests2Handle, "Resources_Reload" );
	tt_pattern_callback_add( smGD.requests2Handle,
				 handleSessionMgrRequest );
	status = tt_pattern_register( smGD.requests2Handle );
	if (status != TT_OK) {
	    smGD.bmsDead = True;
	}
    }


    if (smGD.bmsDead) {
	tmpString = 
	    SmNewString(((char *)
			 GETMESSAGE
			 (6, 1, 
			 "Unable to start message server - exiting.")));
	PrintError(DtError, tmpString);
	SM_FREE(tmpString);
	WarnNoStartup();
    }
} /* END OF FUNCTION StartMsgServer  */


/*************************************<->*************************************
 *
 *  DtwmStarted (fields, client_data, num_words)
 *
 *
 *  Description:
 *  -----------
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
static void 
DtwmStarted()
{
    smGD.dtwmRunning = True;
} /* END OF FUNCTION  DtwmStarted */


/*************************************<->*************************************
 *
 *  RestoreDefaults ()
 *
 *
 *  Description:
 *  -----------
 *  A request has come in (usually from the customizer) to restore one of
 *  the settings to their default states
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  smToSet = (global) turns off flag of setting no longer set
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
RestoreDefaults(toRestore)
Atom	toRestore;
#else
RestoreDefaults(
		Atom toRestore)
#endif /* _NO_PROTO */
{

  if(toRestore == XaDtSmScreenInfo)
  {
    smToSet.screenSavChange = False;
  }
  else if(toRestore == XaDtSmAudioInfo)
  {
    smToSet.audioChange = False;
  }
  else if(toRestore == XaDtSmKeyboardInfo)
  {
    smToSet.keyboardChange = False;
  }
  else if(toRestore == XaDtSmPointerInfo)
  {
    smToSet.pointerChange = False;
    smToSet.pointerMapChange = False;
  }
} /* END OF FUNCTION RestoreDefaults  */


/*************************************<->*************************************
 *
 *  ProcessPropertyNotify ()
 *
 *
 *  Description:
 *  -----------
 *  The customizer has changed one of the properties on the sm top level
 *  window.  This fact is remembered, so that at shutdown the information
 *  can be saved
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *  smToSet = (global) turns on flag of setting being set
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
ProcessPropertyNotify(next)
        XEvent *next;
#else
ProcessPropertyNotify(
	XEvent *next)
#endif /* _NO_PROTO */
{
  XPropertyEvent *pEvent = (XPropertyEvent *) next;

  if (pEvent->state != PropertyNewValue)
  {
    return;
  }

  if(pEvent->atom == XaDtSmScreenInfo)
  {
    smToSet.screenSavChange = True;
    smCust.screenSavChange = True;
  }
  else if(pEvent->atom == XaDtSmAudioInfo)
  {
    smToSet.audioChange = True;
    smCust.audioChange = True;
  }
  else if(pEvent->atom == XaDtSmKeyboardInfo)
  {
    smToSet.keyboardChange = True;
    smCust.keyboardChange = True;
  }
  else if(pEvent->atom == XaDtSmFontInfo)
  {
    smCust.fontChange = True;
  }
  else if(pEvent->atom == XaDtSmPointerInfo)
  {
    smToSet.pointerChange = True;
    smCust.pointerChange = True;
    smToSet.pointerMapChange = True;
    smCust.dClickChange = True;
  }
} /* END OF FUNCTION RecordChanges   */



/*************************************<->*************************************
 *
 *  ProcessClientMessage(next)
 *
 *
 *  Description:
 *  -----------
 *  A client message has come from somewhere.  Process it if we know how.
 *
 *  Inputs:
 *  ------
 *  next - the client message event
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
ProcessClientMessage(next)
        XEvent *next;
#else
ProcessClientMessage(
		     XEvent *next)
#endif /* _NO_PROTO */
{
  XClientMessageEvent	*cEvent = (XClientMessageEvent *) next;
  char		newRes[120];

 /*
  * If this event came from the style manager - process it
  */
  if(cEvent->message_type == XaDtSmStmProtocol)
  {
    if(cEvent->data.l[0] == XaDtSmSaveToHome)
    {
     /*
      * The style manager is asking for a save home session
      */
      if(smGD.smState == READY)
      {
        int startStateOrig;
        int confirmModeOrig;
	Tt_message msg;

        smGD.smState = IN_PROCESS;
	msg = tttk_message_create( 0, TT_NOTICE, TT_SESSION, 0,
				   "DtActivity_Beginning", 0 );
	tt_message_send( msg );
	tt_message_destroy( msg );

        startStateOrig = smSettings.startState;
        confirmModeOrig = smSettings.confirmMode;

        smSettings.startState = cEvent->data.l[1];
        smSettings.confirmMode = cEvent->data.l[2];;
                
	SaveState(True, smSettings.startState);

        smSettings.startState = startStateOrig;
        smSettings.confirmMode = confirmModeOrig;

	msg = tttk_message_create( 0, TT_NOTICE, TT_SESSION, 0,
				   "DtActivity_Began", 0 );
	tt_message_send( msg );
	tt_message_destroy( msg );
        SetSystemReady();
      }
    }
    else if (cEvent->data.l[0] == XaDtSmRestoreDefault)
    {
      RestoreDefaults((Atom) cEvent->data.l[1]);
    }
  }
  else if(cEvent->message_type == XaSmWmProtocol)
  {
   /*
    * If this event came from the ws manager - process it
    */
    if(cEvent->data.l[0] == XaWmExitSession)
    {
     /*
      * The ws manager is sending an exit session message
      */
      if(smGD.smState == READY)
      {
        smGD.smState = IN_PROCESS;
        ExitSession(0);
      }
    }
    else if(cEvent->data.l[0] == XaWmLockDisplay)
    {
     /*
      * The ws manager is sending an lock display message
      */
      if(smGD.smState == READY)
      {
        smGD.smState = IN_PROCESS;
        LockDisplay(True);
      }
    }
    else if(cEvent->data.l[0] == XaWmWindowAck)
    {
     /*
      * The ws manager is sending a "client has been managed"
      */
      clientRunning = True;
    }
    else if(cEvent->data.l[0] == XaWmReady)
    {
     /*
      * The ws manager is sending a "ready for clients"
      */
      smGD.dtwmRunning = True;
    }
  }
  else if (cEvent->message_type == XaDtSmStateInfo)
  {
    SmStateInfo state;
    int flags = (int)cEvent->data.l[0];

   /*
    * Session state has been changed. Get new values.
    */
    if(_DtGetSmState(smGD.display, smGD.topLevelWindow, &state) == Success)
    {
     /*
      * Copy selected changes to data areas.
      */
      if (flags & SM_STATE_START)
      {
        smSettings.startState = state.smStartState;
      }

      if (flags & SM_STATE_CONFIRM)
      {
        smSettings.confirmMode = state.smConfirmMode;
      }

      newRes[0] = '\0';
      if (flags & SM_STATE_CYCLETIMEOUT)
      {
        smSaverRes.cycleTimeout = state.smCycleTimeout;
        sprintf(newRes+strlen(newRes), "dtsession*%scycleTimeout: %d\n",
                smGD.extensionSpec,
                smSaverRes.cycleTimeout/60);
      }

      if (flags & SM_STATE_LOCKTIMEOUT)
      {
        smSaverRes.lockTimeout = state.smLockTimeout;
        sprintf(newRes+strlen(newRes), "dtsession*%slockTimeout: %d\n",
                smGD.extensionSpec,
                smSaverRes.lockTimeout/60);
      }

      if (flags & SM_STATE_SAVERTIMEOUT)
      {
        smSaverRes.saverTimeout = state.smSaverTimeout;
        sprintf(newRes+strlen(newRes), "dtsession*%ssaverTimeout: %d\n",
                smGD.extensionSpec,
                smSaverRes.saverTimeout/60);
      }

      if (flags & SM_STATE_RANDOM)
      {
        smSaverRes.random = state.smRandom;
        sprintf(newRes+strlen(newRes), "dtsession*%srandom: %s\n",
                smGD.extensionSpec,
                smSaverRes.random ? "True\n" : "False\n");
      }

      if (newRes[0] != '\0')
      {
        _DtAddToResource(smGD.display, newRes);
      }
    }
  }
  else if (cEvent->message_type == XaDtSmSaverInfo)
  {
    SmSaverInfo saver;

   /*
    * Session screen saver list has been changed. Get new values.
    */
    if(_DtGetSmSaver(smGD.display, smGD.topLevelWindow, &saver) == Success)
    {
      char *pRes;

      SM_FREE(smGD.saverList);
      smGD.saverList = SmNewString(saver.saverList);
      SM_FREE(saver.saverList);

      pRes = malloc(strlen("dtsession*saverList: ") +
                    strlen(smGD.saverList) +
                    strlen(smGD.extensionSpec) +
                    2); /* for the '/n' and '/0' */
      if (pRes)
      {
        sprintf(pRes, "dtsession*%ssaverList: %s\n",
               smGD.extensionSpec,
               smGD.saverList);
        _DtAddToResource(smGD.display, pRes);
        free(pRes);
      }

      if (smGD.saverListParse)
      {
        SM_FREE(smGD.saverListParse);
        smGD.saverListParse = NULL; 
      }
    }
  }
#if defined (USE_HPSSEXT)
  else if(cEvent->message_type == XaSmScreenSaveRet)
  {
    if(cEvent->data.l[0] == SS_ON)
    {
      if (smSaverRes.saverTimeout + smSaverRes.lockTimeout > 0 &&
          smGD.smState == READY)
      {
        smGD.smState = IN_PROCESS;
        LockDisplay(False);
      }
    }
  }
#endif /* USE_HPSSEXT */

  return;

} /* END OF FUNCTION ProcessClientMessage  */

/*************************************<->*************************************
 *
 *  ProcessScreenSaverMessage(next)
 *
 *
 *  Description:
 *  -----------
 *  A screen saver message has come from the server.  Process it if we know how.
 *
 *  Inputs:
 *  ------
 *  next - the client message event
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
#if defined (USE_X11SSEXT)

void
#ifdef _NO_PROTO
ProcessScreenSaverMessage(next)
        XEvent *next;
#else
ProcessScreenSaverMessage(
                     XEvent *next)
#endif /* _NO_PROTO */
{
    XScreenSaverNotifyEvent *ssEvent =  (XScreenSaverNotifyEvent *) next;
    static int          ssCount = 0;

    switch (ssEvent->state) {
    case ScreenSaverOn:
	/*
    	 * Screen saver activated. 
    	 */
	if (smSaverRes.saverTimeout + smSaverRes.lockTimeout > 0 &&
                smGD.smState == READY)
	{
	    /*
	     * Resource says to lock the display and SM is ready, lock it.
	     */
	    smGD.smState = IN_PROCESS;
	    LockDisplay(False);
	}
	break;

    case ScreenSaverOff:
	/*
	 * End of screen save period
	 */
	EventDetected(NULL, NULL, NULL, NULL);
	break;

    default:
	break;
    }
}
#endif /* USE_X11SSEXT */

/*************************************<->*************************************
 *
 *  ProcessReloadActionsDatabase(void)
 *
 *
 *  Description:
 *  -----------
 *  Register for notification of action database changes and load the action
 *  database into our address space. This function can be called directly
 *  by session mgr code, or called as a callback by the actions database.
 *
 *  Inputs:
 *  ------
 *
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  The first time in, this call registers itself as a callback routine with
 *  the actions database. The first and subsequent times in, this call will
 *  load the actions database into the session mgr address space.
 *
 *************************************<->***********************************/

void
#ifdef _NO_PROTO
ProcessReloadActionsDatabase()
#else
ProcessReloadActionsDatabase(void)
#endif /* _NO_PROTO */
{
  static int needtoregister = 1;

  if (needtoregister)
  {
   /*
    * Have not yet registered with the actions database to call this
    * callback when the database changes. Do so.
    */
    DtDbReloadNotify((DtDbReloadCallbackProc) ProcessReloadActionsDatabase,
			(XtPointer) NULL);
    needtoregister = 0;
  }

 /*
  * Our copy of the actions database must be out of date. Reload.
  */
  DtDbLoad();  
}

void
#ifndef _NO_PROTO
ProcessEvent(w, client_data, event, continue_to_dispatch)
  Widget w;
  XtPointer client_data;
  XEvent *event;
  Boolean *continue_to_dispatch;
#else
ProcessEvent(
  Widget w,
  XtPointer client_data,
  XEvent *event,
  Boolean *continue_to_dispatch)
#endif
{
  switch(event->type)
  {
    case ClientMessage:
      ProcessClientMessage(event);
      break;
    case PropertyNotify:
      ProcessPropertyNotify(event);
      break;
    default:
#if defined (USE_X11SSEXT)
      if (event->type == smGD.ssEventType)
        ProcessScreenSaverMessage(event);
#endif
      break;
  }
}
