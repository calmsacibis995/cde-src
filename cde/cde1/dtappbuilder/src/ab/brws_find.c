
/*
 *	$XConsortium: brws_find.c /main/cde1_maint/3 1995/10/16 13:37:09 rcs $
 *
 *	@(#)brws_find.c	1.8 12 Aug 1994
 *
 * 	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1994 Sun Microsystems, Inc.  All rights reserved.
 *
 */

#pragma ident "@(#)brws_find.c	1.8 94/08/12 SMI"

#include <sys/param.h>
#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/ScrollBar.h>
#include <Xm/SelectioB.h>
#include <ab_private/obj.h>
#include <ab/util_types.h>
#include <ab_private/abobj.h>
#include <ab_private/proj.h>
#include <ab_private/brwsP.h>
#include <ab_private/ab.h>
#include <ab_private/objxm.h>

/*************************************************************************
**                                                                      **
**       Private Function Declarations                                  **
**                                                                      **
**************************************************************************/

/*************************************************************************
**                                                                      **
**       Data     	                                                **
**                                                                      **
**************************************************************************/

void
brwsP_destroy_find_box(
    Vwr		v
)
{
    BrowserUiObjects	*ui;

    if (!v)
	return;

    ui = aob_ui_from_browser(v);

    if (!ui)
        return;

    if (ui->find_box)
    {
	XtDestroyWidget(ui->find_box);
	ui->find_box = NULL;
    }
}
