/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmSetTitle.c
 **
 **   RCS:	$XConsortium: WmSetTitle.c /main/cde1_maint/1 1995/07/14 20:28:44 drk $
 **
 **   Project:  DT Workspace Manager
 **
 **   Description: Set the title for a workspace
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
 *  status DtWsmSetWorkspaceTitle (widget, aWs, pchNewName)
 *
 *
 *  Description:
 *  -----------
 *  Rename a workspace
 *
 *
 *  Inputs:
 *  ------
 *  widget	- a widget
 *  aWs  	- atom of workspace 
 *  pchNewName	- new name for the workspace
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
_DtWsmSetWorkspaceTitle (widget, aWs, pchNewName)
	Widget widget;
        Atom aWs;
	char *pchNewName;
#else
_DtWsmSetWorkspaceTitle (
	Widget widget,
        Atom aWs,
	char * pchNewName)
#endif /* _NO_PROTO */
{
    Tt_status	status;
    Tt_message	msg;
    char	pch[40];

    sprintf (pch, "0x%lx", aWs);

    msg = tttk_message_create(0, TT_REQUEST, TT_SESSION, 0,
	    "DtWorkspace_Title_Set", 0);
    status = tt_ptr_error(msg);
    if (status != TT_OK) {
	return DT_SVC_FAIL;
    }
    status = tt_message_arg_add(msg, TT_IN, "integer", NULL);
    if (status != TT_OK) {
	return DT_SVC_FAIL;
    }
    status = tt_message_arg_ival_set(msg, 0,
	    XScreenNumberOfScreen(XtScreen(widget)) % 1000);
    if (status != TT_OK) {
	return DT_SVC_FAIL;
    }
    status = tt_message_arg_add(msg, TT_IN, Tttk_string, pch);
    if (status != TT_OK) {
	return DT_SVC_FAIL;
    }
    status = tt_message_arg_add(msg, TT_IN, Tttk_string, pchNewName);
    if (status != TT_OK) {
	return DT_SVC_FAIL;
    }
    status = tt_message_callback_add(msg, _DtWsmConsumeReply);
    if (status != TT_OK) {
	return dtmsg_FAIL;
    }
    status = tt_message_send(msg);
    if (status != TT_OK) {
	return DT_SVC_FAIL;
    }

    return DT_SVC_SUCCESS;

} /* END OF FUNCTION DtWsmSetWorkspaceTitle */
