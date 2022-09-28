/*******************************************************************************
**
**  alarm.h
**
**  #pragma ident "@(#)alarm.h	1.11 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: alarm.h /main/cde1_maint/2 1995/10/03 12:16:54 barstow $
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

#ifndef _ALARM_H
#define _ALARM_H

#include <X11/Intrinsic.h>
#include <csa.h>
#include "ansi_c.h"

extern void compose	(Calendar*, CSA_reminder_reference *, char**);
extern void flash_it	(XtPointer, XtIntervalId*);
extern void mail_it	(XtPointer, XtIntervalId*, CSA_reminder_reference *);
extern void open_it	(XtPointer, XtIntervalId*, CSA_reminder_reference *);
extern void postup_show_proc(Calendar*, CSA_reminder_reference *);
extern void reminder_driver(XtPointer, XtIntervalId*);
extern void ring_it	(XtPointer, XtIntervalId*);
extern void slp		(int x);

#endif
