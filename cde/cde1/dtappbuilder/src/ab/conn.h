
/*
 *      $XConsortium: conn.h /main/cde1_maint/3 1995/10/16 11:03:04 rcs $
 *
 * @(#)conn.h	1.17 02 May 1995      cde_app_builder/src/libABobj
 *
 *      RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *      The information in this document is subject to special
 *      restrictions in a confidential disclosure agreement between
 *      HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *      document outside HP, IBM, Sun, USL, SCO, or Univel without
 *      Sun's specific written approval.  This document and all copies
 *      and derivative works thereof must be returned or destroyed at
 *      Sun's request.
 *
 *      Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */

#ifndef _CONN_H_
#define _CONN_H_
/*
 * conn.h -
 */
#include <X11/Intrinsic.h>
#include <ab_private/obj.h>
#include "conn_ui.h"

extern ABObj		project;



extern void		conn_drag_chord(
			    Widget	widget,
			    XEvent	*event,
			    String	*params,
			    Cardinal	num_params
			);
extern void		conn_popup_dialog(
			    Widget	widget,
			    XtPointer	client_data,
			    XtPointer	call_data
			);
extern void		conn_set_source(
			    ABObj	obj
			);
extern void		conn_set_target(
			    ABObj	obj
			);

extern void		conn_enable_action_interpret(
			    ABObj	project,
			    BOOL	to_test_project
			);

extern void		conn_disable_action_interpret(
			    ABObj	project
			);
extern void		conn_test_mode_initialize(
			    ABObj	project
			);
extern void		conn_test_mode_cleanup(
			    ABObj	project
			);
extern void		conn_init();
extern void		conn_strings_init();

extern void		conn_override_default_when(
			    AB_WHEN	when
			);
extern void		conn_reset_default_when(void);
extern void		conn_reset_default_action(void);
extern void		conn_override_default_action_type(
			    AB_FUNC_TYPE	func_type
			);
extern void		conn_reset_default_action_type();

#endif /* _CONN_H_ */
