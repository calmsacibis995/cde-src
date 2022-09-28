/* $XConsortium: WmIPC.c /main/cde1_maint/2 1995/10/09 13:43:45 pascale $ */
/* 
 * (c) Copyright 1987, 1988, 1989, 1990, 1992, 1993 HEWLETT-PACKARD COMPANY 
 * ALL RIGHTS RESERVED 
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = ""
#endif
#endif

/*
 * Included Files:
 */
#include "WmGlobal.h"
#include <Dt/DtP.h>
#include <Dt/StandardM.h>
#include <Dt/Action.h>
#include <Dt/WsmM.h>
#include <Dt/IndicatorM.h>
#include <Dt/UserMsg.h>
#include <Dt/Icon.h>
#include <Dt/ServiceP.h>

#include "WmBackdrop.h"
#include "WmError.h"
#include "WmFunction.h"
#include "WmWrkspace.h"
#include "WmIPC.h"
#include "DataBaseLoad.h"


/*
 * include extern functions and definitions
 */
#ifdef _NO_PROTO

extern void UpdateFileTypeControlFields ();
extern WmScreenData * GetScreenForWindow ();

#else

extern void UpdateFileTypeControlFields ( void );
extern WmScreenData * GetScreenForWindow (Window);

#endif /* _NO_PROTO */


/*
 *   data for the "DT screen"
 */


/*
 * globals
 */
Const char *szWM_TOOL_CLASS = DtWM_TOOL_CLASS;



/******************************<->*************************************
 *
 *  dtInitialize (char * program_name, XtAppContext appContext)
 *
 *  Description:
 *  -----------
 *  Initialize the messaging mechanism
 *
 *  Inputs:
 *  ------
 *  program_name - argv[0]
 *  appContext   - used throughout the WM
 *
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *
 ******************************<->***********************************/
void 
#ifdef _NO_PROTO
dtInitialize( program_name, appContext )
        char *program_name ;
        XtAppContext appContext ;
#else
dtInitialize(
        char *program_name,
        XtAppContext appContext )
#endif /* _NO_PROTO */
{
    
    (void) DtAppInitialize(appContext, DISPLAY, wmGD.topLevelW1, 
				program_name, (char *)szWM_TOOL_CLASS);

    /* 
     * Load action definitions from the action database.
     */
#ifdef DT_PERFORMANCE
_DtPerfChkpntMsgSend("Begin action database load");
#endif

    DtDbLoad(); 

#ifdef DT_PERFORMANCE
_DtPerfChkpntMsgSend("End   action database load");
#endif

} /* END OF FUNCTION dtInitialize */

/******************************<->*************************************
 *
 *  dtInitializeMessaging (void)
 *
 *  Description:
 *  -----------
 *  Initialize the messaging mechanisms
 *
 *  Inputs:
 *  ------
 *
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *
 ******************************<->***********************************/

void
#ifdef _NO_PROTO
dtInitializeMessaging()
#else
dtInitializeMessaging(void)
#endif /* _NO_PROTO */
{
    int 		i;
    WmScreenData *	pSD;
    String		sName;

    Tt_status		status;
    Tt_pattern		notice_pattern, request_pattern;
    char		*default_session;

    int			fd;
    char		*procId;

#ifdef _NO_PROTO
    Tt_callback_action NoticeMsgCB();
    Tt_callback_action RequestMsgCB();
#else
    Tt_callback_action NoticeMsgCB(
	Tt_message m,
	Tt_pattern p);
    Tt_callback_action RequestMsgCB(
	Tt_message m,
	Tt_pattern p);
#endif /* _NO_PROTO */

    /*
     * Makef sure we have a ToolTalk connection
     */
    procId = tt_open();
    status = tt_ptr_error(procId);
    if (status != TT_OK) {
	return;
    }
    fd = tt_fd();
    status = tt_int_error(fd);
    if (status == TT_OK) {
	XtAppAddInput(XtWidgetToApplicationContext(wmGD.topLevelW), fd,
		(XtPointer)XtInputReadMask, tttk_Xt_input_handler, procId);
    }

    default_session = tt_default_session();
    status = tt_ptr_error(default_session);
    if (status != TT_OK) {
	return;
    }

    notice_pattern = tt_pattern_create();
    status = tt_ptr_error(notice_pattern);
    if (status != TT_OK) {
	return;
    }
    request_pattern = tt_pattern_create();
    status = tt_ptr_error(request_pattern);
    if (status != TT_OK) {
	return;
    }

    if (tt_pattern_category_set(notice_pattern, TT_OBSERVE) != TT_OK) {
	return;
    }
    if (tt_pattern_category_set(request_pattern, TT_HANDLE) != TT_OK) {
	return;
    }
    if (tt_pattern_scope_add(notice_pattern, TT_SESSION) != TT_OK) {
	return;
    }
    if (tt_pattern_scope_add(request_pattern, TT_SESSION) != TT_OK) {
	return;
    }
    if (tt_pattern_session_add(notice_pattern, default_session) != TT_OK) {
	return;
    }
    if (tt_pattern_session_add(request_pattern, default_session) != TT_OK) {
	return;
    }
    tt_free( default_session );
    if (tt_pattern_class_add(notice_pattern, TT_NOTICE) != TT_OK) {
	return;
    }
    if (tt_pattern_state_add(notice_pattern, TT_SENT) != TT_OK) {
	return;
    }
    if (tt_pattern_class_add(request_pattern, TT_REQUEST) != TT_OK) {
	return;
    }
    if (tt_pattern_state_add(request_pattern, TT_SENT) != TT_OK) {
	return;
    }


    /*
     * Ops handled by the notice_pattern
     */
    if (tt_pattern_op_add(notice_pattern, "DtActivity_Beginning") != TT_OK) {
	return;
    }
    if (tt_pattern_op_add(notice_pattern, "DtActivity_Began") !=
	TT_OK) {
	return;
    }
    if (tt_pattern_op_add(notice_pattern, "DtTypes_Reloaded") != TT_OK) {
	return;
    }

    /*
     * Ops handled by the request_pattern
     */
    if (tt_pattern_op_add(request_pattern, "DtPanel_Restore") != TT_OK) {
	return;
    }
    if (tt_pattern_op_add(request_pattern, "DtWorkspace_SetCurrent") != TT_OK) {
	return;
    }
    if (tt_pattern_op_add(request_pattern, "DtWorkspace_Title_Set") !=
	TT_OK) {
	return;
    }
    if (tt_pattern_op_add(request_pattern, "DtWorkspace_Add") != TT_OK) {
	return;
    }
    if (tt_pattern_op_add(request_pattern, "DtWorkspace_Delete") != TT_OK) {
	return;
    }

    /*
     * Register callback for the notice_pattern
     */
    if (tt_pattern_callback_add(notice_pattern, NoticeMsgCB) != TT_OK) {
	return;
    }

    /*
     * Register callback for the request_pattern
     */
    if (tt_pattern_callback_add(request_pattern, RequestMsgCB) != TT_OK) {
	return;
    }

    if (tt_pattern_register(notice_pattern) != TT_OK) {
	return;
    }
    if (tt_pattern_register(request_pattern) != TT_OK) {
	return;
    }

} /* END OF FUNCTION dtInitializeMessaging */


/******************************<->*************************************
 *
 *  dtCloseIPC ()
 *
 *  Description:
 *  -----------
 *  Shuts down the messaging mechanism
 *
 *  Inputs:
 *  ------
 *
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  Should be done before exiting
 *
 ******************************<->***********************************/
void 
#ifdef _NO_PROTO
dtCloseIPC()
#else
dtCloseIPC( void )
#endif /* _NO_PROTO */
{
} /* END OF FUNCTION dtCloseIPC */



/******************************<->*************************************
 *  
 * void dtReadyNotification()
 *  
 *  Description:
 *  -----------
 *  Tells the world that we're up and ready.
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  Invoked as the fitting culmination of dtwm initialization
 *
 ******************************<->***********************************/
void 
#ifdef _NO_PROTO
dtReadyNotification()
#else
dtReadyNotification( void )
#endif /* _NO_PROTO */
{
    SendClientMsg( wmGD.dtSmWindow,
		  (long) wmGD.xa_DT_SM_WM_PROTOCOL,
		  (long) wmGD.xa_DT_WM_READY,
		  CurrentTime, NULL, 0);

} /* END OF FUNCTION dtReadyNotification */


/******************************<->*************************************
 *
 *  WmStopWaiting ()
 *
 *  Description:
 *  -----------
 *  This is called to turn off "system busy" activity
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *  None
 *
 *  Comments:
 *  ---------
 *  This routine relies on two globals, blinkerPCW and dtSD,
 *  on the major assumptions that:
 *     - there is just one DT Screen, with the front panel enabled
 *     - there is just one dtwmbusy control in that front panel
 ******************************<->***********************************/
void 
#ifdef _NO_PROTO
WmStopWaiting()
#else
WmStopWaiting( void )
#endif /* _NO_PROTO */
{
#ifdef PANELIST 
     if (wmGD.dtSD)
     {
	 WmFrontPanelSetBusy (False);
     }
#endif /* PANELIST */
} /* END OF FUNCTION WmStopWaiting */



/******************************<->*************************************
 *
 *  dtSendWorkspaceModifyNotification ()
 *
 *  Description:
 *  -----------
 *  This is called to announce that the workspace set has been
 *  modified
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 *  aWs		- id of workspace just modified
 *  iType	- type of modification
 * 
 *  Outputs:
 *  -------
 *  None
 *
 *  Comments:
 *  ---------
 *  Sends the message:
 *
 *    "MODIFY_WORKSPACE"  "<atom_for_wsname>" "<modify_type>"
 *
 *    <modify_type> is one of:
 *				ADD
 *				DELETE
 *				BACKDROP
 *				TITLE
 ******************************<->***********************************/
void
#ifdef _NO_PROTO
dtSendWorkspaceModifyNotification(pSD, aWs, iType)
	WmScreenData *pSD;
	Atom	aWs;
	int	iType;
#else
dtSendWorkspaceModifyNotification(
	WmScreenData *pSD,
	Atom aWs,
	int iType)
#endif /* _NO_PROTO */
{
    char        sNum[40];
    char        pch[40];
    char        pchType[40];
    Tt_message  msg;
    Tt_status   status;

    msg = tt_pnotice_create(TT_SESSION, "DtWorkspace_Modified");
    status = tt_ptr_error(msg);
    if (status != TT_OK) {
        return;
    }
    sprintf(sNum, "%d", pSD->screen);
    status = tt_message_arg_add(msg, TT_IN, Tttk_string, sNum);
    if (status != TT_OK) {
        return;
    }
    sprintf (pch, "%ld", aWs);
    status = tt_message_arg_add(msg, TT_IN, Tttk_string, pch);
    if (status != TT_OK) {
        return;
    }
    sprintf (pchType, "%d", iType);
    status = tt_message_arg_add(msg, TT_IN, Tttk_string, pchType);
    if (status != TT_OK) {
        return;
    }
    status = tt_message_send(msg);
    if (status != TT_OK) {
        return;
    }
    tt_message_destroy(msg);
}


/******************************<->*************************************
 *
 *  dtSendMarqueeSelectionNotification ()
 *
 *  Description:
 *  -----------
 *  This is called to announce marquee selection state
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 *  type	- id of workspace just modified
 *  x		- x position of UL corner of rectangle
 *  y		- y position of UL corner of rectangle
 *  width	- width of rectangle
 *  heigth	- height of rectangle
 * 
 *  Outputs:
 *  -------
 *  None
 *
 *  Comments:
 *  ---------
 *  Sends the message:
 *
 *    "MARQUEE_SELECTION"  "<type>" "<x>" "<y>" "<width>" "<height>"
 *
 *    <modify_type> is one of:
 *				BEGIN
 *				END
 *				CONTINUE
 *				CANCEL
 ******************************<->***********************************/
void
#ifdef _NO_PROTO
dtSendMarqueeSelectionNotification(pSD, type, x, y, width, height)
	WmScreenData *pSD;
	int	type;
	Position x,y;
	Dimension width,height;
#else
dtSendMarqueeSelectionNotification(
	WmScreenData *pSD,
	int	type,
	Position x,
	Position y,
	Dimension width,
	Dimension height)
#endif /* _NO_PROTO */
{
    Tt_message  msg;
    Tt_status   status;
    char        sNum[40];

    msg = tt_pnotice_create(TT_SESSION, "DtMarquee_Selection");
    status = tt_ptr_error(msg);
    if (status != TT_OK) {
        return;
    }

    sprintf(sNum, "%d", pSD->screen);
    status = tt_message_arg_add(msg, TT_IN, Tttk_string, sNum);
    if (status != TT_OK) {
        return;
    }
    if (tt_message_arg_add(msg, TT_IN, "integer", NULL) != TT_OK) {
	return;
    }
    if (tt_message_arg_ival_set(msg, 1, type) != TT_OK) {
	return;
    }
    if (tt_message_arg_add(msg, TT_IN, "integer", NULL) != TT_OK) {
	return;
    }
    if (tt_message_arg_ival_set(msg, 2, x) != TT_OK) {
	return;
    }
    if (tt_message_arg_add(msg, TT_IN, "integer", NULL) != TT_OK) {
	return;
    }
    if (tt_message_arg_ival_set(msg, 3, y) != TT_OK) {
	return;
    }
    if (tt_message_arg_add(msg, TT_IN, "integer", NULL) != TT_OK) {
	return;
    }
    if (tt_message_arg_ival_set(msg, 4, width) != TT_OK) {
	return;
    }
    if (tt_message_arg_add(msg, TT_IN, "integer", NULL) != TT_OK) {
	return;
    }
    if (tt_message_arg_ival_set(msg, 5, height) != TT_OK) {
	return;
    }
    status = tt_message_send(msg);
    if (status != TT_OK) {
        return;
    }
    tt_message_destroy(msg);
}


/******************************<->*************************************
 *
 *  NoticeMsgCB ()
 *
 *
 *  Description:
 *  -----------
 *  This is called to handle busy and stopbusy message
 *
 *  Inputs:
 *  ------
 *  m = ToolTalk message
 *  p = ToolTalk pattern
 * 
 *  Outputs:
 *  -------
 *  TT_CALLBACK_PROCESSED
 *  TT_CALLBACK_CINTINUE
 *
 *  Comments:
 *  ---------
 ******************************<->***********************************/
Tt_callback_action
#ifdef _NO_PROTO
NoticeMsgCB(m, p)
	Tt_message	m;
	Tt_pattern	p;
#else
NoticeMsgCB(Tt_message m, Tt_pattern p)
#endif /* _NO_PROTO */
{
    char	*op;
    Tt_status	status;

    if (tt_message_state(m) != TT_SENT) {
	return TT_CALLBACK_CONTINUE;
    }
    op = tt_message_op(m);
    status = tt_ptr_error(op);
    if (status != TT_OK) {
	return TT_CALLBACK_CONTINUE;
    }
    if (!strcmp(op, "DtActivity_Began")) {
	WmStopWaiting(); 
    }
#ifdef PANELIST 
    else if (!strcmp(op, "DtActivity_Beginning")) {
	if (wmGD.dtSD) {
	   WmFrontPanelSetBusy (True);
	}
    }
#endif /* PANELIST */
    else if (!strcmp(op, "DtTypes_Reloaded")) {
	/*
	 * Blink busy light during reload.
	 */
	WmFrontPanelSetBusy (True);

	/* 
	 * Load action definitions from the action database.
	 */
	DtDbLoad(); 

	UpdateFileTypeControlFields();

	/*
	 * Turn off busy light.
	 */
	WmFrontPanelSetBusy (False);
    }

    tt_free(op);
    return TT_CALLBACK_PROCESSED;

} /* END OF FUNCTION NoticeMsgCB */


/******************************<->*************************************
 *
 *  RequestMsgCB ()
 *
 *
 *  Description:
 *  -----------
 *  This is called to handle busy and stopbusy message
 *
 *  Inputs:
 *  ------
 *  m = ToolTalk message
 *  p = ToolTalk pattern
 * 
 *  Outputs:
 *  -------
 *  TT_CALLBACK_PROCESSED
 *  TT_CALLBACK_CINTINUE
 *
 *  Comments:
 *  ---------
 ******************************<->***********************************/
Tt_callback_action
#ifdef _NO_PROTO
RequestMsgCB(m, p)
	Tt_message	m;
	Tt_pattern	p;
#else
RequestMsgCB(Tt_message m, Tt_pattern p)
#endif /* _NO_PROTO */
{
    char	*op;
    Tt_status	status;

    int			screen_num;
    WmScreenData	*pSD;
    WmWorkspaceData	*pWS = NULL;
    Pixmap		pixmap;
    Atom		aWs;
    char		*pch;
    String		sName;
    int			i;

    if (tt_message_state(m) != TT_SENT) {
	return TT_CALLBACK_CONTINUE;
    }
    op = tt_message_op(m);
    status = tt_ptr_error(op);
    if (status != TT_OK) {
	return TT_CALLBACK_CONTINUE;
    }
    if (!strcmp(op, "DtPanel_Restore")) {
	tt_message_reply(m);
	tt_message_destroy(m);

	SessionDeleteAll();

	F_Restart( DTWM_REQP_NO_CONFIRM, NULL, NULL );
    }
    else if (!strcmp(op, "DtWorkspace_SetCurrent")) {
	/*
	 * 1st arg: integer, screen number
	 * 2nd arg: string, atom of workspace name
	 */

	/* get the first arg from the message */
	tt_message_arg_ival(m, 0, &screen_num);
	pSD = &wmGD.Screens[screen_num];

	/* get the second arg from the message */
	pch = tt_message_arg_val(m, 1);

	/* retrieve the selected workspace */
	aWs = strtoul (pch, (char **) NULL, 0);
	tt_free( pch );
	pWS = GetWorkspaceData (pSD, aWs);

	if (pWS) {
	    ChangeToWorkspace (pWS);
	}

	tt_message_reply(m);
	tt_message_destroy(m);
    }
    else if (!strcmp(op, "DtWorkspace_Title_Set")) {
	/*
	 * 1st arg: integer, screen number
	 * 2nd arg: string, atom of workspace name
	 * 3rd arg: string, new name for the workspace
	 */

	/* get the first arg from the message */
	tt_message_arg_ival(m, 0, &screen_num);
	pSD = &wmGD.Screens[screen_num];

	/* get the second arg from the message */
	pch = tt_message_arg_val(m, 1);

	/* retrieve the selected workspace */
	aWs = strtoul (pch, (char **) NULL, 0);
	tt_free( pch );
	pWS = GetWorkspaceData (pSD, aWs);

	/* get the third arg from the message */
	pch = tt_message_arg_val(m, 2);

	if (pWS) {
	    ChangeWorkspaceTitle (pWS, pch);
	}
	tt_free( pch );

	tt_message_reply(m);
	tt_message_destroy(m);
    }
    else if (!strcmp(op, "DtWorkspace_Add")) {
	/*
	 * 1st arg: string, user-visible title of the workspace
	 */
	pch = tt_message_arg_val(m, 0);

	F_CreateWorkspace( pch, NULL, NULL );
	tt_free( pch );

	tt_message_reply(m);
	tt_message_destroy(m);
    }
    else if (!strcmp(op, "DtWorkspace_Delete")) {
	/*
	 * 1st arg: string, atom of workspace name
	 */
	pch = tt_message_arg_val(m, 0);

	/* retrieve the selected workspace */
	aWs = strtoul (pch, (char **) NULL, 0);
	tt_free( pch );
	sName = (String) XmGetAtomName (DISPLAY1, aWs);

	F_DeleteWorkspace( sName, NULL, NULL );

	tt_message_reply(m);
	tt_message_destroy(m);

	XtFree(sName);
    } else {
	tt_free( op );
	return TT_CALLBACK_CONTINUE;
    }

    tt_free(op);
    return TT_CALLBACK_PROCESSED;

} /* END OF FUNCTION RequestMsgCB */

/****************************   eof    ***************************/
