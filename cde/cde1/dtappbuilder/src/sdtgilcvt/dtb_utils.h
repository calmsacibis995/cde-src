
/*
 *	$Revision: 1.1 $
 *
 *	@(#)dtb_utils.h	1.2 01/20/95	cde_app_builder/src/dtgilcvt
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
#pragma ident "@(#)dtb_utils.h	1.2 95/01/20 SMI"

/*
 * dtb_utils.h - partial set of dtb_utils.h functions
 */
#ifndef _GILCVT_DTB_UTILS_H_
#define _GILCVT_DTB_UTILS_H_

#include <ab/util_types.h>

void 	dtb_save_command(char *argv0);
STRING	dtb_get_exe_dir(void);


#endif /* _GILCVT_DTB_UTILS_H_ */

