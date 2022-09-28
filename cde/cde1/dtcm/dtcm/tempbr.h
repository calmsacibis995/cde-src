/*******************************************************************************
**
**  tempbr.h
**
**  #pragma ident "@(#)tempbr.h	1.13 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: tempbr.h /main/cde1_maint/1 1995/07/14 23:15:57 drk $
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

#ifndef _TEMPBR_H
#define _TEMPBR_H

#include "ansi_c.h"
#include "calendar.h"
#ifdef FNS
#include "cmfns.h"
#endif

typedef struct {
	Calendar	*cal;
	Widget		frame;
	Widget		form;
	Widget         	name_label;
	Widget		name_text;
	Widget         	show_button;
	Widget		cancel_button;
	Widget		help_button;
	Widget         	show_message;
#ifdef FNS
	Widget		close_button;
#endif
#ifdef FNS_DEMO
	Widget		sundex_button;
#endif
} Tempbr;

extern caddr_t	make_tempbr();

#endif /* _TEMPBR_H */
