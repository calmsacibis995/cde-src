/*******************************************************************************
**
**  select.h
**
**  #pragma ident "@(#)select.h	1.9 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: select.h /main/cde1_maint/1 1995/07/14 23:14:47 drk $
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

#ifndef _SELECT_H
#define _SELECT_H

#include "ansi_c.h"
#include "browser.h"
#include "calendar.h"

typedef enum {
	daySelect,
	weekSelect,
	monthSelect,
	hourSelect,
	weekdaySelect,
	weekhotboxSelect 
} Selection_unit;

typedef struct {
	int row;
	int col;
	int nunits;
	int active;
	int boxw;
	int boxh;
} Selection;

typedef struct {
	int	x;
	int	y;
} pr_pos;

extern void	activate_selection(Selection*);
extern void	browser_deselect(Calendar *, Browser*);	
extern void	browser_select	(Calendar *, Browser*, pr_pos*);	
extern void	calendar_deselect(Calendar*);
extern void	calendar_select	(Calendar*, Selection_unit, caddr_t);
extern void	deactivate_selection(Selection *);
extern void	monthbox_deselect(Calendar*);	
extern void	monthbox_select	(Calendar*);
extern void	paint_selection	(Calendar*);
extern int	selection_active(Selection*);
extern void	weekchart_select(Calendar*);	

#endif
