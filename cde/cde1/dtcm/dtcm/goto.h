/*******************************************************************************
**
**  goto.h
**
**  #pragma ident "@(#)goto.h	1.9 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: goto.h /main/cde1_maint/1 1995/07/14 23:10:46 drk $
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

#ifndef _GOTO_H
#define _GOTO_H

#include "ansi_c.h"

typedef struct {
	Widget          	frame;
	Widget			form;
	Widget			datelabel;
	Widget			datetext;
	Widget			datebutton;
	Widget			cancelbutton;
	Widget			helpbutton;
	Widget			goto_message;
}Goto;

extern	caddr_t	make_goto(Calendar*);

#define DATESIZ 40

#endif
