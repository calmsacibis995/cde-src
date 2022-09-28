/*******************************************************************************
**
**  dssw.h
**
**  #pragma ident "@(#)dssw.h	1.21 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: dssw.h /main/cde1_maint/2 1995/10/03 12:20:10 barstow $
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

#ifndef _DSSW_H
#define _DSSW_H

#include <csa.h>
#include "ansi_c.h"
#include "calendar.h"

#define DATE_LEN	100
#define START_STOP_LEN	6
#define WHAT_LEN	256

/*******************************************************************************
**
**  DSSW enumerated types
**
*******************************************************************************/
typedef enum {
	TIME_AM,
	TIME_PM
} Time_block_op;

/*******************************************************************************
**
**  DSSW structures
**
*******************************************************************************/
typedef struct {
	char		val[START_STOP_LEN];
	Time_block_op	block;
} Time_val;

typedef struct {
	Calendar		*cal;
	Widget			date_label;
	Widget			date_text;
	char			date_val[DATE_LEN];
	Widget			dssw_form_mgr;
	Widget			parent;
	Widget			start_am;
	Widget			start_ampm_rc_mgr;
	Widget			start_label;
	Widget			start_menu;
	Widget			*start_menu_widget_list;
	int			start_menu_widget_count;
	Widget			start_pm;
	Widget			start_text;
	Time_val		start_val;
	Widget			stop_am;
	Widget			stop_ampm_rc_mgr;
	Widget			stop_label;
	Widget			stop_menu;
	Widget			*stop_menu_widget_list;
	int			stop_menu_widget_count;
	Widget			stop_pm;
	Widget			stop_text;
	Time_val		stop_val;
	Widget			what_label;
	Widget			what_text;
	Widget			what_scrollwindow;
	char			what_val[WHAT_LEN];
	Boolean			show_notime_selection;
	Boolean			show_allday_selection;
} DSSW;

/*******************************************************************************
**
**  DSSW external function declarations
**
*******************************************************************************/
extern void	build_dssw	(DSSW*, Calendar*, Widget, Boolean, Boolean);
extern Boolean	dssw_appt_to_form(DSSW*, CSA_entry_handle);
extern Boolean	dssw_attrs_to_form(DSSW*, Dtcm_appointment*);
extern Boolean	dssw_form_to_appt(DSSW*, Dtcm_appointment*, char*,
					   Tick);
extern Boolean	dssw_form_to_todo(DSSW*, Dtcm_appointment*, char*,
					   Tick);
extern void	load_dssw_times	(DSSW*, Tick, Tick, Boolean);
extern void	get_dssw_times	(DSSW*);
extern void	get_dssw_vals	(DSSW*, Tick);
extern void	set_dssw_defaults(DSSW*, Tick, Boolean);
extern void	set_dssw_times	(DSSW*);
extern void	set_dssw_vals	(DSSW*, Tick);
extern void	set_dssw_menus	(DSSW*, Props *);

#endif
