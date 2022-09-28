/*******************************************************************************
**
**  rfp.h
**
**  #pragma ident "@(#)rfp.h	1.23 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: rfp.h /main/cde1_maint/2 1995/09/06 08:15:00 lehors $
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

#ifndef _RFP_H
#define _RFP_H

#include <csa.h>
#include "ansi_c.h"
#include "calendar.h"
#include "misc.h"

#define FOR_LEN	5

/*******************************************************************************
**
**  Structure for repeat, for scope, and privacy values
**
*******************************************************************************/
typedef struct {
	Widget			for_label;
	Widget			for_menu;
	Widget			for_scope;
	int			for_val;
	Widget			frequency_label;
	Widget			parent;
	Widget			privacy_label;
	Widget			privacy_menu;
	CSA_sint32		privacy_val;
	Calendar		*cal;
	Widget			repeat_form_mgr;
	Widget			repeat_label;
	Widget			repeat_menu;
	Widget			repeat_popup_frame;
	Widget			repeat_popup_menu;
	Widget			repeat_popup_text;
	Widget			repeat_apply_button;
	Widget			repeat_cancel_button;
	int			repeat_nth;
	CSA_sint32		repeat_type;
	char			*recurrence_rule;
	Widget			rfp_form_mgr;
} RFP;

/*******************************************************************************
**
**  RFP (repeat, for, privacy) external function declarations
**
*******************************************************************************/
extern void	build_rfp	(RFP*, Calendar*, Widget);
extern void	get_rfp_privacy_val(RFP*);
extern void	get_rfp_repeat_val(RFP*, time_t);
extern void	get_rfp_vals	(RFP*, time_t);
extern Boolean	rfp_attrs_to_form(RFP*, Dtcm_appointment*);
extern Boolean	rfp_appt_to_form(RFP*, CSA_entry_handle);
extern Boolean	rfp_form_to_appt(RFP*, Dtcm_appointment*, char*);
extern void	set_rfp_privacy_val(RFP*);
extern void	set_rfp_repeat_val(RFP*);
extern void	set_rfp_defaults(RFP*);
extern void	set_rfp_vals	(RFP*);
extern void	rfp_set_repeat_values(RFP*);
extern void	rfp_init	(RFP*, Calendar *, Widget);


#endif
