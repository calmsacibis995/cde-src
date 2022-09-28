/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmWsCallB.c
 **
 **   RCS:	$XConsortium: WmWsCallB.c /main/cde1_maint/2 1995/10/08 22:19:46 pascale $
 **
 **   Project:  DT Workspace Manager
 **
 **   Description: Workspace change callback functions
 **
 ** (c) Copyright 1993, 1994 Hewlett-Packard Company
 ** (c) Copyright 1993, 1994 International Business Machines Corp.
 ** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 ** (c) Copyright 1993, 1994 Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/
#include <Tt/tttk.h>
#include <Dt/Service.h>
#include <Dt/Wsm.h> 
#include <Dt/WsmM.h>
#include "WsmP.h" 


/*************************************<->*************************************
 *
 *  DtWsmWsModifiedProc _DtWsmWsChangeHandler (widget, aWS, 
 *						reason, client_data);
 *
 *
 *  Description:
 *  -----------
 *  Internal function called when workspace changes.
 *
 *
 *  Inputs:
 *  ------
 *  widget		- widget (for window where service is registered)
 *  aWS			- Atom for workspace identification
 *  reason		- type of workspace modification
 *  client_data		- pointer to data
 *
 *  Outputs:
 *  --------
 *  Return	- none
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
static void
#ifdef _NO_PROTO
_DtWsmWsChangeHandler (widget, aWS, reason, client_data)
	Widget			widget;
	Atom			aWS;
	DtWsmWsReason		reason;
	XtPointer		client_data;
#else
_DtWsmWsChangeHandler (
	Widget			widget,
	Atom			aWS,
	DtWsmWsReason		reason,
	XtPointer		client_data)
#endif /* _NO_PROTO */
{
    struct _DtWsmCBContext *pCbCtx;

    pCbCtx = (struct _DtWsmCBContext *) client_data;

    /*
     * We only deal with the workspace changes
     */
    if (reason == DtWSM_REASON_CURRENT)
    {
	/*
	 * Call registered callback function.
	 */
	(*(pCbCtx->ws_cb)) (pCbCtx->widget, aWS, 
					pCbCtx->client_data);
    }
    
} /* END OF FUNCTION _DtWsmWsChangeHandler */

/*************************************<->*************************************
 *
 *  DtWsmCBContext * DtWsmAddCurrentWorkspaceCallback (widget, 
 *						 	ws_change, 
 *							client_data)
 *
 *
 *  Description:
 *  -----------
 *  Register a function to be called when the workspace changes.
 *
 *
 *  Inputs:
 *  ------
 *  widget	- widget for this client
 *  ws_change	- function to call when workspace changes
 *  client_data	- additional data to pass back to client when called.
 *
 *  Outputs:
 *  --------
 *  Return	- ptr to callback context data (opaque) 
 *
 *  Comments:
 *  ---------
 *  The callback context data ptr should be saved if you intend to
 *  removed this callback at some point in the future. 
 * 
 *************************************<->***********************************/
DtWsmCBContext 
#ifdef _NO_PROTO
DtWsmAddCurrentWorkspaceCallback (widget, ws_change, client_data)
	Widget			widget;
	DtWsmWsChangeProc	ws_change;
	XtPointer		client_data;

#else
DtWsmAddCurrentWorkspaceCallback (
	Widget			widget,
	DtWsmWsChangeProc	ws_change,
	XtPointer		client_data)
#endif /* _NO_PROTO */
{
    struct _DtWsmCBContext *pCbCtx;

    /*
     * Allocate data to remember stuff about this callback
     */
    pCbCtx = (struct _DtWsmCBContext * ) 
		XtMalloc (sizeof(struct _DtWsmCBContext));


    /* 
     * Save what we want to remember
     */
    pCbCtx->widget = widget;
    pCbCtx->ws_cb = ws_change;
    pCbCtx->client_data = client_data;

    /*
     * Register interest in the workspace change message
     */
    pCbCtx->nested_context = (XtPointer)
	DtWsmAddWorkspaceModifiedCallback (widget, 
			      (DtWsmWsModifiedProc)_DtWsmWsChangeHandler, 
			      (XtPointer) pCbCtx);

    return (pCbCtx);

} /* END OF FUNCTION DtWsmAddCurrentWorkspaceCallback */

/*
----------------------------------------------------------------------
*/

/*************************************<->*************************************
 *
 *  Tt_callback_action _WsModifiedCB (Tt_message m, tt_pattern p)
 *
 *
 *  Description:
 *  -----------
 *  Internal function called when a workspace is modified.
 *
 *
 *  Inputs:
 *  ------
 *  m		- ToolTalk message
 *  p		- ToolTalk pattern
 *
 *  Outputs:
 *  --------
 *  Return	- ToolTalk callback status
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
static Tt_callback_action
#ifdef _NO_PROTO
_WsModifiedCB (m, p)
	Tt_message	m;
	Tt_pattern	p;
#else
_WsModifiedCB (Tt_message m, Tt_pattern p)
#endif /* _NO_PROTO */
{
    struct _DtWsmCBContext *pCbCtx;
    Atom 	aWs;
    DtWsmWsReason	reason;

    Widget		widget;
    DtWsmWsModifiedProc	ws_modify;
    XtPointer		client_data;
    char                *arg;

    /*
     * user data 0: Widget		widget;
     * user data 1: DtWsmWsModifiedProc	ws_modify;
     * user data 2: XtPointer		client_data;
     */
    widget = (Widget)tt_pattern_user(p, 0);
    ws_modify = (DtWsmWsModifiedProc)tt_pattern_user(p, 1);
    client_data = (XtPointer)tt_pattern_user(p, 2);

    /*
     * 0th arg: screen number, string, not used
     */

    /*
     * Convert the atom to binary.
     */
    arg = tt_message_arg_val(m, 1);
    aWs = (Atom)strtoul(arg, (char **)NULL, 0);
    tt_free((caddr_t)arg);

    /*
     * Convert "reason" of workspace modification
     */
    arg = tt_message_arg_val(m, 2);
    reason = (DtWsmWsReason)strtoul(arg, (char **)NULL, 0);
    tt_free((caddr_t)arg);

    /*
     * Call registered callback function.
     */
    (*ws_modify)(widget, aWs, reason, client_data);

    tt_message_destroy(m);
    
    return TT_CALLBACK_PROCESSED;
} /* END OF FUNCTION _DtWsmWsModifyHandler */

/*************************************<->*************************************
 *
 *  DtWsmCBContext * DtWsmAddWorkspaceModifiedCallback (widget, 
 *						 	ws_modify, 
 *							client_data)
 *
 *
 *  Description:
 *  -----------
 *  Register a function to be called when the workspace is modified.
 *
 *
 *  Inputs:
 *  ------
 *  widget	- widget for this client
 *  ws_modify	- function to call when workspace is modified
 *  client_data	- additional data to pass back to client when called.
 *
 *  Outputs:
 *  --------
 *  Return	- ptr to callback context data (opaque) 
 *
 *  Comments:
 *  ---------
 *  The callback context data ptr should be saved if you intend to
 *  removed this callback at some point in the future. 
 * 
 *************************************<->***********************************/
DtWsmCBContext 
#ifdef _NO_PROTO
DtWsmAddWorkspaceModifiedCallback (widget, ws_modify, client_data)
	Widget			widget;
	DtWsmWsModifiedProc	ws_modify;
	XtPointer		client_data;

#else
DtWsmAddWorkspaceModifiedCallback (
	Widget			widget,
	DtWsmWsModifiedProc	ws_modify,
	XtPointer		client_data)
#endif /* _NO_PROTO */
{
    struct _DtWsmCBContext *pCbCtx;
    int		screen;
    String	sName;
    char	sNum[32];

    Tt_status	status;
    Tt_pattern	pattern;
    char *	sessId;

    /*
     * This function register a ToolTalk pattern for every
     * callback added.
     */
    _DtSvcInitToolTalk(widget);

    pattern = tt_pattern_create();
    status = tt_ptr_error(pattern);
    if (status != TT_OK) {
	return NULL;
    }

    if (tt_pattern_scope_add(pattern, TT_SESSION) != TT_OK) {
	return NULL;
    }
    if (tt_pattern_category_set(pattern, TT_OBSERVE) != TT_OK) {
	return NULL;
    }
    if (tt_pattern_class_add(pattern, TT_NOTICE) != TT_OK) {
	return NULL;
    }
    if (tt_pattern_state_add(pattern, TT_SENT) != TT_OK) {
	return NULL;
    }
    sessId = tt_default_session();
    if (tt_pattern_session_add(pattern, sessId) != TT_OK) {
	return NULL;
    }
    tt_free( sessId );

    screen = XScreenNumberOfScreen(XtScreen(widget));
    sprintf(sNum, "%d", screen);
    sName = _DtWsmSelectionNameForScreen (screen);

    /*
     * Only receive DtWorkspace_Modified notice from the screen
     * we registered with.
     */
    status = tt_pattern_arg_add(pattern, TT_IN, Tttk_string, sNum);
    if (status != TT_OK) {
	return NULL;
    }

    if (tt_pattern_op_add(pattern, "DtWorkspace_Modified") != TT_OK) {
	return NULL;
    }

    /*
     * Store information needed by the callback in the user data
     * fields of the pattern.
     */
    status = tt_pattern_user_set(pattern, 0, (void *)widget);
    if (status != TT_OK) {
	return NULL;
    }
    status = tt_pattern_user_set(pattern, 1, (void *)ws_modify);
    if (status != TT_OK) {
	return NULL;
    }
    status = tt_pattern_user_set(pattern, 2, (void *)client_data);
    if (status != TT_OK) {
	return NULL;
    }

    /*
     * _WsModifiedCB is the ToolTalk callback which will call
     * the user callback.
     */
    if (tt_pattern_callback_add(pattern, _WsModifiedCB) != TT_OK) {
	return NULL;
    }

    if (tt_pattern_register(pattern) != TT_OK) {
	return NULL;
    }

    /*
     * Allocate data to remember stuff about this callback
     */
    pCbCtx = (struct _DtWsmCBContext * ) 
		XtMalloc (sizeof(struct _DtWsmCBContext));

    /* 
     * Save what we want to remember
     */
    pCbCtx->pattern = pattern;
    pCbCtx->widget = widget;
    pCbCtx->ws_cb = ws_modify;
    pCbCtx->client_data = client_data;
    pCbCtx->nested_context = NULL;

    XtFree (sName);

    return (pCbCtx);

} /* END OF FUNCTION DtWsmAddWorkspaceModifiedCallback */

/*************************************<->*************************************
 *
 *  DtWsmRemoveWorkspaceCallback (pCbCtx)
 *
 *  Description:
 *  -----------
 *  Unregister a workspace callback.
 *
 *
 *  Inputs:
 *  ------
 *  pCbCtx	- ptr to context returned when callback added
 *
 *  Outputs:
 *  --------
 *  Return	- none
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
void
#ifdef _NO_PROTO
DtWsmRemoveWorkspaceCallback (pCbCtx)
	DtWsmCBContext		pCbCtx;

#else
DtWsmRemoveWorkspaceCallback (DtWsmCBContext 	pCbCtx)
#endif /* _NO_PROTO */
{
    /*
     * Is this somewhat valid?
     */
    if (pCbCtx && (pCbCtx->widget != NULL)) {
	if (pCbCtx->nested_context) {
	    /*
	     * This was a convenience callback for just the workspace
	     * change info.
	     */
	    DtWsmRemoveWorkspaceCallback (
				(DtWsmCBContext) pCbCtx->nested_context);
	}
	else {
	    /*
	     * Unregister interest in this message
	     */
	    tt_pattern_destroy(pCbCtx->pattern);
	}

	/*
	 * Free previously allocated data
	 */
	XtFree((char *) pCbCtx);
    }

} /* END OF FUNCTION DtWsmRemoveWorkspaceCallback */

/*************************************<->*************************************
 *
 *  _DtWsmSelectionNameForScreen (scr)
 *
 *  Description:
 *  -----------
 *  Returns a string containing the selection name used for
 *  communication with the workspace manager on this screen
 *
 *
 *  Inputs:
 *  ------
 *  scr		- number of screen
 *
 *  Outputs:
 *  --------
 *  Return	- ptr to string with selection name (free with XtFree)
 *
 *  Comments:
 *  ---------
 *  Assumes the screen number is < 1000.
 * 
 *************************************<->***********************************/
String
#ifdef _NO_PROTO
_DtWsmSelectionNameForScreen (scr)
	int			scr;

#else
_DtWsmSelectionNameForScreen (
	int			scr)
#endif /* _NO_PROTO */
{
    String sName;

    sName = (String) XtMalloc (strlen(DtWSM_TOOL_CLASS) + 4 + 1);

    sprintf ((char *)sName, "%s_%d", DtWSM_TOOL_CLASS, (scr % 1000));

    return (sName);
} /* END OF FUNCTION _DtWsmSelectionNameForScreen */


Tt_callback_action
#ifdef _NO_PROTO
_DtWsmConsumeReply( msg, pat )
        Tt_message msg;
        Tt_pattern pat;
#else
_DtWsmConsumeReply(
        Tt_message msg,
        Tt_pattern pat )
#endif /* _NO_PROTO */
{
	switch (tt_message_state( msg )) {
	    case TT_HANDLED:
	    case TT_FAILED:
		tttk_message_destroy( msg );
		return TT_CALLBACK_PROCESSED;
	    default:
		return TT_CALLBACK_CONTINUE;
	}
}
