/*****************************<+>*************************************
 *********************************************************************
 **
 **   File:     WsmM.h
 **
 **   RCS:	$XConsortium: WsmM.h /main/cde1_maint/1 1995/07/14 20:29:53 drk $
 **   Project:  DT Workspace Manager
 **
 **   Description: Defines parameters necessary for messaging with
 **                the workspace manager.
 **
 ** (c) Copyright 1993, 1994 Hewlett-Packard Company
 ** (c) Copyright 1993, 1994 International Business Machines Corp.
 ** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 ** (c) Copyright 1993, 1994 Novell, Inc.
 **
 *********************************************************************
 *****************************<+>*************************************/
#ifndef _Dt_WsmM_h
#define _Dt_WsmM_h

/*
 * Messaging definitions
 */

/* Workspace manager tool class (for old messages) */
#define DtWM_TOOL_CLASS			"WORKSPACEMGR"

/* Workspace manager tool class (for new messages) */
#define DtWSM_TOOL_CLASS		"WORKSPACE_MANAGER"

/* Workspace manager tool class (for front panel messages) */
#define DtFP_TOOL_CLASS		"FRONTPANEL"

/* Request from dtstyle to change backdrop */
#define DtWM_BACKDROP_CHANGE		"BACKDROP_CHANGE"

/* Request from session manager to unlock the display (depress button) */
#define DtWM_UNLOCK_DISPLAY		"UNLOCK_DISPLAY"

/* Request from session manager to cancel an exit (depress button) */
#define DtWM_CANCEL_EXIT		"CANCEL_EXIT"

/* Request from session manager to retore the front panel to default state */
#define DtWM_RESTORE_PANEL		"RESTORE_PANEL"

/* Notification from Workspace manager of a new workspace */
#define DtWSM_NEW_WORKSPACE		"ACTIVE_WORKSPACE"

#define DtWSM_NEW_WORKSPACE_ARG_NAME	0
#define DtWSM_NEW_WORKSPACE_ARG_ATOM	1

/* Request to set the current workspace */
#define DtWSM_SET_WORKSPACE		"SET_WORKSPACE"

#define DtWSM_SET_WORKSPACE_ARG_ATOM	0

/* Request to change the title of a workspace */
#define DtWSM_SET_WORKSPACE_TITLE	"SET_WORKSPACE_TITLE"

#define DtWSM_SET_WORKSPACE_TITLE_ARG_ATOM	0
#define DtWSM_SET_WORKSPACE_TITLE_ARG_TITLE	1

/* Request to add a workspace */
#define DtWSM_ADD_WORKSPACE		"ADD_WORKSPACE"

#define DtWSM_ADD_WORKSPACE_ARG_TITLE		0

/* Request to delete a workspace */
#define DtWSM_DELETE_WORKSPACE		"DELETE_WORKSPACE"

#define DtWSM_DELETE_WORKSPACE_ARG_ATOM		0

/* Notification from Workspace manager of a modified workspace */
#define DtWSM_MODIFY_WORKSPACE		"MODIFY_WORKSPACE"

#define DtWSM_MODIFY_WORKSPACE_ARG_ATOM		0
#define DtWSM_MODIFY_WORKSPACE_ARG_TYPE		1

#define DtWSM_MODIFY_WORKSPACE_TYPE_ADD		0
#define DtWSM_MODIFY_WORKSPACE_TYPE_DELETE	1
#define DtWSM_MODIFY_WORKSPACE_TYPE_BACKDROP	2
#define DtWSM_MODIFY_WORKSPACE_TYPE_TITLE	3
#define DtWSM_MODIFY_WORKSPACE_TYPE_ACTIVE	4

/* Notification from Workspace manager of a modified workspace */
#define DtWSM_MARQUEE_SELECTION		"MARQUEE_SELECTION"

#define DtWSM_MARQUEE_SELECTION_ARG_TYPE	0
#define DtWSM_MARQUEE_SELECTION_ARG_X		1
#define DtWSM_MARQUEE_SELECTION_ARG_Y		2
#define DtWSM_MARQUEE_SELECTION_ARG_WIDTH	3
#define DtWSM_MARQUEE_SELECTION_ARG_HEIGHT	4

#define DtWSM_MARQUEE_SELECTION_TYPE_BEGIN	1
#define DtWSM_MARQUEE_SELECTION_TYPE_CONTINUE	2
#define DtWSM_MARQUEE_SELECTION_TYPE_END	3
#define DtWSM_MARQUEE_SELECTION_TYPE_CANCEL	4

/*
 * Macros to set and fetch argument fields into a message
 *
 * (NOTE: Since sending the request breaks out the zero'th 
 *  field, there is a difference of one in these two macros.)
 */
#define DtWSM_SET_FIELD(fields,ix,value) ((fields)[(ix)]=(value))
#define DtWSM_GET_FIELD(fields,ix) ((fields)[((ix)+1)])


#endif /* _Dt_WsmM_h */
/* Do not add anything after this endif. */
