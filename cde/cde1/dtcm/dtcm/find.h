/*******************************************************************************
**
**  find.h
**
**  #pragma ident "@(#)find.h	1.11 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: find.h /main/cde1_maint/1 1995/07/14 23:09:23 drk $
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

#ifndef _FIND_H
#define _FIND_H

#include "ansi_c.h"

typedef struct tick_list {
	Tick tick;
	struct tick_list *next;
} Tick_list;

typedef enum {search_all, search_range} Searchset;

typedef struct {
	Widget          	frame;
	Widget          	form;
	Widget			apptstr_label;
	Widget			apptstr;
	Widget			search_rc_mgr;
	Widget			search_all;
	Widget			search_range;
	Widget			search_from;
	Widget			search_tolabel;
	Widget			search_to;
	Widget			find_button;
	Widget			help_button;
	Widget			date_label;
	Widget			time_label;
	Widget			what_label;
	Widget			find_list;
	Widget			find_list_sw;
	Widget			show_button;
	Widget          	cancel_button;
	Widget			find_message;
	Tick_list		*ticks;
	Searchset		search_set;
	Tick			search_from_date;
	Tick			search_to_date;
}Find;

extern caddr_t	make_find(Calendar*);
static Tick f_get_searchdate(Widget, Props*);

#endif
