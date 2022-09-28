#pragma ident "@(#)print.h	1.6 96/11/12 Sun Microsystems, Inc."

/*******************************************************************************
**
**  print.h
**
**  $XConsortium: print.h /main/cde1_maint/1 1995/07/14 23:13:02 drk $
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

#ifndef _PRINT_H
#define _PRINT_H

/*
 * IMPORTANT
 *
 * These indicate the type of report to be printed
 * they are used in callback for option menu in print.c
 * and stored in print data structure to record the
 * option that's selected.
 */
#define	PR_DAY_VIEW	0
#define	PR_WEEK_VIEW	1
#define	PR_MONTH_VIEW	2
#define	PR_YEAR_VIEW	3
#define	PR_APPT_LIST	4
#define	PR_TODO_LIST	5

typedef struct {
   /* widget handles */
   Widget      pdb;
   Widget      pane;
   Widget      sep;
   Widget      printer_text;
   Widget      copies_spin;
   Widget      file_toggle;
   Widget      report_type_option;
   Widget      report_type_label;
   Widget      print;
   Widget      cancel;
   Widget      help;
   Widget      printer_label;
   Widget      copies_label;
   Widget      control;
   Widget      action;
   Widget      from_label;
   Widget      from_text;
   Widget      to_label;
   Widget      to_text;
   Widget      file_text;
   Widget      args_label;
   Widget      args_text;
   int         report_type;
} _DtCmPrintData;

#endif
