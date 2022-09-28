/* $XConsortium: props_pu.h /main/cde1_maint/1 1995/07/17 19:43:12 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)props_pu.h	1.23 96/11/12 Sun Microsystems, Inc."

#ifndef _PROPS_PU_H
#define _PROPS_PU_H

#include "ansi_c.h"
#include "dssw.h"
#include "props.h"
#include "reminders.h"

/* Array indices for group access toggle items. */
#define GAP_VIEW        0
#define GAP_INSERT      1
#define GAP_CHANGE      2

/*******************************************************************************
**
**  Bit Masks for determining redisplay behavior.
**
*******************************************************************************/

#define PROPS_REDO_DISPLAY	0x1 /* This is all displays.  The main 
				       canvas, the appointment and todo 
				       lists, and the multi-browse window. */
#define PROPS_REDO_EDITORS	0x2 /* This is the todo and appointment 
				       editor. */
#define PROPS_REDO_PRINT_DIALOG 0x4 /* The print dialog. */

/*******************************************************************************
**
**  Supporting typedefs
**
*******************************************************************************/
typedef enum {
	EDITOR_PANE,
	DISPLAY_PANE,
	GROUP_ACCESS_PANE,
	PRINTER_OPS_PANE,
	DATE_FORMAT_PANE
} PaneType;

/*******************************************************************************
**
**  The Props Pop Up Structure
**
*******************************************************************************/
typedef struct {
	Widget		ok_button;
	Widget		apply_button;
	Widget		base_form_mgr;
	Calendar	*cal;
	Widget		category_menu;
	Boolean		changed_flag;
	Widget		close_button;
	Pixmap		contract_pixmap;	/* 16x16-contract buttons */
	Widget		defaults_button;
	Pixmap		expand_pixmap;		/* 16x16-expand buttons */
	Widget		frame;
	Widget		help_button;
	PaneType	last_props_pane;
	Widget		*pane_list;
	Pixmap		postup_pixmap;		/* 64x128-reminder image */
	Widget 		separator1;
	Widget 		separator2;
	Pixmap		xm_error_pixmap;	/* 20x20-motif error */
	Pixmap		xm_info_pixmap;		/* 11x24-motif information */
	Pixmap		xm_question_pixmap;	/* 22x22-motif question */
	Pixmap		xm_warning_pixmap;	/* 9x22-motif warning */

	/*
	 * These are temporary - until problem with resource file is worked out
	 */
	Pixmap		appt_button_xbm;
	Pixmap		todo_button_xbm;
	Pixmap		year_button_xbm;
	Pixmap		month_button_xbm;
	Pixmap		week_button_xbm;
	Pixmap		day_button_xbm;
	Pixmap		drag_icon_xbm;

	/***********************************************************************
	**
	**  Editor pane widgets
	**
	***********************************************************************/
	Widget		ep_appt_label;
	DSSW		ep_dssw;
	Widget		ep_dur_cb;
	Widget		ep_dur_label;
	Widget		ep_dur_message;
	Reminders	ep_reminders;
	Widget		privacy_label;
	Widget		privacy_menu;

	/***********************************************************************
	**
	**  Display pane widgets
	**
	***********************************************************************/
	Widget		dp_beg_label;
	Widget		dp_beg_scale;
	Widget		dp_beg_value;
	Widget		dp_day_label;
	Widget		dp_end_label;
	Widget		dp_end_scale;
	Widget		dp_end_value;
	Widget		dp_hour12_toggle;
	Widget		dp_hour24_toggle;
	Widget		dp_hour_label;
	Widget		dp_hour_rc_mgr;
	Widget		dp_init_view_label;
	Widget		dp_init_view_text;
	Widget		dp_loc_label;
	Widget		dp_loc_text;
	Widget		dp_view_label;
	Widget		dp_view_rc_mgr;
	Widget		dp_view_day_toggle;
	Widget		dp_view_month_toggle;
	Widget		dp_view_week_toggle;
	Widget		dp_view_year_toggle;
#ifdef FNS
	Widget		dp_use_fns_label;
	Widget		dp_use_fns_toggle;
#endif /* FNS */


	/***********************************************************************
	**
	**  Group Access pane widgets
	**
	***********************************************************************/
	Widget		gap_access_header;
	Widget		gap_access_list;
	Widget		gap_access_list_sw;
	Widget		gap_access_msg;
	Widget		gap_add_button;
	Widget		gap_browse_toggle;
	Widget		gap_delete_toggle;
	Widget		gap_insert_toggle;
	CSA_access_list	gap_list;
	Widget		gap_perm_label;
	Widget		gap_perm_rc_mgr;
	Widget		gap_remove_button;
	Widget		gap_user_label;
	Widget		gap_user_text;
	Boolean		v4_gap_pending_message_up;

        Widget          gap2_user_label;
        Widget          gap2_access_label;
        Widget          gap2_permissions_label;
        Widget          gap2_user_text;
        Widget          gap2_button_form;
        Widget          gap2_add_button;
        Widget          gap2_delete_button;
        Widget          gap2_change_button;
        Widget          gap2_access_list;
        Widget          gap2_perm_form;
        Widget          gap2_public_label;
        Widget          gap2_semi_label;
        Widget          gap2_private_label;
        Widget          gap2_view_label;
        Widget          gap2_insert_label;
        Widget          gap2_change_label;
        Widget          gap2_public_tog[3];
        Widget          gap2_semi_tog[3];
        Widget          gap2_private_tog[3];
	Boolean		v5_gap_pending_message_up;

	/***********************************************************************
	**
	**  Printer Ops pane widgets
	**
	***********************************************************************/
	Widget		pop_copies_label;
	Widget		pop_copies_spin;
	Widget		pop_dest_label;
	Widget		pop_dest_file_toggle;
	Widget		pop_dest_print_toggle;
	Widget		pop_dest_rc_mgr;
	Widget		pop_directory_label;
	Widget		pop_directory_text;
	Widget		pop_file_label;
	Widget		pop_file_text;
	Widget		pop_height_label;
	Widget		pop_height_text;
	Widget		pop_options_label;
	Widget		pop_options_text;
	Widget		pop_pos_bottom_label;
	Widget		pop_pos_bottom_text;
	Widget		pop_pos_label;
	Widget		pop_pos_left_label;
	Widget		pop_pos_left_text;
	Widget		pop_printer_label;
	Widget		pop_printer_text;
	Widget		pop_privacy_label;
	Widget		pop_privacy_private_toggle;
	Widget		pop_privacy_public_toggle;
	Widget		pop_privacy_rc_mgr;
	Widget		pop_privacy_semi_toggle;
	Widget		pop_units_label;
	Widget		pop_units_spin;
	Widget		pop_width_label;
	Widget		pop_width_text;

	/***********************************************************************
	**
	**  Date Format pane widgets
	**
	***********************************************************************/
	Widget		dfp_order_ddmmyy_toggle;
	Widget		dfp_order_label;
	Widget		dfp_order_mmddyy_toggle;
	Widget		dfp_order_rc_mgr;
	Widget		dfp_order_yymmdd_toggle;
	Widget		dfp_sep_blank_toggle;
	Widget		dfp_sep_dash_toggle;
	Widget		dfp_sep_label;
	Widget		dfp_sep_period_toggle;
	Widget		dfp_sep_rc_mgr;
	Widget		dfp_sep_slash_toggle;
} Props_pu;


/*******************************************************************************
**
**  External function declarations
**
*******************************************************************************/
extern void	create_all_pixmaps(Props_pu*, Widget);
extern Boolean	get_date_vals_from_ui(Props_pu*, Props*);
extern Boolean	get_display_vals_from_ui(Props_pu*, Props*);
extern Boolean	get_editor_vals_from_ui(Props_pu*, Props*);
extern Boolean	get_gap_vals_from_ui(Props_pu*, Props*);
extern Boolean	get_printer_vals_from_ui(Props_pu*, Props*);
extern Boolean	get_props_from_ui(Props_pu*, Props*, int*);
extern void	set_date_vals_on_ui(Props_pu*, Props*);
extern void	set_display_vals_on_ui(Props_pu*, Props*);
extern void	set_editor_vals_on_ui(Props_pu*, Props*);
extern void	set_gap_vals_on_ui(Props_pu*, Props*);
extern void	set_printer_vals_on_ui(Props_pu*, Props*);
extern void	set_props_on_ui	(Props_pu*, Props*);
extern void	set_props_pu_defaults(Props_pu*, Props*);
extern void	show_props_pu	(Calendar*);
extern void	props_pu_clean_up(Props_pu*);
extern Boolean	props_pu_showing(Props_pu*);
extern Boolean	p_flush_changes	(Calendar*);

#endif
