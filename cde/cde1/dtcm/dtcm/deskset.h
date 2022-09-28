/*******************************************************************************
**
**  deskset.h
**
**  #pragma ident "@(#)deskset.h	1.9 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: deskset.h /main/cde1_maint/1 1995/07/14 23:07:48 drk $
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

#ifndef _DESKSET_H
#define _DESKSET_H
 
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include "ansi_c.h"

/*
**  Location ops for ds_position_popup().
*/
typedef enum {
        DS_POPUP_RIGHT,       /* Place popup to right of baseframe */
        DS_POPUP_LEFT,        /* Place popup to left of baseframe */
        DS_POPUP_ABOVE,       /* Place popup above baseframe */
        DS_POPUP_BELOW,       /* Place popup below baseframe */
        DS_POPUP_LOR,         /* Place popup to right or left of baseframe */
        DS_POPUP_AOB,         /* Place popup above or below baseframe */
        DS_POPUP_CENTERED     /* Center popup within baseframe */
} ds_location_op;

/*
**  Function declarations.
*/
extern int		ds_force_popup_on_screen(Widget, int*, int*) ;
extern void		ds_get_screen_size(Widget, int*, int*) ;
extern int		ds_position_popup(Widget, Widget,
							ds_location_op) ;
extern char		*ds_relname	();

#endif
