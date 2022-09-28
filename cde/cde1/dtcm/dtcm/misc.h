/*******************************************************************************
**
**  misc.h
**
**  #pragma ident "@(#)misc.h	1.30 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: misc.h /main/cde1_maint/4 1995/09/06 08:12:02 lehors $
**
**  RESTRICTED CONFIDENTIAL INFORMATION:
**
**  The information in this document is subject to special
**  restrictions in a confidential disclosure agreement between
**  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
**  document outside HP, IBM, Sun, USL, SCO, or Univel without
**  Sun's specific written approval.  This document and all copies
**  and derivative works thereof must be returned or destroyed at
**  Sun's request.
**
**  Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
**
*******************************************************************************/

/*									*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#ifndef _MISC_H
#define _MISC_H

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <csa.h>
#include "ansi_c.h"
#include "calendar.h"
#include "cm_tty.h"
#include "datefield.h"

/*
**  Variable "types" to be passed to the dialog_popup function (that
**  function accepts a variable argument list with these attribute, value
**  pairs).
**
**  The BUTTON_IDENT attribute can be used to identify 1 or more buttons on
**  the dialog.  The int id value will be returned if that button is
**  pressed and the char* name value will be the button label.
*/
typedef enum {
	DIALOG_TITLE = 1,	/*  Followed by char*  */
	DIALOG_TEXT,		/*  Followed by char* */
	BUTTON_IDENT,		/*  Followed by int, char*  */
	DIALOG_IMAGE,		/*  Followed by pixmap from Props_pu */
	BUTTON_HELP,		/*  Followed by char*  */
	BUTTON_INSENSITIVE	/*  Followed by int, char*  */
} Dialog_create_op;

/*
**  External function declarations
*/
extern char	*cm_mbchar	(char*);
extern int	cm_mbstrlen	(char*);
extern Widget	create_for_menu	(Widget);
extern Widget	create_privacy_menu(Widget);
extern Widget	create_repeat_menu(Widget, XtCallbackProc, XtPointer);
extern Widget	create_repeat_scope_menu(Widget, XmString, XtCallbackProc,
					    XtPointer);
extern Widget	create_start_stop_time_menu(Widget, XmString, XtCallbackProc,
					       XtPointer, Props*, Boolean, 
						Boolean, Widget**, int*);
extern Widget	create_time_scope_menu(Widget, XmString, XtCallbackProc,
					   XtPointer);
#ifndef _AIX
extern int	dialog_popup	(Widget, ...);
#endif /* _AIX */
extern void	editor_err_msg	(Widget, char*, Validate_op, Pixmap);
extern void	backend_err_msg	(Widget, char*, CSA_return_code, Pixmap);
extern void	get_range	(Glance, time_t, time_t*, time_t*);
extern boolean_t query_user	(void*);
extern void	set_message	(Widget, char*);
extern void	setup_quit_handler(Widget, XtCallbackProc, caddr_t);
extern int	ds_is_double_click     (XEvent*, XEvent*);
extern int	cm_select_text	(Widget, Time);
extern void	set_time_submenu(Widget, Widget, Props*, 
					   XtCallbackProc, XtPointer, 
					   Boolean, Boolean, Widget**, int*);
extern void 	ManageChildren	(Widget);
#endif
