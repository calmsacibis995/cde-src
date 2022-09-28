/*******************************************************************************
**
**  monthglance.h
**
**  $XConsortium: monthglance.h /main/cde1_maint/2 1995/09/06 08:13:00 lehors $
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

#pragma ident "@(#)monthglance.h	1.19 96/11/12 Sun Microsystems, Inc."

#ifndef _MONTHGLANCE_H
#define _MONTHGLANCE_H

#include <csa.h>
#include "ansi_c.h"

typedef struct month {
	Widget	*hot_button;
	Widget	month_label;
	XPoint	*button_loc;
	int	ndays;
	int	dayname_height;
} Month;

extern void 	prepare_to_paint_monthview(Calendar*, XRectangle*);
extern int	count_month_appts(CSA_entry_handle*, int, Calendar*);
extern void	get_time_str	(Dtcm_appointment*, char*);
extern void	month_button	(Widget, XtPointer, XtPointer);
extern void	month_event	(XEvent*);
extern void	paint_grid	(Calendar*, XRectangle*);
extern void	print_month_pages(Calendar*);
extern void	print_month_range(Calendar*, Tick, Tick);
extern void	cleanup_after_monthview(Calendar*);


#endif
