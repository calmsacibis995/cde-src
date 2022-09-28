/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmMarquee.c
 **
 **   RCS:	$XConsortium: WmMarquee.c /main/cde1_maint/2 1995/09/06 02:15:09 lehors $
 **
 **   Project:  DT Workspace Manager
 **
 **   Description: Get Marquee Selection Data
 **
 ** (c) Copyright 1993, 1994 Hewlett-Packard Company
 ** (c) Copyright 1993, 1994 International Business Machines Corp.
 ** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 ** (c) Copyright 1993, 1994 Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Tt/tttk.h>
#include <Dt/Wsm.h> 
#include <Dt/WsmM.h>
#include "WsmP.h" 
#include <Dt/Service.h>
#include <Xm/Xm.h>

/*************************************<->*************************************
 *
 *  Tt_callback_action _WsSelectionCB (Tt_message m, Tt_pattern p)
 *
 *
 *  Description:
 *  -----------
 *  Internal function called for a marquee selection
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
_WsSelectionCB (m, p)
	Tt_message	m;
	Tt_pattern	p;
#else
_WsSelectionCB (Tt_message m, Tt_pattern p)
#endif /* _NO_PROTO */
{
    struct _DtWsmCBContext	*pCbCtx;
    int				type, val;
    Position			x, y;
    Dimension			width, height;

    Widget			widget;
    DtWsmMarqueeSelectionProc	marquee_selection;
    XtPointer			client_data;

    /*
     * user data 0: Widget		widget;
     * user data 1: DtWsmWsModifiedProc	ws_modify;
     * user data 2: XtPointer		client_data;
     */
    widget = (Widget)tt_pattern_user(p, 0);
    marquee_selection = (DtWsmMarqueeSelectionProc)tt_pattern_user(p, 1);
    client_data = (XtPointer)tt_pattern_user(p, 2);

    /*
     * 0th arg: screen number, string, not used
     */

    tt_message_arg_ival(m, 1, &type);	/* type */
    tt_message_arg_ival(m, 2, &val);	/* x */
    x = val;
    tt_message_arg_ival(m, 3, &val);	/* y */
    y = val;
    tt_message_arg_ival(m, 4, &val);	/* width */
    width = val;
    tt_message_arg_ival(m, 5, &val);	/* height */
    height = val;

    /*
     * Call registered callback function.
     */
    (*marquee_selection)(widget, type, x, y, width, height, client_data);
    tt_message_destroy(m);
    return TT_CALLBACK_PROCESSED;
} /* END OF FUNCTION Tt_callback_action */

/*************************************<->*************************************
 *
 *  DtWsmCBContext * DtWsmAddMarqueeSelectionCallback (widget, 
 *						 	marquee_select, 
 *							client_data)
 *
 *
 *  Description:
 *  -----------
 *  Register a function to be called when a marquee selection is made
 *
 *
 *  Inputs:
 *  ------
 *  widget		- widget for this client
 *  marquee_select	- function to call for marquee select
 *  client_data		- additional data to pass back to client when called.
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
_DtWsmAddMarqueeSelectionCallback (widget, marquee_selection, client_data)
	Widget				widget;
	DtWsmMarqueeSelectionProc	marquee_selection;
	XtPointer			client_data;

#else
_DtWsmAddMarqueeSelectionCallback (
	Widget				widget,
	DtWsmMarqueeSelectionProc	marquee_selection,
	XtPointer			client_data)
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

    if (tt_pattern_category_set(pattern, TT_OBSERVE) != TT_OK) {
	return NULL;
    }
    if (tt_pattern_scope_add(pattern, TT_SESSION) != TT_OK) {
	return NULL;
    }
    sessId = tt_default_session();
    if (tt_pattern_session_add(pattern, sessId) != TT_OK) {
	return NULL;
    }
    tt_free(sessId);

    screen = XScreenNumberOfScreen(XtScreen(widget));
    sprintf(sNum, "%d", screen);
    sName = _DtWsmSelectionNameForScreen (screen);

    /*
     * Only receive DtMarquee_Selection notice from the screen
     * we registered with.
     */
    status = tt_pattern_arg_add(pattern, TT_IN, Tttk_string, sNum);
    if (status != TT_OK) {
	return NULL;
    }

    if (tt_pattern_op_add(pattern, "DtMarquee_Selection") != TT_OK) {
	return NULL;
    }

    if (tt_pattern_state_add(pattern, TT_SENT) != TT_OK) {
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
    status = tt_pattern_user_set(pattern, 1, (void *)marquee_selection);
    if (status != TT_OK) {
	return NULL;
    }
    status = tt_pattern_user_set(pattern, 2, (void *)client_data);
    if (status != TT_OK) {
	return NULL;
    }

    /*
     * _WsSelectionCB is the ToolTalk callback which will call
     * the user callback.
     */
    if (tt_pattern_callback_add(pattern, _WsSelectionCB) != TT_OK) {
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
    pCbCtx->ws_cb = marquee_selection;
    pCbCtx->client_data = client_data;

    XtFree (sName);

    return (pCbCtx);

} /* END OF FUNCTION DtWsmAddMarqueeSelectionCallback */
