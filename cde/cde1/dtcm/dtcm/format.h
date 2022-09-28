/*******************************************************************************
**
**  format.h
**
**  #pragma ident "@(#)format.h	1.14 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: format.h /main/cde1_maint/2 1995/10/10 13:24:40 pascale $
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

#ifndef _FORMAT_H
#define _FORMAT_H

#include <csa.h>
#include "ansi_c.h"
#include "props.h"

#define DEFAULT_APPT_LEN	50
#define DEFAULT_GAPPT_LEN	80


extern void format_date	(Tick, OrderingType, char*, int, int, int);
extern void format_date3(Tick, OrderingType, SeparatorType, char*);
extern Boolean format_line(Tick, char*, char*, int, Boolean,
				   DisplayType);
extern void format_line2(Dtcm_appointment*, char*, char*,
				   DisplayType);
extern void format_appt	(Dtcm_appointment*, char*, DisplayType, int);
extern void format_abbrev_appt(Dtcm_appointment*, char*, Boolean,
				   DisplayType);
extern void format_maxchars(Dtcm_appointment*, char*, int, DisplayType);
extern void format_gappt(Dtcm_appointment*, char*, char*,
				   DisplayType, int);

#endif
