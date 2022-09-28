/* $XConsortium: blank.c /main/cde1_maint/2 1995/08/29 19:56:43 gtsang $ */
/*
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*-
 * blank.c - blank screen for dtscreen, the X Window System lockscreen.
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * See dtscreen.c for copying information.
 *
 * Revision History:
 * 31-Aug-90: Written.
 */

#include "dtscreen.h"

/*ARGSUSED*/
void
drawblank(pwin)
    perwindow  *pwin;
{
}

void
initblank(pwin)
    perwindow  *pwin;
{
    XClearWindow(dsp, pwin->w);
}
