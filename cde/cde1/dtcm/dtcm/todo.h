/*******************************************************************************
**
**  todo.h
**
**  #pragma ident "@(#)todo.h	1.28 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: todo.h /main/cde1_maint/2 1995/10/03 12:27:52 barstow $
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

#ifndef _TODO_H
#define _TODO_H

#include <csa.h>
#include "ansi_c.h"
#include "calendar.h"
#include "misc.h"
#include "dssw.h"
#include "rfp.h"
#include "reminders.h"

/*******************************************************************************
**
**  ToDo structures
**
*******************************************************************************/
struct TodoView {
	Widget		view_item_number;
	Widget		view_item_toggle;
	Boolean		modified;
	CSA_entry_handle	appt;
	struct TodoView	*next;
};
typedef struct TodoView TodoView;

typedef enum { VIEW_ALL, VIEW_PENDING, VIEW_COMPLETED } todo_view_op;
	
typedef struct {
	Widget			base_form_mgr;
	Calendar		*cal;
	Widget			change_button;
	Widget			clear_button;
	Widget			close_button;
	Widget			completed_toggle;
	Boolean			completed_val;
	Widget			delete_button;
	DSSW			dssw;
	Widget			expand_ui_button;
	Widget			frame;
	Widget			help_button;
	Widget			insert_button;
	Widget			list_label;
	Widget			message_text;
	Reminders		reminders;
	RFP			rfp;
	Widget			separator1;
	Widget			separator2;
	CSA_entry_handle	*todo_head;
	int			todo_count;
	Widget			todo_list;
	Widget			todo_list_sw;
	Boolean			todo_is_up;
	Glance			view_list_glance;
	Tick			view_list_date;
	Boolean			todo_view_is_up;
	todo_view_op		view_filter;
	Widget			view_form_mgr;
	Widget			view_form;
	Widget			view_frame;
	Widget			view_filter_menu;
	Widget			view_list_label;
	TodoView		*view_list;
	Widget			view_sw_mgr;
	Boolean			view_list_modified;
	Widget			view_apply_button;
	Widget			view_ok_button;
	Widget			view_cancel_button;
	Widget			view_help_button;
	Widget			drag_icon;
	Widget			drag_source;
	int			initialX;
	int			initialY;
	Boolean			doing_drag;
} ToDo;

/*******************************************************************************
**
**  ToDo external function declarations
**
*******************************************************************************/
extern void	add_all_todo	(ToDo*, Dtcm_appointment *);
extern void	build_todo_list	(ToDo*, Tick, Glance, CSA_entry_handle**,
					   CSA_uint32*, todo_view_op);
extern int	build_todo_view	(ToDo*, Glance, Boolean);
extern void	get_todo_vals	(ToDo*);
extern void	todo_hide	(ToDo*);
extern void	set_todo_defaults(ToDo*);
extern void	set_todo_title	(ToDo*, char*);
extern void	set_todo_vals	(ToDo*);
extern void	show_todo	(Calendar*);
extern void	t_make_todo	(Calendar*);
extern void	show_todo_view	(Calendar*, todo_view_op);
extern void	todo_clean_up	(ToDo*);
extern Boolean	todo_showing	(ToDo*);
extern Boolean	todo_view_showing(ToDo*);
extern CSA_entry_handle	t_nth_appt(ToDo*, int);
extern Boolean  todo_insert          (Dtcm_appointment*, CSA_entry_handle*,
					Calendar*);

#endif
