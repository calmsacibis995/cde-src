/*******************************************************************************
**
**  datefield.h
**
**  #pragma ident "@(#)datefield.h	1.8 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: datefield.h /main/cde1_maint/1 1995/07/14 23:06:52 drk $
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

#ifndef _DATEFIELD_H
#define _DATEFIELD_H

#include <X11/Intrinsic.h>
#include "ansi_c.h"
#include "props.h"
#include "timeops.h"

/*
**  External function declaration
*/
extern char	*get_date_from_widget(Tick, Widget, OrderingType,
						SeparatorType);
extern void	set_date_in_widget(Tick, Widget, OrderingType,
						SeparatorType);

#endif
