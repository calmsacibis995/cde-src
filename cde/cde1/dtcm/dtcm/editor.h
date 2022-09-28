/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)editor.h	1.34 96/11/12 Sun Microsystems, Inc."

/*******************************************************************************
**
**  editor.h
**
**  #pragma ident "@(#)editor.h	1.34 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: editor.h /main/cde1_maint/2 1995/10/03 12:21:40 barstow $
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

#ifndef _EDITOR_H
#define _EDITOR_H

#include <csa.h>
#include "ansi_c.h"
#include "calendar.h"
#include "misc.h"
#include "dssw.h"
#include "rfp.h"
#include "reminders.h"

/*******************************************************************************
**
**  Editor structures
**
*******************************************************************************/
typedef struct {
	int			appt_count;
	CSA_entry_handle	*appt_head;
	Widget			appt_list;
	Widget			appt_list_sw;
	Widget			base_form_mgr;
	Calendar		*cal;
	Widget			change_button;
	Widget			clear_button;
	Widget			close_button;
	Widget			delete_button;
	Boolean			doing_drag;
	Pixmap			drag_bitmap;
	Widget			drag_icon;
	Widget			drag_source;
	Pixmap			drag_mask;
	DSSW			dssw;
	Boolean			editor_is_up;
	Boolean			editor_view_is_up;
	Widget			expand_ui_button;
	Widget			frame;
	Widget			help_button;
	int			initialX;
	int			initialY;
	Widget			insert_button;
	Widget			list_label;
	Widget			message_text;
	Reminders		reminders;
	RFP			rfp;
	Widget			separator1;
	Widget			separator2;
	Widget			view_form_mgr;
	Widget			view_frame;
	Widget			view_form;
	Widget			view_list_label;
	Widget			view_list;
	Widget			view_sw_mgr;
	Widget			view_cancel_button;
	Widget			view_help_button;
	Glance			view_list_glance;
	Tick			view_list_date;
} Editor;

/*******************************************************************************
**
**  Editor external function declarations
**
*******************************************************************************/
extern void	add_all_appt	(Editor*, Dtcm_appointment *);
extern void	build_editor_list(Editor*, Tick, Glance, CSA_entry_handle**,
					   CSA_uint32*);
extern int	build_editor_view(Editor*, Glance, Boolean);
extern Boolean	editor_change	(Dtcm_appointment*, CSA_entry_handle,
					   CSA_entry_handle*, Calendar*);
extern void	editor_clean_up	(Editor*);
extern Boolean	editor_created	(Editor*);
extern Boolean	editor_delete	(CSA_entry_handle, Calendar*);
extern Boolean	editor_insert	(Dtcm_appointment*, CSA_entry_handle*,
					   Calendar*);
extern CSA_entry_handle editor_nth_appt(Editor*, int);
extern Boolean	editor_showing	(Editor*);
extern Boolean	editor_view_showing(Editor*);
extern void	get_editor_vals	(Editor*);
extern void	set_editor_defaults(Editor*, Tick, Tick, Boolean);
extern void	set_editor_title(Editor*, char *);
extern void	set_editor_vals	(Editor*, Tick, Tick);
extern void	show_editor	(Calendar*, Tick, Tick, Boolean);
extern void	show_editor_view(Calendar*, Glance);
extern void	e_make_editor	(Calendar*);

#endif
