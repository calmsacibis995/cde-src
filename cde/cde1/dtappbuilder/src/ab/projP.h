
/*
 *	$XConsortium: projP.h /main/cde1_maint/2 1995/10/16 11:05:45 rcs $
 *
 * @(#)projP.h	1.5 15 Feb 1994	cde_app_builder/src/
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
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */

/*
 * projP.h
 * Private declarations for project window
 */

#ifndef _PROJP_H
#define _PROJP_H

#include <ab_private/proj.h>
#include "proj_ui.h"

extern void     projP_show_save_as_bil_chooser(
                    Widget      widget,
                    ABObj       obj
                );

extern int      projP_write_out_bil_file(
                    ABObj       obj,
                    char        *exp_path,
                    STRING      old_proj_dir,
                    BOOL        Exporting
                );

extern int      projP_save_export_bil(
                    ABObj       module_obj,
                    STRING      fullpath,
                    BOOL        Exporting
                );

extern void	projP_store_viewer(
		    Widget      widget,
		    XtPointer   client_data,
		    XtPointer   call_data
		);

extern void	projP_update_menu_items(
		    int         old_count,
		    int         new_count
		);

#endif /* _PROJP_H */
