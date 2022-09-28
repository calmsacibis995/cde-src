/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     WmAddDelWs.c
 **
 **   RCS:	$XConsortium: WmAddDelWs.c /main/cde1_maint/2 1995/09/06 02:14:55 lehors $
 **
 **   Project:  DT Workspace Manager
 **
 **   Description: Add/Delete a workspace
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
 *  status _DtWsmDeleteWorkspace (widget, aWs)
 *
 *
 *  Description:
 *  -----------
 *  Delete a workspace
 *
 *
 *  Inputs:
 *  ------
 *  widget	- a widget (with a window!)
 *  aWs  	- atom of workspace to delete
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
_DtWsmDeleteWorkspace (widget, aWs)
	Widget widget;
        Atom aWs;
#else
_DtWsmDeleteWorkspace (
	Widget widget,
        Atom aWs)
#endif /* _NO_PROTO */
{
    char	pch[40];
    Tt_message	msg;
    Tt_status	status;

    sprintf (pch, "0x%lx", aWs);
    msg = tttk_message_create(0, TT_REQUEST, TT_SESSION, 0,
	    "DtWorkspace_Delete", 0);
    status = tt_ptr_error(msg);
    if (status != TT_OK) {
	return dtmsg_FAIL;
    }
    status = tt_message_arg_add(msg, TT_IN, Tttk_string, pch);
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

} /* END OF FUNCTION _DtWsmDeleteWorkspace */


/*************************************<->*************************************
 *
 *  status _DtWsmCreateWorkspace (widget, pchTitle)
 *
 *
 *  Description:
 *  -----------
 *  Add a workspace
 *
 *
 *  Inputs:
 *  ------
 *  widget	- a widget (with a window!)
 *  pchTitle	- user-visible title of the workspace
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
_DtWsmCreateWorkspace (widget, pchTitle)
	Widget 	widget;
	char *	pchTitle;
#else
_DtWsmCreateWorkspace (Widget widget, char * pchTitle)
#endif /* _NO_PROTO */
{
    Tt_message	msg;
    Tt_status	status;

    msg = tttk_message_create(0, TT_REQUEST, TT_SESSION, 0,
	    "DtWorkspace_Add", 0);
    status = tt_ptr_error(msg);
    if (status != TT_OK) {
	return dtmsg_FAIL;
    }
    status = tt_message_arg_add(msg, TT_IN, Tttk_string, pchTitle);
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

} /* END OF FUNCTION _DtWsmCreateWorkspace */
