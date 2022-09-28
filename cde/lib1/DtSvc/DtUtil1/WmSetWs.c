/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmSetWs.c
 **
 **   RCS:	$XConsortium: WmSetWs.c /main/cde1_maint/1 1995/07/14 20:28:57 drk $
 **
 **   Project:  DT Workspace Manager
 **
 **   Description: Set the current workspace
 **
 ** (c) Copyright 1993, 1994 Hewlett-Packard Company
 ** (c) Copyright 1993, 1994 International Business Machines Corp.
 ** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 ** (c) Copyright 1993, 1994 Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <Dt/Wsm.h> 
#include <Dt/WsmP.h> 
#include <Dt/WsmM.h>
#include <Dt/Service.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Tt/tttk.h>

/*************************************<->*************************************
 *
 *  status DtWsmSetCurrentWorkspace (widget, aWs)
 *
 *
 *  Description:
 *  -----------
 *  Set the current workspace
 *
 *
 *  Inputs:
 *  ------
 *  screen_num	- screen number, integer
 *  aWs  	- atom of workspace 
 *
 *  Outputs:
 *  --------
 *  Return	- Success if communication to workspace manager
 *		  was successful.
 *
 *  Comments:
 *  ---------
 * 
 *************************************<->***********************************/
Status
#ifdef _NO_PROTO
DtWsmSetCurrentWorkspace (widget, aWs)
	Widget widget;
        Atom aWs;
#else
DtWsmSetCurrentWorkspace (
	Widget widget,
        Atom aWs)
#endif /* _NO_PROTO */
{
    String	pStr[2];
    char	pch[40];
    Tt_message	msg;
    Tt_status	status;

    sprintf (pch, "0x%lx", aWs);
    pStr[0] = (String) &pch[0];
    pStr[1] = NULL;
    _DtInitializeToolTalk();
    msg = tttk_message_create(0, TT_REQUEST, TT_SESSION, 0,
	    "DtWorkspace_SetCurrent", 0);
    status = tt_ptr_error(msg);
    if (status != TT_OK) {
	return dtmsg_FAIL;
    }
    status = tt_message_arg_add(msg, TT_IN, "integer", NULL);
    if (status != TT_OK) {
	return dtmsg_FAIL;
    }
    status = tt_message_arg_ival_set(msg, 0,
	    XScreenNumberOfScreen(XtScreen(widget)) % 1000);
    if (status != TT_OK) {
	return dtmsg_FAIL;
    }
    status = tt_message_arg_add(msg, TT_IN, Tttk_string, pStr[0]);
    if (status != TT_OK) {
	return dtmsg_FAIL;
    }
    status = tt_message_callback_add(msg, _DtWsmConsumeReply);
    if (status != TT_OK) {
	return dtmsg_FAIL;
    }
    status = tt_message_send(msg);
    if (status != TT_OK) {
	return dtmsg_FAIL;
    }

    return (dtmsg_SUCCESS);

} /* END OF FUNCTION DtWsmSetCurrentWorkspace */
