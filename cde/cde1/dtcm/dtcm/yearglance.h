/*******************************************************************************
**
**  yearglance.h
**
**  #pragma ident "@(#)yearglance.h	1.17 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: yearglance.h /main/cde1_maint/1 1995/07/14 23:17:17 drk $
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

#ifndef _YEARGLANCE_H
#define _YEARGLANCE_H


#define ROW 0
#define COL 1

typedef struct year {
	Widget	form;
	Widget	label_form;
        Widget  *month_panels;
	Widget  year_label;
	int	year_shown;
} Year;

extern void	init_year	(Calendar *);
extern void	init_yearview	(Calendar *);
extern void	print_std_month	(FILE *, int , int , int , int );
extern void	cleanup_after_yearview (Calendar *);
extern void	year_button	(Widget, XtPointer, XtPointer);
extern void	year_event	(XEvent*);
extern CSA_return_code paint_year(Calendar*);
extern void     ps_year_button_std     ();
extern void     print_std_year         (int);
extern void     print_std_year_range   (int, int);

#endif
