/*******************************************************************************
**
**  group_editor.h
**
**  #pragma ident "@(#)group_editor.h	1.28 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: group_editor.h /main/cde1_maint/1 1995/07/14 23:11:17 drk $
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

#ifndef _GEDITOR_H
#define _GEDITOR_H

#include <csa.h>
#include "ansi_c.h"
#include "util.h"
#include "calendar.h"
#include "misc.h"
#include "dssw.h"
#include "rfp.h"

/*******************************************************************************
**
**  Group Editor structures
**
*******************************************************************************/
typedef struct {
	int			appt_count;
	CSA_entry_handle	*appt_head;
	char			*name;
	CSA_session_handle	cal_handle;
	int			version;
	CSA_flags		entry_access;
	Boolean			set_charset;
} Access_data;

/*
**  An entry in this structure will point to the big list in the Access_data
**  structure
*/
typedef struct {
	int		entry_idx;
	Tick		tick;
	Access_data	*ad;
} List_data;

typedef struct {
	CmDataList		*access_data;
	Widget			access_list;
	Widget			access_list_label;
	Widget			access_list_sw;
	Widget			appt_list;
	Widget			appt_list_label;
	Widget			appt_list_sw;
	Widget			base_form_mgr;
	Widget			button_rc_mgr;
	Calendar		*cal;
	Widget			change_button;
	Widget			clear_button;
	Widget			close_button;
	Widget			delete_button;
	DSSW			dssw;
	Widget			expand_ui_button;
	Widget			frame;
	Boolean			geditor_is_up;
	Widget			help_button;
	Widget			insert_button;
	CmDataList		*list_data;
	Widget			mail_button;
	Widget			message_text;
	RFP			rfp;
	Widget			separator1;
	Widget			separator2;
        Pixmap                  drag_bitmap;
        Pixmap                  drag_mask;
        Widget                  drag_icon;
        Widget                  drag_source;
        int                     initialX;
        int                     initialY;
        Boolean                 doing_drag;
} GEditor;

/*******************************************************************************
**
**  Group Editor external function declarations
**
*******************************************************************************/
extern void		add_to_gaccess_list(char*, CSA_session_handle,
						   unsigned int, int, Boolean,
						   GEditor*, Boolean);
extern void		add_all_gappt	(GEditor*);
extern void		geditor_clean_up(GEditor*);
extern CSA_entry_handle	geditor_nth_appt(GEditor*, int,
						   Access_data**);
extern Boolean		geditor_showing	(GEditor*);
extern void		get_geditor_vals(GEditor*);
extern void		remove_all_gaccess_data(GEditor*);
extern void		remove_all_gaccess(GEditor*);
extern void		remove_from_gaccess_list(char*, GEditor*);
extern void		set_geditor_defaults(GEditor*, Tick, Tick);
extern void		set_geditor_title(GEditor*, char*);
extern void		set_geditor_vals(GEditor*, Tick, Tick);
extern void		show_geditor	(Calendar*, Tick, Tick);

#endif
